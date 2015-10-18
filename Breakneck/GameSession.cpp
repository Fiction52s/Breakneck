//game session

#include "GameSession.h"
#include <fstream>
#include <iostream>
#include <assert.h>
#include "Actor.h"
#include "poly2tri/poly2tri.h"
#include "VectorMath.h"
#include "Camera.h"
#include <sstream>
#include <ctime>

#define TIMESTEP 1.0 / 60.0
#define V2d sf::Vector2<double>

using namespace std;
using namespace sf;

GameSession::GameSession( GameController &c, RenderWindow *rw, RenderTexture *preTex, RenderTexture *miniTex )
	:controller(c),va(NULL),edges(NULL), window(rw), player( this ), activeEnemyList( NULL ), pauseFrames( 0 )
	,groundPar( sf::Quads, 2 * 4 )
{
	usePolyShader = true;
	minimapTex = miniTex;

	if (!polyShader.loadFromFile("mat_shader.frag", sf::Shader::Fragment ) )
	//if (!sh.loadFromMemory(fragmentShader, sf::Shader::Fragment))
	{
		cout << "MATERIAL SHADER NOT LOADING CORRECTLY" << endl;
		//assert( 0 && "polygon shader not loaded" );
		usePolyShader = false;
	}
	
	if (!cloneShader.loadFromFile("clone_shader.frag", sf::Shader::Fragment))
	{
		cout << "CLONE SHADER NOT LOADING CORRECTLY" << endl;
	}

	stringstream ss;

	for( int i = 1; i <= 17; ++i )
	{
		ss << i;
		string texName = "deathbg" + ss.str() + ".png";
		ss.str( "" );
		ss.clear();
		wipeTextures[i-1].loadFromFile( texName );
	}

	deathWipe = false;
	deathWipeFrame = 0;
	deathWipeLength = 17 * 5;

	preScreenTex = preTex;

	terrainTree = new QuadTree( 1000000, 1000000 );
	//testTree = new EdgeLeafNode( V2d( 0, 0), 1000000, 1000000);
	//testTree->parent = NULL;
	//testTree->debug = rw;

	enemyTree = new QuadTree( 1000000, 1000000 );

	borderTree = new QuadTree( 1000000, 1000000 ); 

	grassTree = new QuadTree( 1000000, 1000000 ); 

	lightTree = new QuadTree( 1000000, 1000000 );

	listVA = NULL;
	lightList = NULL;

	inactiveEffects = NULL;
	pauseImmuneEffects = NULL;

	//sets up fx so that they can be used
	for( int i = 0; i < MAX_EFFECTS; ++i )
	{
		AllocateEffect();
	}
	

	//enemyTree = new EnemyLeafNode( V2d( 0, 0), 1000000, 1000000);
	//enemyTree->parent = NULL;
	//enemyTree->debug = rw;
}

GameSession::~GameSession()
{
	for( int i = 0; i < numPoints; ++i )
	{
		delete edges[i];
	}
	delete [] edges;

	for( list<VertexArray*>::iterator it = polygons.begin(); it != polygons.end(); ++it )
	{
		delete (*it);
	}

	for( list<Tileset*>::iterator it = tilesetList.begin(); it != tilesetList.end(); ++it )
	{
		delete (*it);
	}
}

//should only be used to assign a variable. don't use at runtime
Tileset * GameSession::GetTileset( const string & s, int tileWidth, int tileHeight )
{
	for( list<Tileset*>::iterator it = tilesetList.begin(); it != tilesetList.end(); ++it )
	{
		if( (*it)->sourceName == s )
		{
			return (*it);
		}
	}


	//not found


	Tileset *t = new Tileset();
	t->texture = new Texture();
	t->texture->loadFromFile( s );
	t->tileWidth = tileWidth;
	t->tileHeight = tileHeight;
	t->sourceName = s;
	tilesetList.push_back( t );

	return t;
	//make sure to set up tileset here
}

void GameSession::UpdateEnemiesPrePhysics()
{
	Enemy *current = activeEnemyList;
	while( current != NULL )
	{
		current->UpdatePrePhysics();
		current = current->next;
	}
}

void GameSession::UpdateEnemiesPhysics()
{
	Enemy *current = activeEnemyList;
	while( current != NULL )
	{
		current->UpdatePhysics();
		current = current->next;
	}
}

void GameSession::UpdateEnemiesPostPhysics()
{
	Enemy *current = activeEnemyList;
	while( current != NULL )
	{
		Enemy *temp = current->next; //need this in case enemy is removed during its update

		current->UpdatePostPhysics();
		
		current = temp;
	}
}

void GameSession::UpdateEnemiesDraw()
{
	Enemy *current = activeEnemyList;
	while( current != NULL )
	{
		current->Draw( preScreenTex );
		current = current->next;
	}
}

void GameSession::UpdateEnemiesSprites()
{
	Enemy *current = activeEnemyList;
	while( current != NULL )
	{
	//	current->up();
		current = current->next;
	}
}

void GameSession::Test( Edge *e )
{
	cout << "testing" << endl;
}

void GameSession::AddEnemy( Enemy *e )
{
//	cout << "adding enemy: " << e << endl;
	if( activeEnemyList != NULL )
	{
		activeEnemyList->prev = e;
		e->next = activeEnemyList;
		activeEnemyList = e;
	}
	else
	{
		activeEnemyList = e;
	}

	if( player.record > 0 )
	{
		e->spawnedByClone = true;
	}
	
}

void GameSession::RemoveEnemy( Enemy *e )
{
	Enemy *prev = e->prev;
	Enemy *next = e->next;

	if( prev == NULL && next == NULL )
	{
		activeEnemyList = NULL;
	}
	else
	{
		if( e == activeEnemyList )
		{
			next->prev = NULL;
			activeEnemyList = next;
		}
		else
		{
			if( prev != NULL )
			{
				prev->next = next;
			}

			if( next != NULL )
			{
				next->prev = prev;
			}
		}
		
	}

	if( player.record > 0 )
	{
		if( cloneInactiveEnemyList == NULL )
		{
			cloneInactiveEnemyList = e;
			e->next = NULL;
			e->prev = NULL;
			//cout << "creating first dead clone enemy" << endl;

			/*int listSize = 0;
			Enemy *ba = cloneInactiveEnemyList;
			while( ba != NULL )
			{
				listSize++;
				ba = ba->next;
			}

			cout << "size of dead list after first add: " << listSize << endl;*/
		}
		else
		{
			//cout << "creating more dead clone enemies" << endl;
			e->next = cloneInactiveEnemyList;
			cloneInactiveEnemyList->prev = e;
			cloneInactiveEnemyList = e;
		}
	}
	

//	cout << "number of enemies is now: " << CountActiveEnemies() << endl;


	/*if( inactiveEnemyList != NULL )
	{
		inactiveEnemyList->next = e;
	}
	else
	{
		inactiveEnemyList = e;
	}*/
	
	
}

int GameSession::CountActiveEnemies()
{
	Enemy *currEnemy = activeEnemyList;
	int counter = 0;
	while( currEnemy != NULL )
	{
		counter++;	
		currEnemy = currEnemy->next;
	}

	return counter;
}

