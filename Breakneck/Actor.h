#include <SFML/Graphics.hpp>
#include "Tileset.h"
#include "Physics.h"
#include "Input.h"

#ifndef __ACTOR_H__
#define __ACTOR_H__

struct GameSession;
struct Actor
{
	enum Action
	{
		STAND,
		RUN,
		DASH,
		SLIDE,
		WALLCLING,
		JUMP,
		LAND,
		Count
	};

	Actor( GameSession *owner );

	void ActionEnded();

	void UpdatePrePhysics();
	
	bool ResolvePhysics( Edge** edges, 
		int numPoints, sf::Vector2<double> vel );
	void UpdatePhysics( Edge **edges, 
		int numPoints );
	void UpdatePostPhysics();

	GameSession *owner;
	bool leftGround;
	Contact minContact;
	sf::Shader sh;
	bool collision;
	sf::Sprite *sprite;
	Tileset *tilesetStand;
	Tileset *tilesetRun;
	Tileset *tilesetDash;
	Tileset *tilesetSlide;
	Tileset *tilesetWallcling;
	Tileset *tilesetJump;
	Tileset *tilesetLand;
	CollisionBox b;
	ControllerState prevInput;
	ControllerState currInput;
	
	
	Edge *ground;
	int numActiveEdges;
	Edge ** activeEdges;
	double edgeQuantity;
	int actionLength[Action::Count]; //actionLength-1 is the max frame counter for each action
	double groundOffsetX;
	double offsetX;

	double groundSpeed;
	double maxNormalRun;
	double runAccel;
	double maxFallSpeed;
	double gravity;
	bool facingRight;
	double jumpStrength;
	double airAccel;
	double maxAirXSpeed;
	double maxAirXSpeedNormal;

	Action action;
	int frame;
	sf::Vector2<double> position;
	sf::Vector2<double> velocity;
	CollisionBox *physBox;
};


#endif