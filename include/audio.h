#ifndef __AUDIO_H__
#define __AUDIO_H__

#include <SDL.h>
#include <stdio.h>
#include "simple_logger.h"

typedef struct Audio_S
{
	int temp;
}Audio;

int innitalizeAudio();

void fillAudio(void* udata, Uint8* stream, int len);


#endif