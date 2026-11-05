/* song.cpp -- song file loader and mixer for the backslash_n demo.
 *
 * The 2000-era Sawteeth engine (Song/Player/InsPly/Wave) was never part of this
 * repository, so this file and its neighbours implement the API declared in
 * st/song.h for Haiku.  The file format and the playback model follow the
 * Sawteeth engine this demo was written against (MIT licence, copyright 2001
 * Jonas Norberg), which is what makes data/Spruce-ComicFakery.NNN load and
 * drive the scene changes through the Break() hook again.
 *
 * Text song layout, magic "SWTT":
 *
 *   spspal                             samples per PAL screen (50 Hz tick)
 *   channelcount
 *   channelcount x { len, len x { part, transp } }
 *   partcount
 *   partcount x { sps, len, len x { ins, eff, note } }
 *   instrumentcount
 *   instrumentcount x { filterpoints, (time, lev)*,
 *                       amppoints,     (time, lev)*,
 *                       vibs, vibd, pwms, pwmd, res, sps, len,
 *                       (len & 0x7f) x { mask, note } }
 *   breakpcount
 *   breakpcount x { PAL, command }
 *   name, channel names, part names, instrument names
 *
 * Instrument 0 is the "no instrument" slot, parts are 0 based.
 */

#include "song.h"
#include "player.h"
#include "txt.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

static void *CALLOC(int count, int size)
{
	if (count < 1)
		count = 1;
	if (size < 1)
		size = 1;

	return calloc(count, size);
}

/* ------------------------------------------------------------- frequency tables */

static void build_tables(float *n2f, float *r2f, int count)
{
	const double MUL = 1.0594630943593;	// 2^(1/12)
	const double anchor = 1.02197486445547712033;
	double octbase = anchor;
	int i = 0;

	for (int oc = 0; oc < 22; oc++) {
		double base = octbase;
		for (int c = 0; c < 12 && i < count; c++) {
			n2f[i++] = (float)base;
			base *= MUL;
		}
		octbase *= 2.0;
	}

	octbase = 1.0;
	i = 0;
	for (int oc = 0; oc < 22; oc++) {
		double base = octbase;
		for (int c = 0; c < 12 && i < count; c++) {
			r2f[i++] = (float)base;
			base *= MUL;
		}
		octbase *= 2.0;
	}
}

/* ------------------------------------------------------- names (BeOS API style) */

void Channel::SetName(char *iname)
{
	free(name);
	name = strdup(iname != NULL ? iname : "");
}

void Part::SetName(char *iname)
{
	free(name);
	name = strdup(iname != NULL ? iname : "");
}

void Ins::SetName(char *iname)
{
	free(name);
	name = strdup(iname != NULL ? iname : "");
}

/* --------------------------------------------------------------- Song lifetime */

Song::Song(char *filename)
{
	N2F = (float *)malloc(22 * 12 * sizeof(float));
	R2F = (float *)malloc(22 * 12 * sizeof(float));
	build_tables(N2F, R2F, 22 * 12);

	name = NULL;
	spspal = 882;

	channelcount = 0;
	partcount = 0;
	instrumentcount = 0;
	breakpcount = 0;

	chan = NULL;
	parts = NULL;
	ins = NULL;
	breakpoints = NULL;

	bpcount = 0;
	init = B_OK;

	PALs = 0;
	tmp = 0.0f;
	pal = 0;
	p = NULL;
	smp_left = 0;

	AllocDefSong();

	if (filename != NULL)
		Load(filename);
}

Song::~Song()
{
	FreeSong();

	free(N2F);
	free(R2F);
}

void Song::FreeSong()
{
	if (p != NULL) {
		for (uint8 c = 0; c < channelcount; c++)
			delete p[c];
		delete [] p;
		p = NULL;
	}

	if (chan != NULL) {
		for (uint8 c = 0; c < channelcount; c++) {
			free(chan[c].steps);
			free(chan[c].name);
		}
		free(chan);
		chan = NULL;
	}

	if (parts != NULL) {
		for (uint8 c = 0; c < partcount; c++) {
			free(parts[c].steps);
			free(parts[c].name);
		}
		free(parts);
		parts = NULL;
	}

	if (ins != NULL) {
		for (uint8 c = 0; c <= instrumentcount; c++) {
			free(ins[c].steps);
			free(ins[c].amp);
			free(ins[c].filter);
			free(ins[c].name);
		}
		free(ins);
		ins = NULL;
	}

	free(breakpoints);
	breakpoints = NULL;

	free(name);
	name = NULL;

	channelcount = 0;
	partcount = 0;
	instrumentcount = 0;
	breakpcount = 0;
}

