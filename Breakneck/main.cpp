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

#define TIMESTEP 1.0 / 60.0

using namespace std;
using namespace sf;

RenderWindow *window;


GameController controller(0);
ControllerState prevInput;
ControllerState currInput;


struct Edge
{
	Vector2<double> v0;
	Vector2<double> v1;
	Edge * GetEdge0();
	Edge * GetEdge1();
	Edge *edge0;
	Edge *edge1;

	//material ---
	Edge()
	{
		edge0 = NULL;
		edge1 = NULL;
	}

	Vector2<double> Normal()
	{
		Vector2<double> v = v1 - v0;
		Vector2<double> temp = normalize( v );
		return Vector2<double>( temp.y, -temp.x );
	}

	Vector2<double> GetPoint( double quantity )
	{
		//gets the point on a line w/ length quantity in the direction of the edge vector
		Vector2<double> e( v1 - v0 );
		e = normalize( e );
		return v0 + quantity * e;
	}

	

	double GetQuantity( Vector2<double> p )
	{
		//projects the origin of the line to p onto the edge. if the point is on the edge it will just be 
		//normal to use dot product to get cos(0) =1
		Vector2<double> vv = p - v0;
		Vector2<double> e = normalize(v1 - v0);
		return dot( vv, e );
	}

	double GetQuantityGivenX( double x )
	{

		Vector2<double> e = normalize(v1 - v0);
		double deltax = x - v0.x;
		double factor = deltax / e.y;
	}
};

#define V2d sf::Vector2<double>

struct CollisionBox
{

	enum BoxType
	{
		Physics,
		Hit,
		Hurt
	};

	Vector2<double> offset;
	double offsetAngle;
	
	double rw; //radius or half width
	double rh; //radius or half height
	bool isCircle;
	BoxType type;

	Vector2<double> GetAxis1( Vector2<double> actorPos )
	{
	/*	double left = actorPos.x + offset.x - rw;
		double right = actorPos.x + offset.x + rw;
		double top = actorPos.y + offset.y - rh;
		double bottom = actorPos.y + offset.y + rh;
		Vector2<double> topLeft( left, top );
		Vector2<double> topRight( right, top );
		Vector2<double> bottomLeft( left, bottom );

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
		
	double collisionPriority;	
	Vector2<double> position;
	Vector2<double> resolution;
	Edge *edge;
};

bool approxEquals( double a, double b )
{
	return abs( a - b ) < .00001;
}

struct Tileset
{
	IntRect GetSubRect( int localID )
	{
		int xi,yi;
		xi = localID % (texture->getSize().x / tileWidth );
		yi = localID / (texture->getSize().x / tileWidth );
		

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

struct Actor;
Contact * collideEdge( const Actor &a, const CollisionBox &b, Edge * e, const V2d &vel );
struct Actor
{
	enum Action
	{
		STAND,
		RUN,
		DASH,
		SLIDE,
		WALLCLING,
		JUMP,
		LAND,
		Count
	};


	Shader sh;
	bool collision;
	Sprite *sprite;
	Tileset *tilesetStand;
	Tileset *tilesetRun;
	Tileset *tilesetDash;
	Tileset *tilesetSlide;
	Tileset *tilesetWallcling;
	Tileset *tilesetJump;
	Tileset *tilesetLand;
	CollisionBox b;
	
	Edge *ground;
	int numActiveEdges;
	Edge ** activeEdges;
	double edgeQuantity;
	int actionLength[Action::Count]; //actionLength-1 is the max frame counter for each action
	double groundOffsetX;
	double offsetX;

	double groundSpeed;
	double maxNormalRun;
	double runAccel;
	double maxFallSpeed;
	double gravity;
	bool facingRight;
	double jumpStrength;
	double airAccel;
	double maxAirXSpeed;
	double maxAirXSpeedNormal;

	Actor::Actor()
	{
		activeEdges = new Edge*[16]; //this can probably be really small I don't think it matters. 
		numActiveEdges = 0;

		//assert( Shader::isAvailable() && "help me" );
		/*const std::string fragmentShader = \
			"void main()" \
			"{" \
			"    ..." \
			"}";

		assert( Shader::isAvailable() && "help me" );
		if (!sh.loadFromFile("player_shader.frag", sf::Shader::Fragment))
		//if (!sh.loadFromMemory(fragmentShader, sf::Shader::Fragment))
		{
			cout << "PLAYER SHADER NOT LOADING CORRECTLY" << endl;
			assert( 0 && "player shader not loaded" );
		}*/

		offsetX = 0;
		sprite = new Sprite;
		velocity = Vector2<double>( 0, 0 );
		actionLength[STAND] = 18 * 8;
		tilesetStand = GetTileset( "stand.png", 64, 64 );

		actionLength[RUN] = 10 * 4;
		tilesetRun = GetTileset( "run.png", 128, 64 );

		actionLength[JUMP] = 2;
		tilesetJump = GetTileset( "jump.png", 64, 64 );

		actionLength[LAND] = 1;
		tilesetLand = GetTileset( "land.png", 64, 64 );

		actionLength[DASH] = 120;
		tilesetDash = GetTileset( "dash.png", 64, 64 );

		actionLength[WALLCLING] = 1;
		tilesetWallcling = GetTileset( "wallcling.png", 64, 64 );

		actionLength[SLIDE] = 1;
		tilesetSlide = GetTileset( "slide.png", 64, 64 );		

		action = JUMP;
		frame = 1;

		gravity = 2;
		maxFallSpeed = 30;

		ground = NULL;
		groundSpeed = 0;
		maxNormalRun = 100;
		runAccel = 2;
		facingRight = true;
		collision = false;
		jumpStrength = 30;
		airAccel = 2;
		maxAirXSpeed = 30;
		maxAirXSpeedNormal = 10;
		groundOffsetX = 0;

		//CollisionBox b;
		b.isCircle = false;
		b.offsetAngle = 0;
		b.offset.x = 0;
		b.offset.y = 0;
		b.rw = 10;
		b.rh = 32;
		b.type = b.Physics;
	}

