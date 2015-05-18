#include "Tileset.h"	

using namespace std;
using namespace sf;

IntRect Tileset::GetSubRect( int localID )
{
	int xi,yi;
	xi = localID % (texture->getSize().x / tileWidth );
	yi = localID / (texture->getSize().x / tileWidth );
		

	return IntRect( xi * tileWidth, yi * tileHeight, tileWidth, tileHeight ); 
}
