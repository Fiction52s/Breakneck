#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>

using namespace std;
using namespace sf;

#define V2d sf::Vector2<double>


Crawler::Crawler( GameSession *owner, Edge *g, double q, bool cw, double s )
:Enemy( owner ), ground( g ), edgeQuantity( q ), clockwise( cw ), groundSpeed( s )
{
	ts = owner->GetTileset( "crawler.png", 32, 32 );
	sprite.setTexture( *ts->texture );
	sprite.setTextureRect( ts->GetSubRect( 0 ) );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2 );
	V2d gPoint = g->GetPoint( edgeQuantity );
	sprite.setPosition( gPoint.x, gPoint.y );

	spawnRect = sf::Rect<double>( gPoint.x - 16, gPoint.y - 16, 16 * 2, 16 * 2 );

	physBody.isCircle = false;
	physBody.offset.x = 0;
	physBody.offset.y = 0;
	physBody.rw = 16;
	physBody.rh = 16;
	physBody.type = CollisionBox::BoxType::Physics;

	

	V2d gn = g->Normal();
	if( gn.x > 0 )
		offset.x = physBody.rw;
	else if( gn.x < 0 )
		offset.x = -physBody.rw;
	if( gn.y > 0 )
		offset.y = physBody.rh;
	else if( gn.y < 0 )
		offset.y = -physBody.rh;

	position = gPoint + offset;
}

void Crawler::HandleEntrant( QuadTreeEntrant *qte )
{
	assert( queryMode != "" );

	Edge *e = (Edge*)qte;


	if( ground == e )
			return;

	if( queryMode == "resolve" )
	{
		Contact *c = owner->coll.collideEdge( position + physBody.offset, physBody, e, tempVel, owner->window );
		if( c != NULL )
		{
			if( !col || (c->collisionPriority >= -.00001 && ( c->collisionPriority <= minContact.collisionPriority || minContact.collisionPriority < -.00001 ) ) )
			{	
				if( c->collisionPriority == minContact.collisionPriority )
				{
					if( length(c->resolution) > length(minContact.resolution) )
					{
						minContact.collisionPriority = c->collisionPriority;
						minContact.edge = e;
						minContact.resolution = c->resolution;
						minContact.position = c->position;
						col = true;
					}
				}
				else
				{
					//if( minContact.edge != NULL )
					//cout << minContact.edge->Normal().x << ", " << minContact.edge->Normal().y << "... " 
					//	<< e->Normal().x << ", " << e->Normal().y << endl;
					minContact.collisionPriority = c->collisionPriority;
					//cout << "pri: " << c->collisionPriority << endl;
					minContact.edge = e;
					minContact.resolution = c->resolution;
					minContact.position = c->position;
					col = true;
					
				}
			}
		}
	}
	++possibleEdgeCount;
}

void Crawler::UpdateHitboxes()
{
	
	if( ground != NULL )
	{
		V2d gn = ground->Normal();
		double angle = 0;
		if( !approxEquals( abs(offset.x), physBody.rw ) )
		{

			//this should never happen
		}
		else
		{
			angle = atan2( gn.x, -gn.y );
		}
		hitBody.globalAngle = angle;
		hurtBody.globalAngle = angle;
	}
	else
	{
		hitBody.globalAngle = 0;
		hurtBody.globalAngle = 0;
	}

	hitBody.globalPosition = position + V2d( hitBody.offset.x * cos( hitBody.globalAngle ) + hitBody.offset.y * sin( hitBody.globalAngle ), hitBody.offset.x * -sin( hitBody.globalAngle ) + hitBody.offset.y * cos( hitBody.globalAngle ) );
	physBody.globalPosition = position;//+ V2d( -16, 0 );// + //physBody.offset + offset;
}

void Crawler::UpdatePrePhysics()
{
	groundSpeed = 1;
}