bool GameSession::OpenFile( string fileName )
{
	currentFile = fileName;
	int insertCount = 0;
	ifstream is;
	is.open( fileName );//+ ".brknk" );
	if( is.is_open() )
	{
		is >> numPoints;
		points = new Vector2<double>[numPoints];
		

		is >> player.position.x;
		is >> player.position.y;
		originalPos.x = player.position.x;
		originalPos.y = player.position.y;

		int pointsLeft = numPoints;

		int pointCounter = 0;

		edges = new Edge*[numPoints];

		int polyCounter = 0;
		//could use an array later if i wanted to
		map<int, int> polyIndex;

		while( pointCounter < numPoints )
		{
			string matStr;
			is >> matStr;

			int polyPoints;
			is >> polyPoints;

			polyIndex[polyCounter] = pointCounter;

			int currentEdgeIndex = pointCounter;
			for( int i = 0; i < polyPoints; ++i )
			{
				int px, py;
				is >> px;
				is >> py;
				//is >> spec;
			
				points[pointCounter].x = px;
				points[pointCounter].y = py;
				++pointCounter;
			}

			double left, right, top, bottom;
			for( int i = 0; i < polyPoints; ++i )
			{
				Edge *ee = new Edge();
					
  				edges[currentEdgeIndex + i] = ee;
				ee->v0 = points[i+currentEdgeIndex];
				if( i < polyPoints - 1 )
					ee->v1 = points[i+1 + currentEdgeIndex];
				else
					ee->v1 = points[currentEdgeIndex];

				terrainTree->Insert( ee );

				double localLeft = min( ee->v0.x, ee->v1.x );
				double localRight = max( ee->v0.x, ee->v1.x );
				double localTop = min( ee->v0.y, ee->v1.y );
				double localBottom = max( ee->v0.y, ee->v1.y ); 
				if( i == 0 )
				{
					left = localLeft;
					right = localRight;
					top = localTop;
					bottom = localBottom;
				}
				else
				{
					left = min( left, localLeft );
					right = max( right, localRight );
					top = min( top, localTop);
					bottom = max( bottom, localBottom);
				}

			}


			for( int i = 0; i < polyPoints; ++i )
			{
				Edge * ee = edges[i + currentEdgeIndex];
				if( i == 0 )
				{
					ee->edge0 = edges[currentEdgeIndex + polyPoints - 1];
					ee->edge1 = edges[currentEdgeIndex + 1];
				}
				else if( i == polyPoints - 1 )
				{
					ee->edge0 = edges[currentEdgeIndex + i - 1];
					ee->edge1 = edges[currentEdgeIndex];
				
				}
				else
				{
					ee->edge0 = edges[currentEdgeIndex + i - 1];
					ee->edge1 = edges[currentEdgeIndex + i + 1];
				}
			}

			int edgesWithSegments;
			is >> edgesWithSegments;

			
			list<GrassSegment> segments;
			for( int i = 0; i < edgesWithSegments; ++i )
			{
				int edgeIndex;
				is >> edgeIndex;
				int numSegments;
				is >> numSegments;
				for( int j = 0; j < numSegments; ++j )
				{
					int index;
					is >> index;
					int reps;
					is >> reps;

					segments.push_back( GrassSegment( edgeIndex, index, reps ) );
				}
			}

			for( list<GrassSegment>::iterator it = segments.begin(); it != segments.end(); ++it )
			{
				V2d A,B,C,D;
				Edge * currE = edges[currentEdgeIndex + (*it).edgeIndex];
				V2d v0 = currE->v0;
				V2d v1 = currE->v1;

				double grassSize = 22;
				double grassSpacing = -5;

				double edgeLength = length( v1 - v0 );
				double remainder = edgeLength / ( grassSize + grassSpacing );

				double num = floor( remainder ) + 1;

				int reps = (*it).reps;

				V2d edgeDir = normalize( v1 - v0 );
				
				//V2d ABmin = v0 + (v1-v0) * (double)(*it).index / num - grassSize / 2 );
				V2d ABmin = v0 + edgeDir * ( edgeLength * (double)(*it).index / num - grassSize / 2 );
				V2d ABmax = v0 + edgeDir * ( edgeLength * (double)( (*it).index + reps )/ num + grassSize / 2 );
				double height = grassSize / 2;
				V2d normal = normalize( v1 - v0 );
				double temp = -normal.x;
				normal.x = normal.y;
				normal.y = temp;

				A = ABmin + normal * height;
				B = ABmax + normal * height;
				C = ABmax;
				D = ABmin;
				
				Grass * g = new Grass;
				g->A = A;
				g->B = B;
				g->C = C;
				g->D = D;

				grassTree->Insert( g );
			}

			vector<p2t::Point*> polyline;
			for( int i = 0; i < polyPoints; ++i )
			{
				polyline.push_back( new p2t::Point( points[currentEdgeIndex +i].x, points[currentEdgeIndex +i].y ) );

			}

			p2t::CDT * cdt = new p2t::CDT( polyline );
	
			cdt->Triangulate();
			vector<p2t::Triangle*> tris;
			tris = cdt->GetTriangles();

			
			va = new VertexArray( sf::Triangles , tris.size() * 3 );
			VertexArray & v = *va;
			Color testColor( 0x75, 0x70, 0x90 );
			for( int i = 0; i < tris.size(); ++i )
			{	
				p2t::Point *p = tris[i]->GetPoint( 0 );	
				p2t::Point *p1 = tris[i]->GetPoint( 1 );	
				p2t::Point *p2 = tris[i]->GetPoint( 2 );	
				v[i*3] = Vertex( Vector2f( p->x, p->y ), testColor );
				v[i*3 + 1] = Vertex( Vector2f( p1->x, p1->y ), testColor );
				v[i*3 + 2] = Vertex( Vector2f( p2->x, p2->y ), testColor );
			}

			polygons.push_back( va );

			VertexArray *polygonVA = va;

			double totalPerimeter = 0;


			double grassSize = 22;
			double grassSpacing = -5;

			Edge * testEdge = edges[currentEdgeIndex];

			int numGrassTotal = 0;

			for( list<GrassSegment>::iterator it = segments.begin(); it != segments.end(); ++it )
			{
				numGrassTotal += (*it).reps + 1;
			}

			
			if( numGrassTotal > 0 )
			{
			va = new VertexArray( sf::Quads, numGrassTotal * 4 );

			//cout << "num grass total: " << numGrassTotal << endl;
			VertexArray &grassVa = *va;

			int segIndex = 0;
			int totalGrass = 0;
			for( list<GrassSegment>::iterator it = segments.begin(); it != segments.end(); ++it )
			{	
				Edge *segEdge = edges[currentEdgeIndex + (*it).edgeIndex];
				V2d v0 = segEdge->v0;
				V2d v1 = segEdge->v1;

				int start = (*it).index;
				int end = (*it).index + (*it).reps;

				int grassCount = (*it).reps + 1;
				//cout << "Grasscount: " << grassCount << endl;

				double remainder = length( v1 - v0 ) / ( grassSize + grassSpacing );

				int num = floor( remainder ) + 1;

				for( int i = 0; i < grassCount; ++i )
				{
					//cout << "indexing at: " << i*4 + segIndex * 4 << endl;
					V2d posd = v0 + (v1 - v0 ) * ((double)( i + start ) / num);
					Vector2f pos( posd.x, posd.y );

					Vector2f topLeft = pos + Vector2f( -grassSize / 2, -grassSize / 2 );
					Vector2f topRight = pos + Vector2f( grassSize / 2, -grassSize / 2 );
					Vector2f bottomLeft = pos + Vector2f( -grassSize / 2, grassSize / 2 );
					Vector2f bottomRight = pos + Vector2f( grassSize / 2, grassSize / 2 );

					//grassVa[i*4].color = Color( 0x0d, 0, 0x80 );//Color::Magenta;
					//borderVa[i*4].color.a = 10;
					grassVa[(i+totalGrass)*4].position = topLeft;
					grassVa[(i+totalGrass)*4].texCoords = Vector2f( 0, 0 );

					//grassVa[i*4+1].color = Color::Blue;
					//borderVa[i*4+1].color.a = 10;
					grassVa[(i+totalGrass)*4+1].position = bottomLeft;
					grassVa[(i+totalGrass)*4+1].texCoords = Vector2f( 0, grassSize );

					//grassVa[i*4+2].color = Color::Blue;
					//borderVa[i*4+2].color.a = 10;
					grassVa[(i+totalGrass)*4+2].position = bottomRight;
					grassVa[(i+totalGrass)*4+2].texCoords = Vector2f( grassSize, grassSize );

					//grassVa[i*4+3].color = Color( 0x0d, 0, 0x80 );
					//borderVa[i*4+3].color.a = 10;
					grassVa[(i+totalGrass)*4+3].position = topRight;
					grassVa[(i+totalGrass)*4+3].texCoords = Vector2f( grassSize, 0 );
					//++i;
				}
				totalGrass += grassCount;
				segIndex++;
			}
			}
			else
			{
				va = NULL;
			}

			VertexArray * grassVA = va;

			//testEdge = edges[currentEdgeIndex];
			
			double size = 16;
			double inward = 16;
			double spacing = 2;

			int innerPolyPoints = 0;
			do
			{
				V2d bisector0 = normalize( testEdge->Normal() + testEdge->edge0->Normal() );
				V2d bisector1 = normalize( testEdge->Normal() + testEdge->edge1->Normal() );
				V2d adjv0 = testEdge->v0 - bisector0 * inward;
				V2d adjv1 = testEdge->v1 - bisector1 * inward;

				V2d nbisector0 = normalize( testEdge->edge1->Normal() + testEdge->Normal() );
				V2d nbisector1 = normalize( testEdge->edge1->Normal() + testEdge->edge1->edge1->Normal() );
				V2d nadjv0 = testEdge->edge1->v0 - nbisector0 * inward;
				V2d nadjv1 = testEdge->edge1->v1 - nbisector1 * inward;


				//double remainder = length( adjv1 - adjv0 ) / size;
				double remainder = length( testEdge->v1- testEdge->v0 ) / size;
				
				//int num = remainder / size;

				//remainder = remainder - floor( remainder );

				//double eachAdd = remainder / num;

				int num = 1;
				while( remainder > 1 )
				{
					++num;
					remainder -= 1;
				}
				

				innerPolyPoints += num;
				

				testEdge = testEdge->edge1;
			}
			while( testEdge != edges[currentEdgeIndex] );
			
		
			//double amount = totalPerimeter / spacing;

			va = new VertexArray( sf::Quads, innerPolyPoints * 4 );
			VertexArray & borderVa = *va;
			double testQuantity = 0;

			int i = 0;
			do
			{
				V2d bisector0 = normalize( testEdge->Normal() + testEdge->edge0->Normal() );
				double q = cross( normalize( testEdge->v1 - testEdge->v0 ), normalize(testEdge->edge0->v0 - testEdge->v0) );
				if( q > 0 )
				{
				//	bisector0 *= ( q );
				}
				
				V2d bisector1 = normalize( testEdge->Normal() + testEdge->edge1->Normal() );
				V2d adjv0 = testEdge->v0 - testEdge->Normal() * inward;//testEdge->v0 - bisector0 * inward;//V2d( testEdge->v0.x, testEdge->v0.y + inward );//
				V2d adjv1 = testEdge->v1 - testEdge->Normal() * inward;//testEdge->v1 - bisector1 * inward;//V2d( testEdge->v1.x, testEdge->v1.y + inward );

				V2d adje0v0 = testEdge->edge0->v0 - testEdge->edge0->Normal() * inward;
				V2d adje0v1 = testEdge->edge0->v1 - testEdge->edge0->Normal() * inward;

				V2d adje1v0 = testEdge->edge1->v0 - testEdge->edge1->Normal() * inward;
				V2d adje1v1 = testEdge->edge1->v1 - testEdge->edge1->Normal() * inward;


				V2d nbisector0 = normalize( testEdge->edge1->Normal() + testEdge->Normal() );
				V2d nbisector1 = normalize( testEdge->edge1->Normal() + testEdge->edge1->edge1->Normal() );
				V2d nadjv0 = testEdge->edge1->v0 - nbisector0 * inward;
				V2d nadjv1 = testEdge->edge1->v1 - nbisector1 * inward;

				LineIntersection li0 = lineIntersection( adjv0, adjv1, adje0v0, adje0v1 );
				LineIntersection li1 = lineIntersection( adjv0, adjv1, adje1v0, adje1v1 );
				//cout << "li0: " << li0.position.x << ", " << li0.position.y << endl;

				//double remainder = length( adjv1 - adjv0 ) / size;
				double remainder = length( testEdge->v1- testEdge->v0 ) / size;
				

				
				bool clockwiseAngle0 = cross( testEdge->edge0->v0 - testEdge->v0, testEdge->v1 - testEdge->v0 ) > 0;
				bool clockwiseAngle1 = cross( testEdge->v0 - testEdge->v1, testEdge->edge1->v1 - testEdge->v1 ) > 0;
				//int num = remainder / size;

				//remainder = remainder - floor( remainder );

				//double eachAdd = remainder / num;

				int num = 1;
				while( remainder > 1 )
				{
					++num;
					remainder -= 1;
				}

				for( int j = 0; j < num; ++j )
				{
					Vector2f surface, inner, surfaceNext, innerNext;


					surface = Vector2f( testEdge->v0.x + (testEdge->v1.x - testEdge->v0.x) * (double)j / num, 
							testEdge->v0.y + (testEdge->v1.y - testEdge->v0.y) * (double)j / num );
					surfaceNext = Vector2f( testEdge->v0.x + (testEdge->v1.x - testEdge->v0.x) * (double)(j+1) / num, 
							testEdge->v0.y + (testEdge->v1.y - testEdge->v0.y) * (double)(j+1) / num );

					if( j == 0 || j == num - 1 )
					{
						if( j == 0 && j == num - 1 )
						{
							V2d v00 = testEdge->v0 - bisector0 * inward;
						

							//inner = Vector2f( v00.x, v00.y );
							if( !li0.parallel )
								inner = Vector2f( li0.position.x, li0.position.y );
							else
								inner = Vector2f( v00.x, v00.y );

							V2d v11 = testEdge->v1 - bisector1 * inward;

							//innerNext = Vector2f( v11.x, v11.y );
							if( !li1.parallel )
								innerNext = Vector2f( li1.position.x, li1.position.y );
							else
								innerNext = Vector2f( v11.x, v11.y );
						}
						else if( j == 0 )
						{
							V2d v00 = testEdge->v0 - bisector0 * inward;
						

							//inner = Vector2f( v00.x, v00.y );

							if( !li0.parallel )
								inner = Vector2f( li0.position.x, li0.position.y );
							else
								inner = Vector2f( v00.x, v00.y );

						
							innerNext = Vector2f( adjv0.x + ( adjv1.x - adjv0.x ) * (double)(j+1) / num,
								adjv0.y + (adjv1.y - adjv0.y) * (double)(j+1) / num );
						}
						else if( j == num - 1 )
						{
							V2d v11 = testEdge->v1 - bisector1 * inward;

							inner = Vector2f( adjv0.x + ( adjv1.x - adjv0.x ) * (double)j / num,
								adjv0.y + (adjv1.y - adjv0.y) * (double)j / num );

							//innerNext = Vector2f( v11.x, v11.y );
							if( !li1.parallel )
								innerNext = Vector2f( li1.position.x, li1.position.y );
							else
								innerNext = Vector2f( v11.x, v11.y );
						}
					}
					else
					{
						
						inner = Vector2f( adjv0.x + ( adjv1.x - adjv0.x ) * (double)j / num,
							adjv0.y + (adjv1.y - adjv0.y) * (double)j / num );

						
						innerNext = Vector2f( adjv0.x + ( adjv1.x - adjv0.x ) * (double)(j+1) / num,
							adjv0.y + (adjv1.y - adjv0.y) * (double)(j+1) / num );
					}

					
					//borderVa[i*4].color = Color( 0x0d, 0, 0x80 );//Color::Magenta;
					//borderVa[i*4].color.a = 10;

					Vector2f coordsTopLeft, coordsTopRight, coordsBottomLeft, coordsBottomRight;
					V2d testN = testEdge->Normal();

					int tileIndex = 0;
					if( abs( testN.x ) > player.wallThresh )
					{
						tileIndex = 2;
					}
					else if( testN.y < 0 && testN.y >= -player.steepThresh ) //might be an equal or not equal prob here with checks for player
					{
						tileIndex = 3;
					}
					else if( testN.y > 0 && testN.y <= player.steepThresh )
					{
						tileIndex = 4;
					}
					else if( testN.y > 0 )
					{
						tileIndex = 1;
					}
					

					int tileX = tileIndex % 3;
					int tileY = tileIndex / 3;
					
					

					coordsTopLeft = Vector2f( tileX * size, tileY * size );
					coordsTopRight = Vector2f( (tileX + 1) * size, tileY * size );
					coordsBottomLeft = Vector2f( tileX * size, (tileY+1) * size );
					coordsBottomRight = Vector2f( (tileX+1) * size, (tileY+1) * size );
					coordsBottomLeft.y -= 1;
					coordsBottomRight.y -= 1;

					coordsBottomRight.x -= 1;
					coordsTopRight.x -= 1;
					
					Vector2f adjSurface = surface;
					//adjSurface.x = floor( adjSurface.x + .5 );
					//adjSurface.y = floor( adjSurface.y + .5 );

					Vector2f adjInner = inner;
					//adjInner.x = floor( adjInner.x + .5 );
					//adjInner.y = floor( adjInner.y + .5 );

					Vector2f adjSurfaceNext = surfaceNext;
					//adjSurfaceNext.x = floor( adjSurfaceNext.x + .5 );
					//adjSurfaceNext.y = floor( adjSurfaceNext.y + .5 );

					Vector2f adjInnerNext = innerNext;
					//adjInnerNext.x = floor( adjInnerNext.x + .5 );
					//adjInnerNext.y = floor( adjInnerNext.y + .5 );

				//	borderVa[i*4].color = Color( 0x0d, 0, 0x80 );
					borderVa[i*4].position = adjSurface;
					borderVa[i*4].texCoords = coordsTopLeft;

				//	borderVa[i*4+1].color = Color::Blue;
					//borderVa[i*4+1].color.a = 10;
					borderVa[i*4+1].position = adjInner;
					borderVa[i*4+1].texCoords = coordsBottomLeft;

				//	borderVa[i*4+2].color = Color::Blue;
					//borderVa[i*4+2].color.a = 10;
					borderVa[i*4+2].position = adjInnerNext;
					borderVa[i*4+2].texCoords = coordsBottomRight;

				//	borderVa[i*4+3].color = Color( 0x0d, 0, 0x80 );
					//borderVa[i*4+3].color.a = 10;
					borderVa[i*4+3].position = adjSurfaceNext;
					borderVa[i*4+3].texCoords = coordsTopRight;
					++i;

					//borderVa[i*4].position = Vector2f( testEdge->v0.x, testEdge->v0.y );
					//borderVa[i*4].texCoords = Vector2f( 0, 0 );

					////borderVa[i*4+1].color = Color::Blue;
					//borderVa[i*4+1].position = Vector2f( adjv0.x, adjv0.y  );
					//borderVa[i*4+1].texCoords = Vector2f( 0, size );

					////borderVa[i*4+2].color = Color::Green;
					//borderVa[i*4+2].position = Vector2f( nadjv0.x, nadjv0.y  );
					//borderVa[i*4+2].texCoords = Vector2f( size, size );

					////borderVa[i*4+3].color = Color::Magenta;
					//borderVa[i*4+3].position = Vector2f( testEdge->edge1->v0.x, testEdge->edge1->v0.y  );
					//borderVa[i*4+3].texCoords = Vector2f( size, 0 );
					//++i;
				}

				

				testEdge = testEdge->edge1;
			}
			while( testEdge != edges[currentEdgeIndex] );


			/*V2d bisector0 = normalize( testEdge->Normal() + testEdge->edge0->Normal() );
			V2d bisector1 = normalize( testEdge->Normal() + testEdge->edge1->Normal() );
			V2d adjv0 = testEdge->v0 - bisector0 * inward;
			V2d adjv1 = testEdge->v1 - bisector1 * inward;

			borderVa[i*2].color = Color::Red;
			borderVa[i*2].position = Vector2f( testEdge->v0.x, testEdge->v0.y );
			borderVa[i*2+1].color = Color::Red;
			borderVa[i*2+1].position = Vector2f( adjv0.x, adjv0.y  );*/
			

				//cout << "loaded to here" << endl;
			//double left, right, bottom, top;
			bool first = true;
			
			/*for( int i = 0; i < amount; ++i )
			{
				double movement = spacing;
				while( movement > 0 )
				{
					testQuantity += movement;
					double testLength = length( adjv1 - adjv0 );
					if( testQuantity > testLength )
					{
						movement = testQuantity - testLength;
						testEdge = testEdge->edge1;
						bisector0 = normalize( testEdge->Normal() + testEdge->edge0->Normal() );
						bisector1 = normalize( testEdge->Normal() + testEdge->edge1->Normal() );
						adjv0 = testEdge->v0 - bisector0 * inward;
						adjv1 = testEdge->v1 - bisector1 * inward;
						testQuantity = 0;
					}
					else
					{
						movement = 0;
					}
				}

				V2d spriteCenter = adjv0 + (adjv1 - adjv0) * testQuantity / length( testEdge->v1 - testEdge->v0 );//testEdge->GetPoint( testQuantity );
				spriteCenter.x = floor( spriteCenter.x + .5 );
				spriteCenter.y = floor( spriteCenter.y + .5 );
			//	spriteCenter.y -= 8 * testEdge->Normal().y;
			//	spriteCenter.x += 8 * testEdge->Normal().x;
				VertexArray & testVa = (*va);
				
				int halfSize = size / 2;
				testVa[i*4].position = Vector2f( spriteCenter.x - halfSize, spriteCenter.y - halfSize );
				testVa[i*4+1].position = Vector2f( spriteCenter.x + halfSize, spriteCenter.y - halfSize );
				testVa[i*4+2].position = Vector2f( spriteCenter.x + halfSize, spriteCenter.y + halfSize );
				testVa[i*4+3].position = Vector2f( spriteCenter.x - halfSize, spriteCenter.y + halfSize );
				//testVa[i*4].position = Vector2f( 0, i * 32 + 100 );
				//testVa[i*4+1].position = Vector2f( 32, i * 32 + 100 );
				//testVa[i*4+2].position = Vector2f( 32, (i+1) * 32+ 100 );
				//testVa[i*4+3].position = Vector2f( 0, (i+1) * 32+100);
				

			//	cout << "vertex x: " << testVa[i].position.x << ", " <<
			//		testVa[i+1].position.x << ", " <<
			//		testVa[i+2].position.x << ", " <<
			//		testVa[i+3].position.x << endl;

				testVa[i*4].texCoords = Vector2f( 0, 0 );
				testVa[i*4+1].texCoords = Vector2f( size, 0 );
				testVa[i*4+2].texCoords = Vector2f( size, size );
				testVa[i*4+3].texCoords = Vector2f( 0, size );
				
				if( first )
				{
					left = spriteCenter.x - halfSize;
					right = spriteCenter.x + halfSize;
					top = spriteCenter.y - halfSize;
					bottom = spriteCenter.y + halfSize;

					first = false;
				}
				else
				{
					double tempLeft = spriteCenter.x - halfSize;
					double tempRight = spriteCenter.x + halfSize;
					double tempTop = spriteCenter.y - halfSize;
					double tempBottom = spriteCenter.y + halfSize;

					if( tempLeft < left )
						left = tempLeft;
					if( tempRight > right )
						right = tempRight;
					if( tempTop < top )
						top = tempTop;
					if( tempBottom > bottom )
						bottom = tempBottom;
				}
				


				
				//testva->aabb
				//polygonBorders.push_back( va );

			}
			*/

			

			TestVA * testva = new TestVA;
			testva->next = NULL;
			testva->va = va;
			testva->aabb.left = left;
			testva->aabb.top = top;
			testva->aabb.width = right - left;
			testva->aabb.height = bottom - top;
			testva->terrainVA = polygonVA;
			testva->grassVA = grassVA;
			
			//cout << "before insert border: " << insertCount << endl;
			borderTree->Insert( testva );


			//cout << "after insert border: " << insertCount << endl;
			insertCount++;
			

			delete cdt;
			for( int i = 0; i < polyPoints; ++i )
			{
				delete polyline[i];
			//	delete tris[i];
			}

			//cout << "loaded to here" << endl;
			++polyCounter;
		}
		//cout << "insertCount: " << insertCount << endl;
		//cout << "polyCOUNTER: " << polyCounter << endl;
		
			cout << "loaded to here" << endl;
		int numMovingPlats;
		is >> numMovingPlats;
		for( int i = 0; i < numMovingPlats; ++i )
		{
			string matStr;
			is >> matStr;


			int polyPoints;
			is >> polyPoints;

			list<Vector2i> poly;

			for( int i = 0; i < polyPoints; ++i )
			{
				int px, py;
				is >> px;
				is >> py;
			
				poly.push_back( Vector2i( px, py ) );
			}


			
			list<Vector2i>::iterator it = poly.begin();
			int left = (*it).x;
			int right = (*it).x;
			int top = (*it).y;
			int bottom = (*it).y;
			
			for( ;it != poly.end(); ++it )
			{
				if( (*it).x < left )
					left = (*it).x;

				if( (*it).x > right )
					right = (*it).x;

				if( (*it).y < top )
					top = (*it).y;

				if( (*it).y > bottom )
					bottom = (*it).y;
			}


			//might need to round for perfect accuracy here
			Vector2i center( (left + right ) / 2, (top + bottom) / 2 );

			for( it = poly.begin(); it != poly.end(); ++it )
			{
				(*it).x -= center.x;
				(*it).y -= center.y;
			}

			int pathPoints;
			is >> pathPoints;

			list<Vector2i> path;

			for( int i = 0; i < pathPoints; ++i )
			{
				int x,y;
				is >> x;
				is >> y;
				path.push_back( Vector2i( x, y ) );
			}

			
			MovingTerrain *mt = new MovingTerrain( this, center, path, poly, false, 2 );
			movingPlats.push_back( mt );
		}

		int numLights;
		is >> numLights;
		for( int i = 0; i < numLights; ++i )
		{
			int x,y,r,g,b;
			is >> x;
			is >> y;
			is >> r;
			is >> g;
			is >> b;

			Light *light = new Light( this, Vector2i( x,y ), Color( r,g,b ), 100 );
			lightTree->Insert( light );
		}
		cout << "loaded to here" << endl;		

		int numGroups;
		is >> numGroups;
		for( int i = 0; i < numGroups; ++i )
		{
			string gName;
			is >> gName;
			int numActors;
			is >> numActors;

			for( int j = 0; j < numActors; ++j )
			{
				string typeName;
				is >> typeName;

				if( typeName == "goal" )
				{
					//always grounded
					string airStr;
					is >> airStr;

					int terrainIndex;
					is >> terrainIndex;

					int edgeIndex;
					is >> edgeIndex;

					double edgeQuantity;
					is >> edgeQuantity;

					Goal *enemy = new Goal( this, edges[polyIndex[terrainIndex] + edgeIndex], edgeQuantity );

					enemyTree->Insert( enemy );
				}
				else if( typeName == "patroller" )
				{
					string airStr;
					is >> airStr;

					int xPos,yPos;

					if( airStr == "+air" )
					{
						is >> xPos;
						is >> yPos;
					}
					else
					{
						assert( false && "air wrong gamesession" );
					}
					

					int pathLength;
					is >> pathLength;

					list<Vector2i> localPath;
					for( int i = 0; i < pathLength; ++i )
					{
						int localX,localY;
						is >> localX;
						is >> localY;
						localPath.push_back( Vector2i( localX, localY ) );
					}


					bool loop;
					string loopStr;
					is >> loopStr;

					if( loopStr == "+loop" )
					{
						loop = true;
					}
					else if( loopStr == "-loop" )
					{
						loop = false;
					}
					else
					{
						assert( false && "should be a boolean" );
					}


					float speed;
					is >> speed;

					Patroller *enemy = new Patroller( this, Vector2i( xPos, yPos ), localPath, loop, speed );
					enemyTree->Insert( enemy );// = Insert( enemyTree, enemy );
				}
				else if( typeName == "crawler" )
				{
					//always grounded
					string airStr;
					is >> airStr;

					int terrainIndex;
					is >> terrainIndex;

					int edgeIndex;
					is >> edgeIndex;

					double edgeQuantity;
					is >> edgeQuantity;

					bool clockwise;
					string cwStr;
					is >> cwStr;

					if( cwStr == "+clockwise" )
						clockwise = true;
					else if( cwStr == "-clockwise" )
						clockwise = false;
					else
					{
						assert( false && "boolean problem" );
					}

					float speed;
					is >> speed;

					Crawler *enemy = new Crawler( this, edges[polyIndex[terrainIndex] + edgeIndex], edgeQuantity, clockwise, speed );
					//enemyTree = Insert( enemyTree, enemy );
					enemyTree->Insert( enemy );
				}
				else if( typeName == "basicturret" )
				{
					//always grounded
					string airStr;
					is >> airStr;

					int terrainIndex;
					is >> terrainIndex;

					int edgeIndex;
					is >> edgeIndex;

					double edgeQuantity;
					is >> edgeQuantity;

					double bulletSpeed;
					is >> bulletSpeed;

					int framesWait;
					is >> framesWait;

					BasicTurret *enemy = new BasicTurret( this, edges[polyIndex[terrainIndex] + edgeIndex], edgeQuantity, bulletSpeed, framesWait );
					//enemyTree = Insert( enemyTree, enemy );
					enemyTree->Insert( enemy );
				}
				else if( typeName == "foottrap" )
				{
					//always grounded
					string airStr;
					is >> airStr;

					int terrainIndex;
					is >> terrainIndex;

					int edgeIndex;
					is >> edgeIndex;

					double edgeQuantity;
					is >> edgeQuantity;

					FootTrap *enemy = new FootTrap( this, edges[polyIndex[terrainIndex] + edgeIndex], edgeQuantity );

					enemyTree->Insert( enemy );
				}
				else
				{
					assert( false && "not a valid type name" );
				}

			}
		}
		
		is.close();
	}
	else
	{

		//new file
		assert( false && "error getting file to edit " );
	}
}

