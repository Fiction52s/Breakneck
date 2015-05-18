#include "Actor.h"
#include "GameSession.h"
#include "VectorMath.h"
#include <iostream>
#include <assert.h>

using namespace sf;
using namespace std;

#define V2d sf::Vector2<double>

Actor::Actor( GameSession *gs )
	:owner( gs )
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
		tilesetStand = owner->GetTileset( "stand.png", 64, 64 );

		actionLength[RUN] = 10 * 4;
		tilesetRun = owner->GetTileset( "run.png", 128, 64 );

		actionLength[JUMP] = 2;
		tilesetJump = owner->GetTileset( "jump.png", 64, 64 );

		actionLength[LAND] = 1;
		tilesetLand = owner->GetTileset( "land.png", 64, 64 );

		actionLength[DASH] = 120;
		tilesetDash = owner->GetTileset( "dash.png", 64, 64 );

		actionLength[WALLCLING] = 1;
		tilesetWallcling = owner->GetTileset( "wallcling.png", 64, 64 );

		actionLength[SLIDE] = 1;
		tilesetSlide = owner->GetTileset( "slide.png", 64, 64 );		

		action = JUMP;
		frame = 1;

		gravity = 2;
		maxFallSpeed = 30;

		ground = NULL;
		groundSpeed = 0;
		maxNormalRun = 30;
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

void Actor::ActionEnded()
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

void Actor::UpdatePrePhysics()
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

bool Actor::ResolvePhysics( Edge** edges, int numPoints, V2d vel )
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

		Contact *c = owner->coll.collideEdge( position , b, edges[i], vel );
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
	return col;
}