	void ActionEnded()
	{
		if( frame >= actionLength[action] )
		{
			switch( action )
			{
			case STAND:
				frame = 0;
				break;
			case RUN:
				frame = 0;
			case JUMP:
				frame = 1;
				break;
			case LAND:
				frame = 0;
				break;
			}
		}
	}

	void UpdatePrePhysics()
	{
		ActionEnded();


		switch( action )
		{
		case STAND:
			if( currInput.A && !prevInput.A )
			{
				action = JUMP;
				frame = 0;
				break;
			}
			if( currInput.Left() || currInput.Right() )
			{
				action = RUN;
				frame = 0;
				break;
			}
			
			break;
		case RUN:
			if( currInput.A && !prevInput.A )
			{
				action = JUMP;
				frame = 0;
				break;
			}
			if(!( currInput.Left() || currInput.Right() ))
			{
				action = STAND;
				frame = 0;
				break;
			}
			break;
		case JUMP:
			break;
		case LAND:
			if( currInput.Left() || currInput.Right() )
			{
				action = RUN;
				frame = 0;
			}
			else
			{
				action = STAND;
				frame = 0;
			}

			break;
		}

		switch( action )
		{
		case STAND:
			groundSpeed = 0;
			break;
		case RUN:
			if( currInput.Left() )
			{
				if( groundSpeed > 0 )
				{
					groundSpeed = 0;
				}
				else
				{
					groundSpeed -= runAccel;
					if( groundSpeed < -maxNormalRun )
						groundSpeed = -maxNormalRun;
				}
				facingRight = false;
			}
			else if( currInput.Right() )
			{
				if (groundSpeed < 0 )
					groundSpeed = 0;
				else
				{
					groundSpeed += runAccel;
					if( groundSpeed > maxNormalRun )
						groundSpeed = maxNormalRun;
				}
				facingRight = true;
			}

			break;
		case JUMP:
			if( frame == 0 )
			{
				if( ground != NULL ) //this should always be true but we haven't implemented running off an edge yet
				{
					velocity = groundSpeed * normalize(ground->v1 - ground->v0 );
					if( velocity.y > 0 )
						velocity.y = 0;
					velocity.y -= jumpStrength;
					ground = NULL;
				}
			}
			else
			{
				
				if( currInput.Left() )
				{
					//if( !( velocity.x > -maxAirXSpeedNormal && velocity.x - airAccel < -maxAirXSpeedNormal ) )
					//{
						velocity.x -= airAccel;
					//}
					
				}
				else if( currInput.Right() )
				{
					//if( !( velocity.x < maxAirXSpeedNormal && velocity.x + airAccel > maxAirXSpeedNormal ) )
					//{
						velocity.x += airAccel;
					//}
				}
			}
			break;
		}


		if( velocity.x > maxAirXSpeed )
			velocity.x = maxAirXSpeed;
		else if( velocity.x < -maxAirXSpeed )
			velocity.x = -maxAirXSpeed;

		velocity += V2d( 0, gravity );
		if( velocity.y > maxFallSpeed )
			velocity.y = maxFallSpeed;
	//	cout << "position: " << position.x << ", " << position.y << endl;
	//	cout << "velocity: " << velocity.x << ", " << velocity.y << endl;
		collision = false;
	}
	Contact minContact;
	bool ResolvePhysics( Edge** edges, int numPoints, V2d vel )
	{
		position += vel;
		bool col = false;
		int collisionNumber = 0;
			
		minContact.collisionPriority = 1000000;
		for( int i = 0; i < numPoints; ++i )
		{
			/*bool match = false;
			for( int j = 0; j < numActiveEdges; ++j )
			{
				if( edges[i] == activeEdges[j] )
				{
					match = true;
					break;
				}
			}*/

			if( ground == edges[i] )
				continue;

		//	if( match )
		//		continue;

			Contact *c = collideEdge( *this , b, edges[i], vel );
			if( c != NULL )
			{
				collisionNumber++;
				if( c->collisionPriority <= minContact.collisionPriority || minContact.collisionPriority < -1 )
				{	
					if( c->collisionPriority == minContact.collisionPriority )
					{
						if( length(c->resolution) > length(minContact.resolution) )
						{
							minContact.collisionPriority = c->collisionPriority;
							minContact.edge = edges[i];
							minContact.resolution = c->resolution;
							minContact.position = c->position;
							col = true;
						}
					}
					else
					{

						minContact.collisionPriority = c->collisionPriority;
						minContact.edge = edges[i];
						minContact.resolution = c->resolution;
						minContact.position = c->position;
						col = true;
					}
				}
			}
		}


		//cout << "collisionNumber: " << collisionNumber << endl;
		if( false )
		{
				

		//	Contact *c = collideEdge( *this, b, minEdge );
			//cout << "priority at: " << minEdge->Normal().x << ", " << minEdge->Normal().y << ": " << minPriority << endl;
		//	assert( res.x == c->resolution.x && res.y == c->resolution.y );
			position += minContact.resolution;//c->resolution;

			sf::CircleShape cs;
			cs.setFillColor( Color::Magenta );
				
			cs.setRadius( 20 );
			cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
		//	cs.setPosition( c->position.x, c->position.y );
			cs.setPosition( minContact.position.x, minContact.position.y );
			window->draw( cs );
			Color lc = Color::Green;
			sf::Vertex linez[] =
			{
				sf::Vertex(sf::Vector2<float>(minContact.edge->v0.x + 1, minContact.edge->v0.y + 1), lc),
				sf::Vertex(sf::Vector2<float>(minContact.edge->v1.x + 1, minContact.edge->v1.y + 1), lc),
				sf::Vertex(sf::Vector2<float>(minContact.edge->v0.x - 1, minContact.edge->v0.y - 1), lc),
				sf::Vertex(sf::Vector2<float>(minContact.edge->v1.x - 1, minContact.edge->v1.y - 1), lc)
			};

			window->draw(linez, 4, sf::Lines);
				
			if( minContact.edge->Normal().y < 0 )
			{
				groundOffsetX = (position.x - minContact.position.x) / 2; //halfway?
				ground = minContact.edge;
				edgeQuantity = minContact.edge->GetQuantity( minContact.position );
			}
				

				
		}

		return col;
	}

