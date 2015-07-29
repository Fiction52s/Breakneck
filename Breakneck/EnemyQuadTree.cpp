#include "Enemy.h"
#include <iostream>

using namespace sf;
using namespace std;

#define V2d sf::Vector2<double>

EnemyParentNode::EnemyParentNode( const V2d &poss, double rww, double rhh )
{
	pos = poss;
	rw = rww;
	rh = rhh;
	leaf = false;
	children[0] = new EnemyLeafNode( V2d(pos.x - rw / 2.0, pos.y - rh / 2.0), rw / 2.0, rh / 2.0 );
	children[1] = new EnemyLeafNode( V2d(pos.x + rw / 2.0, pos.y - rh / 2.0), rw / 2.0, rh / 2.0 );
	children[2] = new EnemyLeafNode( V2d(pos.x - rw / 2.0, pos.y + rh / 2.0), rw / 2.0, rh / 2.0 );
	children[3] = new EnemyLeafNode( V2d(pos.x + rw / 2.0, pos.y + rh / 2.0), rw / 2.0, rh / 2.0 );	
}


EnemyLeafNode::EnemyLeafNode( const V2d &poss, double rww, double rhh )
	:objCount(0)
{
	pos = poss;
	rw = rww;
	rh = rhh;

	leaf = true;
	for( int i = 0; i < 4; ++i )
	{
		enemies[i] = NULL;
	}
}

bool IsEnemyTouchingBox( Enemy *e, const sf::Rect<double> & ir )
{
	sf::Rect<double> er = e->spawnRect;

	return er.intersects( ir );
	

	/*V2d as[4];
	V2d bs[4];
	as[0] = V2d( ir.left, ir.top );
	bs[0] = V2d( ir.left + ir.width, ir.top );

	as[1] =  V2d( ir.left, ir.top + ir.height );
	bs[1] = V2d( ir.left + ir.width, ir.top + ir.height );

	as[2] = V2d( ir.left, ir.top );
	bs[2] = V2d( ir.left, ir.top + ir.height);

	as[3] = V2d( ir.left + ir.width, ir.top );
	bs[3] = V2d( ir.left + ir.width, ir.top + ir.height );

	double erLeft = er.left;
	double erRight = er.left + er.width;
	double erTop = er.top;
	double erBottom = er.top + er.height;

	if( erLeft >= ir.left && erRight <= ir.left + ir.width && erTop >= ir.top && erBottom <= ir.top + ir.height )
		return true;
	//else
	//	return false;
	
	
	for( int i = 0; i < 4; ++i )
	{
		LineIntersection li = lineIntersection( as[i], bs[i], e->v0, e->v1 );

		if( !li.parallel )
		{
			
				V2d a = as[i];
				V2d b = bs[i];
				double e1Left = min( a.x, b.x );
				double e1Right = max( a.x, b.x );
				double e1Top = min( a.y, b.y );
				double e1Bottom = max( a.y, b.y );

				
			//cout << "compares: " << e1Left << ", " << erRight << " .. " << e1Right << ", " << erLeft << endl;
			//cout << "compares y: " << e1Top << " <= " << erBottom << " && " << e1Bottom << " >= " << erTop << endl;
			if( e1Left <= erRight && e1Right >= erLeft && e1Top <= erBottom && e1Bottom >= erTop )
			{
			//	cout << "---!!!!!!" << endl;
				if( (li.position.x < e1Right || approxEquals(li.position.x, e1Right) ) && ( li.position.x > e1Left || approxEquals(li.position.x, e1Left ) ) && ( li.position.y > e1Top || approxEquals( li.position.y, e1Top ) )&& ( li.position.y < e1Bottom || approxEquals( li.position.y, e1Bottom ) ) )
				{
				//	cout << "pos: " << li.position.x << ", " << li.position.y << endl;
				//	cout << "erlrud: " << erLeft << ", " << erRight << ", " << erTop << ", " << erBottom << endl;
					if( ( li.position.x < erRight || approxEquals( li.position.x, erRight )) && ( li.position.x > erLeft || approxEquals( li.position.x, erLeft ) ) && ( li.position.y > erTop || approxEquals( li.position.y, erTop ) ) && ( li.position.y < erBottom || approxEquals( li.position.y, erBottom ) ) )
					{
				//		cout << "seg intersect!!!!!!" << endl;
					//	assert( 0 );
						return true;
					}
				}
			}
		}
	}
	//cout << "return false" << endl;
	return false;*/
}