void Crawler::UpdatePhysics()
{
	double movement = 0;
	double maxMovement = min( physBody.rw, physBody.rh );
	V2d movementVec;
	V2d lastExtra( 100000, 100000 );

	//make sure ground != NULL
	movement = groundSpeed;


	while( (movement != 0) )
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

			if( approxEquals( offset.x, physBody.rw ) )
				offset.x = physBody.rw;
			else if( approxEquals( offset.x, -physBody.rw ) )
				offset.x = -physBody.rw;

			if( approxEquals( offset.y, physBody.rh ) )
				offset.y = physBody.rh;
			else if( approxEquals( offset.y, -physBody.rh ) )
				offset.y = -physBody.rh;

			Edge *e0 = ground->edge0;
			Edge *e1 = ground->edge1;
			V2d e0n = e0->Normal();
			V2d e1n = e1->Normal();

			bool transferLeft = false;
			bool transferRight = false;
			bool changeOffsetX = false;
			bool changeOffsetY = false;
			
			cout << "offset: " << offset.x << ", " << offset.y << endl;

			if( q == 0 && movement < 0 )
			{
				if( e0n.x < 0 )
				{
					if( e0n.y < 0 )
					{
						if( offset.x == -physBody.rw && offset.y == -physBody.rh )
						{
							transferLeft = true;
						}
						else
						{
							if( offset.x > -physBody.rw )
								changeOffsetX = true;
							else if( offset.y > -physBody.rh )
								changeOffsetY = true;
							else
								assert( false && "what 0" );
						}
					}
					else if( e0n.y > 0 )
					{
						if( offset.x == -physBody.rw && offset.y == physBody.rh )
						{
							transferLeft = true;
						}
						else
						{
							if( offset.x > -physBody.rw )
								changeOffsetX = true;
							else if( offset.y < physBody.rh )
								changeOffsetY = true;
							else
								assert( false && "what 1" );
						}
					}
					else
					{
						if( offset.x == -physBody.rw && offset.y == -physBody.rh )
						{
							transferLeft = true;
						}
						else
						{
							if( offset.x > -physBody.rw )
								changeOffsetX = true;
							else if( offset.y > -physBody.rh )
								changeOffsetY = true;
							else
								assert( false && "what 0" );
						}
					}
				}
				else if( e0n.x > 0 )
				{
					if( e0n.y < 0 )
					{
						if( offset.x == physBody.rw && offset.y == -physBody.rh )
						{
							transferLeft = true;
						}
						else
						{
							if( offset.x < physBody.rw )
								changeOffsetX = true;
							else if( offset.y > -physBody.rh )
								changeOffsetY = true;
							else
								assert( false && "what 1" );
						}
					}
					else if( e0n.y > 0 )
					{
						if( offset.x == physBody.rw && offset.y == physBody.rh )
						{
							transferLeft = true;
						}
						else
						{
							if( offset.x < physBody.rw )
								changeOffsetX = true;
							else if( offset.y < physBody.rh )
								changeOffsetY = true;
							else
								assert( false && "what 1" );
						}
					}
					else
					{
					}
				}
				else
				{
					if( e0n.y < 0 )
					{
						if( offset.x == -physBody.rw )
						{
							transferLeft = true;
						}
						else
						{
							changeOffsetX = true;
						}
					}
					else if( e0n.y > 0 )
					{
						if( offset.x == physBody.rw )
						{
							transferLeft = true;
						}
						else
						{
							changeOffsetX = true;
						}
					}
					else
						assert( false && "cant happen" );
				}
			}
			else if( q == groundLength && movement > 0 )
			{
				if( e1n.x < 0 )
				{
					if( e1n.y < 0 )
					{
						if( offset.x == -physBody.rw && offset.y == -physBody.rh )
						{
							transferRight = true;
						}
						else
						{
							if( offset.x > -physBody.rw )
								changeOffsetX = true;
							else if( offset.y > -physBody.rh )
								changeOffsetY = true;
							else
								assert( false && "what 0" );
						}
					}
					else if( e1n.y > 0 )
					{
						if( offset.x == -physBody.rw && offset.y == physBody.rh )
						{
							transferRight = true;
						}
						else
						{
							if( offset.x > -physBody.rw )
								changeOffsetX = true;
							else if( offset.y < physBody.rh )
								changeOffsetY = true;
							else
								assert( false && "what 1" );
						}
					}
					else
					{
						if( offset.x == -physBody.rw && offset.y == -physBody.rh )
						{
							transferRight = true;
						}
						else
						{
							if( offset.x > -physBody.rw )
								changeOffsetX = true;
							else if( offset.y > -physBody.rh )
								changeOffsetY = true;
							else
								assert( false && "what 0" );
						}
					}
				}
				else if( e1n.x > 0 )
				{
					if( e1n.y < 0 )
					{
						if( offset.x == physBody.rw && offset.y == -physBody.rh )
						{
							transferRight = true;
						}
						else
						{
							if( offset.x < physBody.rw )
								changeOffsetX = true;
							else if( offset.y > -physBody.rh )
								changeOffsetY = true;
							else
								assert( false && "what 1" );
						}
					}
					else if( e1n.y > 0 )
					{
						if( offset.x == physBody.rw && offset.y == physBody.rh )
						{
							transferRight = true;
						}
						else
						{
							if( offset.x < physBody.rw )
								changeOffsetX = true;
							else if( offset.y < physBody.rh )
								changeOffsetY = true;
							else
								assert( false && "what 1" );
						}
					}
					else
					{
						if( offset.x == physBody.rw && offset.y == physBody.rh )
						{
							transferRight = true;
						}
						else
						{
							if( offset.x < physBody.rw )
								changeOffsetX = true;
							else if( offset.y < physBody.rh )
								changeOffsetY = true;
							else
								assert( false && "what 0" );
						}
					}
				}
				else
				{
					if( e1n.y < 0 )
					{
						if( offset.x == physBody.rw && offset.y == -physBody.rh )
						{
							transferRight = true;
						}
						else
						{
							if( offset.x < physBody.rw )
								changeOffsetX = true;
							else if( offset.y > -physBody.rh )
								changeOffsetY = true;
							else
								assert( false && "blah1" );
						}
					}
					else if( e1n.y > 0 )
					{
						if( offset.x == -physBody.rw && offset.y == physBody.rh )
						{
							transferRight = true;
						}
						else
						{
							if( offset.x > -physBody.rw )
								changeOffsetX = true;
							else if( offset.y < physBody.rh )
								changeOffsetY = true;
							else
								assert( false && "blah2" );
						}
					}
					else
						assert( false && "cant happen" );
				}
			}
				
			if( transferLeft )
			{
			//	cout << "transfer left "<< endl;
				Edge *next = ground->edge0;
				
				ground = next;
				q = length( ground->v1 - ground->v0 );					
			}
			else if( transferRight )
			{
				Edge *next = ground->edge1;
				
				ground = next;
				q = 0;
			}
			else if( changeOffsetX )
			{
				if( gNormal.y < 0 || (gNormal.y == 0 && gNormal.x < 0 ) )
				{
				if( movement > 0 )
				{				
					extra = (offset.x + movement) - physBody.rw;				
				}
				else 
				{
					extra = (offset.x + movement) + physBody.rw;
				}
				double m = movement;
				if( (movement > 0 && extra > 0) || (movement < 0 && extra < 0) )
				{
					m -= extra;
					movement = extra;

					if( movement > 0 )
					{
						offset.x = physBody.rw;
					}
					else
					{
						offset.x = -physBody.rw;
					}
				}
				else
				{
					movement = 0;
					offset.x += m;
				}

				if(!approxEquals( m, 0 ) )
				{
					bool hit = ResolvePhysics( V2d( m, 0 ));
					if( hit && (( m > 0 && minContact.edge != ground->edge0 ) || ( m < 0 && minContact.edge != ground->edge1 ) ) )
					{
						V2d eNorm = minContact.edge->Normal();
						if( eNorm.y < 0 )
						{
							//if( minContact.position.y >= position.y + physBody.rh - 5 )
							{
								if( m > 0 && eNorm.x < 0 )
								{
									ground = minContact.edge;
									q = ground->GetQuantity( minContact.position );
									edgeQuantity = q;
									offset.x = -physBody.rw;
									continue;
								}
								else if( m < 0 && eNorm.x > 0 )
								{
									ground = minContact.edge;
									q = ground->GetQuantity( minContact.position );
									edgeQuantity = q;
									offset.x = physBody.rw;
									continue;
								}
								

							}
						}
						else
						{
								offset.x += minContact.resolution.x;
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
						extra = (-offset.x + movement) - physBody.rw;				
					}
					else 
					{
						extra = (-offset.x + movement) + physBody.rw;
					}
					double m = movement;
					if( (m > 0 && extra > 0) || (m < 0 && extra < 0) )
					{
						m -= extra;
						movement = extra;

						if( movement > 0 )
						{
							offset.x = -physBody.rw;
						}
						else
						{
							offset.x = physBody.rw;
						}
					}
					else
					{
						movement = 0;
						offset.x -= m;
					}

					if(!approxEquals( m, 0 ) )
					{
						bool hit = ResolvePhysics( V2d( -m, 0 ));
						if( hit && (( m > 0 && minContact.edge != ground->edge0 ) || ( m < 0 && minContact.edge != ground->edge1 ) ) )
						{
							V2d eNorm = minContact.edge->Normal();
							if( eNorm.y < 0 )
							{
								//if( minContact.position.y >= position.y + physBody.rh - 5 )
								{
									if( m > 0 && eNorm.x < 0 )
									{
										ground = minContact.edge;
										q = ground->GetQuantity( minContact.position );
										edgeQuantity = q;
										offset.x = -physBody.rw;
										continue;
									}
									else if( m < 0 && eNorm.x > 0 )
									{
										ground = minContact.edge;
										q = ground->GetQuantity( minContact.position );
										edgeQuantity = q;
										offset.x = physBody.rw;
										continue;
									}
								

								}
							}
							else
							{
									offset.x += minContact.resolution.x;
									groundSpeed = 0;
									break;
							}
						}
					}
				}

			}
			else if( changeOffsetY )
			{
				if( gNormal.x < 0 )
				{
					cout << "changing offsety: " << offset.x << ", " << offset.y << endl;
					if( movement > 0 )
					{				
						extra = (-offset.y + movement) - physBody.rh;				
					}
					else 
					{
						extra = (-offset.y + movement) + physBody.rh;
					}
					double m = movement;

					if( (movement > 0 && extra > 0) || (movement < 0 && extra < 0) )
					{
						//m -= extra;
					
						m -= extra;
						movement = extra;

						if( movement > 0 )
						{
							offset.y = -physBody.rh;
						}
						else
						{
							offset.y = physBody.rh;
						}
					}
					else
					{
						//m = -m;
						movement = 0;
						offset.y -= m;
					}

					if(!approxEquals( m, 0 ) )
					{
						bool hit = ResolvePhysics( V2d( 0, -m ));
						if( hit && (( m > 0 && minContact.edge != ground->edge0 ) || ( m < 0 && minContact.edge != ground->edge1 ) ) )
						{
							V2d eNorm = minContact.edge->Normal();

							ground = minContact.edge;

							q = ground->GetQuantity( minContact.position + minContact.resolution );
							edgeQuantity = q;

							V2d gn = ground->Normal();
							if( gn.x > 0 )
								offset.x = physBody.rw;
							else if( gn.x < 0 )
								offset.x = -physBody.rw;

							if( gn.y > 0 )
								offset.y = physBody.rh;
							else if( gn.y < 0 )
								offset.y = -physBody.rh;

							position = ground->GetPoint( edgeQuantity ) + offset;

							break;
						}
					}
				}
				else
				{
					if( movement > 0 )
					{				
						extra = (offset.y + movement) - physBody.rh;				
					}
					else 
					{
						extra = (offset.y + movement) + physBody.rh;
					}
					double m = movement;

					if( (movement > 0 && extra > 0) || (movement < 0 && extra < 0) )
					{
						//m -= extra;
					
						m -= extra;
						movement = extra;

						if( movement > 0 )
						{
							offset.y = physBody.rh;
						}
						else
						{
							offset.y = -physBody.rh;
						}
					}
					else
					{
						//m = -m;
						movement = 0;
						offset.y += m;
					}

					if(!approxEquals( m, 0 ) )
					{
						bool hit = ResolvePhysics( V2d( 0, m ));
						if( hit && (( m > 0 && minContact.edge != ground->edge0 ) || ( m < 0 && minContact.edge != ground->edge1 ) ) )
						{
							V2d eNorm = minContact.edge->Normal();

							ground = minContact.edge;

							q = ground->GetQuantity( minContact.position + minContact.resolution );
							edgeQuantity = q;

							V2d gn = ground->Normal();
							if( gn.x > 0 )
								offset.x = physBody.rw;
							else if( gn.x < 0 )
								offset.x = -physBody.rw;

							if( gn.y > 0 )
								offset.y = physBody.rh;
							else if( gn.y < 0 )
								offset.y = -physBody.rh;

							position = ground->GetPoint( edgeQuantity ) + offset;

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
				

				if( m == 0 )
				{
					cout << "secret: " << endl;//<< gNormal.x << ", " << gNormal.y << ", " << q << ", " << offsetX <<  endl;
					groundSpeed = 0;
					break;
				}

				if( !approxEquals( m, 0 ) )//	if(m != 0 )
				{	
					bool down = true;
					bool hit = ResolvePhysics( normalize( ground->v1 - ground->v0 ) * m);
					if( hit && (( m > 0 && minContact.edge != ground->edge0 ) || ( m < 0 && minContact.edge != ground->edge1 ) ) )
					{
						V2d eNorm = minContact.edge->Normal();

						ground = minContact.edge;

						//q = ground->GetQuantity( ground->GetPoint( q ) + minContact.resolution);
						q = ground->GetQuantity( minContact.position + minContact.resolution );
						//groundSpeed = 0;
						edgeQuantity = q;

						V2d gn = ground->Normal();
						if( gn.x > 0 )
							offset.x = physBody.rw;
						else if( gn.x < 0 )
							offset.x = -physBody.rw;
						else
						{
							offset.x = position.x + minContact.resolution.x - minContact.position.x;
							//offsetX = position.x + minContact.resolution.x - minContact.position.x;
						}

						if( gn.y > 0 )
							offset.y = physBody.rh;
						else if( gn.y < 0 )
							offset.y = -physBody.rh;
						else
						{
							offset.y = position.y + minContact.resolution.y - minContact.position.y;
						}

						position = ground->GetPoint( edgeQuantity ) + offset;

						break;
					}
						
				}	
			}

			if( movement == extra )
				movement += steal;
			else
				movement = steal;

			edgeQuantity = q;
		}
	}
}

bool Crawler::ResolvePhysics( V2d vel )
{
	possibleEdgeCount = 0;
	position += vel;
	
	Rect<double> r( position.x + physBody.offset.x - physBody.rw, 
		position.y + physBody.offset.y - physBody.rh, 2 * physBody.rw, 2 * physBody.rh );
	minContact.collisionPriority = 1000000;

	

	tempVel = vel;

	col = false;
	minContact.edge = NULL;

	queryMode = "resolve";
	owner->terrainTree->Query( this, r );
	//Query( this, owner->testTree, r );

	return col;
}

void Crawler::UpdatePostPhysics()
{
	double angle = 0;
	V2d gn = ground->Normal();
	if( !approxEquals( abs(offset.x), physBody.rw ) )
	{
		if( gn.y > 0 )
			angle = PI;
	}
	else if( !approxEquals( abs( offset.y ), physBody.rh ) )
	{
		cout << "gn: " << gn.x << ", " << gn.y << endl;
		if( gn.x > 0 )
		{
			angle = PI / 2;
		}
		else if( gn.x < 0 )
		{
			angle = -PI / 2;
		}
		else
		{
			angle = atan2( gn.x, -gn.y );
		}
	}
	else
	{
		angle = atan2( gn.x, -gn.y );
	}

	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
	sprite.setRotation( angle / PI * 180 );

	V2d pp = ground->GetPoint( edgeQuantity );

	cout << "angle: " << angle << endl;
	if( angle == 0 || approxEquals( angle, PI ) )
	{
		cout << "one" << endl;
		sprite.setPosition( pp.x + offset.x, pp.y );
	}
	else if( approxEquals( angle, PI / 2 ) || approxEquals( angle, -PI / 2 ) )
	{
		cout << "two" << endl;
		sprite.setPosition( pp.x, pp.y + offset.y );
	}
	else
	{
		cout << "three: " << angle << endl;
		sprite.setPosition( pp.x, pp.y );

	}

	//sprite.setPosition( position );
	UpdateHitboxes();
}

bool Crawler::PlayerSlowingMe()
{
	return false;
}

void Crawler::Draw(sf::RenderTarget *target )
{
	target->draw( sprite );


}

bool Crawler::IHitPlayer()
{
	return false;
}

bool Crawler::PlayerHitMe()
{
	return false;
}

void Crawler::UpdateSprite()
{
}

void Crawler::DebugDraw( RenderTarget *target )
{
	CircleShape cs;
	cs.setFillColor( Color::Cyan );
	cs.setRadius( 10 );
	cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
	V2d g = ground->GetPoint( edgeQuantity );
	cs.setPosition( g.x, g.y );

	owner->window->draw( cs );

	physBody.DebugDraw( target );
//	hurtBody.DebugDraw( target );
//	hitBody.DebugDraw( target );
}

void Crawler::SaveEnemyState()
{
}

void Crawler::LoadEnemyState()
{
}