int GameSession::Run( string fileN )
{
	sf::Texture &mountain01Tex = *GetTileset( "mountain01.png", 1920, 1045 / 2 /*540*/ )->texture;

	

	bool showFrameRate = false;
	sf::Font arial;
	arial.loadFromFile( "arial.ttf" );

	sf::Text frameRate( "00", arial, 30 );

	activeSequence = NULL;

	fileName = fileN;
	sf::Texture backTex;
	backTex.loadFromFile( "bg01.png" );
	background = Sprite( backTex );
	background.setOrigin( background.getLocalBounds().width / 2, background.getLocalBounds().height / 2 );
	background.setPosition( 0, 0 );
	bgView = View( sf::Vector2f( 0, 0 ), sf::Vector2f( 960, 540 ) );

	sf::Texture alphaTex;
	alphaTex.loadFromFile( "alphatext.png" );
	Sprite alphaTextSprite(alphaTex);

	//sf::Texture healthTex;
	//healthTex.loadFromFile( "lifebar.png" );
	//sf::Sprite healthSprite( healthTex );
	//healthSprite.setScale( 4, 4 );
	//healthSprite.setPosition( 10, 100 );
	
	//window->setPosition( pos );
	window->setVerticalSyncEnabled( true );
	//window->setFramerateLimit( 60 );
	window->setMouseCursorVisible( true );

	view = View( Vector2f( 300, 300 ), sf::Vector2f( 960 * 2, 540 * 2 ) );
	preScreenTex->setView( view );
	//window->setView( view );

	uiView = View( sf::Vector2f( 480, 270 ), sf::Vector2f( 960, 540 ) );

	window->setVerticalSyncEnabled( true );

	
	sf::RectangleShape bDraw;
	bDraw.setFillColor( Color::Red );
	bDraw.setSize( sf::Vector2f(32 * 2, 32 * 2) );
	bDraw.setOrigin( bDraw.getLocalBounds().width /2, bDraw.getLocalBounds().height / 2 );
	bool bdrawdraw = false;

	OpenFile( fileName );
	
	//parTest = RectangleShape( Vector2f( 1000, 1000 ) );
	//parTest.setFillColor( Color::Red );
	//Texture tex;
	//tex.loadFromFile( "cloud01.png" );
	Tileset *cloudTileset = GetTileset( "cloud01.png", 1269, 350 );
	//parTest.setTexture( tex ); 
	parTest.setTexture( *cloudTileset->texture );
	parTest.setPosition( 0, 0 );

	/*groundPar[0].position = Vector2f( 0, 1080 - 300 );
	groundPar[1].position = Vector2f( 1920 / 2, 1080 - 300 );
	groundPar[2].position = Vector2f( 1920 / 2, 1080 );
	groundPar[3].position = Vector2f( 0, 1080 );

	groundPar[0].texCoords = Vector2f( 0, 0 );
	groundPar[1].texCoords = Vector2f( 1920 / 2, 0 );
	groundPar[2].texCoords = Vector2f( 1920 / 2, 300 );
	groundPar[3].texCoords = Vector2f( 0, 300 );

	groundPar[4].position = Vector2f( 1920 / 2, 1080 - 300 );
	groundPar[5].position = Vector2f( 1920, 1080 - 300 );
	groundPar[6].position = Vector2f( 1920, 1080 );
	groundPar[7].position = Vector2f( 1920 / 2, 1080 );

	groundPar[4].texCoords = Vector2f( 1920 / 2, 0 );
	groundPar[5].texCoords = Vector2f( 1920, 0 );
	groundPar[6].texCoords = Vector2f( 1920, 300 );
	groundPar[7].texCoords = Vector2f( 1920 / 2, 300 );*/

	groundTrans = Transform::Identity;
	groundTrans.translate( 0, 0 );
//	groundPar[0].position = Vector2f( 0, 1080 - 300 );
//	groundPar[1].position = Vector2f( 1920, 1080 - 300 );
//	groundPar[2].position = Vector2f( 1920, 1080 );
//	groundPar[3].position = Vector2f( 0, 1080 );
	

	cam.pos.x = player.position.x;
	cam.pos.y = player.position.y;
	
	sf::Vertex *line = new sf::Vertex[numPoints*2];
	for( int i = 0; i < numPoints; ++i )
	{
		//cout << "i: " << i << endl;
		line[i*2] = sf::Vertex( Vector2f( edges[i]->v0.x, edges[i]->v0.y  ) );
		line[i*2+1] =  sf::Vertex( Vector2f( edges[i]->v1.x, edges[i]->v1.y ) );
	}	

	sf::Vector2<double> nLine( ( line[1].position - line[0].position).x, (line[1].position - line[0].position).y );
	nLine = normalize( nLine );

	sf::Vector2<double> lineNormal( -nLine.y, nLine.x );

	sf::CircleShape circle( 30 );
	circle.setFillColor( Color::Blue );


	

	sf::Clock gameClock;
	double currentTime = 0;
	double accumulator = TIMESTEP + .1;

	Vector2<double> otherPlayerPos;
	
	double zoomMultiple = 1;

	Color borderColor = sf::Color::Green;
	int max = 1000000;
	sf::Vertex border[] =
	{
		sf::Vertex(sf::Vector2<float>(-max, -max), borderColor ),
		sf::Vertex(sf::Vector2<float>(-max, max), borderColor),
		sf::Vertex(sf::Vector2<float>(-max, max), borderColor),
		sf::Vertex(sf::Vector2<float>(max, max), borderColor),
		sf::Vertex(sf::Vector2<float>(max, max), borderColor),
		sf::Vertex(sf::Vector2<float>(max, -max), borderColor),
		sf::Vertex(sf::Vector2<float>(max, -max), borderColor),
		sf::Vertex(sf::Vector2<float>(-max, -max), borderColor)
	};

	
	bool skipped = false;
	bool oneFrameMode = false;
	bool quit = false;

	controller.UpdateState();
	currInput = controller.GetState();
	//ControllerState con = controller.GetState();

	

	bool t = currInput.start;//sf::Keyboard::isKeyPressed( sf::Keyboard::Y );
	bool s = t;
	t = false;
	//bool goalPlayerCollision = false;
	int returnVal = 0;

	polyShader.setParameter( "u_texture", *GetTileset( "testterrain2.png", 96, 96 )->texture );
	Texture & borderTex = *GetTileset( "borders.png", 16, 16 )->texture;

	Texture & grassTex = *GetTileset( "newgrass2.png", 22, 22 )->texture;

	goalDestroyed = false;

	//list<Vector2i> pathTest;
	//list<Vector2i> pointsTest;
	//pathTest.push_back( Vector2i( 200, 0 ) );
	////pathTest.push_back( Vector2i( 0, 100 ) );
	////pathTest.push_back( Vector2i( 100, 100 ) );
	
	//pointsTest.push_back( Vector2i(-100, -100) );
	//pointsTest.push_back( Vector2i(300, 100) );
	//pointsTest.push_back( Vector2i(300, 200) );
	//pointsTest.push_back( Vector2i(-100, 200) );

	////MovingTerrain *mt = new MovingTerrain( this, Vector2i( 900, -600 ), pathTest, pointsTest, false, 2 );
	////movingPlats.push_back( mt );
	
	
	LevelSpecifics();
	//lights.push_back( new Light( this ) );

	View v;
	v.setCenter( 0, 0 );
	v.setSize( 1920/ 2, 1080 / 2 );
	window->setView( v );

	stringstream ss;

	int frameCounterWait = 20;
	int frameCounter = 0;
	double total = 0;

	cloudView = View( Vector2f( 0, 0 ), Vector2f( 1920, 1080 ) );

	while( !quit )
	{
		double newTime = gameClock.getElapsedTime().asSeconds();
		double frameTime = newTime - currentTime;

		if ( frameTime > 0.25 )
			frameTime = 0.25;	
        currentTime = newTime;

		if( showFrameRate )
		{
			if( frameCounter == frameCounterWait )
			{
				double blah = 1.0 / frameTime;
				total += blah;
				ss << total / ( frameCounterWait + 1 ) ;
				frameRate.setString( ss.str() );
				ss.clear();
				ss.str( "" );
				frameCounter = 0;
				total = 0;
			}
			else
			{
				double blah = 1.0 / frameTime;
				total += blah;
				++frameCounter;
			}
		}
		

		accumulator += frameTime;

		
		preScreenTex->clear();
		preScreenTex->setSmooth( false );

		
		coll.ClearDebug();		

		while ( accumulator >= TIMESTEP  )
        {
		//	cout << "currInputleft: " << currInput.leftShoulder << endl;
			bool skipInput = sf::Keyboard::isKeyPressed( sf::Keyboard::PageUp );
			if( oneFrameMode )
			{
				//controller.UpdateState();
				

				ControllerState con;
				//con = controller.GetState();
				
				
				bool tookScreenShot = false;
				bool screenShot = false;
				
				while( true )
				{
					//prevInput = currInput;
					//player.prevInput = currInput;
					controller.UpdateState();
					con = controller.GetState();
					//player.currInput = currInput;
					skipInput = sf::Keyboard::isKeyPressed( sf::Keyboard::PageUp );
					
					bool stopSkippingInput = sf::Keyboard::isKeyPressed( sf::Keyboard::PageDown );
					screenShot = Keyboard::isKeyPressed( sf::Keyboard::Num0 ) && !tookScreenShot;
					
					if( screenShot )
					{
						tookScreenShot = true;
						Image im = window->capture();

						 time_t now = time(0);
						 char* dt = ctime(&now);
						im.saveToFile( "screenshot.png" );//+ string(dt) + ".png" );
					}
					else
					{
						if( skipInput )
						{
							tookScreenShot = false;
						}
					}
					

					if( !skipped && skipInput )//sf::Keyboard::isKeyPressed( sf::Keyboard::K ) && !skipped )
					{
						skipped = true;
						accumulator = 0;//TIMESTEP;
						
						//currentTime = gameClock.getElapsedTime().asSeconds() - TIMESTEP;

						break;
					}
					if( skipped && !skipInput )//!sf::Keyboard::isKeyPressed( sf::Keyboard::K ) && skipped )
					{
						skipped = false;
						//break;
					}
					if( sf::Keyboard::isKeyPressed( sf::Keyboard::L ) )
					{

						//oneFrameMode = false;
						break;
					}
					//if( sf::Keyboard::isKeyPressed( sf::Keyboard::M ) )
					if( stopSkippingInput )
					{

						oneFrameMode = false;
						break;
					}
					

				}

				window->clear();
			}
			else
			{
				window->clear();
			}

			if( skipInput )
				oneFrameMode = true;


			if( sf::Keyboard::isKeyPressed( sf::Keyboard::K ) || player.dead || ( currInput.back && !prevInput.back ) )
			{
				if( player.record > 1 )
				{
					player.LoadState();
					LoadState();
				}

				RespawnPlayer();
				ResetEnemies();

				activeEnemyList = NULL;
				pauseImmuneEffects = NULL;
				cloneInactiveEnemyList = NULL;
			}

			if( sf::Keyboard::isKeyPressed( sf::Keyboard::Y ) || currInput.start )
			{
				quit = true;
				break;
			}
		//	if( !s && currInput.start )//sf::Keyboard::isKeyPressed( sf::Keyboard::Y ) )
	//	{
			
		//	cout << "exit" << endl;
		//	quit = true;
		//	returnVal = 1;
		//	break;
			
			//t = true;
		//}
		//	else if( s && !currInput.start )//!sf::Keyboard::isKeyPressed( sf::Keyboard::Y ) )
		//{
		//	s = false;

//		}


		if( sf::Keyboard::isKeyPressed( sf::Keyboard::Escape ) )
		{
			quit = true;
			returnVal = 1;
			break;
		}

		if( goalDestroyed )
		{
			quit = true;
			returnVal = 1;
			break;
		}

		


			prevInput = currInput;
			player.prevInput = currInput;

			if( !controller.UpdateState() )
			{
				bool up = Keyboard::isKeyPressed( Keyboard::Up );// || Keyboard::isKeyPressed( Keyboard::W );
				bool down = Keyboard::isKeyPressed( Keyboard::Down );// || Keyboard::isKeyPressed( Keyboard::S );
				bool left = Keyboard::isKeyPressed( Keyboard::Left );// || Keyboard::isKeyPressed( Keyboard::A );
				bool right = Keyboard::isKeyPressed( Keyboard::Right );// || Keyboard::isKeyPressed( Keyboard::D );

			//	bool altUp = Keyboard::isKeyPressed( Keyboard::U );
			//	bool altLeft = Keyboard::isKeyPressed( Keyboard::H );
			//	bool altRight = Keyboard::isKeyPressed( Keyboard::K );
			//	bool altDown = Keyboard::isKeyPressed( Keyboard::J );

				ControllerState keyboardInput;    
				keyboardInput.B = Keyboard::isKeyPressed( Keyboard::X );// || Keyboard::isKeyPressed( Keyboard::Period );
				keyboardInput.rightShoulder = Keyboard::isKeyPressed( Keyboard::C );// || Keyboard::isKeyPressed( Keyboard::Comma );
				keyboardInput.Y = Keyboard::isKeyPressed( Keyboard::D );// || Keyboard::isKeyPressed( Keyboard::M );
				keyboardInput.A = Keyboard::isKeyPressed( Keyboard::Z ) || Keyboard::isKeyPressed( Keyboard::Space );// || Keyboard::isKeyPressed( Keyboard::Slash );
				//keyboardInput.leftTrigger = 255 * (Keyboard::isKeyPressed( Keyboard::F ) || Keyboard::isKeyPressed( Keyboard::L ));
				keyboardInput.leftShoulder = Keyboard::isKeyPressed( Keyboard::LShift );
				keyboardInput.X = Keyboard::isKeyPressed( Keyboard::F );
				keyboardInput.start = Keyboard::isKeyPressed( Keyboard::J );
				keyboardInput.back = Keyboard::isKeyPressed( Keyboard::H );
				keyboardInput.rightTrigger = 255 * Keyboard::isKeyPressed( Keyboard::LControl );
			
				keyboardInput.rightStickPad = 0;
				if( Keyboard::isKeyPressed( Keyboard::A ) )
				{
					keyboardInput.rightStickPad += 1 << 1;
				}
				else if( Keyboard::isKeyPressed( Keyboard::S ) )
				{
					keyboardInput.rightStickPad += 1;
				}
				

				
				
				//keyboardInput.rightStickMagnitude
				

				/*if( altRight )
					currInput .altPad += 1 << 3;
				if( altLeft )
					currInput .altPad += 1 << 2;
				if( altUp )
					currInput .altPad += 1;
				if( altDown )
					currInput .altPad += 1 << 1;*/
				
				if( up && down )
				{
					if( prevInput.LUp() )
						keyboardInput.leftStickPad += 1;
					else if( prevInput.LDown() )
						keyboardInput.leftStickPad += ( 1 && down ) << 1;
				}
				else
				{
					keyboardInput.leftStickPad += 1 && up;
					keyboardInput.leftStickPad += ( 1 && down ) << 1;
				}

				if( left && right )
				{
					if( prevInput.LLeft() )
					{
						keyboardInput.leftStickPad += ( 1 && left ) << 2;
					}
					else if( prevInput.LRight() )
					{
						keyboardInput.leftStickPad += ( 1 && right ) << 3;
					}
				}
				else
				{
					keyboardInput.leftStickPad += ( 1 && left ) << 2;
					keyboardInput.leftStickPad += ( 1 && right ) << 3;
				}

				currInput = keyboardInput;
			}
			else
			{
				controller.UpdateState();
				currInput = controller.GetState();

				//currInput.X |= currInput.rightShoulder;

			//currInput.B;//|= currInput.rightTrigger > 200;
	//		cout << "up: " << currInput.LUp() << ", " << (int)currInput.leftStickPad << ", " << (int)currInput.pad << ", " << (int)currInput.rightStickPad << endl;
			}




			player.currInput = currInput;



			if( pauseFrames > 0 )
			{
				if( player.changingClone )
				{
					player.percentCloneChanged += player.percentCloneRate;
					//if( player.percentCloneChanged >= 1 )
					{
						player.percentCloneChanged = 0;
					//	player.percentCloneChanged = 1;
						player.changingClone = false;
						pauseFrames = 0;
					}

					//pauseFrames--;
					accumulator -= TIMESTEP;
					break;
				}

				//cam.offset.y += 10;
				cam.Update( &player );
				
				//view fx that are outside of hitlag pausing
				Enemy *currFX = activeEnemyList;
				while( currFX != NULL )
				{
					if( currFX->type == Enemy::BASICEFFECT )
					{
						BasicEffect * be = (BasicEffect*)currFX;
						if( be->pauseImmune )
						{
							currFX->UpdatePostPhysics();
						}
					}
					
					currFX = currFX->next;
				}


				pauseFrames--;
				accumulator -= TIMESTEP;
				break;
			}

			if( deathWipe )
			{
				deathWipeFrame++;
				if( deathWipeFrame == deathWipeLength )
				{
					deathWipe = false;
					deathWipeFrame = 0;
				}
			}

			if( activeSequence != NULL && activeSequence == startSeq )
			{
				if( !activeSequence->Update() )
				{
					activeSequence = NULL;	
				}
				else
				{
					
				}
			}
			else
			{
				player.UpdatePrePhysics();

			

				UpdateEnemiesPrePhysics();


				for( list<MovingTerrain*>::iterator it = movingPlats.begin(); it != movingPlats.end(); ++it )
				{
					(*it)->UpdatePhysics();
				}

				player.UpdatePhysics( );

				UpdateEnemiesPhysics();


				player.UpdatePostPhysics();

				UpdateEnemiesPostPhysics();
			

				//Vector2f oldCam = cam.pos;
				//float oldCamZoom = cam.GetZoom();

				cam.Update( &player );


				//Vector2f diff = cam.pos - oldCam;


				double camWidth = 960 * cam.GetZoom();
				double camHeight = 540 * cam.GetZoom();
				screenRect = sf::Rect<double>( cam.pos.x - camWidth / 2, cam.pos.y - camHeight / 2, camWidth, camHeight );
			
			
				
				queryMode = "enemy";

				tempSpawnRect = screenRect;
				enemyTree->Query( this, screenRect );

				if( player.blah || player.record > 1 )
				{
					int playback = player.recordedGhosts;
					if( player.record > 1 )
						playback--;

					for( int i = 0; i < playback; ++i )
					{
						PlayerGhost *g = player.ghosts[i];
						if( player.ghostFrame < g->totalRecorded )
						{
							//cout << "querying! " << player.ghostFrame << endl;
							tempSpawnRect = g->states[player.ghostFrame].screenRect;
							enemyTree->Query( this, g->states[player.ghostFrame].screenRect );
						}
					}
				}
			
				if( player.record > 0 )
				{
					player.ghosts[player.record-1]->states[player.ghosts[player.record-1]->currFrame].screenRect =
						screenRect;
				}
			}
			

			accumulator -= TIMESTEP;
		}


		sf::Event ev;
		while( window->pollEvent( ev ) )
		{
			if( ev.type == Event::MouseWheelMoved )
			{
				if( ev.mouseWheel.delta > 0 )
				{
					zoomMultiple /= 2;
				}
				else if( ev.mouseWheel.delta < 0 )
				{
					zoomMultiple *= 2;
				}
				
				if( zoomMultiple < 1 )
				{
					zoomMultiple = 1;
				}
				else if( zoomMultiple > 65536 )
				{
					zoomMultiple = 65536;
				}
			}
		}
		Vector2f camOffset;
		
		
		
		
		if( activeSequence != NULL && activeSequence == startSeq )
		{
			activeSequence->Draw( preScreenTex );
			

			preScreenTex->display();
			const Texture &preTex = preScreenTex->getTexture();
		
			Sprite preTexSprite( preTex );
			preTexSprite.setPosition( -960 / 2, -540 / 2 );
			preTexSprite.setScale( .5, .5 );		

			window->draw( preTexSprite  );
		}
		else
		{


		view.setSize( Vector2f( 960 * cam.GetZoom(), 540 * cam.GetZoom()) );
		lastViewSize = view.getSize();

		//view.setCenter( player.position.x + camOffset.x, player.position.y + camOffset.y );
		view.setCenter( cam.pos.x, cam.pos.y );
		lastViewCenter = view.getCenter();


		
		//window->setView( bgView );
		preScreenTex->setView( bgView );

		preScreenTex->draw( background );
		//window->draw( background );

		
		
		//window->setView( view );

		

		cloudView.setCenter( 960, 540 );
		//cloudView.setSize( 1920, 1080 );
		
		preScreenTex->setView( cloudView );
		//cavedepth
		if( SetGroundPar() )
			preScreenTex->draw( groundPar, &mountain01Tex );
	

		Vector2f orig( originalPos.x, originalPos.y );
		float depth = 3;
		parTest.setPosition( orig / depth + ( cam.pos - orig ) / depth );
		float scale = 1 + ( 1 - 1 / ( cam.GetZoom() * depth ) );
		//parTest.setScale( scale, scale );
		preScreenTex->draw( parTest );
		
		preScreenTex->setView( view );
		
		bDraw.setSize( sf::Vector2f(player.b.rw * 2, player.b.rh * 2) );
		bDraw.setOrigin( bDraw.getLocalBounds().width /2, bDraw.getLocalBounds().height / 2 );
		bDraw.setPosition( player.position.x + player.b.offset.x , player.position.y + player.b.offset.y );
	//	bDraw.setRotation( player.sprite->getRotation() );
		if( bdrawdraw)
		{
			preScreenTex->draw( bDraw );
		}
		//window->draw( bDraw );

	/*	CircleShape cs;
		cs.setFillColor( Color::Cyan );
		cs.setRadius( 10 );
		cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
		cs.setPosition( player.position.x, player.position.y );
		window->draw( cs );*/

	
		//player.sh.setParameter( "u_texture", *GetTileset( "testrocks.png", 25, 25 )->texture );
		//player.sh.setParameter( "u_texture1", *GetTileset( "testrocksnormal.png", 25, 25 )->texture );
		
		
		
		
		
		


		//player.sprite->setTextureRect( IntRect( 0, 0, 300, 225 ) );
		//if( false )
		
		while( lightList != NULL )
		{
			Light *l = lightList->next;
			lightList->next = NULL;
			lightList = l;
		}

		queryMode = "lightdisplay";
		lightTree->Query( this, screenRect );

		Light *lightListIter = lightList;
		while( lightListIter != NULL )
		{
			lightListIter->Draw( preScreenTex );
			lightListIter = lightListIter->next;
		}

		if( activeSequence != NULL )
		{
			activeSequence->Draw( preScreenTex );
		}
		
		

		


		
		sf::RectangleShape rs;
		rs.setSize( Vector2f(64, 64) );
		rs.setOrigin( rs.getLocalBounds().width / 2, rs.getLocalBounds().height / 2 );
		rs.setPosition( otherPlayerPos.x, otherPlayerPos.y  );
		rs.setFillColor( Color::Blue );
		//window->draw( circle );
		//window->draw(line, numPoints * 2, sf::Lines);
		
		polyShader.setParameter( "u_texture", *GetTileset( "testterrain2.png" , 96, 96 )->texture ); //*GetTileset( "testrocks.png", 25, 25 )->texture );
		polyShader.setParameter( "u_normals", *GetTileset( "testterrain2_NORMALS.png", 96, 96 )->texture );
		Vector2i vi = Mouse::getPosition();
		//Vector2i vi = window->mapCoordsToPixel( Vector2f( player.position.x, player.position.y ) );
		//Vector2i vi = window->mapCoordsToPixel( sf::Vector2f( 0, -300 ) );
		//vi -= Vector2i( view.getSize().x / 2, view.getSize().y / 2 );
		Vector3f blahblah( vi.x / 1920.f, (1080 - vi.y) / 1080.f, .015 );
		blahblah.y = 1 - blahblah.y;


		//polyShader.setParameter( "LightPos", blahblah );//Vector3f( 0, -300, .075 ) );
		//polyShader.setParameter( "LightColor", 1, .8, .6, 1 );
		polyShader.setParameter( "AmbientColor", .6, .6, 1, .8 );
		//polyShader.setParameter( "Falloff", Vector3f( .4, 3, 20 ) );

		polyShader.setParameter( "Resolution", window->getSize().x, window->getSize().y);
		polyShader.setParameter( "zoom", cam.GetZoom() );
		polyShader.setParameter( "topLeft", view.getCenter().x - view.getSize().x / 2, 
			view.getCenter().y + view.getSize().y / 2 );
		
		//polyShader.setParameter( "u_texture", *GetTileset( "testterrain.png", 32, 32 )->texture );


		//polyShader.setParameter(  = GetTileset( "testterrain.png", 25, 25 )->texture;

		//for( list<VertexArray*>::iterator it = polygons.begin(); it != polygons.end(); ++it )
		//{
		//	if( usePolyShader )
		//	{
		//		

		//		UpdateTerrainShader();

		//		preScreenTex->draw( *(*it ), &polyShader);
		//	}
		//	else
		//	{
		//		preScreenTex->draw( *(*it ) );
		//	}
		//	//GetTileset( "testrocks.png", 25, 25 )->texture );
		//}
		
		

		sf::Rect<double> testRect( view.getCenter().x - view.getSize().x / 2, view.getCenter().y - view.getSize().y / 2,
			view.getSize().x, view.getSize().y );

		while( listVA != NULL )
		{
			TestVA *t = listVA->next;
			listVA->next = NULL;
			listVA = t;
		}

		//listVA is null here
		queryMode = "border";
		numBorders = 0;
		borderTree->Query( this, screenRect );

		
		
		

		//screenRect = sf::Rect<double>( cam.pos.x - camWidth / 2, cam.pos.y - camHeight / 2, camWidth, camHeight );
		
	

		
		int timesDraw = 0;
		TestVA * listVAIter = listVA;
		//listVAIter->next = NULL;
		while( listVAIter != NULL )
		//for( int i = 0; i < numBorders; ++i )
		{
			if( listVAIter->grassVA != NULL )
				preScreenTex->draw( *listVAIter->grassVA, &grassTex );

			if( usePolyShader )
			{

				sf::Rect<double> polyAndScreen;
				sf::Rect<double> aabb = listVAIter->aabb;
				double rightScreen = screenRect.left + screenRect.width;
				double bottomScreen = screenRect.top + screenRect.height;
				double rightPoly = aabb.left + aabb.width;
				double bottomPoly = aabb.top + aabb.height;

				double left = std::max( screenRect.left, aabb.left );

				double right = std::min( rightPoly, rightScreen );
				
				double top = std::max( screenRect.top, aabb.top );

				double bottom = std::min( bottomScreen, bottomPoly );


				polyAndScreen.left = left;
				polyAndScreen.top = top;
				polyAndScreen.width = right - left;
				polyAndScreen.height = bottom - top;
				
				UpdateTerrainShader( polyAndScreen );//listVAIter->aabb );
				/*sf::RectangleShape rs( Vector2f( listVAIter->aabb.width, listVAIter->aabb.height ) );
				rs.setPosition( listVAIter->aabb.left, listVAIter->aabb.top );
				rs.setOutlineColor( Color::Red );
				rs.setOutlineThickness( 3 );
				rs.setFillColor( Color::Transparent );
				preScreenTex->draw( rs );*/

				preScreenTex->draw( *listVAIter->terrainVA, &polyShader );
			}
			else
			{
				preScreenTex->draw( *listVAIter->terrainVA );
			}
			//cout << "drawing border" << endl;
			preScreenTex->draw( *listVAIter->va, &borderTex );
			//preScreenTex->draw( *listVAIter->va );
			listVAIter = listVAIter->next;
			timesDraw++; 
		}
		//cout << "times draw: " << timesDraw << endl;
		//cout << "drew: " << timesDraw << endl;
		//for( list<VertexArray*>::iterator it = polygonBorders.begin(); it != polygonBorders.end(); ++it )
	//	{
	//		window->draw( *(*it ), &borderTex);//GetTileset( "testrocks.png", 25, 25 )->texture );
	//	}
		UpdateEnemiesDraw();

		if( player.action != Actor::DEATH )
			player.Draw( preScreenTex );

		if( player.action != Actor::GRINDBALL )
		{
			player.leftWire->Draw( preScreenTex );
			player.rightWire->Draw( preScreenTex );
		}

		if( false )//if( currInput.back || sf::Keyboard::isKeyPressed( sf::Keyboard::H ) )
		{
			//alphaTextSprite.setOrigin( alphaTextSprite.getLocalBounds().width / 2, alphaTextSprite.getLocalBounds().height / 2 );
//			alphaTextSprite.setScale( .5, .5 );
			alphaTextSprite.setScale( .5 * view.getSize().x / 960.0, .5 * view.getSize().y / 540.0 );
			alphaTextSprite.setOrigin( alphaTextSprite.getLocalBounds().width / 2, alphaTextSprite.getLocalBounds().height / 2 );
			alphaTextSprite.setPosition( view.getCenter().x, view.getCenter().y );

			preScreenTex->draw( alphaTextSprite );
			//window->draw( alphaTextSprite );
		}

		/*Enemy *currFX = active;
		while( currFX != NULL )
		{
			currFX->Draw( window );
			currFX = currFX->next;
		}*/
		
		for( list<MovingTerrain*>::iterator it = movingPlats.begin(); it != movingPlats.end(); ++it )
		{
			//(*it)->DebugDraw( preScreenTex );
			(*it)->Draw( preScreenTex );
		}

		

		DebugDrawActors();


		//grassTree->DebugDraw( preScreenTex );


		coll.DebugDraw( preScreenTex );

		double minimapZoom = 15;

		View vv;
		vv.setCenter( player.position.x, player.position.y );
		vv.setSize( minimapTex->getSize().x * minimapZoom, minimapTex->getSize().y * minimapZoom );
		minimapTex->setView( vv );
		minimapTex->clear( Color( 0, 0, 0, 191 ) );
		
		CircleShape cs;
		cs.setFillColor( Color::Green );
		cs.setRadius( 50 );
		cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
		cs.setPosition( vv.getCenter().x, vv.getCenter().y );
		


		//vv.setSize( 1920 * 10, 1080 * 10 );
		queryMode = "border";
		numBorders = 0;
		sf::Rect<double> minimapRect(vv.getCenter().x - vv.getSize().x / 2.0,
			vv.getCenter().y - vv.getSize().y / 2.0, vv.getSize().x, vv.getSize().y );

		borderTree->Query( this, minimapRect );

		listVAIter = listVA;
		while( listVAIter != NULL )
		{
			//if( listVAIter->grassVA != NULL )
			//	minimapTex->draw( *listVAIter->grassVA, &grassTex );
			
			minimapTex->draw( *listVAIter->terrainVA );
			//preScreenTex->draw( *listVAIter->va, &borderTex );
			listVAIter = listVAIter->next;
			//timesDraw++; 
		}
		
		minimapTex->draw( cs );

		minimapTex->display();
		const Texture &miniTex = minimapTex->getTexture();

		Sprite minimapSprite( miniTex );
		minimapSprite.setPosition( preScreenTex->getSize().x / 2 - 150, preScreenTex->getSize().y / 2 - 150 );
		minimapSprite.setScale( .5, .5 );
		minimapSprite.setColor( Color( 255, 255, 255, 200 ) );

		preScreenTex->setView( uiView );
		preScreenTex->draw( minimapSprite );

		//window->setView( uiView );
	//	window->draw( healthSprite );
		powerBar.Draw( preScreenTex );

		if( showFrameRate )
		{
			preScreenTex->draw( frameRate );
		}
		

		preScreenTex->setView( view );
		//window->setView( view );

		

		

		

		
		//terrainTree->DebugDraw( window );
		//DebugDrawQuadTree( window, enemyTree );
	//	enemyTree->DebugDraw( window );
		

		if( deathWipe )
		{
			//cout << "showing death wipe frame: " << deathWipeFrame << " panel: " << deathWipeFrame / 5 << endl;
			wipeSprite.setTexture( wipeTextures[deathWipeFrame / 5] );
			wipeSprite.setTextureRect( IntRect( 0, 0, wipeSprite.getTexture()->getSize().x, 
				wipeSprite.getTexture()->getSize().y) );
			wipeSprite.setOrigin( wipeSprite.getLocalBounds().width / 2, wipeSprite.getLocalBounds().height / 2 );
			wipeSprite.setPosition( player.position.x, player.position.y );//view.getCenter().x, view.getCenter().y );
			preScreenTex->draw( wipeSprite );
		}

		if( player.action == Actor::DEATH )
		{
			player.Draw( preScreenTex );
		}
	//	preScreenTex->setSmooth( true );
		preScreenTex->display();
		const Texture &preTex = preScreenTex->getTexture();
		
		Sprite preTexSprite( preTex );
		preTexSprite.setPosition( -960 / 2, -540 / 2 );
		preTexSprite.setScale( .5, .5 );
		//preTexSprite.setOrigin( preTexSprite.getLocalBounds().width / 2, preTexSprite.getLocalBounds().height / 2 );
		
		cloneShader.setParameter( "u_texture", preScreenTex->getTexture() );
		cloneShader.setParameter( "newscreen", player.percentCloneChanged );
		cloneShader.setParameter( "resolution", window->getSize().x, window->getSize().y);
		cloneShader.setParameter( "zoom", cam.GetZoom() );

		window->draw( preTexSprite );//, &cloneShader );
		}


		window->display();

		
	}

	delete [] line;

	//window->setView( window->getDefaultView() );
	//window->clear( Color::Red );
	//window->display();
	return returnVal;
}

