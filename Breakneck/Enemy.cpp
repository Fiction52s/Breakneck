#include "Enemy.h"
#include "GameSession.h"

Enemy::Enemy( GameSession *own )
	:owner( own ), prev( NULL ), next( NULL ), spawned( false )
{

}

Patroller::Patroller( GameSession *owner )
	:Enemy( owner )
{
	ts = owner->GetTileset( "patroller.png", 32, 32 );
	sprite.setTexture( *ts->texture );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2 );
	position.x = 0;
	position.y = 0;
	hurtBody.type = CollisionBox::Hurt;
	hurtBody.isCircle = true;
	hurtBody.offsetAngle = 0;
	hurtBody.offset.x = 0;
	hurtBody.offset.y = 0;
	hurtBody.rw = 16;
	hurtBody.rh = 16;

	hitBody.type = CollisionBox::Hit;
	hitBody.isCircle = true;
	hitBody.offsetAngle = 0;
	hitBody.offset.x = 0;
	hitBody.offset.y = 0;
	hitBody.rw = 16;
	hitBody.rh = 16;
}

void Patroller::HandleEdge( Edge *e )
{
}

void Patroller::UpdatePrePhysics()
{
}

void Patroller::UpdatePhysics()
{
}

void Patroller::UpdatePostPhysics()
{
	UpdateSprite();
}

void Patroller::UpdateSprite()
{
	sprite.setPosition( position.x, position.y );
}

void Patroller::Draw( sf::RenderTarget *target )
{
	target->draw( sprite );
}



bool Patroller::IHitPlayer()
{
	
	if( hitBody
	Actor &player = owner->player;
	return false;
}

bool Patroller::PlayerHitMe()
{
	Actor &player = owner->player;
	return false;
}

