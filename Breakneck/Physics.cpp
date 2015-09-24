#include "Physics.h"
#include "VectorMath.h"
#include <iostream>
#include <assert.h>
#include "GameSession.h"
#include "poly2tri/poly2tri.h"

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

//pathparam is local. pointsParam is local
MovingTerrain::MovingTerrain( GameSession *own, Vector2i pos, list<Vector2i> &pathParam, list<Vector2i> &pointsParam,
	bool loopP, float pspeed )
	:quadTree( NULL ), numEdges( 0 ), edgeArray( NULL ), loop( loopP ), speed( pspeed ), 
	position( pos.x, pos.y ), targetNode( 1 ), slowCounter( 1 ), slowMultiple( 1 ), owner( own )
{
	//cout << "pos is: " << pos.x << ", " << pos.y << endl;
	//set up path
	pathLength = pathParam.size() + 1;
	path = new Vector2i[pathLength];
	path[0] = pos;

	int index = 1;
	for( list<Vector2i>::iterator it = pathParam.begin(); it != pathParam.end(); ++it )
	{
		path[index] = (*it) + pos;
		++index;
	}


	//finalize edges and stuff
	list<Vector2i>::iterator it = pointsParam.begin();
	left = (*it).x;
	right = (*it).x;
	top = (*it).y;
	bottom = (*it).y;
	++it;

	for( ; it != pointsParam.end(); ++it )
	{
		if( (*it).x < left )
			left = (*it).x;
		if( (*it).x > right )
			right = (*it).x;
		if( (*it).y < top )
			top = (*it).y;
		if( (*it).y > bottom )
			bottom = (*it).y;
	}
	
	
	
	list<Edge*> edges;

	//could be smaller/more optimized
	quadTree = new QuadTree( right - left, bottom - top );

	list<Vector2i>::iterator last = pointsParam.end();
	--last;

	for( it = pointsParam.begin(); it != pointsParam.end(); ++it )
	{
		Edge *e = new Edge;
		e->v0 = V2d( (double)(*last).x, (double)(*last).y );
		e->v1 = V2d( (double)(*it).x, (double)(*it).y );
		edges.push_back( e );
		last = it;
	}

	//tempPoints.clear();

	//set up the quadtree and array
	numEdges = edges.size();
	edgeArray = new Edge*[numEdges];

	int i = 0;
	for( list<Edge*>::iterator eit = edges.begin(); eit != edges.end(); ++eit )
	{
		quadTree->Insert( (*eit) );
		edgeArray[i] = (*eit);
		++i;
	}

	//give the edges their links
	for( i = 0; i < numEdges; ++i )
	{
		if( i == 0 )
		{
			edgeArray[i]->edge0 = edgeArray[numEdges-1];
		}
		else
		{
			edgeArray[i]->edge0 = edgeArray[i-1];
		}

		if( i == numEdges - 1 )
		{
			edgeArray[i]->edge1 = edgeArray[0];
		}
		else
		{
			edgeArray[i]->edge1 = edgeArray[i+1];
		}
	}
	cout << "creating moving terrain with position: " << position.x << ", " << position.y << endl;

	vector<p2t::Point*> polyline;
	for( it = pointsParam.begin(); it != pointsParam.end(); ++it )
	{
		polyline.push_back( new p2t::Point( (*it).x + position.x, (*it).y + position.y ) );
	}

	p2t::CDT * cdt = new p2t::CDT( polyline );
	
	cdt->Triangulate();
	vector<p2t::Triangle*> tris;
	tris = cdt->GetTriangles();
	numTris = tris.size();
	polygonVA = new VertexArray( sf::Triangles , tris.size() * 3 );
	VertexArray & v = *polygonVA;
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
}

MovingTerrain::~MovingTerrain()
{
	delete quadTree;
	delete [] edgeArray;
}

void MovingTerrain::AddPoint( Vector2i p )
{
//	tempPoints.push_back( p );
}

void MovingTerrain::Finalize()
{
	
}

void MovingTerrain::Query( QuadTreeCollider *qtc, const sf::Rect<double> &r )
{
	sf::Rect<double> realR = r;
	realR.left -= position.x;
	realR.top -= position.y;
	quadTree->Query( qtc, realR );
}

void MovingTerrain::UpdatePhysics()
{
	//return;
	oldPosition = position;

	double movement = speed;
	
	/*if( PlayerSlowingMe() )
	{
		if( slowMultiple == 1 )
		{
			slowCounter = 1;
			slowMultiple = 5;
		}
	}
	else
	{
		slowMultiple = 1;
		slowCounter = 1;
	}*/

	//if( dead )
	//	return;


	movement /= (double)slowMultiple;

	while( movement != 0 )
	{
		V2d targetPoint = V2d( path[targetNode].x, path[targetNode].y );
		V2d diff = targetPoint - position;
		double len = length( diff );
		if( len >= abs( movement ) )
		{
			position += normalize( diff ) * movement;
			movement = 0;
		}
		else
		{
			position += diff;
			movement -= length( diff );
			AdvanceTargetNode();	
		}
	}

	vel = position - oldPosition;

	VertexArray &v = *polygonVA;
	for( int i = 0; i < numTris; ++i )
	{
		v[i*3].position += Vector2f( vel.x, vel.y );
		v[i*3+1].position += Vector2f( vel.x, vel.y );
		v[i*3+2].position += Vector2f( vel.x, vel.y );
	}
}

void MovingTerrain::AdvanceTargetNode()
{
	if( loop )
	{
		++targetNode;
		if( targetNode == pathLength )
			targetNode = 0;
	}
	else
	{
		if( forward )
		{
			++targetNode;
			if( targetNode == pathLength )
			{
				targetNode -= 2;
				forward = false;
			}
		}
		else
		{
			--targetNode;
			if( targetNode < 0 )
			{
				targetNode = 1;
				forward = true;
			}
		}
	}

//	cout << "new targetNode: " << targetNode << endl;
}

