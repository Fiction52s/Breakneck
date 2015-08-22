#include "GUI.h"
#include <assert.h>
#include <iostream>

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
			//icons[i][j].setPosition( i * tileSizeX, j * tileSizeY );
			names[i][j] = "not set";
		}
	}

	pos.x = 32;
	pos.y = 32;
	focusX = -1;
	focusY = -1;
}

void GridSelector::Set( int xi, int yi, Sprite s, const std::string &name )
{
	icons[xi][yi] = s;
	icons[xi][yi].setPosition( xi * tileSizeX, yi * tileSizeY );
	names[xi][yi] = name;
}

void GridSelector::Draw( sf::RenderTarget *target )
{
	if( active )
	{
		sf::RectangleShape rs;
		rs.setSize( Vector2f( xSize * tileSizeX, ySize * tileSizeY ) );
		rs.setFillColor( Color::Yellow );
		rs.setPosition( pos.x, pos.y );

		target->draw( rs );

		for( int x = 0; x < xSize; ++x )
		{
			for( int y = 0; y < ySize; ++y )
			{
				Sprite &s = icons[x][y];
				Vector2f realPos = s.getPosition();
				s.setPosition( Vector2f( realPos.x + pos.x, realPos.y + pos.y ) );
				target->draw( s );
				s.setPosition( realPos );
			}
		}
	}
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

		sf::Rect<int> r( pos.x, pos.y, xSize * tileSizeX, ySize * tileSizeY );
		if( r.contains( sf::Vector2i( posx, posy ) ) )
		{
			
			focusX = ( posx - pos.x ) / tileSizeX;
			focusY = ( posy - pos.y ) / tileSizeY;
			cout << "contains index: " << focusX << ", " << focusY << endl;
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
		sf::Rect<int> r( pos.x, pos.y, xSize * tileSizeX, ySize * tileSizeY );
		if( r.contains( sf::Vector2i( posx, posy ) ) )
		{
			int tempX = ( posx - pos.x ) / tileSizeX;
			int tempY = ( posy - pos.y ) / tileSizeY;
			if( tempX == focusX && tempY == focusY )
			{
				cout << "tempX: " << tempX << ", tempY: " << tempY << endl;
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

Panel::Panel( const string &n, int width, int height, GUIHandler *h )
	:handler( h ), size( width, height ), name( n )
	//:t( 0, 0, 200, 10, f, "hello" ), t2( 0, 100, 100, 10, f, "blah" ), b( 0, 50, 100, 50, f, "button!" )
{
	arial.loadFromFile( "arial.ttf" );
}

void Panel::Update( bool mouseDown, int posx, int posy )
{
	//cout << "pos: " << posx << ", " << posy << endl;
	for( std::map<string,TextBox*>::iterator it = textBoxes.begin(); it != textBoxes.end(); ++it )
	{
		//(*it).SendKey( k, shift );
		bool temp = (*it).second->Update( mouseDown, posx, posy );
		if( temp )
		{
			for( std::map<string,TextBox*>::iterator it2 = textBoxes.begin(); it2 != textBoxes.end(); ++it2 )
			{
				if( (*it2).second != (*it).second )
				{
					(*it2).second->focused = false;
				}
			}

			(*it).second->focused = true;
		}
	}

	for( map<string,Button*>::iterator it = buttons.begin(); it != buttons.end(); ++it )
	{
		//(*it).SendKey( k, shift );
		bool temp = (*it).second->Update( mouseDown, posx, posy );

	}

	for( map<string,CheckBox*>::iterator it = checkBoxes.begin(); it != checkBoxes.end(); ++it )
	{
		//(*it).SendKey( k, shift );
		bool temp = (*it).second->Update( mouseDown, posx, posy );
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

void Panel::SendEvent( CheckBox *cb, const std::string & e )
{
	handler->CheckBoxCallback( cb, e );
}

void Panel::AddButton( const string &name, sf::Vector2i pos, sf::Vector2f size, const std::string &text )
{
	assert( buttons.count( name ) == 0 );
	buttons[name] = new Button( name, pos.x, pos.y, size.x, size.y, arial, text, this );
}

void Panel::AddCheckBox( const string &name, sf::Vector2i pos )
{
	assert( checkBoxes.count( name ) == 0 );
	checkBoxes[name] = new CheckBox( name, pos.x, pos.y, this );
}

void Panel::AddTextBox( const std::string &name, sf::Vector2i pos, int width, int lengthLimit, const std::string &initialText )
{
	//Button *b = new Button( pos.x, pos.y, size.x, size.y, arial, handler );
	assert( textBoxes.count( name ) == 0 );
	textBoxes[name] = new TextBox( name, pos.x, pos.y, width, lengthLimit, arial, this, initialText );
	//textBoxes.push_back(  );
}

void Panel::AddLabel( const std::string &name, sf::Vector2i pos, int characterHeight, const std::string &text )
{
	assert( labels.count( name ) == 0 );
	sf::Text *t = new sf::Text( text, arial, characterHeight );
	t->setPosition( pos.x, pos.y );
	t->setColor( Color::Black );

	labels[name] = t;
}

void Panel::Draw( RenderTarget *target )
{
	sf::RectangleShape rs;
	rs.setSize( size );
	rs.setFillColor( Color::Yellow );
	rs.setPosition( pos.x, pos.y );
	target->draw( rs );

	for( map<string,sf::Text*>::iterator it = labels.begin(); it != labels.end(); ++it )
	{
		target->draw( *(*it).second );
	}

	for( map<string,TextBox*>::iterator it = textBoxes.begin(); it != textBoxes.end(); ++it )
	{
		(*it).second->Draw( target );
	}
	
	for( map<string,Button*>::iterator it = buttons.begin(); it != buttons.end(); ++it )
	{
		(*it).second->Draw( target );
	}

	for( map<string,CheckBox*>::iterator it = checkBoxes.begin(); it != checkBoxes.end(); ++it )
	{
		(*it).second->Draw( target );
	}
	
}

void Panel::SendKey( sf::Keyboard::Key k, bool shift )
{
	
	for( map<string,TextBox*>::iterator it = textBoxes.begin(); it != textBoxes.end(); ++it )
	{
		if( (*it).second->focused )
			(*it).second->SendKey( k, shift );
	}
}

TextBox::TextBox( const string &n, int posx, int posy, int width_p, int lengthLimit, sf::Font &f, Panel *p,const std::string & initialText = "")
	:pos( posx, posy ), width( width_p ), maxLength( lengthLimit ), cursorIndex( initialText.length() ), clickedDown( false ), name( n )
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
	rs.setFillColor( Color::White );
	rs.setPosition( pos.x, pos.y );

	target->draw( rs );

	if( focused )
	{
		target->draw( cursor );
	}
	target->draw( text );
}

Button::Button( const string &n, int posx, int posy, int width, int height, sf::Font &f, const std::string & t, Panel *p )
	:pos( posx, posy ), clickedDown( false ), characterHeight( 20 ), size( width, height ), owner( p ), name( n )
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

CheckBox::CheckBox( const std::string &n, int posx, int posy, Panel *own )
	:pos( posx, posy ), clickedDown( false ), owner( own ), name( n ), checked( false )
{

}

bool CheckBox::Update( bool mouseDown, int posx, int posy )
{
	sf::Rect<int> r( pos.x, pos.y, SIZE, SIZE );
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
			checked = !checked;
			if( checked )
			{
				owner->SendEvent( this, "checked" );
			}
			else
			{
				owner->SendEvent( this, "unchecked" );
			}
			
			return true;
		}
		else
		{
			clickedDown = false;
		}
	}

	return false;
}

void CheckBox::Draw( RenderTarget *target )
{
	sf::RectangleShape rs;
	rs.setSize( sf::Vector2f( SIZE, SIZE ) );
	rs.setPosition( pos.x, pos.y );

	if( clickedDown )
	{
		rs.setFillColor( Color::Cyan );
	}
	else
	{
		if( checked )
		{
			rs.setFillColor( Color::Magenta );
		}
		else
		{
			rs.setFillColor( Color::Black );
		}
	}

	target->draw( rs );
}