/* A tiny silent song so that Play() is always safe, even if a load failed. */
void Song::AllocDefSong()
{
	spspal = 882;

	channelcount = 1;
	partcount = 1;
	instrumentcount = 1;
	breakpcount = 0;

	chan = (Channel *)CALLOC(1, sizeof(Channel));
	chan[0].len = 1;
	chan[0].steps = (ChStep *)CALLOC(1, sizeof(ChStep));
	chan[0].steps[0].part = 0;
	chan[0].steps[0].transp = 0;
	chan[0].SetName((char *)"channel");

	parts = (Part *)CALLOC(1, sizeof(Part));
	parts[0].sps = 6;
	parts[0].len = 1;
	parts[0].steps = (Step *)CALLOC(1, sizeof(Step));
	parts[0].steps[0].ins = 1;
	parts[0].steps[0].eff = 0;
	parts[0].steps[0].note = 60;
	parts[0].SetName((char *)"part");

	ins = (Ins *)CALLOC(2, sizeof(Ins));
	ins[1].filterpoints = 1;
	ins[1].filter = (TimeLev *)CALLOC(1, sizeof(TimeLev));
	ins[1].filter[0].time = 1;
	ins[1].filter[0].lev = 255;
	ins[1].amppoints = 2;
	ins[1].amp = (TimeLev *)CALLOC(2, sizeof(TimeLev));
	ins[1].amp[0].time = 1;
	ins[1].amp[0].lev = 255;
	ins[1].amp[1].time = 200;
	ins[1].amp[1].lev = 0;
	ins[1].sps = 6;
	ins[1].len = 1;
	ins[1].steps = (InsStep *)CALLOC(1, sizeof(InsStep));
	ins[1].steps[0].mask = 0x02;	// square wave
	ins[1].steps[0].note = 0;
	ins[1].SetName((char *)"default");

	breakpoints = NULL;
	name = strdup("untitled");

	p = new Player *[channelcount];
	for (uint8 c = 0; c < channelcount; c++)
		p[c] = new Player(this, c);

	smp_left = 0;
	PALs = 0;
	init = B_OK;
}

/* ---------------------------------------------------------------------- loading */

