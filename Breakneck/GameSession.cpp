//game session

#include "GameSession.h"
#include <fstream>
#include <iostream>
#include <assert.h>
#include "Actor.h"
#include "poly2tri/poly2tri.h"
#include "VectorMath.h"
#include "Camera.h"

#define TIMESTEP 1.0 / 60.0
#define V2d sf::Vector2<double>

using namespace std;
using namespace sf;

GameSession::GameSession( GameController &c, RenderWindow *rw)
	:controller(c),va(NULL),edges(NULL), window(rw), player( this ), activeEnemyList( NULL )
{
	if (!polyShader.loadFromFile("mat_shader.frag", sf::Shader::Fragment))
	//if (!sh.loadFromMemory(fragmentShader, sf::Shader::Fragment))
	{
		cout << "PLAYER SHADER NOT LOADING CORRECTLY" << endl;
		assert( 0 && "polygon shader not loaded" );
	}

	if( !goalTex.loadFromFile( "goal.png" ) )
	{
		assert( 0 && "goal couldnt load" );
	}
	goalSprite.setTexture( goalTex );
	goalSprite.setOrigin( goalSprite.getLocalBounds().width / 2, goalSprite.getLocalBounds().height / 2 );

	testTree = new EdgeLeafNode( V2d( 0, 0), 1000000, 1000000);
	testTree->parent = NULL;
	testTree->debug = rw;

	enemyTree = new EnemyLeafNode( V2d( 0, 0), 1000000, 1000000);
	enemyTree->parent = NULL;
	enemyTree->debug = rw;
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
	t->sourceName = s;
	tilesetList.push_back( t );

	return t;
	//make sure to set up tileset here
}

void GameSession::UpdateEnemiesPrePhysics()
{
	Enemy *current = activeEnemyList;
	while( current != NULL )
	{
		current->UpdatePrePhysics();
		current = current->next;
	}
}

void GameSession::UpdateEnemiesPhysics()
{
	Enemy *current = activeEnemyList;
	while( current != NULL )
	{
		current->UpdatePhysics();
		current = current->next;
	}
}

void GameSession::UpdateEnemiesPostPhysics()
{
	Enemy *current = activeEnemyList;
	while( current != NULL )
	{
		current->UpdatePostPhysics();
		current = current->next;
	}
}

void GameSession::UpdateEnemiesDraw()
{
	Enemy *current = activeEnemyList;
	while( current != NULL )
	{
		current->Draw( window );
		current = current->next;
	}
}

void GameSession::UpdateEnemiesSprites()
{
	Enemy *current = activeEnemyList;
	while( current != NULL )
	{
	//	current->up();
		current = current->next;
	}
}

void GameSession::Test( Edge *e )
{
	cout << "testing" << endl;
}

void GameSession::AddEnemy( Enemy *e )
{
	if( activeEnemyList != NULL )
	{
		activeEnemyList->prev = e;
		e->next = activeEnemyList;
		activeEnemyList = e;
	}
	else
	{
		activeEnemyList = e;
	}
	
}

