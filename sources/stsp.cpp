#include <stdio.h>
#include <ByteOrder.h>
#include <SoundPlayer.h>
#include "stsp.h"

void stsp::BufferProc(void *player, void *buffer, size_t size, const media_raw_audio_format &format)
{ 
// only work for mono floating-point audio 
	if (format.channel_count != 1 || 
		format.format != media_raw_audio_format::B_AUDIO_FLOAT) { 
		puts("sorry, I only do mono-float");
		return; 
	} 
	((Song*)player)->Play((float*)buffer,size >> 2);	
}

stsp::stsp(char *filename):Song(filename)
{

	// mono-float, at the host byte order
	media_raw_audio_format af = {	44100,
									1,
									media_raw_audio_format::B_AUDIO_FLOAT,
									B_HOST_IS_LENDIAN ? B_MEDIA_LITTLE_ENDIAN : B_MEDIA_BIG_ENDIAN,
									1024 };
	player = new BSoundPlayer(&af, "SawTeeth", BufferProc, NULL, this); 
}

stsp::~stsp()
{
	delete player;
}

void stsp::Start()
{
	player->Start(); 
	player->SetHasData(true); 
}

void stsp::Stop()
{
	player->Stop();
}