status_t Song::Load(char *filename)
{
	if (filename == NULL)
		return init = B_ERROR;

	txt file(filename);
	if (file.InitCheck() != B_OK) {
		fprintf(stderr, "song: could not open \"%s\"\n", filename);
		return init = B_ERROR;
	}

	/* the magic is four characters; read them as bytes so the check does not
	 * depend on host byte order (and works for the text format)
	 */
	file.IsBin(true);
	int magic[4];
	int c = file.NextVal(1);
	while (c == ' ' || c == '\n' || c == '\r' || c == '\t')
		c = file.NextVal(1);
	magic[0] = c;
	magic[1] = file.NextVal(1);
	magic[2] = file.NextVal(1);
	magic[3] = file.NextVal(1);
	file.IsBin(false);

	if (magic[0] != 'S' || magic[1] != 'W' || magic[2] != 'T' || magic[3] != 'T') {
		fprintf(stderr, "song: \"%s\" is not a Sawteeth song\n", filename);
		return init = B_ERROR;
	}

	FreeSong();

	spspal = (uint16)file.NextVal(2);
	if (spspal < 1)
		spspal = 882;

	/* channels */
	channelcount = (uint8)file.NextVal();
	if (channelcount < 1) {
		fprintf(stderr, "song: no channels\n");
		FreeSong();
		AllocDefSong();
		return init = B_ERROR;
	}
	if (channelcount > CHN)
		channelcount = CHN;

	chan = (Channel *)CALLOC(channelcount, sizeof(Channel));

	for (uint8 ch = 0; ch < channelcount; ch++) {
		chan[ch].len = (uint8)file.NextVal();
		if (chan[ch].len < 1)
			chan[ch].len = 1;
		if (chan[ch].len > CHNSTEPS - 1)
			chan[ch].len = (uint8)(CHNSTEPS - 1);

		chan[ch].steps = (ChStep *)CALLOC(chan[ch].len, sizeof(ChStep));
		for (uint8 s = 0; s < chan[ch].len; s++) {
			chan[ch].steps[s].part = (uint8)file.NextVal();
			chan[ch].steps[s].transp = (int8)file.NextVal();
		}
	}

	/* parts */
	partcount = (uint8)file.NextVal();
	if (partcount < 1) {
		fprintf(stderr, "song: no parts\n");
		FreeSong();
		AllocDefSong();
		return init = B_ERROR;
	}
	if (partcount > PARTS - 1)
		partcount = (uint8)(PARTS - 1);

	parts = (Part *)CALLOC(partcount, sizeof(Part));

	for (uint8 pa = 0; pa < partcount; pa++) {
		parts[pa].sps = (uint8)file.NextVal();
		if (parts[pa].sps < 1)
			parts[pa].sps = 1;

		parts[pa].len = (uint8)file.NextVal();
		if (parts[pa].len < 1)
			parts[pa].len = 1;
		if (parts[pa].len > PARTSTEPS - 1)
			parts[pa].len = (uint8)(PARTSTEPS - 1);

		parts[pa].steps = (Step *)CALLOC(parts[pa].len, sizeof(Step));
		for (uint8 s = 0; s < parts[pa].len; s++) {
			Step &st = parts[pa].steps[s];
			st.ins = (uint8)file.NextVal();
			st.eff = (uint8)file.NextVal();
			st.note = (uint8)file.NextVal();
		}
	}

	/* instruments; slot 0 is the "no instrument" slot */
	instrumentcount = (uint8)file.NextVal();
	if (instrumentcount < 1) {
		fprintf(stderr, "song: no instruments\n");
		FreeSong();
		AllocDefSong();
		return init = B_ERROR;
	}
	if (instrumentcount > INS - 1)
		instrumentcount = (uint8)(INS - 1);

	ins = (Ins *)CALLOC(instrumentcount + 1, sizeof(Ins));

	for (uint8 i = 1; i <= instrumentcount; i++) {
		Ins &instrument = ins[i];

		instrument.filterpoints = (uint8)file.NextVal();
		if (instrument.filterpoints < 1)
			instrument.filterpoints = 1;
		if (instrument.filterpoints > INSPOINTS)
			instrument.filterpoints = INSPOINTS;

		instrument.filter = (TimeLev *)CALLOC(instrument.filterpoints,
			sizeof(TimeLev));
		for (uint8 s = 0; s < instrument.filterpoints; s++) {
			instrument.filter[s].time = (uint8)file.NextVal();
			instrument.filter[s].lev = (uint8)file.NextVal();
		}

		instrument.amppoints = (uint8)file.NextVal();
		if (instrument.amppoints < 1)
			instrument.amppoints = 1;
		if (instrument.amppoints > INSPOINTS)
			instrument.amppoints = INSPOINTS;

		instrument.amp = (TimeLev *)CALLOC(instrument.amppoints,
			sizeof(TimeLev));
		for (uint8 s = 0; s < instrument.amppoints; s++) {
			instrument.amp[s].time = (uint8)file.NextVal();
			instrument.amp[s].lev = (uint8)file.NextVal();
		}

		instrument.vibs = (uint8)file.NextVal();
		instrument.vibd = (uint8)file.NextVal();
		instrument.pwms = (uint8)file.NextVal();
		instrument.pwmd = (uint8)file.NextVal();
		instrument.res = (uint8)file.NextVal();

		instrument.sps = (uint8)file.NextVal();
		if (instrument.sps < 1)
			instrument.sps = 1;

		/* msb means the instrument pattern loops */
		instrument.len = (uint8)file.NextVal();
		uint8 steps = instrument.len & 0x7f;

		instrument.steps = (InsStep *)CALLOC(steps > 0 ? steps : 1,
			sizeof(InsStep));
		for (uint8 s = 0; s < steps; s++) {
			instrument.steps[s].mask = (uint8)file.NextVal();
			instrument.steps[s].note = (uint8)file.NextVal();
		}
	}

	/* breakpoints, in PAL screens */
	breakpcount = (uint8)file.NextVal(2);
	breakpoints = (BreakPoint *)CALLOC(breakpcount > 0 ? breakpcount : 1,
		sizeof(BreakPoint));

	for (uint8 b = 0; b < breakpcount; b++) {
		breakpoints[b].PAL = (uint32)file.NextVal(2);
		breakpoints[b].command = (uint32)file.NextVal(2);
	}

	/* names */
	char *n = file.NextString();
	name = strdup(n != NULL ? n : "");

	for (uint8 ch = 0; ch < channelcount; ch++)
		chan[ch].SetName(file.NextString());

	for (uint8 pa = 0; pa < partcount; pa++)
		parts[pa].SetName(file.NextString());

	for (uint8 i = 1; i <= instrumentcount; i++)
		ins[i].SetName(file.NextString());

	/* voices */
	p = new Player *[channelcount];
	for (uint8 ch = 0; ch < channelcount; ch++)
		p[ch] = new Player(this, ch);

	PALs = 0;
	smp_left = 0;
	init = B_OK;

	return B_OK;
}