void MovingTerrain::DebugDraw( sf::RenderTarget *target )
{
	for( int i = 0; i < numEdges; ++i )
	{
		sf::CircleShape cs;
		cs.setFillColor( Color::Green );
		cs.setRadius( 20 );
		cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
		V2d realPos = position + edgeArray[i]->v0;
		V2d realv1 = position + edgeArray[i]->v1;
		cs.setPosition( realPos.x, realPos.y );
		//cout << i << ": " << realPos.x << ", " << realPos.y << endl;
		target->draw( cs );

		sf::Vertex line[] =
		{
			sf::Vertex(sf::Vector2f(realPos.x, realPos.y)),
			sf::Vertex(sf::Vector2f(realv1.x, realv1.y))
		};

		target->draw(line, 2, sf::Lines);
	}


}

void MovingTerrain::Draw( RenderTarget *target )
{
	sf::Rect<double> realRect( left + position.x, top + position.y, right - left, bottom - top );
	owner->UpdateTerrainShader( realRect );
	owner->polyShader.setParameter( "topLeft", owner->view.getCenter().x - owner->view.getSize().x / 2 - ( position.x - path[0].x ),
			owner->view.getCenter().y - owner->view.getSize().y / 2 - ( position.y - path[0].y ) );
	target->draw( *polygonVA, &owner->polyShader );
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
	:edge( NULL ), movingPlat( NULL )
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

	if( !aabbCollision )
	{
		return NULL;
	}

	
	
	Vector2<double> edgeNormal = e->Normal();
	if( edgeNormal.x == 0 || edgeNormal.y == 0 )
	{
		if( edgeNormal.x == 0 )
		{
			if( edgeNormal.y < 0 )
			{

			}
			else // .y > 0
			{
			}
		}
		else //edgeNormal.y == 0 
		{
		}
		//cout << "bad case" << endl;
	}
	else
	{
		V2d opp;
		Vector2<double> corner(0,0);
		bool cornerTest = false;

		if( edgeNormal.x > 0 )
		{
			corner.x = left;
			opp.x = right;
			cornerTest = true;
		}
		else if( edgeNormal.x < 0 )
		{
			corner.x = right;
			opp.x = left;
			cornerTest = true;
		}

		if( edgeNormal.y > 0 )
		{
			corner.y = top;
			opp.y = bottom;
			cornerTest = true;
		}
		else if( edgeNormal.y < 0 )
		{
			corner.y = bottom;
			opp.y = top;
			cornerTest = true;
		}

		assert( cornerTest );

		double distIntersected = -cross( corner - e->v0, normalize( e->v1 - e->v0 ) );
		double measureNormal = dot( edgeNormal, normalize(-vel) );
		bool isMovingAtAll = ( vel.x != 0 || vel.y != 0 );

		if( distIntersected > 0 /*&& distIntersected < length( vel ) + .1*/ && measureNormal > 0 && isMovingAtAll )	
		{
			Vector2<double> revDir = normalize(-vel);
			
			LineIntersection li = lineIntersection( corner, corner - (vel), e->v0, e->v1 );

			//double testing = dot( normalize( (corner-vel) - corner), normalize( e->v1 - e->v0 ));
			
			//assert( !li.parallel );
			if( li.parallel )
				return NULL;
			

			Vector2<double> intersect = li.position;

			double intersectQuantity = e->GetQuantity( intersect );
			
			//Vector2<double> intersect = corner + revDir * distIntersected;

			//double intersectQuantity = dot( corner - e->v0, normalize( e->v1 - e->v0 ) );
			
			bool borderPre = intersectQuantity <= 0;
			bool borderPost = intersectQuantity >= length( e->v1 - e->v0 ) + .1;

			double edgeLength = length( e->v1 - e->v0 );

			V2d returnNormal;
			V2d resolution( 1000, 1000 );
			double resolveFactor = 10000;

			if( borderPre || borderPost )
			{
				if( borderPre )
				{
					intersect = e->v0;
					intersectQuantity = 0;

					if( edgeNormal.y < 0 )
					{
						if( edgeNormal.x < 0 )
						{
							if( vel.x == 0 )
							{
								return NULL;
							}
							returnNormal = V2d( -1, 0 );
							resolveFactor = ( corner.x - intersect.x ) / abs( vel.x );
							cout << "up left: " << resolveFactor << endl;
						}
						else if( edgeNormal.x > 0 )
						{
							if( vel.y == 0 )
							{
								return NULL;
							}
							returnNormal = V2d( 0, -1 );
							resolveFactor = ( corner.y - intersect.y ) / abs( vel.y );
							cout << "up right: " << resolveFactor << endl;
						}
					}
					else //edgeNormal.y > 0 
					{
						if( edgeNormal.x < 0 )
						{
							if( vel.x == 0 )
							{
								return NULL;
							}
							returnNormal = V2d( 0, 1 );
							resolveFactor = ( intersect.y - corner.y ) / abs( vel.y );
							cout << "down left: " << resolveFactor << endl;
						}
						else if( edgeNormal.x > 0 )
						{
							if( vel.y == 0 )
							{
								return NULL;
							}
							returnNormal = V2d( 1, 0 );
							resolveFactor = ( intersect.x - corner.x ) / abs( vel.x );
							cout << "down right: " << resolveFactor << endl;
						}
					}


				}
				else //borderPost
				{
					intersect = e->v1;
					intersectQuantity = edgeLength;

					if( edgeNormal.y < 0 )
					{
						if( edgeNormal.x < 0 )
						{
							returnNormal = V2d( 0, -1 );
							resolveFactor = ( corner.y - intersect.y ) / abs( vel.y );
						}
						else if( edgeNormal.x > 0 )
						{
							returnNormal = V2d( 1, 0 );
							resolveFactor = ( intersect.x - corner.x ) / abs( vel.x );
						}
					}
					else //edgeNormal.y > 0 
					{
						if( edgeNormal.x < 0 )
						{
							returnNormal = V2d( -1, 0 );
							resolveFactor = ( corner.x - intersect.x ) / abs( vel.x );
						}
						else if( edgeNormal.x > 0 )
						{
							returnNormal = V2d( 0, 1 );
							resolveFactor = ( intersect.y - corner.y ) / abs( vel.y );
						}
					}
				}

				if( resolveFactor > 1 )
				{
					return NULL;
				}

				resolution = -vel * resolveFactor;

				currentContact->normal = returnNormal;
				currentContact->edge = e;
				currentContact->position = intersect;
				currentContact->resolution = resolution;
				currentContact->collisionPriority = length( vel + resolution );
				
				CircleShape *cs = new CircleShape;
				cs->setFillColor( Color::Red );
				cs->setRadius( 10 );
				cs->setOrigin( cs->getLocalBounds().width / 2, cs->getLocalBounds().height / 2 );
				cs->setPosition( currentContact->position.x, currentContact->position.y );
				progressDraw.push_back( cs );

				return currentContact;
			}
			else if( distIntersected < length( vel ) + .1 )
			{
				currentContact->resolution = intersect - corner;
				currentContact->collisionPriority = length( vel + resolution );//length( intersect - ( corner - vel ) );
				currentContact->edge = e;
				currentContact->position = intersect;
				currentContact->normal = edgeNormal;
			//	cout << "2nd res: " << currentContact->resolution.x << ", " << currentContact->resolution.y << endl;

				CircleShape *cs = new CircleShape;
				cs->setFillColor( Color::Cyan );
				cs->setRadius( 10 );
				cs->setOrigin( cs->getLocalBounds().width / 2, cs->getLocalBounds().height / 2 );
				cs->setPosition( currentContact->position.x, currentContact->position.y );
				progressDraw.push_back( cs );

				return currentContact;
			}
		}
		
	}

	

	/*double distIntersected = -cross( corner - e->v0, normalize( e->v1 - e->v0 ) ); //distance corner is inside solid
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
	}*/
	return NULL;
}

