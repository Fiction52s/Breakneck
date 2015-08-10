//edit mode

#include "GUI.h"
#include "EditSession.h"
#include <fstream>
#include <assert.h>
#include <iostream>
#include "poly2tri/poly2tri.h"
#include <sstream>

using namespace std;
using namespace sf;

#define V2d sf::Vector2<double>

TerrainPolygon::TerrainPolygon()
{
	va = NULL;
	lines = NULL;
	selected = false;
	
}

TerrainPolygon::~TerrainPolygon()
{
	if( lines != NULL )
		delete [] lines;
	if( va != NULL )
		delete va;
}

void TerrainPolygon::Finalize()
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

void TerrainPolygon::Draw( RenderTarget *rt )
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

void TerrainPolygon::SetSelected( bool select )
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

bool TerrainPolygon::ContainsPoint( Vector2f test )
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

void TerrainPolygon::FixWinding()
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

void TerrainPolygon::Reset()
{
	points.clear();
	if( lines != NULL )
		delete [] lines;
	if( va != NULL )
		delete va;
	lines = NULL;
	va = NULL;
}

bool TerrainPolygon::IsClockwise()
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

bool TerrainPolygon::IsTouching( TerrainPolygon *p )
{
	assert( p != this );
	if( left <= p->right && right >= p->left && top <= p->bottom && bottom >= p->top )
	{	
		//return true;

		//points.push_back( points.front() );
		//p->points.push_back( p->points.front() );

		list<Vector2i>::iterator it = points.begin();
		Vector2i curr = (*it);
		++it;
		Vector2i next;
		

		

		for( ;; ++it )
		{
			if( it == points.end() )
				it = points.begin();

			next = (*it);


			list<Vector2i>::iterator pit = p->points.begin();
			Vector2i pcurr = (*pit);
			++pit;
			Vector2i pnext;// = (*pit);

			for( ;; ++pit )		
			{
				if( pit == p->points.end() )
					pit = p->points.begin();

				pnext = (*pit);
			
				LineIntersection li = EditSession::SegmentIntersect( curr, next, pcurr, pnext );	

				if( !li.parallel )
				{
					//points.pop_back();
					//p->points.pop_back();
					cout << "touching!" << endl;
					return true;
				}

				pcurr = (*pit);

				if( pit == p->points.begin() )
					break;
			}
			curr = (*it);

			if( it == points.begin() )
			{
				break;
			}
		}
	}

	//points.pop_back();
	//p->points.pop_back();

	return false;
}

EditSession::EditSession( RenderWindow *wi)
	:w( wi ), zoomMultiple( 1 )
{

	minAngle = .99;
	//	VertexArray *va = new VertexArray( sf::Lines, 
//	progressDrawList.push( new 
}

EditSession::~EditSession()
{
	delete polygonInProgress;
	for( list<TerrainPolygon*>::iterator it = polygons.begin(); it != polygons.end(); ++it )
	{
		delete (*it);
	}
}

