#ifndef PLAYER_H
#define PLAYER_H

#include "types.h"
#include "insply.h"
#include "song.h"

class Player
{
public:
	Player(Song *s, uint8 chno);
	~Player();

	void	PAL();
	float	Next();

	void DumpPart(uint8 part);
	void DumpIns(uint8 ins);
	void JumpPos(uint8 seqpos, uint8 steppos, uint8 pal);
private:

	InsPly *ip;
	Song *song;
	ChStep *step;
	Channel *ch;
	
	Step *currstep;	
	Part *currpart;
	uint8 seqcount;		// step in sequencer
	uint8 stepc;		// step in part
	
	uint32 nexts;		// PAL-screen counter	

	float amp;
	float ampstep;	

	float freq;
	float freqstep;
	float targetfreq;	

	float cutoff;
	float cutoffstep;	

};
#endif