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
		edgeQuantity( q ), bulletVA( sf::Quads, maxBullets * 4 ), dead( false )
{
	ts = owner->GetTileset( "basicturret.png", 48, 48 );
	sprite.setTexture( *ts->texture );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height /2 );
	V2d gPoint = g->GetPoint( edgeQuantity );
	sprite.setPosition( gPoint.x, gPoint.y );
	position = gPoint;

	gn = g->Normal();
	angle = atan2( gn.x, -gn.y );

	sprite.setTextureRect( ts->GetSubRect( 0 ) );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height );
	//V2d gPoint = ground->GetPoint( edgeQuantity );
	sprite.setPosition( gPoint.x, gPoint.y );
	sprite.setRotation( angle / PI * 180 );

	
	ts_bullet = owner->GetTileset( "basicbullet.png", 16, 16 );

	activeBullets = NULL;
	inactiveBullets = NULL;


	for( int i = 0; i < maxBullets; ++i )
	{
		AddBullet();
	}

	
	bulletHitboxInfo = new HitboxInfo;
	bulletHitboxInfo->damage = 10;
	bulletHitboxInfo->drain = 0;
	bulletHitboxInfo->hitlagFrames = 0;
	bulletHitboxInfo->hitstunFrames = 10;
	bulletHitboxInfo->knockback = 0;

	frame = 0;
	deathFrame = 0;
	animationFactor = 5;
	slowCounter = 1;
	slowMultiple = 1;

	bulletSpeed = 1;

	spawnRect = sf::Rect<double>( gPoint.x - 24, gPoint.y - 24, 24 * 2, 24 * 2 );
}

void BasicTurret::HandleEntrant( QuadTreeEntrant *qte )
{
}

void BasicTurret::UpdatePrePhysics()
{
	if( frame == 0 )
	{
		Bullet *b = ActivateBullet();
		if( b != NULL )
		{
			//cout << "firing bullet" << endl;
			b->position = position;
		}
		else
		{
			//cout << "unable to make bullet" << endl;
		}
	}
}

void BasicTurret::UpdatePhysics()
{
	Bullet *currBullet = activeBullets;
	while( currBullet != NULL )
	{	
		//cout << "moving bullet" << endl;
		currBullet->position += gn * bulletSpeed;

		currBullet = currBullet->next;
	}
}

