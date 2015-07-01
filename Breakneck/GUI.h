#ifndef __GUI_H__
#define __GUI_H__
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <list>

struct Panel;
struct GUIHandler;


struct GridSelector
{

	GridSelector( int xSize, int ySize, int iconX, int iconY, GUIHandler *handler  );
	void Set( int xi, int yi, sf::Sprite s, const std::string &name );
	void Draw( sf::RenderTarget *target );
	bool Update( bool mouseDown, int posx, int posy );
	int tileSizeX;
	int tileSizeY;
	int xSize;
	int ySize;
	sf::Sprite ** icons;
	std::string ** names;
	sf::RenderTexture control;
	sf::Sprite controlSprite;
	bool active;
	int focusX;
	int focusY;
	GUIHandler *handler;
};



struct TextBox
{
	TextBox( int posx, int posy, int width, int lengthLimit, sf::Font &f, Panel *p, const std::string & initialText);
	void SendKey( sf::Keyboard::Key k, bool shift );
	void Draw( sf::RenderTarget *rt );
	bool Update( bool mouseDown, int posx, int posy );
	sf::Vector2i pos;
	int width;
	int maxLength;
	sf::Text text;
	int cursorIndex;
	sf::Text cursor;
	int characterHeight;
	int verticalBorder;
	int leftBorder;
	bool clickedDown;
	bool focused;
	Panel *owner;
};

struct Button
{
	Button( int posx, int posy, int width, int height, sf::Font &f, const std::string & text, Panel *owner );
	void Draw( sf::RenderTarget *rt );
	bool Update( bool mouseDown, int posx, int posy );
	sf::Vector2i pos;
	sf::Vector2f size;
	sf::Text text;

	int characterHeight;
	bool clickedDown;
	Panel *owner;
};

struct Panel
{
	Panel( int width, int height, GUIHandler *handler );
	void Draw(sf::RenderTarget *rt);
	void Update( bool mouseDown, int posx, int posy );
	void AddButton( sf::Vector2i pos, sf::Vector2f size, const std::string &text );
	void AddTextBox( sf::Vector2i pos, int width, int lengthLimit, const std::string &initialText );
	void SendKey( sf::Keyboard::Key k, bool shift );
	void SendEvent( Button *b, const std::string & e );
	void SendEvent( GridSelector *gs, const std::string & e );
	void SendEvent( TextBox *tb, const std::string & e );
	sf::Font arial;
	//TextBox t;
	//TextBox t2;
	//Button b;
	std::list<TextBox*> textBoxes;
	std::list<Button*> buttons;
	sf::RenderTexture control;
	sf::Sprite controlSprite;

	GUIHandler *handler;
	
	bool active;
};

struct GUIHandler
{
	virtual void ButtonCallback( Button *b, const std::string & e ) = 0;
	virtual void TextBoxCallback( TextBox *tb, const std::string & e ) = 0;
	virtual void GridSelectorCallback( GridSelector *gs, const std::string & e ) = 0;
};

#endif