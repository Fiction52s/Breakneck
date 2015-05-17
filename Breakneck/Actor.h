#ifndef __ACTOR_H__
#define __ACTOR_H__

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


	Shader sh;
	bool collision;
	Sprite *sprite;
	Tileset *tilesetStand;
	Tileset *tilesetRun;
	Tileset *tilesetDash;
	Tileset *tilesetSlide;
	Tileset *tilesetWallcling;
	Tileset *tilesetJump;
	Tileset *tilesetLand;
	CollisionBox b;
	
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

	Actor();

	void ActionEnded();

	void UpdatePrePhysics();
	Contact minContact;
	bool ResolvePhysics( Edge** edges, int numPoints, V2d vel );

	bool leftGround;
	void UpdatePhysics( Edge **edges, int numPoints );

	void UpdatePostPhysics();


	Action action;
	int frame;
	Vector2<double> position;
	Vector2<double> velocity;
	CollisionBox *physBox;
};


#endif