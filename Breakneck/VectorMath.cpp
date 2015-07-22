#include "VectorMath.h"
#include <iostream>

using namespace std;
using namespace sf;

double cross( sf::Vector2<double> a, sf::Vector2<double> b )
{
	double ax = a.x;
	double ay = a.y;
	double bx = b.x;
	double by = b.y;
	return ax * by - ay * bx;
	//return a.x * b.y - a.y * b.x;
}


double length( sf::Vector2<double> v)
{
	double vx = v.x;
	double vy = v.y;
	return sqrt( vx * vx + vy * vy );
}

sf::Vector2<double> normalize( sf::Vector2<double> v )
{
	double vLen = length( v );
	if( vLen > 0 )
		return sf::Vector2<double>( v.x / vLen, v.y / vLen );
	else
		return sf::Vector2<double>( 0, 0 );
}

double dot( sf::Vector2<double> a, sf::Vector2<double> b )
{
	double ax = a.x;
	double ay = a.y;
	double bx = b.x;
	double by = b.y;
	return ax * bx + ay * by;
}

bool approxEquals( double a, double b )
{
	//before was .00001. testing for rounding errors
	return abs( a - b ) < .000001;
}


LineIntersection::LineIntersection(const sf::Vector2<double> &pos, bool p )
{
	position = pos;
	parallel = p;
}


LineIntersection lineIntersection( sf::Vector2<double> a, Vector2<double> b, Vector2<double> c, Vector2<double> d )
{
	double ax = a.x;
	double ay = a.y;
	double bx = b.x;
	double by = b.y;
	double cx = c.x;
	double cy = c.y;
	double dx = d.x;
	double dy = d.y;

	double x= 0,y = 0;
	bool parallel = false;
	if( (ax-bx)*(cy - dy) - (ay - by) * (cx - dx ) == 0 )
	{
		parallel = true;
	}
	else
	{
		x = ((ax * by - ay * bx ) * ( cx - dx ) - (ax - bx ) * ( cx * dy - cy * dx ))
			/ ( (ax-bx)*(cy - dy) - (ay - by) * (cx - dx ) );
		y = ((ax * by - ay * bx ) * ( cy - dy ) - (ay - by ) * ( cx * dy - cy * dx ))
			/ ( (ax-bx)*(cy - dy) - (ay - by) * (cx - dx ) );
	}

	return LineIntersection( sf::Vector2<double>(x,y), parallel );
}

LineIntersection SegmentIntersect( sf::Vector2<double> a, 
	sf::Vector2<double> b, sf::Vector2<double> c, 
	sf::Vector2<double> d )
{
	LineIntersection li = lineIntersection( sf::Vector2<double>( a.x, a.y ), sf::Vector2<double>( b.x, b.y ), 
				sf::Vector2<double>( c.x, c.y ), sf::Vector2<double>( d.x, d.y ) );
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
		//cout << "e1left: " << e1Left << ", e1Right: " << e1Right << ", e1Top: " << e1Top << ", e1Bottom: " << e1Bottom << endl;
	//	cout << "e2left: " << e2Left << ", e2Right: " << e2Right << ", e2Top: " << e2Top << ", e2Bottom: " << e2Bottom << endl;
	//	cout << "li: " << li.position.x << ", " << li.position.y << endl;

		//cout << "compares: " << e1Left << ", " << e2Right << " .. " << e1Right << ", " << e2Left << endl;
		//cout << "compares y: " << e1Top << " <= " << e2Bottom << " && " << e1Bottom << " >= " << e2Top << endl;
		if( e1Left <= e2Right && e1Right >= e2Left && e1Top <= e2Bottom && e1Bottom >= e2Top )
		{
			//cout << "---!!!!!!" << endl;
			if( li.position.x <= e1Right + .0001 && li.position.x >= e1Left - .0001 && li.position.y >= e1Top - .0001 && li.position.y <= e1Bottom + .0001)
			{
				if( li.position.x <= e2Right + .0001 && li.position.x >= e2Left - .0001 && li.position.y >= e2Top - .0001 && li.position.y <= e2Bottom + .0001)
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