#include "VectorMath.h"

using namespace std;
using namespace sf;

double cross( sf::Vector2f a, sf::Vector2f b )
{
	double ax = a.x;
	double ay = a.y;
	double bx = b.x;
	double by = b.y;
	return ax * by - ay * bx;
	//return a.x * b.y - a.y * b.x;
}


double length( sf::Vector2f v)
{
	double vx = v.x;
	double vy = v.y;
	return sqrt( vx * vx + vy * vy );
}

sf::Vector2f normalize( sf::Vector2f v )
{
	double vLen = length( v );
	if( vLen > 0 )
		return sf::Vector2f( v.x / vLen, v.y / vLen );
	else
		return sf::Vector2f( 0, 0 );
}

double dot( sf::Vector2f a, sf::Vector2f b )
{
	double ax = a.x;
	double ay = a.y;
	double bx = b.x;
	double by = b.y;
	return ax * bx + ay * by;
}


LineIntersection::LineIntersection(const sf::Vector2f &pos, bool p )
{
	position = pos;
	parallel = p;
}

LineIntersection lineIntersection( sf::Vector2f a, Vector2f b, Vector2f c, Vector2f d )
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

	return LineIntersection( sf::Vector2f(x,y), parallel );
}

