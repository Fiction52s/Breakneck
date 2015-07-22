#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"


using namespace std;
using namespace sf;

#define V2d sf::Vector2<double>

Enemy::Enemy( GameSession *own )
	:owner( own ), prev( NULL ), next( NULL ), spawned( false )
{

}

Patroller::Patroller( GameSession *owner, Vector2i pos, list<Vector2i> &pathParam, bool loopP, float speed )
	:Enemy( owner )
{
	position.x = pos.x;
	position.y = pos.y;

	spawnRect = sf::Rect<double>( pos.x - 16, pos.y - 16, pos.x + 16, pos.y + 16 );
	
	pathLength = pathParam.size() + 1;
	path = new Vector2i[pathLength];
	path[0] = pos;

	int index = 1;
	for( list<Vector2i>::iterator it = pathParam.begin(); it != pathParam.end(); ++it )
	{
		path[index] = (*it) + pos;
		++index;
		//path.push_back( (*it) );

	}
	loop = loopP;
	maxSpeed = speed;

	ts = owner->GetTileset( "patroller.png", 32, 32 );
	sprite.setTexture( *ts->texture );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2 );
	position.x = 0;
	position.y = 0;
	hurtBody.type = CollisionBox::Hurt;
	hurtBody.isCircle = true;
	hurtBody.globalAngle = 0;
	hurtBody.offset.x = 0;
	hurtBody.offset.y = 0;
	hurtBody.rw = 16;
	hurtBody.rh = 16;

	hitBody.type = CollisionBox::Hit;
	hitBody.isCircle = true;
	hitBody.globalAngle = 0;
	hitBody.offset.x = 0;
	hitBody.offset.y = 0;
	hitBody.rw = 16;
	hitBody.rh = 16;

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 10;
	hitboxInfo->drain = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 60;
	hitboxInfo->knockback = 0;

	targetNode = 1;
	forward = true;
}

void Patroller::HandleEdge( Edge *e )
{
}

void Patroller::UpdatePrePhysics()
{
}

void Patroller::UpdatePhysics()
{
	//cout << "setting to targetnode: " << targetNode << endl;
	position = V2d( path[targetNode].x, path[targetNode].y );

	if( loop )
	{
		++targetNode;
		if( targetNode == pathLength )
			targetNode = 0;
	}
	else
	{
		if( forward )
		{
			++targetNode;
			if( targetNode == pathLength )
			{
				targetNode -= 2;
				forward = false;
			}
		}
		else
		{
			--targetNode;
			if( targetNode < 0 )
			{
				targetNode = 1;
				forward = true;
			}
		}
	}
}

void Patroller::UpdatePostPhysics()
{
	UpdateHitboxes();

	if( PlayerHitMe() )
	{
		cout << "patroller received damage of: " << receivedHit->damage;
		receivedHit = NULL;
	}

	if( IHitPlayer() )
	{
		cout << "patroller just hit player for " << hitboxInfo->damage << " damage!" << endl;
	}

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
	Actor &player = owner->player;
	if( hitBody.Intersects( player.hurtBody ) )
	{
		player.ApplyHit( hitboxInfo );
		return true;
	}
	return false;
}

void Patroller::UpdateHitboxes()
{
	hurtBody.globalPosition = position;
	hurtBody.globalAngle = 0;
	hitBody.globalPosition = position;
	hitBody.globalAngle = 0;
}

bool Patroller::PlayerHitMe()
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
	return false;
}

void Patroller::DebugDraw( RenderTarget *target )
{
	hurtBody.DebugDraw( target );
	hitBody.DebugDraw( target );
}

Crawler::Crawler( GameSession *owner, Edge *g, double q, bool cw, double s )
:Enemy( owner ), ground( g ), groundQuantity( q ), clockwise( cw ), speed( s )
{
	ts = owner->GetTileset( "crawler.png", 32, 32 );
	sprite.setTexture( *ts->texture );
	sprite.setTextureRect( ts->GetSubRect( 0 ) );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2 );
	V2d gPoint = g->GetPoint( groundQuantity );
	sprite.setPosition( gPoint.x, gPoint.y );

	spawnRect = sf::Rect<double>( gPoint.x - 16, gPoint.y - 16, gPoint.x + 16, gPoint.y + 16 );
}

void Crawler::HandleEdge( Edge *e )
{
}

void Crawler::UpdateHitboxes()
{
	
	if( ground != NULL )
	{
		V2d gn = ground->Normal();
		double angle = 0;
		if( !approxEquals( abs(offsetX), physBody.rw ) )
		{

			//this should never happen
		}
		else
		{
			angle = atan2( gn.x, -gn.y );
		}
		hitBody.globalAngle = angle;
		hurtBody.globalAngle = angle;
	}
	else
	{
		hitBody.globalAngle = 0;
		hurtBody.globalAngle = 0;
	}

	hitBody.globalPosition = V2d( hitBody.offset.x * cos( hitBody.globalAngle ) + hitBody.offset.y * sin( hitBody.globalAngle ), hitBody.offset.x * -sin( hitBody.globalAngle ) + hitBody.offset.y * cos( hitBody.globalAngle ) );
}

void Crawler::UpdatePrePhysics()
{
}

void Crawler::UpdatePhysics()
{
}

void Crawler::UpdatePostPhysics()
{
	UpdateHitboxes();
}

void Crawler::Draw(sf::RenderTarget *target )
{
	target->draw( sprite );
}

bool Crawler::IHitPlayer()
{
	return false;
}

bool Crawler::PlayerHitMe()
{
	return false;
}

void Crawler::UpdateSprite()
{
}

void Crawler::DebugDraw( RenderTarget *target )
{
//	hurtBody.DebugDraw( target );
//	hitBody.DebugDraw( target );
}