void GameSession::HandleEntrant( QuadTreeEntrant *qte )
{
	if( queryMode == "enemy" )
	{
		Enemy *e = (Enemy*)qte;
		//sf::Rect<double> screenRect( cam.pos.x - camWidth / 2, cam.pos.y - camHeight / 2, camWidth, camHeight );
		if( e->spawnRect.intersects( tempSpawnRect ) )
		{
			//cout << "spawning enemy!" << endl;
			assert( e->spawned == false );
			e->spawned = true;

			

			AddEnemy( e );
		}
	}
	else if( queryMode == "border" )
	{
		if( listVA == NULL )
		{
			listVA = (TestVA*)qte;
		//	cout << "1" << endl;
			numBorders++;
		}
		else
		{
			
			TestVA *tva = (TestVA*)qte;
			TestVA *temp = listVA;
			bool okay = true;
			while( temp != NULL )
			{
				if( temp == tva )
				{
					okay = false;
					break;
				}	
				temp = temp->next;
			}

			if( okay )
			{
			
			//cout << "blah: " << (unsigned)tva << endl;
				tva->next = listVA;
				listVA = tva;
				numBorders++;
				//cout << numBorders + 1 << endl;
			}
		}
		
	}
	else if( queryMode == "lightdisplay" )
	{
		if( lightList == NULL )
		{
			lightList = (Light*)qte;
		}
		else
		{
			
			Light *tlight = (Light*)qte;
			Light *temp = lightList;
			bool okay = true;
			while( temp != NULL )
			{
				if( temp == tlight )
				{
					okay = false;
					break;
				}	
				temp = temp->next;
			}

			if( okay )
			{
				tlight->next = lightList;
				lightList = tlight;
			}
		}
	}
	else if( queryMode == "lights" )
	{
		Light *light = (Light*)qte;

		if( lightsAtOnce < tempLightLimit )
		{
			touchedLights[lightsAtOnce] = light;
			lightsAtOnce++;
		}
		else
		{
			//for( int i = 0; i < lightsAtOnce; ++i )
			//{
			//	if( length( V2d( touchedLights[i]->pos.x, touchedLights[i]->pos.y ) - position ) > length( V2d( light->pos.x, light->pos.y ) - position ) )//some calculation here
			//	{
			//		touchedLights[i] = light;
			//		break;
			//	}
					
			//}
		}
	
	}
}

