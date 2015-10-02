#ifndef __LEVEL_SELECTOR_H__
#define __LEVEL_SELECTOR_H__

#include <boost/filesystem.hpp>
#include <list>
#include <string>
#include <SFML/Graphics.hpp>

struct TreeNode
{
	TreeNode *parent;
	TreeNode *next;
	std::list<boost::filesystem::path> files;
	std::list<TreeNode*> dirs;
	std::string name;
	boost::filesystem::path filePath;
	int GetLevel();
	std::string GetLocalPath();
};

struct LevelSelector
{
	LevelSelector( sf::Font & p_font );
	void UpdateMapList(TreeNode *parentNode, const std::string &relativePath);
	void UpdateMapList();
	void PrintDir( TreeNode * dir );
	void ClearEntries();
	void ClearEntries(TreeNode *n);
	void Draw( sf::RenderTarget *target );
	void MouseUpdate( sf::Vector2i mousePos );
	void LeftClick( bool click, sf::Vector2i mousePos );
	//std::list<std::string> maps;
	TreeNode *entries;
	void Print();
	int numTotalEntries;
	int Tex(int index, int level, TreeNode *entry);
	sf::Font font;
	sf::Text * text;
	std::string * localPaths;
	TreeNode **dirNode;
	int mouseOverIndex;
	int selectedIndex;
	sf::Vector2f position;
	bool mouseDown;
	//sf::Vector2f mouseDownPos;
	int mouseDownIndex;
	int width;
	int height;
	int fontHeight;
	int xspacing;
	int yspacing;
	sf::Vector2f windowStretch;
	std::string newLevelName;

	sf::RenderTexture drawPanel;
};


#endif