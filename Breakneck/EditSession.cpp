//edit mode
#include "EditSession.h"
#include <fstream>
#include <assert.h>
#include <iostream>
#include "poly2tri/poly2tri.h"

using namespace std;
using namespace sf;

#define V2d sf::Vector2<double>

Polygon::Polygon()
{
	va = NULL;
	lines = NULL;
	selected = false;
	
}

Polygon::~Polygon()
{
	if( lines != NULL )
		delete [] lines;
	if( va != NULL )
		delete va;
}

void Polygon::Finalize()
{

	material = "mat";
	lines = new sf::Vertex[points.size()*2+1];
	int i = 0;

	FixWinding();
	//cout << "points size: " << points.size() << endl;

	vector<p2t::Point*> polyline;
	for( list<Vector2i>::iterator it = points.begin(); it != points.end(); ++it )
	{
		polyline.push_back( new p2t::Point((*it).x, (*it).y ) );
	}

	p2t::CDT * cdt = new p2t::CDT( polyline );
	
	cdt->Triangulate();
	vector<p2t::Triangle*> tris;
	tris = cdt->GetTriangles();
	
	vaSize = tris.size() * 3;
	va = new VertexArray( sf::Triangles , vaSize );
	
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

	//assert( tris.size() * 3 == points.size() );
	delete cdt;
	for( int i = 0; i < points.size(); ++i )
	{
		delete polyline[i];
	//	delete tris[i];
	}

	if( points.size() > 0 )
	{
		list<Vector2i>::iterator it = points.begin(); 
		lines[0] = sf::Vector2f( (*it).x, (*it).y );
		lines[2 * points.size() - 1 ] = sf::Vector2f( (*it).x, (*it).y );
		++it;
		++i;
		while( it != points.end() )
		{
			lines[i] = sf::Vector2f( (*it).x, (*it).y );
			lines[++i] = sf::Vector2f( (*it).x, (*it).y ); 
			++i;
			++it;
		}
	}
	list<Vector2i>::iterator it = points.begin();
	left = (*it).x;
	right = (*it).x;
	top = (*it).y;
	bottom = (*it).y;
	++it;
	for( ; it != points.end(); ++it )
	{
		left = min( (*it).x, left );
		right = max( (*it).x, right );
		top = min( (*it).y, top );
		bottom = max( (*it).y, bottom );
	}
	

	//p2t::Sweep
	//va = new VertexArray( sf::Triangles, points.size() * 3 );
/*	vector<int> working_set;
	list<vector<int>> monotone_poly_list;
	int i = 0;
	for( int i = 0; i < points.size(); ++i )
		working_set.push_back( i++ );

	for( list<Vector2i>::iterator it = points.begin(); it != points.end(); ++it )
	{
		
	}

	while( true )
	{
		vector<int> sorted_vertex_list;
		int n = 0;
		for( int i = 0, 
	}*/



	
	/*for( ; it != points.end(); ++it )
	{
		lines[i] = sf::Vertex((*it));
		lines[i*2+1] = sf::Vertex((*it));
		++i;
		
	}*/
}

void Polygon::Draw( RenderTarget *rt )
{
	
//	rt->draw(lines, points.size()*2, sf::Lines );
	if( va != NULL )
	rt->draw( *va );

	if( selected )
	{
		for( list<Vector2i>::iterator it = points.begin(); it != points.end(); ++it )
		{
			CircleShape cs;
			cs.setRadius( 10 );
			cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
			cs.setFillColor( Color::Green );
			cs.setPosition( (*it).x, (*it).y );
			rt->draw( cs );
		}
		rt->draw( lines, points.size() * 2, sf::Lines );
	}
}

void Polygon::SetSelected( bool select )
{
	selected = select;
	Color selectCol( 0x77, 0xBB, 0xDD );
	if( selected )
	{
		for( int i = 0; i < vaSize; ++i )
		{
			VertexArray & v = *va;
			v[i].color = selectCol;
		}
	}
	else
	{
		Color testColor( 0x75, 0x70, 0x90 );
		for( int i = 0; i < vaSize; ++i )
		{
			VertexArray & v = *va;
			v[i].color = testColor;
		}
	}
}

bool Polygon::ContainsPoint( Vector2f test )
{
	int pointCount = points.size();

	int i, j, c = 0;

	list<Vector2i>::iterator it = points.begin();
	list<Vector2i>::iterator jt = points.end();
	jt--;
	
	for (; it != points.end(); jt = it++ ) 
	{
		Vector2f point((*it).x, (*it).y );
		Vector2f pointJ((*jt).x, (*jt).y );
		if ( ((point.y>test.y) != (pointJ.y>test.y)) &&
			(test.x < (pointJ.x-point.x) * (test.y-point.y) / (pointJ.y-point.y) + point.x) )
				c = !c;
	}
	return c;
}

void Polygon::FixWinding()
{
    if (IsClockwise())
    {
		cout << "not fixing" << endl;
    }
    else
    {
		cout << "fixing winding" << endl;
        list<Vector2i> temp;
		for( list<Vector2i>::iterator it = points.begin(); it != points.end(); ++it )
		{
			temp.push_front( (*it) );
		}
		points.clear();
		for( list<Vector2i>::iterator it = temp.begin(); it != temp.end(); ++it )
		{
			points.push_back( (*it) );
		}          
    }
}

