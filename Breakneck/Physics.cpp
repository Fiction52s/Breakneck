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

bool CollisionBox::Intersects( CollisionBox &c )
{
	//first, box with box aabb. can adjust it later
	if( c.isCircle && this->isCircle )
	{
		double dist = length( this->globalPosition - c.globalPosition );
		//cout << "dist: " << dist << endl;
		if( dist <= this->rw + c.rw )
			return true;
	}
	else if( c.isCircle && !this->isCircle )
	{
		V2d pA = globalPosition + V2d( -rw * cos( globalAngle ) + -rh * sin( globalAngle ), -rw * -sin( globalAngle ) + -rh * cos( globalAngle ) );
		V2d pB = globalPosition + V2d( rw * cos( globalAngle ) + -rh * sin( globalAngle ), rw * -sin( globalAngle ) + -rh * cos( globalAngle ) );
		V2d pC = globalPosition + V2d( rw * cos( globalAngle ) + rh * sin( globalAngle ), rw * -sin( globalAngle ) + rh * cos( globalAngle ) );
		V2d pD = globalPosition + V2d( -rw * cos( globalAngle ) + rh * sin( globalAngle ), -rw * -sin( globalAngle ) + rh * cos( globalAngle ) );
		
		double A = cross( c.globalPosition - pA, normalize(pB - pA) );
		double B = cross( c.globalPosition - pB, normalize(pC - pB) );
		double C = cross( c.globalPosition - pC, normalize(pD - pC) );
		double D = cross( c.globalPosition - pD, normalize(pA - pD) );

		if( A <= c.rw && B <= c.rw && C <= c.rw && D <= c.rw )
		{
			return true;
		}


		return false;
	}
	else if( !c.isCircle && this->isCircle )
	{
		V2d pA = c.globalPosition + V2d( -c.rw * cos( c.globalAngle ) + -c.rh * sin( c.globalAngle ), -c.rw * -sin( c.globalAngle ) + -c.rh * cos( c.globalAngle ) );
		V2d pB = c.globalPosition + V2d( c.rw * cos( c.globalAngle ) + -c.rh * sin( c.globalAngle ), c.rw * -sin( c.globalAngle ) + -c.rh * cos( c.globalAngle ) );
		V2d pC = c.globalPosition + V2d( c.rw * cos( c.globalAngle ) + c.rh * sin( c.globalAngle ), c.rw * -sin( c.globalAngle ) + c.rh * cos( c.globalAngle ) );
		V2d pD = c.globalPosition + V2d( -c.rw * cos( c.globalAngle ) + c.rh * sin( c.globalAngle ), -c.rw * -sin( c.globalAngle ) + c.rh * cos( c.globalAngle ) );
		
		double A = cross( globalPosition - pA, normalize(pB - pA) );
		double B = cross( globalPosition - pB, normalize(pC - pB) );
		double C = cross( globalPosition - pC, normalize(pD - pC) );
		double D = cross( globalPosition - pD, normalize(pA - pD) );

		//cout << "a: " << a << ", b: " << b << ", c: " << c << ", d: " << d << ", rw: " << rw << endl;

		if( A <= rw && B <= rw && C <= rw && D <= rw )
		{
			return true;
		}


		return false;
	}
	else //both are boxes
	{
		V2d pA0 = globalPosition + V2d( -rw * cos( globalAngle ) + -rh * sin( globalAngle ), -rw * -sin( globalAngle ) + -rh * cos( globalAngle ) );
		V2d pB0 = globalPosition + V2d( rw * cos( globalAngle ) + -rh * sin( globalAngle ), rw * -sin( globalAngle ) + -rh * cos( globalAngle ) );
		V2d pC0 = globalPosition + V2d( rw * cos( globalAngle ) + rh * sin( globalAngle ), rw * -sin( globalAngle ) + rh * cos( globalAngle ) );
		V2d pD0 = globalPosition + V2d( -rw * cos( globalAngle ) + rh * sin( globalAngle ), -rw * -sin( globalAngle ) + rh * cos( globalAngle ) );

		V2d pA1 = c.globalPosition + V2d( -c.rw * cos( c.globalAngle ) + -c.rh * sin( c.globalAngle ), -c.rw * -sin( c.globalAngle ) + -c.rh * cos( c.globalAngle ) );
		V2d pB1 = c.globalPosition + V2d( c.rw * cos( c.globalAngle ) + -c.rh * sin( c.globalAngle ), c.rw * -sin( c.globalAngle ) + -c.rh * cos( c.globalAngle ) );
		V2d pC1 = c.globalPosition + V2d( c.rw * cos( c.globalAngle ) + c.rh * sin( c.globalAngle ), c.rw * -sin( c.globalAngle ) + c.rh * cos( c.globalAngle ) );
		V2d pD1 = c.globalPosition + V2d( -c.rw * cos( c.globalAngle ) + c.rh * sin( c.globalAngle ), -c.rw * -sin( c.globalAngle ) + c.rh * cos( c.globalAngle ) );

		//finish this up!

	}
	return false;
}