	bool leftGround;
	void UpdatePhysics( Edge **edges, int numPoints )
	{
		leftGround = false;
		//V2d trueVel = velocity;
		double movement = 0;
		double maxMovement = 5;
		V2d movementVec;

		if( ground != NULL ) movement = groundSpeed;
		else
		{
			//movement = length( velocity );
			movementVec = velocity;
		}

		while( (ground != NULL && movement != 0) || ( ground == NULL && length( movementVec ) > 0 ) )
		{
			double steal = 0;

			if( ground != NULL )
			{
				if( movement > 0 )
				{
					if( movement > maxMovement )
					{
						steal = movement - maxMovement;
						movement = maxMovement;
					}
				}
				else 
				{
					if( movement < -maxMovement )
					{
						steal = movement + maxMovement;
						movement = -maxMovement;
					}
				}


				double extra = 0;
				bool leaveGround = false;
				double q = edgeQuantity;

				V2d gNormal = ground->Normal();


				double m = movement;
			//	double extra;
				double groundLength = length( ground->v1 - ground->v0 ); 

				if( approxEquals( q, 0 ) )
					q = 0;
				else if( approxEquals( q, groundLength ) )
					q = groundLength;


				bool a1 = gNormal.x >= 0 && q == 0;
				bool b1 = gNormal.x <= 0 && q == 0;
				bool c1 = gNormal.x <= 0 && q == groundLength;
				bool d1 = offsetX == -b.rw;
				bool g1 = offsetX == b.rw;
				bool t1 = gNormal.x >= 0 && q == groundLength;
				
				Edge *e0 = ground->edge0;
				Edge *e1 = ground->edge1;
				V2d e0n = e0->Normal();
				V2d e1n = e1->Normal();

				bool cond1right = movement > 0 && offsetX == b.rw && e1->Normal().x > 0;
				bool cond2right = movement > 0 && offsetX == -b.rw && e1->Normal().x < 0;

				bool cond1left = movement < 0 && offsetX == b.rw && e0->Normal().x > 0;
				bool cond2left = movement < 0 && offsetX == -b.rw && e0->Normal().x < 0;				


				//bool f1 = ( movement > 0 && offsetX < b.rw && gNormal.x <= 0 && (e1n.y >= 0 || e1n.x > 0 )  ) || (movement < 0 && offsetX > -b.rw && gNormal.x >= 0 && (e0n.y >= 0 || e0n.x < 0 )  );
				bool f1 = ( movement > 0 && offsetX < b.rw  ) || (movement < 0 && offsetX > -b.rw  );

				bool transferLeft =  q == 0 && movement < 0
					&& ((gNormal.x == 0 && e0->Normal().x == 0 )
					|| ( offsetX == -b.rw && e0->Normal().x <= 0 ) 
					|| (offsetX == b.rw && e0->Normal().x >= 0 && e0->Normal().y != 0 ));
				bool transferRight = q == groundLength && movement > 0 
					&& ((gNormal.x == 0 && e1->Normal().x == 0 )
					|| ( offsetX == b.rw && e1->Normal().x >= 0 )
					|| (offsetX == -b.rw && e1->Normal().x <= 0 && e1->Normal().y != 0 ) );
				bool offsetLeft = movement < 0 && offsetX > -b.rw && ( (q == 0 && e0->Normal().x < 0) || (q == groundLength && gNormal.x < 0) );
				
				bool offsetRight = movement > 0 && offsetX < b.rw && ( ( q == groundLength && e1->Normal().x > 0 ) || (q == 0 && gNormal.x > 0) );
				bool changeOffset = offsetLeft || offsetRight;
				
				//cout << transferLeft << " " << transferRight << " " << changeOffset << endl;
				//cout << "gn: " << gNormal.x << ", " << gNormal.y << endl;
				//cout << "e0n: " << e0n.x << ", " << e0n.y << ", " << offsetX  << endl;
				//cout << "e1n: " << e1n.x << ", " << e1n.y << ", " << offsetX  << endl;
				if( transferLeft )
				{
					if( e0n.y == 0 && e0n.x > 0 )
					{
						offsetX = b.rw;
						cout <<"YEAH" << endl;
						break;
					}

					cout << "b" << endl;
					Edge *next = ground->edge0;
					if( next->Normal().y < 0 )
					{
						ground = next;
						q = length( ground->v1 - ground->v0 );	
					}
					else
					{
						velocity = normalize(ground->v1 - ground->v0 ) * groundSpeed;
						//movement = extra;
						movementVec = normalize( ground->v1 - ground->v0 ) * extra;
						//bool hit = ResolvePhysics( edges, numPoints, normalize( ground->v1 - ground->v0 ) * extra);
						//if( hit )
						//	position += minContact.resolution;
						leftGround = true;
						ground = NULL;
						//movement = 0;
						//dont transfer
						//break;
					}
					//else break;
				}
				else if( transferRight )
				{
					if( e1n.y == 0 && e1n.x < 0)
					{
						cout <<"YEAH2" << endl;
						offsetX = -b.rw;
						break;
					}

					//cout << "a" << endl;
					Edge *next = ground->edge1;
					if( next->Normal().y < 0 )
					{
						ground = next;
						q = 0;

						//cout << "aa" << endl;
						//assert( false );
					}
					else
					{
						velocity = normalize(ground->v1 - ground->v0 ) * groundSpeed;
						
						movementVec = normalize( ground->v1 - ground->v0 ) * extra;
						
						leftGround = true;
						ground = NULL;
					}

				}
				else if( changeOffset || (( gNormal.x == 0 && movement > 0 && offsetX < b.rw ) || ( gNormal.x == 0 && movement < 0 && offsetX > -b.rw ) )  )
				{
					cout << "rep: " << offsetX << " " << gNormal.x << ", " << gNormal.y << endl;
					if( movement > 0 )
						extra = (offsetX + movement) - b.rw;
					else 
					{
						extra = (offsetX + movement) + b.rw;
					}
					double m = movement;
					if( (movement > 0 && extra > 0) || (movement < 0 && extra < 0) )
					{
						m -= extra;
						movement = extra;

						if( movement > 0 )
						{
							offsetX = b.rw;
						}
						else
						{
							offsetX = -b.rw;
						}


					}
					else
					{
						movement = 0;
						offsetX += m;
					}



					if(!approxEquals( m, 0 ) )
					{
						
						cout << "BLAH: " << m << endl;
						
						bool hit = ResolvePhysics( edges, numPoints, V2d( m, 0 ));
						if( hit && (( m > 0 && minContact.edge != ground->edge0 ) || ( m < 0 && minContact.edge != ground->edge1 ) ) )
						{
							V2d eNorm = minContact.edge->Normal();
							if( eNorm.y < 0 )
							{
								if( minContact.position.y >= position.y + b.rh - 5 )
								{
									if( m > 0 && eNorm.x < 0 )
									{
										ground = minContact.edge;
										q = ground->GetQuantity( minContact.position );
										offsetX = -b.rw;
										continue;
									}
									else if( m < 0 && eNorm.x > 0 )
									{
										ground = minContact.edge;
										q = ground->GetQuantity( minContact.position );
										offsetX = b.rw;
										continue;
									}

								}
								else
								{
									offsetX += minContact.resolution.x;
									//cout << "one" << endl;
									break;
								}
							}
							else
							{
								//cout << "two" << endl;
									offsetX += minContact.resolution.x;
									break;
							}
						}
					}

					
					//	position += minContact.resolution;
					

				}
				//else if( 
				else
				{

					//	cout << "q: " << q << endl;
					//cout << offsetX << ", " << movement << " " << gNormal.x << ", " << gNormal.y << " " << q << endl;
					//cout << "a1: " << a1 << " c1 " << c1 << " f1 " << f1 << endl;
					//double steal = 0;
					if( movement > 0 )
					{
						if( movement > maxMovement )
						{
					//		steal = movement - maxMovement;
					//		movement = maxMovement;
						}
						
						extra = (q + movement) - groundLength;
					}
					else 
					{
						if( movement < -maxMovement )
						{
					//		steal = movement + maxMovement;
					//		movement = -maxMovement;
						}
						extra = (q + movement);
					}
					
					if( (movement > 0 && extra > 0) || (movement < 0 && extra < 0) )
					{
						if( movement > 0 )
						{
							q = groundLength;
							//if( gNormal.x == 0 )
							//	q -= b.rw;
							
					//		cout << "zeh" << endl;
						}
						else
						{
							q = 0;
							//if( gNormal.x == 0)
							//	q += 2 * b.rw;
						}
						movement = extra;
						m -= extra;
						
					}
					else
					{
						movement = 0;
						q += m;
					}
				
					if(!approxEquals( m, 0 ) )
					{
					//	cout << "run" << endl;
						
						bool down = true;//(gNormal.x >= 0 && groundSpeed > 0) || (gNormal.x <= 0 && groundSpeed < 0 );
						bool hit = ResolvePhysics( edges, numPoints, normalize( ground->v1 - ground->v0 ) * m);
						if( hit && (( m > 0 && minContact.edge != ground->edge0 ) || ( m < 0 && minContact.edge != ground->edge1 ) ) )
						{
						//	cout << "dfdfdsfd: " << offsetX << " " << groundSpeed << ", " << gNormal.x << ",, " << gNormal.y << "  " << minContact.edge->Normal().x << ", " << minContact.edge->Normal().y << endl;
						//	bool extraCond = m > 0 && gNormal.x > 0 && ground->edge1->Normal().x > 0 && minContact.edge == 
							if( down)
							{
								cout << "errer: " << m << endl;
							V2d eNorm = minContact.edge->Normal();
						//	cout << "zerrer: " << eNorm.x << ", " << eNorm.y << endl;
							if( minContact.position.y > position.y + b.rh - 5 && eNorm.y >= 0 )
							{
								if( minContact.position == minContact.edge->v0 ) 
								{
									if( minContact.edge->edge0->Normal().y <= 0 )
									{
										minContact.edge = minContact.edge->edge0;
										eNorm = minContact.edge->Normal();
							//			cout << "fdfd" << endl;
									}
								}
								else if( minContact.position == minContact.edge->v1 )
								{
									if( minContact.edge->edge1->Normal().y <= 0 )
									{
										minContact.edge = minContact.edge->edge1;
										eNorm = minContact.edge->Normal();
						//				cout << "fsdfdsfdsfdfd" << endl;
									}
								}
							}



							if( eNorm.y < 0 )
							{
								//bool 
								if( minContact.position.y >= position.y + minContact.resolution.y + b.rh - 5 )
								{
									double test = position.x + minContact.resolution.x - minContact.position.x;
									if( test < -b.rw || test > b.rw )
									{
										cout << "BROKEN OFFSET: " << test << endl;
									}
									else
									{
									CircleShape s;
									
									s.setRadius( 10 );
									s.setOrigin( s.getLocalBounds().width / 2, s.getLocalBounds().height / 2 );
									s.setFillColor( Color::White );
									s.setPosition( minContact.position.x, minContact.position.y );
									//window->draw( s );
								//	cout << "glitch " << endl;
									ground = minContact.edge;
									q = ground->GetQuantity( minContact.position );
									V2d eNorm = minContact.edge->Normal();			

									/*if( abs(q - length( ground->v1 - ground->v0 ) ) < .001 )
									{
										q = length( ground->v1 - ground->v0 );
									}
									else if( abs( q ) < .001 )
									{
										q = 0;
									}*/

									offsetX = position.x + minContact.resolution.x - minContact.position.x;
									}

										if( offsetX < -b.rw || offsetX > b.rw )
									{
										cout << "BROKEN OFFSET: " << offsetX << endl;
										assert( false && "T_T" );
									}
									//assert( offsetX >= -b.rw && offsetX <= b.rw );
									//cout << "setting offsetx : " << offsetX << endl;
									//if( eNorm.x > 0 )
									//	offsetX = b.rw;
									//if( eNorm.x < 0 )
									//	offsetX = -b.rw;
								}
								else
								{
							//	cout << "q: " << q << ", new q: " << ground->GetQuantity( ground->GetPoint( q ) + minContact.resolution) << endl;
								//	offsetX = b.rw;
							//	cout << "point: " << minContact.position.x << ", " << minContact.position.y << endl;
							//	cout << "ResL: " << minContact.resolution.x << ", " << minContact.resolution.y << endl;
									q = ground->GetQuantity( ground->GetPoint( q ) + minContact.resolution);
									edgeQuantity = q;
									//q += b.rw;
									break;
								}
							}
							else
							{
								//if( eNorm.y == 0 && eNorm.x < 0 )
								//	offsetX = -b.rw;
								cout << "zzz: " << q << ", " << eNorm.x << ", " << eNorm.y << endl;
								q = ground->GetQuantity( ground->GetPoint( q ) + minContact.resolution);
								edgeQuantity = q;
								break;
							}
							}
							//else if( minContact.position.y < position.y + minContact.resolution.y + b.rh - 5 )
							else
							{
							//else if( 
								cout << "Sdfsdfd" << endl;
								q = ground->GetQuantity( ground->GetPoint( q ) + minContact.resolution);
								edgeQuantity = q;
								break;
							}
						
						}
						
					}
					else
					{
						//q = ground->GetQuantity( ground->GetPoint( q ) + minContact.resolution);
								
						cout << "secret" << endl;
					//	break;
					/*	if( movement > 0 )
							offsetX = -b.rw;
						else
							offsetX = b.rw;
						break;*/
					}
					cout << "adding stolen: " << steal << ", groundspeed: " << groundSpeed << endl;
					//if( steal != 0 )
					//	movement = steal;

					
				}
				if( movement == extra )
					movement += steal;
				else
					movement = steal;
				//if( steal != 0 )
				//	movement = steal;

			//	movement += steal - m;

				edgeQuantity = q;
			}
			else
			{
				collision = ResolvePhysics( edges, numPoints, velocity / 2.0 );
				if( collision )
				{
					position += minContact.resolution;
					V2d extraVel = dot( normalize( velocity ), normalize( minContact.edge->v1 - minContact.edge->v0 ) ) * normalize( minContact.edge->v1 - minContact.edge->v0 ) * length(minContact.resolution);
	
					movementVec = extraVel;
					velocity = dot( normalize( velocity ), normalize( minContact.edge->v1 - minContact.edge->v0 ) ) * normalize( minContact.edge->v1 - minContact.edge->v0 ) * length( velocity );
					cout << "extra vel 1: " << extraVel.x << ", " << extraVel.y << endl;
				}
				else
				{
					collision = ResolvePhysics( edges, numPoints, velocity / 2.0 );
					if( collision )
					{
						position += minContact.resolution;
				
						V2d extraVel = dot( normalize( velocity ), normalize( minContact.edge->v1 - minContact.edge->v0 ) ) * normalize( minContact.edge->v1 - minContact.edge->v0 ) * length(minContact.resolution);

						velocity = dot( normalize( velocity ), normalize( minContact.edge->v1 - minContact.edge->v0 ) ) * normalize( minContact.edge->v1 - minContact.edge->v0 ) * length( velocity );
						movementVec = extraVel;
						cout << "extra vel 2: " << extraVel.x << ", " << extraVel.y << endl;
					}
					else
					{
						movementVec.x = 0;
						movementVec.y = 0;
					}
				}


				if( collision && minContact.edge->Normal().y < 0 && minContact.position.y >= position.y + b.rh )
				{
					groundOffsetX = (position.x - minContact.position.x) / 2; //halfway?
					ground = minContact.edge;
					edgeQuantity = minContact.edge->GetQuantity( minContact.position );
					double groundLength = length( ground->v1 - ground->v0 );
					groundSpeed = dot( velocity, normalize( ground->v1 - ground->v0 ));
					movement = 0;
					//velocity.x = 0;
					//velocity.y = 0;
					offsetX = position.x - minContact.position.x;
					//if( ground->Normal().x == 0 )
					//	offsetX = -b.rw;
					//cout << "offfff: " << offsetX << endl;
				}
			}
		}
	}