void EditSession::Draw()
{
	for( list<TerrainPolygon*>::iterator it = polygons.begin(); it != polygons.end(); ++it )
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

		is >> goalPosition.x;
		is >> goalPosition.y;

		while( numPoints > 0 )
		{
			TerrainPolygon *poly = new TerrainPolygon;
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

		//enemies here
		int numGroups;
		is >> numGroups;
		cout << "num groups " << numGroups << endl;
		for( int i = 0; i < numGroups; ++i )
		{
			string groupName;
			is >> groupName;

			int numActors;
			is >> numActors;

			ActorGroup *gr = new ActorGroup( groupName );
			groups[groupName] = gr;

			for( int j = 0; j < numActors; ++j )
			{
				string typeName;
				is >> typeName;

				ActorParams *a = new ActorParams;
				gr->actors.push_back( a );


				ActorType *at;
				cout << "typename: " << typeName << endl;
				if( types.count( typeName ) == 0 )
				{
					assert( false && "bad typename" );
				//	at = new ActorType( typeName, CreateOptionsPanel( typeName ) );
				//	types[typeName] = at;
				}
				else
				{
					at = types[typeName];
				}


				if( typeName == "patroller" )
				{
					Vector2i pos;

					string airStr;
					is >> airStr;

					if( airStr == "+air" )
					{
						is >> pos.x;
						is >> pos.y;
					}
					else
					{
						assert( false && "air wrong" );
					}

					int pathLength;
					is >> pathLength;
					
					list<Vector2i> globalPath;
					globalPath.push_back( Vector2i( pos.x, pos.y ) );

					for( int i = 0; i < pathLength; ++i )
					{
						int localX,localY;
						is >> localX;
						is >> localY;
						globalPath.push_back( Vector2i( pos.x + localX, pos.y + localY ) );
					}


					bool loop;
					string loopStr;
					is >> loopStr;
					if( loopStr == "+loop" )
						loop = true;
					else if( loopStr == "-loop" )
						loop = false;
					else
						assert( false && "should be a boolean" );


					float speed;
					is >> speed;

					a->SetAsPatroller( at, pos, globalPath, speed, loop );	
				}
				else if( typeName == "crawler" )
				{

					//always grounded
					string airStr;
					is >> airStr;

					int terrainIndex;
					is >> terrainIndex;

					int edgeIndex;
					is >> edgeIndex;

					

					double edgeQuantity;
					is >> edgeQuantity;

					bool clockwise;
					string cwStr;
					is >> cwStr;

					if( cwStr == "+clockwise" )
						clockwise = true;
					else if( cwStr == "-clockwise" )
						clockwise = false;
					else
					{
						assert( false && "boolean problem" );
					}

					float speed;
					is >> speed;

					int testIndex = 0;
					TerrainPolygon *terrain = NULL;
					for( list<TerrainPolygon*>::iterator it = polygons.begin(); it != polygons.end(); ++it )
					{
						if( testIndex == terrainIndex )
						{
							terrain = (*it);
							break;
						}
						testIndex++;
					}

					if( terrain == NULL )
						assert( 0 && "failure terrain indexing" );

					if( edgeIndex == terrain->points.size() - 1 )
						edgeIndex = 0;
					else
						edgeIndex++;

					a->SetAsCrawler( at, terrain, edgeIndex, edgeQuantity, clockwise, speed ); 
				}
				else if( typeName == "basicturret" )
				{
					//always grounded
					string airStr;
					is >> airStr;

					int terrainIndex;
					is >> terrainIndex;

					int edgeIndex;
					is >> edgeIndex;

					double edgeQuantity;
					is >> edgeQuantity;

					int framesBetweenFiring;
					is >> framesBetweenFiring;

					int testIndex = 0;
					TerrainPolygon *terrain = NULL;
					for( list<TerrainPolygon*>::iterator it = polygons.begin(); it != polygons.end(); ++it )
					{
						if( testIndex == terrainIndex )
						{
							terrain = (*it);
							break;
						}
						testIndex++;
					}

					if( terrain == NULL )
						assert( 0 && "failure terrain indexing" );

					if( edgeIndex == terrain->points.size() - 1 )
						edgeIndex = 0;
					else
						edgeIndex++;

					a->SetAsBasicTurret( at, terrain, edgeIndex, edgeQuantity, framesBetweenFiring );
				}
				else if( typeName == "foottrap" )
				{
					//always grounded
					string airStr;
					is >> airStr;

					int terrainIndex;
					is >> terrainIndex;

					int edgeIndex;
					is >> edgeIndex;

					double edgeQuantity;
					is >> edgeQuantity;

					int testIndex = 0;
					TerrainPolygon *terrain = NULL;
					for( list<TerrainPolygon*>::iterator it = polygons.begin(); it != polygons.end(); ++it )
					{
						if( testIndex == terrainIndex )
						{
							terrain = (*it);
							break;
						}
						testIndex++;
					}

					if( terrain == NULL )
						assert( 0 && "failure terrain indexing" );

					if( edgeIndex == terrain->points.size() - 1 )
						edgeIndex = 0;
					else
						edgeIndex++;

					a->SetAsFootTrap( at, terrain, edgeIndex, edgeQuantity );
				}

			}
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
	for( list<TerrainPolygon*>::iterator it = polygons.begin(); it != polygons.end(); ++it )
	{
		pointCount += (*it)->points.size();
	}

	of << pointCount << endl;
	of << playerPosition.x << " " << playerPosition.y << endl;
	of << goalPosition.x << " " << goalPosition.y << endl;

	int writeIndex = 0;
	for( list<TerrainPolygon*>::iterator it = polygons.begin(); it != polygons.end(); ++it )
	{
		(*it)->writeIndex = writeIndex;
		++writeIndex;

		of << (*it)->material << " " << (*it)->points.size() << endl;
		for( list<Vector2i>::iterator it2 = (*it)->points.begin(); it2 != (*it)->points.end(); ++it2 )
		{
			of << (*it2).x << " " << (*it2).y << endl;
		}
	}

	of << groups.size() << endl;
	for( map<string, ActorGroup*>::iterator it = groups.begin(); it != groups.end(); ++it )
	{
		(*it).second->WriteFile( of );
		//(*it).second->( w );
	}

	//enemies here


}

void EditSession::Add( TerrainPolygon *brush, TerrainPolygon *poly )
{

	TerrainPolygon z;
	//1: choose start point

	Vector2i startPoint;
	bool startPointFound = false;

	TerrainPolygon *currentPoly = NULL;
	TerrainPolygon *otherPoly = NULL;
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


	//z.points.push_back( startPoint );

	//2. run a loobclockwise until you arrive back at the original state

	sf::Clock alphaClock; //Remove later!!!!


	bool firstTime = true;
	while( firstTime || currPoint != startPoint )
	{
	//	sf::Time time = alphaClock.getElapsedTime();
	//	if( time.asSeconds() > 15 )
	//	{
	//		polygonTimeoutTextTimer = 0;
	//		cout << "successful error" << endl;
	//		return;
	//	}
		//cout << "start loop: " << currPoint.x << ", " << currPoint.y << endl;

		/*CircleShape cs;
		cs.setRadius( 30  );
		cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
		cs.setFillColor( Color::Magenta );
		cs.setPosition( currPoint.x, currPoint.y );
		w->clear();
		this->Draw();
		w->draw( cs );

		cs.setPosition( nextPoint.x, nextPoint.y );
		cs.setFillColor( Color::Yellow );
		w->draw( cs );

		w->display();*/

		
		list<Vector2i>::iterator min;
		Vector2i minIntersection;
		bool emptyInter = true;
		

		list<Vector2i>::iterator bit = otherPoly->points.begin();
		Vector2i bCurrPoint = (*bit);
		++bit;
		Vector2i bNextPoint;// = (*++bit);
		Vector2i minPoint;
		
		LineIntersection li = SegmentIntersect( currPoint, nextPoint, otherPoly->points.back(), bCurrPoint );
		if( !li.parallel && (abs( li.position.x - currPoint.x ) >= 1 || abs( li.position.y - currPoint.y ) >= 1 ))
		{
			minIntersection.x = li.position.x;
			minIntersection.y = li.position.y;
			minPoint = bCurrPoint;
			min = --bit;
			++bit;
			emptyInter = false;
			//cout << "using this" << endl;
		}

		for(; bit != otherPoly->points.end(); ++bit )
		{
			bNextPoint = (*bit);
			LineIntersection li = SegmentIntersect( currPoint, nextPoint, bCurrPoint, bNextPoint );
			Vector2i lii( floor(li.position.x + .5), floor(li.position.y + .5) );
			if( !li.parallel && length( li.position - V2d(currPoint.x, currPoint.y) ) >= 5 )
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
					//cout << "lengths: " << length( li.position - V2d(currPoint.x, currPoint.y) ) << ", " << length( V2d( blah.x, blah.y ) ) << endl;
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

			if( currPoint == startPoint && !firstTime )
			{
			//	cout << "secondary break" << endl;
				break;
			}
			//cout << "switching polygon and adding point" << endl;
			
			//push back intersection
			TerrainPolygon *temp = currentPoly;
			currentPoly = otherPoly;
			otherPoly = temp;
			it = min;
			

			currPoint = minIntersection;

			z.points.push_back( currPoint );

			
			nextPoint = (*it);

			
		/*	if( nextPoint == startPoint )
			{
				if( nextPoint == startPoint && currentPoly == brush )
				{
					assert( 0 && "TT" );
				}
				cout << "break1. next point is: " << nextPoint.x << ", " << nextPoint.y << endl;
				break;
			}*/
			//cout << "fff: " << (*it).x << ", " << (*it).y << endl;
			

			//if( otherPoly == poly )
			//	cout << "switching to brush: " << currPoint.x << ", " << currPoint.y << endl;
			//else
			//	cout << "switching to poly: " << currPoint.x << ", " << currPoint.y << endl;
			//cout << "nextpoint : " << nextPoint.x << ", " << nextPoint.y << endl;
		}
		else
		{

			currPoint = (*it);

			z.points.push_back( currPoint );

		//	cout << "adding point: " << currPoint.x << ", " << currPoint.y << endl;

			if( currPoint == startPoint && !firstTime )
				break;
			

			++it;
			if( it == currentPoly->points.end() )
			{
				it = currentPoly->points.begin();
			}
			nextPoint = (*it);
	//		cout << "nextpoing from adding: " << nextPoint.x << ", " << nextPoint.y << endl;
		}
		firstTime = false;
	}

	poly->Reset();
	for( list<Vector2i>::iterator zit = z.points.begin(); zit != z.points.end(); ++zit )
	{
		poly->points.push_back( (*zit) );
	}
	//cout << "before killer finalize. poly size: " << poly->points.size() << endl;
	poly->Finalize();
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
	//RenderTexture rtt;
	//rtt.create( 0, 0 );
	//rtt.clear(Color::Red);
	
	//rtt.
	//rtt.create( 400, 400 );
	//rtt.clear();

	trackingEnemy = NULL;
	showPanel = NULL;

	sf::Font arial;
	arial.loadFromFile( "arial.ttf" );

//	Panel p( 300, 300, this );
//	p.active = true;
//	p.AddButton( Vector2i( 50, 100 ), Vector2f( 50, 50 ), "LOL");
//	p.AddTextBox( Vector2i( 200, 200 ), 200, 15, "testing" );

	ActorGroup *emptyGroup = new ActorGroup( "--" );
	//emptyGroup->name = "";
	groups[emptyGroup->name] = emptyGroup;


	Panel *patrollerPanel = CreateOptionsPanel( "patroller" );//new Panel( 300, 300, this );
	ActorType *patrollerType = new ActorType( "patroller", patrollerPanel );

	Panel *crawlerPanel = CreateOptionsPanel( "crawler" );
	ActorType *crawlerType = new ActorType( "crawler", crawlerPanel );

	Panel *basicTurretPanel = CreateOptionsPanel( "basicturret" );
	ActorType *basicTurretType = new ActorType( "basicturret", crawlerPanel );

	Panel *footTrapPanel = CreateOptionsPanel( "foottrap" );
	ActorType *footTrapType = new ActorType( "foottrap", footTrapPanel );

	types["patroller"] = patrollerType;
	types["crawler"] = crawlerType;
	types["basicturret"] = basicTurretType;
	types["foottrap"] = footTrapType;


	GridSelector gs( 2, 2, 32, 32, this );
	gs.active = false;

	sf::Sprite s0( patrollerType->iconTexture );
	sf::Sprite s1( crawlerType->iconTexture );
	sf::Sprite s2( basicTurretType->iconTexture );
	sf::Sprite s3( footTrapType->iconTexture );

	gs.Set( 0, 0, s0, "patroller" );
	gs.Set( 1, 0, s1, "crawler" );
	gs.Set( 0, 1, s2, "basicturret" );
	gs.Set( 1, 1, s3, "foottrap" );

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

	Texture goalTex;
	goalTex.loadFromFile( "goal.png" );
	Sprite goalSprite( goalTex );

	sf::Texture iconsTex;
	iconsTex.loadFromFile( "editoricons.png" );
	Sprite iconSprite( iconsTex );

	sf::Texture alphaTex;
	alphaTex.loadFromFile( "alphatext.png" );
	sf::Sprite alphaTextSprite( alphaTex );
	alphaTextSprite.setOrigin( alphaTextSprite.getLocalBounds().width / 2, alphaTextSprite.getLocalBounds().height / 2 );

	
	sf::Vector2u wSize = w->getSize();
	sf::View uiView( sf::Vector2f( 480, 270 ), sf::Vector2f( 960, 540 ) );
	//sf::View uiView( Vector2f( wSize.x / 2, wSize.y / 2 ), Vector2f( wSize.x, wSize.y ) );

	goalSprite.setOrigin( goalSprite.getLocalBounds().width / 2, goalSprite.getLocalBounds().height / 2 );

	playerSprite.setTextureRect( IntRect(0, 0, 64, 64 ) );
	playerSprite.setOrigin( playerSprite.getLocalBounds().width / 2, playerSprite.getLocalBounds().height / 2 );

	w->setVerticalSyncEnabled( false );

	OpenFile( fileName );


//	ActorParams *ap = new ActorParams;
//	ap->CreatePatroller( patrollerType, Vector2i( playerPosition.x, playerPosition.y ), true, 10 );
//	groups["--"]->actors.push_back( ap );
	//ap->CreatePatroller( 



	//Vector2f vs(  );
	if( cameraSize.x == 0 && cameraSize.y == 0 )
		view.setCenter( (float)playerPosition.x, (float)playerPosition.y );

	mode = "neutral";
	bool quit = false;
	polygonInProgress = new TerrainPolygon();
	zoomMultiple = 1;
	Vector2<double> prevWorldPos;
	Vector2i pixelPos;
	Vector2f tempWorldPos = w->mapPixelToCoords(sf::Mouse::getPosition( *w ));
	Vector2<double> worldPos = Vector2<double>( tempWorldPos.x, tempWorldPos.y );
	bool panning = false;
	Vector2<double> panAnchor;
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

	sf::Texture guiMenuCubeTexture;
	guiMenuCubeTexture.loadFromFile( "guioptions.png" );
	sf::Sprite guiMenuSprite;
	guiMenuSprite.setTexture( guiMenuCubeTexture );
	guiMenuSprite.setOrigin( guiMenuSprite.getLocalBounds().width / 2, guiMenuSprite.getLocalBounds().height / 2 );

	

	bool s = sf::Keyboard::isKeyPressed( sf::Keyboard::T );
	
	V2d menuDownPos;
	Emode menuDownStored;

	Emode mode = CREATE_TERRAIN;
	Emode stored = mode;
	bool canCreatePoint = true;
	gs.active = true;

	int enemyEdgeIndex;
	TerrainPolygon *enemyEdgePolygon;
	double enemyEdgeQuantity;

	while( !quit )
	{
		w->clear();
		prevWorldPos = worldPos;
		pixelPos = sf::Mouse::getPosition( *w );
		Vector2f tempWorldPos = w->mapPixelToCoords(pixelPos);
		worldPos.x = tempWorldPos.x;
		worldPos.y = tempWorldPos.y;

		w->setView( uiView );
		Vector2f uiMouse = w->mapPixelToCoords( pixelPos );
		w->setView( view );
		sf::Event ev;

		testPoint.x = worldPos.x;
		testPoint.y = worldPos.y;
		
		while( w->pollEvent( ev ) )
		{
			switch( mode )
			{
			case CREATE_TERRAIN:
				{
					switch( ev.type )
					{
					case Event::MouseButtonPressed:
						{
							if( ev.mouseButton.button == Mouse::Left )
							{
							}
							
							break;
						}
					case Event::MouseButtonReleased:
						{
							
							break;
						}
					case Event::MouseWheelMoved:
						{
							
						}
					case Event::KeyPressed:
						{
							if( ev.key.code == Keyboard::Space )
							{
								if( polygonInProgress->points.size() > 2 )
								{
									//test final line

									if( !PointValid( polygonInProgress->points.back(), polygonInProgress->points.front() ) )
										break;

									list<TerrainPolygon*>::iterator it = polygons.begin();
									bool added = false;
									polygonInProgress->Finalize();
									bool recursionDone = false;
									TerrainPolygon *currentBrush = polygonInProgress;

										while( it != polygons.end() )
										{
											TerrainPolygon *temp = (*it);
											if( temp != currentBrush && currentBrush->IsTouching( temp ) )
											{
												cout << "before addi: " << (*it)->points.size() << endl;
						
												Add( currentBrush, temp );

												polygonInProgress->Reset();
						
												cout << "after adding: " << (*it)->points.size() << endl;
												polygons.erase( it );

												currentBrush = temp;

												it = polygons.begin();

												added = true;
							
												continue;
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
										polygonInProgress = new TerrainPolygon();
									}
									else
									{

										polygons.push_back( currentBrush );
										polygonInProgress->Reset();
									}
								}

								if( polygonInProgress->points.size() <= 2 && polygonInProgress->points.size() > 0  )
								{
									cout << "cant finalize. cant make polygon" << endl;
									polygonInProgress->points.clear();
								}
							}
							else if( ev.key.code == sf::Keyboard::V )
							{
								if( polygonInProgress->points.size() > 0 )
								{
									polygonInProgress->points.pop_back();
								}
								/*else if( mode == SELECT_POLYGONS )
								{
									list<TerrainPolygon*>::iterator it = polygons.begin();
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
								}*/
							}
							
							break;
						}
					case Event::KeyReleased:
						{
							break;
						}
					case Event::LostFocus:
						{
							break;
						}
					case Event::GainedFocus:
						{
							break;
						}
					}
					break;	
				}
			case EDIT:
				{
					switch( ev.type )
					{
					case Event::MouseButtonPressed:
						{
							/*bool emptySpace = true;
							if( polygonInProgress->points.size() == 0 )
							{
								for( list<TerrainPolygon*>::iterator it = polygons.begin(); it != polygons.end(); ++it )
								{
						
										if((*it)->ContainsPoint( Vector2f(worldPos.x, worldPos.y ) ) )
										{
											emptySpace = false;
											(*it)->SetSelected( !((*it)->selected ) );
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
							}*/
							break;
						}
					case Event::MouseButtonReleased:
						{
							break;
						}
					case Event::MouseWheelMoved:
						{
							break;
						}
					case Event::KeyPressed:
						{
							break;
						}
					case Event::KeyReleased:
						{
							break;
						}
					case Event::LostFocus:
						{
							break;
						}
					case Event::GainedFocus:
						{
							break;
						}
					}
					break;
				}
			case CREATE_ENEMY:
				{
					switch( ev.type )
					{
					case Event::MouseButtonPressed:
						{
							if( ev.mouseButton.button == Mouse::Left )
							{
								if( showPanel != NULL )
								{	
									showPanel->Update( true, uiMouse.x, uiMouse.y );
								}
								else if( gs.active )
								{
									gs.Update( true, uiMouse.x, uiMouse.y );
								}
							}
							break;
						}
					case Event::MouseButtonReleased:
						{
							if( ev.mouseButton.button == Mouse::Left )
							{
								if( trackingEnemy != NULL )
								{
									if( trackingEnemy->name == "patroller" )
									{
										mode = CREATE_PATROL_PATH;
										patrolPath.clear();
										patrolPath.push_back( Vector2i( worldPos.x, worldPos.y ) );
									}
									else if( trackingEnemy->name == "crawler" )
									{
										//groups["--"]->name
										if( enemyEdgePolygon != NULL )
										{
											showPanel = trackingEnemy->panel;
											trackingEnemy = NULL;
											ActorParams *actor = new ActorParams;
											cout << "blah" << endl;
											actor->SetAsCrawler( crawlerType, enemyEdgePolygon, enemyEdgeIndex, 
												enemyEdgeQuantity, true, 10 );
											
											groups["--"]->actors.push_back( actor);
										}
									}
									else if( trackingEnemy->name == "basicturret" )
									{
										if( enemyEdgePolygon != NULL )
										{
											showPanel = trackingEnemy->panel;
											trackingEnemy = NULL;
											ActorParams *actor = new ActorParams;
											actor->SetAsBasicTurret( basicTurretType, enemyEdgePolygon, enemyEdgeIndex, 
												enemyEdgeQuantity, 30 );
											groups["--"]->actors.push_back( actor);
										}
									}
									else if( trackingEnemy->name == "foottrap" )
									{
										if( enemyEdgePolygon != NULL )
										{
											showPanel = trackingEnemy->panel;
											trackingEnemy = NULL;
											ActorParams *actor = new ActorParams;
											actor->SetAsFootTrap( footTrapType, enemyEdgePolygon, enemyEdgeIndex, 
												enemyEdgeQuantity );
											groups["--"]->actors.push_back( actor);
										}
									}
								}

								if( showPanel != NULL )
								{	
									showPanel->Update( false, uiMouse.x, uiMouse.y );
								}
								else if( gs.active )
								{
									if( gs.Update( false, uiMouse.x, uiMouse.y ) )
									{
										cout << "selected enemy index: " << gs.focusX << ", " << gs.focusY << endl;
									}
								}

								
							}
							break;
						}
					case Event::MouseWheelMoved:
						{
							
						}
					}
					break;
				}
			case PAUSED:
				{
					switch( ev.type )
					{
						case Event::MouseButtonPressed:
							{
								break;
							}
						case Event::MouseButtonReleased:
							{

								break;
							}
						case Event::GainedFocus:
						{
							mode = stored;
							break;
						}
					}
					break;
				}
			case SELECT_MODE:
				{
					switch( ev.type )
					{
					case Event::MouseButtonPressed:
						{
							break;
						}
					case Event::MouseButtonReleased:
						{
							V2d releasePos(uiMouse.x, uiMouse.y);
							if( length( releasePos - menuDownPos ) > 100 )
							{
								//mode = EDIT;
								mode = CREATE_ENEMY;
								//cout << "blah: " << length( releasePos - menuDownPos ) << endl;
							}
							else
								mode = menuDownStored;

							break;
						}
					case Event::MouseWheelMoved:
						{
							break;
						}
					case Event::KeyPressed:
						{
							break;
						}
					case Event::KeyReleased:
						{
							break;
						}
					case Event::LostFocus:
						{
							break;
						}
					case Event::GainedFocus:
						{
							break;
						}
					}
					break;
				}
			case CREATE_PATROL_PATH:
				{
					minimumPathEdgeLength = 16;
					switch( ev.type )
					{
					case Event::MouseButtonPressed:
						{

							break;
						}
					case Event::MouseButtonReleased:
						{
							break;
						}
					case Event::MouseWheelMoved:
						{
							break;
						}
					case Event::KeyPressed:
						{
							if( ev.key.code == Keyboard::V && patrolPath.size() > 1 )
							{
								patrolPath.pop_back();
							}
							else if( ev.key.code == Keyboard::Space )
							{
								showPanel = trackingEnemy->panel;
								trackingEnemy = NULL;
								ActorParams *actor = new ActorParams;
								actor->SetAsPatroller( patrollerType, patrolPath.front(), patrolPath, 10, false );
								groups["--"]->actors.push_back( actor);
								patrolPath.clear();
								mode = CREATE_ENEMY;
							}
							break;
						}
					case Event::KeyReleased:
						{
							break;
						}
					case Event::LostFocus:
						{
							break;
						}
					case Event::GainedFocus:
						{
							break;
						}
					}
					break;
				}
			}

			//ones that aren't specific to mode
			
			if( mode != PAUSED && mode != SELECT_MODE )
			{
				switch( ev.type )
				{
				case Event::MouseButtonPressed:
					{
						if( ev.mouseButton.button == Mouse::Button::Middle )
						{
							panning = true;
							panAnchor = worldPos;
						}
						else if( ev.mouseButton.button == Mouse::Button::Right )
						{
							menuDownStored = mode;
							mode = SELECT_MODE;
							menuDownPos = V2d( uiMouse.x, uiMouse.y );
							guiMenuSprite.setPosition( uiMouse.x, uiMouse.y );//pixelPos.x, pixelPos.y );//uiMouse.x, uiMouse.y );
						}
						break;
					}
				case Event::MouseButtonReleased:
					{
						if( ev.mouseButton.button == Mouse::Button::Middle )
						{
							panning = false;
						}
						break;
					}
				case Event::MouseWheelMoved:
					{
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
						break;
					}
				case Event::KeyPressed:
					{
						if( ev.key.code == Keyboard::S && ev.key.control )
						{
							polygonInProgress->points.clear();
							cout << "writing to file: " << currentFile << ".brknk" << endl;
							WriteFile(currentFile);
						}
						else if( ev.key.code == Keyboard::T )
						{
							quit = true;
						}
						else if( ev.key.code == Keyboard::Escape )
						{
							if( sf::Keyboard::isKeyPressed( sf::Keyboard::Escape ) )
							{
								quit = true;
								returnVal = 1;
							}
						}
						else if( ev.key.code == sf::Keyboard::Z )
						{
							panning = true;
							panAnchor = worldPos;	
						}
						else if( ev.key.code == sf::Keyboard::Equal || ev.key.code == sf::Keyboard::Dash )
						{
							if( ev.key.code == sf::Keyboard::Equal )
							{
								zoomMultiple /= 2;
							}
							else if( ev.key.code == sf::Keyboard::Dash )
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

							break;
						}
						break;
					}
				case Event::KeyReleased:
					{
						if( ev.key.code == sf::Keyboard::Z )
						{
							panning = false;
						}
						break;
					}
				case Event::LostFocus:
					{
						stored = mode;
						mode = PAUSED;
						break;
					}
				case Event::GainedFocus:
					{
						mode = stored;
						break;
					}
				}
			}
			
			/*switch( ev.type )
			{
			case Event::MouseButtonPressed:
				{
					if( ev.mouseButton.button == Mouse::Left )
					{
						if( showPanel != NULL )
						{	
							showPanel->Update( true, uiMouse.x, uiMouse.y );
							
						//	p.Update( true, pixelPos.x, pixelPos.y );
						}
						else if( mode == CREATE_ENEMY)
						{
							if( gs.active )
							{

								gs.Update( true, uiMouse.x, uiMouse.y );
							}
							canCreatePoint = false;
						}
						else if( mode == PLACE_PLAYER )
						{
							playerPosition.x = (int)worldPos.x;
							playerPosition.y = (int)worldPos.y;
							mode = CREATE_POLYGONS;
							canCreatePoint = false;
						}
						else if( mode == PLACE_GOAL )
						{
							goalPosition.x = (int)worldPos.x;
							goalPosition.y = (int)worldPos.y;
							mode = CREATE_POLYGONS;
							canCreatePoint = false;
						}
					}
					
					if( ev.mouseButton.button == Mouse::Button::Middle )
					{
						panning = true;
						panAnchor = worldPos;
					}

					break;
				}
			case Event::MouseButtonReleased:
				{
					if( !canCreatePoint )
						canCreatePoint = true;

					if( ev.mouseButton.button == Mouse::Button::Middle )
					{
						panning = false;
					}
					else if( ev.mouseButton.button == Mouse::Button::Left )
					{
						if( showPanel != NULL )
						{
							showPanel->Update( false, uiMouse.x, uiMouse.y );
						//	p.Update( false, pixelPos.x, pixelPos.y );
						}
						else if( mode == CREATE_ENEMY  )
						{

							if( trackingEnemy != NULL )
							{
								showPanel = trackingEnemy->panel;
								trackingEnemy = NULL;
								ActorParams *actor = new ActorParams;
								actor->SetAsPatroller( patrollerType, Vector2i( worldPos.x, worldPos.y ), true, 10 );
								groups["--"]->actors.push_back( actor);
							}

							if( gs.active )
							{
								if( gs.Update( false, uiMouse.x, uiMouse.y ) )
								{
									cout << "selected enemy index: " << gs.focusX << ", " << gs.focusY << endl;
								}
							}

							
						}
						else if( mode == SELECT_POLYGONS )
						{
							bool emptySpace = true;
							if( polygonInProgress->points.size() == 0 )
							{
								for( list<TerrainPolygon*>::iterator it = polygons.begin(); it != polygons.end(); ++it )
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
						for( list<TerrainPolygon*>::iterator it = polygons.begin(); it != polygons.end(); ++it )
						{
							(*it)->SetSelected( false );					
						}
					}
					break;
				}
			case Event::MouseWheelMoved:
				{
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

					break;
				}
			case Event::KeyPressed:
				{
					if( showPanel != NULL )
					{
						showPanel->SendKey( ev.key.code, ev.key.shift );
					}
					else if( mode != PAUSED && mode != PLACE_GOAL && mode != PLACE_PLAYER )
					{
						Emode oldMode = mode;



						if( ev.key.code == Keyboard::S && ev.key.control )
						{
							polygonInProgress->points.clear();
							cout << "writing to file: " << currentFile << ".brknk" << endl;
							WriteFile(currentFile);
						}
						if( ev.key.code == Keyboard::B )
						{
							mode = PLACE_PLAYER;

						}
						else if( ev.key.code == Keyboard::G )
						{
							mode = PLACE_GOAL;
						}
						else if( ev.key.code == Keyboard::Q )
						{
							mode = SELECT_POLYGONS;
						}
						else if( ev.key.code == Keyboard::C )
						{
							mode = CREATE_POLYGONS;
						}
						else if( ev.key.code == Keyboard::F )
						{
							mode = CREATE_ENEMY;
							gs.active = true;
							trackingEnemy = NULL;
						}
						else if( ev.key.code == Keyboard::Space )
						{

							if( mode == CREATE_POLYGONS && polygonInProgress->points.size() > 2 )
							{
								//test final line

								if( !PointValid( polygonInProgress->points.back(), polygonInProgress->points.front() ) )
									break;

								list<TerrainPolygon*>::iterator it = polygons.begin();
								bool added = false;
								polygonInProgress->Finalize();
								bool recursionDone = false;
								TerrainPolygon *currentBrush = polygonInProgress;

									while( it != polygons.end() )
									{
										TerrainPolygon *temp = (*it);
										if( temp != currentBrush && currentBrush->IsTouching( temp ) )
										{
											cout << "before addi: " << (*it)->points.size() << endl;
						
											Add( currentBrush, temp );

											polygonInProgress->Reset();
						
											cout << "after adding: " << (*it)->points.size() << endl;
											polygons.erase( it );

											currentBrush = temp;

											it = polygons.begin();

											added = true;
							
											continue;
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
									polygonInProgress = new TerrainPolygon();
								}
								else
								{

									polygons.push_back( currentBrush );
									polygonInProgress->Reset();
								}
							}

							if( polygonInProgress->points.size() <= 2 && polygonInProgress->points.size() > 0  )
							{
								cout << "cant finalize. cant make polygon" << endl;
								polygonInProgress->points.clear();
							}
						}
						else if( ev.key.code == Keyboard::T )
						{
							quit = true;
						}
						else if( ev.key.code == Keyboard::Escape )
						{
							if( sf::Keyboard::isKeyPressed( sf::Keyboard::Escape ) )
							{
								quit = true;
								returnVal = 1;
							}
						}
						else if( ev.key.code == sf::Keyboard::V )
						{
							if( mode == CREATE_POLYGONS && polygonInProgress->points.size() > 0 )
							{
								polygonInProgress->points.pop_back();
							}
							else if( mode == SELECT_POLYGONS )
							{
								list<TerrainPolygon*>::iterator it = polygons.begin();
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
						}
						else if( ev.key.code == sf::Keyboard::Z )
						{
							panning = true;
							panAnchor = worldPos;	
						}
						else if( ev.key.code == sf::Keyboard::Equal || ev.key.code == sf::Keyboard::Dash )
						{
							if( ev.key.code == sf::Keyboard::Equal )
							{
								zoomMultiple /= 2;
							}
							else if( ev.key.code == sf::Keyboard::Dash )
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

							break;
						}
						

						if( oldMode == CREATE_POLYGONS && mode != CREATE_POLYGONS )
						{
							polygonInProgress->points.clear();
						}
						else if( oldMode == SELECT_POLYGONS && mode != SELECT_POLYGONS )
						{
							for( list<TerrainPolygon*>::iterator it = polygons.begin(); it != polygons.end(); ++it )
							{
								(*it)->SetSelected( false );					
							}
						}
						
					}
					break;
				}
			case Event::KeyReleased:
				{
					if( ev.key.code == sf::Keyboard::Z )
					{
						panning = false;
					}
					break;
				}
			case Event::LostFocus:
				{
					stored = mode;
					mode = PAUSED;
					break;
				}
			case Event::GainedFocus:
				{
					mode = stored;
				}
			}*/
		}

		if( quit )
			break;

		

		switch( mode )
		{
		case CREATE_TERRAIN:
			{
				if( polygonInProgress->points.size() > 0 && Keyboard::isKeyPressed( Keyboard::LShift ) ) 
				{

					Vector2i last = polygonInProgress->points.back();
					Vector2f diff = testPoint - Vector2f(last.x, last.y);

					double len;
					double angle = atan2( -diff.y, diff.x );
					if( angle < 0 )
						angle += 2 * PI;
					Vector2f dir;
			
					//cout << "angle : " << angle << endl;
					if( angle + PI / 8 >= 2 * PI || angle < PI / 8 )
					{
						len = dot( V2d( diff.x, diff.y ), V2d( 1, 0 ) );
						dir = Vector2f( 1, 0 );
					}
					else if( angle < 3 * PI / 8 )
					{
						len = dot( V2d( diff.x, diff.y ), normalize( V2d( 1, -1 ) ) );
						V2d tt = normalize( V2d( 1, -1 ) );
						dir = Vector2f( tt.x, tt.y );
					}
					else if( angle < 5 * PI / 8 )
					{
						len = dot( V2d( diff.x, diff.y ), V2d( 0, -1 ) );
						dir = Vector2f( 0, -1 );
					}
					else if( angle < 7 * PI / 8 )
					{
						len = dot( V2d( diff.x, diff.y ), normalize(V2d( -1, -1 )) );
						V2d tt = normalize( V2d( -1, -1 ) );
						dir = Vector2f( tt.x, tt.y );
					}
					else if( angle < 9 * PI / 8 )
					{
						len = dot( V2d( diff.x, diff.y ), V2d( -1, 0 ) );
						dir = Vector2f( -1, 0 );
					}
					else if( angle < 11 * PI / 8 )
					{
						len = dot( V2d( diff.x, diff.y ), normalize(V2d( -1, 1 )) );
						V2d tt = normalize( V2d( -1, 1 ) );
						dir = Vector2f( tt.x, tt.y );
					}
					else if( angle < 13 * PI / 8 )
					{
						len = dot( V2d( diff.x, diff.y ), V2d( 0, 1 ) );
						dir = Vector2f( 0, 1 );
					}
					else //( angle < 15 * PI / 8 )
					{
						len = dot( V2d( diff.x, diff.y ), normalize(V2d( 1, 1 )) );
						V2d tt = normalize( V2d( 1, 1 ) );
						dir = Vector2f( tt.x, tt.y );
					}

					testPoint = Vector2f(last.x, last.y) + dir * (float)len;
					//angle = asin( dot( ground->Normal(), V2d( 1, 0 ) ) ); 
				}

				if( !panning && Mouse::isButtonPressed( Mouse::Left ) )
				{
					bool emptySpace = true;
					for( list<TerrainPolygon*>::iterator it = polygons.begin(); it != polygons.end(); ++it )
					{
						if((*it)->ContainsPoint( testPoint ) )
						{
							//emptySpace = false;
						
							break;
						}
					}

					if( emptySpace )
					{
						if( length( worldPos - Vector2<double>(polygonInProgress->points.back().x, 
							polygonInProgress->points.back().y )  ) >= minimumEdgeLength * std::max(zoomMultiple,1.0 ) )
						{
							Vector2i worldi( testPoint.x, testPoint.y );
						
						
							if( polygonInProgress->points.size() > 0 )
							{
								if( PointValid( polygonInProgress->points.back(), worldi ) )
								{
									polygonInProgress->points.push_back( worldi );
								}
							}
							else
								polygonInProgress->points.push_back( worldi );
						}
					}
					else
					{
						//polygonInProgress->points.clear();
					}
					
				}
				break;
			}
		case EDIT:
			{
				break;
			}
		case CREATE_ENEMY:
			{
				if( trackingEnemy != NULL )
				{
					enemySprite.setOrigin( enemySprite.getLocalBounds().width / 2, enemySprite.getLocalBounds().height / 2 );
					enemySprite.setRotation( 0 );
					enemySprite.setPosition( w->mapPixelToCoords( sf::Mouse::getPosition( *w ) ) );
					
				}

				if( trackingEnemy != NULL && ( trackingEnemy->name == "crawler" 
					|| trackingEnemy->name == "basicturret"
					|| trackingEnemy->name == "foottrap" ) )
				{
					enemyEdgePolygon = NULL;
				
					double testRadius = 200;
					
					for( list<TerrainPolygon*>::iterator it = polygons.begin(); it != polygons.end(); ++it )
					{
						if( testPoint.x >= (*it)->left - testRadius && testPoint.x <= (*it)->right + testRadius
							&& testPoint.y >= (*it)->top - testRadius && testPoint.y <= (*it)->bottom + testRadius )
						{
							list<Vector2i>::iterator prevIt = (*it)->points.end();
							prevIt--;
							list<Vector2i>::iterator currIt = (*it)->points.begin();

							if( (*it)->ContainsPoint( Vector2f( testPoint.x, testPoint.y ) ) )
							{
								//prev is starting at 0. start normally at 1
								int edgeIndex = 0;
								double minDistance = 10000000;
								int storedIndex;
								double storedQuantity;
							
								V2d closestPoint;

								for( ; currIt != (*it)->points.end(); ++currIt )
								{
									double dist = abs(
										cross( 
										V2d( testPoint.x - (*prevIt).x, testPoint.y - (*prevIt).y ), 
										normalize( V2d( (*currIt).x - (*prevIt).x, (*currIt).y - (*prevIt).y ) ) ) );
									double testQuantity =  dot( 
											V2d( testPoint.x - (*prevIt).x, testPoint.y - (*prevIt).y ), 
											normalize( V2d( (*currIt).x - (*prevIt).x, (*currIt).y - (*prevIt).y ) ) );

									V2d pr( (*prevIt).x, (*prevIt).y );
									V2d cu( (*currIt).x, (*currIt).y );
									V2d te( testPoint.x, testPoint.y );
									
									V2d newPoint( pr.x + (cu.x - pr.x) * (testQuantity / length( cu - pr ) ), pr.y + (cu.y - pr.y ) *
											(testQuantity / length( cu - pr ) ) );

									//int testA = dist < 100;
									//int testB = testQuantity >= 0 && testQuantity <= length( cu - pr );
									//int testC = testQuantity >= enemySprite.getLocalBounds().width / 2 && testQuantity <= length( cu - pr ) - enemySprite.getLocalBounds().width / 2;
									//int testD = length( newPoint - te ) < length( closestPoint - te );
									
									//cout << testA << " " << testB << " " << testC << " " << testD << endl;

									if( dist < 100 && testQuantity >= 0 && testQuantity <= length( cu - pr ) && testQuantity >= enemySprite.getLocalBounds().width / 2 && testQuantity <= length( cu - pr ) - enemySprite.getLocalBounds().width / 2 
										&& length( newPoint - te ) < length( closestPoint - te ) )
									{
										minDistance = dist;
										storedIndex = edgeIndex;
										double l = length( cu - pr );
										
										storedQuantity = testQuantity;
										closestPoint = newPoint ;
										//minDistance = length( closestPoint - te )  
										
										enemySprite.setOrigin( enemySprite.getLocalBounds().width / 2, enemySprite.getLocalBounds().height );
										enemySprite.setPosition( closestPoint.x, closestPoint.y );
										enemySprite.setRotation( atan2( (cu - pr).y, (cu - pr).x ) / PI * 180 );
									}
									else
									{
										
										//cout << "dist: " << dist << ", testquant: " << testQuantity  << endl;
									}

									prevIt = currIt;
									++edgeIndex;
								}

								enemyEdgeIndex = storedIndex;

								enemyEdgeQuantity = storedQuantity;
								
								enemyEdgePolygon = (*it);
								

								//cout << "pos: " << closestPoint.x << ", " << closestPoint.y << endl;
								//cout << "minDist: " << minDistance << endl;

								break;
							}
						}
					}


				}

				
				break;
			}
		case PAUSED:
			{
				break;
			}
		case CREATE_PATROL_PATH:
			{
				if( !panning && Mouse::isButtonPressed( Mouse::Left ) )
				{
					if( length( worldPos - Vector2<double>(patrolPath.back().x, 
						patrolPath.back().y )  ) >= minimumPathEdgeLength * std::max(zoomMultiple,1.0 ) )
					{
						Vector2i worldi( testPoint.x, testPoint.y );

						patrolPath.push_back( worldi );
					}					
				}
				break;
			}
		}


		

		if( panning )
		{
			Vector2<double> temp = panAnchor - worldPos;
			view.move( Vector2f( temp.x, temp.y ) );
		}
		
		


	/*	if( mode == PLACE_PLAYER )
		{
			playerSprite.setPosition( w->mapPixelToCoords(sf::Mouse::getPosition( *w )) );
			//cout << "placing: " << playerSprite.getPosition().x << ", " << playerSprite.getPosition().y << endl;
		}
		else
			playerSprite.setPosition( playerPosition.x, playerPosition.y );*/




	/*	if( mode == PLACE_GOAL )
		{
			goalSprite.setPosition( w->mapPixelToCoords( sf::Mouse::getPosition( *w )) );
		}
		else
			goalSprite.setPosition( goalPosition.x, goalPosition.y );*/
		
		

		w->setView( view );
		w->draw(border, 8, sf::Lines);

		Draw();

		for( map<string, ActorGroup*>::iterator it = groups.begin(); it != groups.end(); ++it )
		{
			(*it).second->Draw( w );
		}

		switch( mode )
		{
		case CREATE_TERRAIN:
			{
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
						sf::Vertex(sf::Vector2<float>(testPoint.x, testPoint.y), colorSelection)
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
					}
				}
				break;
			}
		case CREATE_ENEMY:
			{
				if( trackingEnemy != NULL )
				{
					w->draw( enemySprite );
				}
				break;
			}
		case CREATE_PATROL_PATH:
			{
				if( trackingEnemy != NULL )
				{
					w->draw( enemySprite );
				}
				int pathSize = patrolPath.size();
				if( pathSize > 0 )
				{
					Vector2i backPoint = patrolPath.back();
			
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
						sf::Vertex(sf::Vector2<float>(testPoint.x, testPoint.y), colorSelection)
					};
					w->draw( activePreview, 2, sf::Lines );

					if( pathSize > 1 )
					{
						VertexArray v( sf::LinesStrip, pathSize );
						int i = 0;
						for( list<sf::Vector2i>::iterator it = patrolPath.begin(); 
							it != patrolPath.end(); ++it )
						{
							v[i] = Vertex( Vector2f( (*it).x, (*it).y ) );
							++i;
						}
						w->draw( v );
					}
				}
				
				if( pathSize > 0 ) //always
				{
					CircleShape cs;
					cs.setRadius( 5 * zoomMultiple  );
					cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
					cs.setFillColor( Color::Green );

		
					for( list<Vector2i>::iterator it = patrolPath.begin(); it != patrolPath.end(); ++it )
					{
						cs.setPosition( (*it).x, (*it).y );
						w->draw( cs );
					}		
				}
				break;
			}
		}
		

		iconSprite.setScale( view.getSize().x / 960.0, view.getSize().y / 540.0 );
		iconSprite.setPosition( view.getCenter().x + 200 * iconSprite.getScale().x, view.getCenter().y - 250 * iconSprite.getScale().y );
		

		w->draw( playerSprite );
		w->draw( goalSprite );
		w->draw( iconSprite );

		if( showPanel == NULL && sf::Keyboard::isKeyPressed( Keyboard::H ) )
		{
			alphaTextSprite.setScale( .5 * view.getSize().x / 960.0, .5 * view.getSize().y / 540.0 );
			alphaTextSprite.setOrigin( alphaTextSprite.getLocalBounds().width / 2, alphaTextSprite.getLocalBounds().height / 2 );
			alphaTextSprite.setPosition( view.getCenter().x, view.getCenter().y );
			w->draw( alphaTextSprite );
		}
		
		w->setView( uiView );

		switch( mode )
		{
			case CREATE_TERRAIN:
				{
					break;
				}
			case CREATE_ENEMY:
				{
					gs.Draw( w );
					if( showPanel != NULL )
					{
						showPanel->Draw( w );
					}
					break;
				}
			case SELECT_MODE:
				{
					w->draw( guiMenuSprite );
					break;
				}
		}

		w->setView( view );


		w->display();
	}
	
	return returnVal;
	
}