void CollisionBox::DebugDraw( sf::RenderTarget *target )
{
	if( isCircle )
	{
		CircleShape cs;
		//cs.setFillColor( Color( 255, 0, 0, 255 ) );

		if( type == Physics )
		{
			cs.setFillColor( Color( 255, 0, 0, 100 ) );
		}
		else if( type == Hit )
		{
			cs.setFillColor( Color( 0, 255, 0, 100 ) );
		}
		else if( type == Hurt )
		{
			cs.setFillColor( Color( 0, 0, 255, 100 ) );
		}

		cs.setRadius( rw );
		cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
		cs.setPosition( globalPosition.x, globalPosition.y );

		target->draw( cs );

	}
	else
	{
		V2d pos = globalPosition;
		double angle = globalAngle;
		sf::RectangleShape r;
		if( type == Physics )
		{
			r.setFillColor( Color( 255, 0, 0, 100 ) );
		}
		else if( type == Hit )
		{
			r.setFillColor( Color( 0, 255, 0, 100 ) );
		}
		else if( type == Hurt )
		{
			r.setFillColor( Color( 0, 0, 255, 100 ) );
		}
		
		r.setSize( sf::Vector2f( rw * 2, rh * 2 ) );
		r.setOrigin( r.getLocalBounds().width / 2, r.getLocalBounds().height / 2 );
		r.setRotation( angle / PI * 180 );
		
		r.setPosition( globalPosition.x, globalPosition.y );


		target->draw( r );

	}
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

Contact *Collider::collideEdge3( sf::Vector2<double> position, const CollisionBox &b, Edge *e, 
		const sf::Vector2<double> &vel )
{
	Vector2<double> oldPosition = position - vel;
	double left = position.x - b.rw;
	double right = position.x + b.rw;
	double top = position.y - b.rh;
	double bottom = position.y + b.rh;

	

	double oldLeft = oldPosition.x - b.rw;
	double oldRight = oldPosition.x + b.rw;
	double oldTop = oldPosition.y - b.rh;
	double oldBottom = oldPosition.y + b.rh;


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
			//	cout << "DAFHGSAIOHGEIWHGIWEHG" << endl;
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
			//	cout << "zzzzzzzzzzzzzzzzzzzzzzzzzzzzzz" << endl;
				opp.y = corner.y;
			}
		}

		double res = cross( corner - e->v0, e->v1 - e->v0 );
		double resOpp = cross( opp - e->v0, e->v1 - e->v0 );



	//	cout << "res: " << res << endl;
		double measureNormal = dot( edgeNormal, normalize(-vel) );
		if( res < 0 && resOpp > 0 && measureNormal > 0 && ( vel.x != 0 || vel.y != 0 )  )	
		{
			//cout << "vezzzzz: " << vel.x << ", " << vel.y << " .. norm: " << edgeNormal.x << ", " << edgeNormal.y << endl;
			Vector2<double> invVel = normalize(-vel);



			LineIntersection li = lineIntersection( corner, corner - (vel), e->v0, e->v1 );

			/*V2d cvdiff = normalize(corner - (corner - vel));
			V2d ediff = normalize(e->v1 - e->v0);
			double le = 200;
			sf::Vertex activePreview[4] =
			{
				sf::Vertex(sf::Vector2<float>((cvdiff.x * le + (corner - vel).x), (cvdiff.y * le + (corner - vel).y)), Color::Yellow ),
				sf::Vertex(sf::Vector2<float>((-cvdiff.x * le + (corner - vel).x), (-cvdiff.y * le + (corner - vel).y)), Color::Yellow),
				sf::Vertex(sf::Vector2<float>((ediff.x * le + e->v1.x), (ediff.y * le + e->v1.y)), Color::Red ),
				sf::Vertex(sf::Vector2<float>((-ediff.x * le + e->v1.x), (-ediff.y * le + e->v1.y)), Color::Red)
				//sf::Vertex(sf::Vector2<float>((e->v0 + e->Normal() * 10.0).x, (e->v0 + e->Normal() * 10.0).y), Color::Red ),
				//sf::Vertex(sf::Vector2<float>((e->v1 + e->Normal() * 10.0).x, (e->v1 + e->Normal() * 10.0).y), Color::Red )
			}; 
			w->draw( activePreview, 4, sf::Lines );*/
			//cout << "active preview: " << vel.x << ", " << vel.y << endl;

			double testing = dot( normalize( (corner-vel) - corner), normalize( e->v1 - e->v0 ));
			if( li.parallel || abs( testing ) == 1 )
			{
				//cout << "returning null1" << endl;
				return NULL;
			}
			Vector2<double> intersect = li.position;

			double intersectQuantity = e->GetQuantity( intersect );

			/*CircleShape cs;
			cs.setFillColor( Color::Cyan );
			cs.setRadius( 10 );
			cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
			cs.setPosition( intersect.x, intersect.y );
			w->draw( cs );*/

			Vector2<double> collisionPosition = intersect;
			//cout << "testing: " << dot( normalize( (corner-vel) - corner), normalize( e->v1 - e->v0 ))  << endl;
			if( intersectQuantity <= 0 || intersectQuantity >= length( e->v1 - e->v0 ) )
			{
			//	cout << "this option: " << intersectQuantity << ", " << length( e->v1 - e->v0 ) << endl;
			//	cout << "norm: " << e->Normal().x  << ", " << e->Normal().y << endl;
			//	cout << "bottom: " << bottom << ", edgeBottom: " << edgeTop << endl;
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
					
						resolveLeft = ( edgeRight - left ) / abs( vel.x );//dot( V2d(edgeRight - left, 0), normalize( -vel ) );// / abs(normalize(vel).x);
						if( resolveLeft  > 1.1  )
						resolveLeft  = 10000;

							cout << "temp resolveleft: " << resolveLeft << endl;
				}
				else if( right >= edgeLeft && vel.x > 0 )
				{
				//	if( vel.x == 0 )
				//		resolveRight = 0;
					resolveRight = (right - edgeLeft) / abs(vel.x );
					
					if( resolveRight > 1.1 )
						resolveRight = 10000;

					cout << "temp resolveright: " << resolveRight << endl;
					//dot( V2d((right - edgeLeft),0), normalize( -vel ) );//
				}
				else
				{
				//	cout << "left: " << vel.x << ", " << vel.y << endl;
				}

				if( top <= edgeBottom && vel.y < 0 )
				{
				//	cout << "choose T" << endl;
				//	if( vel.y == 0 )
				//		resolveTop = 0;
						
						resolveTop = (edgeBottom - top) / abs(vel.y) ;//abs(normalize(vel).y );//dot( V2d( 0, (edgeBottom - top)), normalize( -vel ) );// / abs(normalize(vel).y);// / abs(a.velocity.x);
						if( resolveTop > 1.1 )
							resolveTop = 10000;
						cout << "temp resolvetop: " << resolveTop << endl;
				}
				else if( bottom >= edgeTop && vel.y > 0 )
				{
				//	if( vel.y == 0 )
				//		resolveBottom  = 0;
				//	elsekkkkkkkkkkmkkkkkkkkkkkkkkkkkkkk
						resolveBottom = (bottom- edgeTop) / abs(vel.y) ;// abs(normalize(vel).y);// / abs(a.velocity.x);
						cout << "bottom preadjust =: " << resolveBottom << endl;
						if( resolveBottom > 1.1 )
							resolveBottom = 10000;

						cout << "temp resolvebottom: " << resolveBottom << endl;
					//dot( V2d( 0, (bottom - edgeTop)), normalize( -vel ) );//
				}
				else
				{
					//cout << "top: " << top << ", " << edgeTop << endl;
				}


				resolveDist = min( resolveTop, min( resolveBottom, min( resolveLeft, resolveRight) ) );
				//cout << "resolve dist: " << resolveDist << ", " << resolveBottom << endl;
				if( approxEquals( resolveDist, 0 ) )
				{
					cout << "returning nNULLL heree" << endl;
					//return NULL;
				}

				/*if( resolveDist == resolveTop ) cout << "T" << endl;
				else if( resolveDist == resolveBottom ) cout << "B" << endl;
				else if( resolveDist == resolveLeft ) cout << "L" << endl;
				else if( resolveDist == resolveRight ) cout << "R" << endl;*/
			



				

				currentContact->resolution = -vel * resolveDist;
				//cout << "resolution toooo: " << currentContact->resolution.x  << ", " << currentContact->resolution.y << endl;
			//	cout << "dist: " << resolveDist << endl;

				if( resolveDist == 10000 || length( currentContact->resolution) > length(vel) + 1 )
				{
					//resolve
				//	if( resolveDist == 10000kmkkkkkkkkkkkkkkkkk )
				//		cout << "weird case" << endl;
			//		elsekkkkkkkkk
					cout << "formally an error: " 
						<< currentContact->resolution.x << ", " << currentContact->resolution.y << endl;
					if( resolveDist != 10000 ) assert( false && "thought I had it" );

					return NULL;
				}

				
				assert( resolveDist != 10000 );
				
				//dot( vel + currentContact->resolution, normalize( vel ) );
			
				if( intersectQuantity <= 0 )
				{
					currentContact->position = e->v0;
				}
				else
				{
					currentContact->position = e->v1;
				}

	//			double pri = dot( currentContact->position - ( corner - vel ), normalize( vel ) );
		//		double pri = dot( intersect - ( corner - vel ), normalize( vel ) );
				//double pri = dotcurrentContact->resolution			
				double pri;
				if( resolveDist == resolveLeft || resolveDist == resolveRight )
				{
					pri = -vel.x - resolveDist * vel.x;
				}
				else
				{
					pri = -vel.y - resolveDist * vel.y;
				}
				

				if( approxEquals( pri, 0 ) )
					pri = 0;


				cout << "pri 222: " << pri << endl;
				currentContact->collisionPriority = pri;


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
			//	cout << "else case: " << intersectQuantity << ", " << length( e->v1 - e->v0 ) << endl;
			//cout << "else case" << endl;
				currentContact->resolution = e->GetPoint( intersectQuantity ) - corner;
			}
		

			if( length( currentContact->resolution ) > length( vel ) + 1 )
			{
				cout << "resolution: " << currentContact->resolution.x << ", " << currentContact->resolution.y << endl;
				cout << "vel: " << vel.x << ", " << vel.y << endl;
				
				cout << "returning null--" << endl;	
			//	return NULL;
			}
			else
			{
				//cout << "what: " << currentContact->resolution.x << ", " << currentContact->resolution.y << endl;
			}

			double pri = dot( intersect - ( corner - vel ), normalize( vel ) );

			if( approxEquals( pri, 0 ) )
					pri = 0;

			cout << "pri 111: " << pri << endl;
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

