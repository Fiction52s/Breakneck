#include <SFML/Graphics.hpp>

#ifndef __VECTOR_MATH_H__
#define __VECTOR_MATH_H__

double cross( sf::Vector2<double> a, sf::Vector2<double> b );

#define PI 3.14159265359
double length( sf::Vector2<double> v);

sf::Vector2<double> normalize( sf::Vector2<double> v );

double dot( sf::Vector2<double> a, sf::Vector2<double> b );

struct LineIntersection
{
	LineIntersection(const sf::Vector2<double> &pos, bool p );
	sf::Vector2<double> position;
	bool parallel;
};

bool approxEquals( double a, double b )
{
	return abs( a - b ) < .00001;
}

LineIntersection lineIntersection( sf::Vector2<double> a, sf::Vector2<double> b, sf::Vector2<double> c, sf::Vector2<double> d );



#endif