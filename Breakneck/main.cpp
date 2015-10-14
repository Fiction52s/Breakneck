#include <iostream>
//#include "PlayerChar.h"
#include <sstream>
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
#include <boost/filesystem.hpp>


#define TIMESTEP 1.0 / 60.0

using namespace std;
using namespace sf;

RenderWindow *window;
View v;
using namespace boost::filesystem;

#define V2d sf::Vector2<double>
GameController controller(0);

RenderTexture *preScreenTexture;
RenderTexture *minimapTexture;

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
		GameSession gs( controller, window, preScreenTexture, minimapTexture );
		
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
		GameSession gs( controller, window, preScreenTexture, minimapTexture );
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

struct CustomMapsHandler : GUIHandler 
{
	LevelSelector &ls;
	bool optionChosen;
	bool showNamePopup;

	CustomMapsHandler( LevelSelector &levelSelector )
		:ls( levelSelector ), optionChosen( false ), showNamePopup( false )
	{
	}

	
	void ButtonCallback( Button *b, const std::string & e )
	{
		if( ls.text[ls.selectedIndex].getColor() == Color::Blue )
		{
			if( b->name == "Play" )
			{
				optionChosen = true;
				GameSession gs( controller, window, preScreenTexture, minimapTexture );
				gs.Run( ls.localPaths[ls.selectedIndex] );
				window->setView( uiView );
			}
			else if( b->name == "Edit" )
			{
				optionChosen = true;
				GameEditLoop( ls.localPaths[ls.selectedIndex] );//ls.paths[ls.selectedIndex].().string() );//ls.text[ls.selectedIndex].getString() );
				window->setView( uiView );
			}
			else if( b->name == "Delete" )
			{	
				boost::filesystem::remove(ls.localPaths[ls.selectedIndex]);
				ls.UpdateMapList();
			}
		}
		else
		{
			
		}

		if( b->name == "Create New" )
		{			
			showNamePopup = true;
			optionChosen = true;	
			//cout << "what" << endl;
			//cout << b->owner->name << ", " << b->owner->textBoxes.size() << ", " << b->owner->textBoxes.count( "name" ) << endl;
				
		}
		else if( b->name == "ok" )
		{
			showNamePopup = false;
			ls.newLevelName = b->owner->textBoxes["name"]->text.getString().toAnsiString();
				
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

void CustomMapsOption( LevelSelector &ls )
{
	sf::Event ev;
	window->setView( uiView );
	bool quit = false;

	CustomMapsHandler customMapHandler( ls );

	Panel p( "custom maps", 960 - ls.width, 540, &customMapHandler );
	p.pos.x += ls.width;
	p.AddButton( "Play", Vector2i( 100, 0 ), Vector2f( 100, 50 ), "PLAY" );
	p.AddButton( "Edit", Vector2i( 100, 100 ), Vector2f( 100, 50 ), "EDIT" );
	p.AddButton( "Create New", Vector2i( 100, 200 ), Vector2f( 175, 50 ), "CREATE NEW" );
	p.AddButton( "Delete", Vector2i( 100, 300 ), Vector2f( 150, 50 ), "DELETE" );

	Panel namePopup( "name popup", 300, 200, &customMapHandler );
	namePopup.pos = Vector2i( 960 / 2, 540 / 2 );
	namePopup.AddButton( "ok", Vector2i( 100, 0 ), Vector2f( 100, 50 ), "OK" );	
	namePopup.AddTextBox( "name", Vector2i( 10, 10 ), 100, 40, "test" );
	//bool showNamePopup = false;

	ls.UpdateMapList();

	string empty = "5\n0 0\nmat\n5\n-209 78\n286 78\n286 132\n60 132\n-201 132\n0\n0\n0\n1\n-- 1\ngoal -air 0 0 76";

	while( !quit )
	{
		window->clear();
		
		
		Vector2i mousePos = sf::Mouse::getPosition( *window );
		//mousePos /= 2;
		ls.MouseUpdate( mousePos );

		//if( !customMapHandler.showNamePopup )
		
		while( window->pollEvent( ev ) )
		{
		//	cout << "some input" << endl;
			switch( ev.type )
			{
			case sf::Event::KeyPressed:
				{
					if( ev.key.code == Keyboard::Escape )
					{
						window->setView( v );
						return;
					}
					else
					{
						if( customMapHandler.showNamePopup )
						{
							//ls.newLevelName = "";
							ls.newLevelName = "";
							namePopup.SendKey( ev.key.code, ev.key.shift );
							if( ls.newLevelName != "" )
							{
								
								if( ls.text[ls.selectedIndex].getColor() == Color::Red )
								{
									path from( "Maps/empty.brknk" );
									string toString = ls.localPaths[ls.selectedIndex] + ls.newLevelName + ".brknk";
									path to( toString );

									try 
									{
										boost::filesystem::copy_file( from, to, copy_option::fail_if_exists );

										GameEditLoop( ls.localPaths[ls.selectedIndex] + ls.newLevelName + ".brknk" );

										ls.UpdateMapList();

										window->setView( uiView );
									}
									catch (const boost::system::system_error &err) 
									{
										cout << "file already exists!" << endl;
									}
								}
								else
								{
									path from( ls.localPaths[ls.selectedIndex] );
									TreeNode * toNode = ls.dirNode[ls.selectedIndex];
									path to( toNode->GetLocalPath() + ls.newLevelName + ".brknk" );
									
									try 
									{
										boost::filesystem::copy_file( from, to, copy_option::fail_if_exists );

										//cout << "copying to: " << to.string() << endl;
										GameEditLoop( to.string() );

										ls.UpdateMapList();

										window->setView( uiView );
									}
									catch (const boost::system::system_error &err) 
									{
										cout << "file already exists!" << endl;
									}
								}
								
								
							}
						}
						
					}
					break;
				}
			case sf::Event::MouseButtonPressed:
				{
					if( ev.mouseButton.button == Mouse::Button::Left )
					{
						if( customMapHandler.showNamePopup )
						{
							
							namePopup.Update( true, mousePos.x /2, mousePos.y/2 );
						}
						else
						{
							p.Update( true, mousePos.x /2, mousePos.y/2 );
							//cout << "blah: " << mousePos.x << ", " << mousePos.y << endl;
							ls.LeftClick( true, mousePos );
						}
					}
					break;
				}
			case sf::Event::MouseButtonReleased:
				{
					if( ev.mouseButton.button == Mouse::Button::Left )
					{
						if( customMapHandler.showNamePopup )
						{
							ls.newLevelName = "";
							namePopup.Update( false, mousePos.x /2, mousePos.y/2 );
							if( ls.newLevelName != "" )
							{
								
								if( ls.text[ls.selectedIndex].getColor() == Color::Blue )
								{
									path from( "Maps/empty.brknk" );
									string toString = ls.localPaths[ls.selectedIndex] + ls.newLevelName + ".brknk";
									path to( toString );

									try 
									{
										boost::filesystem::copy_file( from, to, copy_option::fail_if_exists );

										GameEditLoop( ls.localPaths[ls.selectedIndex] + ls.newLevelName + ".brknk" );

										ls.UpdateMapList();

										window->setView( uiView );
									}
									catch (const boost::system::system_error &err) 
									{
										cout << "file already exists!" << endl;
									}
								}
								else
								{
									path from( ls.localPaths[ls.selectedIndex] );
									TreeNode * toNode = ls.dirNode[ls.selectedIndex];
									path to( toNode->GetLocalPath() + ls.newLevelName + ".brknk" );

									try 
									{
										boost::filesystem::copy_file( from, to, copy_option::fail_if_exists );

										GameEditLoop( to.string() );

										ls.UpdateMapList();

										window->setView( uiView );
									}
									catch (const boost::system::system_error &err) 
									{
										cout << "file already exists!" << endl;
									}
								}
								
								
							}
						}
						else
						{
							p.Update( false, mousePos.x/2, mousePos.y/2 );
							ls.LeftClick( false, mousePos );
							if( ls.text[ls.selectedIndex].getColor() == Color::Red )
							{
								p.buttons["Create New"]->text.setString( "CREATE NEW" );
							}
							else
							{
								p.buttons["Create New"]->text.setString( "CREATE COPY" );
							}
						}
					}
					break;
				}
			}
		}

		p.Draw( window );
		ls.Draw( window );

		if( customMapHandler.showNamePopup )
		{
			namePopup.Draw( window );
		}

		window->display();
		
		
	}
	window->setView( v );
}

void NewCampaignOption()
{
	GameSession gs( controller, window, preScreenTexture, minimapTexture );
	gs.Run( "test3.brknk" );
}

void LoadCampaignOption()
{
	cout << "loading!" << endl;
}

void OptionsOption()
{
	cout << "options!" << endl;
}

void ExitOption()
{
	cout << "exit!" << endl;
}

int main()
{
	preScreenTexture = new RenderTexture;
	preScreenTexture->create( 960 * 2, 540 * 2 );
	preScreenTexture->clear();

	minimapTexture = new RenderTexture;
	minimapTexture->create( 300, 300 );
	minimapTexture->clear();

	sf::Font arial;
	arial.loadFromFile( "arial.ttf" );

	std::cout << "starting program" << endl;
	bool fullWindow = true;

	int windowWidth = 1920;
	int windowHeight = 1080;

	LevelSelector ls( arial );
	ls.windowStretch = Vector2f( windowWidth / 960.0, windowHeight / 540.0 );
	LoadMenus();

	int currentMenuSelect = 0;

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
		window = new sf::RenderWindow( i.front(), "Breakneck", sf::Style::None );
			//sf::VideoMode( 1920 / 1, 1080 / 1), "Breakneck", sf::Style::Fullscreen, sf::ContextSettings( 0, 0, 0, 0, 0 ));
	}
	window->setVerticalSyncEnabled( true );
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

	Text mainMenu[5];

	int h = 50;
	int index = 0;
	Text &newText = mainMenu[index];
	newText.setString( "New Campaign" );
	newText.setFont( arial );
	newText.setColor( Color::Red );
	newText.setCharacterSize( 16 );
	newText.setPosition( 100, 100 + 50 * index );
	
	++index;

	Text &loadText = mainMenu[index];
	loadText.setString( "Load Campaign" );
	loadText.setFont( arial );
	loadText.setColor( Color::Red );
	loadText.setCharacterSize( 16 );
	loadText.setPosition( 100, 100 + 50 * index );

	++index; 

	Text &customText = mainMenu[index];
	customText.setString( "Custom Maps" );
	customText.setFont( arial );
	customText.setColor( Color::Red );
	customText.setCharacterSize( 16 );
	customText.setPosition( 100, 100 + 50 * index );

	++index;

	Text &optionsText = mainMenu[index];
	optionsText.setString( "Options" );
	optionsText.setFont( arial );
	optionsText.setColor( Color::Red );
	optionsText.setCharacterSize( 16 );
	optionsText.setPosition( 100, 100 + 50 * index );

	++index;

	Text &exitText = mainMenu[index];
	exitText.setString( "Exit" );
	exitText.setFont( arial );
	exitText.setColor( Color::Red );
	exitText.setCharacterSize( 16 );
	exitText.setPosition( 100, 100 + 50 * index );

	
	
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
					else if( ev.key.code == Keyboard::Return || ev.key.code == Keyboard::Space )
					{
						//select from option menu
						switch( currentMenuSelect )
						{
						case 0:
							NewCampaignOption();
							break;
						case 1:
							LoadCampaignOption();
							break;
						case 2:
							CustomMapsOption( ls );
							break;
						case 3:
							OptionsOption();
							break;
						case 4:
							ExitOption();
							break;
						}
					}
					else if( ev.key.code == Keyboard::Up )
					{
						currentMenuSelect--;
						if( currentMenuSelect < 0 )
							currentMenuSelect = 4;
					}
					else if( ev.key.code == Keyboard::Down )
					{
						currentMenuSelect++;
						if( currentMenuSelect > 4 )
							currentMenuSelect = 0;
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
			
		for( int i = 0; i < 5; ++i )
		{
			if( i == currentMenuSelect )
			{
				mainMenu[i].setColor( Color::White );
			}
			else
			{
				mainMenu[i].setColor( Color::Red );
			}
			//cout << "drawing i: " << i <<  endl;
			window->draw( mainMenu[i] );
		}

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

