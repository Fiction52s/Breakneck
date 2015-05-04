//edit mode
#include "EditSession.h"
#include <fstream>
#include <assert.h>
#include <iostream>

using namespace std;
using namespace sf;

double length1( Vector2f v)
{
	double vx = v.x;
	double vy = v.y;
	return sqrt( vx * vx + vy * vy );
}

void Polygon::Finalize()
{
	material = "mat";
	lines = new sf::Vertex[points.size()*2+1];
	int i = 0;

	FixWinding();
	//cout << "points size: " << points.size() << endl;
	if( points.size() > 0 )
	{
		list<Vector2f>::iterator it = points.begin(); 
		lines[0] = (*it);
		lines[2 * points.size() - 1 ] = (*it);
		++it;
		++i;
		while( it != points.end() )
		{
			lines[i] = (*it);
			lines[++i] = (*it); 
			++i;
			++it;
		}
	}

	
	/*for( ; it != points.end(); ++it )
	{
		lines[i] = sf::Vertex((*it));
		lines[i*2+1] = sf::Vertex((*it));
		++i;
		
	}*/
}

void Polygon::Draw( RenderTarget *rt )
{
	rt->draw(lines, points.size()*2, sf::Lines );
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
        list<Vector2f> temp;
		for( list<Vector2f>::iterator it = points.begin(); it != points.end(); ++it )
		{
			temp.push_front( (*it) );
		}
		points.clear();
		for( list<Vector2f>::iterator it = temp.begin(); it != temp.end(); ++it )
		{
			points.push_back( (*it) );
		}          
    }
}

bool Polygon::IsClockwise()
{
	assert( points.size() > 0 );


    Vector2f *pointArray = new Vector2f[points.size()];

	int i = 0;
	for( list<Vector2f>::iterator it = points.begin(); it != points.end(); ++it )
	{
		pointArray[i] = (*it);
		++i;
	}

    int sum = 0;
	for (int i = 0; i < points.size(); ++i)
    {
        Vector2f first, second;
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
    return sum < 0;
}

EditSession::EditSession( RenderWindow *wi)
	:w( wi )
{
}

void EditSession::Draw()
{
	int psize = polygonInProgress->points.size();
	if( psize > 0 )
	{
		CircleShape cs;
		cs.setRadius( 10 );
		cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
		cs.setFillColor( Color::Green );

		
		for( list<Vector2f>::iterator it = polygonInProgress->points.begin(); it != polygonInProgress->points.end(); ++it )
		{
			cs.setPosition( (*it) );
			w->draw( cs );
		}

		cs.setPosition( 0, 0 );
		w->draw( cs );
		
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
			float x,y;
			for( int i = 0; i < polyPoints; ++i )
			{
				is >> x;
				is >> y;
				poly->points.push_back( Vector2f( x,y ) );
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
		for( list<Vector2f>::iterator it2 = (*it)->points.begin(); it2 != (*it)->points.end(); ++it2 )
		{
			of << (*it2).x << " " << (*it2).y << endl;
		}
	}
}

void EditSession::Run( string fileName )
{
	View view( sf::Vector2f(  300, 300 ), sf::Vector2f( 960, 540 ) );
	w->setView( view );
	Texture playerTex;
	playerTex.loadFromFile( "stand.png" );
	sf::Sprite playerSprite( playerTex );
	playerSprite.setTextureRect( IntRect(0, 0, 64, 64 ) );
	playerSprite.setOrigin( playerSprite.getLocalBounds().width / 2, playerSprite.getLocalBounds().height / 2 );

	w->setVerticalSyncEnabled( false );

	OpenFile( fileName );

	view.setCenter( playerPosition );

	mode = "neutral";
	bool quit = false;
	polygonInProgress = new Polygon();
	float zoomMultiple = 1;
	Vector2f prevWorldPos;
	Vector2i pixelPos;
	Vector2f worldPos = w->mapPixelToCoords(sf::Mouse::getPosition( *w ));
	bool panning = false;
	Vector2f panAnchor;
	bool backspace = true;
	double minimumEdgeLength = 16;


	while( !quit )
	{
		w->clear();
		 prevWorldPos = worldPos;
		 pixelPos = sf::Mouse::getPosition( *w );
		 worldPos = w->mapPixelToCoords(pixelPos);

		if( Mouse::isButtonPressed( Mouse::Left ) )
		{
			if( mode == "neutral" )
			{
				if( length1( worldPos - polygonInProgress->points.back() ) >= minimumEdgeLength)
					polygonInProgress->points.push_back( worldPos );
			}
			else if( mode == "set player" )
			{
				playerPosition = worldPos;
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
			
				Vector2f ff = view.getCenter();//worldPos - ( - (  .5f * view.getSize() ) );
				view.setSize( Vector2f( 960 * (zoomMultiple), 540 * ( zoomMultiple ) ) );
				w->setView( view );
				Vector2f newWorldPos = w->mapPixelToCoords(pixelPos);
				view.setCenter( ff + ( worldPos - newWorldPos ) );
				w->setView( view );
				//cout << "altering dleta: " << ev.mouseWheel.delta << endl;
			}
		
		}
		else if( ev.type == Event::MouseMoved )
		{
			if( Mouse::isButtonPressed( Mouse::Middle ) )
			{
				//Vector2f fff(prevWorldPos - w->mapPixelToCoords(Vector2i(ev.mouseMove.x, ev.mouseMove.y )));
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
		}
		}

		if( panning )
		{

			view.move( panAnchor - worldPos );
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
		else if( sf::Keyboard::isKeyPressed( sf::Keyboard::Space ) )
		{
			//cout << "mode: set player" << endl;
			if( mode == "neutral" && polygonInProgress->points.size() > 2 )
			{
				polygonInProgress->Finalize();
				polygons.push_back( polygonInProgress );
				polygonInProgress = new Polygon();
			}

			if( polygonInProgress->points.size() <= 2 )
			{
				cout << "cant finalize. cant make polygon" << endl;
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
		}
		else
			playerSprite.setPosition( playerPosition );

		w->setView( view );
		Draw();
		w->draw( playerSprite );
		w->display();
	}
	
	
	
}


