#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>

using namespace std;
using namespace sf;

#define V2d sf::Vector2<double>

Enemy::Enemy( GameSession *own )
	:owner( own ), prev( NULL ), next( NULL ), spawned( false ), slowMultiple( 1 ), slowCounter( 1 )
{

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



