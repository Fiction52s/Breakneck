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

void GameEditLoop2( std::string filename)
{
	int result = 0;

	Vector2f lastViewSize( 0, 0 );
	Vector2f lastViewCenter( 0, 0 );
	while( result == 0 )
	{
		GameSession gs( controller, window );
		result = gs.Run( filename );
		lastViewCenter = gs.lastViewCenter;
		lastViewSize = gs.lastViewSize;
		if( result > 0 )
			break;

		EditSession es(window );
		result = es.Run( filename, lastViewCenter, lastViewSize );
	}
}

int main()
{
	cout << "starting program" << endl;
	bool fullWindow = true;

	if( sf::Keyboard::isKeyPressed( Keyboard::W ) )
		fullWindow = false;
	if( !fullWindow )
	{
		window = new sf::RenderWindow(/*sf::VideoMode(1400, 900)sf::VideoMode::getDesktopMode()*/
			sf::VideoMode( 1920 / 2, 1080 / 2), "Breakneck", sf::Style::Default, sf::ContextSettings( 0, 0, 0, 0, 0 ));
		window->setPosition( Vector2i(800, 0 ));
	}
	else
	{
		std::vector<sf::VideoMode> i = sf::VideoMode::getFullscreenModes();
        //sf::RenderWindow window(i.front(), "SFML WORKS!", sf::Style::Fullscreen);
		//window = new sf::RenderWindow(/*sf::VideoMode(1400, 900)sf::VideoMode::getDesktopMode()*/
		//	sf::VideoMode( 1920 / 1, 1079 / 1), "Breakneck", sf::Style::Fullscreen, sf::ContextSettings( 0, 0, 0, 0, 0 ));
		window = new sf::RenderWindow( i.front(), "Breakneck", sf::Style::Default );
			//sf::VideoMode( 1920 / 1, 1080 / 1), "Breakneck", sf::Style::Fullscreen, sf::ContextSettings( 0, 0, 0, 0, 0 ));
	}

	//test.clear();

	sf::Music titleMusic;
	if( !titleMusic.openFromFile( "titletheme.ogg" ))
		assert( false && "no music found" );

	titleMusic.setLoop( true );
	titleMusic.play();

	cout << "opened window" << endl;
	sf::Texture t;
	t.loadFromFile( "title.png" );
	
	Sprite titleSprite;
	titleSprite.setTexture( t );
	titleSprite.setOrigin( titleSprite.getLocalBounds().width / 2, titleSprite.getLocalBounds().height / 2 );
	titleSprite.setPosition( 0, 0 );
	titleSprite.setScale( 2, 2 );
	
	View v;
	v.setCenter( 0, 0 );
	v.setSize( 1920/ 2, 1080 / 2 );
	window->setView( v );

	sf::Text menu;
	menu.setString( "\t\tPress any button to start \nFor help and information check README.txt");
	menu.setCharacterSize( 20 );
	menu.setColor( Color::Red );
	sf::Font arial;
	arial.loadFromFile( "arial.ttf" );
	menu.setFont( arial );
	menu.setOrigin( menu.getLocalBounds().width / 2, menu.getLocalBounds().height / 2 );
	menu.setPosition( 0, 200 );

	sf::Event ev;
	bool quit = false;

	window->setVerticalSyncEnabled( true );

	window->setView( v );
	window->draw( titleSprite );
	window->draw( menu );		

	window->display();

	//cout << "beginning input loop" << endl;
	while( !quit )
	{
		window->clear();
		


		while( window->pollEvent( ev ) )
		{
		//	cout << "some input" << endl;
			switch( ev.type )
			{
			case sf::Event::KeyPressed:
				{
					/*if( ev.key.code == Keyboard::Num1 )
					{
						cout << "starting level 1" << endl;
						titleMusic.stop();
						GameEditLoop2( "test1" );
						window->setView( v );
						titleMusic.play();
					}
					else if( ev.key.code == Keyboard::Num2 )
					{
						cout << "starting level 2" << endl;
						titleMusic.stop();
						GameEditLoop2( "test2" );
						window->setView( v );
						titleMusic.play();
					}
					else if( ev.key.code == Keyboard::Num3 )
					{
						cout << "starting level 3" << endl;
						titleMusic.stop();

						GameEditLoop2( "test3" );
						window->setView( v );
						titleMusic.play();
					}*/
					if( ev.key.code == Keyboard::Escape )
					{
						quit = true;
					}
					else
					{
						titleMusic.stop();
						GameEditLoop2( "test3" );
						window->setView( v );
						titleMusic.play();
					}

					break;
				}
			}
		}

		if( controller.UpdateState() )
		{
			ControllerState cs = controller.GetState();
			if( cs.A || cs.back || cs.Y || cs.X || cs.rightShoulder || cs.leftShoulder )
			{
				GameEditLoop2( "test3" );
				window->setView( v );
			}
		}

		window->setView( v );
		window->draw( titleSprite );
		window->draw( menu );		

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

