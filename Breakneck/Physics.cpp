#include "Physics.h"
#include "VectorMath.h"
#include <iostream>
#include <assert.h>

using namespace sf;
using namespace std;

#define V2d sf::Vector2<double>

//EDGE FUNCTIONS
Edge::Edge()
{
	edge0 = NULL;
	edge1 = NULL;
}

V2d Edge::Normal()
{
	V2d v = v1 - v0;
	V2d temp = normalize( v );
	return V2d( temp.y, -temp.x );
}

V2d Edge::GetPoint( double quantity )
{
	//gets the point on a line w/ length quantity in the direction of the edge vector
	V2d e( v1 - v0 );
	e = normalize( e );
	return v0 + quantity * e;
}

double Edge::GetQuantity( V2d p )
{
	//projects the origin of the line to p onto the edge. if the point is on the edge it will just be 
	//normal to use dot product to get cos(0) =1
	V2d vv = p - v0;
	V2d e = normalize(v1 - v0);
	double result = dot( vv, e );
	double len = length( v1 - v0 );
	if( approxEquals( result, 0 ) )
		return 0;
	else if( approxEquals( result, length( v1 - v0 ) ) )
		return len;
	else
		return result;
}

double Edge::GetQuantityGivenX( double x )
{

	V2d e = normalize(v1 - v0);
	double deltax = x - v0.x;
	double factor = deltax / e.y;
}

//CONTACT FUNCTIONS
Contact::Contact()
	:edge( NULL )
{
	collisionPriority = 0;
}

Collider::Collider()
	:currentContact(NULL)
{
	currentContact = new Contact;
}

Collider::~Collider()
{
	delete currentContact;
}

