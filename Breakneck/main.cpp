#include <iostream>
//#include "PlayerChar.h"
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include <assert.h>
#include <fstream>
#include <list> 
#include <stdlib.h>
#include "EditSession.h"

#define TIMESTEP 1.f / 60.f

using namespace std;
using namespace sf;

RenderWindow *window;

double cross( sf::Vector2f a, sf::Vector2f b )
{
	double ax = a.x;
	double ay = a.y;
	double bx = b.x;
	double by = b.y;
	return ax * by - ay * bx;
	//return a.x * b.y - a.y * b.x;
}


double length( Vector2f v)
{
	double vx = v.x;
	double vy = v.y;
	return sqrt( vx * vx + vy * vy );
}

sf::Vector2f normalize( sf::Vector2f v )
{
	double vLen = length( v );
	if( vLen > 0 )
		return sf::Vector2f( v.x / vLen, v.y / vLen );
	else
		return Vector2f( 0, 0 );
}

double dot( sf::Vector2f a, sf::Vector2f b )
{
	double ax = a.x;
	double ay = a.y;
	double bx = b.x;
	double by = b.y;
	return ax * bx + ay * by;
	//return a.x * b.x + a.y * b.y;
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
	Edge()
	{
		
	}

	Vector2f Normal()
	{
		Vector2f v = v1 - v0;
		Vector2f temp = normalize( v );
		return Vector2f( temp.y, -temp.x );
	}

	Vector2f GetPoint( float quantity )
	{
		//gets the point on a line w/ length quantity in the direction of the edge vector
		Vector2f e( v1 - v0 );
		e = normalize( e );
		return v0 + quantity * e;
	}

	

	float GetQuantity( Vector2f p )
	{
		//projects the origin of the line to p onto the edge. if the point is on the edge it will just be 
		//normal to use dot product to get cos(0) =1
		Vector2f vv = normalize(p - v0);
		Vector2f e = normalize(v1 - v0);
		return dot( vv, e ) * length( p - v0 );                          
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
		collisionPriority = 0;
	}
		
	float collisionPriority;	
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

struct LineIntersection
{
	LineIntersection(const Vector2f &pos, bool p )
	{
		position = pos;
		parallel = p;
	}

	Vector2f position;
	bool parallel;
};

LineIntersection lineIntersection( Vector2f a, Vector2f b, Vector2f c, Vector2f d )
{
	double ax = a.x;
	double ay = a.y;
	double bx = b.x;
	double by = b.y;
	double cx = c.x;
	double cy = c.y;
	double dx = d.x;
	double dy = d.y;

	double x= 0,y = 0;
	bool parallel = false;
	if( (ax-bx)*(cy - dy) - (ay - by) * (cx - dx ) == 0 )
	{
		parallel = true;
		//cout << "error with: ( " << ax << ", " << ay << " ), ( " << bx << ", " << by << " ) and ( "
		//	<< cx << ", " << cy << " ), ( " << dx << ", " << dy << endl;
	}
	else
	{
		x = ((ax * by - ay * bx ) * ( cx - dx ) - (ax - bx ) * ( cx * dy - cy * dx ))
			/ ( (ax-bx)*(cy - dy) - (ay - by) * (cx - dx ) );
		y = ((ax * by - ay * bx ) * ( cy - dy ) - (ay - by ) * ( cx * dy - cy * dx ))
			/ ( (ax-bx)*(cy - dy) - (ay - by) * (cx - dx ) );
		sf::CircleShape cs;
		//cs.setOutlineColor( Color::Cyan );
		//cs.setOutlineThickness( 10 );
		cs.setFillColor( Color::Cyan );
		cs.setRadius( 20 );
		cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
		cs.setPosition( x,y );
	//	window->draw( cs );
		//cout << "circle pos: " << cs.getPosition().x << ", " << cs.getPosition().y << endl;
	}


	return LineIntersection( Vector2f(x,y), parallel );
}

Contact *currentContact;
Contact *collideEdge( Actor &a, const CollisionBox &b, Edge *e )
{
	Vector2f oldPosition = a.position - a.velocity;
	float left = a.position.x + b.offset.x - b.rw;
	float right = a.position.x + b.offset.x + b.rw;
	float top = a.position.y + b.offset.y - b.rh;
	float bottom = a.position.y + b.offset.y + b.rh;

	float oldLeft = oldPosition.x + b.offset.x - b.rw;
	float oldRight = oldPosition.x + b.offset.x + b.rw;
	float oldTop = oldPosition.y + b.offset.y - b.rh;
	float oldBottom = oldPosition.y + b.offset.y + b.rh;


	float edgeLeft = min( e->v0.x, e->v1.x );
	float edgeRight = max( e->v0.x, e->v1.x ); 
	float edgeTop = min( e->v0.y, e->v1.y ); 
	float edgeBottom = max( e->v0.y, e->v1.y ); 

	bool aabbCollision = false;
	if( left <= edgeRight && right >= edgeLeft && top <= edgeBottom && bottom >= edgeTop )
	{	
		aabbCollision = true;
	}

	if( aabbCollision )
	{
		Vector2f corner(0,0);
		Vector2f edgeNormal = e->Normal();

		if( edgeNormal.x > 0 )
		{
			corner.x = left;
		}
		else if( edgeNormal.x < 0 )
		{
			corner.x = right;
		}
		else
		{
			if( edgeLeft <= left )
				corner.x = left;
			else if ( edgeRight >= right )
				corner.x = right;
			else
			{
				corner.x = (edgeLeft + edgeRight) / 2;
			}
			//aabb
			//cout << "this prob" << endl;
		}

		if( edgeNormal.y > 0 )
		{
			corner.y = top;
		}
		else if( edgeNormal.y < 0 )
		{
			corner.y = bottom;
		}
		else
		{
			//aabb
			if( edgeTop <= top )
			corner.y = top;
			else if ( edgeBottom >= bottom )
				corner.y = bottom;
			else
				corner.y = (edgeTop+ edgeBottom) / 2;
			//else
			//cout << "this 2" << endl;
		}

		int res = cross( corner - e->v0, e->v1 - e->v0 );

		double measureNormal = dot( edgeNormal, normalize(-a.velocity) );

		if( res < 0 )// && ( a.velocity.x != 0 || a.velocity.y != 0 )  )	
		{
			
			Vector2f invVel = normalize(-a.velocity);



			LineIntersection li = lineIntersection( corner, corner - (a.velocity), e->v0, e->v1 );

			//assert( li.parallel == false );
			if( li.parallel )
			{
				return NULL;
			}
			Vector2f intersect = li.position;

			float intersectQuantity = e->GetQuantity( intersect );

			if( intersectQuantity < 0 )
			{
				cout << "under: " << e->v0.x << ", " << e->v0.y << endl;
				float minx = min( intersect.x, corner.x );
				float maxx = max( intersect.x, corner.x );
				float miny = min( intersect.y, corner.y );
				float maxy = max( intersect.y, corner.y );

				LineIntersection vertical = lineIntersection( intersect, corner, e->v0, Vector2f( e->v0.x, e->v0.y - 1 ) );
			//	intersect = lii.position;

				LineIntersection horizontal = lineIntersection( intersect, corner, e->v0, Vector2f( e->v0.x-1, e->v0.y ) );
			//	intersect = lii1.position;

				float verticalDist = dot( vertical.position - corner, intersect - corner );//length( lii.position - corner )* dot(normalize(lii.position - corner), normalize( intersect - corner )) ;
				float horizontalDist =dot( horizontal.position - corner, intersect - corner ); //length( lii1.position - corner )* dot(normalize(lii1.position - corner), normalize( intersect - corner )) ;

//				if( distanceI <= 0 && distanceI1 <= 0 )
//					assert( false && "bbbbsdfdf" );
				bool vertOK = false;
				bool horiOK = false;
				if( !vertical.parallel && verticalDist >= 0 )// && length(vertical.position - corner) < length( intersect - corner ) )
				{
					vertOK = true;
				}

				if( !horizontal.parallel && horizontalDist >= 0 )//&& length(horizontal.position - corner) < length( intersect - corner ))
				{
					horiOK = true;
				}
				if( vertOK && horiOK ) 
				{
					if( verticalDist <= horizontalDist)//length( vertical.position - corner ) < length( horizontal.position - corner ) )
					{
						intersect = vertical.position;
					}
					else
					{
						intersect = horizontal.position;
					}
					//intersect = lii.position;
				//	cout << "case 0" << endl;
				}
				else if( vertOK )
				{
					intersect = vertical.position;
				//	cout << "case 1" << endl;
				}
				else if( horiOK )
				{
					//cout << "case 2" << endl;
					intersect = horizontal.position;
				}
				else
				{
				//	cout << "case failure" << endl;
					//return NULL;
				//	assert( false && "case error" );
				}
			}
			else if( intersectQuantity > length( e->v1 - e->v0 ) )
			{

				cout << "over: " << e->v0.x << ", " << e->v0.y << endl;
				float minx = min( intersect.x, corner.x );
				float maxx = max( intersect.x, corner.x );
				float miny = min( intersect.y, corner.y );
				float maxy = max( intersect.y, corner.y );

				LineIntersection vertical = lineIntersection( intersect, corner, e->v1, Vector2f( e->v1.x, e->v1.y - 1 ) );
			//	intersect = lii.position;

				LineIntersection horizontal = lineIntersection( intersect, corner, e->v1, Vector2f( e->v1.x-1, e->v1.y ) );
			//	intersect = lii1.position;

				float verticalDist = dot( vertical.position - corner, intersect - corner );//length(lii.position - corner) * dot(normalize(lii.position - corner), normalize( intersect - corner )) ;
				float horizontalDist = dot( horizontal.position - corner, intersect - corner );//length( lii1.position - corner ) * dot(normalize(lii1.position - corner), normalize( intersect - corner )) ;

			//	if( distanceI < 0 && distanceI1 < 0 )
				//	assert( false && "bbbbsdfdf" );
				bool vertOK = false;
				bool horiOK = false;
				if( !vertical.parallel && verticalDist >= 0 )// && length(vertical.position - corner) < length( intersect - corner ) )
				{
					vertOK = true;
				}

				if( !horizontal.parallel && horizontalDist >= 0 )//&& length(horizontal.position - corner) < length( intersect - corner ))
				{
					horiOK = true;
				}
				if( vertOK && horiOK ) 
				{
					if( verticalDist <= horizontalDist )//length( vertical.position - corner ) < length( horizontal.position - corner ) )
					{
						intersect = vertical.position;
					}
					else
					{
						intersect = horizontal.position;
					}
					//intersect = lii.position;
				//	cout << "case 0" << endl;
				}
				else if( vertOK )
				{
					intersect = vertical.position;
				//	cout << "case 1" << endl;
				}
				else if( horiOK )
				{
					//cout << "case 2" << endl;
					intersect = horizontal.position;
				}
				else
				{
					cout << "case failure" << endl;
					//return NULL;
				//	assert( false && "case error" );
				}

			
			}
			if( dot( normalize( -a.velocity ), normalize(intersect - corner ) ) < .99 )
			{
				return NULL;
			}
			double pri = dot( intersect - ( a.position - a.velocity ), normalize( a.velocity ) );
				//cross( (corner - a.velocity) - e->v0, normalize( e->v1 - e->v0 ) );
			//double pri = -cross( normalize((corner) - e->v0), normalize( e->v1 - e->v0 ) );
			//double pri = length( intersect - (corner - a.velocity ) );
			//cout << "pri: " << pri <<" .... " << e->v0.x << ", " << e->v0.y << " .. " << e->v1.x << ", " << e->v1.y << endl;
			if( pri < -1 )
			{
				cout << "BUSTED--------------- " << pri  << endl;
				return NULL;
			}

			intersectQuantity = e->GetQuantity( intersect );
			//cout << "intersectQuantity2222: " << intersectQuantity << endl;
			

			//cs.setPosition( bottomLeft - invVel );
			//window->draw( cs );

			//Vector2f p = intersect - corner;
			currentContact->position = intersect;
			currentContact->resolution = intersect - corner;
			currentContact->collisionPriority = pri;//dot(intersect - ( corner + invVel), e->Normal());
			currentContact->edge = e;

			return currentContact;
			//a.position = a.position + p;
		}

		
	}

	return NULL;
}

Contact *collideeEdge( Actor &a, const CollisionBox &b, Edge *e )
{
	Vector2f oldPosition = a.position - a.velocity;
	float left = a.position.x + b.offset.x - b.rw;
	float right = a.position.x + b.offset.x + b.rw;
	float top = a.position.y + b.offset.y - b.rh;
	float bottom = a.position.y + b.offset.y + b.rh;

	float oldLeft = oldPosition.x + b.offset.x - b.rw;
	float oldRight = oldPosition.x + b.offset.x + b.rw;
	float oldTop = oldPosition.y + b.offset.y - b.rh;
	float oldBottom = oldPosition.y + b.offset.y + b.rh;


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
				if( bottom > e->v0.y && bottom < e->v1.y )
				{
					Vector2f bottomLeft( left, bottom);
					corner = bottomLeft;
				}
				else if( top < e->v1.y && top > e->v0.y )
				{
					Vector2f topLeft( left, top );
					corner = topLeft;
				}
				else if( e->v0.y < top && e->v1.y > bottom )
				{
					//Vector2f bottomLeft( left, bottom);
					//corner = bottomLeft;
					//corner = Vector2f( left, (top + bottom) / 2 );
					Vector2f topLeft( left, top );
					corner = topLeft;
				}
				else
				{
				
					//corner = Vector2f(left, (e->v0.y + e->v1.y)/2);
					Vector2f bottomLeft( left, bottom);
					corner = bottomLeft;
				}

				//Vector2f topLeft( top, left );
				

				//corner = Vector2f( left, (top + bottom) / 2 );
			//	corner = bottomLeft;
				resolve = true;
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
				if( bottom < e->v0.y && bottom > e->v1.y )
				{
					Vector2f bottomRight( right, bottom);
					corner = bottomRight;
				}
				else if( top < e->v0.y && top > e->v1.y )
				{
					Vector2f topRight( right, top );
					corner = topRight;
				}
				else if( bottom < e->v0.y && top > e->v1.y)
				{
					Vector2f bottomRight( right, bottom);
					corner = bottomRight;
					//corner = Vector2f( right, (top + bottom) / 2 );
				}
				else
				{
					Vector2f bottomRight( right, bottom);
					corner = bottomRight;
					cout << "WARNNINGGGGGGGGGGGGGGGGGGGG" << endl;
					//corner = Vector2f(right, (e->v0.y + e->v1.y)/2);
				}
		//		Vector2f topRight( right, top );
			//	Vector2f bottomRight( right, bottom );
				//corner = (topRight + bottomRight) / 2;
				//corner = Vector2f( right, bottom );
				//corner = Vector2f( right, (top + bottom) / 2);
				resolve = true;
			//	resolve = true;
			}
		}
		else
		{
			//up and down
			if( edgeNormal.y > 0 )
			{
				if( left > e->v1.x && left < e->v0.x )
				{
					Vector2f topLeft( left, top);
					corner = topLeft;
				}
				else if( right > e->v1.x && right < e->v0.x )
				{
					Vector2f topRight( right, top);
					corner = topRight;
				}
				else if( e->v1.x < left && e->v0.x > right)
				{
					Vector2f topLeft( left, top);
					corner = topLeft;
				}
				else
				{
					Vector2f topLeft( left, top);
					corner = topLeft;
						cout << "WARNNINGGGGGGGGGGGGGGGGGGGG" << endl;
					//corner = Vector2f((e->v0.x + e->v1.x)/2, top);
				}
				//Vector2f topLeft( left, top );
				//Vector2f bottomRight( right, bottom );
				//Vector2f topRight( right, top );
				//corner = Vector2f( (left + right) / 2, top );
			//	corner = topRight;
				resolve = true;
				cout << "up" << endl;
			}
			else if( edgeNormal.y < 0 )
			{
				if( left > e->v0.x && left < e->v1.x )
				{
					Vector2f bottomLeft( left, bottom);
					corner = bottomLeft;
				}
				else if( right < e->v1.x && right > e->v0.x )
				{
					Vector2f bottomRight( right, bottom);
					corner = bottomRight;
				}
				else if( e->v0.x < left && e->v1.x > right)
				{
					Vector2f bottomLeft( left, bottom);
					corner = bottomLeft;
					//corner = Vector2f( (left + right) / 2, bottom );
				}
				else
				{
					Vector2f bottomLeft( left, bottom);
					corner = bottomLeft;
						cout << "WARNNINGGGGGGGGGGGGGGGGGGGG" << endl;
					//corner = Vector2f((e->v0.x + e->v1.x)/2, bottom);
				}
		
				//corner = Vector2f( (left + right) / 2, bottom );
				resolve = true;
				cout << "down" << endl;
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

		double measureNormal = dot( e->Normal(), a.velocity );
		if( res == 0 )
		{
			//cout << "STUPID" << endl;
		}
		if( measureNormal > 0 )
		{
			//cout << e->Normal().x << ", " << e->Normal().y << endl;
		}
		//if( res < 0 && resolve && measureNormal < 0 && ( a.velocity.x != 0 || a.velocity.y != 0 )  )
		if( res < 0 && resolve && measureNormal < 0 && ( a.velocity.x != 0 || a.velocity.y != 0 )  )
		{
			cout << "starting this" << endl;
			Vector2f invVel = normalize(-a.velocity);

			//double pri = -cross( normalize((corner - normalize(a.velocity)) - e->v0), normalize( e->v1 - e->v0 ) );
			//if( pri <= 0 )
			//	return NULL;

			Vector2f norm = e->Normal();
			LineIntersection li = lineIntersection( corner, corner - (a.velocity), e->v0, e->v1 );
			assert( li.parallel == false );
			Vector2f intersect = li.position;
			float intersectQuantity = e->GetQuantity( intersect );
			//cout << "intersectQuantity: " << intersectQuantity << endl;

		//	if( intersect.x < edgeLeft )
		//		intersect.x = edgeLeft;
		//	else if( intersect.x > edgeRight )
		//		intersect.x = edgeRight;
			if( intersectQuantity < 0 )
			{
				float minx = min( intersect.x, corner.x );
				float maxx = max( intersect.x, corner.x );
				float miny = min( intersect.y, corner.y );
				float maxy = max( intersect.y, corner.y );

				LineIntersection lii = lineIntersection( intersect, corner, e->v0, Vector2f( e->v0.x, e->v0.y - 1 ) );
			//	intersect = lii.position;

				LineIntersection lii1 = lineIntersection( intersect, corner, e->v0, Vector2f( e->v0.x-1, e->v0.y ) );
			//	intersect = lii1.position;

				float distanceI = dot( lii.position - corner, intersect - corner );//length( lii.position - corner )* dot(normalize(lii.position - corner), normalize( intersect - corner )) ;
				float distanceI1 =dot( lii1.position - corner, intersect - corner ); //length( lii1.position - corner )* dot(normalize(lii1.position - corner), normalize( intersect - corner )) ;

//				if( distanceI <= 0 && distanceI1 <= 0 )
//					assert( false && "bbbbsdfdf" );
				bool diOk = false;
				bool di1Ok = false;
				if( !lii.parallel && length(lii.position - corner) < length( intersect - corner ) )
				{
					diOk = true;
				}

				if( !lii1.parallel && length(lii1.position - corner) < length( intersect - corner ))
				{
					di1Ok = true;
				}
				if( diOk && di1Ok ) 
				{
					if( length( lii.position - corner ) < length( intersect - corner ) )
					{
						intersect = lii.position;
					}
					else
					{
						intersect = lii1.position;
					}
					//intersect = lii.position;
					cout << "case 0" << endl;
				}
				else if( diOk )
				{
					intersect = lii.position;
					cout << "case 1" << endl;
				}
				else if( di1Ok )
				{
					cout << "case 2" << endl;
					intersect = lii1.position;
				}
				else
				{
					cout << "case failure" << endl;
					return NULL;
				//	assert( false && "case error" );
				}

				/*if( e->v0.x >= minx && e->v0.x <= maxx )
				{
					LineIntersection lii = lineIntersection( intersect, corner, e->v0, Vector2f( e->v0.x, e->v0.y - 1 ) );
					intersect = lii.position;
					if( lii.parallel )
				{
					cout << "-----------" << endl;
				}
				}
				else if( e->v0.y >= miny && e->v0.y <= maxy )
				{
					
					LineIntersection lii = lineIntersection( intersect, corner, e->v0, Vector2f( e->v0.x-1, e->v0.y ) );
					intersect = lii.position;
					if( lii.parallel )
				{
					cout << "-----------" << endl;
				}
				}
				else
				{
					assert( false );
				}*/
				//Vector2f zz = normalize( e->v0 - corner );
				//Vector2f yy = normalize( intersect - corner );
				//intersect = e->v0 - dot( zz, yy ) * (intersect - corner);
				//cout << "a" << endl;
				//intersect = e->v0;
				
				cout << "YOOO: " << e->Normal().x << ", " << e->Normal().y << endl;
				//if( lii.parallel )
				//{
				//	cout << "88888888888888" << endl;
				//}
			}
			else if( intersectQuantity > length( e->v1 - e->v0 ) )
			{
				cout << " not yo: " << e->Normal().x << ", " << e->Normal().y << endl;
				//float r = cross(normalize(intersect - corner), normalize(e->v1 - corner) );
				//intersect += 
				//float slope = (e->v1.y - e->v0.y) / (e->v1.x - e->v0.x );
				//Vector2f ss = intersect - e->v1;
				/*LineIntersection lii = lineIntersection( intersect, corner, e->v1, Vector2f( e->v1.x, e->v1.y - 1 ) );
				if( lii.parallel )
				{
					cout << "-----------" << endl;
				}
				intersect = lii.position;*/

				float minx = min( intersect.x, corner.x );
				float maxx = max( intersect.x, corner.x );
				float miny = min( intersect.y, corner.y );
				float maxy = max( intersect.y, corner.y );

				LineIntersection lii = lineIntersection( intersect, corner, e->v1, Vector2f( e->v1.x, e->v1.y - 1 ) );
			//	intersect = lii.position;

				LineIntersection lii1 = lineIntersection( intersect, corner, e->v1, Vector2f( e->v1.x-1, e->v1.y ) );
			//	intersect = lii1.position;

				float distanceI = dot( lii.position - corner, intersect - corner );//length(lii.position - corner) * dot(normalize(lii.position - corner), normalize( intersect - corner )) ;
				float distanceI1 = dot( lii1.position - corner, intersect - corner );//length( lii1.position - corner ) * dot(normalize(lii1.position - corner), normalize( intersect - corner )) ;

			//	if( distanceI < 0 && distanceI1 < 0 )
				//	assert( false && "bbbbsdfdf" );
				bool diOk = false;
				bool di1Ok = false;
				if( !lii.parallel && length(lii.position - corner) < length( a.velocity ))
				{
					diOk = true;
				}

				if( !lii1.parallel && length(lii1.position - corner) < length( a.velocity ) )
				{
					di1Ok = true;
				}
				if( diOk && di1Ok ) 
				{
					if( length( lii.position - corner ) < length( intersect - corner ) )
					{
						intersect = lii.position;
					}
					else
					{
						intersect = lii1.position;
					}
					//intersect = lii.position;
					cout << "case 0" << endl;
				}
				else if( diOk )
				{
					intersect = lii.position;
					cout << "case 1" << endl;
				}
				else if( di1Ok )
				{
					cout << "case 2" << endl;
					intersect = lii1.position;
				}
				else
				{
							cout << "case failure not yo" << endl;
					return NULL;
					//assert( false && "case error" );
				}

				//if( distanceI < 0 )
				//	intersect = lii1.position;
				//else if( distanceI1 < 0 )
			//		intersect = lii.position;
				//intersect = lii1.position;
				/*if( e->v0.x > minx && e->v0.x < maxx )
				{
					intersect = lii1.position;
					if( lii.parallel )
					{
						cout << "-----------" << endl;
					}
				}
				else if( e->v0.y > miny && e->v0.y < maxy )
				{
					intersect = lii.position;
					if( lii1.parallel )
					{
					cout << "-----------" << endl;
					}
				}*/
			
				if( distanceI >= distanceI1 )
				{
			//		intersect = lii1.position;
				}
				else if( distanceI < distanceI1)
				{
				//	intersect = lii.position;
				}
				else
				{

					//assert( false && "stupid" );
				}


				//Vector2f zz = normalize( e->v1 - corner );
				//Vector2f yy = normalize( intersect - corner );
				//intersect = e->v1 - dot( zz, yy ) * (e->v1 - corner);
				//cout << "bb" << endl;
				//intersect = e->v1;
			}
			double pri = -cross( normalize((corner - a.velocity) - e->v0), normalize( e->v1 - e->v0 ) );
			//double pri = -cross( normalize((corner) - e->v0), normalize( e->v1 - e->v0 ) );
			//double pri = length( intersect - (corner - a.velocity ) );
			cout << "pri: " << pri <<" .... " << e->v0.x << ", " << e->v0.y << " .. " << e->v1.x << ", " << e->v1.y << endl;
			if( pri < 0 )
			{
				cout << "BUSTED---------------" << endl;
				//return NULL;
			}

			intersectQuantity = e->GetQuantity( intersect );
			//cout << "intersectQuantity2222: " << intersectQuantity << endl;
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
			currentContact->collisionPriority = pri;//dot(intersect - ( corner + invVel), e->Normal());
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
		sf::VideoMode( 1920 / 1, 1080 / 1), "Breakneck", sf::Style::Default, sf::ContextSettings( 0, 0, 0, 0, 0 ));
	//window->setPosition( Vector2i(800, 0 ));
	sf::Vector2i pos( 0, 0 );

	//window->setPosition( pos );
	window->setVerticalSyncEnabled( true );
	//window->setFramerateLimit( 60 );
	window->setMouseCursorVisible( true );
	
	View view( sf::Vector2f(  300, 300 ), sf::Vector2f( 960, 540 ) );
	window->setView( view );

	EditSession es(window );
	es.Run( "test1" );

	window->setVerticalSyncEnabled( true );

	currentContact = new Contact;

	
	sf::RectangleShape bDraw;
	bDraw.setFillColor( Color::Red );
	bDraw.setSize( sf::Vector2f(32 * 2, 32 * 2) );
	bDraw.setOrigin( bDraw.getLocalBounds().width /2, bDraw.getLocalBounds().height / 2 );

	ifstream is;
	is.open( "test1.brknk" );

	int numPoints;
	is >> numPoints;
	Vector2f *points = new Vector2f[numPoints];
	list<Vector2f> lvec;

	Actor player;
	//player.position = Vector2f( -500, 300 );

	is >> player.position.x;
	is >> player.position.y;

	int pointsLeft = numPoints;

	int pointCounter = 0;

	Edge **edges = new Edge*[numPoints];
	while( pointCounter < numPoints )
	{
		string matStr;
		is >> matStr;
		int polyPoints;
		is >> polyPoints;

		int currentEdgeIndex = pointCounter;
		for( int i = 0; i < polyPoints; ++i )
		{
			float px, py;
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

		

			if( i == 0 )
			{
				ee->edge0 = edges[currentEdgeIndex + polyPoints - 1];
				ee->edge1 = edges[currentEdgeIndex + 1];
			}
			else if( i == polyPoints - 1 )
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
		}
	}
	
	//load points
	/*for( int i = 0; i < numPoints; ++i )
	{
		float px, py;
		is >> px;
		is >> py;
			
		points[i].x = px;
		points[i].y = py;
	}
	
	int totalEdges = numPoints;
	int numPolygons;

	is >> numPolygons;*/

	
	
	
	
	//int currentEdgeIndex = 0;

	//start loop

	/*for( int n = 0; n < numPolygons; ++n )
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
	}*/
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
	sf::Vertex *line = new sf::Vertex[numPoints*2];
	for( int i = 0; i < numPoints; ++i )
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

	

	sf::Clock gameClock;
	double currentTime = 0;
	double accumulator = TIMESTEP + .1;

	Vector2f otherPlayerPos;
	

	

	while( true )
	{
		double newTime = gameClock.getElapsedTime().asSeconds();
		double frameTime = newTime - currentTime;

		if ( frameTime > 0.25 )
			frameTime = 0.25;	
        currentTime = newTime;

		accumulator += frameTime;

		
		while ( accumulator >= TIMESTEP  )
        {
			window->clear();
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
			float xkl = 1;
			for( int jkl = 0; jkl < xkl; ++jkl )
			{
			player.position += player.velocity / xkl;
			float minPriority = 1000000;
			Edge *minEdge = NULL;
			Vector2f res(0,0);
			int collisionNumber = 0;
			for( int i = 0; i < numPoints; ++i )
			{
				Contact *c = collideEdge( player, b, edges[i] );
				if( c != NULL )
				{
					collisionNumber++;
				//player.position += c->resolution;
				if( c->collisionPriority <= minPriority )
				{
					if( c->collisionPriority == minPriority )
					{
						if( length(c->resolution) > length(res) )
						{
							minPriority = c->collisionPriority;
							minEdge = edges[i];
							res = c->resolution;
						}
					}
					else
					{

						minPriority = c->collisionPriority;
						minEdge = edges[i];
						res = c->resolution;
					}
				}
				
				//cout << "p: " << c->collisionPriority << endl;
				//cout << "resolution: " << c->resolution.x << ", " << c->resolution.y << endl;

				//break;
				}
			}


			cout << "collisionNumber: " << collisionNumber << endl;
			if( minEdge != NULL )
			{
				
				Contact *c = collideEdge( player, b, minEdge );
				//cout << "priority at: " << minEdge->Normal().x << ", " << minEdge->Normal().y << ": " << minPriority << endl;
				otherPlayerPos = player.position;
				player.position += c->resolution;

				sf::CircleShape cs;
				cs.setFillColor( Color::Magenta );
				
				cs.setRadius( 20 );
				cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
				cs.setPosition( c->position );
				window->draw( cs );
				//cout << "resolution: " << c->resolution.x << ", " << c->resolution.y << endl;
				//cout << "cpos: " << c->position.x << ", " << c->position.y << endl;
				Color lc = Color::Green;
				sf::Vertex linez[] =
				{
					sf::Vertex(sf::Vector2f(minEdge->v0.x + 1, minEdge->v0.y + 1), lc),
					sf::Vertex(sf::Vector2f(minEdge->v1.x + 1, minEdge->v1.y + 1), lc),
					sf::Vertex(sf::Vector2f(minEdge->v0.x - 1, minEdge->v0.y - 1), lc),
					sf::Vertex(sf::Vector2f(minEdge->v1.x - 1, minEdge->v1.y - 1), lc)
				};

				window->draw(linez, 4, sf::Lines);
				 
			}
			}
			

			player.UpdatePostPhysics();

			//r.setPosition( player.position.x - r.getLocalBounds().width / 2, player.position.y - r.getLocalBounds().height / 2 );
			//cout << "playerPos: " << player.position.x << ", " << player.position.y << endl;	
		

			accumulator -= TIMESTEP;
			while( sf::Keyboard::isKeyPressed( sf::Keyboard::Space ) )
		{
		}
		}

		view.setSize( Vector2f( 960 * 1, 540 * 1 ) );
		view.setCenter( player.position );
		window->setView( view );

		bDraw.setPosition( player.position );
		window->draw( bDraw );

		window->draw( *(player.sprite) );
		sf::RectangleShape rs;
		rs.setSize( Vector2f(64, 64) );
		rs.setOrigin( rs.getLocalBounds().width / 2, rs.getLocalBounds().height / 2 );
		rs.setPosition( otherPlayerPos );
		rs.setFillColor( Color::Blue );
		window->draw( circle );
		window->draw(line, numPoints * 2, sf::Lines);

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

