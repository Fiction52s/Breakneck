#include <iostream>
//#include "PlayerChar.h"
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include <assert.h>
#include <fstream>
#include <list> 
#include <stdlib.h>

#define TIMESTEP 1.f / 60.f

using namespace std;
using namespace sf;

RenderWindow *window;

float cross( sf::Vector2f a, sf::Vector2f b )
{
	return a.x * b.y - a.y * b.x;
}

sf::Vector2f normalize( sf::Vector2f v )
{
	float vLen = sqrtf( v.x * v.x + v.y * v.y );
	return sf::Vector2f( v.x / vLen, v.y / vLen );
}

float dot( sf::Vector2f a, sf::Vector2f b )
{
	return a.x * b.x + a.y * b.y;
}

struct Edge
{
	Vector2f v0;
	Vector2f v1;
	Edge * GetEdge0();
	Edge * GetEdge1();
	Edge *edge0;
	Edge *edge1;
	//material ---

	Vector2f Normal()
	{
		Vector2f v = v1 - v0;
		Vector2f temp = normalize( v );
		return Vector2f( temp.y, -temp.x );
	}

	Vector2f GetPoint( float f )
	{
		Vector2f e( v1 - v0 );
		e.x *= f;
		e.y *= f;
		e += v0;
		return e; 
	}
};

struct CollisionBox
{

	enum BoxType
	{
		Physics,
		Hit,
		Hurt
	};

	Vector2f offset;
	float offsetAngle;
	
	float rw; //radius or half width
	float rh; //radius or half height
	bool isCircle;
	BoxType type;

	Vector2f GetAxis1( Vector2f actorPos )
	{
	/*	float left = actorPos.x + offset.x - rw;
		float right = actorPos.x + offset.x + rw;
		float top = actorPos.y + offset.y - rh;
		float bottom = actorPos.y + offset.y + rh;
		Vector2f topLeft( left, top );
		Vector2f topRight( right, top );
		Vector2f bottomLeft( left, bottom );

		topLeft.x = cos( offsetAngle ) * topLeft.x + sin( offsetAngle ) * topLeft.y;
		topLeft.y = -sin( offsetAngle ) * topLeft.x + cos( offsetAngle ) * topLeft.y;*/
	}
};

struct Contact
{
	Contact()
		:edge( NULL )
	{
	}
	Vector2f position;
	Vector2f resolution;
	Edge *edge;
};



struct Tileset
{
	IntRect GetSubRect( int localID )
	{
		int xi,yi;
		yi = localID / (texture->getSize().x / tileWidth );
		xi = localID % (texture->getSize().x / tileWidth );

		return IntRect( xi * tileWidth, yi * tileHeight, tileWidth, tileHeight ); 
	}

	Texture * texture;
	int tileWidth;
	int tileHeight;
	string sourceName;
};

list<Tileset*> tilesetList;

Tileset * GetTileset( const string & s, int tileWidth, int tileHeight )
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

struct Actor
{
	enum Action
	{
		STAND,
		RUN,
		JUMP,
		Count
	};

	Sprite *sprite;
	Tileset *tilesetStand;
	Tileset *tilesetRun;

	int actionLength[Action::Count]; //actionLength-1 is the max frame counter for each action
	Actor::Actor()
	{
		sprite = new Sprite;
		velocity = Vector2f( 0, 0 );
		actionLength[STAND] = 18 * 4;
		tilesetStand = GetTileset( "stand.png", 64, 64 );

		actionLength[RUN] = 10 * 4;
		tilesetRun = GetTileset( "run.png", 128, 64 );

		actionLength[JUMP] = 5;

		currentAction = RUN;
		frame = 0;
	}

	void ActionEnded()
	{
		if( frame >= actionLength[currentAction] )
		{
			switch( currentAction )
			{
			case STAND:
				frame = 0;
				break;
			case RUN:
				frame = 0;
				break;
			}
		}
	}

	void UpdatePrePhysics()
	{
		ActionEnded();


		switch( currentAction )
		{
		case STAND:
			break;
		case RUN:
			break;
		case JUMP:
			break;
		}

	//	cout << "position: " << position.x << ", " << position.y << endl;
	//	cout << "velocity: " << velocity.x << ", " << velocity.y << endl;
		position += velocity;
	}

	void UpdatePhysics()
	{
	}

	void UpdatePostPhysics()
	{
		//cout << "updating" << endl;
		switch( currentAction )
		{
		case STAND:
				//display stand at the current frame
			sprite->setPosition( position );
			
			sprite->setTexture( *(tilesetStand->texture));
			sprite->setTextureRect( tilesetStand->GetSubRect( frame / 4 ) );
			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2 );
			//cout << "setting to frame: " << frame / 4 << endl;
			break;
		case RUN:
			sprite->setPosition( position );
			
			sprite->setTexture( *(tilesetRun->texture));
			sprite->setTextureRect( tilesetRun->GetSubRect( frame / 4 ) );
			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2 );
			break;
		case JUMP:
			break;
		}

		++frame;
	}


	Action currentAction;
	int frame;
	Vector2f position;
	Vector2f velocity;
	CollisionBox *physBox;
};

