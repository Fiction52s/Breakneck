#ifndef __EDIT_SESSION__
#define __EDIT_SESSION__
#include <string>
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include <list>

struct Polygon
{
	std::list<sf::Vector2f> points;
	std::string material;
	void Finalize();
	void Draw( sf::RenderTarget * rt);
	void FixWinding();
	bool IsClockwise();
	sf::Vertex *lines;
};

struct EditSession
{
	EditSession( sf::RenderWindow *w);
	void Run(std::string fileName );
	void Draw();
	bool OpenFile( std::string fileName );
	void WriteFile(std::string fileName);
	std::list<Polygon*> polygons;
	sf::Vector2f playerPosition;
	Polygon *polygonInProgress;
	sf::RenderWindow *w;
	std::string mode;
	std::string currentFile;
	
	
};



#endif