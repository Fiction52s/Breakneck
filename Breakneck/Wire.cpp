#include "Wire.h"
#include "Actor.h"
#include "GameSession.h"
#include <iostream>

using namespace sf;
using namespace std;

#define V2d sf::Vector2<double>

Wire::Wire( Actor *p, bool r)
	:state( IDLE ), numPoints( 0 ), framesFiring( 0 ), fireRate( 120 ), maxTotalLength( 10000 ), minSegmentLength( 50 )
	, player( p ), triggerThresh( 200 ), hitStallFrames( 20 ), hitStallCounter( 0 ), pullStrength( 10 ), right( r )
{
}

void Wire::UpdateState( bool touchEdgeWithWire )
{
	ControllerState &currInput = player->currInput;
	ControllerState &prevInput = player->prevInput;

	bool triggerDown;
	bool prevTriggerDown;

	if( right )
	{
		triggerDown = currInput.rightTrigger >= triggerThresh;
		prevTriggerDown = prevInput.rightTrigger >= triggerThresh;
	}
	else
	{
		triggerDown = currInput.leftTrigger >= triggerThresh;
		prevTriggerDown = prevInput.leftTrigger >= triggerThresh;
	}


	switch( state )
	{
	case IDLE:
		{
			
			if( triggerDown && !prevTriggerDown )
			{
				//cout << "firing" << endl;
				fireDir = V2d( 0, 0 );


				if( false )
				{
					if( currInput.LLeft() )
					{
						fireDir.x -= 1;
					}
					else if( currInput.LRight() )
					{
						fireDir.x += 1;
					}
			
					if( currInput.LUp() )
					{
						fireDir.y -= 1;
					}
					else if( currInput.LDown() )
					{
						fireDir.y += 1;
					}
				}
				else
				{
					fireDir.x = cos( currInput.leftStickRadians );
					fireDir.y = -sin( currInput.leftStickRadians );
				}

				if( length( fireDir ) > .1 )
				{
					fireDir = normalize( fireDir );
					state = FIRING;
					framesFiring = 0;
				}
			}
			break;
		}
	case FIRING:
		{
			//rcEdge = NULL;
		//	RayCast( this, player->owner->terrainTree->startNode, anchor.pos, player->position );
			if( rcEdge != NULL )
			{
				state = HIT;
				hitStallCounter = framesFiring;
			}

			if( framesFiring * fireRate > 10000 )
			{
				state = IDLE;
				framesFiring = 0;
			}
			break;
		}
	case HIT:
		{
			double total = 0;
			
			if( numPoints > 0 )
			{
				total += length( points[0].pos - anchor.pos );
				for( int i = 1; i < numPoints; ++i )
				{
					total += length( points[i].pos - points[i-1].pos );
				}
			}
			else
			{
				total += length( anchor.pos - player->position );
			}
			totalLength = total;

			if( totalLength > maxTotalLength )
			{
				state = IDLE;
			}

			if( player->ground == NULL && hitStallCounter >= hitStallFrames && triggerDown )
			{
				state = PULLING;
			}
			break;
		}
	case PULLING:
		{
			if( !triggerDown )
			{
				state = RELEASED;
			}
			if( touchEdgeWithWire )
			{
				state = RELEASED;
			}
			
			break;
		}
	case RELEASED:
		{
			state = IDLE;
			break;
		}
	}

	switch( state )
	{
	case IDLE:
		{
			//no updates
			break;
		}
	case FIRING:
		{
			rcEdge = NULL;
			//rcQuantity = 0;
			
			RayCast( this, player->owner->terrainTree->startNode, player->position, player->position + fireDir * fireRate * (double)(framesFiring + 1 ) );
			
			++framesFiring;

			if( rcEdge != NULL )
			{
				anchor.pos = rcEdge->GetPoint( rcQuant );
				anchor.e = rcEdge;
				anchor.quantity = rcQuant;
				numPoints = 0;
			}
			break;
		}
	case HIT:
		{


			if( hitStallCounter < hitStallFrames )
				hitStallCounter++;
			break;
		}
	case PULLING:
		{
			double total = 0;
			
			if( numPoints > 0 )
			{
				total += length( points[0].pos - anchor.pos );
				for( int i = 1; i < numPoints; ++i )
				{
					total += length( points[i].pos - points[i-1].pos );
				}
				total += length( points[numPoints-1].pos - player->position );
				cout << "multi: " << "numPoints: " << numPoints << " , " << total << endl;
			}
			else
			{
				total += length( anchor.pos - player->position );
				cout << "single: " << total << endl;
			}
			//totalLength = total;
			//if( total < totalLength )
				totalLength = total;

			V2d wn;

			if( numPoints == 0 )
			{
				//if( totalLength < segmentLength )
					segmentLength = totalLength;
				wn = normalize( anchor.pos - player->position );
				cout << "segment length single: " << segmentLength << endl;
			}
			else
			{
				double temp = length( points[numPoints-1].pos - player->position );
				//if( temp < segmentLength )
				{
					segmentLength = temp;
				}
				wn = normalize( points[numPoints-1].pos - player->position );
				cout << "segment length multi: " << segmentLength << endl;
			}
			
			bool shrinkInput = false;

			
			if( wn.y <= 0 )
			{
				if( wn.x < 0 )
				{
					shrinkInput = currInput.LLeft();
				}
				else if( wn.x > 0 )
				{
					shrinkInput = currInput.LRight();
				}

				shrinkInput |= currInput.LUp();
			}
			else if( wn.y > 0 )
			{
				if( wn.x < 0 )
				{
					shrinkInput = currInput.LLeft();
				}
				else if( wn.x > 0 )
				{
					shrinkInput = currInput.LRight();
				}

				shrinkInput |= currInput.LDown();
			}

			shrinkInput = false;

			if( currInput.B )
			{
				shrinkInput = true;
			}
			else if( currInput.Y )
			{
				if( triggerDown && player->ground == NULL )
				{
						segmentLength += pullStrength;
						totalLength += pullStrength;
				}
			}

			if( shrinkInput && triggerDown && player->ground == NULL )
			//if( false )
			{
				//totalLength -= pullStrength;

				
				double segmentChange = pullStrength;
				if( segmentLength - pullStrength < minSegmentLength )
					segmentChange = minSegmentLength - (segmentLength - pullStrength);

				totalLength -= segmentChange;
				segmentLength -= segmentChange;
			}
			break;
		}	
	case RELEASED:
		{
			break;
		}
	}
}