void GameSession::DebugDrawActors()
{
	player.DebugDraw( preScreenTex );
	
	Enemy *currEnemy = activeEnemyList;
	while( currEnemy != NULL )
	{
		currEnemy->DebugDraw( preScreenTex );
		currEnemy = currEnemy->next;
	}
}

void GameSession::TestVA::HandleQuery( QuadTreeCollider *qtc )
{
	qtc->HandleEntrant( this );
}

bool GameSession::TestVA::IsTouchingBox( const sf::Rect<double> &r )
{
	return IsBoxTouchingBox( aabb, r );
}

void GameSession::RespawnPlayer()
{
	player.position = originalPos;
	player.action = player.JUMP;
	player.frame = 1;
	player.velocity.x = 0;
	player.velocity.y = 0;
	player.reversed = false;
	player.b.offset.y = 0;
	player.b.rh = player.normalHeight;
	player.facingRight = true;
	player.offsetX = 0;
	player.prevInput = ControllerState();
	player.currInput = ControllerState();
	player.ground = NULL;
	player.grindEdge = NULL;
	player.dead = false;
	powerBar.points = 100;
	powerBar.layer = 0;
	player.record = 0;
	player.recordedGhosts = 0;
	player.blah = false;
	player.receivedHit = NULL;
}

void GameSession::UpdateTerrainShader( const sf::Rect<double> &aabb )
{
	lightsAtOnce = 0;
	tempLightLimit = 9;

	queryMode = "lights"; 
	lightTree->Query( this, aabb );

	Vector2i vi = Mouse::getPosition();
	Vector3f blahblah( vi.x / 1920.f,  -1 + vi.y / 1080.f, .015 );

/*	Vector3f pos0( vi0.x / 1920.f, (1080 - vi0.y) / 1080.f, .015 ); 
	pos0.y = 1 - pos0.y;
	Vector3f pos1( vi1.x / 1920.f, (1080 - vi1.y) / 1080.f, .015 ); 
	pos1.y = 1 - pos1.y;
	Vector3f pos2( vi2.x / 1920.f, (1080 - vi2.y) / 1080.f, .015 ); 
	pos2.y = 1 - pos2.y;*/
	
	bool on[9];
	for( int i = 0; i < 9; ++i )
	{
		on[i] = false;
	}

	if( lightsAtOnce > 0 )
	{
		float depth0 = touchedLights[0]->depth;
		Vector2i vi0 = Vector2i( preScreenTex->mapCoordsToPixel( Vector2f( touchedLights[0]->pos.x, touchedLights[0]->pos.y ) ) );
		//Vector3f pos0( vi0.x / (float)window->getSize().x, ((float)window->getSize().y - vi0.y) / (float)window->getSize().y, .015 ); 
		Vector3f pos0( vi0.x / (float)window->getSize().x, -1 + vi0.y / (float)window->getSize().y, depth0 );
			//pos0.y = 1 - pos0.y;
		//Vector3f pos0( vi0.x, vi0.y, .015 );
		//cout << pos0.x << ", " << pos0.y << endl;
		Color c0 = touchedLights[0]->color;

		Vector3f falloff0 = touchedLights[0]->falloff;

		
		//sh.setParameter( "On0", true );
		on[0] = true;
		polyShader.setParameter( "LightPos0", pos0 );//Vector3f( 0, -300, .075 ) );
		polyShader.setParameter( "LightColor0", c0.r / 255.0, c0.g / 255.0, c0.b / 255.0, 1 );
		polyShader.setParameter( "Falloff0", falloff0 );
	}
	if( lightsAtOnce > 1 )
	{
		float depth1 = touchedLights[1]->depth;
		Vector2i vi1 = preScreenTex->mapCoordsToPixel( Vector2f( touchedLights[1]->pos.x, touchedLights[1]->pos.y ) );
		Vector3f pos1( vi1.x / (float)window->getSize().x, -1 + vi1.y / (float)window->getSize().y, depth1 ); 
		//	pos1.y = 1 - pos1.y;
		Color c1 = touchedLights[1]->color;
		Vector3f falloff1 = touchedLights[1]->falloff;
		
		on[1] = true;
		//sh.setParameter( "On1", true );
		polyShader.setParameter( "LightPos1", pos1 );//Vector3f( 0, -300, .075 ) );
		polyShader.setParameter( "LightColor1", c1.r / 255.0, c1.g / 255.0, c1.b / 255.0, 1 );
		polyShader.setParameter( "Falloff1", falloff1 );
	}
	if( lightsAtOnce > 2 )
	{
		float depth2 = touchedLights[2]->depth;
		Vector2i vi2 = preScreenTex->mapCoordsToPixel( Vector2f( touchedLights[2]->pos.x, touchedLights[2]->pos.y ) );
		Vector3f pos2( vi2.x / (float)window->getSize().x, -1 + vi2.y / (float)window->getSize().y, depth2 ); 
		//	pos2.y = 1 - pos2.y;
		Color c2 = touchedLights[2]->color;
		Vector3f falloff2 = touchedLights[2]->falloff;
		
		on[2] = true;
		//sh.setParameter( "On2", true );
		polyShader.setParameter( "LightPos2", pos2 );//Vector3f( 0, -300, .075 ) );
		polyShader.setParameter( "LightColor2", c2.r / 255.0, c2.g / 255.0, c2.b / 255.0, 1 );
		polyShader.setParameter( "Falloff2", falloff2 );
	}
	if( lightsAtOnce > 3 )
	{
		float depth3 = touchedLights[3]->depth;
		Vector2i vi3 = preScreenTex->mapCoordsToPixel( Vector2f( touchedLights[3]->pos.x, touchedLights[3]->pos.y ) );
		Vector3f pos3( vi3.x / (float)window->getSize().x, -1 + vi3.y / (float)window->getSize().y, depth3 ); 
		//	pos3.y = 1 - pos3.y;
		Color c3 = touchedLights[3]->color;
		Vector3f falloff3 = touchedLights[3]->falloff;
		
		on[3] = true;
		//sh.setParameter( "On3", true );
		polyShader.setParameter( "LightPos3", pos3 );
		polyShader.setParameter( "LightColor3", c3.r / 255.0, c3.g / 255.0, c3.b / 255.0, 1 );
		polyShader.setParameter( "Falloff3", falloff3 );
	}
	if( lightsAtOnce > 4 )
	{
		float depth4 = touchedLights[4]->depth;
		Vector2i vi4 = preScreenTex->mapCoordsToPixel( Vector2f( touchedLights[4]->pos.x, touchedLights[4]->pos.y ) );
		Vector3f pos4( vi4.x / (float)window->getSize().x, -1 + vi4.y / (float)window->getSize().y, depth4 ); 
		//	pos4.y = 1 - pos4.y;
		Color c4 = touchedLights[4]->color;
		Vector3f falloff4 = touchedLights[4]->falloff;
		
		on[4] = true;
		//sh.setParameter( "On4", true );
		polyShader.setParameter( "LightPos4", pos4 );
		polyShader.setParameter( "LightColor4", c4.r / 255.0, c4.g / 255.0, c4.b / 255.0, 1 );
		polyShader.setParameter( "Falloff4", falloff4 );
	}
	if( lightsAtOnce > 5 )
	{
		float depth5 = touchedLights[5]->depth;
		Vector2i vi5 = preScreenTex->mapCoordsToPixel( Vector2f( touchedLights[5]->pos.x, touchedLights[5]->pos.y ) );
		Vector3f pos5( vi5.x / (float)window->getSize().x, -1 + vi5.y / (float)window->getSize().y, depth5 ); 
		//	pos5.y = 1 - pos5.y;
		Color c5 = touchedLights[5]->color;
		Vector3f falloff5 = touchedLights[5]->falloff;
		
		on[5] = true;
		//sh.setParameter( "On5", true );
		polyShader.setParameter( "LightPos5", pos5 );
		polyShader.setParameter( "LightColor5", c5.r / 255.0, c5.g / 255.0, c5.b / 255.0, 1 );
		polyShader.setParameter( "Falloff5", falloff5 );
	}
	if( lightsAtOnce > 6 )
	{
		float depth6 = touchedLights[6]->depth;
		Vector2i vi6 = preScreenTex->mapCoordsToPixel( Vector2f( touchedLights[6]->pos.x, touchedLights[6]->pos.y ) );
		Vector3f pos6( vi6.x / (float)window->getSize().x, -1 + vi6.y / (float)window->getSize().y, depth6 ); 
		//	pos6.y = 1 - pos6.y;
		Color c6 = touchedLights[6]->color;
		Vector3f falloff6 = touchedLights[6]->falloff;
		
		on[6] = true;
		//sh.setParameter( "On6", true );
		polyShader.setParameter( "LightPos6", pos6 );
		polyShader.setParameter( "LightColor6", c6.r / 255.0, c6.g / 255.0, c6.b / 255.0, 1 );
		polyShader.setParameter( "Falloff6", falloff6 );
	}
	if( lightsAtOnce > 7 )
	{
		float depth7 = touchedLights[7]->depth;
		Vector2i vi7 = preScreenTex->mapCoordsToPixel( Vector2f( touchedLights[7]->pos.x, touchedLights[7]->pos.y ) );
		Vector3f pos7( vi7.x / (float)window->getSize().x, -1 + vi7.y / (float)window->getSize().y, depth7 ); 
		//	pos7.y = 1 - pos7.y;
		Color c7 = touchedLights[7]->color;
		Vector3f falloff7 = touchedLights[7]->falloff;
		
		on[7] = true;
		//sh.setParameter( "On7", true );
		polyShader.setParameter( "LightPos7", pos7 );
		polyShader.setParameter( "LightColor7", c7.r / 255.0, c7.g / 255.0, c7.b / 255.0, 1 );
		polyShader.setParameter( "Falloff7", falloff7 );
	}
	if( lightsAtOnce > 8 )
	{
		float depth8 = touchedLights[8]->depth;
		Vector2i vi8 = preScreenTex->mapCoordsToPixel( Vector2f( touchedLights[8]->pos.x, touchedLights[8]->pos.y ) );
		Vector3f pos8( vi8.x / (float)window->getSize().x, -1 + vi8.y / (float)window->getSize().y, depth8 ); 
		//	pos8.y = 1 - pos8.y;
		Color c8 = touchedLights[8]->color;
		Vector3f falloff8 = touchedLights[8]->falloff;
		
		on[8] = true;
		//sh.setParameter( "On8", true );
		polyShader.setParameter( "LightPos8", pos8 );
		polyShader.setParameter( "LightColor8", c8.r / 255.0, c8.g / 255.0, c8.b / 255.0, 1 );
		polyShader.setParameter( "Falloff8", falloff8 );
	}

	polyShader.setParameter( "On0", on[0] );
	polyShader.setParameter( "On1", on[1] );
	polyShader.setParameter( "On2", on[2] );
	polyShader.setParameter( "On3", on[3] );
	polyShader.setParameter( "On4", on[4] );
	polyShader.setParameter( "On5", on[5] );
	polyShader.setParameter( "On6", on[6] );
	polyShader.setParameter( "On7", on[7] );
	polyShader.setParameter( "On8", on[8] );
}