sf::Vector2f lineIntersection( Vector2f a, Vector2f b, Vector2f c, Vector2f d )
{
	float x = ((a.x * b.y - a.y * b.x ) * ( c.x - d.x ) - (a.x - b.x ) * ( c.x * d.y - c.y * d.x ))
			/ ( (a.x-b.x)*(c.y - d.y) - (a.y - b.y) * (c.x - d.x ) );
	
	float y = ((a.x * b.y - a.y * b.x ) * ( c.y - d.y ) - (a.y - b.y ) * ( c.x * d.y - c.y * d.x ))
			/ ( (a.x-b.x)*(c.y - d.y) - (a.y - b.y) * (c.x - d.x ) );

	sf::CircleShape cs;
	//cs.setOutlineColor( Color::Cyan );
	//cs.setOutlineThickness( 10 );
	cs.setFillColor( Color::Cyan );
	cs.setRadius( 20 );
	cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
	cs.setPosition( x,y );
	window->draw( cs );
	cout << "cirlce pos: " << cs.getPosition().x << ", " << cs.getPosition().y << endl;

	return sf::Vector2f( x, y );
}

Contact *currentContact;
Contact *collideEdge( Actor &a, const CollisionBox &b, Edge *e )
{
	float left = a.position.x + b.offset.x - b.rw;
	float right = a.position.x + b.offset.x + b.rw;
	float top = a.position.y + b.offset.y - b.rh;
	float bottom = a.position.y + b.offset.y + b.rh;

	float edgeLeft = min( e->v0.x, e->v1.x );
	float edgeRight = max( e->v0.x, e->v1.x ); 
	float edgeTop = min( e->v0.y, e->v1.y ); 
	float edgeBottom = max( e->v0.y, e->v1.y ); 

	/*sf::RectangleShape r1( Vector2f(100,300) );
	r1.setPosition( actorPos.x - 50, actorPos.y - 150 );
	r1.setOutlineColor( sf::Color::Red );
	r1.setOutlineThickness( 10 );
	r1.setFillColor( sf::Color::Transparent );
	window->draw( r1 );

	sf::RectangleShape r2( Vector2f(edgeRight - edgeLeft,edgeBottom - edgeTop) );
	r2.setPosition( edgeLeft, edgeTop );
	r2.setOutlineColor( sf::Color::Green );
	r2.setOutlineThickness( 10 );
	r2.setFillColor( sf::Color::Transparent );
	window->draw( r2 );*/


	bool aabbCollision = false;
	if( left <= edgeRight && right >= edgeLeft && top <= edgeBottom && bottom >= edgeTop )
	{	
		aabbCollision = true;
	}

	if( aabbCollision )
	{
		Vector2f corner(0,0);
		Vector2f edgeNormal = e->Normal();
		cout << "normal: " << edgeNormal.x << ", " << edgeNormal.y << endl;
		bool resolve = false;

		if( edgeNormal.x > 0 )
		{
			if( edgeNormal.y > 0 )
			{
				Vector2f topLeft( left, top );
				corner = topLeft;
				resolve = true;
			}
			else if( edgeNormal.y < 0 )
			{
				
				Vector2f bottomLeft( left, bottom );
				corner = bottomLeft;
				resolve = true;
			}
			else
			{
				//Vector2f topLeft( top, left );
				//Vector2f bottomLeft( bottom, left );

			//	corner = Vector2f( left, (top + bottom) / 2 );
			//	resolve = true;
				//currentContact->position = (topLeft + bottomLeft) / 2;
				//currentContact->resolution = intersect - currentContact->position;
				//currentContact->edge = e;
				

			}
		}
		else if( edgeNormal.x < 0 )
		{
			if( edgeNormal.y > 0 )
			{
				Vector2f topRight( right, top );
				corner = topRight;
				resolve = true;
			}
			else if( edgeNormal.y < 0 )
			{
				Vector2f bottomRight( right, bottom );
				corner = bottomRight;
				resolve = true;
			}
			else
			{
				//Vector2f topRight( right, top );
				//Vector2f bottomRight( right, bottom );
				//corner = (topRight + bottomRight) / 2;
			//	corner = Vector2f( right, (top + bottom) / 2);
			//	resolve = true;
			}
		}
		else
		{
			//up and down
			if( edgeNormal.y > 0 )
			{
				//Vector2f bottomLeft( left, bottom );
				//Vector2f bottomRight( right, bottom );
			//	corner = Vector2f( (left + right) / 2, bottom );
			//	resolve = true;
			}
			else if( edgeNormal.y < 0 )
			{
				//Vector2f topLeft( left, top );
				//Vector2f topRight( right, top);
				//corner = (topLeft + topRight) / 2;
			//	corner = Vector2f( (left + right) / 2, top );
			//	resolve = true;
			}
			else
			{
				//never happens
			}
		}

		if( resolve )
		{
		//	Vector2f invVel = normalize(-a.velocity);
		}

		int res = cross( corner - e->v0, e->v1 - e->v0 );
		if( res < 0 && resolve )
		{
			Vector2f invVel = normalize(-a.velocity);
			Vector2f intersect = lineIntersection( corner, corner + invVel, e->v0, e->v1 );

			sf::CircleShape cs;
			cs.setFillColor( Color::Magenta );
				
			cs.setRadius( 20 );
			cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
			cs.setPosition( corner );
			window->draw( cs );

			//cs.setPosition( bottomLeft - invVel );
			//window->draw( cs );

			//Vector2f p = intersect - corner;
			currentContact->position = corner;
			currentContact->resolution = intersect - corner;
			currentContact->edge = e;

			return currentContact;
			//a.position = a.position + p;
		}


	}

	return NULL;
}

