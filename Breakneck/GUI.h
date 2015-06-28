#ifndef __GUI_H__
#define __GUI_H__
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>


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



struct TextBox
{
	TextBox( int posx, int posy, int lengthLimit, sf::Font &f, const std::string & initialText);
	void SendKey( sf::Keyboard::Key k, bool shift );
	void Draw( sf::RenderTarget *rt, bool focused );
	sf::Vector2i pos;
	sf::Vector2i size;
	int maxLength;
	sf::Text text;
	int cursorIndex;
	sf::Text cursor;
};

struct Panel
{
	Panel( int width, int height, sf::Font &f);
	void Draw(sf::RenderTarget *rt);
	TextBox t;
	sf::RenderTexture control;
	sf::Sprite controlSprite;
	bool active;
};



#endif