void Wire::UpdateAnchors2()
{
	if( state == HIT || state == PULLING )
	{
	//	rayCastMode = "check";
		rcEdge = NULL;


		sf::VertexArray *line = new VertexArray( sf::Lines, 0 );
		line->append( sf::Vertex(sf::Vector2f(player->position.x, player->position.y), Color::Magenta ) );
		
		

		//target->draw(line, 2, sf::Lines);

		if( numPoints == 0 )
		{
			line->append( sf::Vertex(sf::Vector2f(anchor.pos.x, anchor.pos.y), Color::Black) );
			RayCast( this, player->owner->terrainTree->startNode, anchor.pos, player->position );
		}
		else
		{
			line->append( sf::Vertex(sf::Vector2f(points[numPoints - 1].pos.x, points[numPoints - 1].pos.y), Color::Black) );
			RayCast( this, player->owner->terrainTree->startNode, points[numPoints - 1].pos, player->position );
		}

		progressDraw.push_back( line );

		if( rcEdge != NULL )
		{
			//if( numPoints > 0 )
			//cout << "accumulating: " << wirePoints[pointNum-1].pos.x << ", " << wirePoints[pointNum -1].pos.y << endl;
			if( rcQuant > length( rcEdge->v1 - rcEdge->v0 ) - rcQuant )
			{
				points[numPoints].pos = rcEdge->v1;
				//wirePoints[pointNum].e = rcEdge;
				points[numPoints].test = normalize(rcEdge->edge1->v1 - rcEdge->edge1->v0 );
				//cout << "over" << endl;

				numPoints++;
			}
			else
			{
				//cout << "under" << endl;
				points[numPoints].pos = rcEdge->v0;
				points[numPoints].test = normalize( rcEdge->edge0->v1 - rcEdge->edge0->v0 );
				numPoints++;
			}
		}

		for( int i = numPoints - 1; i >= 0; --i )
		{
			double result = cross( player->position - points[numPoints-1].pos, points[i].test );
			if( result > 0 )
			{
				//cout << "removing point " << result << endl;
				numPoints--;
			}
			else
			{
				break;
			}
		}
	}
}

