#include <SFML/Graphics.hpp>
#include "Tileset.h"
#include "Physics.h"
#include "Input.h"
#include <SFML/Audio.hpp>

#ifndef __ACTOR_H__
#define __ACTOR_H__

struct GameSession;
struct Actor : QuadTreeCollider
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
		STEEPSLIDE,
		GRINDBALL,
		Count
	};

	Actor( GameSession *owner );
	
	void ActionEnded();

	void HandleEdge( Edge *e );
	int possibleEdgeCount;


	void UpdatePrePhysics();
	
	bool ResolvePhysics( Edge** edges, 
		int numPoints, sf::Vector2<double> vel );
	void UpdatePhysics( Edge **edges, 
		int numPoints );
	void UpdatePostPhysics();
	bool CheckWall( bool right );
	bool CheckStandUp();
	void UpdateReversePhysics( Edge **edges, int numPoints );
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
	int framesInAir;
	double holdDashAccel;
	
	Edge *ground;
	
	Edge *grindEdge;
	double grindQuantity;
	double grindSpeed;

	double steepThresh;
	bool reversed;

	int numActiveEdges;
	Edge ** activeEdges;
	double edgeQuantity;
	int actionLength[Action::Count]; //actionLength-1 is the max frame counter for each action
	double groundOffsetX;
	double offsetX;
	bool holdJump;

	sf::SoundBuffer testSound;
	sf::Sound fairSound;

	int wallJumpFrameCounter;
	int wallJumpMovementLimit;

	double dashHeight;
	double normalHeight;
	double doubleJumpHeight;
	double sprintHeight;

	double airSlow;

	double slopeLaunchMinSpeed;
	double maxRunInit;
	double maxGroundSpeed;
	double runAccelInit;
	double runGain;
	double sprintAccel;

	double groundSpeed;
	double maxNormalRun;
	double runAccel;
	double maxFallSpeed;
	double gravity;
	bool facingRight;
	double jumpStrength;
	double airAccel;
	double maxAirXSpeed;
	double maxAirXControl;
	double frameCountTest;
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

	bool col;
	sf::Vector2<double> tempVel;
	std::string queryMode;
	bool checkValid;
};


#endif