void Polygon::Reset()
{
	points.clear();
	if( lines != NULL )
		delete [] lines;
	if( va != NULL )
		delete va;
	lines = NULL;
	va = NULL;
}

bool Polygon::IsClockwise()
{
	assert( points.size() > 0 );

    Vector2i *pointArray = new Vector2i[points.size()];

	int i = 0;
	for( list<Vector2i>::iterator it = points.begin(); it != points.end(); ++it )
	{
		pointArray[i] = (*it);
		++i;
	}

    long long int sum = 0;
	for (int i = 0; i < points.size(); ++i)
    {
        Vector2<long long int> first, second;
		
        if (i == 0)
        {
			first.x = pointArray[points.size() - 1].x;
			first.y = pointArray[points.size() - 1].y;
        }
        else
        {
            first.x = pointArray[i - 1].x;
			first.y = pointArray[i - 1].y;

        }
        second.x = pointArray[i].x;
		second.y = pointArray[i].y;

        sum += (second.x - first.x) * (second.y + first.y);
    }

	delete [] pointArray;

    return sum < 0;
}

bool Polygon::IsTouching( Polygon *p )
{
	if( left <= p->right && right >= p->left && top <= p->bottom && bottom >= p->top )
	{	
		/*(for( list<Vector2i>::iterator it = p->points.begin(); it != p->points.end(); ++it )
		{

		}*/
		//aabbCollision = true;
		return true;
	}
	return false;
}

EditSession::EditSession( RenderWindow *wi)
	:w( wi ), zoomMultiple( 1 )
{
//	VertexArray *va = new VertexArray( sf::Lines, 
//	progressDrawList.push( new 
}

EditSession::~EditSession()
{
	delete polygonInProgress;
	for( list<Polygon*>::iterator it = polygons.begin(); it != polygons.end(); ++it )
	{
		delete (*it);
	}
}

void EditSession::Draw()
{
	for( list<Polygon*>::iterator it = polygons.begin(); it != polygons.end(); ++it )
	{
		(*it)->Draw( w );
	}

	int psize = polygonInProgress->points.size();
	if( psize > 0 )
	{
		CircleShape cs;
		cs.setRadius( 5 * zoomMultiple  );
		cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
		cs.setFillColor( Color::Green );

		
		for( list<Vector2i>::iterator it = polygonInProgress->points.begin(); it != polygonInProgress->points.end(); ++it )
		{
			cs.setPosition( (*it).x, (*it).y );
			w->draw( cs );
		}		
	}

	
}

bool EditSession::OpenFile( string fileName )
{
	currentFile = fileName;

	ifstream is;
	is.open( fileName + ".brknk" );
	if( is.is_open() )
	{
		int numPoints;
		is >> numPoints;
		is >> playerPosition.x;
		is >> playerPosition.y;

		while( numPoints > 0 )
		{
			Polygon *poly = new Polygon;
			polygons.push_back( poly );
			is >> poly->material;
			int polyPoints;
			is >> polyPoints;
			
			numPoints -= polyPoints;
			int x,y;
			for( int i = 0; i < polyPoints; ++i )
			{
				is >> x;
				is >> y;
				poly->points.push_back( Vector2i(x,y) );
			}

			poly->Finalize();
		}

		is.close();




	}
	else
	{

		//new file
		assert( false && "error getting file to edit " );
	}
	
}

void EditSession::WriteFile(string fileName)
{
	ofstream of;
	of.open( fileName + ".brknk" );

	int pointCount = 0;
	for( list<Polygon*>::iterator it = polygons.begin(); it != polygons.end(); ++it )
	{
		pointCount += (*it)->points.size();
	}

	of << pointCount << endl;
	of << playerPosition.x << " " << playerPosition.y << endl;

	for( list<Polygon*>::iterator it = polygons.begin(); it != polygons.end(); ++it )
	{
		of << (*it)->material << " " << (*it)->points.size() << endl;
		for( list<Vector2i>::iterator it2 = (*it)->points.begin(); it2 != (*it)->points.end(); ++it2 )
		{
			of << (*it2).x << " " << (*it2).y << endl;
		}
	}
}

