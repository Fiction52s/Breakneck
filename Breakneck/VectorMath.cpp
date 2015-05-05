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