	void UpdatePostPhysics()
	{
		if( ground != NULL )
		{
			if( collision )
			{
				action = LAND;
				frame = 0;

				V2d gn = ground->Normal();
			}
			Vector2<double> groundPoint = ground->GetPoint( edgeQuantity );
			position = groundPoint;
			
			V2d gn = ground->Normal();

			//if( gn.x > 0 )
			//	offsetX =  b.rw;
			//else if( gn.x < 0 )
			//	offsetX = -b.rw;

			position.x += offsetX;
		//	cout << "offsetx: " << offsetX << endl;

			//if( gn.y > 0 )
			//	position.y += 32;
			if( gn.y < 0 )
				position.y -= b.rh;

		//+ ground->Normal() * 32.0;//Vector2<double>( collisionPosition.x, collisionPosition.y );
		//	position.x += groundOffsetX;
		}
		else
		{
			if( leftGround )
			{
				action = JUMP;
				frame = 1;
			}
		}

			//sprite->setRotation( PI );

		
		



		//cout << "updating" << endl;

		switch( action )
		{
		case STAND:
			
			
			sprite->setTexture( *(tilesetStand->texture));
			
			
			//sprite->setTextureRect( tilesetStand->GetSubRect( frame / 4 ) );
			if( facingRight )
			{
				sprite->setTextureRect( tilesetStand->GetSubRect( frame / 8 ) );
			}
			else
			{
				sf::IntRect ir = tilesetStand->GetSubRect( frame / 8 );
				
				sprite->setTextureRect( sf::IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height ) );
			}
			//sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2 );
			//sprite->setRotation(  );


			if( ground != NULL )
			{
				double angle = 0;
				//if( edgeQuantity == 0 || edgeQuantity == length( ground->v1 - ground->v0 ) )
				if( offsetX < b.rw && offsetX > -b.rw )
				{

				}
				else
				{
					angle = asin( dot( ground->Normal(), V2d( 1, 0 ) ) ); 
				}

				sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height);
				V2d pp = ground->GetPoint( edgeQuantity );
				sprite->setPosition( pp.x, pp.y );
				sprite->setRotation( angle / PI * 180 );


				//cout << "angle: " << angle / PI * 180  << endl;
			}

