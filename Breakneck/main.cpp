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
#include "LevelSelector.h"


#define TIMESTEP 1.0 / 60.0

using namespace std;
using namespace sf;

RenderWindow *window;
View v;

#define V2d sf::Vector2<double>
GameController controller(0);

RenderTexture *preScreenTexture;

sf::Texture worldMapTex;
sf::Sprite worldMapSpr;

sf::View uiView( sf::Vector2f( 480, 270 ), sf::Vector2f( 960, 540 ) );

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

		window->setView( v );
		GameSession gs( controller, window, preScreenTexture );
		
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
		window->setView( v );
		GameSession gs( controller, window, preScreenTexture );
		result = gs.Run( filename );
		lastViewCenter = gs.lastViewCenter;
		lastViewSize = gs.lastViewSize;
		if( result > 0 )
			break;

		EditSession es(window );
		result = es.Run( filename, lastViewCenter, lastViewSize );
	}
}

void WorldSelectMenu()
{
	window->setView( uiView );
	bool quit = false;
	int worldSel = 0;
	while( !quit )
	{
		window->clear();
		if( controller.UpdateState() )
		{
			ControllerState cs = controller.GetState();
			if( cs.LDown() )
			{
				worldSel++;
				//cout << "worldSel: " << worldSel << endl;
			}
		}
		window->draw( worldMapSpr );
		window->display();
	}
	window->setView( v );
}

void LoadMenus()
{
	worldMapTex.loadFromFile( "worldmap.png" );
	worldMapSpr.setTexture( worldMapTex );
}

struct TestHandler : GUIHandler 
{
	LevelSelector &ls;
	bool optionChosen;

	TestHandler( LevelSelector &levelSelector )
		:ls( levelSelector ), optionChosen( false )
	{
	}

	
	void ButtonCallback( Button *b, const std::string & e )
	{
		if( ls.text[ls.selectedIndex].getColor() != Color::Red )
		{
			if( b->name == "Play" )
			{
				optionChosen = true;
				GameEditLoop2( ls.localPaths[ls.selectedIndex] );//ls.text[ls.selectedIndex].getString() );
			}
			else if( b->name == "Edit" )
			{
				optionChosen = true;
				GameEditLoop( ls.localPaths[ls.selectedIndex] );//ls.paths[ls.selectedIndex].().string() );//ls.text[ls.selectedIndex].getString() );
			}
			else if( b->name == "Create New" )
			{
				optionChosen = true;
				
				if( ls.text[ls.selectedIndex].getColor() == Color::Red )
				{
					//ls.paths[
					//string dirName= ls.text[ls.selectedIndex].getString();

				}
				//ofstream of;
				//of.open(  + ".brknk" );

				/*
				0
				0 0
				0
				0
				0
				*/

				//GameEditLoop( ls.text[ls.selectedIndex].getString() );
			}
		}
	}

	void TextBoxCallback( TextBox *tb, const std::string & e )
	{
	}

	void GridSelectorCallback( GridSelector *gs, const std::string & e )
	{
	}

	void CheckBoxCallback( CheckBox *cb, const std::string & e )
	{
	}
};

void CustomMapMenu( LevelSelector &ls )
{
	sf::Event ev;
	window->setView( uiView );
	bool quit = false;

	TestHandler customMapHandler( ls );

	Panel p( "custom maps", 960 - ls.width, 540, &customMapHandler );
	p.pos.x += ls.width;
	p.AddButton( "Play", Vector2i( 100, 0 ), Vector2f( 100, 50 ), "PLAY" );
	p.AddButton( "Edit", Vector2i( 100, 100 ), Vector2f( 100, 50 ), "EDIT" );
	p.AddButton( "Create New", Vector2i( 100, 200 ), Vector2f( 150, 50 ), "CREATE NEW" );
	ls.UpdateMapList();


	while( !quit )
	{
		window->clear();
		
		
		Vector2i mousePos = sf::Mouse::getPosition( *window );
		//mousePos /= 2;
		ls.MouseUpdate( mousePos );

		while( window->pollEvent( ev ) )
		{
		//	cout << "some input" << endl;
			switch( ev.type )
			{
			case sf::Event::KeyPressed:
				{
					break;
				}
			case sf::Event::MouseButtonPressed:
				{
					if( ev.mouseButton.button == Mouse::Button::Left )
					{
						p.Update( true, mousePos.x /2, mousePos.y/2 );
						//cout << "blah: " << mousePos.x << ", " << mousePos.y << endl;
						ls.LeftClick( true, mousePos );
					}
					break;
				}
			case sf::Event::MouseButtonReleased:
				{
					if( ev.mouseButton.button == Mouse::Button::Left )
					{
						p.Update( false, mousePos.x/2, mousePos.y/2 );
						
						ls.LeftClick( false, mousePos );
					}
					break;
				}
			}
		}

		if( customMapHandler.optionChosen )
		{
			window->setView( v );
			return;
		}

		p.Draw( window );
		ls.Draw( window );
		window->display();
	}
	window->setView( v );
}

