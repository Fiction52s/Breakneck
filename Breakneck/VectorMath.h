#include <SFML/Graphics.hpp>

#ifndef __VECTOR_MATH_H__
#define __VECTOR_MATH_H__

double cross( sf::Vector2f a, sf::Vector2f b );


double length( sf::Vector2f v);

sf::Vector2f normalize( sf::Vector2f v );

double dot( sf::Vector2f a, sf::Vector2f b );

struct LineIntersection
{
	LineIntersection(const sf::Vector2f &pos, bool p );
	sf::Vector2f position;
	bool parallel;
};

LineIntersection lineIntersection( sf::Vector2f a, sf::Vector2f b, sf::Vector2f c, sf::Vector2f d );

#endif