			//sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2 );
			//sprite->setPosition( position.x, position.y );
			//cout << "setting to frame: " << frame / 4 << endl;
			break;
		case RUN:
			
			
			sprite->setTexture( *(tilesetRun->texture));
			if( facingRight )
			{
				sprite->setTextureRect( tilesetRun->GetSubRect( frame / 4 ) );
			}
			else
			{
				sf::IntRect ir = tilesetRun->GetSubRect( frame / 4 );
				
				sprite->setTextureRect( sf::IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height ) );
			}
			

			if( ground != NULL )
			{
				double angle = 0;
				//if( edgeQuantity == 0 || edgeQuantity == length( ground->v1 - ground->v0 ) )
				if( offsetX < b.rw && offsetX > -b.rw )
				{

				}
				else
				{
					angle = asin( dot( ground->Normal(), V2d( 1, 0 ) ) ); 
				}
				//sprite->setOrigin( b.rw, 2 * b.rh );
				sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);
				//V2d pp = ground->GetPoint( edgeQuantity );
				//sprite->setPosition( pp.x, pp.y );
				sprite->setPosition( position.x, position.y );
				sprite->setRotation( angle / PI * 180 );
				//sprite->setPosition( position.x, position.y );
				//cout << "angle: " << angle / PI * 180  << endl;
			}
			break;
		case JUMP:
			sprite->setTexture( *(tilesetJump->texture));
			{
			sf::IntRect ir;


			

			if( frame == 0 )
			{
				ir = tilesetJump->GetSubRect( 0 );
			}
			else if( velocity.y < 0 )
			{
				ir = tilesetJump->GetSubRect( 1 );
			}
			else if( velocity.y == 0 )
			{
				ir = tilesetJump->GetSubRect( 2 );
			}
			else
			{
				ir = tilesetJump->GetSubRect( 8 );
			}

			if( frame > 0 )
			{
				sprite->setRotation( 0 );
			}

			if( !facingRight )
			{
				sprite->setTextureRect( sf::IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height ) );
			}
			else
			{
				sprite->setTextureRect( ir );
			}
			}
			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2 );
			sprite->setPosition( position.x, position.y );

			break;
		case LAND: 
			sprite->setTexture( *(tilesetLand->texture));
			if( facingRight )
			{
				sprite->setTextureRect( tilesetLand->GetSubRect( frame / 4 ) );
			}
			else
			{
				sf::IntRect ir = tilesetLand->GetSubRect( frame / 4 );
				
				sprite->setTextureRect( sf::IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height ) );
			}
			double angle = asin( dot( ground->Normal(), V2d( 1, 0 ) ) ); 
			sprite->setRotation( angle / PI * 180 );

			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2 );
			sprite->setPosition( position.x, position.y );

			break;
		}

		

		++frame;
		//cout << "end frame: " << position.x << ", " << position.y << endl;
	}


	Action action;
	int frame;
	Vector2<double> position;
	Vector2<double> velocity;
	CollisionBox *physBox;
};