Contact *Collider::collideEdge2( sf::Vector2<double> position, const CollisionBox &b, Edge *e, 
		const sf::Vector2<double> &vel )
{
	Vector2<double> oldPosition = position - vel;
	double left = position.x - b.rw;
	double right = position.x + b.rw;
	double top = position.y - b.rh;
	double bottom = position.y + b.rh;

	

	double oldLeft = oldPosition.x - b.rw;
	double oldRight = oldPosition.x + b.rw;
	double oldTop = oldPosition.y - b.rh;
	double oldBottom = oldPosition.y + b.rh;


	double edgeLeft = min( e->v0.x, e->v1.x );
	double edgeRight = max( e->v0.x, e->v1.x ); 
	double edgeTop = min( e->v0.y, e->v1.y ); 
	double edgeBottom = max( e->v0.y, e->v1.y ); 

	bool aabbCollision = false;

	if( left <= edgeRight && right >= edgeLeft && top <= edgeBottom && bottom >= edgeTop )
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
			/*if( edgeLeft <= left )
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
			//	cout << "DAFHGSAIOHGEIWHGIWEHG" << endl;
				corner.x = (edgeLeft + edgeRight) / 2;
				opp.x = corner.x;
			}*/
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
			/*if( edgeTop <= top )
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
			//	cout << "zzzzzzzzzzzzzzzzzzzzzzzzzzzzzz" << endl;
				opp.y = corner.y;
			}*/
		}

		if( corner.x == 0 && corner.y == 0 )
		{
			return NULL;
		}

		double distIntersected = -cross( corner - e->v0, normalize( e->v1 - e->v0 ) ); //distance corner is inside solid
		//double resOpp = cross( opp - e->v0, e->v1 - e->v0 );
		double measureNormal = dot( edgeNormal, normalize(-vel) ); //make sure you are traveling at the edge

		bool isMovingAtAll = ( vel.x != 0 || vel.y != 0 );
		//cout << "res: " << res << endl;
		//&& resOpp > 0 
		
		if( distIntersected > 0 && distIntersected <= length( vel ) && measureNormal > 0 && isMovingAtAll )	
		{
			Vector2<double> revDir = normalize(-vel);
			
			Vector2<double> intersect = corner + revDir * distIntersected;

			double intersectQuantity = dot( corner - e->v0, normalize( e->v1 - e->v0 ) );
			
			bool borderPre = intersectQuantity <= 0;
			bool borderPost = intersectQuantity >= length( e->v1 - e->v0 );
			double resolveTop = 10000; //push up
			double resolveBottom = 10000; //push down
			double resolveRight = 10000;
			double resolveLeft = 10000;

			if( borderPre || borderPost ) //hit the intersection of more than 1 edge
			{
				if( corner.x >= edgeLeft && vel.x > 0 )
				{
					double distX = corner.x - edgeLeft;
					resolveLeft = distX / vel.x;
				}
				else if( corner.x <= edgeRight && vel.x < 0 )
				{
					double distX = edgeRight - corner.x;
					resolveRight = distX / -vel.x;
				}

				if( corner.y >= edgeTop && vel.y > 0 )
				{
					double distY = corner.y - edgeTop;
					resolveTop = distY / vel.y;
				}
				else if( corner.y <= edgeBottom && vel.y < 0 )
				{
					double distY = edgeBottom - corner.y;
					resolveBottom = distY / -vel.y;
				}

				double resolveDist = min( resolveTop, min( resolveBottom, min( resolveLeft, resolveRight ) ) );

				assert( resolveDist != 10000 );

				currentContact->resolution = resolveDist * -vel;
				currentContact->edge = e;

				if( borderPre )
				{
					currentContact->position = e->v0;
				}
				else //borderPost
				{
					currentContact->position = e->v1;
				}

				if( resolveDist == resolveTop )
				{
					currentContact->normal = V2d( 0, -1 );
				}
				else if( resolveDist == resolveBottom )
				{
					currentContact->normal = V2d( 0, 1 );
				}
				else if( resolveDist == resolveLeft )
				{
					currentContact->normal = V2d( -1, 0 );
				}
				else if( resolveDist == resolveRight )
				{
					currentContact->normal = V2d( 1, 0 );
				}


				CircleShape *cs = new CircleShape;
				cs->setFillColor( Color::Cyan );
				cs->setRadius( 10 );
				cs->setOrigin( cs->getLocalBounds().width / 2, cs->getLocalBounds().height / 2 );
				cs->setPosition( currentContact->position.x, currentContact->position.y );
				progressDraw.push_back( cs );

				currentContact->collisionPriority = length( vel ) - distIntersected;
				cout << "dist int: " << distIntersected << ", " << length( vel ) << endl;
				cout << "rev dir: " << revDir.x << ", " << revDir.y << endl;
				cout << "contact: " << currentContact->resolution.x << ", " << currentContact->resolution.y << 
					", normal: " << currentContact->edge->Normal().x << ", " 
					<< currentContact->edge->Normal().y << ", pri: " << currentContact->collisionPriority << endl;
				return currentContact;
			}
			else //hit only a single edge
			{
				currentContact->resolution = intersect - corner;
				currentContact->collisionPriority = 1;
				currentContact->edge = e;
				currentContact->position = intersect;

				CircleShape *cs = new CircleShape;
				cs->setFillColor( Color::Red );
				cs->setRadius( 10 );
				cs->setOrigin( cs->getLocalBounds().width / 2, cs->getLocalBounds().height / 2 );
				cs->setPosition( currentContact->position.x, currentContact->position.y );
				progressDraw.push_back( cs );

				return currentContact;
			}
		}

	

	}
	return NULL;
}