//save state to enter clone world
void GameSession::SaveState()
{
	stored.activeEnemyList = activeEnemyList;
	cloneInactiveEnemyList = NULL;

	Enemy *currEnemy = activeEnemyList;
	while( currEnemy != NULL )
	{
		currEnemy->SaveState();
		currEnemy = currEnemy->next;
	}
}

//reset from clone world
void GameSession::LoadState()
{
	Enemy *test = cloneInactiveEnemyList;
	int listSize = 0;
	while( test != NULL )
	{
		listSize++;
		test = test->next;
	}

	cout << "there are " << listSize << " enemies killed during the last clone process" << endl;


	//enemies killed while in the clone world
	Enemy *deadEnemy = cloneInactiveEnemyList;
	while( deadEnemy != NULL )
	{
		
		Enemy *next = deadEnemy->next;
		if( deadEnemy->spawnedByClone )
		{
			deadEnemy->Reset();
			//cout << "resetting dead enemy: " << deadEnemy << endl;
		}
		else
		{
			deadEnemy->LoadState();
			//cout << "loading dead enemy: " << deadEnemy << endl;
		}
		deadEnemy = next;
	}

	//enemies that are still alive
	Enemy *currEnemy = activeEnemyList;
	while( currEnemy != NULL )
	{		
		Enemy *next = currEnemy->next;
		if( currEnemy->spawnedByClone )
		{
			//cout << "resetting enemy: " << currEnemy << endl;
			currEnemy->Reset();
		}
		else
		{
			currEnemy->LoadState();
			//cout << "loading enemy: " << currEnemy << endl;
		}

		currEnemy = next;
	}

	//restore them all to their original state and then reset the list pointer

	//cloneInactiveEnemyList = NULL;
	activeEnemyList = stored.activeEnemyList;
}

