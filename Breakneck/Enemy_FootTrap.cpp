#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>

using namespace std;
using namespace sf;

#define V2d sf::Vector2<double>

FootTrap::FootTrap( GameSession *owner, Edge *g, double q )
		:Enemy( owner, EnemyType::FOOTTRAP ), ground( g ), edgeQuantity( q ), dead( false )
{
	ts = owner->GetTileset( "foottrap.png", 48, 32 );
	sprite.setTexture( *ts->texture );
	
	V2d gPoint = g->GetPoint( edgeQuantity );
	position = gPoint;

	gn = g->Normal();
	angle = atan2( gn.x, -gn.y );

	sprite.setTextureRect( ts->GetSubRect( 0 ) );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height );
	sprite.setPosition( gPoint.x, gPoint.y );
	sprite.setRotation( angle / PI * 180 );

	
	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 10;
	hitboxInfo->drain = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 0;

	frame = 0;
	deathFrame = 0;
	animationFactor = 7;
	slowCounter = 1;
	slowMultiple = 1;

	spawnRect = sf::Rect<double>( gPoint.x - 24, gPoint.y - 24, 24 * 2, 24 * 2 );
}

void FootTrap::ResetEnemy()
{
	frame = 0;
	deathFrame = 0;
}

void FootTrap::HandleEntrant( QuadTreeEntrant *qte )
{
}

void FootTrap::UpdatePrePhysics()
{
	if( frame == 0 )
	{
		
	}
}

void FootTrap::UpdatePhysics()
{
}

void FootTrap::UpdatePostPhysics()
{
	if( !dead )
	{
		UpdateHitboxes();

		if( PlayerHitMe() )
		{
		//	cout << "patroller received damage of: " << receivedHit->damage << endl;
		//	owner->Pause( 20 );
		//	dead = true;
		//	receivedHit = NULL;
		}

		if( IHitPlayer() )
		{
		//	cout << "patroller just hit player for " << hitboxInfo->damage << " damage!" << endl;
		}
	}

	

	if( slowCounter == slowMultiple )
	{
		++frame;
		slowCounter = 1;
	
		if( dead )
		{
			deathFrame++;
		}

	}
	else
	{
		slowCounter++;
	}

	if( frame == 4 * animationFactor )
	{
		frame = 0;
	}

	if( deathFrame == 60 )
	{
		owner->RemoveEnemy( this );
	}

	UpdateSprite();
}

void FootTrap::Draw(sf::RenderTarget *target )
{
	target->draw( sprite );
}

bool FootTrap::IHitPlayer()
{
	Actor &player = owner->player;
	

	/*if( currBullet->hitBody.Intersects( player.hurtBody ) )
		{
			player.ApplyHit( bulletHitboxInfo );
			return true;
		}
	*/
	
	return false;
}

bool FootTrap::PlayerHitMe()
{
	return false;
}

bool FootTrap::PlayerSlowingMe()
{
	return false;
}

void FootTrap::UpdateSprite()
{
	sprite.setTextureRect( ts->GetSubRect( frame / animationFactor ) );
}

void FootTrap::DebugDraw(sf::RenderTarget *target)
{
}

void FootTrap::UpdateHitboxes()
{
}

bool FootTrap::ResolvePhysics( sf::Vector2<double> vel )
{
	return false;
}

void FootTrap::SaveEnemyState()
{
}

void FootTrap::LoadEnemyState()
{
}

