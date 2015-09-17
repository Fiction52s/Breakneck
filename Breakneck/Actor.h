#ifndef __ACTOR_H__
#define __ACTOR_H__

#include <list>
#include <map>
#include <SFML/Audio.hpp>
#include "Tileset.h"
#include "Physics.h"
#include "Input.h"
#include <SFML/Graphics.hpp>
#include "Wire.h"

struct GameSession;
struct PlayerGhost;

struct Actor : QuadTreeCollider,
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
		STANDU,
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
		BOUNCEAIR,
		BOUNCEGROUND,
		BOUNCEGROUNDEDWALL,
		DEATH,
		Count
	};

	Actor( GameSession *owner );
	
	void ActionEnded();

	//void HandleEdge( Edge *e );
	void HandleEntrant( QuadTreeEntrant *qte );


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
	void UpdateHitboxes();
	void AirMovement();

	//unsaved vars
	int possibleEdgeCount;
	GameSession *owner;
	double steepClimbSpeedThresh;
	Contact minContact;
	sf::Shader sh;
	bool collision;
	sf::Sprite *sprite;
	Tileset *tileset[Count];

	Tileset *normal[Count];

	sf::Sprite gsdodeca;
	sf::Sprite gstriblue;
	sf::Sprite gstricym;
	sf::Sprite gstrigreen;
	sf::Sprite gstrioran;
	sf::Sprite gstripurp;
	sf::Sprite gstrirgb;

	int grindActionLength;

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

	sf::Sprite dairSword1;
	Tileset *ts_dairSword1;

	sf::Sprite uairSword1;
	Tileset *ts_uairSword1;

	sf::Sprite standingNSword1;
	Tileset *ts_standingNSword1;

	sf::Sprite standingDSword1;
	Tileset *ts_standingDSword1;

	sf::Sprite standingUSword1;
	Tileset *ts_standingUSword1;

	Tileset *ts_bounceRun;
	Tileset *ts_bounceSprint;

	Tileset * ts_fx_airdash;
	Tileset * ts_fx_double;
	Tileset * ts_fx_gravReverse;
	Tileset * ts_fx_bigRunRepeat;
	bool bounceGrounded;

	const static int MAX_MOTION_GHOSTS = 2;
	sf::Sprite motionGhosts[MAX_MOTION_GHOSTS];
	int numMotionGhosts;
	float percentCloneChanged;
	float percentCloneRate;
	bool changingClone;

	double holdDashAccel;
	double wallThresh;

	double airDashSpeed;

	CollisionBox b;
	CollisionBox hurtBody;
	std::list<CollisionBox> *currHitboxes;
	//int numCurrHitboxes;
	HitboxInfo *currHitboxInfo;
	std::map<int, std::list<CollisionBox>*> fairHitboxes;
	std::map<int, std::list<CollisionBox>*> uairHitboxes;
	std::map<int, std::list<CollisionBox>*> dairHitboxes;
	std::map<int, std::list<CollisionBox>*> standNHitboxes;
	std::map<int, std::list<CollisionBox>*> standDHitboxes;
	std::map<int, std::list<CollisionBox>*> standUHitboxes;


	double steepThresh;

	int actionLength[Action::Count]; //actionLength-1 is the max frame counter for each action

	int wallJumpMovementLimit;

	sf::SoundBuffer fairBuffer;
	sf::Sound fairSound;

	sf::SoundBuffer runTappingBuffer;
	sf::Sound runTappingSound;

	sf::SoundBuffer dashStartBuffer;
	sf::Sound dashStartSound;


	sf::SoundBuffer playerHitBuffer;
	sf::Sound playerHitSound;

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

	double maxNormalRun;
	double runAccel;
	double maxFallSpeed;
	double gravity;

	double jumpStrength;
	double airAccel;
	double maxAirXSpeed;
	double maxAirXControl;
	double dashSpeed;

	double doubleJumpStrength;

	int timeSlowStrength;

	sf::Vector2<double> wallJumpStrength;
	double clingSpeed;

	bool col;
	sf::Vector2<double> tempVel;
	std::string queryMode;
	MovingTerrain *currMovingTerrain;

	int maxDespFrames;
	int despCounter;
	bool desperationMode;
	//MovingTerrain *minContactTerrain;

	bool checkValid;

	Edge *rcEdge;
	double rcQuantity;
	std::string rayCastMode;

	bool leftGround;
	double grindActionCurrent;
	ControllerState prevInput;
	ControllerState currInput;
	sf::Vector2<double> oldVelocity;
	int framesInAir;
	sf::Vector2<double> startAirDashVel;

	Edge *ground;
	MovingTerrain *movingGround;

	bool hasAirDash;
	bool hasGravReverse;

	Edge *grindEdge;
	MovingTerrain *grindMovingTerrain;
	double grindQuantity;
	double grindSpeed;

	bool reversed;

	double edgeQuantity;
	
	double groundOffsetX;

	double offsetX;

	bool holdJump;

	int wallJumpFrameCounter;

	double groundSpeed;

	bool facingRight;
	
	bool hasDoubleJump;

	int slowMultiple;
	int slowCounter;

	sf::Vector2<double> wallNormal;

	Action action;
	int frame;
	sf::Vector2<double> position;
	sf::Vector2<double> velocity;
	//CollisionBox *physBox;

	int hitlagFrames;
	int hitstunFrames;
	int invincibleFrames;
	HitboxInfo *receivedHit;

	sf::Vector2<double> storedBounceVel;
	double storedBounceGroundSpeed;
	
	Wire *leftWire;
	Wire *rightWire;
	bool touchEdgeWithLeftWire;
	bool touchEdgeWithRightWire;

	Edge *bounceEdge;
	MovingTerrain *bounceMovingTerrain;
	double bounceQuant;
	Edge *oldBounceEdge;
	int framesSinceBounce;
	bool groundedWallBounce;

	

	
	//dont need to store these
	const static int maxBubbles = 6;
	int bubbleLifeSpan;
	int bubbleRadius;
	Tileset * ts_bubble;
	sf::Sprite bubbleSprite;
	//--
	Tileset *ts_fx_hurtSpack;
	Tileset *ts_fx_dashStart;
	Tileset *ts_fx_dashRepeat;
	Tileset *ts_fx_land;
	
	sf::Vector2<double> bubblePos[maxBubbles];
	int bubbleFramesToLive[maxBubbles];
	int currBubble;

	
	bool hasPowerAirDash;
	bool hasPowerGravReverse;
	bool hasPowerBounce;
	bool hasPowerGrindBall;
	bool hasPowerTimeSlow;
	bool hasPowerLeftWire;
	bool hasPowerRightWire;
	int hasPowerClones;


	//unstored while working on
	bool dead;	

	//end unstored

	void SaveState();
	void LoadState();

	struct Stored
	{
		bool leftGround;
		double grindActionCurrent;
		ControllerState prevInput;
		ControllerState currInput;
		sf::Vector2<double> oldVelocity;
		int framesInAir;
		sf::Vector2<double> startAirDashVel;
		Edge *ground;
		bool hasAirDash;
		bool hasGravReverse;

		Edge *grindEdge;
		
		double grindQuantity;
		double grindSpeed;

		bool reversed;

		double edgeQuantity;
	
		double groundOffsetX;

		double offsetX;

		bool holdJump;

		int wallJumpFrameCounter;

		double groundSpeed;

		bool facingRight;
	
		bool hasDoubleJump;

		int slowMultiple;
		int slowCounter;

		sf::Vector2<double> wallNormal;

		Action action;
		int frame;
		sf::Vector2<double> position;
		sf::Vector2<double> velocity;
		//CollisionBox *physBox;

		int hitlagFrames;
		int hitstunFrames;
		int invincibleFrames;
		HitboxInfo *receivedHit;

		sf::Vector2<double> storedBounceVel;
		Wire *wire;
		Edge *bounceEdge;
		double bounceQuant;

		Edge *oldBounceEdge;
		int framesSinceBounce;

		bool touchEdgeWithLeftWire;
		bool touchEdgeWithRightWire;

		sf::Vector2<double> bubblePos[maxBubbles];
		int bubbleFramesToLive[maxBubbles];
		int currBubble;
	};
	Stored stored;


	//double rotation;
	



	PlayerGhost *testGhost;
	const static int MAX_GHOSTS = 4;
	PlayerGhost *ghosts[MAX_GHOSTS];
	int recordedGhosts;
	int record;
	//bool record;
	bool blah;
	int ghostFrame;

	

	

	//sf::Vector2<double> bounceOffset;

};

struct PlayerGhost
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
		STANDU,
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
		BOUNCEAIR,
		BOUNCEGROUND,
		BOUNCEGROUNDEDWALL,
		DEATH,
		Count
	};
	PlayerGhost();

	struct P
	{
		sf::Vector2<double> position;
		Action action;
		int frame;
		sf::Sprite s;
		bool showSword1;
		sf::Sprite swordSprite1;
		double angle;
		bool createBubble;
		sf::Rect<double> screenRect;
	};
	
	void Draw( sf::RenderTarget *target );
	void UpdatePrePhysics( int ghostFrame );
	void DebugDraw( sf::RenderTarget *target );

	const static int maxFrames = 300;
	P states[maxFrames];//[240];
	int totalRecorded;
	int currFrame;

	std::list<CollisionBox> *currHitboxes;

	std::map<int, std::list<CollisionBox>*> fairHitboxes;
	std::map<int, std::list<CollisionBox>*> uairHitboxes;
	std::map<int, std::list<CollisionBox>*> dairHitboxes;
	std::map<int, std::list<CollisionBox>*> standNHitboxes;
	std::map<int, std::list<CollisionBox>*> standDHitboxes;
	std::map<int, std::list<CollisionBox>*> standUHitboxes;



};

#endif