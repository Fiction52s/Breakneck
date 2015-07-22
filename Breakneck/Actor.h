#include <SFML/Graphics.hpp>
#include "Tileset.h"
#include "Physics.h"
#include "Input.h"
#include <SFML/Audio.hpp>
#include <list>
#include <map>

#ifndef __ACTOR_H__
#define __ACTOR_H__

struct GameSession;
struct Actor : EdgeQuadTreeCollider,
	RayCastHandler
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
		AIRHITSTUN,
		GROUNDHITSTUN,
		WIREHOLD,
		Count
	};

	Actor( GameSession *owner );
	
	void ActionEnded();

	void HandleEdge( Edge *e );
	int possibleEdgeCount;


	void UpdatePrePhysics();
	
	void ApplyHit( HitboxInfo *info );

	bool ResolvePhysics( sf::Vector2<double> vel );
	void UpdatePhysics();
	void UpdatePostPhysics();
	bool CheckWall( bool right );
	bool CheckStandUp();
	void UpdateReversePhysics();
	void Draw( sf::RenderTarget *target );

	void DebugDraw( sf::RenderTarget *target );
	void HandleRayCollision( Edge *edge, double edgeQuantity, double rayPortion );
	GameSession *owner;

	void UpdateHitboxes();
	
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
	
	sf::Sprite fairSword1;
	Tileset *ts_fairSword1;
	bool showSword1;

	CollisionBox b;
	CollisionBox hurtBody;
	std::list<CollisionBox> *currHitboxes;
	//int numCurrHitboxes;
	HitboxInfo *currHitboxInfo;

	std::map<int, std::list<CollisionBox>*> fairHitboxes;



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

	int hitlagFrames;
	int hitstunFrames;
	int invincibleFrames;
	HitboxInfo *receivedHit;

	double rotation;

	int framesFiring;
	sf::Vector2<double> fireDir;
	Edge *rcEdge;
	double rcQuantity;
	int wireState;
	Edge *wireEdge;
	double wireQuant;
	std::string rayCastMode;

	struct WirePoint
	{
		Edge *e;
		bool start;
		sf::Vector2<double> pos;
		sf::Vector2<double> edgeEnd;
		sf::Vector2<double> test;
		bool clockwise;
	};

	int pointNum;
	//sf::Vector2<double> points[16];
	WirePoint wirePoints[16];

};

#endif