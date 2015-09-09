#ifndef __QUAD_TREE_H__
#define __QUAD_TREE_H__

#include <SFML\Graphics.hpp>
#include <list>

//using namespace sf;
//using namespace std;

struct QuadTreeEntrant;

struct QuadTreeCollider
{
	virtual void HandleEntrant( QuadTreeEntrant *qte ) = 0;
};

struct QuadTreeEntrant
{
	virtual void HandleQuery( QuadTreeCollider * qtc ) = 0;
	virtual bool IsTouchingBox( sf::Rect<double> &r ) = 0;
};

struct ParentNode;
struct QNode
{
	QNode():parent(NULL){}
	sf::Vector2<double> pos;
	double rw;
	double rh;
//	sf::RenderTarget *debug;
	ParentNode *parent;
	bool leaf;
};

struct ParentNode : QNode
{
	ParentNode( const sf::Vector2<double> &pos, double rw, double rh );
	QNode *children[4];
	std::list<QuadTreeEntrant*> extraChildren;
	// 0    |     1
	//--------------
	// 2    |     3
	
};

struct LeafNode : QNode
{
	int objCount;
	LeafNode( const sf::Vector2<double> &pos, double rw, double rh );
	QuadTreeEntrant *entrants[4];
};

struct RayCastHandler;
struct QuadTree
{
	QuadTree( int width, int height );
	//void Query( QuadTreeCollider *qtc, 
	void DebugDraw( sf::RenderTarget *target );
	void Insert( QuadTreeEntrant *qte );
	void Query(QuadTreeCollider *qtc, const sf::Rect<double> &r );
	QNode *startNode;
private:
	void rQuery( QuadTreeCollider *qtc, QNode *node, const sf::Rect<double> &r );
	QNode * rInsert( QNode *node, QuadTreeEntrant *qte );
	
	void rDebugDraw( sf::RenderTarget *target, QNode *node );
};

#endif