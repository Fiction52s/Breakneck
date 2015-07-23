#include <SFML/Graphics.hpp>
#include "Actor.h"
#include "Physics.h"
#include "GameSession.h"

#ifndef __WIRE_H__
#define __WIRE_H__



struct Wire : RayCastHandler
{
	enum WireState
	{
		IDLE,
		FIRING,
		HIT,
		PULLING,
		RELEASED
	};

	//sf::Vector2<double> 
	Wire( Actor *player );
	void UpdateAnchors();
	void SetFireDirection( sf::Vector2<double> dir );
	void Check();
	void HandleRayCollision( Edge *edge, double edgeQuantity, double rayPortion );
	void UpdateState();
	void Draw( sf::RenderTarget *target );

	WireState state;

	struct WirePoint
	{
		Edge *e;
		double quantity;
		bool start;
		sf::Vector2<double> pos;
		sf::Vector2<double> edgeEnd;
		sf::Vector2<double> test;
		bool clockwise;
	};

	int numPoints;
	//sf::Vector2<double> points[16];
	WirePoint points[16];
	int framesFiring;
	double fireRate;
	sf::Vector2<double> fireDir;
	WirePoint anchor;

	int triggerThresh;
	int hitStallFrames;
	int hitStallCounter;

	double maxTotalLength;
	double totalLength;
	//double minTotalLength;
	double segmentLength;
	double minSegmentLength;
	double pullStrength;

	Actor *player;

	Edge *rcEdge;
	double rcQuant;
};

#endif