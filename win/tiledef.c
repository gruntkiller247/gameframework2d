#include "simple_logger.h"
#include "tiledef.h"
#include "simple_json.h"
#include <cstddef>

TileDef* tiledefNew()
{
	TileDef* tiledef;
	tiledef = gfc_allocate_array(sizeof(TileDef), 1);

	if (!tiledef)
		return NULL;

	return tiledef;
}

void tiledefFree(TileDef* tiledef)
{
	if (!tiledef)
		return;

	free(tiledef);
}

TileDef* tiledefParse(SJson* config)
{
	;
}