void EditSession::Add2( Polygon *brush, Polygon *poly )
{
	Polygon z;
	//1: choose start point

	Vector2i startPoint;
	bool startPointFound = false;

	Polygon *currentPoly = NULL;
	Polygon *otherPoly = NULL;
	list<Vector2i>::iterator it = poly->points.begin();
	for(; it != poly->points.end(); ++it )
	{
		if( !brush->ContainsPoint( Vector2f( (*it).x, (*it).y) ) )
		{
			startPoint = (*it);
			startPointFound = true;
			currentPoly = poly;
			otherPoly = brush;
			break;
		}
	}

	if( !startPointFound )
	{
		it = brush->points.begin();
		for(; it != brush->points.end(); ++it )
		{
			if( !poly->ContainsPoint( Vector2f( (*it).x, (*it).y) ) )
			{
				startPoint = (*it);
				startPointFound = true;
				currentPoly = brush;
				otherPoly = poly;
				break;
			}
		}
	}

	assert( startPointFound );
	
	Vector2i currPoint = startPoint;
	++it;
	if( it == currentPoly->points.end() )
	{
		it = currentPoly->points.begin();
	}
	Vector2i nextPoint = (*it);


	z.points.push_back( startPoint );

	//2. run a loobclockwise until you arrive back at the original state
	bool firstTime = true;
	while( firstTime || currPoint != startPoint )
	{
		cout << "stbart loop: " << currPoint.x << ", " << currPoint.y << endl;
		firstTime = false;


		
		list<Vector2i>::iterator min;
		Vector2i minIntersection;
		bool emptyInter = true;
		

		list<Vector2i>::iterator bit = otherPoly->points.begin();
		Vector2i bCurrPoint = (*bit);
		++bit;
		Vector2i bNextPoint;// = (*++bit);
		Vector2i minPoint;
		
		LineIntersection li = SegmentIntersect( currPoint, nextPoint, otherPoly->points.back(), bCurrPoint );
		if( !li.parallel && length( li.position - V2d( currPoint.x, currPoint.y ) ) >= 1 )
		{
			minIntersection.x = li.position.x;
			minIntersection.y = li.position.y;
			minPoint = bCurrPoint;
			min = --bit;
			++bit;
			emptyInter = false;
		}

		for(; bit != otherPoly->points.end(); ++bit )
		{
			bNextPoint = (*bit);
			LineIntersection li = SegmentIntersect( currPoint, nextPoint, bCurrPoint, bNextPoint );
			Vector2i lii( floor(li.position.x + .5), floor(li.position.y + .5) );
			if( !li.parallel && length( li.position - V2d( currPoint.x, currPoint.y ) ) >= 1 ) //just means its invalid
			{
				if( emptyInter )
				{
					emptyInter = false;

					minIntersection.x = li.position.x;
					minIntersection.y = li.position.y;
					minPoint = bNextPoint;
					min = bit;
				}
				else
				{
					Vector2i blah( minIntersection - currPoint );
					if( length( li.position - V2d(currPoint.x, currPoint.y) ) < length( V2d( blah.x, blah.y ) ) )
					{
						minIntersection = lii;
						minPoint = bNextPoint;
						min = bit;
					}
				}

					
			}
			bCurrPoint = bNextPoint;
			
		}

		if( !emptyInter )
		{
			//cout << "switching polygon and adding point" << endl;
			
			//push back intersection
			Polygon *temp = currentPoly;
			currentPoly = otherPoly;
			otherPoly = temp;
			it = min;
			

			currPoint = minIntersection;

			z.points.push_back( currPoint );

			nextPoint = (*it);

			if( nextPoint == startPoint )
				break;
			//cout << "fff: " << (*it).x << ", " << (*it).y << endl;
			

			if( otherPoly == poly )
				cout << "switching to brush: " << currPoint.x << ", " << currPoint.y << endl;
			else
				cout << "switching to poly: " << currPoint.x << ", " << currPoint.y << endl;
			cout << "nextpoint : " << nextPoint.x << ", " << nextPoint.y << endl;
		}
		else
		{
			
			currPoint = (*it);

			if( currPoint == startPoint )
				break;

			cout << "adding point: " << currPoint.x << ", " << currPoint.y << endl;

			z.points.push_back( currPoint );

			++it;
			if( it == currentPoly->points.end() )
			{
				it = currentPoly->points.begin();
			}
			nextPoint = (*it);
		}
	}

	poly->Reset();
	for( list<Vector2i>::iterator zit = z.points.begin(); zit != z.points.end(); ++zit )
	{
		poly->points.push_back( (*zit) );
	}
	cout << "before killer finalize. poly size: " << poly->points.size() << endl;
	poly->Finalize();

	
}
void EditSession::Add( Polygon *brush, Polygon *poly)
{
	cout << "made it here before crashing. brush size: " << brush->points.size() << ", poly size: " << poly->points.size() << endl;
	

	Vector2i startPoint;
	bool startPointFound = false;
	list<Vector2i>::iterator it = poly->points.begin();
	for(; it != poly->points.end(); ++it )
	{
		if( !brush->ContainsPoint( Vector2f( (*it).x, (*it).y) ) )
		{
			startPoint = (*it);
			startPointFound = true;
			break;
		}
	}

	list<Vector2i> *bp; 
	list<Vector2i> *pp; 
	if( startPointFound )
	{
		bp = &brush->points;
		pp = &poly->points;
		//list<Vector2i> & brushP = brush->points;b
		//list<Vector2i> & polyP = poly->points;
		//1. choose a point on poly (any point works equally well)
		//Vector2i point = polyP.front();

		//2. move counter clockwise until you collide with brush
		//3. move on brush clockwise until you reach the starting point again, switching off
		//   between 
	}
	else
	{
		bp = &poly->points;
		pp = &brush->points;
	}
	list<Vector2i> & brushP = *bp;
	list<Vector2i> & polyP = *pp;

	if( startPointFound )
	{
		Polygon z;
		z.points.push_back( startPoint );
		Vector2i prev = startPoint;
		++it;
		bool starting = true;
		while( true )
		{
			cout << "main loop start: " << startPoint.x << ", " << startPoint.y << endl;
			if( it == polyP.end() )
			{
				it = polyP.begin();
			}

			Vector2i curr = (*it);

		//	if( starting )
			{
				//starting = false;
			}
			//else if( prev == startPoint )
			if( curr == startPoint )
			{
				starting = false;
				//break;
			}

			Vector2i activeEdge( curr - prev );
			list<Vector2i>::iterator bit = brushP.begin();
			Vector2i bPrev = (*bit);
			bit++;
			list<Vector2i>::iterator min;
			Vector2i minIntersection;
			bool emptyInter = true;

			brushP.push_back( bPrev );

			for(; bit != brushP.end(); ++bit )
			{
				cout << "second loop attemp: " << prev.x << ", " << prev.y << " to " << curr.x << curr.y << endl; // (*bit).x << ", " << (*bit).y  << endl;
				Vector2i bCurr = (*bit);
				LineIntersection li = SegmentIntersect( prev, curr, bPrev, bCurr );
				
				if( !li.parallel ) //just means its invalid
				{
					if( emptyInter )
					{
						minIntersection.x = li.position.x;
						minIntersection.y = li.position.y;
						min = bit;
						emptyInter = false;
					}
					else
					{
						Vector2i lii( li.position.x, li.position.y );
						Vector2i blah( minIntersection - prev );
						if( length( li.position - V2d(prev.x, prev.y) ) < length( V2d( blah.x, blah.y ) ) )
						{
							minIntersection = lii;
							min = bit;
						}
					}

					
				}
				bPrev = bCurr;

				
			}


			
			

			if( !emptyInter )
			{
				z.points.push_back( Vector2i( floor( minIntersection.x + .5 ), floor( minIntersection.y + .5 ) ) );
				cout << "2nd break: " << z.points.back().x << ", " << z.points.back().y << endl;
				list<Vector2i>::iterator temp = min;
				if( (*min) == (*brushP.rbegin()) )
				{
					min = brushP.begin();
				}
				bit = min;
			}

			brushP.pop_back();

			bool nextCurrModified = false;
			if( bit != brushP.end() )
			{
				Vector2i bStart = (*bit);
				cout << "pushing start of b: " << bStart.x << ", " << bStart.y << endl;
				z.points.push_back( (*bit) );
				bPrev = bStart;
				++bit;
				
				while( true )
				{
					cout << "third loop" << endl;
					if( bit == brushP.end() )
					{
						cout << "third loop thing" << endl;
						bit = brushP.begin(); 
					}

					Vector2i bCurr = (*bit);

					if( (*bit) == bStart )
					{
						//++bit;
						//./continue;
					//	assert( 0 && "I totally failed" );
					}

					

					list<Vector2i>::iterator cit = polyP.begin();
					Vector2i cPrev = (*cit);
					cit++;
					list<Vector2i>::iterator cmin;
					Vector2i cminIntersection;
					bool cemptyInter = true;
					bool exit = false;
					polyP.push_back( polyP.front() );
					Vector2i nextCurr;
					//nextCurrModified = false;
					for(; cit != polyP.end(); ++cit )
					{
						Vector2i cCurr = (*cit);
						LineIntersection li = SegmentIntersect( bPrev, bCurr, cPrev, cCurr);
				
						if( !li.parallel ) //just means its invalid
						{
							cout << "cprev: " << cPrev.x << ", " << cPrev.y << endl;
							if( cemptyInter )
							{
								cminIntersection.x = li.position.x;
								cminIntersection.y = li.position.y;
								cout << "first inter: " << cminIntersection.x << ", " << cminIntersection.y << endl;
								cmin = cit;
								nextCurr = cPrev;
								prev = cPrev;
								cemptyInter = false;
								
							}
							else
							{
								cout << "attemping switch" << endl;
								Vector2i lii( li.position.x, li.position.y );
								Vector2i blah( cminIntersection - bPrev );
								if( length( li.position - V2d(bPrev.x, bPrev.y) ) < length( V2d( blah.x, blah.y ) ) )
								{
									cminIntersection = lii;
									cmin = cit;
									nextCurr = cPrev;
								//	prev = cPrev;
									cout << "switching: " << length( li.position - V2d(cPrev.x, cPrev.y) )
										<< ", " << length( V2d( blah.x, blah.y ) ) << endl;
									cout << "switchinter: " << cminIntersection.x << ", " << cminIntersection.y << endl;
								}
							}

							if( (*cmin) == polyP.back() )
							{
								cmin = polyP.begin();
							}
						}
						cPrev = cCurr;
					}
					polyP.pop_back();
						
					if( !cemptyInter )
					{
						z.points.push_back( Vector2i( floor( cminIntersection.x + .5 ), floor( cminIntersection.y + .5 ) ) );
						cout << "3nd break: " << z.points.back().x << ", " << z.points.back().y << endl;
						cit = cmin;
						curr = nextCurr;//(*cit);
						it = cmin;
						cout << "nextCurr: " << nextCurr.x << ", " << nextCurr.y << endl;
						nextCurrModified = true;
						break;
					}



						/*if( !li.parallel ) //just means its invalid
						{
							bool aaaa = false;
							for( list<Vector2i>::iterator ii = polyP.begin(); ii != polyP.end(); ++ii )
							{
								if( (*ii ) == cPrev )
								{
									aaaa = true;
									break;
									//continue;
								}
								
							}
							if( aaaa )continue;
							cout << "break 3" << endl;
							z.points.push_back( Vector2i( floor( li.position.x), floor( li.position.y) ) );
							exit = true;
							curr = cCurr;
							//it++;
							//prev = curr;
							break;
						}
						cPrev = cCurr;*/
						
					
					
					if( exit )
						break;

					if( bCurr == bStart )
						break;
					else
					{
						cout << "pushing b end: " << bCurr.x << ", " << bCurr.y << endl;
						z.points.push_back( bCurr );
					}

					bPrev = bCurr;
					++bit;
				}

			}

			//assert( z.points.back() != curr );
			if( curr == startPoint )
			{
				//break;
			}
			if( z.points.back() != curr )
			{
				
			}
			if( !starting )
				break;

			
			if( !nextCurrModified )
			{
				prev = curr;
				++it;
				cout << "pushing main 1 1: " << curr.x << ", " << curr.y << endl;
				z.points.push_back( curr );
			}
			else
			{
				prev = (*it);
				cout << "pushing main: 2 2: " << (*it).x << ", " << (*it).y << endl;
				if( (*it) == startPoint )
					break;
				else
					z.points.push_back( (*it) );

				++it;
			}
			

			for( list<Vector2i>::iterator ai = polyP.begin(); ai != polyP.end(); ++ai )
			{
				for( list<Vector2i>::iterator bi = polyP.begin(); bi != polyP.end(); ++bi )
				{
					if( ai == bi )
						continue;
					else
					{
						assert( (*ai) != (*bi ) );
					}
				}	
			}
		}

		poly->Reset();
		for( list<Vector2i>::iterator zit = z.points.begin(); zit != z.points.end(); ++zit )
		{
			poly->points.push_back( (*zit) );
		}
		cout << "before killer finalize. poly size: " << poly->points.size() << endl;
		poly->Finalize();
	}
	else
	{

		assert( 0 && "here we are" );
	}

	
}

