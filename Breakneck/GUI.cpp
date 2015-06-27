#include "GUI.h"
#include <iostream>

using namespace std;
using namespace sf;

GridSelector::GridSelector( int xSizep, int ySizep, int iconX, int iconY )
	:xSize( xSizep ), ySize( ySizep ), tileSizeX( iconX ), tileSizeY( iconY ), active( false )
{
	icons = new Sprite *[xSize];
	for( int i = 0; i < xSize; ++i )
	{
		icons[i] = new Sprite[ySize];
		for( int j = 0; j < ySize; ++j )
		{
			icons[i][j].setTextureRect( sf::IntRect( 0, 0, tileSizeX, tileSizeY ) );
		}
	}
	control.create( xSize * tileSizeX, ySize * tileSizeY );
	control.clear(sf::Color::White );
	controlSprite.setTexture( control.getTexture() );
	focusX = -1;
	focusY = -1;
	//controlSprite.setOrigin( controlSprite.getLocalBounds().width / 2, controlSprite.getLocalBounds().height / 2 );
//	controlSprite.setPosition( -controlSprite.getLocalBounds().width / 2, -controlSprite.getLocalBounds().height / 2 );
//	controlSprite.move( -200, -200 );
}

void GridSelector::Set( int xi, int yi, Sprite s )
{
	//no idea why its backwards on the Y but oh well.
	//xi = xSize - 1 - xi;
	yi = ySize - 1 - yi;
	icons[xi][yi] = s;
	s.setPosition( xi * tileSizeX, yi * tileSizeY );
	//s.setPosition( 0, 0 );
	control.draw( s );
}

void GridSelector::Draw( sf::RenderTarget *target )
{
	target->draw( controlSprite );
}

//returns true if a selection has been made
bool GridSelector::Update( bool mouseDown, int posx, int posy )
{
	if( mouseDown )
	{
		sf::Rect<int> r( controlSprite.getPosition().x, controlSprite.getPosition().y, controlSprite.getLocalBounds().width, controlSprite.getLocalBounds().height );
		//if( controlSprite.getTextureRect().contains( sf::Vector2i( posx, posy ) ) )
		if( r.contains( sf::Vector2i( posx, posy ) ) )
		{
			//cout << "contains index: " << posx / tileSizeX << ", " << posy / tileSizeY << endl;		
			focusX = posx / tileSizeX;
			focusY = posy / tileSizeY;
		}
		else
		{
		//	cout << "doesn't contain!" << endl;
		//	cout << "pos: " << posx << ", " << posy << endl;
			focusX = -1;
			focusY = -1;
		}
	}
	else
	{
		sf::Rect<int> r( controlSprite.getPosition().x, controlSprite.getPosition().y, controlSprite.getLocalBounds().width, controlSprite.getLocalBounds().height );
		if( r.contains( sf::Vector2i( posx, posy ) ) )
		{
			int tempX = posx / tileSizeX;
			int tempY = posy / tileSizeY;
			if( tempX == focusX && tempY == focusY )
			{
				return true;
		//		cout << "success!" << endl;
			}
			else
			{
				focusX = -1;
				focusY = -1;
			}
			//cout << "contains index: " << posx / tileSizeX << ", " << posy / tileSizeY << endl;		
			
		}
		else
		{
		//	cout << "doesn't contain!" << endl;
		//	cout << "pos: " << posx << ", " << posy << endl;
			focusX = -1;
			focusY = -1;
		}
	}

	return false;
}