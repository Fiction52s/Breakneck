#include <SFML/Graphics.hpp>
#include "Actor.h"

#ifndef __WIRE_H__
#define __WIRE_H__



struct Wire
{
	enum WireStates
	{
		IDLE,
		THROWN,
		HIT,
		PULLING,
		RELEASED
	};

	//sf::Vector2<double> 
	void Update( Actor *player );


};

#endif