Contact * Collider::collideEdge( V2d position, const CollisionBox &b, Edge *e, const V2d &vel )
{
	Vector2<double> oldPosition = position - vel;
	double left = position.x + b.offset.x - b.rw;
	double right = position.x + b.offset.x + b.rw;
	double top = position.y + b.offset.y - b.rh;
	double bottom = position.y + b.offset.y + b.rh;

	

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

	Vector2<double> half = (oldPosition + position ) / 2.0;
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
		if( res < 0 && resOpp > 0 && measureNormal > 0 && ( vel.x != 0 || vel.y != 0 )  )	
		{
			//cout << "vezzzzz: " << vel.x << ", " << vel.y << " .. norm: " << edgeNormal.x << ", " << edgeNormal.y << endl;
			Vector2<double> invVel = normalize(-vel);



			LineIntersection li = lineIntersection( corner, corner - (vel), e->v0, e->v1 );
			double testing = dot( normalize( (corner-vel) - corner), normalize( e->v1 - e->v0 ));
			if( li.parallel || abs( testing ) == 1 )
			{
				//cout << "returning null1" << endl;
				return NULL;
			}
			Vector2<double> intersect = li.position;

			double intersectQuantity = e->GetQuantity( intersect );
			Vector2<double> collisionPosition = intersect;
			//cout << "testing: " << dot( normalize( (corner-vel) - corner), normalize( e->v1 - e->v0 ))  << endl;
			if( intersectQuantity <= 0 || intersectQuantity >= length( e->v1 - e->v0 ) )
			{
				cout << "this option: " << intersectQuantity << ", " << length( e->v1 - e->v0 ) << endl;
				double leftDist = edgeLeft - right;
				double rightDist = edgeRight - left;
				double topDist = edgeTop - bottom;
				double bottomDist = edgeBottom - top;

				double resolveDist = 10000;
				double resolveLeft = 10000;
				double resolveRight = 10000;
				double resolveTop = 10000;
				double resolveBottom = 10000;
				//V2d resolveVec;
				if( left <= edgeRight && vel.x < 0 )
				{
				//	cout << "choose L" << endl;
					//if( vel.x == 0 )
					//	resolveLeft = 0;
					
						resolveLeft = ( edgeRight - left ) / abs( normalize(vel).x );//dot( V2d(edgeRight - left, 0), normalize( -vel ) );// / abs(normalize(vel).x);
				}
				else if( right >= edgeLeft && vel.x > 0 )
				{
				//	if( vel.x == 0 )
				//		resolveRight = 0;
					resolveRight = (right - edgeLeft) / abs( normalize(vel).x );
					//dot( V2d((right - edgeLeft),0), normalize( -vel ) );//
				}
				else
				{
					cout << "left: " << vel.x << ", " << vel.y << endl;
				}

				if( top <= edgeBottom && vel.y < 0 )
				{
				//	cout << "choose T" << endl;
				//	if( vel.y == 0 )
				//		resolveTop = 0;
					
						resolveTop = (edgeBottom - top) / abs( normalize(vel).y );//abs(normalize(vel).y );//dot( V2d( 0, (edgeBottom - top)), normalize( -vel ) );// / abs(normalize(vel).y);// / abs(a.velocity.x);
				}
				else if( bottom >= edgeTop && vel.y > 0 )
				{
				//	if( vel.y == 0 )
				//		resolveBottom  = 0;
				//	else
						resolveBottom = (bottom- edgeTop) / abs(normalize(vel).y );// abs(normalize(vel).y);// / abs(a.velocity.x);
					//dot( V2d( 0, (bottom - edgeTop)), normalize( -vel ) );//
				}
				else
				{
					//cout << "top: " << top << ", " << edgeTop << endl;
				}


				resolveDist = min( resolveTop, min( resolveBottom, min( resolveLeft, resolveRight) ) );
				//cout << "resolve dist: " << resolveDist << endl;
				if( approxEquals( resolveDist, 0 ) )
				{
					cout << "returning nNULLL heree" << endl;
					return NULL;
				}

				if( resolveDist == resolveTop ) cout << "T" << endl;
				else if( resolveDist == resolveBottom ) cout << "B" << endl;
				else if( resolveDist == resolveLeft ) cout << "L" << endl;
				else if( resolveDist == resolveRight ) cout << "R" << endl;
			



				

				currentContact->resolution = normalize(-vel) * resolveDist;
				//cout << "resolution toooo: " << currentContact->resolution.x  << ", " << currentContact->resolution.y << endl;
				cout << "dist: " << resolveDist << endl;

				if( resolveDist == 10000 || length( currentContact->resolution) > length(vel) + 1 )
				{
					//resolve
				//	if( resolveDist == 10000kmkkkkkkkkkkkkkkkkk )
				//		cout << "weird case" << endl;
			//		elsekkkkkkkkk
					cout << "formally an error: " 
						<< currentContact->resolution.x << ", " << currentContact->resolution.y << endl;


					return NULL;
				}

				
				assert( resolveDist != 10000 );
				
				double pri = length( vel + currentContact->resolution );
			
				currentContact->collisionPriority = pri;
				if( intersectQuantity < 0 )
				{
					currentContact->position = e->v0;
				}
				else
				{
					currentContact->position = e->v1;
				}
				currentContact->edge = e;
				return currentContact;
				//cout << "here!!!: " << currentContact->resolution.x << ", "
				//	<< currentContact->resolution.y << endl;
			}
			else if( approxEquals(intersectQuantity,0) )
			{
				cout << "aa" << endl;
			//	cout << "new case: " << edgeNormal.x << ", " << edgeNormal.y << ".. vel: " << vel.x << ", " << vel.y << endl;
				//if( approxEquals(intersectQuantity,0) )
					currentContact->position = e->v0;
				//else 
				
				double resolveDist = 10000;
				double resolveLeft = 10000;
				double resolveRight = 10000;
				double resolveTop = 10000;
				double resolveBottom = 10000;

				if( edgeNormal.x < 0 && edgeNormal.y < 0 && right >= edgeLeft && vel.x > 0  )
				{
					
					//	cout << "aa 1" << endl;
						//resolveLeft = (right - edgeLeft)
						currentContact->resolution = normalize(-vel) * (right - edgeLeft) ;//V2d( 0, 0 );
						//cout << "oneeee: " << currentContact->resolution.x << ", " << currentContact->resolution.y << endl;
					
				}
				else if( edgeNormal.x < 0 && edgeNormal.y > 0 && edgeBottom >= top && vel.y < 0  )
				{
					
						//cout << "aa 2" << endl;
						currentContact->resolution = normalize(-vel) * (edgeBottom - top) ;//V2d( 0, 0 );
						//cout << "oneeee: " << currentContact->resolution.x << ", " << currentContact->resolution.y << endl;
					
				}
				else if( edgeNormal.x > 0 && edgeNormal.y > 0 && edgeRight >= left && vel.x < 0  )
				{
					//cout << "aa 3" << endl;
						
						currentContact->resolution = normalize(-vel) * (edgeRight - left) ;//V2d( 0, 0 );
					//	cout << "oneeee: " << currentContact->resolution.x << ", " << currentContact->resolution.y << endl;
					
				}
				else if( edgeNormal.x > 0 && edgeNormal.y < 0 && bottom >= edgeTop && vel.y > 0  )
				{
				//	cout << "aa 4" << endl;
						
						currentContact->resolution = normalize(-vel) * (bottom - edgeTop) ;//V2d( 0, 0 );
					//	cout << "oneeee: " << currentContact->resolution.x << ", " << currentContact->resolution.y << endl;
					
				}
				else
				{
					currentContact->resolution = V2d(0,0);
					return NULL;
				}
				if( length( currentContact->resolution ) == 0)
				{
					return NULL;
				}

				/*else if( edgeNormal.x > 0 && left <= edgeRight && vel.x <= 0 )
				{
					//if( )kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk
					{
						currentContact->resolution = normalize(-vel) * ( edgeRight - left);//V2d( 0, 0 );
						cout << "twoooo: " << currentContact->resolution.x << ", " << currentContact->resolution.y << endl;
						cout << "twwwo vel : " << vel.x << ", " << vel.y << endl;
					}
				}
				else if( edgeNormal.y < 0 && bottom >= edgeTop && vel.y > 0 )
				{
					//if( )kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk
					{
						currentContact->resolution = normalize(-vel) * ( bottom - edgeTop);//V2d( 0, 0 );
						cout << "threeee: " << currentContact->resolution.x << ", " << currentContact->resolution.y << endl;
						cout << "threee vel : " << vel.x << ", " << vel.y << endl;
					}
				}
				else if( edgeNormal.y > 0 && edgeBottom >= top && vel.y < 0 )
				{
					//if( )kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk
					{
						currentContact->resolution = normalize(-vel) * ( edgeBottom - top );//V2d( 0, 0 );
						cout << "fourrrr: " << currentContact->resolution.x << ", " << currentContact->resolution.y << endl;
						cout << "fourr vel: " << vel.x << ", " << vel.y << endl;
					}
				}
				else
					assert( false && "sucks to be u" );*/

				
						
			}
			else if( approxEquals(intersectQuantity ,length( e->v1 - e->v0 )) )
			{
				cout << "bb" << endl;
				currentContact->position = e->v1;
				//cout << "nutso" << endl;

			/*	if( edgeNormal.x < 0 && right >= edgeLeft && vel.x > 0  )
				{
					
						
						currentContact->resolution = normalize(-vel) * (right - edgeLeft) ;//V2d( 0, 0 );
						cout << "oneeee: " << currentContact->resolution.x << ", " << currentContact->resolution.y << endl;
					
				}
				else if( edgeNormal.x > 0 && left <= edgeRight && vel.x < 0 )
				{
					//if( )kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk
					{
						currentContact->resolution = normalize(-vel) * ( edgeRight - left);//V2d( 0, 0 );
						cout << "twoooo: " << currentContact->resolution.x << ", " << currentContact->resolution.y << endl;
						cout << "twwwo vel : " << vel.x << ", " << vel.y << endl;
					}
				}
				else if( edgeNormal.y < 0 && bottom >= edgeTop && vel.y > 0 )
				{
					//if( )kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk
					{
						currentContact->resolution = normalize(-vel) * ( bottom - edgeTop);//V2d( 0, 0 );
						cout << "threeee: " << currentContact->resolution.x << ", " << currentContact->resolution.y << endl;
						cout << "threee vel : " << vel.x << ", " << vel.y << endl;
					}
				}
				else if( edgeNormal.y > 0 && edgeBottom >= top && vel.y < 0 )
				{
					//if( )kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk
					{
						currentContact->resolution = normalize(-vel) * ( edgeBottom - top );//V2d( 0, 0 );
						cout << "fourrrr1: " << currentContact->resolution.x << ", " << currentContact->resolution.y << endl;
						cout << "fourr vel: " << vel.x << ", " << vel.y << endl;
					}
				}
				else
					assert( false && "sucks to be u" );*/

				if( edgeNormal.x < 0 && edgeNormal.y < 0 && bottom >= edgeTop && vel.y > 0  )
				{
					//cout << "bb 1" << endl;
						
						currentContact->resolution = normalize(-vel) * (bottom - edgeTop) ;//V2d( 0, 0 );
						//cout << "oneeee: " << currentContact->resolution.x << ", " << currentContact->resolution.y << endl;
					
				}
				else if( edgeNormal.x < 0 && edgeNormal.y > 0 && right >= edgeLeft && vel.x > 0  )
				{
					//cout << "bb 2" << endl;
						
						currentContact->resolution = normalize(-vel) * (right - edgeLeft) ;//V2d( 0, 0 );
						//cout << "oneeee: " << currentContact->resolution.x << ", " << currentContact->resolution.y << endl;
					
				}
				else if( edgeNormal.x > 0 && edgeNormal.y > 0 && edgeBottom >= top && vel.y < 0  )
				{
					//cout << "bb 3" << endl;
						
						currentContact->resolution = normalize(-vel) * (edgeBottom - top) ;//V2d( 0, 0 );
					//	cout << "oneeee: " << currentContact->resolution.x << ", " << currentContact->resolution.y << endl;
					
				}
				else if( edgeNormal.x > 0 && edgeNormal.y < 0 && edgeRight >= left && vel.x <= 0  )
				{
					//cout << "bb 4" << endl;
						currentContact->resolution = normalize(-vel) * (edgeRight - left);//V2d( 0, 0 );
					//	cout << "oneeee: " << currentContact->resolution.x << ", " << currentContact->resolution.y << endl;
					
				}

				else
				{
					currentContact->resolution = V2d(0,0);
					
				}

				if( length( currentContact->resolution ) == 0)
				{
					return NULL;
				}
			}
			else
			{
				//cout << "else case: " << intersectQuantity << ", " << length( e->v1 - e->v0 ) << endl;
			//cout << "else case" << endl;
				currentContact->resolution = e->GetPoint( intersectQuantity ) - corner;
			}
		//	cout << "intersect quantity: " << intersectQuantity << ", length: " << length( e->v1 - e->v0 ) << endl;
		//	cout << "inter: " << intersect.x << ", " << intersect.y << endl;
		//	cout << "corner: " << corner.x << ", " << corner.y << endl;
		//	cout << "v0: " << e->v0.x << ", " << e->v0.y << endl;
			

			if( length( currentContact->resolution ) > length( vel ) + 1 )
			{
				cout << "resolution: " << currentContact->resolution.x << ", " << currentContact->resolution.y << endl;
		//		cout << "vel: " << vel.x << ", " << vel.y << endl;
				
				cout << "returning null--" << endl;	
				return NULL;
			}
			else
			{
				//cout << "what: " << currentContact->resolution.x << ", " << currentContact->resolution.y << endl;
			}

			double pri = dot( intersect - ( corner - vel ), normalize( vel ) );

			if( pri < -1 )
			{
			cout << "BUSTED--------------- " << edgeNormal.x << ", " << edgeNormal.y  << ", " << pri  << endl;
				//return NULL;
			}

			intersectQuantity = e->GetQuantity( intersect );
			
			currentContact->position = collisionPosition;
			
			currentContact->collisionPriority = pri;
			currentContact->edge = e;

			return currentContact;

		}

		
	}

	return NULL;
}