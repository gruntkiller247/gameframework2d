#ifndef __WINDOW_H__
#define __WINDOW_H__


#include "gfc_shape.h"
#include "simple_logger.h"
#include "gf2d_sprite.h"

typedef enum
{
	WET_LABLE,
	WET_ACTOR,
	WET_BUTTON,
	WET_CHECKBOX,
	WET_SLIDER,
	WET_CONTAINER,
	WET_MAX
}WindowElementTypes;

typedef struct
{
	GFC_Rect bound;

}WindowElement;

typedef struct
{
	GFC_Rect bound;
	
}Window;


#endif
#pragma once
