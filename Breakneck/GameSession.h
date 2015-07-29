#ifndef _GAMESESSION_H__
#define _GAMESESSION_H__

#include "Physics.h"
#include "Tileset.h"
#include <list>
#include "Actor.h"
#include "Enemy.h"


struct GameSession : EnemyQuadTreeCollider
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
	void Test( Edge *e );
	void AddEnemy( Enemy * e );
	void RemoveEnemy( Enemy * e );
	void UpdateEnemiesPrePhysics();
	void UpdateEnemiesPhysics();
	void UpdateEnemiesPostPhysics();
	void UpdateEnemiesSprites();
	void UpdateEnemiesDraw();

	void DebugDrawActors();

	void HandleEnemy( Enemy *e );

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
	std::list<sf::VertexArray*> polygonBorders;
	sf::Vector2f lastViewSize;
	sf::Vector2f lastViewCenter;
	sf::Sprite goalSprite;
	sf::Texture goalTex;
	EdgeQNode *testTree;
	EnemyQNode *enemyTree;

	Enemy *activeEnemyList;
	Enemy *inactiveEnemyList;

	sf::Vector2<double> originalPos;
	sf::Rect<double> screenRect;
};



#endif