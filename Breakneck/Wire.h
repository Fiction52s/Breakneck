#ifndef __WIRE_H__
#define __WIRE_H__

#include <SFML/Graphics.hpp>
//#include "Actor.h"
#include "Physics.h"
#include "QuadTree.h"
//#include "GameSession.h"

struct Actor;
struct Wire : RayCastHandler, QuadTreeCollider
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
	Wire( Actor *player, bool right );
	void UpdateAnchors( sf::Vector2<double> vel );
	void UpdateAnchors2();
	void SetFireDirection( sf::Vector2<double> dir );
	void Check();
	void HandleRayCollision( Edge *edge, double edgeQuantity, double rayPortion );
	void UpdateState( bool touchEdgeWithWire );
	void Draw( sf::RenderTarget *target );
	void DebugDraw( sf::RenderTarget *target );
	void ClearDebug();
	void HandleEntrant( QuadTreeEntrant *qte );
	void TestPoint( Edge * e);
	void SortNewPoints( int start, int end );
	void SwapPoints( int aIndex, int bIndex );

	bool foundPoint;
	sf::Vector2<double> closestPoint;
	
	//double closestInfo;
	double closestDiff;
	sf::Vector2<double> realAnchor;
	sf::Vector2<double> oldPos;
	bool clockwise;

	int addedPoints;
	bool right;
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
		double angleDiff;
	};

	int numPoints;
	//sf::Vector2<double> points[16];
	WirePoint points[100];
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

	std::list<sf::Drawable*> progressDraw;
};

#endif