bool EditSession::PointValid( Vector2i prev, Vector2i point)
{
	//return true;
	float eLeft = min( prev.x, point.x );
	float eRight= max( prev.x, point.x );
	float eTop = min( prev.y, point.y );
	float eBottom = max( prev.y, point.y );

	{
		list<Vector2i>::iterator it = polygonInProgress->points.begin();
		//polygonInProgress->points.push_back( polygonInProgress->points.back() )
		Vector2i pre = (*it);
		++it;
		
		//minimum angle
		{
			if( polygonInProgress->points.size() >= 2 )
			{
				list<Vector2i>::reverse_iterator rit = polygonInProgress->points.rbegin();
				rit++;
				double ff = dot( normalize( V2d( point.x, point.y ) - V2d( polygonInProgress->points.back().x, polygonInProgress->points.back().y ) )
					, normalize( V2d((*rit).x, (*rit).y ) - V2d( polygonInProgress->points.back().x, polygonInProgress->points.back().y ) ) );
				if( ff > minAngle )
				{
					//cout << "ff: " << ff << endl;
					return false;
				}
			}
		}

		//return true;

		//make sure I'm not too close to the very first point and that my line isn't too close to the first point either
		if( point.x != polygonInProgress->points.front().x || point.y != polygonInProgress->points.front().y )
		{
			double separation = length( V2d(point.x, point.y) - V2d(pre.x, pre.y) );
			if( separation < minimumEdgeLength )
			{
				return false;
			}

			if( polygonInProgress->points.size() > 2  )
			{
				if( abs( cross( V2d( point.x, point.y ) - V2d( prev.x, prev.y), 
					normalize( V2d( pre.x, pre.y ) - V2d( prev.x, prev.y ) ) ) ) < minimumEdgeLength
					&& dot( V2d( point.x, point.y ) - V2d( prev.x, prev.y ), normalize( V2d( pre.x, pre.y ) - V2d( prev.x, prev.y )) ) 
					>= length( V2d( pre.x, pre.y ) - V2d( prev.x, prev.y ) ) )
				{
					return false;
				}
			}
		}

		//check for distance to point in the polygon and edge distances

		if( point.x == polygonInProgress->points.front().x && point.y == polygonInProgress->points.front().y )
		{
			pre = (*it);
			++it;
		}

		{
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

				//cout << "minedgelength: " << minimumEdgeLength <<  ", " << res << endl;

				if( point.x == polygonInProgress->points.front().x && point.y == polygonInProgress->points.front().y )
				{
				}
				else

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
	}
	return true;

	int i = 0;
	for( list<TerrainPolygon*>::iterator it = polygons.begin(); it != polygons.end(); ++it )
	{
		//cout << "polygon " << i << " out of " << polygons.size() << " ... " << (*it)->points.size()  << endl;
		++i;
		TerrainPolygon *p = (*it);
		
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

void EditSession::ButtonCallback( Button *b, const std::string & e )
{
	Panel *p = b->owner;
	if( p->name == "patroller_options" )
	{
		if( b->name == "ok" );
		{
			string result;

			//do checks when switching focus from a text box or pressing ok. 
			//for now just use pressing ok
			//do checks? assign variables to the enemy

			showPanel = NULL;
		}
	}
	else if( p->name == "crawler_options" )
	{
		if( b->name == "ok" );
		{
			string result;

			//do checks when switching focus from a text box or pressing ok. 
			//for now just use pressing ok
			//do checks? assign variables to the enemy

			showPanel = NULL;
		}
	}
	
	cout <<"button" << endl;
}

void EditSession::TextBoxCallback( TextBox *tb, const std::string & e )
{
}

void EditSession::GridSelectorCallback( GridSelector *gs, const std::string & name )
{
	if( name != "not set" )
	{
		trackingEnemy = types[name];
		enemySprite.setTexture( trackingEnemy->imageTexture );
		enemySprite.setOrigin( enemySprite.getLocalBounds().width /2 , enemySprite.getLocalBounds().height / 2 );
	//	trackingEnemy = true;
		
//		enemySprite.setPosition
		cout << "set your cursor as the image" << endl;
	}
	else
	{
		cout << "not set" << endl;
	}
}

Panel * EditSession::CreateOptionsPanel( const std::string &name )
{
	if( name == "patroller" )
	{
		Panel *p = new Panel( "patroller_options", 200, 400, this );
		p->AddButton( "ok", Vector2i( 100, 300 ), Vector2f( 100, 50 ), "OK" );
		p->AddTextBox( "name", Vector2i( 20, 20 ), 200, 20, "test" );
		p->AddTextBox( "group", Vector2i( 20, 100 ), 200, 20, "not test" );
		//p->AddLabel( "label1", Vector2i( 20, 200 ), 30, "blah" );
		return p;
		//p->
	}
	else if( name == "crawler" )
	{
		Panel *p = new Panel( "crawler_options", 200, 400, this );
		p->AddButton( "ok", Vector2i( 100, 300 ), Vector2f( 100, 50 ), "OK" );
		p->AddTextBox( "name", Vector2i( 20, 20 ), 200, 20, "test" );
		p->AddTextBox( "group", Vector2i( 20, 100 ), 200, 20, "not test" );
		//p->AddLabel( "label1", Vector2i( 20, 200 ), 30, "blah" );
		return p;
	}
	return NULL;
}

ActorType::ActorType( const std::string & n, Panel *p )
	:name( n ), panel( p )
{
	
	iconTexture.loadFromFile( name + "_icon.png" );
	//icon.setTexture( iconTexture );
	imageTexture.loadFromFile( name + "_editor.png" );
	//image.setTexture( imageTexture );
}

//returns an error msg or "success" on success
std::string ActorParams::SetAsPatroller( ActorType *t, sf::Vector2i pos, 
	list<Vector2i> &globalPath, float speed, bool loop )
{
	type = t;

	image.setTexture( type->imageTexture );
	image.setOrigin( image.getLocalBounds().width / 2, image.getLocalBounds().height / 2 );
	image.setPosition( pos.x, pos.y );

	params.clear();

	stringstream ss;

	position = pos;	

	list<Vector2i> localPath;
	if( globalPath.size() > 1 )
	{
		list<Vector2i>::iterator it = globalPath.begin();
		++it;
		for( ; it != globalPath.end(); ++it )
		{
			Vector2i temp( (*it).x - pos.x, (*it).y - pos.y );
			localPath.push_back( temp );
		}
	}

	ss << localPath.size();
	params.push_back( ss.str() );
	ss.str( "" );

	for( list<Vector2i>::iterator it = localPath.begin(); it != localPath.end(); ++it )
	{
		ss << (*it).x  << " " << (*it).y;
		params.push_back( ss.str() );
		ss.str( "" );
	}

	if( loop )
		params.push_back( "+loop" );
	else
		params.push_back( "-loop" );
	
	ss.precision( 5 );
	ss << fixed << speed;
	params.push_back( ss.str() );

	
	return "success";
}

std::string ActorParams::SetAsCrawler( ActorType *t, TerrainPolygon *edgePolygon,
		int eIndex, double edgeQuantity, bool clockwise, float speed )
{
	type = t;
	ground = edgePolygon;
	edgeIndex = eIndex;
	groundQuantity = edgeQuantity;

	image.setTexture( type->imageTexture );
	image.setOrigin( image.getLocalBounds().width / 2, image.getLocalBounds().height );
	
	//	image.setPosition( pos.x, pos.y );
	int testIndex = 0;

	Vector2i point;

	list<Vector2i>::iterator prev = ground->points.end();
	prev--;
	list<Vector2i>::iterator curr = ground->points.begin();

	for( ; curr != ground->points.end(); ++curr )
	{
		if( edgeIndex == testIndex )
		{
			V2d pr( (*prev).x, (*prev).y );
			V2d cu( (*curr).x, (*curr).y );

			V2d newPoint( pr.x + (cu.x - pr.x) * (groundQuantity / length( cu - pr ) ), pr.y + (cu.y - pr.y ) *
											(groundQuantity / length( cu - pr ) ) );

			double angle = atan2( (cu - pr).y, (cu - pr).x ) / PI * 180;

			image.setPosition( newPoint.x, newPoint.y );
			image.setRotation( angle );

			break;
		}
		prev = curr;
		++testIndex;
	}
	//adjust for ordery
	if( edgeIndex == 0 )
		edgeIndex = ground->points.size() - 1;
	else
		edgeIndex--;


	params.clear();

	stringstream ss; 

	if( clockwise )
		params.push_back( "+clockwise" );
	else
		params.push_back( "-clockwise" );

	ss.precision( 5 );
	ss << fixed << speed;
	params.push_back( ss.str() );	

	return "success";
}

std::string ActorParams::SetAsBasicTurret( ActorType *t, TerrainPolygon *edgePolygon,
		int eIndex, double edgeQuantity, int framesBetweenFiring )
{
	type = t;
	ground = edgePolygon;
	edgeIndex = eIndex;
	groundQuantity = edgeQuantity;

	image.setTexture( type->imageTexture );
	image.setOrigin( image.getLocalBounds().width / 2, image.getLocalBounds().height );
	
	//	image.setPosition( pos.x, pos.y );
	int testIndex = 0;

	Vector2i point;

	list<Vector2i>::iterator prev = ground->points.end();
	prev--;
	list<Vector2i>::iterator curr = ground->points.begin();

	for( ; curr != ground->points.end(); ++curr )
	{
		if( edgeIndex == testIndex )
		{
			V2d pr( (*prev).x, (*prev).y );
			V2d cu( (*curr).x, (*curr).y );

			V2d newPoint( pr.x + (cu.x - pr.x) * (groundQuantity / length( cu - pr ) ), pr.y + (cu.y - pr.y ) *
											(groundQuantity / length( cu - pr ) ) );

			double angle = atan2( (cu - pr).y, (cu - pr).x ) / PI * 180;

			image.setPosition( newPoint.x, newPoint.y );
			image.setRotation( angle );

			break;
		}
		prev = curr;
		++testIndex;
	}
	//adjust for ordery
	if( edgeIndex == 0 )
		edgeIndex = ground->points.size() - 1;
	else
		edgeIndex--;

	params.clear();

	stringstream ss; 

	ss << framesBetweenFiring;

	params.push_back( ss.str() ); 

	return "success";
}

std::string ActorParams::SetAsFootTrap( ActorType *t, TerrainPolygon *edgePolygon,
		int eIndex, double edgeQuantity )
{
	type = t;
	ground = edgePolygon;
	edgeIndex = eIndex;
	groundQuantity = edgeQuantity;

	image.setTexture( type->imageTexture );
	image.setOrigin( image.getLocalBounds().width / 2, image.getLocalBounds().height );
	
	//	image.setPosition( pos.x, pos.y );
	int testIndex = 0;

	Vector2i point;

	list<Vector2i>::iterator prev = ground->points.end();
	prev--;
	list<Vector2i>::iterator curr = ground->points.begin();

	for( ; curr != ground->points.end(); ++curr )
	{
		if( edgeIndex == testIndex )
		{
			V2d pr( (*prev).x, (*prev).y );
			V2d cu( (*curr).x, (*curr).y );

			V2d newPoint( pr.x + (cu.x - pr.x) * (groundQuantity / length( cu - pr ) ), pr.y + (cu.y - pr.y ) *
											(groundQuantity / length( cu - pr ) ) );

			double angle = atan2( (cu - pr).y, (cu - pr).x ) / PI * 180;

			image.setPosition( newPoint.x, newPoint.y );
			image.setRotation( angle );

			break;
		}
		prev = curr;
		++testIndex;
	}
	//adjust for ordery
	if( edgeIndex == 0 )
		edgeIndex = ground->points.size() - 1;
	else
		edgeIndex--;

	params.clear();

	return "success";
}

ActorParams::ActorParams()
	:ground( NULL ), groundQuantity( 420.69 )
{
}

void ActorParams::Draw( sf::RenderTarget *target )
{
	target->draw( image );
}

void ActorParams::WriteFile( ofstream &of )
{
	//if( params.size() == 0 )
	//{
	//	assert( false && "no params" );
	//}
	
	//dont need number of params because the actortype determines that.
	of << type->name << " ";

	if( ground != NULL )
	{
		of << "-air" << " " << ground->writeIndex << " " << edgeIndex << " " << groundQuantity << endl;
	}
	else
	{
		of << "+air" << " " << position.x << " " << position.y << endl;
	}

	for( list<string>::iterator it = params.begin(); it != params.end(); ++it )
	{
		of << (*it) << endl;
	}
}

void ActorGroup::Draw( sf::RenderTarget *target )
{
	for( list<ActorParams*>::iterator it = actors.begin(); it != actors.end(); ++it )
	{
		(*it)->Draw( target );
	}
}

ActorGroup::ActorGroup( const std::string &n )
	:name( n )
{

}

void ActorGroup::WriteFile( std::ofstream &of )
{
	//group name and number of actors in the group
	of << name << " " << actors.size() << endl;
	for( list<ActorParams*>::iterator it = actors.begin(); it != actors.end(); ++it )
	{
		(*it)->WriteFile( of );
	}
}