status_t Song::InitCheck()
{
	return init;
}

/* ---------------------------------------------------------------------- playing */

void Song::Play(float *Buffer, uint32 SampleCount)
{
	if (Buffer == NULL)
		return;

	if (init != B_OK || p == NULL || spspal < 1) {
		for (uint32 i = 0; i < SampleCount; i++)
			Buffer[i] = 0.0f;
		return;
	}

	for (uint32 i = 0; i < SampleCount; i++) {
		if (smp_left == 0) {
			for (uint8 ch = 0; ch < channelcount; ch++) {
				if (p[ch] != NULL)
					p[ch]->PAL();
			}

			smp_left = spspal;

			PALs++;
			for (uint8 b = 0; b < breakpcount; b++) {
				if (breakpoints[b].PAL != PALs)
					continue;

				Break(breakpoints[b].command);

				if (b == breakpcount - 1) {
					/* last breakpoint: start the tune over so that the
					 * scene script can run again
					 */
					PALs = 0;
					for (uint8 ch = 0; ch < channelcount; ch++) {
						if (p[ch] != NULL)
							p[ch]->JumpPos(0, 0, 0);
					}
				}
				break;
			}
		}

		float sample = 0.0f;
		for (uint8 ch = 0; ch < channelcount; ch++) {
			if (p[ch] != NULL)
				sample += p[ch]->Next();
		}

		sample *= 0.25f;
		if (sample > 1.0f)
			sample = 1.0f;
		else if (sample < -1.0f)
			sample = -1.0f;

		Buffer[i] = sample;
		smp_left--;
	}
}

void Song::Break(uint32 /*command*/)
{
	// a plain song ignores breakpoints
}

uint32 Song::GetPos()
{
	return PALs;
}

void Song::SetPos(uint32 pos)
{
	PALs = pos;

	for (uint8 ch = 0; ch < channelcount; ch++) {
		if (p[ch] != NULL)
			p[ch]->JumpPos(0, 0, 0);
	}
}

/* ----------------------------------------------------------------------- saving */