Contact * Collider::collideEdgeREAL( V2d position, const CollisionBox &b, Edge *e, const V2d &vel )
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
			cs->setRadius( 3 );
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
						{
							resolveLeft  = 10000;
				//			cout << "adjusting left" << endl;
						}

						//	cout << "temp resolveleft: " << resolveLeft << endl;
				}
				else if( right >= edgeLeft && vel.x > 0 )
				{
				//	if( vel.x == 0 )
				//		resolveRight = 0;
					resolveRight = (right - edgeLeft) / abs(vel.x );
					

					//cout << "temp resolveright: " << resolveRight << ", normal: " << e->Normal().x << ", " << e->Normal().y << endl;
					if( resolveRight > 1.1 )
					{
						
			//			cout << "adjusting right: " << resolveRight  << endl;
						resolveRight = 10000;
					}

					
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

				//real one for now
				resolveDist = min( resolveTop, min( resolveBottom, min( resolveLeft, resolveRight) ) );
				//cout << "resolve dist: " << resolveDist << ", " << resolveBottom << endl;
				if( approxEquals( resolveDist, 0 ) )
				{
				//	cout << "returning nNULLL heree" << endl;
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
				//	cout << "formally an error: " 
				//		<< currentContact->resolution.x << ", " << currentContact->resolution.y << endl;
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
					//pri = vel.x * resolveDist;//-vel.x - resolveDist * vel.x;
					pri = abs( vel.x ) - abs(vel.x * resolveDist);
				}
				else
				{
					pri = abs( vel.y ) - abs(vel.y * resolveDist);
					//pri = length( vel ) - length( vel * resolveDist );
					//pri = vel.y * resolveDist; //* vel.y;
				}


				

				if( approxEquals( pri, 0 ) )
					pri = 0;


				//cout << "pri 222: " << pri << endl;
				//currentContact->collisionPriority = 20 - abs(pri);//20 - abs(pri);// - pri;

				currentContact->collisionPriority = pri;//100;

				cout << "EDGE pri: " << currentContact->collisionPriority << " normal: " << edgeNormal.x << ", " << edgeNormal.y << 
					" res: " << currentContact->resolution.x << ", " << currentContact->resolution.y <<
					" vel: " << vel.x << ", " << vel.y << endl;

	
				//currentContact->collisionPriority = pri;


				currentContact->edge = e;
				return currentContact;
				//cout << "here!!!: " << currentContact->resolution.x << ", "
				//	<< currentContact->resolution.y << endl;
			}
			else
			{
			//	cout << "else case: " << intersectQuantity << ", " << length( e->v1 - e->v0 ) << endl;
			//cout << "else case" << endl;
				currentContact->resolution = e->GetPoint( intersectQuantity ) - corner;
			}
		

			if( length( currentContact->resolution ) > length( vel ) + 1 )
			{
			//	cout << "resolution: " << currentContact->resolution.x << ", " << currentContact->resolution.y << endl;
			//	cout << "vel: " << vel.x << ", " << vel.y << endl;
				
			//	cout << "returning null--" << endl;	
				//return NULL;
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
			//currentContact->collisionPriority = pri;
			currentContact->edge = e;


			cout << "SURFACE pri: " << currentContact->collisionPriority << " normal: " << edgeNormal.x << ", " << edgeNormal.y << 
				" res: " << currentContact->resolution.x << ", " << currentContact->resolution.y <<
				" vel: " << vel.x << ", " << vel.y << endl;

			return currentContact;

		}	
	}

	return NULL;
	}
}

