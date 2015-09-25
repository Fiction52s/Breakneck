#include "LevelSelector.h"
#include <iostream>

using namespace boost::filesystem;
using namespace std;

void LevelSelector::UpdateMapList()
{
	
	path p( current_path() / "/Maps" );

	vector<path> v;
	try
	{
		if (exists(p))    // does p actually exist?
		{
			if (is_regular_file(p))        // is p a regular file?   
				cout << p << " size is " << file_size(p) << '\n';

			else if (is_directory(p))      // is p a directory?
			{
				//cout << p << " is a directory containing:\n";


				copy(directory_iterator(p), directory_iterator(), back_inserter(v));

				sort(v.begin(), v.end());             // sort, since directory iteration
														// is not ordered on some file systems
  
				
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

	for (vector<path>::const_iterator it (v.begin()); it != v.end(); ++it)
	{
		//cout << (*it).filename() << endl;
		if( (*it).extension().string() == ".brknk" )
		{
			string name = (*it).filename().string();
			maps.push_back( name );
			//cout << name.substr( 0, name.size() - 6 ) << endl;
		}
		//cout << "   " << *it << '\n';
	}
}