void Song::Save(char *filename, bool bin)
{
	txt file(filename, true);
	if (file.InitCheck() != B_OK)
		return;

	file.IsBin(bin);
	file.String((char *)"SWTT");
	file.Comment((char *)"written by backslash_n");

	file.Val(spspal, 2);

	file.Comment((char *)"# of channels");
	file.Val(channelcount);
	for (uint8 ch = 0; ch < channelcount; ch++) {
		file.Comment((char *)"len");
		file.Val(chan[ch].len);
		file.Comment((char *)"part-transp");
		for (uint8 s = 0; s < chan[ch].len; s++) {
			file.Val(chan[ch].steps[s].part);
			file.Val(chan[ch].steps[s].transp);
		}
	}

	file.Comment((char *)"# of parts");
	file.Val(partcount);
	for (uint8 pa = 0; pa < partcount; pa++) {
		file.Comment((char *)"sps & len");
		file.Val(parts[pa].sps);
		file.Val(parts[pa].len);
		file.Comment((char *)"ins - eff - note");
		for (uint8 s = 0; s < parts[pa].len; s++) {
			file.Val(parts[pa].steps[s].ins);
			file.Val(parts[pa].steps[s].eff);
			file.Val(parts[pa].steps[s].note);
		}
	}

	file.Comment((char *)"# of ins");
	file.Val(instrumentcount);
	for (uint8 i = 1; i <= instrumentcount; i++) {
		Ins &instrument = ins[i];

		file.Comment((char *)"points - fadsr time-lev");
		file.Val(instrument.filterpoints);
		for (uint8 s = 0; s < instrument.filterpoints; s++) {
			file.Val(instrument.filter[s].time);
			file.Val(instrument.filter[s].lev);
		}

		file.Comment((char *)"points - adsr time-lev");
		file.Val(instrument.amppoints);
		for (uint8 s = 0; s < instrument.amppoints; s++) {
			file.Val(instrument.amp[s].time);
			file.Val(instrument.amp[s].lev);
		}

		file.Comment((char *)"vibs, vibd, pwms, pwmd");
		file.Val(instrument.vibs);
		file.Val(instrument.vibd);
		file.Val(instrument.pwms);
		file.Val(instrument.pwmd);

		file.Comment((char *)"res");
		file.Val(instrument.res);
		file.Comment((char *)"sps");
		file.Val(instrument.sps);
		file.Comment((char *)"len bit 7 means loop");
		file.Val(instrument.len);
		file.Comment((char *)"track mask - note");
		for (uint8 s = 0; s < (instrument.len & 0x7f); s++) {
			file.Val(instrument.steps[s].mask);
			file.Val(instrument.steps[s].note);
		}
	}

	file.Comment((char *)"breakpoints, PAL - command");
	file.Val(breakpcount, 2);
	for (uint8 b = 0; b < breakpcount; b++) {
		file.Val((int)breakpoints[b].PAL, 2);
		file.Val((int)breakpoints[b].command, 2);
	}

	file.String(name);
	for (uint8 ch = 0; ch < channelcount; ch++)
		file.String(chan[ch].name);
	for (uint8 pa = 0; pa < partcount; pa++)
		file.String(parts[pa].name);
	for (uint8 i = 1; i <= instrumentcount; i++)
		file.String(ins[i].name);
}

/* -------------------------------------------------------------- editor interface */

uint8 Song::ChannelCount()
{
	return channelcount;
}

Channel *Song::GetChannel(uint8 c)
{
	if (c >= channelcount)
		return NULL;
	return chan + c;
}

uint8 Song::PartCount()
{
	return partcount;
}

Part *Song::GetPart(uint8 part)
{
	if (part >= partcount)
		return NULL;
	return parts + part;
}

uint8 Song::InsCount()
{
	return instrumentcount;
}

Ins *Song::GetIns(uint8 i)
{
	if (i < 1 || i > instrumentcount)
		return NULL;
	return ins + i;
}

uint8 Song::NewChannel()
{
	if (channelcount >= CHN)
		return channelcount;

	Channel *new_chan = (Channel *)realloc(chan,
		(channelcount + 1) * sizeof(Channel));
	if (new_chan == NULL)
		return channelcount;
	chan = new_chan;
	memset(chan + channelcount, 0, sizeof(Channel));

	chan[channelcount].len = 1;
	chan[channelcount].steps = (ChStep *)CALLOC(1, sizeof(ChStep));
	chan[channelcount].SetName((char *)"channel");

	Player **new_p = new Player *[channelcount + 1];
	for (uint8 c = 0; c < channelcount; c++)
		new_p[c] = p != NULL ? p[c] : NULL;
	delete [] p;
	p = new_p;

	channelcount++;
	p[channelcount - 1] = new Player(this, channelcount - 1);

	return channelcount - 1;
}

status_t Song::RemoveChannel(uint8 channel)
{
	if (channel >= channelcount || channelcount < 2)
		return B_ERROR;

	delete p[channel];

	for (uint8 c = channel; c + 1 < channelcount; c++) {
		chan[c] = chan[c + 1];
		p[c] = p[c + 1];
	}
	channelcount--;

	return B_OK;
}

uint8 Song::NewPart(Part *copy_me)
{
	if (partcount >= PARTS)
		return partcount;

	Part *new_parts = (Part *)realloc(parts, (partcount + 1) * sizeof(Part));
	if (new_parts == NULL)
		return partcount;
	parts = new_parts;
	memset(parts + partcount, 0, sizeof(Part));

	Part &part = parts[partcount];
	part.sps = copy_me != NULL ? copy_me->sps : 6;
	part.len = copy_me != NULL ? copy_me->len : 1;
	if (part.len < 1)
		part.len = 1;
	part.steps = (Step *)CALLOC(part.len, sizeof(Step));
	if (copy_me != NULL && copy_me->steps != NULL) {
		memcpy(part.steps, copy_me->steps, part.len * sizeof(Step));
		part.SetName(copy_me->name);
	} else
		part.SetName((char *)"part");

	partcount++;

	return partcount - 1;
}

