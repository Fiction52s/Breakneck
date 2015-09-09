#ifndef _GAMESESSION_H__
#define _GAMESESSION_H__

#include "Physics.h"
#include "Tileset.h"
#include <list>
#include "Actor.h"
#include "Enemy.h"
#include "QuadTree.h"
#include <SFML/Graphics.hpp>
#include "Light.h"
#include "Camera.h"


struct PowerBar
{
	PowerBar();
	int pointsPerLayer;
	int points;
	int layer;

	int maxLayer;
	int minUse;
	sf::Sprite panelSprite;
	sf::Texture panelTex;

	int maxRecover;
	int maxRecoverLayer;

	void Draw( sf::RenderTarget *target );
	bool Damage( int power );
    bool Use( int power );
	void Recover( int power );
	void Charge( int power );
};

struct GameSession : QuadTreeCollider
{
	GameSession(GameController &c, sf::RenderWindow *rw, 
		sf::RenderTexture *preTex );
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
	void RespawnPlayer();
	void ResetEnemies();
	void rReset( QNode *node );
	int CountActiveEnemies();

	void DebugDrawActors();

	void HandleEntrant( QuadTreeEntrant *qte );
	void Pause( int frames );

	void AllocateEffect();
	BasicEffect * ActivateEffect( 
		Tileset *ts, 
		sf::Vector2<double> pos, 
		bool pauseImmune,
		double angle, 
		int frameCount,
		int animationFactor,
		bool right );

	void DeactivateEffect( BasicEffect *be );
	BasicEffect *inactiveEffects;


	void SaveState();
	void LoadState();

	const static int MAX_EFFECTS = 100;

	std::list<MovingTerrain*> movingPlats;

	Camera cam;
	Actor player;
	sf::Shader polyShader;
	sf::Shader cloneShader;
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

	sf::RenderTexture *preScreenTex;

	struct TestVA : QuadTreeEntrant
	{
		sf::VertexArray *va;
		bool show;
		//TestVA *prev;
		TestVA *next;
		sf::Rect<double> aabb;
		void HandleQuery( QuadTreeCollider * qtc );
		bool IsTouchingBox( sf::Rect<double> &r );
	};
	TestVA *listVA;
	std::string queryMode;
	QuadTree *borderTree;
	int numBorders;

	sf::Vector2f lastViewSize;
	sf::Vector2f lastViewCenter;
	//EdgeQNode *testTree;
	//EnemyQNode *enemyTree;

	bool goalDestroyed;

	Enemy *activeEnemyList;
	Enemy *pauseImmuneEffects;
	Enemy *cloneInactiveEnemyList;

	sf::Vector2<double> originalPos;
	sf::Rect<double> screenRect;
	sf::Rect<double> tempSpawnRect;

	QuadTree * terrainTree;
	QuadTree * enemyTree;

	bool usePolyShader;

	PowerBar powerBar;

	int pauseFrames;

	std::list<Light*> lights;

	struct Stored
	{
		Enemy *activeEnemyList;
	};
	Stored stored;

	//sf::Sprite healthSprite;

};



#endif