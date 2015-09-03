#ifndef __LIGHT_H__
#define __LIGHT_H__

#include <SFML/Graphics.hpp>

struct GameSession;
struct Light
{
	Light( GameSession *owner );
	sf::Shader sh;
	sf::CircleShape cs;
	GameSession *owner;
	void Draw( sf::RenderTarget *target );
};

#endif