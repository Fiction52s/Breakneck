#include <SFML/Graphics.hpp>
#include "Actor.h"
#ifndef __CAMERA_H__
#define __CAMERA_H__

struct Camera
{
	Camera();
	sf::Vector2f offset;
	sf::Vector2f maxOffset;
	sf::Vector2f pos;
	float zoomFactor;
	float zoomOutRate;
	float zoomInRate;
	float offsetRate;
	float maxZoom;
	void Update( Actor *a );
};

#endif