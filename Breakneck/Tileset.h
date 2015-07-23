#ifndef __TILESET_H__
#define __TILESET_H__

#include <SFML/Graphics.hpp>

struct Tileset
{
	~Tileset();
	sf::IntRect GetSubRect( int localID );

	sf::Texture * texture;
	int tileWidth;
	int tileHeight;
	std::string sourceName;
};

#endif