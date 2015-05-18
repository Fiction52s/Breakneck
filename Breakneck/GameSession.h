#include "Physics.h"
#ifndef _GAMESESSION_H__
#define _GAMESESSION_H__

struct GameSession
{
	GameSession();
	void Run( std::string fileName );
	bool OpenFile( std::string fileName );
	sf::RenderWindow *window;
};

#endif