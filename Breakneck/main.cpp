#include <iostream>
//#include "PlayerChar.h"
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include <assert.h>
#include <fstream>
#include <list> 
#include <stdlib.h>
#include "EditSession.h"
#include "VectorMath.h"
#include "Input.h"
#include "poly2tri/poly2tri.h"
#include "Physics.h"
#include "Actor.h"
#include "Tileset.h"
#include "GameSession.h"


#define TIMESTEP 1.0 / 60.0

using namespace std;
using namespace sf;

RenderWindow *window;

#define V2d sf::Vector2<double>
GameController controller(0);









void collideShapes( Actor &a, const CollisionBox &b, Actor &a1, const CollisionBox &b1 )
{
	if( b.isCircle && b1.isCircle )
	{
		//circle circle
	}
	else if( b.isCircle )
	{
		//circle rect
	}
	else if( b1.isCircle )
	{
		//circle rect
	}
	else
	{
		//rect rect
	}
}

void GameEditLoop( std::string filename)
{
	int result = 0;

	Vector2f lastViewSize( 0, 0 );
	Vector2f lastViewCenter( 0, 0 );
	while( result == 0 )
	{
		EditSession es(window );
		result = es.Run( filename, lastViewCenter, lastViewSize );
		if( result > 0 )
			break;
		GameSession gs( controller, window );
		result = gs.Run( filename );
		lastViewCenter = gs.lastViewCenter;
		lastViewSize = gs.lastViewSize;
	}

	

}

int main()
{
	bool fullWindow = true ;

	if( !fullWindow )
	{
		window = new sf::RenderWindow(/*sf::VideoMode(1400, 900)sf::VideoMode::getDesktopMode()*/
		sf::VideoMode( 1920 / 2, 1080 / 2), "Breakneck", sf::Style::Default, sf::ContextSettings( 0, 0, 0, 0, 0 ));
		window->setPosition( Vector2i(800, 0 ));
	}
	else
	{
		window = new sf::RenderWindow(/*sf::VideoMode(1400, 900)sf::VideoMode::getDesktopMode()*/
			sf::VideoMode( 1920 / 1, 1080 / 1), "Breakneck", sf::Style::Default, sf::ContextSettings( 0, 0, 0, 0, 0 ));
	}
	
	sf::Texture t;
	t.loadFromFile( "goal.png" );
	
	Sprite titleSprite;
	titleSprite.setTexture( t );
	titleSprite.setPosition( 0, 0 );
	
	View v;
	v.setCenter( 0, 0 );
	v.setSize( 1920/ 2, 1080 / 2 );
	window->setView( v );

	sf::Event ev;
	bool quit = false;

	window->setVerticalSyncEnabled( true );

	while( !quit )
	{
		window->clear();
	
		while( window->pollEvent( ev ) )
		{
			switch( ev.type )
			{
			case Event::KeyPressed:
				{
					if( ev.key.code == Keyboard::Num1 )
					{
						GameEditLoop( "test1" );
						window->setView( v );
					}
					else if( ev.key.code == Keyboard::Num2 )
					{
						GameEditLoop( "test2" );
						window->setView( v );
					}
					else if( ev.key.code == Keyboard::Num3 )
					{
						GameEditLoop( "test3" );
						window->setView( v );
					}
					else if( ev.key.code == Keyboard::Escape )
					{
						quit = true;
					}

					break;
				}
			}
		}

		window->setView( v );
		window->draw( titleSprite );
		window->display();
	}


	sf::Vector2i pos( 0, 0 );

	//window->setPosition( pos );
	
	//window->setFramerateLimit( 60 );
	
	
	View view( Vector2f( 300, 300 ), sf::Vector2f( 960 * 2, 540 * 2 ) );
	window->setView( view );
	
	bool edit = false;

	

	window->close();
	delete window;
}