Contact * Collider::collideEdge( V2d position, const CollisionBox &b, Edge *e, const V2d &vel )
{
	if( b.isCircle )
	{
		V2d oldPosition = position - vel;

		V2d v0 = e->v0;
		V2d v1 = e->v1;

		double edgeLength = length( v1 - v0 );
		double radius = b.rw;
		V2d edgeNormal = e->Normal();

		double lineQuantity = dot( position - v0, normalize( v1 - v0 ) );
		double dist = cross( position - v0, normalize( v1 - v0 ) );
		
		
		if( dot( -vel, edgeNormal ) > 0 && dist >= 0 && dist <= radius )
		{
		if( lineQuantity >= 0 && lineQuantity <= edgeLength ) //point is on the circle in the dir of the ege normal
		{
			LineIntersection li = lineIntersection( oldPosition + radius * -edgeNormal, position
				+ radius * -edgeNormal, e->v0, e->v1 );


			//double testing = dot( normalize( (corner-vel) - corner), normalize( e->v1 - e->v0 ));
			if( li.parallel )//|| abs( testing ) == 1 )
			{
				cout << "returning circle null1" << endl;
				return NULL;
			}


			Vector2<double> intersect = li.position;


			//double intersectQuantity = e->GetQuantity( intersect );


			V2d newPosition = intersect + radius * edgeNormal;

			currentContact->resolution = newPosition - position;
			currentContact->edge = e;
			currentContact->normal= edgeNormal;
			currentContact->position = e->GetPoint( lineQuantity );
			currentContact->collisionPriority = length( intersect - ( oldPosition + radius * -edgeNormal ) );

			CircleShape *cs = new CircleShape;
			cs->setFillColor( Color::Cyan );
			cs->setRadius( 10 );
			cs->setOrigin( cs->getLocalBounds().width / 2, cs->getLocalBounds().height / 2 );
			cs->setPosition( intersect.x, intersect.y );

			progressDraw.push_back( cs );

			return currentContact;
		}
		else //special side/hit case for colliding with points
		{
			//use right triangeles from the vertex to the circle point and cross product to figure out the y. then use
			//radius and the y to find the x value which is the value along the velocity that you should go until you
			//collide. thats how u get resolution here and other stuff. don't need it for this build so do it later
			//if( 
			return NULL;
		}
		}

		return NULL;
	}
	else
	{
	Vector2<double> oldPosition = position - vel;
	double left = position.x - b.rw;
	double right = position.x + b.rw;
	double top = position.y - b.rh;
	double bottom = position.y + b.rh;

	

	double oldLeft = oldPosition.x - b.rw;
	double oldRight = oldPosition.x + b.rw;
	double oldTop = oldPosition.y - b.rh;
	double oldBottom = oldPosition.y + b.rh;


	double edgeLeft = min( e->v0.x, e->v1.x );
	double edgeRight = max( e->v0.x, e->v1.x ); 
	double edgeTop = min( e->v0.y, e->v1.y ); 
	double edgeBottom = max( e->v0.y, e->v1.y ); 

	bool aabbCollision = false;
	if( left <= edgeRight && right >= edgeLeft && top <= edgeBottom && bottom >= edgeTop )
	{	
		
		aabbCollision = true;
	}

	/*Vector2<double> half = (oldPosition + position ) / 2.0;
		double halfLeft = half.x - b.rw;
		double halfRight = half.x + b.rw;
		double halfTop = half.y - b.rh;
		double halfBottom = half.y + b.rh;

	if( halfLeft <= edgeRight && halfRight >= edgeLeft && halfTop <= edgeBottom && halfBottom >= edgeTop )
		{
			aabbCollision = true;
		}*/

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
			//	cout << "DAFHGSAIOHGEIWHGIWEHG" << endl;
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
			//	cout << "zzzzzzzzzzzzzzzzzzzzzzzzzzzzzz" << endl;
				opp.y = corner.y;
			}
		}

		double res = cross( corner - e->v0, e->v1 - e->v0 );
		double resOpp = cross( opp - e->v0, e->v1 - e->v0 );



		//cout << "res: " << res << endl;
		double measureNormal = dot( edgeNormal, normalize(-vel) );
		if( res < -.001 && resOpp > 0 && measureNormal > 0 && ( vel.x != 0 || vel.y != 0 )  )	
		{
			
			//cout << "vezzzzz: " << vel.x << ", " << vel.y << " .. norm: " << edgeNormal.x << ", " << edgeNormal.y << endl;
			Vector2<double> invVel = normalize(-vel);



			LineIntersection li = lineIntersection( corner, corner - (vel), e->v0, e->v1 );


			//sf::Vertex activePreview[4] =
			//{
			//	sf::Vertex(sf::Vector2<float>((cvdiff.x * le + (corner - vel).x), (cvdiff.y * le + (corner - vel).y)), Color::Yellow ),
			//	sf::Vertex(sf::Vector2<float>((-cvdiff.x * le + (corner - vel).x), (-cvdiff.y * le + (corner - vel).y)), Color::Yellow),
			//	sf::Vertex(sf::Vector2<float>((ediff.x * le + e->v1.x), (ediff.y * le + e->v1.y)), Color::Red ),
			//	sf::Vertex(sf::Vector2<float>((-ediff.x * le + e->v1.x), (-ediff.y * le + e->v1.y)), Color::Red)
				//sf::Vertex(sf::Vector2<float>((e->v0 + e->Normal() * 10.0).x, (e->v0 + e->Normal() * 10.0).y), Color::Red ),
				//sf::Vertex(sf::Vector2<float>((e->v1 + e->Normal() * 10.0).x, (e->v1 + e->Normal() * 10.0).y), Color::Red )
			//}; 

	//		w->draw( activePreview, 4, sf::Lines );


			/*V2d cvdiff = normalize(corner - (corner - vel));
			V2d ediff = normalize(e->v1 - e->v0);
			double le = 200;
			
			//cout << "active preview: " << vel.x << ", " << vel.y << endl;
			*/
			double testing = dot( normalize( (corner-vel) - corner), normalize( e->v1 - e->v0 ));
			if( li.parallel || abs( testing ) == 1 )
			{
				//cout << "returning null1" << endl;
				return NULL;
			}
			Vector2<double> intersect = li.position;

			double intersectQuantity = e->GetQuantity( intersect );

			CircleShape *cs = new CircleShape;
			cs->setFillColor( Color::Cyan );
			cs->setRadius( 10 );
			cs->setOrigin( cs->getLocalBounds().width / 2, cs->getLocalBounds().height / 2 );
			cs->setPosition( intersect.x, intersect.y );

			progressDraw.push_back( cs );
			//w->draw( cs );
			//w->display();

			Vector2<double> collisionPosition = intersect;
			//cout << "testing: " << dot( normalize( (corner-vel) - corner), normalize( e->v1 - e->v0 ))  << endl;
			if( intersectQuantity <= 0 || intersectQuantity >= length( e->v1 - e->v0 ) )
			{
			//	cout << "this option: " << intersectQuantity << ", " << length( e->v1 - e->v0 ) << endl;
			//	cout << "norm: " << e->Normal().x  << ", " << e->Normal().y << endl;
			//	cout << "bottom: " << bottom << ", edgeBottom: " << edgeTop << endl;
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
					
						resolveLeft = ( edgeRight - left ) / abs( vel.x );//dot( V2d(edgeRight - left, 0), normalize( -vel ) );// / abs(normalize(vel).x);
						if( resolveLeft  > 1.1  )
							resolveLeft  = 10000;

						//	cout << "temp resolveleft: " << resolveLeft << endl;
				}
				else if( right >= edgeLeft && vel.x > 0 )
				{
				//	if( vel.x == 0 )
				//		resolveRight = 0;
					resolveRight = (right - edgeLeft) / abs(vel.x );
					

					//cout << "temp resolveright: " << resolveRight << ", normal: " << e->Normal().x << ", " << e->Normal().y << endl;
					if( resolveRight > 1.1 )
						resolveRight = 10000;

					
					//dot( V2d((right - edgeLeft),0), normalize( -vel ) );//
				}
				else
				{
				//	cout << "left: " << vel.x << ", " << vel.y << endl;
				}

				if( top <= edgeBottom && vel.y < 0 )
				{
				//	cout << "choose T" << endl;
				//	if( vel.y == 0 )
				//		resolveTop = 0;
						
						resolveTop = (edgeBottom - top) / abs(vel.y) ;//abs(normalize(vel).y );//dot( V2d( 0, (edgeBottom - top)), normalize( -vel ) );// / abs(normalize(vel).y);// / abs(a.velocity.x);
						if( resolveTop > 1.1 )
							resolveTop = 10000;
					//	cout << "temp resolvetop: " << resolveTop << endl;
				}
				else if( bottom >= edgeTop && vel.y > 0 )
				{
				//	if( vel.y == 0 )
				//		resolveBottom  = 0;
				//	elsekkkkkkkkkkmkkkkkkkkkkkkkkkkkkkk
						resolveBottom = (bottom- edgeTop) / abs(vel.y) ;// abs(normalize(vel).y);// / abs(a.velocity.x);
				//		cout << "bottom preadjust =: " << resolveBottom << endl;
						if( resolveBottom > 1.1 )
							resolveBottom = 10000;

					//	cout << "temp resolvebottom: " << resolveBottom << endl;
					//dot( V2d( 0, (bottom - edgeTop)), normalize( -vel ) );//
				}
				else
				{
					//cout << "top: " << top << ", " << edgeTop << endl;
				}


				resolveDist = min( resolveTop, min( resolveBottom, min( resolveLeft, resolveRight) ) );
				//cout << "resolve dist: " << resolveDist << ", " << resolveBottom << endl;
				if( approxEquals( resolveDist, 0 ) )
				{
					cout << "returning nNULLL heree" << endl;
					//return NULL;
				}

				/*if( resolveDist == resolveTop ) cout << "T" << endl;
				else if( resolveDist == resolveBottom ) cout << "B" << endl;
				else if( resolveDist == resolveLeft ) cout << "L" << endl;
				else if( resolveDist == resolveRight ) cout << "R" << endl;*/
			



				
				currentContact->resolution = -vel * resolveDist;
				//currentContact->resolution = normalize(-vel) * resolveDist;
				//cout << "resolution toooo: " << currentContact->resolution.x  << ", " << currentContact->resolution.y << endl;
			//	cout << "dist: " << resolveDist << endl;

				if( resolveDist == 10000 || length( currentContact->resolution) > length(vel) + 1 )
				{
					//resolve
				//	if( resolveDist == 10000kmkkkkkkkkkkkkkkkkk )
				//		cout << "weird case" << endl;
			//		elsekkkkkkkkk
					cout << "formally an error: " 
						<< currentContact->resolution.x << ", " << currentContact->resolution.y << endl;
					if( resolveDist != 10000 ) assert( false && "thought I had it" );

					return NULL;
				}

				
				assert( resolveDist != 10000 );
				
				//dot( vel + currentContact->resolution, normalize( vel ) );
			
				if( intersectQuantity <= 0 )
				{
					currentContact->position = e->v0;
				}
				else
				{
					currentContact->position = e->v1;
				}

	//			double pri = dot( currentContact->position - ( corner - vel ), normalize( vel ) );
		//		double pri = dot( intersect - ( corner - vel ), normalize( vel ) );
				//double pri = dotcurrentContact->resolution			
				double pri;
				if( resolveDist == resolveLeft || resolveDist == resolveRight )
				{
					pri = -vel.x - resolveDist * vel.x;
				}
				else
				{
					pri = -vel.y - resolveDist * vel.y;
				}
				

				if( approxEquals( pri, 0 ) )
					pri = 0;


				//cout << "pri 222: " << pri << endl;
				currentContact->collisionPriority = pri;


				currentContact->edge = e;
				return currentContact;
				//cout << "here!!!: " << currentContact->resolution.x << ", "
				//	<< currentContact->resolution.y << endl;
			}
			else if( approxEquals(intersectQuantity,0) )
			{
			//	cout << "aa" << endl;
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
				//cout << "bb" << endl;
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
			//	cout << "else case: " << intersectQuantity << ", " << length( e->v1 - e->v0 ) << endl;
			//cout << "else case" << endl;
				currentContact->resolution = e->GetPoint( intersectQuantity ) - corner;
			}
		

			if( length( currentContact->resolution ) > length( vel ) + 1 )
			{
				cout << "resolution: " << currentContact->resolution.x << ", " << currentContact->resolution.y << endl;
				cout << "vel: " << vel.x << ", " << vel.y << endl;
				
				cout << "returning null--" << endl;	
			//	return NULL;
			}
			else
			{
				//cout << "what: " << currentContact->resolution.x << ", " << currentContact->resolution.y << endl;
			}

			double pri = dot( intersect - ( corner - vel ), normalize( vel ) );

			if( approxEquals( pri, 0 ) )
					pri = 0;

			//cout << "pri 111: " << pri << endl;
			if( pri < -1 )
			{
			//cout << "BUSTED--------------- " << edgeNormal.x << ", " << edgeNormal.y  << ", " << pri  << endl;
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
}

