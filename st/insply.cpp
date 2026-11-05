/* insply.cpp -- Sawteeth instrument player for the backslash_n demo.
 *
 * One instance per channel.  The sequencer (Player) calls PAL() once per PAL
 * screen (spspal samples, 50 Hz) and Next() for every sample.  Envelope and
 * filter points are timed in PAL screens, exactly as in the song files; the
 * oscillator runs at sample rate.
 *
 * Adapted from the Sawteeth engine (MIT licence, copyright 2001 Jonas Norberg)
 * to the 2000-era InsPly interface this demo was written against.
 */

#include "insply.h"
#include "song.h"

#include <math.h>

InsPly::InsPly(Song *s)
{
	song = s;

	w = new Wave(440.0f, SQR);
	vib = new Wave(5.0f, TRI);
	pwm = new Wave(5.0f, TRI);

	vamp = 0.0f;
	pamp = 0.0f;

	ins = NULL;
	currstep = NULL;
	stepc = 0;
	nexts = 0;

	currins = NULL;
	steps = NULL;

	curramp = 0.0f;
	ampstep = 0.0f;
	adsr = 0;
	nextadsr = 0;

	currf = 0.0f;
	fstep = 0.0f;
	fadsr = 0;
	nextfadsr = 0;

	llast = 0.0f;
	last = 0.0f;
	curr = 0.0f;

	insfreq = 1.0f;

	currpartfreq = 1.0f;
	currpartamp = 1.0f;
	currpartco = 1.0f;

	res = 0.0f;
	amp = 0.0f;
	cutoff = 1.0f;
}

void InsPly::TrigADSR(uint8 i)
{
	if (song == NULL || i < 1 || i > song->instrumentcount)
		return;

	ins = song->ins + i;
	currins = ins;
	steps = ins->steps;

	if (steps == NULL || (ins->len & 0x7f) < 1)
		return;

	stepc = 0;
	nexts = 0;

	curramp = 0.0f;
	ampstep = 0.0f;
	adsr = 0;
	nextadsr = 0;

	currf = 0.0f;
	fstep = 0.0f;
	fadsr = 0;
	nextfadsr = 0;

	vib->SetFreq(ins->vibs * 50.0f);
	pwm->SetFreq(ins->pwms * 5.0f);

	vamp = ins->vibd / 255.0f;
	pamp = ins->pwmd / 255.0f;

	res = ins->res / 255.0f;
}

void InsPly::PAL()
{
	if (currins == NULL)
		return;

	/* instrument steps */
	if (nexts < 1) {
		uint8 len = currins->len & 0x7f;
		bool loop = (currins->len & 0x80) != 0;

		if (len < 1)
			return;

		currstep = steps + stepc;
		nexts = currins->sps > 0 ? currins->sps : 1;

		if (currstep->note) {
			if (currstep->mask & 0x80)
				insfreq = song->R2F[currstep->note];
			else
				insfreq = song->N2F[currstep->note];
		}

		if (currstep->mask & 0x0f)
			w->SetForm(currstep->mask & 0x0f);

		stepc++;
		if (stepc >= len)
			stepc = loop ? 0 : len - 1;
	}

	/* amp envelope */
	if (nextadsr < 1) {
		if (adsr < currins->amppoints) {
			nextadsr = 1 + currins->amp[adsr].time;
			ampstep = ((currins->amp[adsr].lev / 257.0f) - curramp) / nextadsr;
			adsr++;
		} else
			ampstep = 0.0f;
	}

	/* filter envelope */
	if (nextfadsr < 1) {
		if (fadsr < currins->filterpoints) {
			float target = currins->filter[fadsr].lev / 257.0f;

			nextfadsr = 1 + currins->filter[fadsr].time;
			target *= target * target;
			fstep = (target - currf) / nextfadsr;
			fadsr++;
		} else
			fstep = 0.0f;
	}

	nextadsr--;
	nextfadsr--;
	nexts--;

	/* pitch and pulse width */
	w->SetFreq(currpartfreq * insfreq * (1.0f + vamp * vib->Next()));
	w->SetPWM(0.5f + pamp * 0.45f * pwm->Next());

	/* levels */
	llast = last;
	curramp += ampstep;
	currf += fstep;

	if (curramp < 0.0f)
		curramp = 0.0f;
	else if (curramp > 1.0f)
		curramp = 1.0f;

	if (currf < 0.0f)
		currf = 0.0f;
	else if (currf > 1.0f)
		currf = 1.0f;

	amp = curramp * currpartamp;
	cutoff = currf * currpartco;
}

float InsPly::Next()
{
	if (currins == NULL)
		return 0.0f;

	float sample = w->Next();

	/* one pole low pass; resonance opens it up a little */
	float coef = cutoff;
	if (coef < 0.0f)
		coef = 0.0f;
	else if (coef > 1.0f)
		coef = 1.0f;

	coef *= coef;
	coef += (1.0f - coef) * res * 0.5f;

	curr += (sample - curr) * coef;
	last = curr * amp;

	return last;
}

void InsPly::SetAmp(float a)
{
	currpartamp = a;
}

void InsPly::SetFreq(float f)
{
	if (f > 0.0f && f < 20000.0f)
		currpartfreq = f;
}

void InsPly::SetCutOff(float co)
{
	currpartco = co;
}
