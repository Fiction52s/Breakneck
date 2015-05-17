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
	std::list<sf::Vector2i> points;
	std::string material;
	void Finalize();
	void Draw( sf::RenderTarget * rt);
	void FixWinding();
	bool IsClockwise();
	bool ContainsPoint( sf::Vector2f p );
	void SetSelected( bool select );
	sf::Vertex *lines;
	sf::VertexArray *va;
	int vaSize;
	bool selected;
};

struct EditSession
{
	EditSession( sf::RenderWindow *w);
	void Run(std::string fileName );
	void Draw();
	bool OpenFile( std::string fileName );
	void WriteFile(std::string fileName);
	std::list<Polygon*> polygons;
	sf::Vector2i playerPosition;
	Polygon *polygonInProgress;
	sf::RenderWindow *w;
	std::string mode;
	std::string currentFile;
	double zoomMultiple;
	
	
};



#endif