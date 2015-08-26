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
#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )
#define COLOR_GREEN Color( 0, 0xcc, 0x44 )
#define COLOR_YELLOW Color( 0xff, 0xf0, 0 )
#define COLOR_ORANGE Color( 0xff, 0xbb, 0 )
#define COLOR_RED Color( 0xff, 0x22, 0 )
#define COLOR_MAGENTA Color( 0xff, 0, 0xff )
#define COLOR_WHITE Color( 0xff, 0xff, 0xff )


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
	for( list<ActorParams*>::iterator it = enemies.begin(); it != enemies.end(); ++it )
	{
		(*it)->group->actors.remove( (*it ) );
		delete (*it);
	}
}

void TerrainPolygon::Finalize()
{
	material = "mat";
	lines = new sf::Vertex[points.size()*2+1];
	int i = 0;

	FixWinding();
	//cout << "points size: " << points.size() << endl;

	vector<p2t::Point*> polyline;
	for( PointList::iterator it = points.begin(); it != points.end(); ++it )
	{
		polyline.push_back( new p2t::Point((*it).first.x, (*it).first.y ) );
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
		PointList::iterator it = points.begin(); 
		lines[0] = sf::Vector2f( (*it).first.x, (*it).first.y );
		lines[2 * points.size() - 1 ] = sf::Vector2f( (*it).first.x, (*it).first.y );
		++it;
		++i;
		while( it != points.end() )
		{
			lines[i] = sf::Vector2f( (*it).first.x, (*it).first.y );
			lines[++i] = sf::Vector2f( (*it).first.x, (*it).first.y ); 
			++i;
			++it;
		}
	}
	PointList::iterator it = points.begin();
	left = (*it).first.x;
	right = (*it).first.x;
	top = (*it).first.y;
	bottom = (*it).first.y;
	++it;
	for( ; it != points.end(); ++it )
	{
		left = min( (*it).first.x, left );
		right = max( (*it).first.x, right );
		top = min( (*it).first.y, top );
		bottom = max( (*it).first.y, bottom );
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

void TerrainPolygon::RemoveSelectedPoints()
{
	PointList temp = points;

	Reset();

	PointList::iterator it = temp.begin();
	while( it != temp.end() )
	{
		if( (*it).second ) //selected
		{
			temp.erase( it++ );
		}
		else
		{
			++it;
		}
	}

	for( PointList::iterator it2 = temp.begin(); it2 != temp.end(); ++it2 )
	{
		points.push_back( (*it2) );
	}
	//cout << "before killer finalize. poly size: " << poly->points.size() << endl;
	Finalize();
	SetSelected( true );
}

void TerrainPolygon::Draw( double zoomMultiple, RenderTarget *rt )
{
	if( va != NULL )
	rt->draw( *va );

	if( selected )
	{
		for( PointList::iterator it = points.begin(); it != points.end(); ++it )
		{
			CircleShape cs;
			cs.setRadius( 8 * zoomMultiple );
			cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );

			if( (*it).second )
				cs.setFillColor( Color::Red );
			else
				cs.setFillColor( Color::Green );

			cs.setPosition( (*it).first.x, (*it).first.y );
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

		for( PointList::iterator it = points.begin(); it != points.end(); ++it )
		{
			(*it).second = false;
		}
	}
}

bool TerrainPolygon::ContainsPoint( Vector2f test )
{
	int pointCount = points.size();

	int i, j, c = 0;

	PointList::iterator it = points.begin();
	PointList::iterator jt = points.end();
	jt--;
	
	for (; it != points.end(); jt = it++ ) 
	{
		Vector2f point((*it).first.x, (*it).first.y );
		Vector2f pointJ((*jt).first.x, (*jt).first.y );
		if ( ((point.y > test.y ) != ( pointJ.y > test.y ) ) &&
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
        PointList temp;
		for( PointList::iterator it = points.begin(); it != points.end(); ++it )
		{
			temp.push_front( (*it) );
		}
		points.clear();
		for( PointList::iterator it = temp.begin(); it != temp.end(); ++it )
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
	for( PointList::iterator it = points.begin(); it != points.end(); ++it )
	{
		pointArray[i] = (*it).first;
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

		PointList::iterator it = points.begin();
		Vector2i curr = (*it).first;
		++it;
		Vector2i next;
		

		

		for( ;; ++it )
		{
			if( it == points.end() )
				it = points.begin();

			next = (*it).first;


			PointList::iterator pit = p->points.begin();
			Vector2i pcurr = (*pit).first;
			++pit;
			Vector2i pnext;// = (*pit);

			for( ;; ++pit )		
			{
				if( pit == p->points.end() )
					pit = p->points.begin();

				pnext = (*pit).first;
			
				LineIntersection li = EditSession::SegmentIntersect( curr, next, pcurr, pnext );	

				if( !li.parallel )
				{
					//points.pop_back();
					//p->points.pop_back();
					cout << "touching!" << endl;
					return true;
				}

				pcurr = (*pit).first;

				if( pit == p->points.begin() )
					break;
			}
			curr = (*it).first;

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
		(*it)->Draw( zoomMultiple, w );
	}

	int psize = polygonInProgress->points.size();
	if( psize > 0 )
	{
		CircleShape cs;
		cs.setRadius( 5 * zoomMultiple  );
		cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
		cs.setFillColor( Color::Green );

		
		for( PointList::iterator it = polygonInProgress->points.begin(); it != polygonInProgress->points.end(); ++it )
		{
			cs.setPosition( (*it).first.x, (*it).first.y );
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
				poly->points.push_back( pair<Vector2i,bool>( Vector2i(x,y), false ) );
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
				a->group = gr;


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

				if( typeName == "goal" )
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

					a->SetAsGoal( at, terrain, edgeIndex, edgeQuantity );
				}
				else if( typeName == "patroller" )
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

					double bulletSpeed;
					is >> bulletSpeed;

					int framesWait;
					is >> framesWait;

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

					a->SetAsBasicTurret( at, terrain, edgeIndex, edgeQuantity, bulletSpeed, framesWait );
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
	bool hasGoal = false;
	for( map<string, ActorGroup*>::iterator it = groups.begin(); it != groups.end(); ++it )
	{
		ActorGroup *group = (*it).second;
		for( list<ActorParams*>::iterator it2 = group->actors.begin(); it2 != group->actors.end(); ++it2 )
		{
			if( (*it2)->type == types["goal"] )
			{
				hasGoal = true;
				break;
			}
		}
	}

	if( !hasGoal )
	{
		cout << "you need to place a goal in the map. file not written to!. add a popup to this alert later"
			<< endl;
		return;
	}



	ofstream of;
	of.open( fileName + ".brknk" );

	int pointCount = 0;
	for( list<TerrainPolygon*>::iterator it = polygons.begin(); it != polygons.end(); ++it )
	{
		pointCount += (*it)->points.size();
	}

	of << pointCount << endl;
	of << playerPosition.x << " " << playerPosition.y << endl;

	int writeIndex = 0;
	for( list<TerrainPolygon*>::iterator it = polygons.begin(); it != polygons.end(); ++it )
	{
		(*it)->writeIndex = writeIndex;
		++writeIndex;

		of << (*it)->material << " " << (*it)->points.size() << endl;
		for( PointList::iterator it2 = (*it)->points.begin(); it2 != (*it)->points.end(); ++it2 )
		{
			of << (*it2).first.x << " " << (*it2).first.y << endl;
		}
	}

	of << groups.size() << endl;
	//write the stuff for goals and remove them from the enemy stuff

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
	PointList::iterator it = poly->points.begin();

	for(; it != poly->points.end(); ++it )
	{
		if( !brush->ContainsPoint( Vector2f( (*it).first.x, (*it).first.y) ) )
		{
			startPoint = (*it).first;
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
			if( !poly->ContainsPoint( Vector2f( (*it).first.x, (*it).first.y) ) )
			{
				startPoint = (*it).first;
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
	Vector2i nextPoint = (*it).first;


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

		
		PointList::iterator min;
		Vector2i minIntersection;
		bool emptyInter = true;
		

		PointList::iterator bit = otherPoly->points.begin();
		Vector2i bCurrPoint = (*bit).first;
		++bit;
		Vector2i bNextPoint;// = (*++bit);
		Vector2i minPoint;
		
		LineIntersection li = SegmentIntersect( currPoint, nextPoint, otherPoly->points.back().first, bCurrPoint );
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
			bNextPoint = (*bit).first;
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

			z.points.push_back( pair<Vector2i,bool>( currPoint, false ) );

			
			nextPoint = (*it).first;

			
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

			currPoint = (*it).first;

			z.points.push_back( pair<Vector2i,bool>( currPoint, false ) );

		//	cout << "adding point: " << currPoint.x << ", " << currPoint.y << endl;

			if( currPoint == startPoint && !firstTime )
				break;
			

			++it;
			if( it == currentPoly->points.end() )
			{
				it = currentPoly->points.begin();
			}
			nextPoint = (*it).first;
	//		cout << "nextpoing from adding: " << nextPoint.x << ", " << nextPoint.y << endl;
		}
		firstTime = false;
	}

	poly->Reset();
	for( PointList::iterator zit = z.points.begin(); zit != z.points.end(); ++zit )
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

	bool showGraph = false;

	selectedActor = NULL;

	trackingEnemy = NULL;
	showPanel = NULL;

	sf::Font arial;
	arial.loadFromFile( "arial.ttf" );

	sf::Texture playerZoomIconTex;
	playerZoomIconTex.loadFromFile( "playerzoomicon.png" );
	sf::Sprite playerZoomIcon( playerZoomIconTex );
	
	playerZoomIcon.setOrigin( playerZoomIcon.getLocalBounds().width / 2, playerZoomIcon.getLocalBounds().height / 2 );

//	Panel p( 300, 300, this );
//	p.active = true;
//	p.AddButton( Vector2i( 50, 100 ), Vector2f( 50, 50 ), "LOL");
//	p.AddTextBox( Vector2i( 200, 200 ), 200, 15, "testing" );

	ActorGroup *emptyGroup = new ActorGroup( "--" );
	//emptyGroup->name = "";
	groups[emptyGroup->name] = emptyGroup;


	Panel *mapOptionsPanel = CreateOptionsPanel( "map" );

	Panel *patrollerPanel = CreateOptionsPanel( "patroller" );//new Panel( 300, 300, this );
	ActorType *patrollerType = new ActorType( "patroller", patrollerPanel );

	Panel *crawlerPanel = CreateOptionsPanel( "crawler" );
	ActorType *crawlerType = new ActorType( "crawler", crawlerPanel );

	Panel *basicTurretPanel = CreateOptionsPanel( "basicturret" );
	ActorType *basicTurretType = new ActorType( "basicturret", basicTurretPanel );

	Panel *footTrapPanel = CreateOptionsPanel( "foottrap" );
	ActorType *footTrapType = new ActorType( "foottrap", footTrapPanel );

	Panel *goalPanel = CreateOptionsPanel( "goal" );
	ActorType *goalType = new ActorType( "goal", goalPanel );


	types["patroller"] = patrollerType;
	types["crawler"] = crawlerType;
	types["basicturret"] = basicTurretType;
	types["foottrap"] = footTrapType;
	types["goal"] = goalType;


	GridSelector gs( 3, 2, 32, 32, this );
	gs.active = false;

	sf::Sprite s0( patrollerType->iconTexture );
	sf::Sprite s1( crawlerType->iconTexture );
	sf::Sprite s2( basicTurretType->iconTexture );
	sf::Sprite s3( footTrapType->iconTexture );
	sf::Sprite s4( goalType->iconTexture );


	gs.Set( 0, 0, s0, "patroller" );
	gs.Set( 1, 0, s1, "crawler" );
	gs.Set( 0, 1, s2, "basicturret" );
	gs.Set( 1, 1, s3, "foottrap" );
	gs.Set( 2, 0, s4, "goal" );

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

	//Texture goalTex;
	//goalTex.loadFromFile( "goal.png" );
	//Sprite goalSprite( goalTex );

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

	//goalSprite.setOrigin( goalSprite.getLocalBounds().width / 2, goalSprite.getLocalBounds().height / 2 );

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

	Color graphColor = Color( 200, 50, 50, 100 );
	//int max = 1000000;
	int numLines = 30;
	sf::VertexArray graphLines( sf::Lines, numLines * 8 );
	int graphSep = 32;
	int graphMax = graphSep * numLines;
	int temp = -graphMax;

	//horiz
	for( int i = 0; i < numLines * 4; i += 2 )
	{
		graphLines[i] = sf::Vertex(sf::Vector2<float>(-graphMax, temp), graphColor );
		graphLines[i+1] = sf::Vertex(sf::Vector2<float>(graphMax, temp), graphColor );
		temp += graphSep;
	}

	//vert
	temp = -graphMax;
	for( int i = numLines * 4; i < numLines * 8; i += 2 )
	{
		graphLines[i] = sf::Vertex(sf::Vector2<float>(temp, -graphMax), graphColor );
		graphLines[i+1] = sf::Vertex(sf::Vector2<float>(temp, graphMax), graphColor );
		temp += graphSep;
	}
	

	bool s = sf::Keyboard::isKeyPressed( sf::Keyboard::T );
	
	V2d menuDownPos;
	Emode menuDownStored;

	Emode mode = CREATE_TERRAIN;
	Emode stored = mode;
	bool canCreatePoint = true;
	gs.active = true;

	


	double circleDist = 100;
	double circleRadius = 50;

	V2d topPos =  V2d( 0, -1 ) * circleDist;
	V2d upperRightPos = V2d( sqrt( 3.0 ) / 2, -.5 ) * circleDist;
	V2d lowerRightPos = V2d( sqrt( 3.0 ) / 2, .5 ) * circleDist;

	V2d upperLeftPos = V2d( -sqrt( 3.0 ) / 2, -.5 ) * circleDist;
	V2d lowerLeftPos = V2d( -sqrt( 3.0 ) / 2, .5 ) * circleDist;

	V2d bottomPos = V2d( 0, 1 ) * circleDist;

	string menuSelection = "";

	selectedPlayer = false;
	selectedActorGrabbed = false;

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
								if( showPanel != NULL )
								{	
									showPanel->Update( true, uiMouse.x, uiMouse.y );
									break;
								}
							}
							
							break;
						}
					case Event::MouseButtonReleased:
						{
							if( showPanel != NULL )
							{	
								showPanel->Update( false, uiMouse.x, uiMouse.y );
							}
							break;
						}
					case Event::MouseWheelMoved:
						{
							
						}
					case Event::KeyPressed:
						{
							if( showPanel != NULL )
							{
								showPanel->SendKey( ev.key.code, ev.key.shift );
								break;
							}


							if( ev.key.code == Keyboard::Space )
							{
								if( polygonInProgress->points.size() > 2 )
								{
									//test final line

									if( !PointValid( polygonInProgress->points.back().first, polygonInProgress->points.front().first ) )
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
							else if( ev.key.code == sf::Keyboard::V || ev.key.code == sf::Keyboard::Delete )
							{
								//cout << "PRESSING V: " << polygonInProgress->points.size() << endl;
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
							if( ev.mouseButton.button == Mouse::Left )
							{
								if( showPanel != NULL )
								{	
									showPanel->Update( true, uiMouse.x, uiMouse.y );
									break;
								}

								if( playerSprite.getGlobalBounds().contains( worldPos.x, worldPos.y ) )
								{
									selectedActor = NULL;
									selectedPlayer = true;
									grabPlayer = true;
									grabPos = Vector2i( worldPos.x, worldPos.y );
									
									break;
								}
								else
								{
									grabPlayer = false;
									selectedPlayer = false;
								}

								bool emptySpace = true;

								for( list<TerrainPolygon*>::iterator it = selectedPolygons.begin(); 
									it != selectedPolygons.end(); ++it )
								{
									for( PointList::iterator it2 = (*it)->points.begin(); 
										it2 != (*it)->points.end(); ++it2 )
									{
										if( length( worldPos - V2d( (*it2).first.x, (*it2).first.y ) ) < 8 * zoomMultiple )
										{
											if( (*it2).second ) //selected 
											{
												(*it2).second = false;
												emptySpace = false;
												break;
											}
											else
											{
												if( Keyboard::isKeyPressed( Keyboard::LShift ) )
												{
													
												}
												else
												{
													for( PointList::iterator tempIt = (*it)->points.begin();
														tempIt != (*it)->points.end(); ++tempIt )
													{
														(*tempIt).second = false;
													}
												}

												(*it2).second = true;
												emptySpace = false;
												break;


											}
											
											
										}
									}
								}

								
								if( emptySpace )
								{
									for( list<TerrainPolygon*>::iterator it = polygons.begin(); 
										it != polygons.end(); ++it )
									{
											if((*it)->ContainsPoint( Vector2f(worldPos.x, worldPos.y ) ) )
											{
												emptySpace = false;
												//(*it)->SetSelected( !((*it)->selected ) );
												if( (*it)->selected )
												{
													//selectedPolygons.push_back( (*it) );
													selectedPolygons.remove( (*it ) );
													(*it)->SetSelected( false );
												}
												else
												{
													if( sf::Keyboard::isKeyPressed( Keyboard::LShift ) )
													{
														selectedPolygons.push_back( (*it) );
														(*it)->SetSelected( true );
													}
													else
													{
														for( list<TerrainPolygon*>::iterator selIt = 
															selectedPolygons.begin(); 
															selIt != selectedPolygons.end(); ++selIt )
														{
															(*selIt)->SetSelected( false );
														}
														selectedPolygons.clear();
														selectedPolygons.push_back( (*it) );
														(*it)->SetSelected( true );
													}
													//selectedPolygons.remove( (*it ) );
												}
												break;
											}
									}
								}

								if( emptySpace )
								{
									for( list<TerrainPolygon*>::iterator it = selectedPolygons.begin(); 
										it != selectedPolygons.end(); ++it )
									{
										(*it)->SetSelected( false );

									}
									selectedPolygons.clear();
								}
								
							//	cout << "here before loop" << endl;
								bool empty = true;
								for( map<string, ActorGroup*>::iterator it = groups.begin(); it != groups.end() && empty; ++it )
								{
									list<ActorParams*> &actors = it->second->actors;
									for( list<ActorParams*>::iterator it2 = actors.begin(); it2 != actors.end() && empty; ++it2 )
									{
										sf::FloatRect bounds = (*it2)->image.getGlobalBounds();
										if( bounds.contains( Vector2f( worldPos.x, worldPos.y ) ) )
										{
											selectedActor = (*it2);
											selectedActorGrabbed = true;
											grabPos = Vector2i( worldPos.x, worldPos.y );

											empty = false;
											//cout << "enemy selected" << endl;

											for( list<TerrainPolygon*>::iterator it3 = selectedPolygons.begin(); 
												it3 != selectedPolygons.end(); ++it3 )
											{
												(*it3)->SetSelected( false );
											}
											selectedPolygons.clear();
										}
									}
								}


							//	cout << "made it!!!" << endl;
								if( empty )
								{
									selectedActor = NULL;
									selectedActorGrabbed = false;
								}

							}
							break;
						}
					case Event::MouseButtonReleased:
						{
							if( ev.mouseButton.button == Mouse::Left )
							{
								if( showPanel != NULL )
								{	
									showPanel->Update( false, uiMouse.x, uiMouse.y );
								}

								grabPlayer = false;
								selectedActorGrabbed = false;
							}
							break;
						}
					case Event::MouseWheelMoved:
						{
							break;
						}
					case Event::KeyPressed:
						{
							if( showPanel != NULL )
							{
								showPanel->SendKey( ev.key.code, ev.key.shift );
								break;
							}


							if( ev.key.code == Keyboard::V || ev.key.code == Keyboard::Delete )
							{
								if( CountSelectedPoints() > 0 )
								{
									for( list<TerrainPolygon*>::iterator it = selectedPolygons.begin(); 
										it != selectedPolygons.end(); ++it )
									{
										(*it)->RemoveSelectedPoints();
									}
								}
								else if( selectedActor != NULL )
								{
									if( selectedActor->ground != NULL )
									{
										selectedActor->ground->enemies.remove( selectedActor );
									}
									selectedActor->group->actors.remove( selectedActor );
									delete selectedActor;
									
									selectedActor = NULL;
								}
								else
								{
									
									for( list<TerrainPolygon*>::iterator it = selectedPolygons.begin();
										it != selectedPolygons.end(); ++it )
									{
										polygons.remove( (*it) );
										delete (*it);
									}
									selectedPolygons.clear();

									cout << "destroying terrain. eney: " << selectedActor << endl;
								}
							}
							else if( ev.key.code == Keyboard::W )
							{
								if( CountSelectedPoints() > 0 )
								{
									pointGrab = true;
									pointGrabPos = Vector2i( worldPos.x, worldPos.y );
								}
								else if( selectedPolygons.size() > 0 )
								{
									polyGrab = true;
									polyGrabPos = Vector2i( worldPos.x, worldPos.y );
								}
							}
							break;
						}
					case Event::KeyReleased:
						{
							if( ev.key.code == Keyboard::W )
							{
								pointGrab = false;
								polyGrab = false;
							}
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
								if( showPanel == NULL && trackingEnemy != NULL )
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
											//trackingEnemy = NULL;
										}
									}
									else if( trackingEnemy->name == "basicturret" )
									{
										if( enemyEdgePolygon != NULL )
										{
											showPanel = trackingEnemy->panel;
										}
									}
									else if( trackingEnemy->name == "foottrap" )
									{
										if( enemyEdgePolygon != NULL )
										{
											showPanel = trackingEnemy->panel;

											/*showPanel = trackingEnemy->panel;
											trackingEnemy = NULL;
											ActorParams *actor = new ActorParams;
											actor->group = groups["--"];
											actor->SetAsFootTrap( footTrapType, enemyEdgePolygon, enemyEdgeIndex, 
												enemyEdgeQuantity );
											groups["--"]->actors.push_back( actor );*/
										}
									}
									else if( trackingEnemy->name == "goal" )
									{
										if( enemyEdgePolygon != NULL )
										{
											showPanel = trackingEnemy->panel;
											trackingEnemy = NULL;
											ActorParams *actor = new ActorParams;
											actor->group = groups["--"];
											actor->SetAsGoal( goalType, enemyEdgePolygon, enemyEdgeIndex, 
												enemyEdgeQuantity );
											groups["--"]->actors.push_back( actor );
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
							break;
						}
					case Event::KeyPressed:
						{
							if( showPanel != NULL )
							{
								showPanel->SendKey( ev.key.code, ev.key.shift );
							}
							break;
						}
					case Event::KeyReleased:
						{
							break;
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
							
							V2d worldTop = menuDownPos + topPos;
							V2d worldUpperLeft = menuDownPos + upperLeftPos;
							V2d worldUpperRight = menuDownPos + upperRightPos;
							V2d worldLowerRight = menuDownPos + lowerRightPos;
							V2d worldLowerLeft = menuDownPos + lowerLeftPos;
							V2d worldBottom = menuDownPos + bottomPos;


							if( length( releasePos - worldTop ) < circleRadius )
							{
								menuSelection = "top";
							}
							else if( length( releasePos - worldUpperLeft ) < circleRadius )
							{
								menuSelection = "upperleft";
							}
							else if( length( releasePos - worldUpperRight ) < circleRadius )
							{
								menuSelection = "upperright";
							}
							else if( length( releasePos - worldLowerLeft ) < circleRadius )
							{
								menuSelection = "lowerleft";
							}
							else if( length( releasePos - worldLowerRight ) < circleRadius )
							{
								menuSelection = "lowerright";
							}
							else if( length( releasePos - worldBottom ) < circleRadius )
							{
								menuSelection = "bottom";
							}
							else
							{
								mode = menuDownStored;
								menuSelection = "none";
							}

							if( menuDownStored == EDIT && menuSelection != "none" )
							{
								selectedPlayer = false;
								selectedActor = NULL;
								for( list<TerrainPolygon*>::iterator it = selectedPolygons.begin(); 
									it != selectedPolygons.end(); ++it )
								{
									(*it)->SetSelected( false );
								}
								selectedPolygons.clear();
							}
							else if( menuDownStored == CREATE_TERRAIN && menuSelection != "none" )
							{
								polygonInProgress->points.clear();
							}

							cout << "menu: " << menuSelection << endl;
							if( menuSelection == "top" )
							{
								mode = EDIT;
							}
							else if( menuSelection == "upperleft" )
							{
								mode = CREATE_ENEMY;
							}
							else if( menuSelection == "upperright" )
							{
								mode = CREATE_TERRAIN;
							}
							else if( menuSelection == "lowerleft" )
							{
							}
							else if( menuSelection == "lowerright" )
							{
								showPanel = mapOptionsPanel;
								mode = menuDownStored;
							}
							else if( menuSelection == "bottom" )
							{
							}
							

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
							if( ( ev.key.code == Keyboard::V || ev.key.code == Keyboard::Delete ) && patrolPath.size() > 1 )
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
								actor->group = groups["--"];
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
			
		
		}

		if( quit )
			break;

		showGraph = false;

		switch( mode )
		{
		case CREATE_TERRAIN:
			{
				/*if( polygonInProgress->points.size() > 0 && Keyboard::isKeyPressed( Keyboard::LShift ) ) 
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
				}*/
				if( showPanel != NULL )
					break;

				if( //polygonInProgress->points.size() > 0 && 
					Keyboard::isKeyPressed( Keyboard::G ) )
				{
					int adjX, adjY;
					
					testPoint.x /= 32;
					testPoint.y /= 32;

					if( testPoint.x > 0 )
						testPoint.x += .5f;
					else if( testPoint.x < 0 )
						testPoint.x -= .5f;

					if( testPoint.y > 0 )
						testPoint.y += .5f;
					else if( testPoint.y < 0 )
						testPoint.y -= .5f;

					adjX = ((int)testPoint.x) * 32;
					adjY = ((int)testPoint.y) * 32;
					
					testPoint = Vector2f( adjX, adjY );
					showGraph = true;
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
						Vector2i worldi( testPoint.x, testPoint.y );
						if( !polygonInProgress->points.empty() && length( V2d( testPoint.x, testPoint.y ) - Vector2<double>(polygonInProgress->points.back().first.x, 
							polygonInProgress->points.back().first.y )  ) >= minimumEdgeLength * std::max(zoomMultiple,1.0 ) )
						{
							if( PointValid( polygonInProgress->points.back().first, worldi ) )
							{
								polygonInProgress->points.push_back( pair<Vector2i,bool>( worldi, false ) );
							}
						}
						else if( polygonInProgress->points.empty() )
						{
							polygonInProgress->points.push_back( pair<Vector2i,bool>( worldi, false ) );
							
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
				if( pointGrab )
				{
					
					pointGrabDelta = Vector2i( worldPos.x, worldPos.y ) - pointGrabPos;
					pointGrabPos = Vector2i( worldPos.x, worldPos.y );

	

					if( true )
					{
						
						for( list<TerrainPolygon*>::iterator it = selectedPolygons.begin();
							it != selectedPolygons.end(); ++it )
						{
							bool affected = false;

							PointList & points = (*it)->points;

							for( PointList::iterator pointIt = points.begin();
								pointIt != points.end(); ++pointIt )
							{
								if( (*pointIt).second ) //selected
								{
									(*pointIt).first += pointGrabDelta;
									affected = true;
								}
							}

							if( affected )
							{
								PointList temp = (*it)->points;

								(*it)->Reset();

								for( PointList::iterator tempIt = temp.begin(); tempIt != temp.end(); 
									++tempIt )
								{
									(*it)->points.push_back( (*tempIt ) );
								}
								(*it)->Finalize();
								(*it)->SetSelected( true );
								
							}
						}
					}

				}
				else if( polyGrab )
				{
					polyGrabDelta = Vector2i( worldPos.x, worldPos.y ) - polyGrabPos;
					polyGrabPos = Vector2i( worldPos.x, worldPos.y );
	
					for( list<TerrainPolygon*>::iterator it = selectedPolygons.begin();
						it != selectedPolygons.end(); ++it )
					{
						PointList & points = (*it)->points;

						for( PointList::iterator pointIt = points.begin();
							pointIt != points.end(); ++pointIt )
						{
							(*pointIt).first += polyGrabDelta;		
						}

						PointList temp = (*it)->points;

						(*it)->Reset();

						for( PointList::iterator tempIt = temp.begin(); tempIt != temp.end(); 
							++tempIt )
						{
							(*it)->points.push_back( (*tempIt ) );
						}
						(*it)->Finalize();
						(*it)->SetSelected( true );
					}
				}
				

				break;
			}
		case CREATE_ENEMY:
			{
				if( trackingEnemy != NULL && showPanel == NULL )
				{
					enemySprite.setOrigin( enemySprite.getLocalBounds().width / 2, enemySprite.getLocalBounds().height / 2 );
					enemySprite.setRotation( 0 );
					enemySprite.setPosition( w->mapPixelToCoords( sf::Mouse::getPosition( *w ) ) );
					
				}

				if( showPanel == NULL && trackingEnemy != NULL && ( trackingEnemy->name == "crawler" 
					|| trackingEnemy->name == "basicturret"
					|| trackingEnemy->name == "foottrap" 
					|| trackingEnemy->name == "goal" ) )
				{
					enemyEdgePolygon = NULL;
				
					double testRadius = 200;
					
					for( list<TerrainPolygon*>::iterator it = polygons.begin(); it != polygons.end(); ++it )
					{
						if( testPoint.x >= (*it)->left - testRadius && testPoint.x <= (*it)->right + testRadius
							&& testPoint.y >= (*it)->top - testRadius && testPoint.y <= (*it)->bottom + testRadius )
						{
							PointList::iterator prevIt = (*it)->points.end();
							prevIt--;
							PointList::iterator currIt = (*it)->points.begin();

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
										V2d( testPoint.x - (*prevIt).first.x, testPoint.y - (*prevIt).first.y ), 
										normalize( V2d( (*currIt).first.x - (*prevIt).first.x, (*currIt).first.y - (*prevIt).first.y ) ) ) );
									double testQuantity =  dot( 
											V2d( testPoint.x - (*prevIt).first.x, testPoint.y - (*prevIt).first.y ), 
											normalize( V2d( (*currIt).first.x - (*prevIt).first.x, (*currIt).first.y - (*prevIt).first.y ) ) );

									V2d pr( (*prevIt).first.x, (*prevIt).first.y );
									V2d cu( (*currIt).first.x, (*currIt).first.y );
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
				/*if( //polygonInProgress->points.size() > 0 && 
					Keyboard::isKeyPressed( Keyboard::G ) )
				{
					int adjX, adjY;
					
					testPoint.x /= 32;
					testPoint.y /= 32;

					if( testPoint.x > 0 )
						testPoint.x += .5f;
					else if( testPoint.x < 0 )
						testPoint.x -= .5f;

					if( testPoint.y > 0 )
						testPoint.y += .5f;
					else if( testPoint.y < 0 )
						testPoint.y -= .5f;

					adjX = ((int)testPoint.x) * 32;
					adjY = ((int)testPoint.y) * 32;
					
					testPoint = Vector2f( adjX, adjY );
					showGraph = true;
				}*/
				if( showPanel != NULL )
					break;


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

		//cout << "here before crash" << endl;
		

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
					Vector2i backPoint = polygonInProgress->points.back().first;
			
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
						for( PointList::iterator it = polygonInProgress->points.begin(); 
							it != polygonInProgress->points.end(); ++it )
						{
							v[i] = Vertex( Vector2f( (*it).first.x, (*it).first.y ) );
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
		case SELECT_MODE:
			{
				

			

				break;
			}
		}
		

		iconSprite.setScale( view.getSize().x / 960.0, view.getSize().y / 540.0 );
		iconSprite.setPosition( view.getCenter().x + 200 * iconSprite.getScale().x, view.getCenter().y - 250 * iconSprite.getScale().y );
		
		if( mode == EDIT )
		{
			if( selectedPlayer && grabPlayer && length( V2d( grabPos.x, grabPos.y ) - worldPos ) > 10 )
			{
				playerPosition = Vector2i( worldPos.x, worldPos.y );
			}
			else if( selectedActorGrabbed && length( V2d( grabPos.x, grabPos.y ) - worldPos ) > 10 )
			{
				/*if(  false && selectedActor != NULL && ( selectedActor->type->name == "crawler" 
					|| selectedActor->type->name== "basicturret"
					|| selectedActor->type->name == "foottrap" 
					|| selectedActor->type->name == "goal" ) )
				{
					enemyEdgePolygon = NULL;
					V2d testPoint = worldPos;
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
								
								//selectedActor->ground = enemyEdgeIndex;
								//selectedActor->groundQuantity = enemyEdgeQuantity;

								
								//cout << "pos: " << closestPoint.x << ", " << closestPoint.y << endl;
								//cout << "minDist: " << minDistance << endl;

								//break;
							}
						}
					}
				}
				else*/
				{
					if( selectedActor->type->name == "patroller" )
					{
						
						selectedActor->position = Vector2i( worldPos.x, worldPos.y );
						selectedActor->image.setPosition( worldPos.x, worldPos.y );
					}
				}
				
			}
		}

		playerSprite.setPosition( playerPosition.x, playerPosition.y );

		w->draw( playerSprite );
		
		w->draw( iconSprite );

		if( showPanel == NULL && sf::Keyboard::isKeyPressed( Keyboard::H ) )
		{
			alphaTextSprite.setScale( .5 * view.getSize().x / 960.0, .5 * view.getSize().y / 540.0 );
			alphaTextSprite.setOrigin( alphaTextSprite.getLocalBounds().width / 2, alphaTextSprite.getLocalBounds().height / 2 );
			alphaTextSprite.setPosition( view.getCenter().x, view.getCenter().y );
			w->draw( alphaTextSprite );
		}

		playerSprite.setPosition( playerPosition.x, playerPosition.y );

		if( mode == EDIT )
		{
			if( selectedActor != NULL )
			{
				
				sf::FloatRect bounds = selectedActor->image.getGlobalBounds();
				sf::RectangleShape rs( sf::Vector2f( bounds.width, bounds.height ) );
				
				rs.setOutlineColor( Color::Cyan );				
				rs.setFillColor( Color::Transparent );
				rs.setOutlineThickness( 5 );
				rs.setPosition( bounds.left, bounds.top );
				//rs.setFillColor( Color::Magenta );
				w->draw( rs );
				//cout << "draw rectangle"  << endl;
			}
			else if( selectedPlayer )
			{
				sf::FloatRect bounds = playerSprite.getGlobalBounds();
				sf::RectangleShape rs( sf::Vector2f( bounds.width, bounds.height ) );

				rs.setOutlineColor( Color::Cyan );				
				rs.setFillColor( Color::Transparent );
				rs.setOutlineThickness( 5 );
				rs.setPosition( bounds.left, bounds.top );				

				w->draw( rs );
			}
		}

		//display graph
		if( showGraph )
		{
			Vector2f adjustment;
			for( int i = 0; i < numLines * 8; ++i )
			{
				int adjX, adjY;
				float x = view.getCenter().x;
				float y = view.getCenter().y;

				x /= 32;
				y /= 32;

				if( x > 0 )
					x += .5f;
				else if( y < 0 )
					y -= .5f;

				if( y > 0 )
					y += .5f;
				else if( y < 0 )
					y -= .5f;

				adjX = ((int)x) * 32;
				adjY = ((int)y) * 32;
					
				adjustment = Vector2f( adjX, adjY );
				
				graphLines[i].position += adjustment;
			}
			
			w->draw( graphLines );

			for( int i = 0; i < numLines * 8; ++i )
			{
				graphLines[i].position -= adjustment;
			}
		}

		if( zoomMultiple > 7 )
		{
			playerZoomIcon.setPosition( playerPosition.x, playerPosition.y );
			playerZoomIcon.setScale( zoomMultiple * 2, zoomMultiple * 2 );
			w->draw( playerZoomIcon );
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
					//if( showPanel != NULL )
					//{
					//	showPanel->Draw( w );
					//}
					break;
				}
			case SELECT_MODE:
				{
					w->draw( guiMenuSprite );


					Color c;


					CircleShape cs;
					cs.setRadius( circleRadius );
					cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );


					cs.setFillColor( COLOR_BLUE );
					cs.setPosition( (menuDownPos + upperRightPos).x, (menuDownPos + upperRightPos).y );
					w->draw( cs );

					sf::Text textblue;
					textblue.setCharacterSize( 14 );
					textblue.setFont( arial );
					textblue.setString( "CREATE\nTERRAIN" );
					textblue.setColor( sf::Color::White );
					textblue.setOrigin( textblue.getLocalBounds().width / 2, textblue.getLocalBounds().height / 2 );
					textblue.setPosition( (menuDownPos + upperRightPos).x, (menuDownPos + upperRightPos).y );
					w->draw( textblue);


					cs.setFillColor( COLOR_GREEN );
					cs.setPosition( (menuDownPos + lowerRightPos).x, (menuDownPos + lowerRightPos).y );
					w->draw( cs );

					sf::Text textgreen;
					textgreen.setCharacterSize( 14 );
					textgreen.setFont( arial );
					textgreen.setString( "MAP\nOPTIONS" );
					textgreen.setColor( sf::Color::White );
					textgreen.setOrigin( textgreen.getLocalBounds().width / 2, textgreen.getLocalBounds().height / 2 );
					textgreen.setPosition( (menuDownPos + lowerRightPos).x, (menuDownPos + lowerRightPos).y );
					w->draw( textgreen );


					cs.setFillColor( COLOR_YELLOW );
					cs.setPosition( (menuDownPos + bottomPos).x, (menuDownPos + bottomPos).y );
					w->draw( cs );

					cs.setFillColor( COLOR_ORANGE );
					cs.setPosition( (menuDownPos + lowerLeftPos).x, (menuDownPos + lowerLeftPos).y );
					w->draw( cs );

					cs.setFillColor( COLOR_RED );
					cs.setPosition( (menuDownPos + upperLeftPos).x, (menuDownPos + upperLeftPos).y );
					w->draw( cs );

					sf::Text textred;
					textred.setString( "CREATE\nENEMIES" );
					textred.setFont( arial );
					textred.setCharacterSize( 14 );
					textred.setColor( sf::Color::White );
					textred.setOrigin( textred.getLocalBounds().width / 2, textred.getLocalBounds().height / 2 );
					textred.setPosition( (menuDownPos + upperLeftPos).x, (menuDownPos + upperLeftPos).y );
					w->draw( textred );

					cs.setFillColor( COLOR_MAGENTA );
					cs.setPosition( (menuDownPos + topPos).x, (menuDownPos + topPos).y );
					w->draw( cs );

					sf::Text textmag;
					if( menuDownStored == EditSession::EDIT && selectedActor != NULL )
					{
						textmag.setString( "EDIT\nENEMY" );
					}
					else
					{
						textmag.setString( "EDIT" );
					}
				
					
					textmag.setFont( arial );
					textmag.setCharacterSize( 14 );
					textmag.setColor( sf::Color::White );
					textmag.setOrigin( textmag.getLocalBounds().width / 2, textmag.getLocalBounds().height / 2 );
					textmag.setPosition( (menuDownPos + topPos).x, (menuDownPos + topPos).y );
					w->draw( textmag );

					break;
				}
			case EDIT:
				{
					
					break;
				}
		}

		if( showPanel != NULL )
		{
			showPanel->Draw( w );
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
		PointList::iterator it = polygonInProgress->points.begin();
		//polygonInProgress->points.push_back( polygonInProgress->points.back() )
		Vector2i pre = (*it).first;
		++it;
		
		//minimum angle
		{
			if( polygonInProgress->points.size() >= 2 )
			{
				PointList::reverse_iterator rit = polygonInProgress->points.rbegin();
				rit++;
				double ff = dot( normalize( V2d( point.x, point.y ) - V2d( polygonInProgress->points.back().first.x, polygonInProgress->points.back().first.y ) )
					, normalize( V2d((*rit).first.x, (*rit).first.y ) - V2d( polygonInProgress->points.back().first.x, polygonInProgress->points.back().first.y ) ) );
				if( ff > minAngle )
				{
					//cout << "ff: " << ff << endl;
					return false;
				}
			}
		}

		//return true;

		//make sure I'm not too close to the very first point and that my line isn't too close to the first point either
		if( point.x != polygonInProgress->points.front().first.x || point.y != polygonInProgress->points.front().first.y )
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

		if( point.x == polygonInProgress->points.front().first.x && point.y == polygonInProgress->points.front().first.y )
		{
			pre = (*it).first;
			++it;
		}

		{
 		for( ; it != polygonInProgress->points.end(); ++it )
		{
			if( (*it) == polygonInProgress->points.back() )
				continue;

			LineIntersection li = lineIntersection( V2d( prev.x, prev.y ), V2d( point.x, point.y ),
						V2d( pre.x, pre.y ), V2d( (*it).first.x, (*it).first.y ) );
			float tempLeft = min( pre.x, (*it).first.x ) - 0;
			float tempRight = max( pre.x, (*it).first.x ) + 0;
			float tempTop = min( pre.y, (*it).first.y ) - 0;
			float tempBottom = max( pre.y, (*it).first.y ) + 0;
			if( !li.parallel )
			{
				
				double separation = length( V2d(point.x, point.y) - V2d((*it).first.x,(*it).first.y ) );
				
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
				Vector2i bi = (*it).first - pre;
				V2d a(ai.x, ai.y);
				V2d b(bi.x, bi.y);
				double res = abs(cross( a, normalize( b )));
				double des = dot( a, normalize( b ));

				Vector2i ci = (*it).first - prev;
				Vector2i di = point - prev;
				V2d c( ci.x, ci.y);
				V2d d( di.x, di.y );

				double res2 = abs( cross( c, normalize( d ) ) );
				double des2 = dot( c, normalize( d ) );

				//cout << "minedgelength: " << minimumEdgeLength <<  ", " << res << endl;

				if( point.x == polygonInProgress->points.front().first.x && point.y == polygonInProgress->points.front().first.y )
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
			pre = (*it).first;
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
			PointList::iterator it2 = p->points.begin();
			Vector2i prevPoint = (*it2).first;
			++it2;
			for( ; it2 != p->points.end(); ++it2 )
			{
				LineIntersection li = lineIntersection( V2d( prevPoint.x, prevPoint.y ), V2d((*it2).first.x, (*it2).first.y),
					V2d( prev.x, prev.y ), V2d( point.x, point.y ) );
				float tempLeft = min( prevPoint.x, (*it2).first.x );
				float tempRight = max( prevPoint.x, (*it2).first.x );
				float tempTop = min( prevPoint.y, (*it2).first.y );
				float tempBottom = max( prevPoint.y, (*it2).first.y );
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
				prevPoint = (*it2).first;
				
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
			bool loop = p->checkBoxes["loop"]->checked;


			

			showPanel = NULL;
		}
	}
	else if( p->name == "crawler_options" )
	{
		if( b->name == "ok" );
		{
			bool clockwise = p->checkBoxes["clockwise"]->checked;
			double speed;

			stringstream ss;
			string s = p->textBoxes["speed"]->text.getString().toAnsiString();
			ss << s;

			ss >> speed;

			if( ss.fail() )
			{
				cout << "stringstream to integer parsing error" << endl;
				ss.clear();
				assert( false );
			}

			ActorParams *actor = new ActorParams;							
			actor->SetAsCrawler( types["crawler"], enemyEdgePolygon, enemyEdgeIndex, 
				enemyEdgeQuantity, clockwise, speed );
			actor->group = groups["--"];
			groups["--"]->actors.push_back( actor);
			trackingEnemy = NULL;

			//cout << "clockwise: " << (int)clockwise << ", speed: " << speed << endl;
			//do checks when switching focus from a text box or pressing ok. 
			//for now just use pressing ok
			//do checks? assign variables to the enemy

			showPanel = NULL;
		}
	}
	else if( p->name == "basicturret_options" )
	{	
		if( b->name == "ok" )
		{
			stringstream ss;
			string bulletSpeedString = p->textBoxes["bulletspeed"]->text.getString().toAnsiString();
			string framesWaitString = p->textBoxes["waitframes"]->text.getString().toAnsiString();
			ss << bulletSpeedString;
			

			double bulletSpeed;
			ss >> bulletSpeed;

			if( ss.fail() )
			{
				assert( false );
			}

			ss.clear();

			ss << framesWaitString;

			int framesWait;
			ss >> framesWait;

			if( ss.fail() )
			{
				assert( false );
			}

			ActorParams *actor = new ActorParams;
			actor->group = groups["--"];
			actor->SetAsBasicTurret( types["basicturret"], enemyEdgePolygon, enemyEdgeIndex, 
				enemyEdgeQuantity, bulletSpeed, framesWait );
			groups["--"]->actors.push_back( actor);

			showPanel = NULL;
			trackingEnemy = NULL;

		}	
	}
	else if( p->name == "foottrap_options" )
	{
		if( b->name == "ok" )
		{
			ActorParams *actor = new ActorParams;
			actor->group = groups["--"];
			actor->SetAsFootTrap( types["foottrap"], enemyEdgePolygon, enemyEdgeIndex, 
				enemyEdgeQuantity );
			groups["--"]->actors.push_back( actor );

			showPanel = NULL;
			trackingEnemy = NULL;
		}
	}

	else if( p->name == "map_options" )
	{
		if( b->name == "ok" );
		{
			int minEdgeSize;

			stringstream ss;
			string s = p->textBoxes["minedgesize"]->text.getString().toAnsiString();
			ss << s;

			ss >> minEdgeSize;

			if( ss.fail() )
			{
				cout << "stringstream to integer parsing error" << endl;
				ss.clear();
				assert( false );
			}

			if( minEdgeSize < 8 )
			{
				assert( false && "made min edge length too small!" );
			}


			minimumEdgeLength = minEdgeSize;
			showPanel = NULL;
		}
	}
	//cout <<"button" << endl;
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

		enemySprite.setTextureRect( sf::IntRect( 0, 0, trackingEnemy->imageTexture.getSize().x, 
			trackingEnemy->imageTexture.getSize().y ) );

		enemySprite.setOrigin( enemySprite.getLocalBounds().width /2 , enemySprite.getLocalBounds().height / 2 );
	
		cout << "set your cursor as the image" << endl;
	}
	else
	{
		cout << "not set" << endl;
	}
}

void EditSession::CheckBoxCallback( CheckBox *cb, const std::string & e )
{
	cout << cb->name << " was " << e << endl;
}

Panel * EditSession::CreateOptionsPanel( const std::string &name )
{
	if( name == "patroller" )
	{
		Panel *p = new Panel( "patroller_options", 200, 400, this );
		p->AddButton( "ok", Vector2i( 100, 300 ), Vector2f( 100, 50 ), "OK" );
		p->AddTextBox( "name", Vector2i( 20, 20 ), 200, 20, "test" );
		p->AddTextBox( "group", Vector2i( 20, 100 ), 200, 20, "not test" );
		p->AddLabel( "loop_label", Vector2i( 20, 150 ), 20, "loop" );
		p->AddCheckBox( "loop", Vector2i( 120, 155 ) ); 
		p->AddTextBox( "speed", Vector2i( 20, 200 ), 200, 20, "10" );
		//p->AddLabel( "label1", Vector2i( 20, 200 ), 30, "blah" );
		return p;
		//p->
	}
	else if( name == "crawler" )
	{
		Panel *p = new Panel( "crawler_options", 200, 400, this );
		p->AddButton( "ok", Vector2i( 100, 300 ), Vector2f( 100, 50 ), "OK" );
		p->AddTextBox( "name", Vector2i( 20, 20 ), 200, 20, "name_test" );
		p->AddTextBox( "group", Vector2i( 20, 100 ), 200, 20, "group_test" );
		p->AddLabel( "clockwise_label", Vector2i( 20, 150 ), 20, "clockwise" );
		p->AddCheckBox( "clockwise", Vector2i( 120, 155 ) ); 
		p->AddTextBox( "speed", Vector2i( 20, 200 ), 200, 20, "10" );
		//p->AddLabel( "label1", Vector2i( 20, 200 ), 30, "blah" );
		return p;
	}
	else if( name == "basicturret" )
	{
		Panel *p = new Panel( "basicturret_options", 200, 400, this );
		p->AddButton( "ok", Vector2i( 100, 300 ), Vector2f( 100, 50 ), "OK" );
		p->AddTextBox( "name", Vector2i( 20, 20 ), 200, 20, "name_test" );
		p->AddTextBox( "group", Vector2i( 20, 100 ), 200, 20, "group_test" );
		p->AddTextBox( "bulletspeed", Vector2i( 20, 150 ), 200, 20, "10" );
		p->AddTextBox( "waitframes", Vector2i( 20, 200 ), 200, 20, "10" );
		//p->AddLabel( "label1", Vector2i( 20, 200 ), 30, "blah" );
		return p;
	}
	else if( name == "foottrap" )
	{
		Panel *p = new Panel( "foottrap_options", 200, 400, this );
		p->AddButton( "ok", Vector2i( 100, 300 ), Vector2f( 100, 50 ), "OK" );
		p->AddTextBox( "name", Vector2i( 20, 20 ), 200, 20, "name_test" );
		p->AddTextBox( "group", Vector2i( 20, 100 ), 200, 20, "group_test" );
		//p->AddLabel( "label1", Vector2i( 20, 200 ), 30, "blah" );
		return p;
	}
	else if( name == "map" )
	{
		Panel *p = new Panel( "map_options", 200, 400, this );
		p->AddButton( "ok", Vector2i( 100, 300 ), Vector2f( 100, 50 ), "OK" );
		p->AddLabel( "minedgesize_label", Vector2i( 20, 150 ), 20, "minimum edge size:" );
		p->AddTextBox( "minedgesize", Vector2i( 20, 20 ), 200, 20, "8" );
		return p;
	}
	return NULL;
}

int EditSession::CountSelectedPoints()
{
	int count = 0;
	for( list<TerrainPolygon*>::iterator it = selectedPolygons.begin(); it != selectedPolygons.end(); ++it )
	{
		for( PointList::iterator it2 = (*it)->points.begin(); it2 != (*it)->points.end(); ++it2 )
		{
			if( (*it2).second ) //selected
			{
				++count;
			}
		}
	}
	return count;
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
	edgePolygon->enemies.push_back( this );
	edgeIndex = eIndex;
	groundQuantity = edgeQuantity;

	image.setTexture( type->imageTexture );
	image.setOrigin( image.getLocalBounds().width / 2, image.getLocalBounds().height );
	
	//	image.setPosition( pos.x, pos.y );
	int testIndex = 0;

	Vector2i point;

	PointList::iterator prev = ground->points.end();
	prev--;
	PointList::iterator curr = ground->points.begin();

	for( ; curr != ground->points.end(); ++curr )
	{
		if( edgeIndex == testIndex )
		{
			V2d pr( (*prev).first.x, (*prev).first.y );
			V2d cu( (*curr).first.x, (*curr).first.y );

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
		int eIndex, double edgeQuantity, double bulletSpeed, int framesWait )
{
	type = t;
	ground = edgePolygon;
	edgePolygon->enemies.push_back( this );
	edgeIndex = eIndex;
	groundQuantity = edgeQuantity;

	image.setTexture( type->imageTexture );
	image.setOrigin( image.getLocalBounds().width / 2, image.getLocalBounds().height );
	
	//	image.setPosition( pos.x, pos.y );
	int testIndex = 0;

	Vector2i point;

	PointList::iterator prev = ground->points.end();
	prev--;
	PointList::iterator curr = ground->points.begin();

	for( ; curr != ground->points.end(); ++curr )
	{
		if( edgeIndex == testIndex )
		{
			V2d pr( (*prev).first.x, (*prev).first.y );
			V2d cu( (*curr).first.x, (*curr).first.y );

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

	ss << bulletSpeed;

	params.push_back( ss.str() ); 

	ss.str("");

	ss << framesWait;

	params.push_back( ss.str() );


	return "success";
}

std::string ActorParams::SetAsFootTrap( ActorType *t, TerrainPolygon *edgePolygon,
		int eIndex, double edgeQuantity )
{
	type = t;
	ground = edgePolygon;
	edgePolygon->enemies.push_back( this );
	edgeIndex = eIndex;
	groundQuantity = edgeQuantity;

	image.setTexture( type->imageTexture );
	image.setOrigin( image.getLocalBounds().width / 2, image.getLocalBounds().height );
	
	//	image.setPosition( pos.x, pos.y );
	int testIndex = 0;

	Vector2i point;

	PointList::iterator prev = ground->points.end();
	prev--;
	PointList::iterator curr = ground->points.begin();

	for( ; curr != ground->points.end(); ++curr )
	{
		if( edgeIndex == testIndex )
		{
			V2d pr( (*prev).first.x, (*prev).first.y );
			V2d cu( (*curr).first.x, (*curr).first.y );

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

std::string ActorParams::SetAsGoal( ActorType *t, TerrainPolygon *edgePolygon,
		int eIndex, double edgeQuantity )
{
	type = t;
	ground = edgePolygon;
	edgePolygon->enemies.push_back( this );
	edgeIndex = eIndex;
	groundQuantity = edgeQuantity;

	image.setTexture( type->imageTexture );
	image.setOrigin( image.getLocalBounds().width / 2, image.getLocalBounds().height );
	
	//	image.setPosition( pos.x, pos.y );
	int testIndex = 0;

	Vector2i point;

	PointList::iterator prev = ground->points.end();
	prev--;
	PointList::iterator curr = ground->points.begin();

	for( ; curr != ground->points.end(); ++curr )
	{
		if( edgeIndex == testIndex )
		{
			V2d pr( (*prev).first.x, (*prev).first.y );
			V2d cu( (*curr).first.x, (*curr).first.y );

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