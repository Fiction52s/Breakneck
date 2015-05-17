//edit mode
#include "EditSession.h"
#include <fstream>
#include <assert.h>
#include <iostream>
#include "poly2tri/poly2tri.h"

using namespace std;
using namespace sf;



Polygon::Polygon()
{
	va = NULL;
	lines = NULL;
	selected = false;
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

	rt->draw( *va );
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
        Vector2i first, second;
        if (i == 0)
        {
            first = pointArray[points.size() - 1];
        }
        else
        {
            first = pointArray[i - 1];

        }
        second = pointArray[i];

        sum += (second.x - first.x) * (second.y + first.y);
    }

	delete [] pointArray;

	//cout << "summm: " << sum << endl;
    return sum < 0;
}

EditSession::EditSession( RenderWindow *wi)
	:w( wi ), zoomMultiple( 1 )
{
}

void EditSession::Draw()
{
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

	//	cs.setPosition( 0, 0 );
	//	w->draw( cs );
		
	}

	for( list<Polygon*>::iterator it = polygons.begin(); it != polygons.end(); ++it )
	{
		(*it)->Draw( w );
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

void EditSession::Run( string fileName )
{
	Color testColor( 0x75, 0x70, 0x90 );
	View view( sf::Vector2f( 300, 300 ), sf::Vector2f( 960, 540 ) );
	w->setView( view );
	Texture playerTex;
	playerTex.loadFromFile( "stand.png" );
	sf::Sprite playerSprite( playerTex );
	playerSprite.setTextureRect( IntRect(0, 0, 64, 64 ) );
	playerSprite.setOrigin( playerSprite.getLocalBounds().width / 2, playerSprite.getLocalBounds().height / 2 );

	w->setVerticalSyncEnabled( false );

	OpenFile( fileName );
	//Vector2f vs(  );
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
	double minimumEdgeLength = 8;

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
			if( mode == "neutral" )
			{
				bool emptySpace = true;
				for( list<Polygon*>::iterator it = polygons.begin(); it != polygons.end(); ++it )
				{
					if((*it)->ContainsPoint( Vector2f(worldPos.x, worldPos.y ) ) )
					{
						emptySpace = false;
						//(*it)->selected = !((*it)->selected);
						//cout << "Point in polygon!!!   " << (*it) << endl;
						break;
					}
				}

				if( emptySpace )
				{
					if( length( worldPos - Vector2<double>(polygonInProgress->points.back().x, polygonInProgress->points.back().y )  ) >= minimumEdgeLength * zoomMultiple )
					{
						Vector2i worldi( worldPos.x, worldPos.y );
						polygonInProgress->points.push_back( worldi  );

					}
				}
				else
				{
					polygonInProgress->points.clear();
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
				//zoomMultiple -= ev.mouseWheel.delta * 10; //( prevDelta - ev.mouseWheel.delta )/ 100.f;
				if( zoomMultiple < 1 )
				{
					zoomMultiple = 1;
				}
				else if( zoomMultiple > 65536 )
				{
					zoomMultiple = 65536;
				}
			
				Vector2<double> ff = Vector2<double>(view.getCenter().x, view.getCenter().y );//worldPos - ( - (  .5f * view.getSize() ) );
				view.setSize( Vector2f( 960 * (zoomMultiple), 540 * ( zoomMultiple ) ) );
				w->setView( view );
				Vector2f newWorldPosTemp = w->mapPixelToCoords(pixelPos);
				Vector2<double> newWorldPos( newWorldPosTemp.x, newWorldPosTemp.y );
				Vector2<double> tempCenter = ff + ( worldPos - newWorldPos );
				view.setCenter( tempCenter.x, tempCenter.y );
				w->setView( view );
				//cout << "altering dleta: " << ev.mouseWheel.delta << endl;
			}
		
		}
		else if( ev.type == Event::MouseMoved )
		{
			
			if( Mouse::isButtonPressed( Mouse::Middle ) )
			{
				//Vector2<double> fff(prevWorldPos - w->mapPixelToCoords(Vector2i(ev.mouseMove.x, ev.mouseMove.y )));
				//if( length( move ) > 
				//view.move( fff );
				//view.setCenter( w->mapPixelToCoords(Mouse::getPosition( *w ) ) );//Vector2i(ev.mouseMove.x, ev.mouseMove.y )) );
				//cout << "blah: " << fff.x << ", " << fff.y << endl; 
				//w->setView( view );
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
					for( list<Polygon*>::iterator it = polygons.begin(); it != polygons.end(); ++it )
					{
						if((*it)->ContainsPoint( Vector2f(worldPos.x, worldPos.y ) ) )
						{
							emptySpace = false;
							//(*it)->selected = !(*it)->selected;
							(*it)->SetSelected( !((*it)->selected ) );

							break;
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

		if( sf::Keyboard::isKeyPressed( sf::Keyboard::T ) )
		{
			quit = true;
		}
		else if( sf::Keyboard::isKeyPressed( sf::Keyboard::B) )
		{
			if( mode == "neutral" )
			{
				mode = "set player";
				polygonInProgress->points.clear();
			}
		}
		else if( sf::Keyboard::isKeyPressed( sf::Keyboard::Delete ) )
		{
			list<Polygon*>::iterator it = polygons.begin();
			while( it != polygons.end() )
			{
				if( (*it)->selected )
					polygons.erase( it++ );
				else
					++it;
			/*	else
				{
					++it;
				}*/
			}
		}
		else if( sf::Keyboard::isKeyPressed( sf::Keyboard::Space ) )
		{
			//cout << "mode: set player" << endl;
			if( mode == "neutral" && polygonInProgress->points.size() > 2 )
			{
				polygonInProgress->Finalize();
				polygons.push_back( polygonInProgress );
				polygonInProgress = new Polygon();
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
				//cin >> fName;
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
			cout << "placing: " << playerSprite.getPosition().x << ", " << playerSprite.getPosition().y << endl;
		}
		else
			playerSprite.setPosition( playerPosition.x, playerPosition.y );


		//cout << zoomMultiple << endl;
		w->setView( view );
		w->draw(border, 8, sf::Lines);
		Draw();
		w->draw( playerSprite );
		w->display();
	}
	
	
	
}