void GameSession::RemoveEnemy( Enemy *e )
{
	Enemy *prev = e->prev;
	Enemy *next = e->next;

	if( prev != NULL )
	{
		prev->next = next;
	}

	if( next != NULL )
	{
		next->prev = prev;
	}

	/*if( inactiveEnemyList != NULL )
	{
		inactiveEnemyList->next = e;
	}
	else
	{
		inactiveEnemyList = e;
	}*/
	
	
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
		originalPos.x = player.position.x;
		originalPos.y = player.position.y;

		int goalPositionx, goalPositiony;
		is >> goalPositionx;
		is >> goalPositiony;
		goalSprite.setPosition( goalPositionx, goalPositiony );

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

			//	cout << "here we go " << i << " out of " << polyPoints << " "  << ee->Normal().x << ", " << ee->Normal().y << endl;
				testTree = Insert(testTree, ee );

				//if( testTree->debug != NULL )
				if( false )
				{
					DebugDrawQuadTree( testTree->debug, testTree );
					testTree->debug->display();
				}
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


		
		int numGroups;
		is >> numGroups;
		for( int i = 0; i < numGroups; ++i )
		{
			string gName;
			is >> gName;
			int numActors;
			is >> numActors;

			for( int j = 0; j < numActors; ++j )
			{
				string typeName;
				is >> typeName;

				if( typeName == "patroller" )
				{
					Patroller *enemy = new Patroller( this );
					int xPos,yPos;
					is >> xPos;
					is >> yPos;
					string blah;
					is >> blah;
					float speed;
					is >> speed;
					enemy->position = V2d( xPos, yPos );
					//AddEnemy( enemy );
					enemy->spawnRect = sf::Rect<double>( xPos - 16, yPos - 16, xPos + 16, yPos + 16 );
					enemyTree = Insert( enemyTree, enemy );

					//enemy->maxSpeed = spee
				}
				else
				{
					assert( false && "not a valid type name" );
				}

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
	sf::Texture backTex;
	backTex.loadFromFile( "bg01.png" );
	sf::Sprite background( backTex );
	background.setOrigin( background.getLocalBounds().width / 2, background.getLocalBounds().height / 2 );
	background.setPosition( 0, 0 );
	sf::View bgView( sf::Vector2f( 0, 0 ), sf::Vector2f( 960, 540 ) );

	sf::Texture alphaTex;
	alphaTex.loadFromFile( "alphatext.png" );
	Sprite alphaTextSprite(alphaTex);
	
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
	bool bdrawdraw = false;

	OpenFile( fileName );
	
	Camera cam;
	cam.pos.x = player.position.x;
	cam.pos.y = player.position.y;
	
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

	controller.UpdateState();
	currInput = controller.GetState();
	//ControllerState con = controller.GetState();

	

	bool t = currInput.start;//sf::Keyboard::isKeyPressed( sf::Keyboard::Y );
	bool s = t;
	t = false;
	bool goalPlayerCollision = false;
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
		//	cout << "currInputleft: " << currInput.leftShoulder << endl;
			if( oneFrameMode )
			{
				//controller.UpdateState();
				

				ControllerState con;
				//con = controller.GetState();

				while( true )
				{
					//prevInput = currInput;
					//player.prevInput = currInput;
					controller.UpdateState();
					con = controller.GetState();
					//player.currInput = currInput;

					if( !skipped && con.leftShoulder )//sf::Keyboard::isKeyPressed( sf::Keyboard::K ) && !skipped )
					{
						skipped = true;
						accumulator = 0;//TIMESTEP;
						
						//currentTime = gameClock.getElapsedTime().asSeconds() - TIMESTEP;

						break;
					}
					if( skipped && !con.leftShoulder )//!sf::Keyboard::isKeyPressed( sf::Keyboard::K ) && skipped )
					{
						skipped = false;
						//break;
					}
					if( sf::Keyboard::isKeyPressed( sf::Keyboard::L ) )
					{

						//oneFrameMode = false;
						break;
					}
					//if( sf::Keyboard::isKeyPressed( sf::Keyboard::M ) )
					if( con.rightShoulder )
					{

						oneFrameMode = false;
						break;
					}
					

				}
			}

			//if( currInput.leftShoulder )
			//	oneFrameMode = true;


			if( sf::Keyboard::isKeyPressed( sf::Keyboard::K ) )
			{
				player.position = originalPos;
				player.action = player.JUMP;
				player.frame = 1;
				player.velocity.x = 0;
				player.velocity.y = 0;
				player.reversed = false;
				player.b.offset.y = 0;
				player.b.rh = player.normalHeight;
				player.facingRight = true;
				player.offsetX = 0;
				player.prevInput = ControllerState();
				player.currInput = ControllerState();
				player.ground = NULL;
				player.grindEdge = NULL;
			}

			if( sf::Keyboard::isKeyPressed( sf::Keyboard::Y ) || currInput.start )
			{
				quit = true;
				break;
			}
		//	if( !s && currInput.start )//sf::Keyboard::isKeyPressed( sf::Keyboard::Y ) )
	//	{
			
		//	cout << "exit" << endl;
		//	quit = true;
		//	returnVal = 1;
		//	break;
			
			//t = true;
		//}
		//	else if( s && !currInput.start )//!sf::Keyboard::isKeyPressed( sf::Keyboard::Y ) )
		//{
		//	s = false;

//		}


		if( sf::Keyboard::isKeyPressed( sf::Keyboard::Escape ) )
		{
			quit = true;
			returnVal = 1;
			break;
		}

		if( goalPlayerCollision )
			{
				quit = true;
				returnVal = 1;
				break;
			}



			prevInput = currInput;
			player.prevInput = currInput;

			if( !controller.UpdateState() )
			{
				bool up = Keyboard::isKeyPressed( Keyboard::Up ) || Keyboard::isKeyPressed( Keyboard::W );
				bool down = Keyboard::isKeyPressed( Keyboard::Down ) || Keyboard::isKeyPressed( Keyboard::S );
				bool left = Keyboard::isKeyPressed( Keyboard::Left ) || Keyboard::isKeyPressed( Keyboard::A );
				bool right = Keyboard::isKeyPressed( Keyboard::Right ) || Keyboard::isKeyPressed( Keyboard::D );

			//	bool altUp = Keyboard::isKeyPressed( Keyboard::U );
			//	bool altLeft = Keyboard::isKeyPressed( Keyboard::H );
			//	bool altRight = Keyboard::isKeyPressed( Keyboard::K );
			//	bool altDown = Keyboard::isKeyPressed( Keyboard::J );

				ControllerState keyboardInput;    
				keyboardInput.B = Keyboard::isKeyPressed( Keyboard::X ) || Keyboard::isKeyPressed( Keyboard::Period );
				keyboardInput.X = Keyboard::isKeyPressed( Keyboard::C ) || Keyboard::isKeyPressed( Keyboard::Comma );
				keyboardInput.Y = Keyboard::isKeyPressed( Keyboard::V ) || Keyboard::isKeyPressed( Keyboard::M );
				keyboardInput.A = Keyboard::isKeyPressed( Keyboard::Z ) || Keyboard::isKeyPressed( Keyboard::Space ) || Keyboard::isKeyPressed( Keyboard::Slash );
				keyboardInput.leftTrigger = 255 * (Keyboard::isKeyPressed( Keyboard::F ) || Keyboard::isKeyPressed( Keyboard::L ));
			
				keyboardInput.start = Keyboard::isKeyPressed( Keyboard::J );
				keyboardInput.back = Keyboard::isKeyPressed( Keyboard::H );
				

				/*if( altRight )
					currInput .altPad += 1 << 3;
				if( altLeft )
					currInput .altPad += 1 << 2;
				if( altUp )
					currInput .altPad += 1;
				if( altDown )
					currInput .altPad += 1 << 1;*/
				
				if( up && down )
				{
					if( prevInput.LUp() )
						keyboardInput.leftStickPad += 1;
					else if( prevInput.LDown() )
						keyboardInput.leftStickPad += ( 1 && down ) << 1;
				}
				else
				{
					keyboardInput.leftStickPad += 1 && up;
					keyboardInput.leftStickPad += ( 1 && down ) << 1;
				}

				if( left && right )
				{
					if( prevInput.LLeft() )
					{
						keyboardInput.leftStickPad += ( 1 && left ) << 2;
					}
					else if( prevInput.LRight() )
					{
						keyboardInput.leftStickPad += ( 1 && right ) << 3;
					}
				}
				else
				{
					keyboardInput.leftStickPad += ( 1 && left ) << 2;
					keyboardInput.leftStickPad += ( 1 && right ) << 3;
				}

				currInput = keyboardInput;
			}
			
			else
			{
			controller.UpdateState();
			currInput = controller.GetState();
			currInput.B |= currInput.rightTrigger > 200;
	//		cout << "up: " << currInput.LUp() << ", " << (int)currInput.leftStickPad << ", " << (int)currInput.pad << ", " << (int)currInput.rightStickPad << endl;
			}
			player.currInput = currInput;
			player.UpdatePrePhysics();

			UpdateEnemiesPrePhysics();
			

			//Vector2<double> rCenter( r.getPosition().x + r.getLocalBounds().width / 2, r.getPosition().y + r.getLocalBounds().height / 2 );

			//colMode = 

			//Rect<double> qrect( player.position.x + player.b.offset.x - player.b.rw, 
			//	player.position.y + player.b.offset.y -player.b.rh, player.b.rw, player.b.rh );
			//Query( &player, testTree, qrect );


			player.UpdatePhysics( );

			UpdateEnemiesPhysics();

			//temporary for goal collision
			double gLeft = goalSprite.getPosition().x - goalSprite.getLocalBounds().width / 2.0;
			double gRight = goalSprite.getPosition().x + goalSprite.getLocalBounds().width / 2.0;
			double gTop = goalSprite.getPosition().y - goalSprite.getLocalBounds().height / 2.0;
			double gBottom = goalSprite.getPosition().y + goalSprite.getLocalBounds().height  / 2.0;

			double pLeft = player.position.x - player.b.rw;
			double pRight = player.position.x + player.b.rw;
			double pTop = player.position.y - player.b.rh;
			double pBottom = player.position.y + player.b.rh;

			
			if( gLeft <= pRight && gRight >= pLeft && gTop <= pBottom && gBottom >= pTop )
			{
				goalPlayerCollision = true;
			}


			player.UpdatePostPhysics();

			UpdateEnemiesPostPhysics();
		

			cam.Update( &player );

			double camWidth = 960 * cam.GetZoom();
			double camHeight = 540 * cam.GetZoom();
			screenRect = sf::Rect<double>( cam.pos.x - camWidth / 2, cam.pos.y - camHeight / 2, camWidth, camHeight );
			
			
			/*sf::RectangleShape rs;
			rs.setSize( sf::Vector2f(screenRect.width, screenRect.height ) );
			rs.setPosition( screenRect.left, screenRect.top );
			//rs.setSize( Vector2f(64, 64) );
			//rs.setOrigin( rs.getLocalBounds().width / 2, rs.getLocalBounds().height / 2 );
			//rs.setPosition( otherPlayerPos.x, otherPlayerPos.y  );
			rs.setFillColor( sf::Color( 0, 0, 255, 100 ) );
			window->draw( rs );*/

			Query( this, enemyTree, screenRect );

			accumulator -= TIMESTEP;
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
		Vector2f camOffset;
		
		
		
		
		


		view.setSize( Vector2f( 960 * cam.GetZoom(), 540 * cam.GetZoom()) );
		lastViewSize = view.getSize();

		//view.setCenter( player.position.x + camOffset.x, player.position.y + camOffset.y );
		view.setCenter( cam.pos.x, cam.pos.y );
		lastViewCenter = view.getCenter();


		
		window->setView( bgView );

		window->draw( background );

		
		
		window->setView( view );



		
		bDraw.setSize( sf::Vector2f(player.b.rw * 2, player.b.rh * 2) );
		bDraw.setOrigin( bDraw.getLocalBounds().width /2, bDraw.getLocalBounds().height / 2 );
		bDraw.setPosition( player.position.x + player.b.offset.x , player.position.y + player.b.offset.y );
	//	bDraw.setRotation( player.sprite->getRotation() );
		if( bdrawdraw)
		window->draw( bDraw );

	/*	CircleShape cs;
		cs.setFillColor( Color::Cyan );
		cs.setRadius( 10 );
		cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
		cs.setPosition( player.position.x, player.position.y );
		window->draw( cs );*/

	
		//player.sh.setParameter( "u_texture", *GetTileset( "testrocks.png", 25, 25 )->texture );
		//player.sh.setParameter( "u_texture1", *GetTileset( "testrocksnormal.png", 25, 25 )->texture );
		
		
		Vector2i vi = Mouse::getPosition();
		
		
		Vector3f blahblah( vi.x / 1920.f, (1080 - vi.y) / 1080.f, .075 );

		window->draw( goalSprite );

		//player.sprite->setTextureRect( IntRect( 0, 0, 300, 225 ) );
		/*if( false )
		//if( player.action == player.RUN )
		{
			player.sh.setParameter( "u_texture",( *GetTileset( "run.png" , 128, 64 )->texture ) ); //*GetTileset( "testrocks.png", 25, 25 )->texture );
			player.sh.setParameter( "u_normals", *GetTileset( "run_normal.png", 128, 64 )->texture );
			player.sh.setParameter( "Resolution", Vector2f( 1920, 1080 ) );
			player.sh.setParameter( "LightPos", blahblah  );
			player.sh.setParameter( "LightColor", Color::White );
			player.sh.setParameter( "AmbientColor", Color( .5, .5, .5, .1 ) );
			player.sh.setParameter( "Falloff", Vector3f( .3, .3, .3 ) );
			window->draw( *(player.sprite), &player.sh );
		}
		else
		{
			if( player.action != player.GRINDBALL )
				window->draw( *player.sprite );
		}*/

		
		player.Draw( window );

		UpdateEnemiesDraw();


		
		sf::RectangleShape rs;
		rs.setSize( Vector2f(64, 64) );
		rs.setOrigin( rs.getLocalBounds().width / 2, rs.getLocalBounds().height / 2 );
		rs.setPosition( otherPlayerPos.x, otherPlayerPos.y  );
		rs.setFillColor( Color::Blue );
		//window->draw( circle );
		//window->draw(line, numPoints * 2, sf::Lines);
		//polyShader.setParameter( "resolution", view.getSize().x, view.getSize().y );
		//polyShader.setParameter( "random", rand() % 100 );
		polyShader.setParameter( "vel", player.velocity.x, player.velocity.y );
		polyShader.setParameter( "u_texture", *GetTileset( "testrocks.png", 25, 25 )->texture );
		//polyShader.CurrentTexture = GetTileset( "testrocks.png", 25, 25 )->texture;

		for( list<VertexArray*>::iterator it = polygons.begin(); it != polygons.end(); ++it )
		{
		//	window->draw( *(*it) );
			//window->draw
			window->draw( *(*it ) );//, &polyShader);//GetTileset( "testrocks.png", 25, 25 )->texture );
		}
		//window->draw(border, 8, sf::Lines);

		//DebugDrawQuadTree( window, testTree );

		

		if( currInput.back || sf::Keyboard::isKeyPressed( sf::Keyboard::H ) )
		{
			//alphaTextSprite.setOrigin( alphaTextSprite.getLocalBounds().width / 2, alphaTextSprite.getLocalBounds().height / 2 );
//			alphaTextSprite.setScale( .5, .5 );
			alphaTextSprite.setScale( .5 * view.getSize().x / 960.0, .5 * view.getSize().y / 540.0 );
			alphaTextSprite.setOrigin( alphaTextSprite.getLocalBounds().width / 2, alphaTextSprite.getLocalBounds().height / 2 );
			alphaTextSprite.setPosition( view.getCenter().x, view.getCenter().y );


			window->draw( alphaTextSprite );
		}

//		DebugDrawQuadTree( window, enemyTree );

		window->display();

		
	}

	delete [] line;

	//window->setView( window->getDefaultView() );
	//window->clear( Color::Red );
	//window->display();
	return returnVal;
}

void GameSession::HandleEnemy( Enemy *e )
{
	//sf::Rect<double> screenRect( cam.pos.x - camWidth / 2, cam.pos.y - camHeight / 2, camWidth, camHeight );
	if( e->spawnRect.intersects( screenRect ) )
	{
		//cout << "spawning enemy!" << endl;
		e->spawned = true;
		AddEnemy( e );
	}
}