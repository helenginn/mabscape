#include "Square.h"
#include "Glowable_sh.h"

Square::Square() : SlipObject()
{
	makeQuad();
	_vString = Glowable_vsh();
	_fString = Glowable_fsh();

}

void Square::makeQuad()
{
	addVertex(-1, -1, 0);
	addVertex(-1, +1, 0);
	addVertex(+1, -1, 0);
	addVertex(+1, +1, 0);

	addIndices(0, 1, 2);
	addIndices(1, 2, 3);
}
