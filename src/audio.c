#include "audio.h"
#include <simple_json.h>
#include "simple_logger.h"

extern void fillAudio(void* udata,Uint8* stream,int len);
SDL_AudioSpec wanted;

static Uint8* audioChunk;
static Uint32 audioLen;
static Uint8* audioPos;

int innitalizeAudio()
{
	
	wanted.freq = 22050;
	wanted.format = AUDIO_S16;
	wanted.channels = 2;
	wanted.samples = 1024;
	wanted.callback = fillAudio;
	wanted.userdata = NULL;

	if (SDL_OpenAudio(&wanted, NULL) < 0)
	{
		slog("Failed to innitalize Audio!");
		return -1;
	}
}

void fillAudio(void* udata,Uint8* stream, int len)
{
	if (audioLen == 0)
		return;

	if (len > audioLen)
	{
		len = audioLen;
	}

	SDL_MixAudio(stream, audioPos, len, SDL_MIX_MAXVOLUME);
	audioPos += len;
	audioLen -= len;

	/*
		Load Audio Here?
	*/

	audioPos = audioChunk;

	SDL_PauseAudio(0);

	/*
		Processing?
	*/

	while (audioLen > 0)
	{
		SDL_Delay(100);
	}

}