void Wire::SortNewPoints( int start, int end )
{
	/*int first;
	for( int i = end; i >= start; --i )
	{
		first = start;
		for( int j = 1; j <= i; ++j )
		{
			if( points[j].angleDiff < points[first].angleDiff )
			{
				first = 
			}
		}
	}*/
}

void Wire::SwapPoints( int aIndex, int bIndex )
{
	WirePoint temp = points[aIndex];
	points[aIndex] = points[bIndex];
	points[bIndex] = temp;
}

void Wire::UpdateAnchors( V2d vel )
{
	if( state == HIT || state == PULLING )
	{
		//cout << "updating anchors" << endl;
	//	rayCastMode = "check";
		//rcEdge = NULL;


		//sf::VertexArray *line = new VertexArray( sf::Lines, 0 );
		//line->append( sf::Vertex(sf::Vector2f(player->position.x, player->position.y), Color::Magenta ) );
		
		oldPos = player->position - vel;
		double radius = length( realAnchor - player->position ); //new position after moving

		if( numPoints == 0 )
		{
			//line->append( sf::Vertex(sf::Vector2f(anchor.pos.x, anchor.pos.y), Color::Black) );
			realAnchor = anchor.pos;
		}
		else
		{
			//line->append( sf::Vertex(sf::Vector2f(points[numPoints - 1].pos.x, points[numPoints - 1].pos.y), Color::Black) );
			realAnchor = points[numPoints-1].pos;
		}

		int counter = 0;
		while( true )
		{
			//progressDraw.push_back( line );
			if( counter > 1 )
			{
				cout << "COUNTER: " << counter << endl;
			}

			double left = min( realAnchor.x, min( oldPos.x, player->position.x ) );
			double right = max( realAnchor.x, max( oldPos.x, player->position.x ) );
			double top = min( realAnchor.y, min( oldPos.y, player->position.y ) );
			double bottom = max( realAnchor.y, max( oldPos.y, player->position.y ) );

			Rect<double> r( left, top, right - left, bottom - top );

			//addedPoints = 0;
			foundPoint = false;

			player->owner->terrainTree->Query( this, r );
		

			/*for( int i = 1; i < addedPoints; ++i )
			{
				for( int j = i; j > 0; ++j )
				{

				}
			
			}*/

			if( foundPoint )
			{
				points[numPoints].pos = closestPoint;


				points[numPoints].test = normalize( closestPoint - realAnchor );
				if( !clockwise )
				{
					points[numPoints].test = -points[numPoints].test;
				}
				//points[numPoints].test = normalize(  
				numPoints++;
				//cout << "closestPoint: " << closestPoint.x << ", " << closestPoint.y << endl;
				//cout << "numpoints now! " << numPoints << endl;

				V2d oldAnchor = realAnchor;
				realAnchor = points[numPoints-1].pos;

				radius = radius - length( oldAnchor - realAnchor );
				oldPos = realAnchor + normalize( realAnchor - oldAnchor ) * radius;

				cout << "point added!: " << points[numPoints-1].pos.x << ", " << points[numPoints-1].pos.y << ", numpoints: " << numPoints << endl;
				counter++;
			}
			else
			{
				break;
			}
			//oldPos = 
			
		}
		
		//if( rcEdge != NULL )
		if( false )
		{
			if( rcQuant > length( rcEdge->v1 - rcEdge->v0 ) - rcQuant )
			{
				points[numPoints].pos = rcEdge->v1;
				//wirePoints[pointNum].e = rcEdge;
				points[numPoints].test = normalize(rcEdge->edge1->v1 - rcEdge->edge1->v0 );
				//cout << "over" << endl;

				numPoints++;
			}
			else
			{
				//cout << "under" << endl;
				points[numPoints].pos = rcEdge->v0;
				points[numPoints].test = normalize( rcEdge->edge0->v1 - rcEdge->edge0->v0 );
				numPoints++;
			}
		}

		for( int i = numPoints - 1; i >= 0; --i )
		{ 
			double result = cross( player->position - points[numPoints-1].pos, points[i].test );
			if( result > 0 )
			{
				//cout << "removing point " << result << endl;
				numPoints--;
			}
			else
			{
				break;
			}
		}
	}

	//UpdateState( false );
}