void BasicTurret::UpdatePostPhysics()
{
	if( !dead )
	{
		UpdateHitboxes();

		Bullet *currBullet = activeBullets;
		while( currBullet != NULL )
		{
			//++frame;
			currBullet->frame++;
			if( currBullet->frame == 11 )
				currBullet->frame = 0;
			currBullet = currBullet->next;
		}


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

	if( frame == 15 * animationFactor )
	{
		frame = 0;
	}

	if( deathFrame == 60 )
	{
		owner->RemoveEnemy( this );
	}

	UpdateSprite();
}

void BasicTurret::Draw(sf::RenderTarget *target )
{
	target->draw( sprite );

	target->draw( bulletVA, ts_bullet->texture );
}

bool BasicTurret::IHitPlayer()
{
	Actor &player = owner->player;
	
	Bullet *currBullet = activeBullets;
	while( currBullet != NULL )
	{
		if( currBullet->hitBody.Intersects( player.hurtBody ) )
		{
			player.ApplyHit( bulletHitboxInfo );
			return true;
		}
		currBullet = currBullet->next;
	}

	
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
	sprite.setTextureRect( ts->GetSubRect( frame / animationFactor ) );

	int i = 0;
	Bullet *currBullet = activeBullets;
	while( currBullet != NULL )
	{	
		bulletVA[i*4].position = Vector2f( currBullet->position.x - 8, currBullet->position.y - 8 );
		bulletVA[i*4+1].position = Vector2f( currBullet->position.x + 8, currBullet->position.y - 8 );
		bulletVA[i*4+2].position = Vector2f( currBullet->position.x + 8, currBullet->position.y + 8 );
		bulletVA[i*4+3].position = Vector2f( currBullet->position.x - 8, currBullet->position.y + 8 );

		sf::IntRect rect = ts_bullet->GetSubRect( currBullet->frame );

		bulletVA[i*4].texCoords = Vector2f( rect.left, rect.top );
		bulletVA[i*4+1].texCoords = Vector2f( rect.left + rect.width, rect.top );
		bulletVA[i*4+2].texCoords = Vector2f( rect.left + rect.width, rect.top + rect.height );
		bulletVA[i*4+3].texCoords = Vector2f( rect.left, rect.top + rect.height );
		
		currBullet = currBullet->next;
		++i;
	}
}

void BasicTurret::DebugDraw(sf::RenderTarget *target)
{
	Bullet *currBullet = activeBullets;
	while( currBullet != NULL )
	{
		currBullet->hitBody.DebugDraw( target );
		currBullet->hurtBody.DebugDraw( target );

		currBullet = currBullet->next;
	}

	sf::CircleShape cs;
	cs.setFillColor( Color( 0, 255, 0, 100 ) );
	cs.setRadius( 15 );
	cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
	cs.setPosition( position.x, position.y );
	
	target->draw( cs );
}

void BasicTurret::UpdateHitboxes()
{
	Bullet *currBullet = activeBullets;
	while( currBullet != NULL )
	{
		currBullet->hurtBody.globalPosition = currBullet->position;
		currBullet->hurtBody.globalAngle = 0;
		currBullet->hitBody.globalPosition = currBullet->position;
		currBullet->hitBody.globalAngle = 0;

		currBullet = currBullet->next;
	}
}

bool BasicTurret::ResolvePhysics( sf::Vector2<double> vel )
{
	return false;
}

void BasicTurret::SaveEnemyState()
{
}

void BasicTurret::LoadEnemyState()
{
}

void BasicTurret::AddBullet()
{
	if( inactiveBullets == NULL )
	{
		inactiveBullets = new Bullet;
		inactiveBullets->prev = NULL;
		inactiveBullets->next = NULL;
	}
	else
	{
		Bullet *b = new Bullet;
		b->next = inactiveBullets;
		inactiveBullets->prev = b;
		//b = inactiveBullets;
		inactiveBullets = b;
	}


	inactiveBullets->hurtBody.isCircle = true;
	inactiveBullets->hurtBody.globalAngle = 0;
	inactiveBullets->hurtBody.offset.x = 0;
	inactiveBullets->hurtBody.offset.y = 0;
	inactiveBullets->hurtBody.rw = 8;
	inactiveBullets->hurtBody.rh = 8;

	inactiveBullets->hitBody.type = CollisionBox::Hit;
	inactiveBullets->hitBody.isCircle = true;
	inactiveBullets->hitBody.globalAngle = 0;
	inactiveBullets->hitBody.offset.x = 0;
	inactiveBullets->hitBody.offset.y = 0;
	inactiveBullets->hitBody.rw = 8;
	inactiveBullets->hitBody.rh = 8;
}

void BasicTurret::DeactivateBullet( Bullet *b )
{
	Bullet *prev = b->prev;
	Bullet *next = b->next;

	if( prev == NULL && next == NULL )
	{
		activeBullets = NULL;
	}
	else
	{
		if( b == activeBullets )
		{
			next->prev = NULL;
			activeBullets = next;
		}
		else
		{
			if( prev != NULL )
			{
				prev->next = next;
			}

			if( next != NULL )
			{
				next->prev = prev;
			}
		}
		
	}
}

BasicTurret::Bullet * BasicTurret::ActivateBullet()
{
	if( inactiveBullets == NULL )
	{
		return NULL;
	}
	else
	{
		Bullet *oldStart = inactiveBullets;
		Bullet *newStart = inactiveBullets->next;

		if( newStart != NULL )
		{
			newStart->prev = NULL;	
		}
		inactiveBullets = newStart;

		

		if( activeBullets == NULL )
		{
			activeBullets = oldStart;
			//oldStart->prev = NULL;
			oldStart->next = NULL;
		}
		else
		{
			//oldStart->prev = NULL;
			oldStart->next = activeBullets;
			activeBullets->prev = oldStart;
			activeBullets = oldStart;
		}


		return oldStart;
	}
}

BasicTurret::Bullet::Bullet()
	:prev( NULL ), next( NULL ), frame( 0 )
{
}