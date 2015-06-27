#include "Actor.h"
#include <list>


#ifndef __ENEMY_H__
#define __ENEMY_H__

struct Enemy : QuadTreeCollider
{
	Enemy( GameSession *owner );
	virtual void HandleEdge( Edge *e ) = 0;
	virtual void UpdatePrePhysics() = 0;
	virtual void UpdatePhysics() = 0;
	virtual void UpdatePostPhysics() = 0;
	virtual void Draw() = 0;
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
	void Draw();
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



#endif