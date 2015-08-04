#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>

using namespace std;
using namespace sf;

#define V2d sf::Vector2<double>

BasicTurret::BasicTurret( GameSession *owner, Edge *g, double q, 
		int betweenFiring )
		:Enemy( owner ), framesBetweenFiring( betweenFiring ), firingCounter( 0 ), ground( g ),
		edgeQuantity( q )
{
	ts = owner->GetTileset( "basicturret.png", 48, 48 );
	sprite.setTexture( *ts->texture );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height /2 );
	V2d gPoint = g->GetPoint( edgeQuantity );
	sprite.setPosition( gPoint.x, gPoint.y );

	V2d gn = g->Normal();
	angle = atan2( gn.x, -gn.y );

	sprite.setTextureRect( ts->GetSubRect( 0 ) );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height );
	//V2d gPoint = ground->GetPoint( edgeQuantity );
	sprite.setPosition( gPoint.x, gPoint.y );
	sprite.setRotation( angle / PI * 180 );



	spawnRect = sf::Rect<double>( gPoint.x - 24, gPoint.y - 24, gPoint.x + 24, gPoint.y + 24 );
}

void BasicTurret::HandleEntrant( QuadTreeEntrant *qte )
{
}

void BasicTurret::UpdatePrePhysics()
{
}

void BasicTurret::UpdatePhysics()
{
}

void BasicTurret::UpdatePostPhysics()
{
	UpdateSprite();
}

void BasicTurret::Draw(sf::RenderTarget *target )
{
	target->draw( sprite );
}

bool BasicTurret::IHitPlayer()
{
	return false;
}

bool BasicTurret::PlayerHitMe()
{
	return false;
}

bool BasicTurret::PlayerSlowingMe()
{
	return false;
}

void BasicTurret::UpdateSprite()
{
	
}

void BasicTurret::DebugDraw(sf::RenderTarget *target)
{
}

void BasicTurret::UpdateHitboxes()
{
}

bool BasicTurret::ResolvePhysics( sf::Vector2<double> vel )
{
	return false;
}