void Collider::DebugDraw( RenderTarget *target )
{
	for( list<Drawable*>::iterator it = progressDraw.begin(); it != progressDraw.end(); ++it )
	{
		target->draw( *(*it) );
	}
	//progressDraw.clear();
}

void Collider::ClearDebug()
{
	for( list<Drawable*>::iterator it = progressDraw.begin(); it != progressDraw.end(); ++it )
	{
		delete (*it);
	}
	progressDraw.clear();
}

EdgeParentNode::EdgeParentNode( const V2d &poss, double rww, double rhh )
{
	pos = poss;
	rw = rww;
	rh = rhh;
	leaf = false;
	children[0] = new EdgeLeafNode( V2d(pos.x - rw / 2.0, pos.y - rh / 2.0), rw / 2.0, rh / 2.0 );
	children[1] = new EdgeLeafNode( V2d(pos.x + rw / 2.0, pos.y - rh / 2.0), rw / 2.0, rh / 2.0 );
	children[2] = new EdgeLeafNode( V2d(pos.x - rw / 2.0, pos.y + rh / 2.0), rw / 2.0, rh / 2.0 );
	children[3] = new EdgeLeafNode( V2d(pos.x + rw / 2.0, pos.y + rh / 2.0), rw / 2.0, rh / 2.0 );

	
}