void GameSession::Pause( int frames )
{
	pauseFrames = frames;
}

void GameSession::AllocateEffect()
{
	if( inactiveEffects == NULL )
	{
		inactiveEffects = new BasicEffect( this );
		inactiveEffects->prev = NULL;
		inactiveEffects->next = NULL;
	}
	else
	{
		BasicEffect *b = new BasicEffect( this ) ;
		b->next = inactiveEffects;
		inactiveEffects->prev = b;
		inactiveEffects = b;
	}
}

BasicEffect * GameSession::ActivateEffect( Tileset *ts, V2d pos, bool pauseImmune, double angle, int frameCount,
	int animationFactor, bool right )
{
	if( inactiveEffects == NULL )
	{
		return NULL;
	}
	else
	{
		BasicEffect *b = inactiveEffects;

		if( inactiveEffects->next == NULL )
		{
			inactiveEffects = NULL;
		}
		else
		{
			inactiveEffects = (BasicEffect*)(inactiveEffects->next);
			inactiveEffects->prev = NULL;
		}

		//assert( ts != NULL );
		b->Init( ts, pos, angle, frameCount, animationFactor, right );
		b->prev = NULL;
		b->next = NULL;
		b->pauseImmune = pauseImmune;

		AddEnemy( b );
		
		//cout << "activating: " << b << " blah: " << b->prev << endl;
		return b;
	}
}

void GameSession::DeactivateEffect( BasicEffect *b )
{
	//cout << "deactivate " << b << endl;
	RemoveEnemy( b );

	if( player.record == 0 )
	{
		if( inactiveEffects == NULL )
		{
			inactiveEffects = b;
			b->next = NULL;
			b->prev = NULL;
		}
		else
		{
			b->next = inactiveEffects;
			inactiveEffects->prev = b;
			inactiveEffects = b;
		}
	}
}

void GameSession::ResetEnemies()
{
	Enemy *curr = activeEnemyList;
	while( curr != NULL )
	{
		Enemy *temp = curr->next;
		if( curr->type == Enemy::BASICEFFECT )
		{
			DeactivateEffect( (BasicEffect*)curr );
		}

		curr = temp;
	}

	rReset( enemyTree->startNode );
}

void GameSession::rReset( QNode *node )
{
	if( node->leaf )
	{
		LeafNode *n = (LeafNode*)node;

		for( int i = 0; i < n->objCount; ++i )
		{			
			Enemy * e = (Enemy*)(n->entrants[i]);
			e->Reset();
			//cout << e->type << endl;
			
			//((Enemy*)node)->Reset();		
		}
	}
	else
	{
		//shouldn't this check for box touching box right here??
		ParentNode *n = (ParentNode*)node;

		for( int i = 0; i < 4; ++i )
		{
			rReset( n->children[i] );
		}
		
	}
}

void GameSession::LevelSpecifics()
{
	if( fileName == "test3" )
	{
		startSeq = new GameStartSeq( this );
		activeSequence = startSeq;
		//GameStartMovie();
		cout << "doing stuff here" << endl;
	}
	else
	{
	//	player.velocity.x = 60;
	}
}

