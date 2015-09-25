#ifndef __LEVEL_SELECTOR_H__
#define __LEVEL_SELECTOR_H__

#include <boost/filesystem.hpp>
#include <list>
#include <string>

struct TreeNode
{
	TreeNode *parent;
	TreeNode *next;
	std::list<std::string> files;
	std::list<TreeNode*> dirs;
	std::string name;
};

struct LevelSelector
{
	LevelSelector();
	void UpdateMapList(TreeNode *parentNode, const std::string &relativePath);
	void UpdateMapList();
	void PrintDir( TreeNode * dir );


	//std::list<std::string> maps;
	TreeNode *entries;
	void AddEntry( TreeNode *entry );
	void Print();
};


#endif