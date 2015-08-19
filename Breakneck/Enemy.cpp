#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>

using namespace std;
using namespace sf;

#define V2d sf::Vector2<double>

Enemy::Enemy( GameSession *own )
	:owner( own ), prev( NULL ), next( NULL ), spawned( false ), slowMultiple( 1 ), slowCounter( 1 ),
	spawnedByClone( false )
{

}

void Enemy::Reset()
{
	slowMultiple = 1;
	slowCounter = 1;
	spawned = false;
	prev = NULL;
	next = NULL;
	spawnedByClone = false;

	ResetEnemy();
}

void Enemy::HandleQuery( QuadTreeCollider * qtc )
{
	if( !spawned )
		qtc->HandleEntrant( this );
}

bool Enemy::IsTouchingBox( sf::Rect<double> &r )
{
	return IsBoxTouchingBox( spawnRect, r );//r.intersects( spawnRect );// 
}

void Enemy::SaveState()
{
	stored.next = next;
	stored.prev = prev;
	stored.receivedHit = receivedHit;
	stored.slowCounter = slowCounter;
	stored.slowMultiple = slowMultiple;
	stored.spawned = spawned;

	SaveEnemyState();
}

void Enemy::LoadState()
{
	next = stored.next;
	prev = stored.prev;
	receivedHit = stored.receivedHit;
	slowCounter = stored.slowCounter;
	slowMultiple = stored.slowMultiple;
	spawned = stored.spawned;

	LoadEnemyState();
}




