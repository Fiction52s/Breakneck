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

	sf::Vector2<double> offset;
	double offsetAngle;
	
	double rw; //radius or half width
	double rh; //radius or half height
	bool isCircle;
	BoxType type;	
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

struct ParentNode;

struct QNode
{
	QNode():parent(NULL){}
	sf::Vector2i pos;
	int rw;
	int rh;
	
	ParentNode *parent;
	bool leaf;
};



struct ParentNode : QNode
{
	ParentNode( const sf::Vector2i &pos, int rw, int rh );
	QNode *children[4];
	// 0    |     1
	//--------------
	// 2    |     3
	
};

struct LeafNode : QNode
{
	int objCount;
	LeafNode( const sf::Vector2i &pos, int rw, int rh );
	Edge *edges[4];
};

QNode *Insert( QNode *node, Edge* e );

void DebugDrawQuadTree( sf::RenderWindow *rw, QNode *node );


#endif