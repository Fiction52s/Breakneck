#include <SFML/Graphics.hpp>
#include "VectorMath.h"
#include "QuadTree.h"
#include <list>

#ifndef __EDGE_H__
#define __EDGE_H__

struct Edge : QuadTreeEntrant
{
	Edge();
	sf::Vector2<double> Normal();
	sf::Vector2<double> GetPoint( double quantity );
	double GetQuantity( sf::Vector2<double> p );
	double GetQuantityGivenX( double x );

	void HandleQuery( QuadTreeCollider * qtc );
	bool IsTouchingBox( sf::Rect<double> &r );

	sf::Vector2<double> v0;
	sf::Vector2<double> v1;
	Edge * GetEdge0();
	Edge * GetEdge1();
	Edge *edge0;
	Edge *edge1;
};

struct GameSession;
struct MovingTerrain
{
	MovingTerrain( GameSession *owner, sf::Vector2i pos, 
		std::list<sf::Vector2i> &pathParam, 
		std::list<sf::Vector2i> &pointsParam,
		bool loop, float speed );
	~MovingTerrain();
	void AddPoint(sf::Vector2i p);
	void Finalize();
	void Query( QuadTreeCollider *qtc, const sf::Rect<double> &r );
	void DebugDraw( sf::RenderTarget *target );
	void Draw( sf::RenderTarget *target );
	void UpdatePhysics();
	void AdvanceTargetNode();
	//std::list<Vector2i> tempPoints;
	QuadTree *quadTree;
	Edge **edgeArray;
	sf::VertexArray *polygonVA;
	int numEdges;
	int left;
	int right;
	int top;
	int bottom;
	sf::Vector2<double> position;
	sf::Vector2<double> oldPosition;
	sf::Vector2i *path; //global coords
	int pathLength;
	bool loop;
	sf::Vector2<double> vel;
	int slowMultiple;
	int slowCounter;
	int targetNode;
	bool forward;
	double speed;
	GameSession *owner;
	int numTris;
};

struct CollisionBox
{
	enum BoxType
	{
		Physics,
		Hit,
		Hurt
	};

	
	bool Intersects( CollisionBox &c );
	//double offsetAngle;
	sf::Vector2<double> globalPosition;
	double globalAngle;

	sf::Vector2<double> offset;
	void DebugDraw( sf::RenderTarget *target );

	double rw; //radius or half width
	double rh; //radius or half height
	bool isCircle;
	BoxType type;	
};

struct HitboxInfo
{
	double knockback; //0+
	double drain; //0-1
	int hitstunFrames; 
	int hitlagFrames;
	int damage;
};

struct Contact
{
	Contact();
		
	double collisionPriority;	
	sf::Vector2<double> position;
	sf::Vector2<double> resolution;
	Edge *edge;
	sf::Vector2<double> normal;
	MovingTerrain *movingPlat;
};

struct Collider
{
	Collider();
	~Collider();
	
	Contact *collideEdge( 
		sf::Vector2<double> position, 
		const CollisionBox &b, Edge *e, 
		const sf::Vector2<double> &vel );
		
	Contact *collideEdge2( 
		sf::Vector2<double> position, 
		const CollisionBox &b, Edge *e, 
		const sf::Vector2<double> &vel );
	Contact *collideEdge3( 
		sf::Vector2<double> position, 
		const CollisionBox &b, Edge *e, 
		const sf::Vector2<double> &vel );
	void DebugDraw( sf::RenderTarget *target );
	void ClearDebug();

	Contact *currentContact;
	std::list<sf::Drawable*> progressDraw;

};

struct EdgeParentNode;

struct EdgeQNode
{
	EdgeQNode():parent(NULL),debug(NULL){}
	sf::Vector2<double> pos;
	double rw;
	double rh;
	sf::RenderWindow *debug;
	EdgeParentNode *parent;
	bool leaf;
};



struct EdgeParentNode : EdgeQNode
{
	EdgeParentNode( const sf::Vector2<double> &pos, double rw, double rh );
	EdgeQNode *children[4];
	// 0    |     1
	//--------------
	// 2    |     3
	
};

struct EdgeLeafNode : EdgeQNode
{
	int objCount;
	EdgeLeafNode( const sf::Vector2<double> &pos, double rw, double rh );
	Edge *edges[4];
};

EdgeQNode *Insert( EdgeQNode *node, Edge* e );
//void Query( EdgeQNode *node, void (*f)( Edge *e ) );

void DebugDrawQuadTree( sf::RenderWindow *rw, EdgeQNode *node );

struct EdgeQuadTreeCollider
{
	virtual void HandleEdge( Edge *e ) = 0;
};

void Query( EdgeQuadTreeCollider *qtc, EdgeQNode *node, const sf::Rect<double> &r );

bool IsEdgeTouchingBox( Edge *e, const sf::Rect<double> & ir );

struct RayCastHandler
{
	virtual void HandleRayCollision( Edge *edge, double edgeQuantity, double rayPortion ) = 0;
};

void RayCast( RayCastHandler *handler, QNode *node, 
	sf::Vector2<double> startPoint, 
	sf::Vector2<double> endPoint );

bool IsBoxTouchingBox( const sf::Rect<double> & r0, const sf::Rect<double> & r1 );

//struct QuadTree
//{
//};
#endif