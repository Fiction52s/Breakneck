#include "LevelSelector.h"
#include <iostream>

using namespace boost::filesystem;
using namespace std;

LevelSelector::LevelSelector()
{		
	entries = NULL;
	//entries = new TreeNode;
	//entries->name = "Maps";
	//entries->next = NULL;
	///entries->parent = NULL;
}

void LevelSelector::UpdateMapList()
{
	UpdateMapList( entries, "Maps" );
}

void LevelSelector::UpdateMapList( TreeNode *parentNode, const std::string &relativePath )
{

	path p( current_path() / relativePath );
	
	vector<path> v;
	try
	{
		if (exists(p))    // does p actually exist?
		{
			if (is_regular_file(p))        // is p a regular file?   
			{
				if( p.extension().string() == ".brknk" )
				{
					string name = p.filename().string();
					parentNode->files.push_back( name );
				}
			}
			else if (is_directory(p))      // is p a directory?
			{
				//cout << p << " is a directory containing:\n";

				TreeNode *newDir = new TreeNode;
				newDir->parent = parentNode;
				newDir->next = NULL;
				newDir->name = p.filename().string();

				copy(directory_iterator(p), directory_iterator(), back_inserter(v));

				sort(v.begin(), v.end());             // sort, since directory iteration
														// is not ordered on some file systems

				if( parentNode == NULL )
				{
					entries = newDir;
				}
				else
				{
					parentNode->dirs.push_back( newDir );
				}
			
				
				for (vector<path>::const_iterator it (v.begin()); it != v.end(); ++it)
				{
					UpdateMapList( newDir, relativePath + "/" + (*it).filename().string() );
					//cout << "   " << *it << '\n';
				}
			}
			else
				cout << p << " exists, but is neither a regular file nor a directory\n";
		}
		else
			cout << p << " does not exist\n";
	}
	catch (const filesystem_error& ex)
	{
		cout << ex.what() << '\n';
	}
}


void LevelSelector::AddEntry( TreeNode *entry )
{
	/*if( entries == NULL )
	{
		entry->next = NULL;
		entry->parent = NULL;
		entries = entry;
	}
	else
	{
		while( entries-
	}*/
}

void LevelSelector::Print()
{
	PrintDir( entries );
}

void LevelSelector::PrintDir( TreeNode * dir )
{
	cout << "directory: " << dir->name << endl;
	cout << "containing files: " << endl;
	for( list<string>::iterator it = dir->files.begin(); it != dir->files.end(); ++it )
	{
		cout << "-- " << (*it) << endl;
	}

	cout << "containing dirs: " << endl;
	for( list<TreeNode*>::iterator it = dir->dirs.begin(); it != dir->dirs.end(); ++it )
	{
		cout << "+ " << (*it)->name << endl;
		PrintDir( (*it) );
	}
}
