#include "Actor.h"
#include <list>


#ifndef __ENEMY_H__
#define __ENEMY_H__

struct Enemy : EdgeQuadTreeCollider
{
	Enemy( GameSession *owner );
	virtual void HandleEdge( Edge *e ) = 0;
	virtual void UpdatePrePhysics() = 0;
	virtual void UpdatePhysics() = 0;
	virtual void UpdatePostPhysics() = 0;
	virtual void Draw( sf::RenderTarget *target) = 0;
	virtual bool IHitPlayer() = 0;
	virtual bool PlayerHitMe() = 0;
	Enemy *prev;
	Enemy *next;
	GameSession *owner;
	bool spawned;
	sf::Rect<double> spawnRect;
};

struct Patroller : Enemy
{
	Patroller( GameSession *owner );
	void HandleEdge( Edge *e );
	void UpdatePrePhysics();
	void UpdatePhysics();
	void UpdatePostPhysics();
	void Draw(sf::RenderTarget *target );
	bool IHitPlayer();
	bool PlayerHitMe();
	void UpdateSprite();
	std::list<sf::Vector2<double>> nodes;
	double acceleration;
	double maxSpeed;
	int nodeWaitFrames;
	sf::Vector2<double> position;
	sf::Sprite sprite;
	Tileset *ts;
	CollisionBox hurtBody;
	CollisionBox hitBody;
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