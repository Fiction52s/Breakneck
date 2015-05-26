#include "VectorMath.h"

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
	return abs( a - b ) < .001;
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

