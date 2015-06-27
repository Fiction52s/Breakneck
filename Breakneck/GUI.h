#ifndef __GUI_H__
#define __GUI_H__
#include <SFML/Graphics.hpp>


struct GridSelector
{

	GridSelector( int xSize, int ySize, int iconX, int iconY );
	void Set( int xi, int yi, sf::Sprite s ); 
	void Draw( sf::RenderTarget *target );
	bool Update( bool mouseDown, int posx, int posy );
	int tileSizeX;
	int tileSizeY;
	int xSize;
	int ySize;
	sf::Sprite ** icons;
	sf::RenderTexture control;
	sf::Sprite controlSprite;
	bool active;
	int focusX;
	int focusY;
};


#endif