LineIntersection EditSession::SegmentIntersect( Vector2i a, Vector2i b, Vector2i c, Vector2i d )
{
	LineIntersection li = lineIntersection( V2d( a.x, a.y ), V2d( b.x, b.y ), 
				V2d( c.x, c.y ), V2d( d.x, d.y ) );
	if( !li.parallel )
	{
		double e1Left = min( a.x, b.x );
		double e1Right = max( a.x, b.x );
		double e1Top = min( a.y, b.y );
		double e1Bottom = max( a.y, b.y );

		double e2Left = min( c.x, d.x );
		double e2Right = max( c.x, d.x );
		double e2Top = min( c.y, d.y );
		double e2Bottom = max( c.y, d.y );
		//cout << "compares: " << e1Left << ", " << e2Right << " .. " << e1Right << ", " << e2Left << endl;
		//cout << "compares y: " << e1Top << " <= " << e2Bottom << " && " << e1Bottom << " >= " << e2Top << endl;
		if( e1Left <= e2Right && e1Right >= e2Left && e1Top <= e2Bottom && e1Bottom >= e2Top )
		{
			//cout << "---!!!!!!" << endl;
			if( li.position.x <= e1Right && li.position.x >= e1Left && li.position.y >= e1Top && li.position.y <= e1Bottom)
			{
				if( li.position.x <= e2Right && li.position.x >= e2Left && li.position.y >= e2Top && li.position.y <= e2Bottom)
				{
					//cout << "seg intersect!!!!!!" << endl;
					//assert( 0 );
					return li;
				}
			}
		}
	}
	//cout << "return false" << endl;
	li.parallel = true;
	return li;
}

