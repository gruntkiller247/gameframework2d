#ifndef __TILEDEF_H__
#define __TILEDEF_H__


#include "simple_json.h"
#include "gf2d_sprite.h"

typedef struct
{
	int test;
}TileDef;

/*

*/
TileDef* tiledefParse(SJson* config);


#endif
