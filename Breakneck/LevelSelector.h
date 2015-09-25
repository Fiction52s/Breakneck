#ifndef __LEVEL_SELECTOR_H__
#define __LEVEL_SELECTOR_H__

#include <boost/filesystem.hpp>
#include <list>
#include <string>

struct LevelSelector
{
	void UpdateMapList();
	std::list<std::string> maps;
};


#endif