int EditSession::Run( string fileName, Vector2f cameraPos, Vector2f cameraSize )
{
	int returnVal = 0;
	w->setMouseCursorVisible( true );
	Color testColor( 0x75, 0x70, 0x90 );
	View view( cameraPos, cameraSize );
	if( cameraSize.x == 0 && cameraSize.y == 0 )
		view.setSize( 960, 540 );

	w->setView( view );
	Texture playerTex;
	playerTex.loadFromFile( "stand.png" );
	sf::Sprite playerSprite( playerTex );
	playerSprite.setTextureRect( IntRect(0, 0, 64, 64 ) );
	playerSprite.setOrigin( playerSprite.getLocalBounds().width / 2, playerSprite.getLocalBounds().height / 2 );

	w->setVerticalSyncEnabled( false );

	OpenFile( fileName );
	//Vector2f vs(  );
	if( cameraSize.x == 0 && cameraSize.y == 0 )
		view.setCenter( (float)playerPosition.x, (float)playerPosition.y );

	mode = "neutral";
	bool quit = false;
	polygonInProgress = new Polygon();
	zoomMultiple = 1;
	Vector2<double> prevWorldPos;
	Vector2i pixelPos;
	Vector2f tempWorldPos = w->mapPixelToCoords(sf::Mouse::getPosition( *w ));
	Vector2<double> worldPos = Vector2<double>( tempWorldPos.x, tempWorldPos.y );
	bool panning = false;
	Vector2<double> panAnchor;
	bool backspace = true;
	minimumEdgeLength = 8;

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

	

	bool s = sf::Keyboard::isKeyPressed( sf::Keyboard::T );
	

	while( !quit )
	{
		w->clear();
		 prevWorldPos = worldPos;
		 pixelPos = sf::Mouse::getPosition( *w );
		 Vector2f tempWorldPos = w->mapPixelToCoords(pixelPos);
		 worldPos.x = tempWorldPos.x;
		 worldPos.y = tempWorldPos.y;

		if( Mouse::isButtonPressed( Mouse::Left ) )
		{
			if( mode == "neutral" && !panning )
			{

				bool emptySpace = true;
				for( list<Polygon*>::iterator it = polygons.begin(); it != polygons.end(); ++it )
				{
					if((*it)->ContainsPoint( Vector2f(worldPos.x, worldPos.y ) ) )
					{
						//emptySpace = false;
						
						break;
					}
				}

				if( emptySpace )
				{
					if( length( worldPos - Vector2<double>(polygonInProgress->points.back().x, 
						polygonInProgress->points.back().y )  ) >= minimumEdgeLength * std::max(zoomMultiple,1.0 ))
					{
						Vector2i worldi( worldPos.x, worldPos.y );
						
						if( polygonInProgress->points.size() > 0 )
						{
							if( PointValid( polygonInProgress->points.back(), worldi ) )
							{
								
								//cout << "point valid" << endl;
								polygonInProgress->points.push_back( worldi  );
								for( list<Polygon*>::iterator it = polygons.begin(); it != polygons.end(); ++it )
								{
									//(*it)->SetSelected( false );					
								}
							}
							//else
								//cout << "INVALID" << endl;
							
						}
						else
						{
							bool okay = true;
							for( list<Polygon*>::iterator it = polygons.begin(); it != polygons.end(); ++it )
							{
								if( (*it)->ContainsPoint( Vector2f(worldi.x, worldi.y) ) )
								{
									okay = false;
									break;
								}

							//	(*it)->SetSelected( false );					
							}
							if( okay)
							polygonInProgress->points.push_back( worldi  );
						
						}
						

					}
				}
				else
				{
					//polygonInProgress->points.clear();
				}
			}
			else if( mode == "set player" )
			{
				playerPosition.x = (int)worldPos.x;
				playerPosition.y = (int)worldPos.y;//Vector2i( worldPos.x, worldPos.y );
				mode = "transition";
			}
			
		}

		if( mode == "transition" && !Mouse::isButtonPressed( Mouse::Left ) )
		{
			mode = "neutral";
		}

		if( mode == "transition1" && !sf::Keyboard::isKeyPressed( sf::Keyboard::S)  )
		{
			mode = "neutral";
		}

		if( mode == "neutral" && Keyboard::isKeyPressed( Keyboard::Q ) )
		{
		//	mode = "editpoints";
		}

		sf::Event ev;
		while( w->pollEvent( ev ) )
		{
		
		if( ev.type == Event::MouseWheelMoved )
		{
			//if( prevDelta - ev.mouseWheel.delta != 0)
			{
				//view.zoom( ev.mouseWheel.delta / 5 );
				if( ev.mouseWheel.delta > 0 )
				{
					zoomMultiple /= 2;
				}
				else if( ev.mouseWheel.delta < 0 )
				{
					zoomMultiple *= 2;
				}

				if( zoomMultiple < .25 )
				{
					zoomMultiple = .25;
					cout << "min zoom" << endl;
				}
				else if( zoomMultiple > 65536 )
				{
					zoomMultiple = 65536;
				}
				else if( abs(zoomMultiple - 1.0) < .1 )
				{
					zoomMultiple = 1;
				}
				
				Vector2<double> ff = Vector2<double>(view.getCenter().x, view.getCenter().y );//worldPos - ( - (  .5f * view.getSize() ) );
				view.setSize( Vector2f( 960 * (zoomMultiple), 540 * ( zoomMultiple ) ) );
				w->setView( view );
				Vector2f newWorldPosTemp = w->mapPixelToCoords(pixelPos);
				Vector2<double> newWorldPos( newWorldPosTemp.x, newWorldPosTemp.y );
				Vector2<double> tempCenter = ff + ( worldPos - newWorldPos );
				view.setCenter( tempCenter.x, tempCenter.y );
				w->setView( view );
			}
		
		}
		else if( ev.type == Event::MouseMoved )
		{
			
			if( Mouse::isButtonPressed( Mouse::Middle ) )
			{
			}
		}
		else if( ev.type == Event::MouseButtonPressed )
		{
			if( ev.mouseButton.button == Mouse::Button::Middle )
			{
				panning = true;
				panAnchor = worldPos;
			}
			
		}
		else if( ev.type == Event::MouseButtonReleased )
		{
			if( ev.mouseButton.button == Mouse::Button::Middle )
			{
				panning = false;
			}
			else if( ev.mouseButton.button == Mouse::Button::Left )
			{
				
				if( mode == "neutral" )
				{
					bool emptySpace = true;
					if( polygonInProgress->points.size() == 0 )
					{
						for( list<Polygon*>::iterator it = polygons.begin(); it != polygons.end(); ++it )
						{
						
								if((*it)->ContainsPoint( Vector2f(worldPos.x, worldPos.y ) ) )
								{
									emptySpace = false;
									(*it)->SetSelected( !((*it)->selected ) );
									//if( (*it)->selected == true )
									//polygonInProgress->points.clear();
									break;
								}
						}
					}

					if( emptySpace )
					{

					}
					else
					{
						polygonInProgress->points.clear();
					}

				}
			}
			else if( ev.mouseButton.button == Mouse::Button::Right )
			{
				for( list<Polygon*>::iterator it = polygons.begin(); it != polygons.end(); ++it )
				{
					(*it)->SetSelected( false );					
				}
			}
		}
		}

		if( panning )
		{
			Vector2<double> temp = panAnchor - worldPos;
			view.move( Vector2f( temp.x, temp.y ) );
		}

		if( !s && sf::Keyboard::isKeyPressed( sf::Keyboard::T ) )
		{
			quit = true;
			break;
			//t = true;
		}
		else if( s && !sf::Keyboard::isKeyPressed( sf::Keyboard::T ) )
		{
			s = false;

		}

		if( sf::Keyboard::isKeyPressed( sf::Keyboard::Escape ) )
		{
			quit = true;
			returnVal = 1;
			break;
		}

		if( sf::Keyboard::isKeyPressed( sf::Keyboard::B) )
		{
			if( mode == "neutral" )
			{
				mode = "set player";
				polygonInProgress->points.clear();
			}
		}
		else if( sf::Keyboard::isKeyPressed( sf::Keyboard::Q ) )
		{
			list<Polygon*>::iterator it = polygons.begin();
			while( it != polygons.end() )
			{
				if( (*it)->selected )
				{
					delete (*it);
					polygons.erase( it++ );
				}
				else
					++it;
			}
		}
		else if( sf::Keyboard::isKeyPressed( sf::Keyboard::Space ) )
		{
			if( mode == "neutral" && polygonInProgress->points.size() > 2 )
			{
				list<Polygon*>::iterator it = polygons.begin();
				bool added = false;
				polygonInProgress->Finalize();
				while( it != polygons.end() )
				{
					if( polygonInProgress->IsTouching( (*it) ) )
					{
						
						cout << "before adding: " << (*it)->points.size() << endl;
						Add2( polygonInProgress, (*it) );
						cout << "after adding: " << (*it)->points.size() << endl;
						
						added = true;
						break;
					}
					else
					{
						//cout << "not" << endl;
					}
					++it;
				}
				//add final check for validity here
				
				if( !added )
				{
					polygonInProgress->Finalize();
					polygons.push_back( polygonInProgress );
					polygonInProgress = new Polygon();
				}
				else
				{
					polygonInProgress->Reset();
					//polygonInProgress->points.clear();
				}
			}

			if( polygonInProgress->points.size() <= 2 && polygonInProgress->points.size() > 0  )
			{
				cout << "cant finalize. cant make polygon" << endl;
				polygonInProgress->points.clear();
			}
		}
		else if( sf::Keyboard::isKeyPressed( sf::Keyboard::S ) )
		{
			if( mode == "neutral")
			{
				polygonInProgress->points.clear();
				string fName;
				mode = "transition1";
				fName = "test1";
				cout << "writing to file: " << fName << ".brknk" << endl;
				WriteFile(fName);
			}
		}
		
		
		if( sf::Keyboard::isKeyPressed( sf::Keyboard::BackSpace ) )
		{
			if( mode == "neutral" && polygonInProgress->points.size() > 0 && backspace )
			{
				polygonInProgress->points.pop_back();
				backspace = false;
			}
		}
		else
			backspace = true;

		if( mode == "set player" )
		{
			playerSprite.setPosition( w->mapPixelToCoords(sf::Mouse::getPosition( *w )) );
			//cout << "placing: " << playerSprite.getPosition().x << ", " << playerSprite.getPosition().y << endl;
		}
		else
			playerSprite.setPosition( playerPosition.x, playerPosition.y );

		w->setView( view );
		w->draw(border, 8, sf::Lines);
		int progressSize = polygonInProgress->points.size();
		if( progressSize > 0 )
		{
			Vector2i backPoint = polygonInProgress->points.back();
			
			Color validColor = Color::Green;
			Color invalidColor = Color::Red;
			Color colorSelection;
			if( true )
			{
				colorSelection = validColor;
			}
			sf::Vertex activePreview[2] =
			{
				sf::Vertex(sf::Vector2<float>(backPoint.x, backPoint.y), colorSelection ),
				sf::Vertex(sf::Vector2<float>(worldPos.x, worldPos.y), colorSelection)
			};
			w->draw( activePreview, 2, sf::Lines );

			if( progressSize > 1 )
			{
				VertexArray v( sf::LinesStrip, progressSize );
				int i = 0;
				for( list<sf::Vector2i>::iterator it = polygonInProgress->points.begin(); 
					it != polygonInProgress->points.end(); ++it )
				{
					v[i] = Vertex( Vector2f( (*it).x, (*it).y ) );
					++i;
				}
				w->draw( v );
				//Vertex *p[polygonInProgress->points.size()]
			}
			
			
		}


		Draw();
		w->draw( playerSprite );
		w->display();
	}
	
	return returnVal;
	
}