void collideRectRect()
{
}








int main()
{
	window = new sf::RenderWindow(/*sf::VideoMode(1400, 900)sf::VideoMode::getDesktopMode()*/
		sf::VideoMode( 1920 / 2, 1080 / 2), "Breakneck", sf::Style::None, sf::ContextSettings( 0, 0, 0, 0, 0 ));
	sf::Vector2i pos( 0, 0 );

	window->setPosition( pos );
	window->setVerticalSyncEnabled( true );
	//window->setFramerateLimit( 60 );
	window->setMouseCursorVisible( true );
	
	View view( sf::Vector2f(  300, 300 ), sf::Vector2f( 960, 540 ) );
	window->setView( view );


	currentContact = new Contact;

	
	ifstream is;
	is.open( "test.brknk" );

	int numPoints;
	is >> numPoints;
	Vector2f *points = new Vector2f[numPoints];
	list<Vector2f> lvec;
	//load points
	for( int i = 0; i < numPoints; ++i )
	{
		float px, py;
		is >> px;
		is >> py;
			
		points[i].x = px;
		points[i].y = py;
	}
	
	int totalEdges = numPoints;
	int numPolygons;

	is >> numPolygons;

	
	
	Edge **edges = new Edge*[totalEdges];
	
	int currentEdgeIndex = 0;

	//start loop

	for( int n = 0; n < numPolygons; ++n )
	{
		string materialStr;
		is >> materialStr;
		int numPolygonEdges;
		is >> numPolygonEdges;

		for( int i = 0; i < numPolygonEdges; ++i )
		{
			Edge *ee = new Edge();
			edges[currentEdgeIndex + i] = ee;
			ee->v0 = points[i+currentEdgeIndex];
			if( i < numPolygonEdges - 1 )
				ee->v1 = points[i+1 + currentEdgeIndex];

		

			if( i == 0 )
			{
				ee->edge0 = edges[currentEdgeIndex + numPolygonEdges - 1];
				ee->edge1 = edges[currentEdgeIndex + 1];
			}
			else if( i == numPolygonEdges - 1 )
			{
				ee->edge0 = edges[currentEdgeIndex + i - 1];
				ee->edge1 = edges[currentEdgeIndex];
				ee->v1 = points[currentEdgeIndex];
			}
			else
			{
				ee->edge0 = edges[currentEdgeIndex + i - 1];
				ee->edge1 = edges[currentEdgeIndex + i + 1];
			}

			//cout << "edge " << i << " from: " << ee->v0.x << ", " << ee->v0.y << " to " << ee->v1.x << ", " << ee->v1.y << endl;
		
		}
		currentEdgeIndex += numPolygonEdges;
	}
	//end loop

	/*for( int i = 0; i < numEdges; ++i )
	{
		int v0i,v1i;
		is >> v0i;
		is >> v1i;

		edges[i].v0 = points[v0i];
		edges[i].v1 = points[v1i];
	}*/

	//int lineCount = lvec.size();
	sf::Vertex *line = new sf::Vertex[totalEdges*2];
	for( int i = 0; i < totalEdges; ++i )
	{
		//cout << "i: " << i << endl;
		line[i*2] = sf::Vertex( edges[i]->v0 );
		line[i*2+1] =  sf::Vertex( edges[i]->v1 );
	}


	//load edges

	/*list<Edge*> eList;
	

	while( true )
	{
		string s;
		is >> s;
		if( s == "x" )
		{
			break;

		}
		else
		{
			int i;
			i = atoi( s.c_str() );
			int i1;
			string s1;
			is >> s1;
			i1 = atoi( s1.c_str() );
			Edge *e = new Edge;
			e->v0 = line

		}
	}*/
	
	

	

	sf::Vector2f nLine = line[1].position - line[0].position;
	nLine = normalize( nLine );

	sf::Vector2f lineNormal( -nLine.y, nLine.x );

	sf::CircleShape circle( 30 );
	circle.setFillColor( Color::Blue );

	/*Edge * edgeList = new Edge[lineCount];
	for( int i = 0; i < lineCount-1; ++i )
	{
		edgeList[i].v0 = line[i*2].position;
		edgeList[i].v1 = line[i*2+1].position;
	}
	edgeList[lineCount-1].v0 = line[(lineCount-2)*2+1].position;
	edgeList[lineCount-1].v1 = line[0].position;*/
	//cout << "blah v1: " << edgeList[lineCount-1].v1.x << ", " << edgeList[lineCount-1].v1.y << endl;
	/*Edge edge1;
	edge1.v0 = sf::Vector2f(0, 600);
	edge1.v1 = sf::Vector2f(1000, 1000);

	Edge edge2;
	edge2.v0 = sf::Vector2f(1000, 1000);
	edge2.v1 = sf::Vector2f(1800, 600);

	Edge edge3;
	edge3.v0 = sf::Vector2f(1800, 600);
	edge3.v1 = sf::Vector2f(900, 100);

	Edge edge4;
	edge4.v0 = sf::Vector2f(900, 100);
	edge4.v1 = sf::Vector2f(0, 150);*/



	CollisionBox b;
	b.isCircle = false;
	b.offsetAngle = 0;
	b.offset.x = 0;
	b.offset.y = 0;
	b.rw = 32;
	b.rh = 32;
	b.type = b.Physics;

	Actor player;
	player.position = Vector2f( -500, 300 );

	sf::Clock gameClock;
	double currentTime = 0;
	double accumulator = TIMESTEP + .1;

	while( true )
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
			float f = 10;			
			player.velocity = Vector2f( 0, 0 );
			if( sf::Keyboard::isKeyPressed( sf::Keyboard::Right ) )
			{
				player.velocity += Vector2f( f, 0 );
				//break;
			}
			if( sf::Keyboard::isKeyPressed( sf::Keyboard::Left ) )
			{
				player.velocity += Vector2f( -f, 0 );
				//break;
			}
			if( sf::Keyboard::isKeyPressed( sf::Keyboard::Up ) )
			{
				player.velocity += Vector2f( 0, -f );
				//break;
			}
			if( sf::Keyboard::isKeyPressed( sf::Keyboard::Down ) )
			{
				player.velocity += Vector2f( 0, f );
				//break;
			}


			player.UpdatePrePhysics();

			//Vector2f rCenter( r.getPosition().x + r.getLocalBounds().width / 2, r.getPosition().y + r.getLocalBounds().height / 2 );
		
			for( int i = 0; i < totalEdges; ++i )
			{
				Contact *c = collideEdge( player, b, edges[i] );
				if( c != NULL )
				{
				player.position += c->resolution;
				cout << "resolution: " << c->resolution.x << ", " << c->resolution.y << endl;
				//break;
				}
			}

			player.UpdatePostPhysics();

			//r.setPosition( player.position.x - r.getLocalBounds().width / 2, player.position.y - r.getLocalBounds().height / 2 );
			cout << "playerPos: " << player.position.x << ", " << player.position.y << endl;	
		

			accumulator -= TIMESTEP;
			while( sf::Keyboard::isKeyPressed( sf::Keyboard::Space ) )
		{
		}
		}

		view.setSize( Vector2f( 960 * 1, 540 * 1 ) );
		view.setCenter( player.position );
		window->setView( view );
		window->draw( *(player.sprite) );
		window->draw( circle );
		window->draw(line, totalEdges * 2, sf::Lines);

		/*Event ev;
		window->pollEvent( ev );
		if( ev.type == Event::Resized )
		{
			window->setSize( 
		}*/

		window->display();

	/*	while( true )
		{
			if( !sf::Keyboard::isKeyPressed( sf::Keyboard::Right ) )
				break;
			if( !sf::Keyboard::isKeyPressed( sf::Keyboard::Left ) )
				break;
			if( !sf::Keyboard::isKeyPressed( sf::Keyboard::Up ) )
				break;
			if( !sf::Keyboard::isKeyPressed( sf::Keyboard::Down) )
				break;
		}*/

		if( sf::Keyboard::isKeyPressed( sf::Keyboard::Escape ) )
				return 0;

		


		
		
		
	}

	window->close();
	delete window;
}

