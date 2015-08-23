#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>

using namespace std;
using namespace sf;

#define V2d sf::Vector2<double>

Goal::Goal( GameSession *owner, Edge *g, double q )
		:Enemy( owner, EnemyType::GOAL ), ground( g ), edgeQuantity( q ), dead( false )
{
	ts = owner->GetTileset( "goal.png", 96, 32 );
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

	hurtBody.type = CollisionBox::Hurt;
	hurtBody.isCircle = true;
	hurtBody.globalAngle = 0;
	hurtBody.offset.x = 0;
	hurtBody.offset.y = 0;
	hurtBody.rw = 16;
	hurtBody.rh = 16;

	double angle = 0;
		
	angle = atan2( gn.x, -gn.y );
		
	hurtBody.globalAngle = angle;

	hurtBody.globalPosition = position + 
		V2d( hitBody.offset.x * cos( hurtBody.globalAngle ) + hurtBody.offset.y * sin( hurtBody.globalAngle ), 
		hitBody.offset.x * -sin( hurtBody.globalAngle ) + hurtBody.offset.y * cos( hurtBody.globalAngle ) );

	frame = 0;
	deathFrame = 0;
	animationFactor = 7;
	slowCounter = 1;
	slowMultiple = 1;

	spawnRect = sf::Rect<double>( gPoint.x - 24, gPoint.y - 24, 24 * 2, 24 * 2 );
}

void Goal::ResetEnemy()
{
	frame = 0;
	deathFrame = 0;
}

void Goal::HandleEntrant( QuadTreeEntrant *qte )
{
}

void Goal::UpdatePrePhysics()
{
	if( frame == 0 )
	{
		
	}
}

void Goal::UpdatePhysics()
{
}

void Goal::UpdatePostPhysics()
{
	if( !dead )
	{
		UpdateHitboxes();

		if( PlayerHitMe() )
		{
			owner->Pause( 6 );
			dead = true;
			receivedHit = NULL;

			
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

	//if( frame == 4 * animationFactor )
	//{
		frame = 0;
	//}

	if( deathFrame == 60 )
	{
		if( owner->player.record == 0 )
		{
			cout << "GAME OVER" << endl;
			owner->goalDestroyed = true;
		}
		owner->RemoveEnemy( this );
	}

	UpdateSprite();
}

void Goal::Draw(sf::RenderTarget *target )
{
	target->draw( sprite );
}

bool Goal::IHitPlayer()
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

bool Goal::PlayerHitMe()
{
	Actor &player = owner->player;
	if( player.currHitboxes != NULL )
	{
		bool hit = false;

		for( list<CollisionBox>::iterator it = player.currHitboxes->begin(); it != player.currHitboxes->end(); ++it )
		{
			if( hurtBody.Intersects( (*it) ) )
			{
				hit = true;
				break;
			}
		}
		

		if( hit )
		{
			receivedHit = player.currHitboxInfo;
			return true;
		}
		
	}

	for( int i = 0; i < player.recordedGhosts; ++i )
	{
		if( player.ghostFrame < player.ghosts[i]->totalRecorded )
		{
			if( player.ghosts[i]->currHitboxes != NULL )
			{
				bool hit = false;
				
				for( list<CollisionBox>::iterator it = player.ghosts[i]->currHitboxes->begin(); it != player.ghosts[i]->currHitboxes->end(); ++it )
				{
					if( hurtBody.Intersects( (*it) ) )
					{
						hit = true;
						break;
					}
				}
		

				if( hit )
				{
					receivedHit = player.currHitboxInfo;
					return true;
				}
			}
			//player.ghosts[i]->curhi
		}
	}
	return false;
}

bool Goal::PlayerSlowingMe()
{
	return false;
}

void Goal::UpdateSprite()
{
	sprite.setTextureRect( ts->GetSubRect( frame / animationFactor ) );
}

void Goal::DebugDraw(sf::RenderTarget *target)
{
}

void Goal::UpdateHitboxes()
{
}

bool Goal::ResolvePhysics( sf::Vector2<double> vel )
{
	return false;
}

void Goal::SaveEnemyState()
{
}

void Goal::LoadEnemyState()
{
}

