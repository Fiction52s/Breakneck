#ifndef __ENEMY_H__
#define __ENEMY_H__

#include "Actor.h"
#include <list>

struct Enemy : QuadTreeCollider
{
	Enemy( GameSession *owner );
	//virtual void HandleEdge( Edge *e ) = 0;
	virtual void HandleEntrant( QuadTreeEntrant *qte ) = 0;
	virtual void UpdatePrePhysics() = 0;
	virtual void UpdatePhysics() = 0;
	virtual void UpdatePostPhysics() = 0;
	virtual void Draw( sf::RenderTarget *target) = 0;
	virtual bool IHitPlayer() = 0;
	virtual void UpdateHitboxes() = 0;
	virtual bool PlayerHitMe() = 0;
	virtual void DebugDraw(sf::RenderTarget *target) = 0;
	Enemy *prev;
	Enemy *next;
	GameSession *owner;
	bool spawned;
	sf::Rect<double> spawnRect;
	HitboxInfo *receivedHit;
};

struct Patroller : Enemy
{
	Patroller( GameSession *owner, sf::Vector2i pos, std::list<sf::Vector2i> &path, bool loop, float speed );
	//void HandleEdge( Edge *e );
	void HandleEntrant( QuadTreeEntrant *qte );
	void UpdatePrePhysics();
	void UpdatePhysics();
	void UpdatePostPhysics();
	void Draw(sf::RenderTarget *target );
	void DebugDraw(sf::RenderTarget *target);
	bool IHitPlayer();
	bool PlayerHitMe();
	void UpdateSprite();
	void UpdateHitboxes();



	//std::list<sf::Vector2i> path;
	sf::Vector2i *path; //global
	int pathLength;
	bool loop;

	int targetNode;
	bool forward;


	double acceleration;
	double maxSpeed;
	int nodeWaitFrames;
	sf::Vector2<double> position;
	sf::Sprite sprite;
	Tileset *ts;
	CollisionBox hurtBody;
	CollisionBox hitBody;
	HitboxInfo *hitboxInfo;

	int hitlagFrames;
	int hitstunFrames;
};

struct Crawler : Enemy
{
	Crawler( GameSession *owner, Edge *ground, double quantity, bool clockwise, double speed );
//	void HandleEdge( Edge *e );
	void HandleEntrant( QuadTreeEntrant *qte );
	void UpdatePrePhysics();
	void UpdatePhysics();
	void UpdatePostPhysics();
	void Draw(sf::RenderTarget *target );
	bool IHitPlayer();
	bool PlayerHitMe();
	void UpdateSprite();
	void DebugDraw(sf::RenderTarget *target);
	void UpdateHitboxes();
	bool ResolvePhysics( sf::Vector2<double> vel );

	sf::Sprite sprite;
	Tileset *ts;

	bool clockwise;
	double groundSpeed;
	Edge *ground;
	sf::Vector2<double> offset;
	double edgeQuantity;

	CollisionBox hurtBody;
	CollisionBox hitBody;
	CollisionBox physBody;
	sf::Vector2<double> position;
	sf::Vector2<double> tempVel;

	Contact minContact;
	bool col;
	std::string queryMode;
	int possibleEdgeCount;
};

struct EnemyParentNode;

struct EnemyQNode
{
	EnemyQNode():parent(NULL),debug(NULL){}
	sf::Vector2<double> pos;
	double rw;
	double rh;
	sf::RenderWindow *debug;
	EnemyParentNode *parent;
	bool leaf;
};



struct EnemyParentNode : EnemyQNode
{
	EnemyParentNode( const sf::Vector2<double> &pos, double rw, double rh );
	EnemyQNode *children[4];
	// 0    |     1
	//--------------
	// 2    |     3
	
};

struct EnemyLeafNode : EnemyQNode
{
	int objCount;
	EnemyLeafNode( const sf::Vector2<double> &pos, double rw, double rh );
	Enemy *enemies[4];
};

EnemyQNode *Insert( EnemyQNode *node, Enemy* e );
//void Query( EnemyQNode *node, void (*f)( Enemy *e ) );

void DebugDrawQuadTree( sf::RenderWindow *rw, EnemyQNode *node );

struct EnemyQuadTreeCollider
{
	virtual void HandleEnemy( Enemy *e ) = 0;
};

void Query( EnemyQuadTreeCollider *qtc, EnemyQNode *node, const sf::Rect<double> &r );

bool IsEnemyTouchingBox( Enemy *e, const sf::Rect<double> & ir );




#endif