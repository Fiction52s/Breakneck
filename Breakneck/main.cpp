#include <iostream>
//#include "PlayerChar.h"
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include <assert.h>
#include <fstream>
#include <list> 
#include <stdlib.h>

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

list<Tileset*> *tilesetList;
Tileset* tilesets;
int tilesetCount;

int GetTilesetIndex( const string & s )
{
	int i = 0;
	for( list<Tileset*>::iterator it = tilesetList->begin(); it != tilesetList->end(); ++it )
	{
		if( (*it)->sourceName == s )
		{
			return i;
		}
		++i;
	}

	//not found

	//make sure to set up tileset here
	tilesetList->push_back( new Tileset() );

	return i;
}

Tileset * GetTileset( int i )
{
	assert( i > 0 && i < tilesetCount );

	return tilesets[i];
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
	int tilesetStand;

	int actionLength[Action::Count]; //actionLength-1 is the max frame counter for each action
	Actor::Actor()
	{
		actionLength[STAND] = 5;
		tilesetStand = GetTilesetIndex( "stand.png" );

		actionLength[RUN] = 10;
		actionLength[JUMP] = 5;
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
	}

	void UpdatePhysics()
	{
	}

	void UpdatePostPhysics()
	{
		switch( currentAction )
		{
		case STAND:
				//display stand at the current frame
			break;
		case RUN:
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

void collideEdge( Actor &a, const CollisionBox &b, Edge &e )
{
	float left = a.position.x + b.offset.x - b.rw;
	float right = a.position.x + b.offset.x + b.rw;
	float top = a.position.y + b.offset.y - b.rh;
	float bottom = a.position.y + b.offset.y + b.rh;

	float edgeLeft = min( e.v0.x, e.v1.x );
	float edgeRight = max( e.v0.x, e.v1.x ); 
	float edgeTop = min( e.v0.y, e.v1.y ); 
	float edgeBottom = max( e.v0.y, e.v1.y ); 

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
		Vector2f edgeNormal = e.Normal();
		cout << "normal: " << edgeNormal.x << ", " << edgeNormal.y << endl;
		if( edgeNormal.x > 0 )
		{
			if( edgeNormal.y > 0 )
			{
				Vector2f topLeft( left, top );
				corner = topLeft;
			}
			else if( edgeNormal.y < 0 )
			{
				
				Vector2f bottomLeft( left, bottom );
				corner = bottomLeft;
			}
			else
			{
				Vector2f topLeft( top, left );
				Vector2f bottomLeft( bottom, left );
			}
		}
		else if( edgeNormal.x < 0 )
		{
			if( edgeNormal.y > 0 )
			{
				Vector2f topRight( right, top );
				corner = topRight;
			}
			else if( edgeNormal.y < 0 )
			{
				Vector2f bottomRight( right, bottom );
				corner = bottomRight;
			}
			else
			{
			}
		}
		else
		{
			//up and down
			if( edgeNormal.y > 0 )
			{

			}
			else if( edgeNormal.y < 0 )
			{
			}
			else
			{
				//never happens
			}
		}

		int res = cross( corner - e.v0, e.v1 - e.v0 );
		if( res < 0 )
		{
			Vector2f invVel = normalize(-a.velocity);
			Vector2f intersect = lineIntersection( corner, corner + invVel, e.v0, e.v1 );

			sf::CircleShape cs;
			cs.setFillColor( Color::Magenta );
				
			cs.setRadius( 20 );
			cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
			cs.setPosition( corner );
			window->draw( cs );

			//cs.setPosition( bottomLeft - invVel );
			//window->draw( cs );

			Vector2f p = intersect - corner;
			a.position = a.position + p;
		}


	}
}

void collideRectRect()
{
}








int main()
{
	window = new sf::RenderWindow(/*sf::VideoMode(1400, 900)sf::VideoMode::getDesktopMode()*/
			sf::VideoMode( 1920, 1080 ), "Breakneck", sf::Style::Default, sf::ContextSettings( 0, 0, 0, 0, 0 ));
	sf::Vector2i pos( 0, 0 );

	window->setPosition( pos );
	window->setVerticalSyncEnabled( true );
	//window->setFramerateLimit( 60 );
	window->setMouseCursorVisible( true );
	

	sf::RectangleShape r( sf::Vector2f( 100, 300 ) );
	r.setPosition( 300, 300 );
	
	ifstream is;
	is.open( "test.brknk" );

	list<Vector2f> lvec;
	while( true )
	{
		string s;
		is >> s;
		if( s == "e" )
		{
			break;
		}
		else
		{
			float f;
			f = atof( s.c_str() );
			float f1;
			string s2;
			is >> s2;
			f1 = atof( s2.c_str() );

			lvec.push_back( Vector2f( f, f1 ) );
		}
	}
	

	int lineCount = lvec.size();
	sf::Vertex *line = new sf::Vertex[lineCount*2];
	list<Vector2f>::iterator it = lvec.begin();
	for( int i = 0; i < lineCount * 2 - 2; ++i )
	{
		line[i] = sf::Vertex( (*it) );
		++it;
		line[++i] =  sf::Vertex( (*it) );
	}
	line[lineCount*2 -1] = sf::Vertex( (*it) );
	it = lvec.begin();
	line[lineCount * 2 - 2] = sf::Vertex( (*it) );

	sf::Vector2f nLine = line[1].position - line[0].position;
	nLine = normalize( nLine );

	sf::Vector2f lineNormal( -nLine.y, nLine.x );

	sf::CircleShape circle( 30 );
	circle.setFillColor( Color::Blue );

	sf::Vector2f rectVel( 0, 100 );

	Edge * edgeList = new Edge[lineCount];
	for( int i = 0; i < lineCount-1; ++i )
	{
		edgeList[i].v0 = line[i*2].position;
		edgeList[i].v1 = line[i*2+1].position;
	}
	edgeList[lineCount-1].v0 = line[(lineCount-2)*2+1].position;
	edgeList[lineCount-1].v1 = line[0].position;
	cout << "blah v1: " << edgeList[lineCount-1].v1.x << ", " << edgeList[lineCount-1].v1.y << endl;
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
	b.rw = 50;
	b.rh = 150;
	b.type = b.Physics;

	Actor player;
	

	while( true )
	{

		window->clear();
		

		r.setPosition( r.getPosition().x + rectVel.x, r.getPosition().y + rectVel.y );


		player.velocity = rectVel;
		player.position = Vector2f( r.getPosition().x + r.getLocalBounds().width / 2, 
			r.getPosition().y + r.getLocalBounds().height / 2 );

		//Vector2f rCenter( r.getPosition().x + r.getLocalBounds().width / 2, r.getPosition().y + r.getLocalBounds().height / 2 );
		
		for( int i = 0; i < lineCount; ++i )
		{
			collideEdge( player, b, edgeList[i] );
		}
		//collideEdge( player, b, edge1 );
		//collideEdge( player, b, edge2 );
		//collideEdge( player, b, edge3 );
		//collideEdge( player, b, edge4 );

		r.setPosition( player.position.x - r.getLocalBounds().width / 2, player.position.y - r.getLocalBounds().height / 2 );
		/*for( int i = 1; i < lineCount; ++i )
		{
			sf::Vector2f temp = sf::Vector2f( r.getPosition().x ,//+ r.getGlobalBounds().width,
			r.getPosition().y + r.getGlobalBounds().height ) - line[i].position;
		
			normalize( temp );

			sf::Vector2f edge = line[i].position - line[i-1].position;

			sf::Vector2f normal( edge.y, -edge.x );
			normal = normalize( normal );

			int res = cross( temp, nLine );

			if( normal.x > 0 )
			{
				if( res < 0 && r.getPosition().x >= 0 && r.getPosition().x < line[1].position.x )
				{
					r.setPosition( r.getPosition().x + lineNormal.x * res, r.getPosition().y + lineNormal.y * res);
					cout << "move" << endl;
				}
			}
			else if( normal.x < 0 )
			{
				if( res < 0 && r.getPosition().x >= 0 && r.getPosition().x < line[1].position.x )
				{
					r.setPosition( r.getPosition().x + lineNormal.x * res, r.getPosition().y + lineNormal.y * res);
					cout << "move" << endl;
				}
			}
			cout << "normal: " << normal.x << ", " << normal.y << endl;
		cout << "cross: " << res << endl;
		
		
			
		}*/
		

		window->draw( r );
		window->draw( circle );
		window->draw(line, lineCount * 2, sf::Lines);

		window->display();

		while( true )
		{
			if( !sf::Keyboard::isKeyPressed( sf::Keyboard::Right ) )
				break;
			if( !sf::Keyboard::isKeyPressed( sf::Keyboard::Left ) )
				break;
			if( !sf::Keyboard::isKeyPressed( sf::Keyboard::Up ) )
				break;
			if( !sf::Keyboard::isKeyPressed( sf::Keyboard::Down) )
				break;
		}

		

		while( true )
		{
			float f = 20;
			rectVel = Vector2f( 0, 0 );
			if( sf::Keyboard::isKeyPressed( sf::Keyboard::Right ) )
			{
				rectVel += Vector2f( f, 0 );
				//break;
			}
			if( sf::Keyboard::isKeyPressed( sf::Keyboard::Left ) )
			{
				rectVel += Vector2f( -f, 0 );
				//break;
			}
			if( sf::Keyboard::isKeyPressed( sf::Keyboard::Up ) )
			{
				rectVel += Vector2f( 0, -f );
				//break;
			}
			if( sf::Keyboard::isKeyPressed( sf::Keyboard::Down ) )
			{
				rectVel += Vector2f( 0, f );
				//break;
			}

			if( rectVel.x != 0 || rectVel.y != 0 )
				break;

			if( sf::Keyboard::isKeyPressed( sf::Keyboard::Escape ) )
				return 0;
			
		}


		
		
		
	}

	window->close();
	delete window;
}

