#ifndef WAVE_H
#define WAVE_H

#include "types.h"

enum {	HOLD = 0,
		SAW,
		SQR,
		TRI,
		NOS,
		
		// octave (detuned)
		SAW2,
		SQR2,
		
		// detuned
		DETSAW,
		DETSQR
};


class Wave
{
public:
	Wave(float freq, uint8 waveform);
	Wave();

	float Next();

	void SetFreq(float freq);
	void SetPWM(float p);
	void SetForm(uint8 waveform);

private:

	uint8 form;

	float pwm;
	float step;
	float step2;
	float stepdet;
	float currval;
	float currval2;
	float currvaldet;

	float curr;

	uint32 noise;	// noise generator state (NOS)
};

#endif
