/* wave.cpp -- Sawteeth oscillators for the backslash_n demo.
 *
 * Rewritten for Haiku from the Sawteeth engine (MIT licence, copyright 2001
 * Jonas Norberg), keeping the 2000-era Wave interface and its waveform set:
 * HOLD, SAW, SQR, TRI, NOS (noise), plus the octave and detuned variants the
 * demo's instruments use.
 */

#include "wave.h"

#include <math.h>
#include <stdlib.h>

static const float SAMPLE_RATE = 44100.0f;

static float clampf(float val, float low, float high)
{
	if (val < low)
		return low;
	if (val > high)
		return high;
	return val;
}

Wave::Wave(float freq, uint8 waveform)
{
	form = waveform;
	pwm = 0.5f;

	currval = 0.0f;
	currval2 = 0.0f;
	currvaldet = 0.0f;
	curr = 0.0f;

	step = 0.0f;
	step2 = 0.0f;
	stepdet = 0.0f;

	noise = 0x1234567;

	SetFreq(freq);
}

Wave::Wave()
{
	form = SAW;
	pwm = 0.5f;

	currval = 0.0f;
	currval2 = 0.0f;
	currvaldet = 0.0f;
	curr = 0.0f;

	step = 0.0f;
	step2 = 0.0f;
	stepdet = 0.0f;

	noise = 0x1234567;

	SetFreq(440.0f);
}

void Wave::SetFreq(float freq)
{
	if (freq < 0.0f)
		freq = -freq;

	/* phase increment per sample, in cycles */
	step = freq / SAMPLE_RATE;
	if (step > 0.45f)
		step = 0.45f;

	step2 = step * 2.0f;
	stepdet = step * 1.0035f;
}

void Wave::SetPWM(float p)
{
	pwm = clampf(p, 0.05f, 0.95f);
}

void Wave::SetForm(uint8 waveform)
{
	if (waveform < 9)
		form = waveform;
}

/* phase helpers: currval, currval2 and currvaldet hold 0..1 */

static float next_phase(float &phase, float step)
{
	phase += step;
	if (phase >= 1.0f)
		phase -= (float)(int)phase;
	if (phase < 0.0f)
		phase = 0.0f;
	return phase;
}

static float saw(float phase)
{
	return 2.0f * phase - 1.0f;
}

static float square(float phase, float pwm)
{
	return phase < pwm ? 1.0f : -1.0f;
}

static float triangle(float phase)
{
	return 4.0f * fabsf(phase - 0.5f) - 1.0f;
}

float Wave::Next()
{
	float phase = next_phase(currval, step);
	float phase2 = next_phase(currval2, step2);
	float phasedet = next_phase(currvaldet, stepdet);

	if (form == NOS && phase < step) {
		/* a fresh value at every zero crossing */
		noise = noise * 16807 + 11;
	}

	switch (form) {
		case HOLD:
			break;

		case SAW:
			curr = saw(phase);
			break;

		case SQR:
			curr = square(phase, pwm);
			break;

		case TRI:
			curr = triangle(phase);
			break;

		case NOS:
			curr = ((noise >> 9) & 0xffff) / 32768.0f - 1.0f;
			break;

		/* one octave up */
		case SAW2:
			curr = saw(phase2);
			break;

		case SQR2:
			curr = square(phase2, pwm);
			break;

		/* two slightly detuned oscillators */
		case DETSAW:
			curr = (saw(phase) + saw(phasedet)) * 0.5f;
			break;

		case DETSQR:
			curr = (square(phase, pwm) + square(phasedet, pwm)) * 0.5f;
			break;

		default:
			curr = saw(phase);
			break;
	}

	return curr;
}
