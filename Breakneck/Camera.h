#ifndef __CAMERA_H__
#define __CAMERA_H__

#include <SFML/Graphics.hpp>
#include "Actor.h"

struct Camera
{
	Camera();
	float GetZoom();
	sf::Vector2f offset;
	sf::Vector2f maxOffset;
	sf::Vector2f pos;
	float zoomFactor;
	float zoomOutRate;
	float zoomInRate;
	float offsetRate;
	float maxZoom;
	float minZoom;
	float zoomLevel;
	float zoomLevel1;
	float zoomLevel2;
	float zoomLevel3;
	
	
	void Update( Actor *a );
};

#endif