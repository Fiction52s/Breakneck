//game session

#include "GameSession.h"
#include <fstream>
#include <iostream>
#include <assert.h>
#include "Actor.h"
#include "poly2tri/poly2tri.h"
#include "VectorMath.h"

#define TIMESTEP 1.0 / 60.0

using namespace std;
using namespace sf;

GameSession::GameSession(GameController &c, RenderWindow *rw)
	:controller(c),va(NULL),edges(NULL), window(rw), player( this )
{
}


GameSession::~GameSession()
{
	for( int i = 0; i < numPoints; ++i )
	{
		delete edges[i];
	}
	delete [] edges;

	for( list<VertexArray*>::iterator it = polygons.begin(); it != polygons.end(); ++it )
	{
		delete (*it);
	}

	for( list<Tileset*>::iterator it = tilesetList.begin(); it != tilesetList.end(); ++it )
	{
		delete (*it);
	}
}

Tileset * GameSession::GetTileset( const string & s, int tileWidth, int tileHeight )
{
	for( list<Tileset*>::iterator it = tilesetList.begin(); it != tilesetList.end(); ++it )
	{
		if( (*it)->sourceName == s )
		{
			return (*it);
		}
	}


	//not found


	Tileset *t = new Tileset();
	t->texture = new Texture();
	t->texture->loadFromFile( s );
	t->tileWidth = tileWidth;
	t->tileHeight = tileHeight;
	tilesetList.push_back( t );

	return t;
	//make sure to set up tileset here
}

bool GameSession::OpenFile( string fileName )
{
	currentFile = fileName;

	ifstream is;
	is.open( fileName + ".brknk" );
	if( is.is_open() )
	{
		is >> numPoints;
		points = new Vector2<double>[numPoints];
		

		is >> player.position.x;
		is >> player.position.y;

		int pointsLeft = numPoints;

		int pointCounter = 0;

		edges = new Edge*[numPoints];

		while( pointCounter < numPoints )
		{
			string matStr;
			is >> matStr;
			int polyPoints;
			is >> polyPoints;

			int currentEdgeIndex = pointCounter;
			for( int i = 0; i < polyPoints; ++i )
			{
				int px, py;
				is >> px;
				is >> py;
			
				points[pointCounter].x = px;
				points[pointCounter].y = py;
				++pointCounter;
			}

			for( int i = 0; i < polyPoints; ++i )
			{
				Edge *ee = new Edge();
				edges[currentEdgeIndex + i] = ee;
				ee->v0 = points[i+currentEdgeIndex];
				if( i < polyPoints - 1 )
					ee->v1 = points[i+1 + currentEdgeIndex];
				else
					ee->v1 = points[currentEdgeIndex];
			}

			for( int i = 0; i < polyPoints; ++i )
			{
				Edge * ee = edges[i + currentEdgeIndex];
				if( i == 0 )
				{
					ee->edge0 = edges[currentEdgeIndex + polyPoints - 1];
					ee->edge1 = edges[currentEdgeIndex + 1];
				}
				else if( i == polyPoints - 1 )
				{
					ee->edge0 = edges[currentEdgeIndex + i - 1];
					ee->edge1 = edges[currentEdgeIndex];
				
				}
				else
				{
					ee->edge0 = edges[currentEdgeIndex + i - 1];
					ee->edge1 = edges[currentEdgeIndex + i + 1];
				}
			}

			vector<p2t::Point*> polyline;
			for( int i = 0; i < polyPoints; ++i )
			{
				polyline.push_back( new p2t::Point( points[currentEdgeIndex +i].x, points[currentEdgeIndex +i].y ) );

			}

			p2t::CDT * cdt = new p2t::CDT( polyline );
	
			cdt->Triangulate();
			vector<p2t::Triangle*> tris;
			tris = cdt->GetTriangles();

			va = new VertexArray( sf::Triangles , tris.size() * 3 );
			VertexArray & v = *va;
			Color testColor( 0x75, 0x70, 0x90 );
			for( int i = 0; i < tris.size(); ++i )
			{	
				p2t::Point *p = tris[i]->GetPoint( 0 );	
				p2t::Point *p1 = tris[i]->GetPoint( 1 );	
				p2t::Point *p2 = tris[i]->GetPoint( 2 );	
				v[i*3] = Vertex( Vector2f( p->x, p->y ), testColor );
				v[i*3 + 1] = Vertex( Vector2f( p1->x, p1->y ), testColor );
				v[i*3 + 2] = Vertex( Vector2f( p2->x, p2->y ), testColor );
			}

			polygons.push_back( va );

			delete cdt;
			for( int i = 0; i < polyPoints; ++i )
			{
				delete polyline[i];
			//	delete tris[i];
			}
		}
		is.close();
	}
	else
	{

		//new file
		assert( false && "error getting file to edit " );
	}
}

