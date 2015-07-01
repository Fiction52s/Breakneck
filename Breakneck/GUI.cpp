#include "GUI.h"
#include <assert.h>

using namespace sf;
using namespace std;

GridSelector::GridSelector( int xSizep, int ySizep, int iconX, int iconY, GUIHandler *h )
	:xSize( xSizep ), ySize( ySizep ), tileSizeX( iconX ), tileSizeY( iconY ), active( false ),
	handler( h )
{
	icons = new Sprite *[xSize];
	names = new string *[xSize];
	for( int i = 0; i < xSize; ++i )
	{
		icons[i] = new Sprite[ySize];
		names[i] = new string[ySize];
		for( int j = 0; j < ySize; ++j )
		{
			icons[i][j].setTextureRect( sf::IntRect( 0, 0, tileSizeX, tileSizeY ) );
			names[i][j] = "not set";
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

void GridSelector::Set( int xi, int yi, Sprite s, const std::string &name )
{
	//no idea why its backwards on the Y but oh well.
	//xi = xSize - 1 - xi;
	int t = yi;
	yi = ySize - 1 - yi;
	icons[xi][yi] = s;
	names[xi][t] = name;
	s.setPosition( xi * tileSizeX, yi * tileSizeY );
	//s.setPosition( 0, 0 );
	control.draw( s );
}

void GridSelector::Draw( sf::RenderTarget *target )
{
	if( active )
		target->draw( controlSprite );
}

//returns true if a selection has been made
bool GridSelector::Update( bool mouseDown, int posx, int posy )
{
	if( !active )
	{
		assert( false && "trying to update inactive grid selector" );
	}
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
				handler->GridSelectorCallback( this, names[tempX][tempY] );
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

Panel::Panel( int width, int height, GUIHandler *h )
	:handler( h )
	//:t( 0, 0, 200, 10, f, "hello" ), t2( 0, 100, 100, 10, f, "blah" ), b( 0, 50, 100, 50, f, "button!" )
{
	arial.loadFromFile( "arial.ttf" );

	control.create( width, height );
	control.clear(sf::Color::White );
	controlSprite.setTexture( control.getTexture() );
}

void Panel::Update( bool mouseDown, int posx, int posy )
{
	for( std::list<TextBox*>::iterator it = textBoxes.begin(); it != textBoxes.end(); ++it )
	{
		//(*it).SendKey( k, shift );
		bool temp = (*it)->Update( mouseDown, posx, posy );
	}
	/*if( t.Update( mouseDown, posx, posy ) )
	{
		t.focused = true;
		t2.focused = false;
	}

	if( t2.Update( mouseDown, posx, posy ) )
	{
		t.focused = false;
		t2.focused = true;
	}*/

	for( list<Button*>::iterator it = buttons.begin(); it != buttons.end(); ++it )
	{
		//(*it).SendKey( k, shift );
		bool temp = (*it)->Update( mouseDown, posx, posy );

	}

	//if( b.Update( mouseDown, posx, posy ) )
	{
	}
}

void Panel::SendEvent( Button *b, const std::string & e )
{
	handler->ButtonCallback( b, e );
}

void Panel::SendEvent( GridSelector *gs, const std::string & e )
{
	handler->GridSelectorCallback( gs, e );
}

void Panel::SendEvent( TextBox *tb, const std::string & e )
{
	handler->TextBoxCallback( tb, e );
}

void Panel::AddButton( sf::Vector2i pos, sf::Vector2f size, const std::string &text )
{
	//Button *b = new Button( pos.x, pos.y, size.x, size.y, arial, handler );
	buttons.push_back( new Button( pos.x, pos.y, size.x, size.y, arial, text, this ) );
}

void Panel::AddTextBox( sf::Vector2i pos, int width, int lengthLimit, const std::string &initialText )
{
	//Button *b = new Button( pos.x, pos.y, size.x, size.y, arial, handler );
	textBoxes.push_back( new TextBox( pos.x, pos.y, width, lengthLimit, arial, this, initialText ) );
}

void Panel::Draw( RenderTarget *target )
{
	target->draw( controlSprite );
	for( list<TextBox*>::iterator it = textBoxes.begin(); it != textBoxes.end(); ++it )
	{
		(*it)->Draw( target );
	}
	
	for( list<Button*>::iterator it = buttons.begin(); it != buttons.end(); ++it )
	{
		//(*it).SendKey( k, shift );
		//bool temp = (*it)->Update( mouseDown, posx, posy );
		(*it)->Draw( target );

	}
	
}

void Panel::SendKey( sf::Keyboard::Key k, bool shift )
{
	
	for( list<TextBox*>::iterator it = textBoxes.begin(); it != textBoxes.end(); ++it )
	{
		(*it)->SendKey( k, shift );
	}
}

TextBox::TextBox( int posx, int posy, int width_p, int lengthLimit, sf::Font &f, Panel *p,const std::string & initialText = "")
	:pos( posx, posy ), width( width_p ), maxLength( lengthLimit ), cursorIndex( initialText.length() ), clickedDown( false )
{
	focused = false;
	leftBorder = 3;
	verticalBorder = 10;
	characterHeight = 20;
	text.setString( initialText );
	text.setFont( f );
	text.setColor( Color::Black );
	text.setCharacterSize( characterHeight );

	cursor.setString( "|" );
	cursor.setFont( f );
	cursor.setColor( Color::Red );
	cursor.setCharacterSize( characterHeight );
	
	cursor.setPosition( pos.x + text.getLocalBounds().width + leftBorder, pos.y );
	text.setPosition( pos.x + leftBorder, pos.y );
}

void TextBox::SendKey( Keyboard::Key k, bool shift )
{
	char c = 0;
	switch( k )
	{
		case Keyboard::A:
			c = 'a';
			break;
		case Keyboard::B:
			c = 'b';
			break;
		case Keyboard::C:
			c = 'c';
			break;
		case Keyboard::D:
			c = 'd';
			break;
		case Keyboard::E:
			c = 'e';
			break;
		case Keyboard::F:
			c = 'f';
			break;
		case Keyboard::G:
			c = 'g';
			break;
		case Keyboard::H:
			c = 'h';
			break;
		case Keyboard::I:
			c = 'i';
			break;
		case Keyboard::J:
			c = 'j';
			break;
		case Keyboard::K:
			c = 'k';
			break;
		case Keyboard::L:
			c = 'l';
			break;
		case Keyboard::M:
			c = 'm';
			break;
		case Keyboard::N:
			c = 'n';
			break;
		case Keyboard::O:
			c = 'o';
			break;
		case Keyboard::P:
			c = 'p';
			break;
		case Keyboard::Q:
			c = 'q';
			break;
		case Keyboard::R:
			c = 'r';
			break;
		case Keyboard::S:
			c = 's';
			break;
		case Keyboard::T:
			c = 't';
			break;
		case Keyboard::U:
			c = 'u';
			break;
		case Keyboard::V:
			c = 'v';
			break;
		case Keyboard::W:
			c = 'w';
			break;
		case Keyboard::X:
			c = 'x';
			break;
		case Keyboard::Y:
			c = 'y';
			break;
		case Keyboard::Z:
			c = 'z';
			break;
		case Keyboard::Space:
			c = ' ';
			break;
		case Keyboard::Num0:
		case Keyboard::Numpad0:
			c = '0';
			break;
		case Keyboard::Num1:
		case Keyboard::Numpad1:
			c = '1';
			break;
		case Keyboard::Num2:
		case Keyboard::Numpad2:
			c = '2';
			break;
		case Keyboard::Num3:
		case Keyboard::Numpad3:
			c = '3';
			break;
		case Keyboard::Num4:
		case Keyboard::Numpad4:
			c = '4';
			break;
		case Keyboard::Num5:
		case Keyboard::Numpad5:
			c = '5';
			break;
		case Keyboard::Num6:
		case Keyboard::Numpad6:
			c = '6';
			break;
		case Keyboard::Num7:
		case Keyboard::Numpad7:
			c = '7';
			break;
		case Keyboard::Num8:
		case Keyboard::Numpad8:
			c = '8';
			break;
		case Keyboard::Num9:
		case Keyboard::Numpad9:
			c = '9';
			break;
		case Keyboard::Dash:
			c = '-';
			break;
		case Keyboard::Period:
			c = '.';
			break;
		case Keyboard::BackSpace:
			{
			//text.setString( text.getString().substring( 0, cursorIndex ) + text.getString().substring( cursorIndex + 1 ) );
			cursorIndex -= 1;

			if( cursorIndex < 0 )
				cursorIndex = 0;
			else
			{
				sf::String s = text.getString();
				s.erase( cursorIndex );
				text.setString( s );
			}

			break;
			}
		case Keyboard::Left:
			cursorIndex -= 1;
			if( cursorIndex < 0 )
				cursorIndex = 0;
			break;
		case Keyboard::Right:
			if( cursorIndex < text.getString().getSize() )
				cursorIndex += 1;
			break;
		
	}

	if( c != 0 && text.getString().getSize() < maxLength )
	{
		if( shift && c >= 'a' && c <= 'z' )
		{
			c -= 32;
		}
		else if( shift && c == '-' )
		{
			c = '_';
		}
		sf::String s = text.getString();
		
		s.insert( cursorIndex, sf::String( c ) );
		text.setString( s );
		cursorIndex++;
	}

	sf::Text test;
	test = text;
	test.setString( test.getString().substring( 0, cursorIndex) );
	cursor.setPosition( pos.x + test.getLocalBounds().width, pos.y);
}

bool TextBox::Update( bool mouseDown, int posx, int posy )
{
	sf::Rect<int> r( pos.x, pos.y, width, characterHeight + verticalBorder );
	if( mouseDown )
	{	
		if( r.contains( sf::Vector2i( posx, posy ) ) )
		{
			clickedDown = true;
		}
		else
		{
			clickedDown = false;
		}
	}
	else
	{
		if( r.contains( sf::Vector2i( posx, posy ) ) && clickedDown )
		{
			clickedDown = false;
			
			//need to make it so that if you click a letter the cursor goes to the left of it. too lazy for now.

			/*int textLength = text.getString().getSize();

			sf::Text tempText;
			tempText = text;
			tempText.setString( text.getString().substring( 0, 1 ) );

			sf::Rect<int> first( pos.x, pos.y, tempText.getLocalBounds().width / 2 , characterHeight + verticalBorder );
			if( first.contains( sf::Vector2i( posx, posy ) ) )
			{
				cursorIndex = 0;
				cursor.setPosition( pos.x, pos.y);
			}
			
			if( textLength > 1 )
			{
				int startX = 0;
				for( int i = 1; i <= textLength; ++i )
				{
					tempText.setString( text.getString().substring( 0, i );
					 //= tempText.getLocalBounds().left + tempText.getLocalBounds().width;

					//tempText.setString( text.getString().substring( i-1, 2 ) );
					sf::Rect<int> temp( pos.x + startX, pos.y, tempText.getLocalBounds().width / 2 , characterHeight + verticalBorder );
					if( temp.contains( sf::Vector2i( posx, posy ) ) )
					{

					}
				}
			}*/

			//cursor.setPosition( pos.x + text.getLocalBounds().width + leftBorder, pos.y );
			return true;
		}
		else
		{
			clickedDown = false;
		}
	}

	return false;
}

void TextBox::Draw( sf::RenderTarget *target )
{
	sf::RectangleShape rs;
	//rs.setSize( Vector2f( 300, characterHeight + verticalBorder) );
	rs.setSize( Vector2f( width, characterHeight + verticalBorder ) );
	rs.setFillColor( Color::Yellow );
	rs.setPosition( pos.x, pos.y );

	target->draw( rs );

	if( focused )
	{
		target->draw( cursor );
	}
	target->draw( text );
}

Button::Button( int posx, int posy, int width, int height, sf::Font &f, const std::string & t, Panel *p )
	:pos( posx, posy ), clickedDown( false ), characterHeight( 20 ), size( width, height ), owner( p )
{	
	text.setString( t );
	text.setFont( f );
	text.setColor( Color::White );
	text.setCharacterSize( characterHeight );
	text.setPosition( pos.x + width / 2 - text.getLocalBounds().width / 2, pos.y + height / 2 - text.getLocalBounds().height / 2);
}

bool Button::Update( bool mouseDown, int posx, int posy )
{
	sf::Rect<int> r( pos.x, pos.y, size.x, size.y );
	if( mouseDown )
	{	
		if( r.contains( sf::Vector2i( posx, posy ) ) )
		{
			clickedDown = true;
		}
		else
		{
			clickedDown = false;
		}
	}
	else
	{
		if( r.contains( sf::Vector2i( posx, posy ) ) && clickedDown )
		{
			clickedDown = false;
			owner->SendEvent( this, "pressed" );
			return true;
		}
		else
		{
			clickedDown = false;
		}
	}

	return false;
}

void Button::Draw( RenderTarget *target )
{
	sf::RectangleShape rs;
	rs.setSize( size );
	rs.setPosition( pos.x, pos.y );
	if( clickedDown )
		rs.setFillColor( Color::Green );
	else
		rs.setFillColor( Color::Blue );

	target->draw( rs );

	target->draw( text );
}