void Wire::HandleRayCollision( Edge *edge, double edgeQuantity, double rayPortion )
{
	if( rayPortion > 1 && ( rcEdge == NULL || length( edge->GetPoint( edgeQuantity ) - player->position ) < length( rcEdge->GetPoint( rcQuant ) - player->position ) ) )
	{
		rcEdge = edge;
		rcQuant = edgeQuantity;
	}
}

void Wire::TestPoint( Edge *e )
{

	V2d p = e->v0;

	if( p == realAnchor )
	{
		return;
	}

	double radius = length( realAnchor - player->position ); //new position after moving

	double anchorDist = length( realAnchor - p );
	if( anchorDist > radius )
	{
		return;
	}
	
	//cout << "anchordist: " << anchorDist << ", radius: " << radius << endl;

	V2d oldVec = normalize( oldPos - realAnchor );
	V2d newVec = normalize( player->position - realAnchor );
	V2d pVec = normalize( p - realAnchor );

	double oldAngle = atan2( oldVec.y, oldVec.x );
	
	

	double newAngle = atan2( newVec.y, newVec.x );
	

	double pAngle = atan2( pVec.y, pVec.x );
	
	double angleDiff = abs( oldAngle - pAngle );

	double maxAngleDiff = abs( newAngle - oldAngle );

	

	//if( angleDiff > maxAngleDiff )
	//	return;

	if( oldAngle < 0 )
		oldAngle += 2 * PI;
	if( newAngle < 0 )
		newAngle += 2 * PI;
	if( pAngle < 0 )
		pAngle += 2 * PI;

	//cout << "p: " << p.x << ", " << p.y << " old: " << oldAngle << ", new: " << newAngle << ", pangle: " << pAngle << endl;
	bool tempClockwise = false;
	if( newAngle > oldAngle )
	{
		if( newAngle - oldAngle < PI )
		{
			tempClockwise = true;
			//cw
			if( pAngle - oldAngle >= 0 && pAngle - oldAngle <= newAngle - oldAngle )
			{
				//good
			}
			else
			{
				return;
			}
		}
		else
		{
			if( pAngle >= newAngle || pAngle <= oldAngle )
			{
				//cw
			}
			else
			{
				return;
			}
		}
	}
	else if( newAngle < oldAngle )
	{
		if( oldAngle - newAngle < PI )
		{
			//ccw
			if( pAngle - newAngle >= 0 && pAngle - newAngle <= oldAngle - newAngle )
			{
				//good
			}
			else
			{
				return;
			}
		}
		else
		{
			tempClockwise = true;
			if( pAngle <= newAngle || pAngle >= oldAngle )
			{
				//ccw
			}
			else
			{
				return;
			}
		}
	}
	else
	{
		return;
	}



	//would be more efficient to remove this calculation and only do it once per frame
	clockwise = tempClockwise;

	
	/*
	points[numPoints + addedPoints].pos = p;
	points[numPoints + addedPoints].angleDiff = angleDiff;
	points[numPoints + addedPoints].e = e;
	addedPoints++;*/
	
	
	if( !foundPoint )
	{
		foundPoint = true;
		closestDiff = angleDiff;
		closestPoint = p;
		//
	}
	else
	{
		double closestDist = length( realAnchor - closestPoint );
		if( angleDiff < closestDiff )
		{
			closestDiff = angleDiff;
			closestPoint = p;
			//cout << "closestPoint: " << p.x << ", " << p.y << endl;
		}
		else if( approxEquals( angleDiff, closestDiff ) )
		{
			if( anchorDist > closestDist )
			{
				closestDiff = angleDiff;
				closestPoint = p;
			}
		}
	}
}

