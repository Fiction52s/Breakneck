#include <SFML/Graphics.hpp>

#ifndef __EDGE_H__
#define __EDGE_H__

struct Edge
{
	Edge();
	sf::Vector2<double> Normal();
	sf::Vector2<double> GetPoint( double quantity );
	double GetQuantity( sf::Vector2<double> p );
	double GetQuantityGivenX( double x );

	sf::Vector2<double> v0;
	sf::Vector2<double> v1;
	Edge * GetEdge0();
	Edge * GetEdge1();
	Edge *edge0;
	Edge *edge1;
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
};

struct Collider
{
	Collider();
	~Collider();
	Contact *currentContact;
		Contact *collideEdge( 
		sf::Vector2<double> position, 
		const CollisionBox &b, Edge *e, 
		const sf::Vector2<double> &vel,
		sf::RenderWindow *w);
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



//struct QuadTree
//{
//};
#endif