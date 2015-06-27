//edit mode

#include "GUI.h"
#include "EditSession.h"
#include <fstream>
#include <assert.h>
#include <iostream>
#include "poly2tri/poly2tri.h"

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

	for( list<TerrainPolygon*>::iterator it = polygons.begin(); it != polygons.end(); ++it )
	{
		of << (*it)->material << " " << (*it)->points.size() << endl;
		for( list<Vector2i>::iterator it2 = (*it)->points.begin(); it2 != (*it)->points.end(); ++it2 )
		{
			of << (*it2).x << " " << (*it2).y << endl;
		}
	}
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
	GridSelector gs( 2, 2, 32, 32 );
	gs.active = true;
	Texture patrolTex;
	patrolTex.loadFromFile( "patroller.png" );
	Texture patrol2Tex;
	patrol2Tex.loadFromFile( "patroller2.png" );
	sf::Sprite s0( patrolTex );
	sf::Sprite s1( patrol2Tex );
	gs.Set( 0, 0, s0 );
	//gs.Set( 0, 1, s0 );
	gs.Set( 1, 1, s1 );
	//gs.Set( 1, 1, s1 );

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

	


	goalSprite.setOrigin( goalSprite.getLocalBounds().width / 2, goalSprite.getLocalBounds().height / 2 );

	playerSprite.setTextureRect( IntRect(0, 0, 64, 64 ) );
	playerSprite.setOrigin( playerSprite.getLocalBounds().width / 2, playerSprite.getLocalBounds().height / 2 );

	w->setVerticalSyncEnabled( false );

	OpenFile( fileName );
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

	

	bool s = sf::Keyboard::isKeyPressed( sf::Keyboard::T );
	
	

	Emode mode = CREATE_POLYGONS;
	Emode stored = mode;
	bool canCreatePoint = true;


	while( !quit )
	{
		w->clear();
		 prevWorldPos = worldPos;
		 pixelPos = sf::Mouse::getPosition( *w );
		 Vector2f tempWorldPos = w->mapPixelToCoords(pixelPos);
		 worldPos.x = tempWorldPos.x;
		 worldPos.y = tempWorldPos.y;

		sf::Event ev;
		
		while( w->pollEvent( ev ) )
		{
			switch( ev.type )
			{
			case Event::MouseButtonPressed:
				{
					if( ev.mouseButton.button == Mouse::Left )
					{
						mode = CREATE_ENEMY;
						if( mode == CREATE_ENEMY && gs.active )
						{

							//gs.Update( true, worldPos.x - view.getCenter().x, worldPos.y - view.getCenter().y );//pixelPos.x - gs.controlSprite., pixelPos.y - w->getSize().y / 2 );
							gs.Update( true, pixelPos.x, pixelPos.y );
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
						mode = CREATE_ENEMY;
						if( mode == CREATE_ENEMY && gs.active )
						{

							//gs.Update( true, worldPos.x - view.getCenter().x, worldPos.y - view.getCenter().y );//pixelPos.x - gs.controlSprite., pixelPos.y - w->getSize().y / 2 );
							if( gs.Update( false, pixelPos.x, pixelPos.y ) )
							{
								cout << "selected enemy index: " << gs.focusX << ", " << gs.focusY << endl;
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
					if( mode != PAUSED && mode != PLACE_GOAL && mode != PLACE_PLAYER )
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
			}
		}

		if( quit )
			break;
		testPoint.x = worldPos.x;
		testPoint.y = worldPos.y;

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


		 if( canCreatePoint && Mouse::isButtonPressed( Mouse::Left ) )
		{
			if( mode == CREATE_POLYGONS && !panning )
			{
				//testPoint.x = worldPos.x;
				//testPoint.y = worldPos.y;


				

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
						//		//cout << "point valid" << endl;
						//		polygonInProgress->points.push_back( worldi  );
						//		for( list<TerrainPolygon*>::iterator it = polygons.begin(); it != polygons.end(); ++it )
						//		{
						//			//(*it)->SetSelected( false );					
						//		}
						//	}
						//	//else
						//		//cout << "INVALID" << endl;
						//	
						//}
						//else
						//{
						//	bool okay = true;
						//	for( list<TerrainPolygon*>::iterator it = polygons.begin(); it != polygons.end(); ++it )
						//	{
						//		if( (*it)->ContainsPoint( Vector2f(worldi.x, worldi.y) ) )
						//		{
						//			okay = false;
						//			break;
						//		}

						//	//	(*it)->SetSelected( false );					
						//	}
						//	if( okay)
						//	polygonInProgress->points.push_back( worldi  );
						//
						//}
						

					}
				}
				else
				{
					//polygonInProgress->points.clear();
				}
			}
		}

		if( panning )
		{
			Vector2<double> temp = panAnchor - worldPos;
			view.move( Vector2f( temp.x, temp.y ) );
		}
		
		


		if( mode == PLACE_PLAYER )
		{
			playerSprite.setPosition( w->mapPixelToCoords(sf::Mouse::getPosition( *w )) );
			//cout << "placing: " << playerSprite.getPosition().x << ", " << playerSprite.getPosition().y << endl;
		}
		else
			playerSprite.setPosition( playerPosition.x, playerPosition.y );

		if( mode == PLACE_GOAL )
		{
			goalSprite.setPosition( w->mapPixelToCoords( sf::Mouse::getPosition( *w )) );
		}
		else
			goalSprite.setPosition( goalPosition.x, goalPosition.y );
		
		//canCreatePoint = true;

	

		w->setView( view );
		w->draw(border, 8, sf::Lines);

		Draw();

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
				//Vertex *p[polygonInProgress->points.size()]
			}
			
			
		}

		iconSprite.setScale( view.getSize().x / 960.0, view.getSize().y / 540.0 );
		iconSprite.setPosition( view.getCenter().x + 200 * iconSprite.getScale().x, view.getCenter().y - 250 * iconSprite.getScale().y );
		

		w->draw( playerSprite );
		w->draw( goalSprite );
		w->draw( iconSprite );

		if( sf::Keyboard::isKeyPressed( Keyboard::H ) )
		{
			alphaTextSprite.setScale( .5 * view.getSize().x / 960.0, .5 * view.getSize().y / 540.0 );
			alphaTextSprite.setOrigin( alphaTextSprite.getLocalBounds().width / 2, alphaTextSprite.getLocalBounds().height / 2 );
			alphaTextSprite.setPosition( view.getCenter().x, view.getCenter().y );
			w->draw( alphaTextSprite );
		}

		
		sf::View uiView( sf::Vector2f( 480, 270 ), sf::Vector2f( 960, 540 ) );
		w->setView( uiView );

		gs.Draw( w );

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