void Wire::HandleEntrant( QuadTreeEntrant *qte )
{
	Edge *e = (Edge*)qte;

	V2d v0 = e->v0;
	V2d v1 = e->v1;

	TestPoint( e );
	//TestPoint( v1 );
}

void Wire::Draw( RenderTarget *target )
{
	if( state == FIRING )
	{
		sf::Vertex line[] =
		{
			sf::Vertex(sf::Vector2f(player->position.x, player->position.y), Color::Blue),
			sf::Vertex(sf::Vector2f(player->position.x + fireDir.x * 40 * framesFiring,
			player->position.y + fireDir.y * 40 * framesFiring), Color::Magenta)
		};

		target->draw(line, 2, sf::Lines);
			
	}
	else if( state == HIT || state == PULLING )
	{
		//V2d wirePos = wireEdge->GetPoint( wireQuant );
		if( numPoints == 0 )
		{
			sf::Vertex line0[] =
			{
				sf::Vertex(sf::Vector2f( player->position.x, player->position.y ), Color::Red),
				sf::Vertex(sf::Vector2f( anchor.pos.x, anchor.pos.y ), Color::Magenta)
			};

			target->draw(line0, 2, sf::Lines);
		}
		else
		{
			sf::Vertex line0[] =
			{
				sf::Vertex(sf::Vector2f( points[numPoints-1].pos.x, points[numPoints-1].pos.y ), Color::Red),
				sf::Vertex(sf::Vector2f( player->position.x, player->position.y ), Color::Magenta)
			};

			target->draw(line0, 2, sf::Lines);
		}

		if( numPoints > 0 )
		{
			sf::Vertex line1[] =
			{
				sf::Vertex(sf::Vector2f( anchor.pos.x, anchor.pos.y ), Color::Red),
				sf::Vertex(sf::Vector2f( points[0].pos.x, points[0].pos.y ), Color::Magenta)
			};

			target->draw(line1, 2, sf::Lines);
		}

		for( int i = 1; i < numPoints; ++i )
		{
		
			sf::Vertex line[] =
			{
				sf::Vertex(sf::Vector2f(points[i-1].pos.x, points[i-1].pos.y ), Color::Red),
				sf::Vertex(sf::Vector2f(points[i].pos.x, points[i].pos.y ), Color::Magenta)
			};

			target->draw(line, 2, sf::Lines);
		}

		CircleShape cs1;
		cs1.setFillColor( Color::Red );
		cs1.setRadius( 10 );
		cs1.setOrigin( cs1.getLocalBounds().width / 2, cs1.getLocalBounds().height / 2 );
		cs1.setPosition( anchor.pos.x, anchor.pos.y );

		target->draw( cs1 );

		for( int i = 0; i < numPoints; ++i )
		{
			CircleShape cs;
			cs.setFillColor( Color::Cyan );
			cs.setRadius( 5 );
			cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
			cs.setPosition( points[i].pos.x, points[i].pos.y );

			target->draw( cs );
		}
	}
}

void Wire::DebugDraw( RenderTarget *target )
{
	for( list<Drawable*>::iterator it = progressDraw.begin(); it != progressDraw.end(); ++it )
	{
		target->draw( *(*it) );
	}
	//progressDraw.clear();
}

void Wire::ClearDebug()
{
	for( list<Drawable*>::iterator it = progressDraw.begin(); it != progressDraw.end(); ++it )
	{
		delete (*it);
	}
	progressDraw.clear();
}