EnemyQNode *Insert( EnemyQNode *node, Enemy* e )
{
	if( node->leaf )
	{
		EnemyLeafNode *n = (EnemyLeafNode*)node;
		if( n->objCount == 4 ) //full
		{
		//	cout << "splitting" << endl;	
			EnemyParentNode *p = new EnemyParentNode( n->pos, n->rw, n->rh );
			p->parent = n->parent;
			p->debug = n->debug;

			for( int i = 0; i < 4; ++i )
			{
				Insert( p, n->enemies[i] );
			}


			delete node;

			return Insert( p, e );
		}
		else
		{
			//cout << "inserting into leaf . " << n->objCount << endl;
			n->enemies[n->objCount] = e;
			++(n->objCount);
			return node;
		}
	}
	else
	{
	//	cout << "inserting into parent" << endl;
		EnemyParentNode *n = (EnemyParentNode*)node;
		sf::Rect<double> nw( node->pos.x - node->rw, node->pos.y - node->rh, node->rw, node->rh);
		sf::Rect<double> ne( node->pos.x, node->pos.y - node->rh, node->rw, node->rh );
		sf::Rect<double> sw( node->pos.x - node->rw, node->pos.y, node->rw, node->rh );
		sf::Rect<double> se( node->pos.x, node->pos.y, node->rw, node->rh );

		if( IsEnemyTouchingBox( e, nw ) )
		{
	//		cout << "calling northwest insert" << endl;
			n->children[0] = Insert( n->children[0], e );
		}
		if( IsEnemyTouchingBox( e, ne ) )
		{
	//		cout << "calling northeast insert" << endl;
			n->children[1] = Insert( n->children[1], e );
		}
		if( IsEnemyTouchingBox( e, sw ) )
		{
	//		cout << "calling southwest insert" << endl;
			n->children[2] = Insert( n->children[2], e );
		}
		if( IsEnemyTouchingBox( e, se ) )
		{
	//		cout << "calling southeast insert" << endl;
			n->children[3] = Insert( n->children[3], e );
		}
	}

	return node;
}

void DebugDrawQuadTree( sf::RenderWindow *w, EnemyQNode *node )
{
	//cout << "pos: " << node->pos.x << ", " << node->pos.y << " , rw: " << node->rw << ", rh: " << node->rh << endl;
	if( node->leaf )
	{
		EnemyLeafNode *n = (EnemyLeafNode*)node;

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
		
		//rs.setFillColor( Color::Green );
		//rs.setOutlineColor( Color::Blue );
	//	rs.setOutlineThickness( 3 );
		//rs.setFillColor( Color::Transparent );
		//rs.setPosition( node->pos.x - node->rw, node->pos.y - node->rh );
		rs.setOrigin( rs.getLocalBounds().width / 2.0, rs.getLocalBounds().height / 2.0 );
		//rs.setPosition( node->pos.x - node->rw, node->pos.y - node->rh );
		rs.setPosition( node->pos.x, node->pos.y );
		//rs.setOrigin( rs.getLocalBounds().width / 2.0, rs.getLocalBounds().height / 2.0 );

		w->draw( rs );

		CircleShape cs;
		cs.setFillColor( Color::Cyan );
		cs.setRadius( 1 );
		cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
		cs.setPosition( node->pos.x, node->pos.y );
		//w->draw( cs );
	}
	else
	{
		EnemyParentNode *n = (EnemyParentNode*)node;
		sf::RectangleShape rs( sf::Vector2f( node->rw * 2, node->rh * 2 ) );
		//rs.setOutlineColor( Color::Red );
		rs.setOrigin( rs.getLocalBounds().width / 2.0, rs.getLocalBounds().height / 2.0 );
		//rs.setPosition( node->pos.x - node->rw, node->pos.y - node->rh );
		rs.setPosition( node->pos.x, node->pos.y );
		rs.setFillColor( Color::Transparent );
		//rs.setOutlineThickness( 10 );
		w->draw( rs );

		CircleShape cs;
		cs.setFillColor( Color::Cyan );
		cs.setRadius( 1 );
		cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
		cs.setPosition( node->pos.x, node->pos.y );

		//w->draw( cs );

		for( int i = 0; i < 4; ++i )
			DebugDrawQuadTree( w, n->children[i] );
	}
	

	
}

void Query( EnemyQuadTreeCollider *qtc, EnemyQNode *node, const sf::Rect<double> &r )
{
	sf::Rect<double> nodeBox( node->pos.x - node->rw, node->pos.y - node->rh, node->rw * 2, node->rh * 2 );

	if( node->leaf )
	{
		EnemyLeafNode *n = (EnemyLeafNode*)node;

		if( r.intersects( nodeBox ) ) //IsBoxTouchingBox( r, nodeBox ) )
		{
			for( int i = 0; i < n->objCount; ++i )
			{
				if( !n->enemies[i]->spawned )
					qtc->HandleEnemy( n->enemies[i] );
			}
		}
	}
	else
	{
		EnemyParentNode *n = (EnemyParentNode*)node;

		if( r.intersects( nodeBox ) )
		{
			for( int i = 0; i < 4; ++i )
			{
				Query( qtc, n->children[i], r );
			}
		}
	}
	
}