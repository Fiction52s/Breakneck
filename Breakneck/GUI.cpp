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

Panel::Panel( int width, int height, sf::Font &f)
	:t( 0, 0, 10, f, "hello" )
{
	control.create( width, height );
	control.clear(sf::Color::White );
	controlSprite.setTexture( control.getTexture() );
}

void Panel::Draw( RenderTarget *target )
{
	//control.clear(sf::Color::White );
	target->draw( controlSprite );
	t.Draw( target, true );
}

TextBox::TextBox( int posx, int posy, int lengthLimit, sf::Font &f, const std::string & initialText = "")
	:pos( posx, posy ), maxLength( lengthLimit ), cursorIndex( initialText.length() )
{

	text.setString( initialText );
	text.setFont( f );
	text.setColor( Color::Black );
	text.setCharacterSize( 50 );

	cursor.setString( "|" );
	cursor.setFont( f );
	cursor.setColor( Color::Red );
	cursor.setCharacterSize( 50 );
	
	/*sf::Text test;
	test = text;
	test.setString( test.getString().substring( 0, cursorIndex) );*/
	
	cursor.setPosition( text.getLocalBounds().width, 0 );
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
			cursorIndex += 1;
			break;
		
	}

	if( c != 0 )
	{
		if( shift && c >= 'a' && c <= 'z' )
		{
			c -= 32;
		}
		sf::String s = text.getString();
		
		s.insert( cursorIndex, sf::String( c ) );
		text.setString( s );
		cursorIndex++;
	}

	sf::Text test;
	test = text;
	test.setString( test.getString().substring( 0, cursorIndex) );
	
	sf::FloatRect r;
	
	cursor.setPosition( test.getLocalBounds().width, 0 );


}

void TextBox::Draw( sf::RenderTarget *target, bool focused )
{

	target->draw( cursor );
	target->draw( text );
}