status_t Song::RemovePart(uint8 part)
{
	if (part >= partcount || partcount < 2)
		return B_ERROR;

	/* refuse if a channel still plays it */
	for (uint8 ch = 0; ch < channelcount; ch++) {
		for (uint8 s = 0; s < chan[ch].len; s++) {
			if (chan[ch].steps[s].part == part)
				return B_ERROR;
		}
	}

	free(parts[part].steps);
	free(parts[part].name);

	for (uint8 c = part; c + 1 < partcount; c++)
		parts[c] = parts[c + 1];
	partcount--;

	for (uint8 ch = 0; ch < channelcount; ch++) {
		for (uint8 s = 0; s < chan[ch].len; s++) {
			if (chan[ch].steps[s].part > part)
				chan[ch].steps[s].part--;
		}
	}

	return B_OK;
}

uint8 Song::NewIns(Ins *copy_me)
{
	if (instrumentcount >= INS)
		return instrumentcount;

	Ins *new_ins = (Ins *)realloc(ins, (instrumentcount + 2) * sizeof(Ins));
	if (new_ins == NULL)
		return instrumentcount;
	ins = new_ins;
	memset(ins + instrumentcount + 1, 0, sizeof(Ins));

	instrumentcount++;

	Ins &instrument = ins[instrumentcount];
	if (copy_me != NULL) {
		instrument = *copy_me;
		instrument.steps = (InsStep *)CALLOC(copy_me->len & 0x7f,
			sizeof(InsStep));
		if (copy_me->steps != NULL) {
			memcpy(instrument.steps, copy_me->steps,
				(copy_me->len & 0x7f) * sizeof(InsStep));
		}
	} else {
		instrument.filterpoints = 1;
		instrument.filter = (TimeLev *)CALLOC(1, sizeof(TimeLev));
		instrument.filter[0].time = 1;
		instrument.filter[0].lev = 255;
		instrument.amppoints = 1;
		instrument.amp = (TimeLev *)CALLOC(1, sizeof(TimeLev));
		instrument.amp[0].time = 1;
		instrument.amp[0].lev = 255;
		instrument.sps = 6;
		instrument.len = 1;
		instrument.steps = (InsStep *)CALLOC(1, sizeof(InsStep));
		instrument.steps[0].mask = 0x01;
		instrument.SetName((char *)"ins");
	}

	return instrumentcount;
}

uint8 Song::BreakPointCount()
{
	return breakpcount;
}

uint8 Song::GetBreakPointIndex(uint32 PAL)
{
	for (uint8 b = 0; b < breakpcount; b++) {
		if (breakpoints[b].PAL == PAL)
			return b;
	}
	return 0;
}

uint32 Song::GetBreakPointByIndex(uint8 index)
{
	if (index >= breakpcount)
		return 0;
	return breakpoints[index].command;
}

void Song::SetBreakPointByIndex(uint8 index, uint32 command)
{
	if (index >= breakpcount)
		return;
	breakpoints[index].command = command;
}

uint32 Song::GetBreakPoint(uint32 PAL)
{
	for (uint8 b = 0; b < breakpcount; b++) {
		if (breakpoints[b].PAL == PAL)
			return breakpoints[b].command;
	}
	return 0;
}

void Song::NewBreakPoint(uint32 PAL, uint32 command)
{
	if (breakpcount >= BREAKPOINTS)
		return;

	uint8 at = 0;
	while (at < breakpcount && breakpoints[at].PAL < PAL)
		at++;

	BreakPoint *new_bp = (BreakPoint *)realloc(breakpoints,
		(breakpcount + 1) * sizeof(BreakPoint));
	if (new_bp == NULL)
		return;
	breakpoints = new_bp;

	for (uint8 c = breakpcount; c > at; c--)
		breakpoints[c] = breakpoints[c - 1];

	breakpoints[at].PAL = PAL;
	breakpoints[at].command = command;
	breakpcount++;
}

void Song::RemoveBreakPoint(uint32 PAL)
{
	uint8 at = breakpcount;
	while (at > 0 && breakpoints[at - 1].PAL != PAL)
		at--;

	if (at == 0)
		return;

	at--;
	for (uint8 c = at; c + 1 < breakpcount; c++)
		breakpoints[c] = breakpoints[c + 1];
	breakpcount--;
}