EdgeLeafNode::EdgeLeafNode( const V2d &poss, double rww, double rhh )
	:objCount(0)
{
	pos = poss;
	rw = rww;
	rh = rhh;

	leaf = true;
	for( int i = 0; i < 4; ++i )
	{
		edges[i] = NULL;
	}
}

sf::Rect<double> GetEdgeBox( Edge *e )
{
	double left = min( e->v0.x, e->v1.x );
	double right = max( e->v0.x, e->v1.x );
	double top = min( e->v0.y, e->v1.y );
	double bottom = max( e->v0.y, e->v1.y );
	return sf::Rect<double>( left, top, right - left, bottom - top );	
}

bool IsEdgeTouchingBox( Edge *e, const sf::Rect<double> & ir )
{
	sf::Rect<double> er = GetEdgeBox( e );

	V2d as[4];
	V2d bs[4];
	as[0] = V2d( ir.left, ir.top );
	bs[0] = V2d( ir.left + ir.width, ir.top );

	as[1] =  V2d( ir.left, ir.top + ir.height );
	bs[1] = V2d( ir.left + ir.width, ir.top + ir.height );

	as[2] = V2d( ir.left, ir.top );
	bs[2] = V2d( ir.left, ir.top + ir.height);

	as[3] = V2d( ir.left + ir.width, ir.top );
	bs[3] = V2d( ir.left + ir.width, ir.top + ir.height );

	double erLeft = er.left;
	double erRight = er.left + er.width;
	double erTop = er.top;
	double erBottom = er.top + er.height;

	if( erLeft >= ir.left && erRight <= ir.left + ir.width && erTop >= ir.top && erBottom <= ir.top + ir.height )
		return true;
	//else
	//	return false;
	
	
	for( int i = 0; i < 4; ++i )
	{
		LineIntersection li = lineIntersection( as[i], bs[i], e->v0, e->v1 );

		if( !li.parallel )
		{
			
				V2d a = as[i];
				V2d b = bs[i];
				double e1Left = min( a.x, b.x );
				double e1Right = max( a.x, b.x );
				double e1Top = min( a.y, b.y );
				double e1Bottom = max( a.y, b.y );

				
			//cout << "compares: " << e1Left << ", " << erRight << " .. " << e1Right << ", " << erLeft << endl;
			//cout << "compares y: " << e1Top << " <= " << erBottom << " && " << e1Bottom << " >= " << erTop << endl;
			if( e1Left <= erRight && e1Right >= erLeft && e1Top <= erBottom && e1Bottom >= erTop )
			{
			//	cout << "---!!!!!!" << endl;
				if( (li.position.x < e1Right || approxEquals(li.position.x, e1Right) ) && ( li.position.x > e1Left || approxEquals(li.position.x, e1Left ) ) && ( li.position.y > e1Top || approxEquals( li.position.y, e1Top ) )&& ( li.position.y < e1Bottom || approxEquals( li.position.y, e1Bottom ) ) )
				{
				//	cout << "pos: " << li.position.x << ", " << li.position.y << endl;
				//	cout << "erlrud: " << erLeft << ", " << erRight << ", " << erTop << ", " << erBottom << endl;
					if( ( li.position.x < erRight || approxEquals( li.position.x, erRight )) && ( li.position.x > erLeft || approxEquals( li.position.x, erLeft ) ) && ( li.position.y > erTop || approxEquals( li.position.y, erTop ) ) && ( li.position.y < erBottom || approxEquals( li.position.y, erBottom ) ) )
					{
				//		cout << "seg intersect!!!!!!" << endl;
					//	assert( 0 );
						return true;
					}
				}
			}
		}
	}
	//cout << "return false" << endl;
	return false;
}

