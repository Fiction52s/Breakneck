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
		AIRDASH,
		STEEPCLIMB,
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

	
	double steepClimbSpeedThresh;
	bool leftGround;
	Contact minContact;
	sf::Shader sh;
	bool collision;
	sf::Sprite *sprite;
	Tileset *tileset[Count];
	
	sf::Sprite gsdodeca;
	sf::Sprite gstriblue;
	sf::Sprite gstricym;
	sf::Sprite gstrigreen;
	sf::Sprite gstrioran;
	sf::Sprite gstripurp;
	sf::Sprite gstrirgb;

	int grindActionLength;
	double grindActionCurrent;

	Tileset * tsgsdodeca;
	Tileset * tsgstriblue;
	Tileset * tsgstricym;
	Tileset * tsgstrigreen;
	Tileset * tsgstrioran;
	Tileset * tsgstripurp;
	Tileset * tsgstrirgb;
	


	CollisionBox b;
	ControllerState prevInput;
	ControllerState currInput;
	sf::Vector2<double> oldVelocity;
	int framesInAir;
	double holdDashAccel;
	double wallThresh;
	sf::Vector2<double> startAirDashVel;

	Edge *ground;
	
	bool hasAirDash;
	bool hasGravReverse;

	double airDashSpeed;

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
	double dashSpeed;

	double doubleJumpStrength;
	bool hasDoubleJump;

	int slowMultiple;
	int slowCounter;
	int timeSlowStrength;

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