bool EditSession::PointValid( Vector2i prev, Vector2i point)
{
	return true;
	float eLeft = min( prev.x, point.x );
	float eRight= max( prev.x, point.x );
	float eTop = min( prev.y, point.y );
	float eBottom = max( prev.y, point.y );
	{
		list<Vector2i>::iterator it = polygonInProgress->points.begin();
		//polygonInProgress->points.push_back( polygonInProgress->points.back() )
		Vector2i pre = (*it);
		++it;
		for( ; it != polygonInProgress->points.end(); ++it )
		{
			if( (*it) == polygonInProgress->points.back() )
				continue;
			LineIntersection li = lineIntersection( V2d( prev.x, prev.y ), V2d( point.x, point.y ),
						V2d( pre.x, pre.y ), V2d( (*it).x, (*it).y ) );
			float tempLeft = min( pre.x, (*it).x ) - 0;
			float tempRight = max( pre.x, (*it).x ) + 0;
			float tempTop = min( pre.y, (*it).y ) - 0;
			float tempBottom = max( pre.y, (*it).y ) + 0;
			if( !li.parallel )
			{
				
				double separation = length( V2d(point.x, point.y) - V2d((*it).x,(*it).y ) );
				
				if( li.position.x <= tempRight && li.position.x >= tempLeft && li.position.y >= tempTop && li.position.y <= tempBottom )
				{
					if( li.position.x <= eRight && li.position.x >= eLeft && li.position.y >= eTop && li.position.y <= eBottom )
					{
						CircleShape cs;
						cs.setRadius( 30  );
						cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
						cs.setFillColor( Color::Magenta );
						cs.setPosition( li.position.x, li.position.y );
						w->draw( cs );

						return false;
					}

				}

				if( separation < minimumEdgeLength )
				{
					
					return false;
				}

				Vector2i ai = point - pre;
				Vector2i bi = (*it) - pre;
				V2d a(ai.x, ai.y);
				V2d b(bi.x, bi.y);
				double res = abs(cross( a, normalize( b )));
				double des = dot( a, normalize( b ));

				Vector2i ci = (*it) - prev;
				Vector2i di = point - prev;
				V2d c( ci.x, ci.y);
				V2d d( di.x, di.y );

				double res2 = abs( cross( c, normalize( d ) ) );
				double des2 = dot( c, normalize( d ) );

				cout << "minedgelength: " << minimumEdgeLength <<  ", " << res << endl;
				if(( res  < minimumEdgeLength && ( des >= 0 && des <= length( b ) ) )
					|| ( res2  < minimumEdgeLength && ( des2 >= 0 && des2 <= length( d ) ) ) )
				{
					return false;
				}
			}
			else
			{
				//cout << "parallel" << endl;
				//return false;
			}
			pre = (*it);
		}
	}
	//return true;

	int i = 0;
	for( list<Polygon*>::iterator it = polygons.begin(); it != polygons.end(); ++it )
	{
		//cout << "polygon " << i << " out of " << polygons.size() << " ... " << (*it)->points.size()  << endl;
		++i;
		Polygon *p = (*it);
		
		if( eLeft <= p->right && eRight >= p->left && eTop <= p->bottom && eBottom >= p->top )
		{	
		
			//aabbCollision = true;
		
		}
		else
		{
			continue;
		}
	//	if( point.x <= p->right && point.x >= p->left && point.y >= p->top && point.y <= p->bottom )
	//	{
			list<sf::Vector2i>::iterator it2 = p->points.begin();
			Vector2i prevPoint = (*it2);
			++it2;
			for( ; it2 != p->points.end(); ++it2 )
			{
				LineIntersection li = lineIntersection( V2d( prevPoint.x, prevPoint.y ), V2d((*it2).x, (*it2).y),
					V2d( prev.x, prev.y ), V2d( point.x, point.y ) );
				float tempLeft = min( prevPoint.x, (*it2).x );
				float tempRight = max( prevPoint.x, (*it2).x );
				float tempTop = min( prevPoint.y, (*it2).y );
				float tempBottom = max( prevPoint.y, (*it2).y );
				if( !li.parallel )
				{
					if( li.position.x <= tempRight && li.position.x >= tempLeft && li.position.y >= tempTop && li.position.y <= tempBottom )
					{
						if( li.position.x <= eRight && li.position.x >= eLeft && li.position.y >= eTop && li.position.y <= eBottom )
						{
							return false;
						}

					}
				}
				prevPoint = (*it2);
				
			}
	}
	return true;
}

