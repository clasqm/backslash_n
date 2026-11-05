#ifndef Song___H
#define Song___H

#include "stdlib.h"
#include "string.h"
#include "types.h"

class Player;
class InsPly;

// välj vad som skall kompileras
#define TARGET_EDITOR

const int CHN		= 4;
const int CHNSTEPS	= 256;
const int PARTS		= 256;
const int PARTSTEPS	= 256;
const int INS		= 256;
const int INSSTEPS	= 128;
const int INSPOINTS	= 32;
const int BREAKPOINTS = 256;

struct InsStep
{
	uint8 note;	//8bit note
	uint8 mask; //0001 bit relative | 1111 bit wform
};

struct TimeLev
{
	uint8 time;
	uint8 lev;
};

class Ins
{
public:
	void SetName(char *iname);
	char *name;

	InsStep *steps;

	// Filter - Amp
	TimeLev *amp;
	TimeLev *filter;

	uint8 amppoints;
	uint8 filterpoints;

	uint8 sps;	// PAL-screens per step	
	uint8 res;	// resonance

	uint8 vibs;
	uint8 vibd;
	
	uint8 pwms;
	uint8 pwmd;
	
	uint8 len;	// msb means loop
};


struct Step
{
	uint8 note;
	uint8 ins;
	uint8 eff;		//4bits eff 4bits value
};

class Part
{
public:
	void SetName(char *iname);
	char *name;

	Step *steps;
	uint8 sps;		// PAL-screens per step	
	uint8 len;

};

struct ChStep
{
	uint8 part;
	int8 transp;
};

class Channel
{
public:
	void SetName(char* iname);
	char *name;

	uint8 len;
	ChStep *steps;
};

class BreakPoint
{
public:
	uint32	PAL;
	uint32	command;
};


class Song
{
friend class Player;
friend class InsPly;
public:

	Song( char *filename = 0L);
	virtual ~Song();

	status_t	Load( char *filename);
	status_t	InitCheck();
	void		Play( float *Buffer, uint32 SampleCount );

// hook funktion implemented by derived class to time breakpoints
virtual void	Break(uint32 command);
	
	uint32		GetPos();
	void		SetPos( uint32 PALs );

#ifdef TARGET_EDITOR
	void		Save( char *filename, bool bin=false );

	// // // // // // // // // // CHANNEL stuff
	uint8		ChannelCount();
	Channel		*GetChannel( uint8 c );
	uint8		NewChannel();
	status_t	RemoveChannel( uint8 channel );

	// // // // // // // // // // PART stuff
	uint8		PartCount();
	Part		*GetPart( uint8 part );
	uint8		NewPart( Part *copy_me = 0L );
	status_t	RemovePart( uint8 part );	// B_ERROR if part is used

	// // // // // // // // // // INS stuff
	uint8		InsCount();
	Ins			*GetIns( uint8 i );
	uint8		NewIns( Ins *copy_me =0L );

	// // // // // // // // // // BREAKPOINT stuff
	uint8		BreakPointCount();
	uint8		GetBreakPointIndex( uint32 PAL );
	uint32		GetBreakPointByIndex( uint8 index );
	void		SetBreakPointByIndex( uint8 index, uint32 command );
	uint32		GetBreakPoint( uint32 PAL );
	void		NewBreakPoint( uint32 PAL, uint32 command );
	void		RemoveBreakPoint( uint32 PAL );

	// // // // // // // // // // OPTIONAL stuff
//	status_t	Optimize();		//B_OK if something was removed.
#endif
	
	char	*name;
	float	*N2F;
	float	*R2F;

private:
	
	void	FreeSong();
	void	AllocDefSong(); // only called by constructor
	
	uint16 spspal;

	uint8 channelcount;
	uint8 partcount;
	uint8 instrumentcount;	// anger inte antal instrument, ett för mycket
	uint8 breakpcount;
	
	Channel	*chan;
	Part	*parts;
	Ins		*ins;

	BreakPoint *breakpoints;
	int		bpcount;		// counting the current bp
	status_t init;	

	// player
	uint32 smp_left;		// samples left of the current PAL screen
	uint32 PALs;
	float tmp;
	int pal;
	Player **p;

};

#endif