//theres a bug on the new slope for the movie where you hold dash and up and you glitch out on a /\ slope. prob priority
void GameSession::GameStartMovie()
{
	startSeq = new GameStartSeq( this );
	activeSequence = startSeq;

	cout << "Starting movie" << endl;
	bool quit = false;
	//sf::View movieView( Vector2f( -460,  ), Vector2f( 960, 540 ) );
	sf::View movieView( sf::Vector2f( 480, 270 ), sf::Vector2f( 960, 540 ) );

	
	startSeq->shipSprite.setPosition( 480, 270 );
	sf::RectangleShape rs( Vector2f( 960, 540 ) );
	rs.setPosition( Vector2f( 0, 0 ) );
	rs.setFillColor( Color::Black );
	

	//View oldView = window->getView();

	player.velocity.x = 60;
	player.velocity.y = 0;
	player.hasDoubleJump = false;

	window->setView( movieView );
	while( !quit )
	{
		controller.UpdateState();
		currInput = controller.GetState();

		if( currInput.LRight() && !prevInput.LRight() )
		{
			break;
		}

		window->clear( Color::Black );

		window->setView( bgView );

		window->draw( background );

		window->setView( movieView );

		 
		//preScreenTex->setView( view );

		window->draw( rs );

		startSeq->stormSprite.setPosition( 0, -440 );
		window->draw( startSeq->stormSprite );
		startSeq->stormSprite.setPosition( 0, 440 );
		window->draw( startSeq->stormSprite );

		window->draw( startSeq->shipSprite );
		window->display();
	}

	//startSeq->shipSprite.setPosition( startSeq->startPos );
	//startSeq->stormSprite.setPosition( Vector2f( startSeq->startPos.x, startSeq->startPos.y + 200 ) );
	//window->setView( oldView );
}

PowerBar::PowerBar()
{
	pointsPerLayer = 240 * 10;
	maxLayer = 6;
	points = pointsPerLayer;//pointsPerLayer * ( maxLayer + 1 );
	layer = maxLayer;
	
	minUse = 1;
	
	panelTex.loadFromFile( "lifebar.png" );
	panelSprite.setTexture( panelTex );
	panelSprite.setScale( 4, 4 );
	panelSprite.setPosition( 10, 100 );

	//powerRect.setPosition( 42, 108 );
	//powerRect.setSize( sf::Vector2f( 4 * 4, 59 * 4 ) );
	//powerRect.setFillColor( Color::Green );
	//powerRect.

	maxRecover = 75;
	maxRecoverLayer = 0;
}

void PowerBar::Draw( sf::RenderTarget *target )
{
	//0x99a9b9
	Color c;
	/*switch( layer )
	{
	case 0:
		c = Color( 0, 0xee, 0xff );
		//c = Color( 0x00eeff );
		break;
	case 1:
		//c = Color( 0x0066cc );
		c = Color( 0, 0x66, 0xcc );
		break;
	case 2:
		c = Color( 0, 0xcc, 0x44 );
		break;
	case 3:
		c = Color( 0xff, 0xf0, 0 );
		break;
	case 4:
		c = Color( 0xff, 0xbb, 0 );
		break;
	case 5:
		c = Color( 0xff, 0x22, 0 );
		break;
	case 6:
		c = Color( 0xff, 0, 0xff );
		break;
	case 7:
		c = Color( 0xff, 0xff, 0xff );
		break;
	}*/
	c = Color( 0, 0xee, 0xff );

	double diffz = (double)points / (double)pointsPerLayer;
	assert( diffz <= 1 );
	diffz = 1 - diffz;
	diffz *= 60 * 4;

	sf::RectangleShape rs;
	rs.setPosition( 42, 108 + diffz );
	rs.setSize( sf::Vector2f( 4 * 4, 60 * 4 - diffz ) );
	rs.setFillColor( c );

	target->draw( panelSprite );
	target->draw( rs );
}

bool PowerBar::Damage( int power )
{
	points -= power;
	if( points <= 0 )
	{
		if( layer > 0 )
		{
			layer--;
			points = pointsPerLayer + points;
		}
		else
		{
			points = 0;
			return false;
		}
	}

	return true;
}

bool PowerBar::Use( int power )
{
	if( layer == 0 )
	{
		if( points - power < minUse )
		{
			return false;
		}
		else
		{
			points -= power;
		}
	}
	else
	{
		points -= power;
		if( points <= 0 )
		{
			points = pointsPerLayer + points;
			layer--;
		}
	}
	return true;
}

void PowerBar::Recover( int power )
{
	if( layer == maxRecoverLayer )
	{
		if( points + power > maxRecover )
		{
			points = maxRecover;
		}
		else
		{
			points += power;
		}
	}
	else
	{
		if( points + power > pointsPerLayer )
		{
			layer++;
			points = points + power - pointsPerLayer;
		}
		else
		{
			points += power;
		}
	}
}

void PowerBar::Charge( int power )
{
	if( layer == maxLayer )
	{
		if( points + power > pointsPerLayer )
		{
			points = pointsPerLayer;
		}
		else
		{
			points += power;
		}
	}
	else
	{
		if( points + power > pointsPerLayer )
		{
			layer++;
			points = points + power - pointsPerLayer;
		}
		else
		{
			points += power;
		}
	}
}

void Grass::HandleQuery( QuadTreeCollider *qtc )
{
	qtc->HandleEntrant( this );
}

bool Grass::IsTouchingBox( const Rect<double> &r )
{
	return isQuadTouchingQuad( V2d( r.left, r.top ), V2d( r.left + r.width, r.top ), 
		V2d( r.left + r.width, r.top + r.height ), V2d( r.left, r.top + r.height ),
		A, B, C, D );


	/*double left = min( edge->v0.x, edge->v1.x );
	double right = max( edge->v0.x, edge->v1.x );
	double top = min( edge->v0.y, edge->v1.y );
	double bottom = max( edge->v0.y, edge->v1.y );

	Rect<double> er( left, top, right - left, bottom - top );

	if( er.intersects( r ) )
	{
		return true;
	}*/
}

GameSession::GameStartSeq::GameStartSeq( GameSession *own )
	:stormVA( sf::Quads, 6 * 3 * 4 ) 
{
	owner = own;
	shipTex.loadFromFile( "ship.png" );
	shipSprite.setTexture( shipTex );
	shipSprite.setOrigin( shipSprite.getLocalBounds().width / 2, shipSprite.getLocalBounds().height / 2 );

	stormTex.loadFromFile( "stormclouds.png" );
	stormSprite.setTexture( stormTex );
	
	//shipSprite.setPosition( 250, 250 );
	startPos = Vector2f( owner->player.position.x, owner->player.position.y );
	frameCount = 1;//180;
	frame = 0;

	int count = 6;
	for( int i = 0; i < count; ++i )
	{
		Vector2f topLeft( startPos.x - 480, startPos.y - 270 );
		topLeft.y -= 540;

		topLeft.x += i * 960;

		stormVA[i*4].position = topLeft;
		stormVA[i*4].texCoords = Vector2f( 0, 0 );

		stormVA[i*4+1].position = topLeft + Vector2f( 0, 540 );
		stormVA[i*4+1].texCoords = Vector2f( 0, 540 );

		stormVA[i*4+2].position = topLeft + Vector2f( 960, 540 );
		stormVA[i*4+2].texCoords = Vector2f( 960, 540 );

		stormVA[i*4+3].position = topLeft + Vector2f( 960, 0 );
		stormVA[i*4+3].texCoords = Vector2f( 960, 0 );

		
		


		topLeft.y += 440 + 540;

		stormVA[i*4 + 4 * count].position = topLeft;
		stormVA[i*4 + 4 * count].texCoords = Vector2f( 0, 0 );

		stormVA[i*4+1+4 * count].position = topLeft + Vector2f( 0, 540 );
		stormVA[i*4+1+4 * count].texCoords = Vector2f( 0, 540 );

		stormVA[i*4+2+4 * count].position = topLeft + Vector2f( 960, 540 );
		stormVA[i*4+2+4 * count].texCoords = Vector2f( 960, 540 );

		stormVA[i*4+3+4 * count].position = topLeft + Vector2f( 960, 0 );
		stormVA[i*4+3+4 * count].texCoords = Vector2f( 960, 0 );

		topLeft.y += 540;
		stormVA[i*4 + 4 * count * 2].position = topLeft;
		stormVA[i*4 + 4 * count * 2].texCoords = Vector2f( 0, 0 );

		stormVA[i*4+1 + 4 * count * 2].position = topLeft + Vector2f( 0, 540 );
		stormVA[i*4+1 + 4 * count * 2].texCoords = Vector2f( 0, 540 );

		stormVA[i*4+2 + 4 * count * 2].position = topLeft + Vector2f( 960, 540 );
		stormVA[i*4+2 + 4 * count * 2].texCoords = Vector2f( 960, 540 );

		stormVA[i*4+3 + 4 * count * 2].position = topLeft + Vector2f( 960, 0 );
		stormVA[i*4+3 + 4 * count * 2].texCoords = Vector2f( 960, 0 );
	}
}

bool GameSession::GameStartSeq::Update()
{
	if( frame < frameCount )
	{
		
		V2d vel( 60, 0 );
		//if( frame > 60 )
			//vel.y = -20;

		shipSprite.setPosition( startPos.x + frame * vel.x, startPos.y + frame * vel.y );
		++frame;

		return true;
	}
	else 
		return false;
}

void GameSession::GameStartSeq::Draw( sf::RenderTarget *target )
{
	target->setView( owner->bgView );
	target->draw( owner->background );
	target->setView( owner->view );

	target->setView( owner->uiView );
	owner->powerBar.Draw( target );

	target->setView( owner->view );
	/*sf::RectangleShape rs( Vector2f( 960 * 4, 540 ) );
	rs.setPosition( Vector2f( startPos.x - 480, startPos.y - 270 ) );
	rs.setFillColor( Color::Black );
	target->draw( rs );*/


	//target->draw( stormVA, &stormTex );

	//target->draw( shipSprite );

}

bool GameSession::SetGroundPar()
{	
	int widthFactor = 8;
	float yView = view.getCenter().y / widthFactor;
	//cout << "yView << " << yView << endl;
	int tileHeight = 1045 / 2;//540;

	if( yView > 1080 || yView < -tileHeight )
	{
		return false;
	}

	
	
	bool flipped = false;
	int a = ((int)view.getCenter().x) % (1080 * widthFactor);
	double ratio = a / (1080.0 * widthFactor);
	if( ratio < 0 )
		ratio = 1 + ratio;

	int b = ((int)view.getCenter().x) % (1080 * widthFactor * 2);
	double ratiob = b / (1080.0 * widthFactor );
	if( ratiob < 0 )
		ratiob = 2 + ratiob;

	if( ratiob > ratio + .001 )
	{
		flipped = true;
	}
	//cout << "ratio: " << ratio << ", ratiob: " << ratiob << endl;

	int i = 0;
	if( flipped )
	{
		i = 1;
	}

	ratio = 1 - ratio;

	groundPar[i*4].position = Vector2f( 0, 1080 - tileHeight );
	groundPar[i*4+1].position = Vector2f( 1920 * ratio, 1080 - tileHeight );
	groundPar[i*4+2].position = Vector2f( 1920 * ratio, 1080 );
	groundPar[i*4+3].position = Vector2f( 0, 1080 );

	groundPar[i*4].texCoords = Vector2f( 0, tileHeight * i );
	groundPar[i*4 + 1].texCoords = Vector2f( 1920 * ratio, tileHeight * i );
	groundPar[i*4 + 2].texCoords = Vector2f( 1920 * ratio, tileHeight * (i + 1) );
	groundPar[i*4 + 3].texCoords = Vector2f( 0, tileHeight * (i + 1) );

	if( flipped )
	{
		i = 0;
	}
	else
	{
		i = 1;
	}

	groundPar[i*4].position = Vector2f( 1920 * ratio , 1080 - tileHeight );
	groundPar[i*4+ 1].position = Vector2f( 1920, 1080 - tileHeight );
	groundPar[i*4+2].position = Vector2f( 1920, 1080 );
	groundPar[i*4+3].position = Vector2f( 1920 * ratio , 1080 );

	groundPar[i*4].texCoords = Vector2f( 1920 * ratio, tileHeight * i );
	groundPar[i*4+1].texCoords = Vector2f( 1920, tileHeight * i );
	groundPar[i*4+2].texCoords = Vector2f( 1920, tileHeight * (i+1) );
	groundPar[i*4+3].texCoords = Vector2f( 1920 * ratio, tileHeight * (i+1) );

	
	cloudView.setCenter( 960, 540 + yView );
	preScreenTex->setView( cloudView );

	return true;
}