Contact *Collider::collideEdge( V2d position, const CollisionBox &b, Edge *e, const V2d &vel )
{
	if( b.isCircle )
	{
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

		V2d en = e->Normal();
		V2d prevEn = e->edge0->Normal();
		V2d point = e->v0;
		//V2d v1 = e->v1;
		//check for point collisions first

		//bool pointInRect = point.x >= left && point.x <= right && point.y >= top && point.y <= bottom;		

		bool pointInRect = point.x >= min( left, oldLeft ) && point.x <= max( right, oldRight ) && point.y >= min( top, oldTop ) && point.y <= max( bottom, oldBottom );		

		double leftTime = 1, rightTime = 1, bottomTime = 1, topTime = 1; // one whole timestep
		
		V2d intersect;

		double pointMinTime = 100;
		int type = 0;
		V2d pointNormal(0,0);
		
		if( pointInRect )
		{

			bool rightCond0 = (prevEn.x < 0 && prevEn.y >= 0 && en.x < 0 && en.y <= 0);
			bool rightCond1 = ( prevEn.x >= 0 && prevEn.y > 0 && en.x <= 0 && en.y < 0 );
			bool rightCond2 = ( prevEn.x < 0 && prevEn.y > 0 && en.x >= 0 && en.y < 0 );
			//bool rightCond3 = ( prevEn.x == 0 && prevEn.y > 0 && en.x < 0 && en.y < 0 );
			
			bool leftCond0 = (prevEn.x > 0 && prevEn.y <= 0 && en.x > 0 && en.y >= 0);
			bool leftCond1 = ( prevEn.x <= 0 && prevEn.y < 0 && en.x > 0 && en.y > 0 );
			bool leftCond2 = ( prevEn.x >= 0 && prevEn.y < 0 && en.x <= 0 && en.y > 0 );
			//cout << "blah: " << (prevEn.x > 0) << ", " << (prevEn.y <= 0) << ", " << (en.x < 0 )<< ", " << (en.y > 0) << endl;

			bool topCond0 = (prevEn.y > 0 && prevEn.x >= 0 && en.y >= 0 && en.x < 0);
			bool topCond1 = ( prevEn.y <= 0 && prevEn.x > 0 && en.y > 0 && en.x < 0 );
			bool topCond2 = ( prevEn.y >= 0 && prevEn.x > 0 && en.y < 0 && en.x < 0 );

			bool bottomCond0 = (prevEn.y <= 0 && prevEn.x <= 0 && en.y < 0 && en.x >= 0);
			bool bottomCond1 = ( prevEn.y > 0 && prevEn.x < 0 && en.y <= 0 && en.x > 0 );
			bool bottomCond2 = ( prevEn.y < 0 && prevEn.x < 0 && en.y >= 0 && en.x > 0 );
			/*bool rightCond0 = (prevEn.x < 0 && prevEn.y > 0 && en.x < 0 && en.y < 0);
			bool rightCond1 = ( prevEn.x >= 0 && prevEn.y >= 0 && en.x < 0 && en.y < 0 );
			bool rightCond2 = ( prevEn.x < 0 && prevEn.y > 0 && en.x >= 0 && en.y < 0 );
			//bool rightCond2 = ( prevEn.x < 0 && prevEn.y > 0 && en.x >= 0 && en.y < 0 );
			bool rightCond3 = (prevEn.x == -1 && prevEn.y == 0 && en.x < 0 && en.y > 0 );

			bool leftCond0 = (prevEn.x > 0 && prevEn.y < 0 && en.x > 0 && en.y > 0);
			bool leftCond1 = ( prevEn.x <= 0 && prevEn.y <= 0 && en.x > 0 && en.y > 0 );
			bool leftCond2 = ( prevEn.x > 0 && prevEn.y < 0 && en.x <= 0 && en.y >= 0 );
			bool leftCond3 = ( prevEn.x == 1 && prevEn.y == 0 && en.x > 0 && en.y > 0 );
			//bool leftCond3 = (prevEn.x > 0 && prevEn.y == 0 && en.x > 0 && en.y > 0 );

			bool topCond0 = (prevEn.y > 0 && prevEn.x > 0 && en.y > 0 && en.x < 0);
			bool topCond1 = ( prevEn.y < 0 && prevEn.x > 0 && en.y > 0 && en.x < 0 );
			bool topCond2 = ( prevEn.y > 0 && prevEn.x > 0 && en.y < 0 && en.x > 0 );

			bool bottomCond0 = (prevEn.y < 0 && prevEn.x < 0 && en.y < 0 && en.x > 0);
			bool bottomCond1 = ( prevEn.y > 0 && prevEn.x < 0 && en.y < 0 && en.x > 0 );
			bool bottomCond2 = ( prevEn.y < 0 && prevEn.x < 0 && en.y > 0 && en.x > 0 );*/

			/*rightCond0 = true;
			rightCond1 = true;
			rightCond2 = true;

			leftCond0 = true;
			leftCond1 = true;
			leftCond2 = true;

			topCond0 = true;
			topCond1 = true;
			topCond2 = true;

			bottomCond0 = true;
			bottomCond1 = true;
			bottomCond2 = true; */
			cout << "oldLeft: " << oldLeft << ", px: " << point.x << ", left: " << left << endl;
			cout << "leftconds: " << leftCond0 << ", " << leftCond1 << ", " << leftCond2 << endl;
			//cout << "vel: " << vel.x << ", " << vel.y << ", bottomconds: " << bottomCond0 <<" , " << bottomCond1 <<", " << bottomCond2 << endl;
			//cout << "prev: " << prevEn.x << ", " << prevEn.y << " n: " << en.x << ", " << en.y << endl;
			//cout << "rightcond3: " << prevEn.x << ", " << prevEn.y << ", en: " << en.x << ", " << en.y << ", cond: " << rightCond3  << endl;
			if( (rightCond0 || rightCond1 || rightCond2 ) && vel.x > 0 && oldRight <= point.x + .001 && right >= point.x  )
			{
			//	cout << "right " << endl;
				pointMinTime = ( point.x - oldRight ) / abs(vel.x);
				pointNormal.x = -1;
			}
			else if( ( leftCond0 || leftCond1 || leftCond2 ) && vel.x < 0 && oldLeft >= point.x - .001 && left <= point.x  )
			{
				cout << "left" << endl;
				pointMinTime = ( oldLeft - point.x ) / abs( vel.x );
				pointNormal.x = 1;
			}
			
			if( (bottomCond0 || bottomCond1 || bottomCond2 ) && vel.y > 0 && oldBottom <= point.y + .001 && bottom >= point.y )
			{
				bool okay = false;
				if( vel.x > 0 )
				{
					if( oldLeft < edgeRight )
					{
						okay = true;
					}
				}
				else if( vel.x < 0 )
				{
					if( oldRight > edgeLeft )
					{
						okay = true;
					}
				}
				else
				{
					okay = true;
				}

				cout << "bottom cond okay: " << okay << endl;

				bottomTime = ( point.y - oldBottom ) / abs( vel.y );
				if( okay && bottomTime < pointMinTime )
				{
					//cout << "bottomtime: " << bottomTime << endl;
					pointMinTime = bottomTime;
					pointNormal.x = 0;
					pointNormal.y = -1;
				}
				//pointMinTime = min( bottomTime, pointMinTime );
			}
			else if( (topCond0 || topCond1 || topCond2 ) && vel.y < 0 && oldTop >= point.y - .001 && top <= point.y )
			{
		//		cout << "top" << endl;
				topTime = ( oldTop - point.y ) / abs( vel.y );
				if( topTime < pointMinTime )
				{
					pointMinTime = topTime;
					pointNormal.x = 0;
					pointNormal.y = 1;
				}
			}

		}

		double time = 100;
		if( en.x == 0 )
		{
			double edgeYPos = edgeTop;
			if( en.y > 0 ) //down
			{
				if( vel.y < 0 && oldTop >= edgeYPos - .001 && top <= edgeYPos )
				{
					bool hit = true;

					bool a = left >= edgeLeft && left <= edgeRight;
					bool b = right >= edgeLeft && right <= edgeRight;
					//cout << "edge l/r: " << edgeLeft << ", " << edgeRight << ", l/r: " << left << ", " << right << endl;
					

					if( a && b )
					{
						intersect.x = (right + left ) / 2.0;
					}
					else if(a  )
					{
						intersect.x = left;
					}
					else if( b )
					{
						intersect.x = right;
					}
					else if( left <= edgeLeft && right >= edgeRight )
					{
						//cout << "blahhhhh:" << endl;
						intersect.x = (edgeLeft + edgeRight ) / 2.0;
					}
					else
					{
						hit = false;
						
					} 

					if( hit )
					{
						time = ( oldTop - edgeYPos ) / abs( vel.y );

						intersect.y = edgeYPos;
					}
					
				}
			}
			else //up
			{
				if( vel.y > 0 && oldBottom <= edgeYPos + .001 && bottom >= edgeYPos )
				{
					//cout << "this one: " << oldBottom << ", bottom: " << bottom << ", eyp: " << edgeYPos << endl;
					
					bool a = left >= edgeLeft && left <= edgeRight;
					bool b = right >= edgeLeft && right <= edgeRight;
					//cout << "edge l/r: " << edgeLeft << ", " << edgeRight << ", l/r: " << left << ", " << right << endl;
					bool hit = true;

					if( a && b )
					{
						intersect.x = (right + left ) / 2.0;
					}
					else if(a  )
					{
						intersect.x = left;
					}
					else if( b )
					{
						intersect.x = right;
					}
					else if( left <= edgeLeft && right >= edgeRight )
					{
						//cout << "blahhhhh:" << endl;
						intersect.x = (edgeLeft + edgeRight ) / 2.0;
					}
					else
					{
						hit = false;
						
					} 
					if( hit )
					{
						
						time = ( edgeYPos - oldBottom ) / abs( vel.y );

						intersect.y = edgeYPos;
					}
				}
			}

		}
		else if( en.y == 0 )
		{
			double edgeXPos = edgeLeft;
			if( en.x > 0 ) //right
			{
				cout << "trying!: oldLeft: " << oldLeft << ", edgeXPos: " << edgeXPos <<", left: " << left << ", vel: " << vel.x << ", " << vel.y << endl;
				cout << "blah: " << (vel.x < 0 ) << ", " << (oldLeft >= edgeXPos ) << ", " << (left <= edgeXPos ) << endl;
				if( vel.x < 0 && oldLeft >= edgeXPos - .001 && left <= edgeXPos )
				{
					bool a = top >= edgeTop && top <= edgeBottom;
					bool b = bottom >= edgeTop && bottom <= edgeBottom;
					//cout << "edge l/r: " << edgetop << ", " << edgebottom << ", l/r: " << top << ", " << bottom << endl;
					cout << "in here: " << a << ", " << b << endl;
					bool hit = true;

					if( a && b )
					{
						intersect.y = (bottom + top ) / 2.0;
					}
					else if(a  )
					{
						intersect.y = top;
					}
					else if( b )
					{
						intersect.y = bottom;
					}
					else if( top <= edgeTop && bottom >= edgeBottom )
					{
						//cout << "blahhhhh:" << endl;
						intersect.y = (edgeTop + edgeBottom) / 2.0;
					}
					else
					{
						cout << "miss: 1 0: " << edgeTop << ", " << edgeBottom << ", l/r: " << top << ", " << bottom << endl;
						hit = false;
					} 

					if( hit )
					{
						time = ( oldLeft - edgeXPos ) / abs( vel.x);
						intersect.x = edgeXPos;
					}

				}
			}
			else //left
			{
				//cout << "attempting right: " << oldRight << ", " << edgeXPos << ", " << right << endl;
				if( vel.x > 0 && oldRight <= edgeXPos + .001 && right >= edgeXPos )
				{
					bool a = top >= edgeTop && top <= edgeBottom;
					bool b = bottom >= edgeTop && bottom <= edgeBottom;
					//cout << "edge l/r: " << edgetop << ", " << edgebottom << ", l/r: " << top << ", " << bottom << endl;
					bool hit = true;

					if( a && b )
					{
						intersect.y = (bottom + top ) / 2.0;
					}
					else if(a  )
					{
						intersect.y = top;
					}
					else if( b )
					{
						intersect.y = bottom;
					}
					else if( top <= edgeTop && bottom >= edgeBottom )
					{
						//cout << "blahhhhh:" << endl;
						intersect.y = (edgeTop + edgeBottom) / 2.0;
					}
					else
					{
						
						hit = false;
					} 

					if( hit )
					{
						time = ( edgeXPos - oldRight ) / abs( vel.x );
						intersect.x = edgeXPos;
					}
				}
				
				

			}

		
			
			
			//return NULL;
				
		}
		else
		{
			Vector2<double> corner(0,0);
			V2d opp;
			if( en.x > 0 )
			{
				corner.x = left;
				opp.x = right;
			}
			else if( en.x < 0 )
			{
				corner.x = right;
				opp.x = left;
			}
			
			if( en.y > 0 )
			{
				corner.y = top;
				opp.y = bottom;
			}
			else if( en.y < 0 )
			{
				corner.y = bottom;
				opp.y = top;
			}

			double res = cross( corner - e->v0, e->v1 - e->v0 );
			double oldRes = cross( (corner - vel ) - e->v0, e->v1 - e->v0 );
			double resOpp = cross( opp - e->v0, e->v1 - e->v0 );
			//might remove the opp thing soon

			double measureNormal = dot( en, normalize(-vel) );
			//cout << "oldRes : " << oldRes << endl;
			bool test = res < -.001 && resOpp > 0 && measureNormal > 0 && ( vel.x != 0 || vel.y != 0 ) ;
			
			if( res < -.001 && oldRes >= -.001 && resOpp > 0 && measureNormal > -.001 && ( vel.x != 0 || vel.y != 0 )  )	
			//if( res < .001 && /*oldRes >= -.001 &&*/ resOpp > 0 && measureNormal > -.001 && ( vel.x != 0 || vel.y != 0 )  )	
			{

				LineIntersection li = lineIntersection( corner, corner - (vel), e->v0, e->v1 );
				double testing = dot( normalize( (corner-vel) - corner), normalize( e->v1 - e->v0 ));
				if( li.parallel || abs( testing ) == 1 )
				{
					//cout << "returning null1" << endl;
					return NULL;
				}
				intersect = li.position;

				double intersectQuantity = e->GetQuantity( intersect );

				//cout << "test: " << test << " normal: " << en.x << ", " << en.y << " q: " << intersectQuantity << "len: " << length( e->v1 - e->v0 ) << endl;
				//if( intersectQuantity < 0 )
				//	intersectQuantity = 0;
				//if( intersectQuantity >length( e->v1 - e->v0 ) )
				//	intersectQuantity = length( e->v1 - e->v0 );
				double len = length( e->v1 - e->v0 );
				if( intersectQuantity < 0 || intersectQuantity > len )
				{
					
					cout << "bad: " << en.x << ", " << en.y << "  " << intersectQuantity << ", len: " << length( e->v1 - e->v0 ) << endl;
					if( intersectQuantity <= 0 )
					{
					//	point = e->v0;
					//	cout << "adjusting" << endl;
					}
					else
					{
					//	point = e->v1;
					}

		
					//bool rightCond0 = (prevEn.x < 0 && prevEn.y > 0 && en.x < 0 && en.y < 0);
					//bool rightCond1 = ( prevEn.x > 0 && prevEn.y > 0 && en.x < 0 && en.y < 0 );
					//bool rightCond2 = ( prevEn.x < 0 && prevEn.y > 0 && en.x > 0 && en.y < 0 );

					//bool leftCond0 = (prevEn.x >= 0 && prevEn.y <= 0 && en.x > 0 && en.y > 0);
					//bool leftCond1 = ( prevEn.x < 0 && prevEn.y < 0 && en.x > 0 && en.y > 0 );
					//bool leftCond2 = ( prevEn.x > 0 && prevEn.y < 0 && en.x < 0 && en.y > 0 );

					//bool topCond0 = (prevEn.y > 0 && prevEn.x > 0 && en.y > 0 && en.x < 0);
					//bool topCond1 = ( prevEn.y < 0 && prevEn.x > 0 && en.y > 0 && en.x < 0 );
					//bool topCond2 = ( prevEn.y > 0 && prevEn.x > 0 && en.y < 0 && en.x > 0 );

					//bool bottomCond0 = (prevEn.y < 0 && prevEn.x < 0 && en.y < 0 && en.x > 0);
					//bool bottomCond1 = ( prevEn.y > 0 && prevEn.x < 0 && en.y < 0 && en.x > 0 );
					//bool bottomCond2 = ( prevEn.y < 0 && prevEn.x < 0 && en.y > 0 && en.x > 0 );
					//cout << "oldLeft: " << oldLeft << ", left: " << left << "point: " << point.x << ", " << point.y << endl;
					//if( (rightCond0 || rightCond1 || rightCond2 ) && vel.x > 0 && oldRight <= point.x && right >= point.x  )
					//{
					//	rightTime = ( point.x - oldRight ) / abs(vel.x);
					//	if( rightTime < pointMinTime )
					//	{
					//		pointMinTime = rightTime;
					//		pointNormal.x = -1;
					//		pointNormal.y = 0;
					//	}
					//}
					//else if( ( leftCond0 || leftCond1 || leftCond2 ) && vel.x < 0 && oldLeft >= point.x && left <= point.x  )
					//{
					//	
					//	leftTime = ( oldLeft - point.x ) / abs( vel.x );
					//	cout << "left try" << leftTime << "pointMinTime: " << pointMinTime << endl;
					//	if( leftTime < pointMinTime )
					//	{
					//		
					//		pointMinTime = leftTime;
					//		pointNormal.x = 1;
					//		pointNormal.y = 0;
					//	}
					//}
			
					//if( (bottomCond0 || bottomCond1 || bottomCond2 ) && vel.y > 0 && oldBottom <= point.y && bottom >= point.y )
					//{
					////	cout << "bottom" << endl;
					//	bottomTime = ( point.y - oldBottom ) / abs( vel.y );
					//	if( bottomTime < pointMinTime )
					//	{
					//		pointMinTime = bottomTime;
					//		pointNormal.x = 0;
					//		pointNormal.y = -1;
					//	}
					//	//pointMinTime = min( bottomTime, pointMinTime );
					//}
					//else if( (topCond0 || topCond1 || topCond2 ) && vel.y < 0 && oldTop >= point.y && top <= point.y )
					//{
					//	topTime = ( oldTop - point.y ) / abs( vel.y );
					//	if( topTime < pointMinTime )
					//	{
					//		pointMinTime = topTime;
					//		pointNormal.x = 0;
					//		pointNormal.y = 1;
					//	}
					//}
				}
				else
				{
					bool okay = true;
					bool a = intersectQuantity == 0 ;
					bool b = intersectQuantity == len;
					if( a || b  )
					{
						okay = false;
						double t;
						if( a )
						{
							t = cross( e->v1 - e->v0, e->edge0->v0 - e->v0 );
						}
						else
						{
							t = -cross( e->edge1->v1 - e->v1, e->v1 - e->v0 ); 
						}
						
						if( t < 0 )
						{
							okay = true;
						}
						cout << "t: " << t << endl;
					}

					if( okay )
					{
				//	cout << "using: " << intersectQuantity << ", length: " << length( e->v1 - e->v0 ) << endl;
					//this is prob wrong
					double tempTime = dot( intersect - ( corner - vel ), normalize( vel ) );
					tempTime /= length( vel );
					//cout << "tempTime: " << tempTime << endl;
					//if( tempTime >= -4 )
					{

						//if( tempTime < 0 )
						//	tempTime = 0;
						time = tempTime;
					}
						
					//		cout << "time: " << time << " normal: " << en.x << ", " << en.y << 
			//" vel: " << vel.x << ", " << vel.y << ", q: " << intersectQuantity << ", len: " << length( e->v1 - e->v0 ) << endl;
					}
						
					
						
					
				}
			}
			else
			{
				cout << "res: " << res << ", " << oldRes << ", " << resOpp << ", " << measureNormal << endl;
			
				cout << "baz: " << (res < 0 ) <<", " << ( oldRes >= -.001 ) << ", " << (resOpp > 0 ) << ", " << (measureNormal > -.001 ) << endl;
			}


		}
		//aabb's already collide

		if( pointMinTime <= time )
		{
			time = pointMinTime;
			currentContact->position = point;
			currentContact->normal = pointNormal;

			if( time == 100 )
			return NULL;

			CircleShape *cs = new CircleShape;
			cs->setFillColor( Color::Yellow );
			cs->setRadius( 5 );
			cs->setOrigin( cs->getLocalBounds().width / 2, cs->getLocalBounds().height / 2 );
			cs->setPosition( point.x, point.y );

			progressDraw.push_back( cs );	

			//cout << "point " << endl;
		}
		else
		{
			currentContact->position = intersect;
			currentContact->normal = V2d( 0, 0 );

			CircleShape *cs = new CircleShape;
			cs->setFillColor( Color::Yellow );
			cs->setRadius( 5 );
			cs->setOrigin( cs->getLocalBounds().width / 2, cs->getLocalBounds().height / 2 );
			cs->setPosition( intersect.x, intersect.y );

			progressDraw.push_back( cs );
		}

		if( time == 100 )
			return NULL;
		
		if( time < 0 )
			time = 0;
		if( approxEquals( time, 0 ) )
			time = 0;
		currentContact->collisionPriority = time;
		currentContact->edge = e;
		currentContact->movingPlat = NULL;
		
		currentContact->resolution = -vel * ( 1 - time );

	//	cout << "pri: " << currentContact->collisionPriority << " normal: " << en.x << ", " << en.y << 
	//		" res: " << currentContact->resolution.x << ", " << currentContact->resolution.y <<
	//		" vel: " << vel.x << ", " << vel.y << ", pos: " << currentContact->position.x << ", " << currentContact->position.y
	//		<< "old: " << oldPosition.x << ", " << oldPosition.y << endl;

//		cout << "pri: " << currentContact->collisionPriority << " normal: " << en.x << ", " << en.y << endl;
		return currentContact;

		
		
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

bool IsCircleTouchingCircle( V2d pos0, double rad_0, V2d pos1, double rad_1 )
{
	return length( pos1 - pos0 ) <= rad_0 + rad_1;
}

bool IsEdgeTouchingCircle( V2d v0, V2d v1, V2d pos, double rad )
{
	double dist = cross( pos - v0, normalize( v1 - v0 ) );
	double q = dot( pos - v0, normalize( v1 - v0 ) );
	double edgeLength = length( v1 - v0 );


	if( q < 0 )
	{
		if( length( v0 - pos ) < rad )
		{
			return true;
		}
	}
	else if( q > edgeLength )
	{
		if( length( v1 - pos ) < rad )
		{
			return true;
		}
	}
	else
	{
		if( dist < rad )
		{
			return true;
		}
	}

	return false;
}

bool IsQuadTouchingCircle( V2d A, V2d B, V2d C, V2d D, V2d pos, double rad )
{
	if( IsEdgeTouchingCircle( A,B, pos, rad ) 
		|| IsEdgeTouchingCircle( B,C, pos, rad ) 
		|| IsEdgeTouchingCircle( C,D, pos, rad ) 
		|| IsEdgeTouchingCircle( D,A, pos, rad ) )
	{
		return true;
	}
	return false;
}
//top left is A then clockwise
bool isQuadTouchingQuad( V2d &A0, V2d &B0, V2d &C0, V2d &D0, V2d &A1, V2d &B1, V2d &C1, V2d &D1 )
{
	double AB = length( B0 - A0 );
	double AD = length( D0 - A0 );

	V2d normalizeAB = normalize( B0 - A0 );
	V2d normalizeAD = normalize( D0 - A0 );
	

	double min1AB = min( dot( A1 - A0, normalizeAB ), min( dot( B1 - A0, normalizeAB ), min( dot( C1 - A0, normalizeAB ),
		dot( D1 - A0, normalizeAB ) ) ) );
	double max1AB = max( dot( A1 - A0, normalizeAB ), max( dot( B1 - A0, normalizeAB ), max( dot( C1 - A0, normalizeAB ),
		dot( D1 - A0, normalizeAB ) ) ) );

	double min1AD = min( dot( A1 - A0, normalizeAD ), min( dot( B1 - A0, normalizeAD ), min( dot( C1 - A0, normalizeAD ),
		dot( D1 - A0, normalizeAD ) ) ) );
	double max1AD = max( dot( A1 - A0, normalizeAD ), max( dot( B1 - A0, normalizeAD ), max( dot( C1 - A0, normalizeAD ),
		dot( D1 - A0, normalizeAD ) ) ) );

	
	double AB1 = length( B1 - A1 );
	double AD1 = length( D1 - A1 );

	V2d normalizeAB1 = normalize( B1 - A1 );
	V2d normalizeAD1 = normalize( D1 - A1 );

	double min0AB = min( dot( A0 - A1, normalizeAB1 ), min( dot( B0 - A1, normalizeAB1 ), min( dot( C0 - A1, normalizeAB1 ),
		dot( D0 - A1, normalizeAB1 ) ) ) );
	double max0AB = max( dot( A0 - A1, normalizeAB1 ), max( dot( B0 - A1, normalizeAB1 ), max( dot( C0 - A1, normalizeAB1 ),
		dot( D0 - A1, normalizeAB1 ) ) ) );

	double min0AD = min( dot( A0 - A1, normalizeAD1 ), min( dot( B0 - A1, normalizeAD1 ), min( dot( C0 - A1, normalizeAD1 ),
		dot( D0 - A1, normalizeAD1 ) ) ) );
	double max0AD = max( dot( A0 - A1, normalizeAD1 ), max( dot( B0 - A1, normalizeAD1 ), max( dot( C0 - A1, normalizeAD1 ),
		dot( D0 - A1, normalizeAD1 ) ) ) );

	if( min1AB <= AB && max1AB >= 0 && min1AD <= AD && max1AD >= 0 
		&& min0AB <= AB1 && max0AB >= 0 && min0AB <= AB1 && max0AD >= 0 )
	{
		return true;
	}
	else
	{
		return false;
	}
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

		Edge e;
		e.v0 = startPoint;
		e.v1 = endPoint;

		sf::Rect<double> nodeBox( node->pos.x - node->rw, node->pos.y - node->rh, node->rw * 2, node->rh * 2 );

		if( IsEdgeTouchingBox( &e, nodeBox ) )
		{
			for( list<QuadTreeEntrant*>::iterator it = n->extraChildren.begin(); it != n->extraChildren.end(); ++it )
			{
				LineIntersection li = SegmentIntersect( startPoint, endPoint, ((Edge*)(*it))->v0, ((Edge*)(*it))->v1 );	
				if( !li.parallel )
				{
					handler->HandleRayCollision( ((Edge*)(*it)), ((Edge*)(*it))->GetQuantity( li.position ), 
						dot( V2d( li.position - startPoint ), normalize( endPoint - startPoint ) ) );
				}
			}

			for( int i = 0; i < 4; ++i )
			{
				RayCast( handler, n->children[i], startPoint, endPoint );
			}
		}
	}
}

void Edge::HandleQuery( QuadTreeCollider * qtc )
{
	qtc->HandleEntrant( this );
}

bool Edge::IsTouchingBox( const sf::Rect<double> &r )
{
	return IsEdgeTouchingBox( this, r );
}