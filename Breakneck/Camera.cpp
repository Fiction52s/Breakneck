#include "Camera.h"
#include "VectorMath.h"
#include <iostream>

using namespace std;

#define V2d sf::Vector2<double>

Camera::Camera()
{
	offset.x = 0;
	offset.y = 0;

	pos.x = 0;
	pos.y = 0;

	zoomFactor = 1;
	zoomOutRate = 1;
	zoomInRate = .1;
	offsetRate = 60;
	maxZoom = 20;
	maxOffset.x = 100 * 10;
	maxOffset.y = 100 * 10;
}

void Camera::Update( Actor *player )
{
	float temp;
	V2d f;
	if( player->ground != NULL )
	{
		temp = abs(player->groundSpeed) / 20.0;
		f = normalize(player->ground->v1 - player->ground->v0 ) * player->groundSpeed * 10.0;
		//if( abs(temp - zoomFactor) > 1 )
		
		
	}
	else
	{
		//temp = length( player->velocity ) / 20.0;
		temp = abs( player->velocity.x ) / 20.0;
		//temp = zoomFactor;
		f = player->velocity * 10.0;
		/*offset.x = player->velocity.x * 10;
		offset.y = player->velocity.y * 10;
		zoomFactor = abs(player->velocity.x) / 20;
		if( zoomFactor < 1 )
			zoomFactor = 1;*/
	}

	if( temp < zoomFactor )
	{
		zoomFactor -= zoomInRate;
	}
	else if( temp > zoomFactor )
	{
		zoomFactor += zoomOutRate;
	}

	if( abs( temp - zoomFactor ) < 1 )
		zoomFactor = temp;

	if( zoomFactor < 1 )
		zoomFactor = 1;
	else if( zoomFactor > maxZoom )
		zoomFactor = maxZoom;

	if( abs( f.x - offset.x ) <= offsetRate )
	{
		offset.x = f.x;
		offset.y = f.y;
	}
	else if( f.x > offset.x )
	{
		offset.x += offsetRate;
	}
	else if( f.x < offset.x )
	{
		offset.x -= offsetRate;
	}

	if( abs( f.y - offset.y ) <= offsetRate )
	{
		offset.y = f.y;
		offset.y = f.y;
	}

	if( offset.x < -maxOffset.x )
		offset.x = -maxOffset.x;
	else if( offset.x > maxOffset.x )
		offset.x = maxOffset.x;


	if( abs( f.y - offset.y ) <= offsetRate )
	{
		offset.y = f.y;
		offset.y = f.y;
	}
	else if( f.y > offset.y )
	{
		offset.y += offsetRate;
	}
	else if( f.y < offset.y )
	{
		offset.y -= offsetRate;
	}

	if( abs( f.y - offset.y ) <= offsetRate )
	{
		offset.y = f.y;
		offset.y = f.y;
	}

	if( offset.y < -maxOffset.y )
		offset.y = -maxOffset.y;
	else if( offset.y > maxOffset.y )
		offset.y = maxOffset.y;


	pos.x = player->position.x;
	pos.y = player->position.y;

	cout << "zoom: " << zoomFactor << endl;
	//pos.x += offset.x;
	//pos.y += offset.y;
}