bool IsBoxTouchingBox( const sf::Rect<double> & r0, const sf::Rect<double> & r1 )
{
	bool test = r0.intersects( r1 );
	bool test2 =r0.left <= r1.left + r1.width 
		&& r0.left + r0.width >= r1.left 
		&& r0.top <= r1.top + r1.height
		&& r0.top + r0.height >= r1.top;
	/*if( test != test2 )
	{
		if( test )
			cout << "test is true" << endl;
		else
			cout << "test is false" << endl;
		cout << "r0: left: " << r0.left  << ", top: " << r0.top << ", w: " << r0.width << ", h: " << r0.height << endl;
		cout << "r1: left: " << r1.left  << ", top: " << r1.top << ", w: " << r1.width << ", h: " << r1.height << endl;
		cout << "wtf" << endl;
	}
	assert( test == test2 );*/
	return test2;
}

bool IsCircleTouchingCircle( V2d pos0, double rad0, V2d pos1, double rad1 )
{
	return length( pos1 - pos0 ) <= rad0 + rad1;
}

EdgeQNode *Insert( EdgeQNode *node, Edge* e )
{
	if( node->leaf )
	{
		EdgeLeafNode *n = (EdgeLeafNode*)node;
		if( n->objCount == 4 ) //full
		{
		//	cout << "splitting" << endl;	
			EdgeParentNode *p = new EdgeParentNode( n->pos, n->rw, n->rh );
			p->parent = n->parent;
			p->debug = n->debug;

		/*	for( int i = 0; i < 4; ++i )
			{
				//EdgeLeafNode *inner = (EdgeLeafNode*)p->children[i];
				Edge * tempEdge = n->edges[i];
				sf::IntRect nw( node->pos.x - node->rw, node->pos.y - node->rh, node->rw, node->rh);
				sf::IntRect ne( node->pos.x + node->rw, node->pos.y - node->rh, node->rw, node->rh );
				sf::IntRect sw( node->pos.x - node->rw, node->pos.y + node->rh, node->rw, node->rh );
				sf::IntRect se( node->pos.x + node->rw, node->pos.y + node->rh, node->rw, node->rh );

				if( IsEdgeTouchingBox( tempEdge, nw ) )
					p->children[0] = Insert( p->children[0], tempEdge );
				if( IsEdgeTouchingBox( tempEdge, ne ) )
					p->children[1] = Insert( p->children[1], tempEdge );
				if( IsEdgeTouchingBox( tempEdge, sw ) )
					p->children[2] = Insert( p->children[2], tempEdge );
				if( IsEdgeTouchingBox( tempEdge, se ) )
					p->children[3] = Insert( p->children[3], tempEdge );
			}*/

			for( int i = 0; i < 4; ++i )
			{
			//	cout << "test: " << n->edges[i]->Normal().x << ", " << n->edges[i]->Normal().y << endl;
				Insert( p, n->edges[i] );
			}


			delete node;

			 

			return Insert( p, e );
		}
		else
		{
		//	cout << "inserting into leaf . " << n->objCount << endl;
		//	cout << "norm: " << e->Normal().x << ", " << e->Normal().y << endl;
			n->edges[n->objCount] = e;
			++(n->objCount);
			return node;
		}
	}
	else
	{
	//	cout << "inserting into parent" << endl;
		EdgeParentNode *n = (EdgeParentNode*)node;
		sf::Rect<double> nw( node->pos.x - node->rw, node->pos.y - node->rh, node->rw, node->rh);
		sf::Rect<double> ne( node->pos.x, node->pos.y - node->rh, node->rw, node->rh );
		sf::Rect<double> sw( node->pos.x - node->rw, node->pos.y, node->rw, node->rh );
		sf::Rect<double> se( node->pos.x, node->pos.y, node->rw, node->rh );

		if( IsEdgeTouchingBox( e, nw ) )
		{
	//		cout << "calling northwest insert" << endl;
			n->children[0] = Insert( n->children[0], e );
		}
		if( IsEdgeTouchingBox( e, ne ) )
		{
	//		cout << "calling northeast insert" << endl;
			n->children[1] = Insert( n->children[1], e );
		}
		if( IsEdgeTouchingBox( e, sw ) )
		{
	//		cout << "calling southwest insert" << endl;
			n->children[2] = Insert( n->children[2], e );
		}
		if( IsEdgeTouchingBox( e, se ) )
		{
	//		cout << "calling southeast insert" << endl;
			n->children[3] = Insert( n->children[3], e );
		}
	}

	


	return node;
}