int GameSession::Run( string fileName )
{
	
	//window->setPosition( pos );
	window->setVerticalSyncEnabled( true );
	//window->setFramerateLimit( 60 );
	window->setMouseCursorVisible( true );

	View view( Vector2f( 300, 300 ), sf::Vector2f( 960 * 2, 540 * 2 ) );
	window->setView( view );

	window->setVerticalSyncEnabled( true );

	
	sf::RectangleShape bDraw;
	bDraw.setFillColor( Color::Red );
	bDraw.setSize( sf::Vector2f(32 * 2, 32 * 2) );
	bDraw.setOrigin( bDraw.getLocalBounds().width /2, bDraw.getLocalBounds().height / 2 );


	OpenFile( fileName );
	
	
	sf::Vertex *line = new sf::Vertex[numPoints*2];
	for( int i = 0; i < numPoints; ++i )
	{
		//cout << "i: " << i << endl;
		line[i*2] = sf::Vertex( Vector2f( edges[i]->v0.x, edges[i]->v0.y  ) );
		line[i*2+1] =  sf::Vertex( Vector2f( edges[i]->v1.x, edges[i]->v1.y ) );
	}	

	sf::Vector2<double> nLine( ( line[1].position - line[0].position).x, (line[1].position - line[0].position).y );
	nLine = normalize( nLine );

	sf::Vector2<double> lineNormal( -nLine.y, nLine.x );

	sf::CircleShape circle( 30 );
	circle.setFillColor( Color::Blue );



	

	sf::Clock gameClock;
	double currentTime = 0;
	double accumulator = TIMESTEP + .1;

	Vector2<double> otherPlayerPos;
	
	double zoomMultiple = 1;

	Color borderColor = sf::Color::Green;
	int max = 1000000;
	sf::Vertex border[] =
	{
		sf::Vertex(sf::Vector2<float>(-max, -max), borderColor ),
		sf::Vertex(sf::Vector2<float>(-max, max), borderColor),
		sf::Vertex(sf::Vector2<float>(-max, max), borderColor),
		sf::Vertex(sf::Vector2<float>(max, max), borderColor),
		sf::Vertex(sf::Vector2<float>(max, max), borderColor),
		sf::Vertex(sf::Vector2<float>(max, -max), borderColor),
		sf::Vertex(sf::Vector2<float>(max, -max), borderColor),
		sf::Vertex(sf::Vector2<float>(-max, -max), borderColor)
	};

	
	bool skipped = false;
	bool oneFrameMode = false;
	bool quit = false;
	bool t = sf::Keyboard::isKeyPressed( sf::Keyboard::T );
	bool s = t;
	t = false;
	
	int returnVal = 0;
	while( !quit )
	{
		double newTime = gameClock.getElapsedTime().asSeconds();
		double frameTime = newTime - currentTime;

		if ( frameTime > 0.25 )
			frameTime = 0.25;	
        currentTime = newTime;

		accumulator += frameTime;

		window->clear();
		while ( accumulator >= TIMESTEP  )
        {
			
			if( oneFrameMode )
				while( true )
				{
					if( sf::Keyboard::isKeyPressed( sf::Keyboard::K ) && !skipped )
					{
						skipped = true;
						accumulator = 0;//TIMESTEP;
						
						//currentTime = gameClock.getElapsedTime().asSeconds() - TIMESTEP;

						break;
					}
					if( !sf::Keyboard::isKeyPressed( sf::Keyboard::K ) && skipped )
					{
						skipped = false;
						//break;
					}
					if( sf::Keyboard::isKeyPressed( sf::Keyboard::L ) )
					{

						//oneFrameMode = false;
						break;
					}
					if( sf::Keyboard::isKeyPressed( sf::Keyboard::M ) )
					{

						oneFrameMode = false;
						break;
					}
				}

		if( sf::Keyboard::isKeyPressed( sf::Keyboard::K ) )
				oneFrameMode = true;
		if( !s && sf::Keyboard::isKeyPressed( sf::Keyboard::T ) )
		{
			quit = true;
			break;
			//t = true;
		}
		else if( s && !sf::Keyboard::isKeyPressed( sf::Keyboard::T ) )
		{
			s = false;

		}


		if( sf::Keyboard::isKeyPressed( sf::Keyboard::Escape ) )
		{
			quit = true;
			returnVal = 1;
			break;
		}



			prevInput = currInput;
			player.prevInput = currInput;
			controller.UpdateState();
			currInput = controller.GetState();
			

			if( currInput.leftStickMagnitude > .4 )
				{
					//cout << "left stick radians: " << currInput.leftStickRadians << endl;
					float x = cos( currInput.leftStickRadians );
					float y = sin( currInput.leftStickRadians );
					float threshold = .4;
					if( x > threshold )
						currInput.pad += 1 << 3;
					if( x < -threshold )
						currInput.pad += 1 << 2;
					if( y > threshold )
						currInput.pad += 1;
					if( y < -threshold )
						currInput.pad += 1 << 1;
				}

				if( currInput.rightStickMagnitude > .4 )
				{
					//cout << "left stick radians: " << currInput.leftStickRadians << endl;
					float x = cos( currInput.rightStickRadians );
					float y = sin( currInput.rightStickRadians );
					float threshold = .4;
					if( x > threshold )
						currInput.altPad += 1 << 3;
					if( x < -threshold )
						currInput.altPad += 1 << 2;
					if( y > threshold )
						currInput.altPad += 1;
					if( y < -threshold )
						currInput.altPad += 1 << 1;
				}
			player.currInput = currInput;
			player.UpdatePrePhysics();

			//Vector2<double> rCenter( r.getPosition().x + r.getLocalBounds().width / 2, r.getPosition().y + r.getLocalBounds().height / 2 );
			
			player.UpdatePhysics( edges, numPoints );

			player.UpdatePostPhysics();

			//r.setPosition( player.position.x - r.getLocalBounds().width / 2, player.position.y - r.getLocalBounds().height / 2 );
			//cout << "playerPos: " << player.position.x << ", " << player.position.y << endl;	
		

			accumulator -= TIMESTEP;
			while( sf::Keyboard::isKeyPressed( sf::Keyboard::Space ) )
		{
		}
		}


		sf::Event ev;
		while( window->pollEvent( ev ) )
		{
			if( ev.type == Event::MouseWheelMoved )
			{
				if( ev.mouseWheel.delta > 0 )
				{
					zoomMultiple /= 2;
				}
				else if( ev.mouseWheel.delta < 0 )
				{
					zoomMultiple *= 2;
				}
				
				if( zoomMultiple < 1 )
				{
					zoomMultiple = 1;
				}
				else if( zoomMultiple > 65536 )
				{
					zoomMultiple = 65536;
				}
			}
		}
		view.setSize( Vector2f( 960 * zoomMultiple, 540 * zoomMultiple ) );
		lastViewSize = view.getSize();
		
		view.setCenter( player.position.x, player.position.y );
		lastViewCenter = view.getCenter();
		window->setView( view );

		
		bDraw.setSize( sf::Vector2f(player.b.rw * 2, player.b.rh * 2) );
		bDraw.setOrigin( bDraw.getLocalBounds().width /2, bDraw.getLocalBounds().height / 2 );
		bDraw.setPosition( player.position.x, player.position.y );
	//	bDraw.setRotation( player.sprite->getRotation() );
		window->draw( bDraw );

		window->draw( *(player.sprite) );//, &player.sh );
		sf::RectangleShape rs;
		rs.setSize( Vector2f(64, 64) );
		rs.setOrigin( rs.getLocalBounds().width / 2, rs.getLocalBounds().height / 2 );
		rs.setPosition( otherPlayerPos.x, otherPlayerPos.y  );
		rs.setFillColor( Color::Blue );
		//window->draw( circle );
		//window->draw(line, numPoints * 2, sf::Lines);
		for( list<VertexArray*>::iterator it = polygons.begin(); it != polygons.end(); ++it )
		{
			window->draw( *(*it ) );
		}
		//window->draw(border, 8, sf::Lines);

		window->display();

		
	}

	delete [] line;
	return returnVal;
}