/* player.cpp -- Sawteeth channel sequencer for the backslash_n demo.
 *
 * PAL() runs once per PAL screen and walks the channel's part list, then the
 * part's step list; Next() returns one sample from the instrument player.
 * Effect handling follows the original Sawteeth player: the high nibble picks
 * the effect, the low nibble its magnitude.
 *
 * Adapted from the Sawteeth engine (MIT licence, copyright 2001 Jonas Norberg)
 * to the 2000-era Player interface this demo was written against.
 */

#include "player.h"
#include "song.h"

#include <stdio.h>

Player::Player(Song *s, uint8 chno)
{
	song = s;

	if (song == NULL || chno >= song->channelcount) {
		ch = NULL;
		ip = NULL;
		step = NULL;
		currstep = NULL;
		currpart = NULL;
		seqcount = 0;
		stepc = 0;
		nexts = 0;
		amp = ampstep = freq = freqstep = cutoff = cutoffstep = 1.0f;
		return;
	}

	ch = song->chan + chno;
	step = ch->steps;

	ip = new InsPly(song);

	amp = 1.0f;
	ampstep = 0.0f;
	freq = 1.0f;
	freqstep = 0.0f;
	cutoff = 1.0f;
	cutoffstep = 1.0f;

	JumpPos(0, 0, 0);
}

Player::~Player()
{
	delete ip;
}

void Player::JumpPos(uint8 seqpos, uint8 steppos, uint8 pal)
{
	if (ch == NULL || song == NULL || ch->steps == NULL)
		return;

	seqcount = seqpos;
	stepc = steppos;
	nexts = 0;

	if (seqcount >= ch->len)
		seqcount = 0;

	uint8 part = step[seqcount].part;
	if (part >= song->partcount)
		part = 0;

	currpart = song->parts + part;

	if (stepc >= currpart->len)
		stepc = 0;

	currstep = currpart->steps + stepc;

	if (pal > 0) {
		stepc++;
		if (stepc >= currpart->len) {
			stepc = 0;
			seqcount++;
			if (seqcount >= ch->len)
				seqcount = 0;
			part = step[seqcount].part;
			if (part >= song->partcount)
				part = 0;
			currpart = song->parts + part;
		}

		nexts = currpart->sps > pal ? currpart->sps - pal : 0;
		currstep = currpart->steps + stepc;
	}
}

void Player::PAL()
{
	if (ch == NULL || currpart == NULL || ip == NULL)
		return;

	if (nexts < 1) {
		nexts = currpart->sps > 0 ? currpart->sps : 1;
		currstep = currpart->steps + stepc;

		if (currstep->ins) {
			ip->TrigADSR(currstep->ins);

			amp = 1.0f;
			ampstep = 0.0f;
			cutoff = 1.0f;
			cutoffstep = 1.0f;
		}

		if (currstep->note) {
			int32 note = currstep->note + step[seqcount].transp;

			if (note < 0)
				note = 0;
			if (note > 22 * 12 - 1)
				note = 22 * 12 - 1;

			targetfreq = song->N2F[note];
			freqstep = 1.0f;
			if ((currstep->eff & 0xf0) != 0x30)
				freq = targetfreq;
		}

		/* effects */
		const float divider = 50.0f;

		if (currstep->eff) {
			switch (currstep->eff & 0xf0) {
				case 0x10:
					targetfreq = 44100.0f;
					freqstep = 1.0f + ((currstep->eff & 0xf) + 1) / (divider * 3);
					break;

				case 0x20:
					targetfreq = 1.0f;
					freqstep = 1.0f - ((currstep->eff & 0xf) + 1) / (divider * 3);
					break;

				case 0x30:
					if (targetfreq > freq)
						freqstep = 1.0f + ((currstep->eff & 0xf) + 1) / divider;
					else
						freqstep = 1.0f - ((currstep->eff & 0xf) + 1) / divider;
					break;

				/* filter */
				case 0x70:
					cutoffstep = 1.0f - ((currstep->eff & 0xf) + 1) / 256.0f;
					break;

				case 0x80:
					cutoffstep = 1.0f + ((currstep->eff & 0xf) + 1) / 256.0f;
					break;

				case 0x90:
					cutoffstep = 1.0f;
					cutoff = ((currstep->eff & 0xf) + 1) / 16.0f;
					cutoff *= cutoff;
					break;

				/* amplitude */
				case 0xa0:
					ampstep = -((currstep->eff & 0xf) + 1) / 256.0f;
					break;

				case 0xb0:
					ampstep = ((currstep->eff & 0xf) + 1) / 256.0f;
					break;

				case 0xc0:
					ampstep = 0.0f;
					amp = (currstep->eff & 0xf) / 15.0f;
					break;
			}
		}

		stepc++;
		if (stepc >= currpart->len) {
			stepc = 0;
			seqcount++;
			if (seqcount >= ch->len)
				seqcount = 0;

			uint8 part = step[seqcount].part;
			if (part >= song->partcount)
				part = 0;
			currpart = song->parts + part;
		}
	}

	nexts--;

	cutoff *= cutoffstep;
	if (cutoff < 0.0f) {
		cutoff = 0.0f;
		cutoffstep = 1.0f;
	} else if (cutoff > 1.0f) {
		cutoff = 1.0f;
		cutoffstep = 1.0f;
	}

	freq *= freqstep;
	if (freqstep > 1.0001f) {
		if (freq > targetfreq) {
			freq = targetfreq;
			freqstep = 1.0f;
		}
	} else if (freqstep < 0.9999f) {
		if (freq < targetfreq) {
			freq = targetfreq;
			freqstep = 1.0f;
		}
	}

	amp += ampstep;
	if (amp < 0.0f) {
		amp = 0.0f;
		ampstep = 0.0f;
	} else if (amp > 1.0f) {
		amp = 1.0f;
		ampstep = 0.0f;
	}

	ip->SetAmp(amp);
	ip->SetFreq(freq);
	ip->SetCutOff(cutoff);

	ip->PAL();
}

float Player::Next()
{
	if (ip == NULL)
		return 0.0f;

	return ip->Next();
}

void Player::DumpPart(uint8 part)
{
	if (song == NULL || part >= song->partcount)
		return;

	Part *p = song->parts + part;

	printf("part %d (%s) sps %d len %d\n", part, p->name ? p->name : "",
		p->sps, p->len);

	for (int32 c = 0; c < p->len; c++)
		printf("  %02d: ins %3d eff %02x note %3d\n", (int)c,
			p->steps[c].ins, p->steps[c].eff, p->steps[c].note);
}

void Player::DumpIns(uint8 ins_no)
{
	if (song == NULL || ins_no < 1 || ins_no > song->instrumentcount)
		return;

	Ins *i = song->ins + ins_no;

	printf("ins %d (%s) sps %d len %d res %d\n", ins_no,
		i->name ? i->name : "", i->sps, i->len & 0x7f, i->res);

	for (int32 c = 0; c < (i->len & 0x7f); c++)
		printf("  %02d: mask %02x note %3d\n", (int)c, i->steps[c].mask,
			i->steps[c].note);
}