void DebugDrawQuadTree( sf::RenderWindow *w, EdgeQNode *node )
{
	//cout << "pos: " << node->pos.x << ", " << node->pos.y << " , rw: " << node->rw << ", rh: " << node->rh << endl;
	if( node->leaf )
	{
		EdgeLeafNode *n = (EdgeLeafNode*)node;

		sf::RectangleShape rs( sf::Vector2f( node->rw * 2, node->rh * 2 ) );
		int trans = 100;
		if( n->objCount == 0 )
			rs.setFillColor( Color( 100, 100, 100, trans ) ); //
		else if( n->objCount == 1 )
			rs.setFillColor( Color( 255, 0, 0, trans) ); // red == 1
		else if( n->objCount == 2 )
			rs.setFillColor( Color( 0, 255, 0, trans ) ); // green == 2
		else if( n->objCount == 3 )
			rs.setFillColor( Color( 0, 0, 255, trans ) ); //blue == 3
		else
		{
			rs.setFillColor( Color( 0, 100, 255, trans ) ); //blah == 4
		}
		
		//rs.setFillColor( Color::Green );
		//rs.setOutlineColor( Color::Blue );
	//	rs.setOutlineThickness( 3 );
		//rs.setFillColor( Color::Transparent );
		//rs.setPosition( node->pos.x - node->rw, node->pos.y - node->rh );
		rs.setOrigin( rs.getLocalBounds().width / 2.0, rs.getLocalBounds().height / 2.0 );
		//rs.setPosition( node->pos.x - node->rw, node->pos.y - node->rh );
		rs.setPosition( node->pos.x, node->pos.y );
		//rs.setOrigin( rs.getLocalBounds().width / 2.0, rs.getLocalBounds().height / 2.0 );

		w->draw( rs );

		CircleShape cs;
		cs.setFillColor( Color::Cyan );
		cs.setRadius( 1 );
		cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
		cs.setPosition( node->pos.x, node->pos.y );
		//w->draw( cs );
	}
	else
	{
		EdgeParentNode *n = (EdgeParentNode*)node;
		sf::RectangleShape rs( sf::Vector2f( node->rw * 2, node->rh * 2 ) );
		//rs.setOutlineColor( Color::Red );
		rs.setOrigin( rs.getLocalBounds().width / 2.0, rs.getLocalBounds().height / 2.0 );
		//rs.setPosition( node->pos.x - node->rw, node->pos.y - node->rh );
		rs.setPosition( node->pos.x, node->pos.y );
		rs.setFillColor( Color::Transparent );
		//rs.setOutlineThickness( 10 );
		w->draw( rs );

		CircleShape cs;
		cs.setFillColor( Color::Cyan );
		cs.setRadius( 1 );
		cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
		cs.setPosition( node->pos.x, node->pos.y );

		//w->draw( cs );

		for( int i = 0; i < 4; ++i )
			DebugDrawQuadTree( w, n->children[i] );
	}
	

	
}

void Query( EdgeQuadTreeCollider *qtc, EdgeQNode *node, const sf::Rect<double> &r )
{
	sf::Rect<double> nodeBox( node->pos.x - node->rw, node->pos.y - node->rh, node->rw * 2, node->rh * 2 );

	if( node->leaf )
	{
		EdgeLeafNode *n = (EdgeLeafNode*)node;

		if( IsBoxTouchingBox( r, nodeBox ) )
		{
			for( int i = 0; i < n->objCount; ++i )
			{
				qtc->HandleEdge( n->edges[i] );
			}
		}
	}
	else
	{
		//shouldn't this check for box touching box right here??
		EdgeParentNode *n = (EdgeParentNode*)node;

		if( r.intersects( nodeBox ) )
		{
			for( int i = 0; i < 4; ++i )
			{
				Query( qtc, n->children[i], r );
			}
		}
	}
	
}

/*void RayCast( RayCastHandler *handler, QNode *node, V2d startPoint, V2d endPoint )
{

	if( node->leaf )
	{
		EdgeLeafNode *n = (EdgeLeafNode*)node;

		Edge e;
		e.v0 = startPoint;
		e.v1 = endPoint;

		sf::Rect<double> nodeBox( node->pos.x - node->rw, node->pos.y - node->rh, node->rw * 2, node->rh * 2 );
	
		if( IsEdgeTouchingBox( &e, nodeBox ) )
		{
			for( int i = 0; i < n->objCount; ++i )
			{
				LineIntersection li = SegmentIntersect( startPoint, endPoint, n->edges[i]->v0, n->edges[i]->v1 );	
				if( !li.parallel )
				{
					handler->HandleRayCollision( n->edges[i], n->edges[i]->GetQuantity( li.position ), 
						dot( V2d( li.position - startPoint ), normalize( endPoint - startPoint ) ) );
				}
			}
		}
		
	}
	else
	{
		EdgeParentNode *n = (EdgeParentNode*)node;

		for( int i = 0; i < 4; ++i )
		{
			RayCast( handler, n->children[i], startPoint, endPoint );
		}
	}
}*/


//only works on edges
void RayCast( RayCastHandler *handler, QNode *node, V2d startPoint, V2d endPoint )
{
	if( node->leaf )
	{
		LeafNode *n = (LeafNode*)node;

		Edge e;
		e.v0 = startPoint;
		e.v1 = endPoint;

		sf::Rect<double> nodeBox( node->pos.x - node->rw, node->pos.y - node->rh, node->rw * 2, node->rh * 2 );
	
		if( IsEdgeTouchingBox( &e, nodeBox ) )
		{
			for( int i = 0; i < n->objCount; ++i )
			{
				LineIntersection li = SegmentIntersect( startPoint, endPoint, ((Edge*)(n->entrants[i]))->v0, ((Edge*)(n->entrants[i]))->v1 );	
				if( !li.parallel )
				{
					handler->HandleRayCollision( ((Edge*)(n->entrants[i])), ((Edge*)(n->entrants[i]))->GetQuantity( li.position ), 
						dot( V2d( li.position - startPoint ), normalize( endPoint - startPoint ) ) );
				}
			}
		}
		
	}
	else
	{
		ParentNode *n = (ParentNode*)node;

		for( int i = 0; i < 4; ++i )
		{
			RayCast( handler, n->children[i], startPoint, endPoint );
		}
	}
}

void Edge::HandleQuery( QuadTreeCollider * qtc )
{
	qtc->HandleEntrant( this );
}

bool Edge::IsTouchingBox( sf::Rect<double> &r )
{
	return IsEdgeTouchingBox( this, r );
}