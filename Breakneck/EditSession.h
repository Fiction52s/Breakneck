#include <string>
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include <list>
#include "VectorMath.h"
#include "GUI.h"

#ifndef __EDIT_SESSION__
#define __EDIT_SESSION__


struct TerrainPolygon
{
	TerrainPolygon();
	~TerrainPolygon();
	std::list<sf::Vector2i> points;
	std::string material;
	void Finalize();
	void Reset();
	void Draw( sf::RenderTarget * rt);
	void FixWinding();
	bool IsClockwise();
	bool ContainsPoint( sf::Vector2f p );
	void SetSelected( bool select );
	bool IsTouching( TerrainPolygon *p );
	sf::Vertex *lines;
	sf::VertexArray *va;
	int vaSize;
	bool selected;
	int left;
	int right;
	int top;
	int bottom;
};



struct ActorType
{
	ActorType( const std::string & name, Panel *panel );
	std::string name;
	sf::Texture iconTexture;
	sf::Texture imageTexture;
	Panel *panel;
};

struct ActorParams
{
	void WriteFile( std::ofstream &of );
	//std::string SetAsPatroller( ActorType *t, sf::Vector2i pos, bool clockwise, float speed );
	std::string SetAsPatroller( ActorType *t, sf::Vector2i pos, bool clockwise, float speed );
	//sf::Sprite icon;
	sf::Sprite image;
	std::list<std::string> params;
	ActorType *type;
	void Draw( sf::RenderTarget *target );
};

struct ActorGroup
{
	ActorGroup( const std::string &name );
	std::string name;
	std::list<ActorParams*> actors;
	void Draw( sf::RenderTarget *target );
	void WriteFile( std::ofstream &of );
};

struct EditSession : GUIHandler
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
	std::list<TerrainPolygon*> polygons;
	sf::Vector2i playerPosition;
	sf::Vector2i goalPosition;
	TerrainPolygon *polygonInProgress;
	sf::RenderWindow *w;
	std::string mode;
	std::string currentFile;
	double zoomMultiple;
	void Add( TerrainPolygon *brush, TerrainPolygon *poly);	
	//std::string polygonTool;
	std::list<sf::VertexArray*> progressDrawList;
	bool PointValid( sf::Vector2i prev, sf::Vector2i point );
	static LineIntersection SegmentIntersect( sf::Vector2i a, 
		sf::Vector2i b, sf::Vector2i c, 
		sf::Vector2i d );
	sf::Vector2f testPoint;
	double minAngle;

 
	sf::Text polygonTimeoutText;
	int polygonTimeoutTextTimer;
	int polygonTimeoutTextLength;

	//static void TestButton();

	void ButtonCallback( Button *b, const std::string & e );
	void TextBoxCallback( TextBox *tb, const std::string & e );
	void GridSelectorCallback( GridSelector *gs, const std::string & e );
	
	std::map<std::string, ActorGroup*> groups;
	std::map<std::string, ActorType*> types;

	sf::Sprite enemySprite;
	ActorType *trackingEnemy;//bool trackingEnemy;
	Panel *showPanel;
	bool trackingEnemyDown;
	Panel *CreateOptionsPanel( const std::string &name );

	std::list<sf::Vector2i> patrolPath;

	enum Emode
	{
		CREATE_TERRAIN,
		EDIT,
		//PLACE_PLAYER,
		//PLACE_GOAL,
		SELECT_POLYGONS,
		PAUSED,
		CREATE_ENEMY,
		DRAW_PATROL_PATH
	};

	
};



#endif