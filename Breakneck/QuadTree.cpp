#include "QuadTree.h"
#include "Physics.h"
#include <assert.h>
#include <iostream>

#define V2d sf::Vector2<double>

using namespace sf;
using namespace std;

ParentNode::ParentNode( const V2d &poss, double rww, double rhh )
{
	pos = poss;
	rw = rww;
	rh = rhh;
	leaf = false;
	children[0] = new LeafNode( V2d(pos.x - rw / 2.0, pos.y - rh / 2.0), rw / 2.0, rh / 2.0 );
	children[1] = new LeafNode( V2d(pos.x + rw / 2.0, pos.y - rh / 2.0), rw / 2.0, rh / 2.0 );
	children[2] = new LeafNode( V2d(pos.x - rw / 2.0, pos.y + rh / 2.0), rw / 2.0, rh / 2.0 );
	children[3] = new LeafNode( V2d(pos.x + rw / 2.0, pos.y + rh / 2.0), rw / 2.0, rh / 2.0 );
}

LeafNode::LeafNode( const V2d &poss, double rww, double rhh )
	:objCount(0)
{
	pos = poss;
	rw = rww;
	rh = rhh;

	leaf = true;
	for( int i = 0; i < 4; ++i )
	{
		entrants[i] = NULL;
	}
}

QuadTree::QuadTree( int width, int height )
	:startNode( NULL )
{
	startNode = new LeafNode( V2d( 0, 0), width, height);
	startNode->parent = NULL;//testTree->parent = NULL;
	//testTree->debug = rw;

}

void QuadTree::Query( QuadTreeCollider *qtc, const sf::Rect<double> &r )
{
	rQuery( qtc, startNode, r ); 	
}

void QuadTree::rQuery( QuadTreeCollider *qtc, QNode *node, const sf::Rect<double> &r )
{
	sf::Rect<double> nodeBox( node->pos.x - node->rw, node->pos.y - node->rh, node->rw * 2, node->rh * 2 );

	if( node->leaf )
	{
		LeafNode *n = (LeafNode*)node;

		if( IsBoxTouchingBox( r, nodeBox ) )
		{
			for( int i = 0; i < n->objCount; ++i )
			{
				//qtc->HandleEntrant( n->entrants[i] );
				n->entrants[i]->HandleQuery( qtc );
			}
		}
	}
	else
	{
		//shouldn't this check for box touching box right here??
		ParentNode *n = (ParentNode*)node;

		//if( IsBoxTouchingBox( r, nodeBox ) )
		{
			for( int i = 0; i < 4; ++i )
			{
				rQuery( qtc, n->children[i], r );
			}
		}
	}
}

QNode *QuadTree::rInsert( QNode *node, QuadTreeEntrant *qte )
{
	if( node->leaf )
	{
		LeafNode *n = (LeafNode*)node;
		//assert( n );
		//cout << "- " << endl;
		//ParentNode *n2 = (ParentNode*)node;
		//cout << n2->children[0]->pos.x << endl;
	//	cout << "count" << n->objCount << endl;
		if( n->objCount == 4 ) //full
		{
		//	cout << "splitting" << endl;	
			ParentNode *p = new ParentNode( n->pos, n->rw, n->rh );
			p->parent = n->parent;
			//p->debug = n->debug;

			for( int i = 0; i < 4; ++i )
			{
				rInsert( p, n->entrants[i] );
			}

			delete node;

			return rInsert( p, qte );
		}
		else
		{
			n->entrants[n->objCount] = qte;
			++(n->objCount);
			return node;
		}
	}
	else
	{
		ParentNode *n = (ParentNode*)node;
		sf::Rect<double> nw( node->pos.x - node->rw, node->pos.y - node->rh, node->rw, node->rh);
		sf::Rect<double> ne( node->pos.x, node->pos.y - node->rh, node->rw, node->rh );
		sf::Rect<double> sw( node->pos.x - node->rw, node->pos.y, node->rw, node->rh );
		sf::Rect<double> se( node->pos.x, node->pos.y, node->rw, node->rh );

		if( qte->IsTouchingBox( nw ) )
		{
		//	cout << "calling northwest insert" << endl;
			n->children[0] = rInsert( n->children[0], qte );
		}
		if( qte->IsTouchingBox( ne ) )
		{
		//	cout << "calling northeast insert" << endl;
			n->children[1] = rInsert( n->children[1], qte );
		}
		if( qte->IsTouchingBox( sw ) )
		{
		//	cout << "calling southwest insert" << endl;
			n->children[2] = rInsert( n->children[2], qte );
		}
		if( qte->IsTouchingBox( se ) )
		{
		//	cout << "calling southeast insert" << endl;
			n->children[3] = rInsert( n->children[3], qte );
		}
	}
	return node;
}

void QuadTree::Insert( QuadTreeEntrant *qte )
{
//	cout << "starting insert!" << endl;
	startNode = rInsert( startNode, qte );
}

void QuadTree::rDebugDraw( sf::RenderTarget *target, QNode *node )
{
	if( node->leaf )
	{
		LeafNode *n = (LeafNode*)node;

		sf::RectangleShape rs( sf::Vector2f( node->rw * 2, node->rh * 2 ) );
		int trans = 100;
		if( n->objCount == 0 )
			rs.setFillColor( Color( 100, 100, 100, trans ) ); //
		else if( n->objCount == 1 )
			rs.setFillColor( Color( 255, 0, 0, trans) ); // red == 1
		else if( n->objCount == 2 )
			rs.setFillColor( Color( 0, 255, 0, trans ) ); // green == 2
		else if( n->objCount == 3 )
			rs.setFillColor( Color( 0, 0, 255, trans ) ); //blue == 3
		else
		{
			rs.setFillColor( Color( 0, 100, 255, trans ) ); //blah == 4
		}
		rs.setOrigin( rs.getLocalBounds().width / 2.0, rs.getLocalBounds().height / 2.0 );
		
		rs.setPosition( node->pos.x, node->pos.y );

		target->draw( rs );

		CircleShape cs;
		cs.setFillColor( Color::Cyan );
		cs.setRadius( 1 );
		cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
		cs.setPosition( node->pos.x, node->pos.y );
		//w->draw( cs );
	}
	else
	{
		ParentNode *n = (ParentNode*)node;
		sf::RectangleShape rs( sf::Vector2f( node->rw * 2, node->rh * 2 ) );
		//rs.setOutlineColor( Color::Red );
		rs.setOrigin( rs.getLocalBounds().width / 2.0, rs.getLocalBounds().height / 2.0 );
		//rs.setPosition( node->pos.x - node->rw, node->pos.y - node->rh );
		rs.setPosition( node->pos.x, node->pos.y );
		rs.setFillColor( Color::Transparent );
		//rs.setOutlineThickness( 10 );
		target->draw( rs );

		CircleShape cs;
		cs.setFillColor( Color::Cyan );
		cs.setRadius( 1 );
		cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
		cs.setPosition( node->pos.x, node->pos.y );

		//w->draw( cs );

		for( int i = 0; i < 4; ++i )
			rDebugDraw( target, n->children[i] );
	}
}

void QuadTree::DebugDraw( sf::RenderTarget *target )
{
	rDebugDraw( target, startNode );
}