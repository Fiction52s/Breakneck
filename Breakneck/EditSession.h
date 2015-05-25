#include <string>
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include <list>
#include "VectorMath.h"

#ifndef __EDIT_SESSION__
#define __EDIT_SESSION__


struct Polygon
{
	Polygon();
	~Polygon();
	std::list<sf::Vector2i> points;
	std::string material;
	void Finalize();
	void Reset();
	void Draw( sf::RenderTarget * rt);
	void FixWinding();
	bool IsClockwise();
	bool ContainsPoint( sf::Vector2f p );
	void SetSelected( bool select );
	bool IsTouching( Polygon *p );
	sf::Vertex *lines;
	sf::VertexArray *va;
	int vaSize;
	bool selected;
	int left;
	int right;
	int top;
	int bottom;
};

struct EditSession
{
	EditSession( sf::RenderWindow *w);
	~EditSession();
	
	int Run(std::string fileName, 
		sf::Vector2f cameraPos, 
		sf::Vector2f cameraSize );
	void Draw();
	bool OpenFile( std::string fileName );
	void WriteFile(std::string fileName);
	double minimumEdgeLength;
	std::list<Polygon*> polygons;
	sf::Vector2i playerPosition;
	sf::Vector2i goalPosition;
	Polygon *polygonInProgress;
	sf::RenderWindow *w;
	std::string mode;
	std::string currentFile;
	double zoomMultiple;
	void Add( Polygon *brush, Polygon *poly);
	void Add2( Polygon *brush, Polygon *poly);
	//std::string polygonTool;
	std::list<sf::VertexArray*> progressDrawList;
	bool PointValid( sf::Vector2i prev, sf::Vector2i point );
	static LineIntersection SegmentIntersect( sf::Vector2i a, 
		sf::Vector2i b, sf::Vector2i c, 
		sf::Vector2i d );
	
};



#endif