int main()
{
	preScreenTexture = new RenderTexture;
	preScreenTexture->create( 960 * 2, 540 * 2 );
	preScreenTexture->clear();

	sf::Font arial;
	arial.loadFromFile( "arial.ttf" );

	std::cout << "starting program" << endl;
	bool fullWindow = true;

	int windowWidth = 1920;
	int windowHeight = 1080;

	LevelSelector ls( arial );
	ls.windowStretch = Vector2f( windowWidth / 960.0, windowHeight / 540.0 );
	LoadMenus();

	if( sf::Keyboard::isKeyPressed( Keyboard::W ) )
	{
		fullWindow = false;
		windowWidth /= 2;
		windowHeight /= 2;
	}
	if( !fullWindow )
	{
		window = new sf::RenderWindow(/*sf::VideoMode(1400, 900)sf::VideoMode::getDesktopMode()*/
			sf::VideoMode( windowWidth, windowHeight ), "Breakneck", sf::Style::Default, sf::ContextSettings( 0, 0, 0, 0, 0 ));
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
	titleMusic.setVolume( 0 );
	titleMusic.play();

	std::cout << "opened window" << endl;
	sf::Texture t;
	t.loadFromFile( "title.png" );
	
	Sprite titleSprite;
	titleSprite.setTexture( t );
	titleSprite.setOrigin( titleSprite.getLocalBounds().width / 2, titleSprite.getLocalBounds().height / 2 );
	titleSprite.setPosition( 0, 0 );
	titleSprite.setScale( 2, 2 );
	
	
	v.setCenter( 0, 0 );
	v.setSize( 1920/ 2, 1080 / 2 );
	window->setView( v );

	sf::Text menu;
	menu.setString( "\t\tPress any button to start \nFor help and information check README.txt");
	menu.setCharacterSize( 20 );
	menu.setColor( Color::Red );
	
	menu.setFont( arial );
	menu.setOrigin( menu.getLocalBounds().width / 2, menu.getLocalBounds().height / 2 );
	menu.setPosition( 0, 0 );

	sf::Event ev;
	bool quit = false;



	window->setVerticalSyncEnabled( true );

	window->setView( v );
	window->draw( titleSprite );
	window->draw( menu );		

	window->display();

	
	CustomMapMenu( ls );
	//ls.UpdateMapList();
	//ls.Print();
	
	//cout << "beginning input loop" << endl;
	while( !quit )
	{
		window->clear();
		
		//ls.MouseUpdate( sf::Mouse::getPosition( *window ) ); 

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
					else if( ev.key.code == Keyboard::M )
					{
						WorldSelectMenu();
					}
					else
					{
						titleMusic.stop();
						GameEditLoop2( "test3.brknk" );
						window->setView( v );
						titleMusic.play();
					}
					break;
				}
			case sf::Event::MouseButtonPressed:
				{
					if( ev.mouseButton.button == Mouse::Button::Left )
					{
				//		ls.LeftClick( true, sf::Mouse::getPosition( *window ) );
					}
					break;
				}
			case sf::Event::MouseButtonReleased:
				{
					if( ev.mouseButton.button == Mouse::Button::Left )
					{
				//		ls.LeftClick( false, sf::Mouse::getPosition( *window ) );
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
				GameEditLoop2( "test3.brknk" );
				window->setView( v );
			}
		}

		window->setView( v );
		window->draw( titleSprite );
		window->draw( menu );	

		window->setView( uiView );

		

		//window->setView( v

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

