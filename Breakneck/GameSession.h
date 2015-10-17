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

struct GrassSegment
{
	GrassSegment( int edgeI, int grassIndex, int rep )
		:edgeIndex( edgeI ), index( grassIndex ), 
		reps (rep)
	{}
	int edgeIndex;
	int index;
	int reps;
};

struct Grass : QuadTreeEntrant
{
	sf::Vector2<double> A;
	sf::Vector2<double> B;
	sf::Vector2<double> C;
	sf::Vector2<double> D; 
	void HandleQuery( QuadTreeCollider * qtc );
	bool IsTouchingBox( const sf::Rect<double> &r );

	//bool prevGrass;
};

struct GameSession : QuadTreeCollider
{
	GameSession(GameController &c, sf::RenderWindow *rw, 
		sf::RenderTexture *preTex,
		sf::RenderTexture *miniTex);
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
	void UpdateTerrainShader( const sf::Rect<double> &aabb );
	void LevelSpecifics();
	void SetGroundPar();


	void DebugDrawActors();

	void HandleEntrant( QuadTreeEntrant *qte );
	void Pause( int frames );

	void GameStartMovie();

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

	sf::View view;

	void SaveState();
	void LoadState();

	const static int MAX_EFFECTS = 100;

	std::list<MovingTerrain*> movingPlats;

	sf::VertexArray groundPar;
	sf::Transform groundTrans;
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
	sf::RenderTexture *minimapTex;
	sf::Sprite background;
	sf::View bgView;

	sf::Sprite parTest;

	struct TestVA : QuadTreeEntrant
	{
		sf::VertexArray *va;
		sf::VertexArray *terrainVA;
		sf::VertexArray *grassVA;
		bool show;
		//TestVA *prev;
		TestVA *next;
		sf::Rect<double> aabb;
		void HandleQuery( QuadTreeCollider * qtc );
		bool IsTouchingBox( const sf::Rect<double> &r );
	};
	TestVA *listVA;
	std::string queryMode;
	QuadTree *borderTree;
	int numBorders;

	sf::Vector2f lastViewSize;
	sf::Vector2f lastViewCenter;
	//EdgeQNode *testTree;
	//EnemyQNode *enemyTree;
	std::string fileName;
	bool goalDestroyed;
	sf::View cloudView;
	Enemy *activeEnemyList;
	Enemy *pauseImmuneEffects;
	Enemy *cloneInactiveEnemyList;

	sf::Vector2<double> originalPos;
	sf::Rect<double> screenRect;
	sf::Rect<double> tempSpawnRect;

	QuadTree * terrainTree;
	QuadTree * enemyTree;
	QuadTree * lightTree;
	QuadTree * grassTree;
	

	bool usePolyShader;

	PowerBar powerBar;

	int pauseFrames;

	const static int MAX_LIGHTS_AT_ONCE = 16;
	int lightsAtOnce;
	Light *touchedLights[MAX_LIGHTS_AT_ONCE];
	int tempLightLimit;

	std::list<Light*> lights;
	Light *lightList;

	int deathWipeLength;
	int deathWipeFrame;
	bool deathWipe;

	sf::Texture wipeTextures[17];
	sf::Sprite wipeSprite;

	sf::View uiView;

	struct Sequence
	{
		//Sequence *next;
		//Sequence *prev;
		int frameCount;
		int frame;
		virtual bool Update() = 0;
		virtual void Draw( sf::RenderTarget *target ) = 0;
	};

	Sequence *activeSequence;

	struct GameStartSeq : Sequence
	{
		GameStartSeq(GameSession *owner);
		bool Update();
		void Draw( sf::RenderTarget *target );
		sf::Texture shipTex;
		sf::Sprite shipSprite;
		sf::Texture stormTex;
		sf::Sprite stormSprite;
		sf::VertexArray stormVA;
		sf::Vector2f startPos;
		GameSession *owner;
	};
	GameStartSeq *startSeq;


	struct Stored
	{
		Enemy *activeEnemyList;
	};
	Stored stored;

	//sf::Sprite healthSprite;

};



#endif