void Actor::UpdatePhysics( Edge **edges, int numPoints )
{
	leftGround = false;
	double movement = 0;
	double maxMovement = min( b.rw, b.rh );
	V2d movementVec;
	V2d lastExtra( 100000, 100000 );

	if( ground != NULL ) movement = groundSpeed;
	else
	{
		movementVec = velocity;
	}
		
	while( (ground != NULL && movement != 0) || ( ground == NULL && length( movementVec ) > 0 ) )
	{
		if( ground != NULL )
		{
			double steal = 0;
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
			double groundLength = length( ground->v1 - ground->v0 ); 

			if( approxEquals( q, 0 ) )
				q = 0;
			else if( approxEquals( q, groundLength ) )
				q = groundLength;

			if( approxEquals( offsetX, b.rw ) )
				offsetX = b.rw;
			else if( approxEquals( offsetX, -b.rw ) )
				offsetX = -b.rw;

			Edge *e0 = ground->edge0;
			Edge *e1 = ground->edge1;
			V2d e0n = e0->Normal();
			V2d e1n = e1->Normal();

			bool transferLeft =  q == 0 && movement < 0
				&& ((gNormal.x == 0 && e0->Normal().x == 0 )
				|| ( offsetX == -b.rw && (e0->Normal().x <= 0 || e0->Normal().y > 0) ) 
				|| (offsetX == b.rw && e0->Normal().x >= 0 && e0->Normal().y != 0 ));
			bool transferRight = q == groundLength && movement > 0 
				&& ((gNormal.x == 0 && e1->Normal().x == 0 )
				|| ( offsetX == b.rw && ( e1->Normal().x >= 0 || e1->Normal().y > 0 ))
				|| (offsetX == -b.rw && e1->Normal().x <= 0 && e1->Normal().y != 0 ) );
			bool offsetLeft = movement < 0 && offsetX > -b.rw && ( (q == 0 && e0->Normal().x < 0) || (q == groundLength && gNormal.x < 0) );
				
			bool offsetRight = movement > 0 && offsetX < b.rw && ( ( q == groundLength && e1->Normal().x > 0 ) || (q == 0 && gNormal.x > 0) );
			bool changeOffset = offsetLeft || offsetRight;
				
			if( transferLeft )
			{
				Edge *next = ground->edge0;
				if( next->Normal().y < 0 )
				{
					ground = next;
					q = length( ground->v1 - ground->v0 );	
				}
				else
				{
					velocity = normalize(ground->v1 - ground->v0 ) * groundSpeed;
					movementVec = normalize( ground->v1 - ground->v0 ) * extra;
					leftGround = true;
					ground = NULL;
				}
			}
			else if( transferRight )
			{
				Edge *next = ground->edge1;
				if( next->Normal().y < 0 )
				{
					ground = next;
					q = 0;
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
				//cout << "slide: " << q << ", " << offsetX << endl;
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
									edgeQuantity = q;
									offsetX = -b.rw;
									continue;
								}
								else if( m < 0 && eNorm.x > 0 )
								{
									ground = minContact.edge;
									q = ground->GetQuantity( minContact.position );
									edgeQuantity = q;
									offsetX = b.rw;
									continue;
								}

							}
							else
							{
								offsetX += minContact.resolution.x;
								groundSpeed = 0;
								break;
							}
						}
						else
						{
								offsetX += minContact.resolution.x;
								groundSpeed = 0;
								break;
						}
					}
				}
			}
			else
			{
				if( movement > 0 )
				{	
					extra = (q + movement) - groundLength;
				}
				else 
				{
					extra = (q + movement);
				}
					
				if( (movement > 0 && extra > 0) || (movement < 0 && extra < 0) )
				{
					if( movement > 0 )
					{
						q = groundLength;
					}
					else
					{
						q = 0;
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
					bool down = true;
					bool hit = ResolvePhysics( edges, numPoints, normalize( ground->v1 - ground->v0 ) * m);
					if( hit && (( m > 0 && minContact.edge != ground->edge0 ) || ( m < 0 && minContact.edge != ground->edge1 ) ) )
					{
						if( down)
						{
							V2d eNorm = minContact.edge->Normal();
							if( minContact.position.y > position.y + b.rh - 5 && eNorm.y >= 0 )
							{
								if( minContact.position == minContact.edge->v0 ) 
								{
									if( minContact.edge->edge0->Normal().y <= 0 )
									{
										minContact.edge = minContact.edge->edge0;
										eNorm = minContact.edge->Normal();
									}
								}
								else if( minContact.position == minContact.edge->v1 )
								{
									if( minContact.edge->edge1->Normal().y <= 0 )
									{
										minContact.edge = minContact.edge->edge1;
										eNorm = minContact.edge->Normal();
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
										ground = minContact.edge;
										q = ground->GetQuantity( minContact.position );
										V2d eNorm = minContact.edge->Normal();			
										offsetX = position.x + minContact.resolution.x - minContact.position.x;
									}

									if( offsetX < -b.rw || offsetX > b.rw )
									{
										cout << "BROKEN OFFSET: " << offsetX << endl;
										assert( false && "T_T" );
									}
								}
								else
								{
									q = ground->GetQuantity( ground->GetPoint( q ) + minContact.resolution);
									groundSpeed = 0;
									edgeQuantity = q;
									break;
								}
							}
							else
							{
								//cout << "zzz: " << q << ", " << eNorm.x << ", " << eNorm.y << endl;
								q = ground->GetQuantity( ground->GetPoint( q ) + minContact.resolution);
								groundSpeed = 0;
								edgeQuantity = q;
								break;
							}
						}
						else
						{
							cout << "Sdfsdfd" << endl;
							q = ground->GetQuantity( ground->GetPoint( q ) + minContact.resolution);
							groundSpeed = 0;
							edgeQuantity = q;
							break;
						}
						
					}
						
				}
				else
				{
					edgeQuantity = q;
					cout << "secret: " << gNormal.x << ", " << gNormal.y << ", " << q << ", " << offsetX <<  endl;
					offsetX = -offsetX;
			//		cout << "prev: " << e0n.x << ", " << e0n.y << endl;
					break;
				}
					
			}
			if( movement == extra )
				movement += steal;
			else
				movement = steal;

			edgeQuantity = q;
		}
		else
		{
			V2d stealVec(0,0);
			double moveLength = length( movementVec );
			V2d velDir = normalize( movementVec );
			if( moveLength > maxMovement )
			{
				stealVec = velDir * ( moveLength - maxMovement);
				movementVec = velDir * maxMovement;
			}

			V2d newVel( 0, 0 );
				
			//cout << "moving you: " << movementVec.x << ", " << movementVec.y << endl;
			collision = ResolvePhysics( edges, numPoints, movementVec );
			V2d extraVel(0, 0);
			if( collision )
			{
				position += minContact.resolution;
				Edge *e = minContact.edge;
				V2d en = e->Normal();
				Edge *e0 = e->edge0;
				Edge *e1 = e->edge1;
				V2d e0n = e0->Normal();
				V2d e1n = e1->Normal();

				V2d extraDir =  normalize( minContact.edge->v1 - minContact.edge->v0 );
			//	if( ( minContact.position == e->v0 && en.x <= 0 && (e0n.x >= 0 || e0n.y > 0 ) ) 
			//		|| ( minContact.position == e->v1 && en.x <= 0 && (e1n.x >= 0 || e1n.y > 0 ) ) )
			//	{
			//		extraDir = V2d( -1, 0 );
			//	}
				if( (minContact.position == e->v0 && en.x < 0 && en.y < 0 ) )
				{
					V2d te = e0->v0 - e0->v1;
					if( te.x > 0 )
					{
							
						extraDir = V2d( 0, -1 );
					}
				}
				else if( (minContact.position == e->v1 && en.x < 0 && en.y > 0 ) )
				{
					V2d te = e1->v1 - e1->v0;
					if( te.x > 0 )
					{
						extraDir = V2d( 0, -1 );
						
					}
				}

				else if( (minContact.position == e->v1 && en.x < 0 && en.y < 0 ) )
				{
					V2d te = e1->v1 - e1->v0;
					if( te.x < 0 )
					{
						extraDir = V2d( 0, 1 );
						
					}
				}
				else if( (minContact.position == e->v0 && en.x > 0 && en.y < 0 ) )
				{
					V2d te = e0->v0 - e0->v1;
					if( te.x > 0 )
					{	
						extraDir = V2d( 0, -1 );
					}
				}
				else if( (minContact.position == e->v1 && en.x > 0 && en.y < 0 ) )
				{
					V2d te = e1->v1 - e1->v0;
					if( te.x < 0 )
					{
						extraDir = V2d( 0, 1 );
					}
				}
				else if( (minContact.position == e->v0 && en.x > 0 && en.y > 0 ) )
				{
					V2d te = e0->v0 - e0->v1;
					if( te.x < 0 )
					{
						extraDir = V2d( 0, 1 );
						cout << "this thing" << endl;
					}
				}

			//	if( approxEquals(minContact.position.x, e->v1.x) && approxEquals(minContact.position.y, e->v1.y) && en.x < 0 && en.y < 0 )
			//	{
			//		cout << "________________________________" << endl;
			//	}
			//	else if( approxEquals(minContact.position.x, e->v0.x) && approxEquals(minContact.position.y, e->v0.y) && en.x > 0 && en.y > 0 )
			//	{
			//		cout << "&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&" << endl;
			//	}
			//	else
				{
					cout << "edge normal : " << minContact.edge->Normal().x << ", "<< minContact.edge->Normal().y << endl;

				}


				if( (minContact.position == e->v1 && en.x > 0 && en.y > 0 ) )
				{
					V2d te = e1->v1 - e1->v0;
					if( te.y < 0 )
					{
						extraDir = V2d( -1, 0 );
						//cout << "here" << endl;
					}
				}
				else if( (minContact.position == e->v0 && en.x < 0 && en.y > 0 ) )
				{
					V2d te = e0->v0 - e0->v1;
					if( te.y < 0 )
					{
						extraDir = V2d( -1, 0 );
					}
				}

					
				//en.y <= 0 && e0n.y >= 0  ) 
				//	|| ( minContact.position == e->v1 && en.y <= 0 && e1n.y >= 0 ) )
				/*{
					if( en.x < 0 )
					{
						extraDir = V2d( 0, -1 );
					}
					else
					{
						extraDir = V2d( 0, 1 );
					}
				}*/
				extraVel = dot( normalize( velocity ), extraDir ) * extraDir * length(minContact.resolution);
				newVel = dot( normalize( velocity ), extraDir ) * extraDir * length( velocity );
				//cout << "extra vel: " << extraVel.x << ", " << extraVel.y << endl;
				if( length( stealVec ) > 0 )
				{
					stealVec = length( stealVec ) * normalize( extraVel );
				}
				if( approxEquals( extraVel.x, lastExtra.x ) && approxEquals( extraVel.y, lastExtra.y ) )
				{
					cout << "glitcffff: " << extraVel.x << ", " << extraVel.y << endl;
					//extraVel.x = 0;
					//extraVel.y = 0;
					//cout << "glitchffff" << endl;
					//break;
					//newVel.x = 0;
					//newVel.y = 0;
						
				}
				lastExtra.x = extraVel.x;
				lastExtra.y = extraVel.y;

			//	cout << "extra vel 1: " << extraVel.x << ", " << extraVel.y << endl;
			}
			else if( length( stealVec ) == 0 )
			{
				movementVec.x = 0;
				movementVec.y = 0;
			}

			//cout << "blah: " << minContact.position.y - (position.y + b.rh ) << ", " << collision << endl;
			if( collision && minContact.edge->Normal().y < 0 && minContact.position.y >= position.y + b.rh  )
			{
				groundOffsetX = (position.x - minContact.position.x) / 2; //halfway?
				ground = minContact.edge;
				edgeQuantity = minContact.edge->GetQuantity( minContact.position );
				double groundLength = length( ground->v1 - ground->v0 );
				groundSpeed = length( velocity );
				if( velocity.x < 0 )
				{
					groundSpeed = -groundSpeed;
				}

				movement = 0;
			
				offsetX = position.x - minContact.position.x;
				//cout << "groundinggg" << endl;
			}
			else if( collision )
			{
					velocity = newVel;
			}

			if( length( extraVel ) > 0 )
			{
				movementVec = stealVec + extraVel;
			//	cout << "x1: " << movementVec.x << ", " << movementVec.y << endl;
			}

			else
			{
				movementVec = stealVec;
				//cout << "x2:  " << movementVec.x << ", " << movementVec.y << endl;
			//	cout << "x21: " << stealVec.x << ", " << stealVec.y << endl;
				//cout << "x22: " << movementVec.x << ", " << movementVec.y << endl;
			}


		}
	}
}

void Actor::UpdatePostPhysics()
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

		position.x += offsetX;

		if( gn.y < 0 )
			position.y -= b.rh;

	}
	else
	{
		if( leftGround )
		{
			action = JUMP;
			frame = 1;
		}
	}


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