Contact *currentContact;
Contact *collideEdge( const Actor &a, const CollisionBox &b, Edge *e, const V2d &vel )
{
	Vector2<double> oldPosition = a.position - vel;
	double left = a.position.x + b.offset.x - b.rw;
	double right = a.position.x + b.offset.x + b.rw;
	double top = a.position.y + b.offset.y - b.rh;
	double bottom = a.position.y + b.offset.y + b.rh;

	

	double oldLeft = oldPosition.x + b.offset.x - b.rw;
	double oldRight = oldPosition.x + b.offset.x + b.rw;
	double oldTop = oldPosition.y + b.offset.y - b.rh;
	double oldBottom = oldPosition.y + b.offset.y + b.rh;


	double edgeLeft = min( e->v0.x, e->v1.x );
	double edgeRight = max( e->v0.x, e->v1.x ); 
	double edgeTop = min( e->v0.y, e->v1.y ); 
	double edgeBottom = max( e->v0.y, e->v1.y ); 

	bool aabbCollision = false;
	if( left <= edgeRight && right >= edgeLeft && top <= edgeBottom && bottom >= edgeTop )
	{	
		
		aabbCollision = true;
		
	}

	Vector2<double> half = (oldPosition + a.position ) / 2.0;
		double halfLeft = half.x - b.rw;
		double halfRight = half.x + b.rw;
		double halfTop = half.y - b.rh;
		double halfBottom = half.y + b.rh;

	if( halfLeft <= edgeRight && halfRight >= edgeLeft && halfTop <= edgeBottom && halfBottom >= edgeTop )
		{
			aabbCollision = true;
		}

	V2d opp;
	if( aabbCollision )
	{
		Vector2<double> corner(0,0);
		Vector2<double> edgeNormal = e->Normal();

		if( edgeNormal.x > 0 )
		{
			corner.x = left;
			opp.x = right;
		}
		else if( edgeNormal.x < 0 )
		{
			corner.x = right;
			opp.x = left;
		}
		else
		{
			if( edgeLeft <= left )
			{
				corner.x = left;
				opp.x = right;
			}
			else if ( edgeRight >= right )
			{
				corner.x = right;
				opp.x = left;
			}
			else
			{
				corner.x = (edgeLeft + edgeRight) / 2;
				opp.x = corner.x;
			}
		}

		if( edgeNormal.y > 0 )
		{
			corner.y = top;
			opp.y = bottom;
		}
		else if( edgeNormal.y < 0 )
		{
			corner.y = bottom;
			opp.y = top;
		}
		else
		{
			//aabb
			if( edgeTop <= top )
			{
				corner.y = top;
				opp.y = bottom;
			}
			else if ( edgeBottom >= bottom )
			{
				corner.y = bottom;
				opp.y = top;
			}
			else
			{
				corner.y = (edgeTop+ edgeBottom) / 2;
				opp.y = corner.y;
			}
		}

		double res = cross( corner - e->v0, e->v1 - e->v0 );
		double resOpp = cross( opp - e->v0, e->v1 - e->v0 );




		double measureNormal = dot( edgeNormal, normalize(-vel) );

		//cout << "measureNormal: " << measureNormal << endl	
		if( res < 0 && resOpp > 0 && measureNormal > 0 && ( vel.x != 0 || vel.y != 0 )  )	
		{
			
			Vector2<double> invVel = normalize(-vel);



			LineIntersection li = lineIntersection( corner, corner - (vel), e->v0, e->v1 );

			//assert( li.parallel == false );
			if( li.parallel )
			{
				return NULL;
			}
			Vector2<double> intersect = li.position;

			double intersectQuantity = e->GetQuantity( intersect );
			Vector2<double> collisionPosition = intersect;

			/*if( e->Normal().y == 0 )
			{
				if( 
				{

				}
				else
				{
				}
			}*/


			if( intersectQuantity < 0 || intersectQuantity > length( e->v1 - e->v0 ) )
			{
				double leftDist = edgeLeft - right;
				double rightDist = edgeRight - left;
				double topDist = edgeTop - bottom;
				double bottomDist = edgeBottom - top;

				double resolveDist = 10000;
				double resolveLeft = 10000;
				double resolveRight = 10000;
				double resolveTop = 10000;
				double resolveBottom = 10000;
				if( left <= edgeRight && vel.x < 0 )
				{
					resolveLeft = (edgeRight - left) / abs(normalize(vel).x);
				}
				else if( right >= edgeLeft && vel.x > 0 )
				{
					resolveRight = (right - edgeLeft) / abs(normalize(vel).x);
				}

				if( top <= edgeBottom && vel.y < 0 )
				{
					resolveTop = (edgeBottom - top) / abs(normalize(vel).y);// / abs(a.velocity.x);
				}
				else if( bottom >= edgeTop && vel.y > 0 )
				{
					resolveBottom = (bottom- edgeTop) / abs(normalize(vel).y);// / abs(a.velocity.x);
				}


				resolveDist = min( resolveTop, min( resolveBottom, min( resolveLeft, resolveRight) ) );


				currentContact->resolution = normalize(-vel) * resolveDist;

				if( resolveDist == 10000 || length( currentContact->resolution) > length(vel) + 10 )
				{
					cout << "formally an error" << endl;
					return NULL;
				}

				
				assert( resolveDist != 10000 );
				
				double pri = length( vel + currentContact->resolution );
				//double pri = length( Vector2<double>( a.velocity.x / currentContact->resolution.x, a.velocity.y / currentContact->resolution.y ) );
			//	double pri = 0;
				currentContact->collisionPriority = pri;
				if( intersectQuantity < 0 )
				currentContact->position = e->v0;
				else 
					currentContact->position = e->v1;

				return currentContact;
				//cout << "here!!!: " << currentContact->resolution.x << ", "
				//	<< currentContact->resolution.y << endl;
			}

			
			currentContact->resolution = intersect - corner;

			if( length( currentContact->resolution ) > length( vel ) + 10 )
				return NULL;
			if( dot( normalize( -vel ), normalize(intersect - corner ) ) < .999 )
			{
			//	return NULL;
			}
			double pri = dot( intersect - ( corner - vel ), normalize( vel ) );

			if( pri < -1 )
			{
				cout << "BUSTED--------------- " << edgeNormal.x << ", " << edgeNormal.y  << ", " << pri  << endl;
				return NULL;
			}

			intersectQuantity = e->GetQuantity( intersect );
			//cout << "intersectQuantity2222: " << intersectQuantity << endl;
			

			//cs.setPosition( bottomLeft - invVel );
			//window->draw( cs );

			//Vector2<double> p = intersect - corner;
			currentContact->position = collisionPosition;//intersect;
			
			currentContact->collisionPriority = pri;//dot(intersect - ( corner + invVel), e->Normal());
			currentContact->edge = e;

			return currentContact;
			//a.position = a.position + p;
		}

		
	}

	return NULL;
}


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

