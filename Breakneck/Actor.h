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


	//unsaved vars
	int possibleEdgeCount;
	GameSession *owner;
	double steepClimbSpeedThresh;
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

	Tileset *ts_bounceRun;
	Tileset *ts_bounceSprint;
	bool bounceGrounded;


	double holdDashAccel;
	double wallThresh;

	double airDashSpeed;

	CollisionBox b;
	CollisionBox hurtBody;
	std::list<CollisionBox> *currHitboxes;
	//int numCurrHitboxes;
	HitboxInfo *currHitboxInfo;
	std::map<int, std::list<CollisionBox>*> fairHitboxes;


	double steepThresh;

	int actionLength[Action::Count]; //actionLength-1 is the max frame counter for each action

	int wallJumpMovementLimit;

	sf::SoundBuffer testSound;
	sf::Sound fairSound;

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

	bool touchEdgeWithWire;

	//unstored while working on
	const static int maxBubbles = 6;
	bool dead;	
	sf::Vector2<double> bubblePos[maxBubbles];
	Tileset * ts_bubble;
	sf::Sprite bubbleSprite;
	int bubbleFramesToLive[maxBubbles];
	int bubbleLifeSpan;
	int currBubble;
	int bubbleRadius;
	struct TimeBubble
	{

	};


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

		bool touchEdgeWithWire;
	};
	Stored stored;


	//double rotation;
	



	PlayerGhost *testGhost;
	PlayerGhost *ghosts[4];
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
	};
	
	void Draw( sf::RenderTarget *target );
	void UpdatePrePhysics( int ghostFrame );
	void DebugDraw( sf::RenderTarget *target );

	P states[240];
	int totalRecorded;
	int maxFrames;
	int currFrame;

	std::list<CollisionBox> *currHitboxes;

	std::map<int, std::list<CollisionBox>*> fairHitboxes;




};

#endif