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
		DAIR,
		DASH,
		DOUBLE,
		FAIR,
		JUMP,
		LAND,
		LAND2,
		RUN,
		SLIDE,
		SPRINT,
		STAND,
		STANDD,
		STANDN,
		UAIR,
		WALLCLING,
		WALLJUMP,
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
	bool CheckWall( bool right );
	GameSession *owner;
	bool leftGround;
	Contact minContact;
	sf::Shader sh;
	bool collision;
	sf::Sprite *sprite;
	Tileset *tileset[Count];
	CollisionBox b;
	ControllerState prevInput;
	ControllerState currInput;
	sf::Vector2<double> oldVelocity;
	
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

	double dashSpeed;

	double doubleJumpStrength;
	bool hasDoubleJump;


	sf::Vector2<double> wallJumpStrength;
	double clingSpeed;

	sf::Vector2<double> wallNormal;

	Action action;
	int frame;
	sf::Vector2<double> position;
	sf::Vector2<double> velocity;
	CollisionBox *physBox;
};


#endif