int main()
{
	bool aaa = true;

	if( aaa )
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
	
	sf::Vector2i pos( 0, 0 );

	//window->setPosition( pos );
	window->setVerticalSyncEnabled( true );
	//window->setFramerateLimit( 60 );
	window->setMouseCursorVisible( true );
	
	View view( Vector2f( 300, 300 ), sf::Vector2f( 960 * 2, 540 * 2 ) );
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
	Vector2<double> *points = new Vector2<double>[numPoints];
	list<Vector2<double>> lvec;

	Actor player;
	//player.position = Vector2<double>( -500, 300 );

	is >> player.position.x;
	is >> player.position.y;

	int pointsLeft = numPoints;

	int pointCounter = 0;

	list<VertexArray*> polygons;

	
	//polygons = new VertexArray*[

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

		VertexArray *va = new VertexArray( sf::Triangles , tris.size() * 3 );
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

		if( sf::Keyboard::isKeyPressed( sf::Keyboard::Escape ) )
				return 0;



			prevInput = currInput;
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
			
			double f = 30;			
			//player.velocity = Vector2<double>( 0, 0 );
			if( sf::Keyboard::isKeyPressed( sf::Keyboard::Right ) )
			{

				if( player.ground == NULL )
				{
					player.velocity += Vector2<double>( f, 0 );
				}
				else
				{
					//player.groundSpeed = abs(player.groundSpeed);
				}
				
				//break;
			}
			if( sf::Keyboard::isKeyPressed( sf::Keyboard::Left ) )
			{
				if( player.ground == NULL )
				{
					player.velocity += Vector2<double>( -f, 0 );
				}
				else
				{
					//player.groundSpeed = -abs(player.groundSpeed);
				}

				//break;
			}
//			if( !( sf::Keyboard::isKeyPressed( sf::Keyboard::Right ) || sf::Keyboard::isKeyPressed( sf::Keyboard::Left ) )
//			{
				
//			}
			if( sf::Keyboard::isKeyPressed( sf::Keyboard::Up ) )
			{
				player.velocity += Vector2<double>( 0, -f );
				//break;
			}
			if( sf::Keyboard::isKeyPressed( sf::Keyboard::Down ) )
			{
				player.velocity += Vector2<double>( 0, f );
				//break;
			}

			

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
		view.setCenter( player.position.x, player.position.y );
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

	window->close();
	delete window;
}

