#ifndef __LIGHT_H__
#define __LIGHT_H__

#include <SFML/Graphics.hpp>
#include "QuadTree.h"

struct GameSession;
struct Light : QuadTreeEntrant
{
	Light( GameSession *owner, sf::Vector2i &p, sf::Color &c );
	Light * next;
	sf::Shader sh;
	sf::CircleShape cs;
	GameSession *owner;
	void Draw( sf::RenderTarget *target );

	sf::Vector2i pos;
	sf::Color color;

	void HandleQuery( QuadTreeCollider * qtc );
	bool IsTouchingBox( sf::Rect<double> &r );
};

#endif