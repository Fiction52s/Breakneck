#include "Physics.h"
#include "Tileset.h"
#include <list>
#include "Actor.h"
#ifndef _GAMESESSION_H__
#define _GAMESESSION_H__

struct GameSession
{
	GameSession(GameController &c, sf::RenderWindow *rw);
	~GameSession();
	int Run( std::string fileName );
	bool OpenFile( std::string fileName );
	sf::RenderWindow *window;
	std::string currentFile;
	std::list<Tileset*> tilesetList;
	Tileset * GetTileset( const std::string & s,
		int tileWidth, int tileHeight );
	Actor player;
	sf::Shader polyShader;
	Edge **edges;
	sf::Vector2<double> *points;
	int numPoints;
	sf::VertexArray *va;
	ControllerState prevInput;
	ControllerState currInput;
	GameController &controller;
	Collider coll;
	std::list<sf::VertexArray*> polygons;
	sf::Vector2f lastViewSize;
	sf::Vector2f lastViewCenter;
	sf::Sprite goalSprite;
	sf::Texture goalTex;
};

#endif