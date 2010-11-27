/*
 * VbaZipIo.cpp
 *
 *  Created on: Nov 24, 2010
 *      Author: halsafar
 */
#include "VbaZipIo.h"

#include "vba/System.h"

#include "cellframework/fileio/FileBrowser.h"
#include "cellframework/logger/Logger.h"



VbaZipIo::VbaZipIo()
{
	_curFex = NULL;
	_currentDirIndex = 0;
}


VbaZipIo::~VbaZipIo()
{
	if (_curFex != NULL)
	{
		_curFex->close();
		delete _curFex;
		_curFex = NULL;
	}
}


//
// Open(filename)
//
// Parses a 7z file, assumes the extractor browser reports names:
//	File1.ext
//	File2.ext
//	Dir1/File1.ext
//	Dir1/DirA/File1.ext
//
// This builds a dir/file mapping which can be browsed
//
void VbaZipIo::Open(std::string filename)
{
	LOG_DBG("VbaZipIo::Open(%s)\n", filename.c_str());

	if (_curFex != NULL)
	{
		_curFex->close();
		delete _curFex;
		_curFex = NULL;
	}

	// discover interface for current fex class
	if (FileBrowser::GetExtension(filename).compare("7z") == 0)
	{
		_curFex = new Zip7_Extractor();
	}
	else if (FileBrowser::GetExtension(filename).compare("zip") == 0)
	{
		_curFex = new Zip_Extractor();
	}
	/*else if (FileBrowser::GetExtension(filename).compare("rar") == 0)
	{
		_curFex = new Rar_Extractor();
	}*/
	else if (FileBrowser::GetExtension(filename).compare("gz") == 0)
	{
		_curFex = new Gzip_Extractor();
	}

	// clear old
	while (!_dir.empty()) { _dir.pop(); }
	_zipMap.clear();
	_curFex->close();
	_currentDirIndex = 0;

	_curFex->open(filename.c_str());

	// parse the zip data
	while (!_curFex->done())
	{
		_curFex->stat();

		struct ZipEntry entry;
		std::string name = _curFex->name();
		std::string fn = name;
		std::string path = "";

		// extract this zip entries path for the mapping
		int slashIndex = name.find_last_of('/');

		// problem here...
		if (slashIndex == 0)
		{
			// should never have a slash at index 0
			LOG_DBG("ZipIO: very odd slash at start of zip filename!\n");
		}
		else if (slashIndex > 0)
		{
			LOG_DBG("ZipIO: dir found!\n");

			// rip out the filename
			fn = name.substr(slashIndex+1);

			// rip out the file's dir for the mapping
			path = name.substr(0, slashIndex);

			// path does not yet exist, add dir for it in entries
			if (_zipMap.find(path) == _zipMap.end())
			{
				// rip out the previous path
				int prevIndex = path.find_last_of('/');
				std::string prevPath = "";
				if (prevIndex > 0)
				{
					prevPath = name.substr(0, prevIndex);
				}

				LOG_DBG("ZipIO: adding dir: %s\n", path.c_str());
				LOG_DBG("ZipIO: previous path: %s\n", prevPath.c_str());
				entry.name = path;
				entry.pos = -1;
				entry.type = ZIPIO_TYPE_DIR;
				_zipMap[prevPath].push_back(entry);
			}
		}

		entry.name = fn;
		entry.pos = _curFex->tell_arc() ;
		entry.type = ZIPIO_TYPE_FILE;
		_zipMap[path].push_back(entry);

		LOG_DBG("ZipIO: Added Entry (%s, %d, %d)\n", entry.name.c_str(), entry.pos, entry.type);

		_curFex->next();
	}

	_dir.push("");
}


void VbaZipIo::PushDir(std::string dir)
{
	LOG_DBG("VbaZipIo::PushDir(%s)\n", dir.c_str());

	// check if key does not exists
	if (_zipMap.find(dir) == _zipMap.end())
	{
		return;
	}

	_dir.push(dir);
	_currentDirIndex = 0;
}


void VbaZipIo::PopDir()
{
	LOG_DBG("VbaZipIo::PopDir()\n");

	if (!_dir.empty())
	{
		_dir.pop();
		_currentDirIndex = 0;
	}
}

size_t VbaZipIo::GetCurrentEntryCount()
{
	//LOG_DBG("VbaZipIo::GetCurrentEntryCount()\n");

	return _zipMap[_dir.top()].size();
}


void VbaZipIo::SetCurrentEntryPosition(size_t index)
{
	//LOG_DBG("VbaZipIo::SetCurrentEntryPosition(%u)\n", index);

	_currentDirIndex = index;
}


/*int VbaZipIo::GetCurrentEntrySize()
{
	_zip7.seek_arc(_zipMap[_dir.top()][_currentDirIndex].pos);
	_zip7.stat();
	int size = _zip7.size();

	// wtf?
	int res = 1;
	while(res < size)
		res <<= 1;

	return res;
}*/


int VbaZipIo::GetEntryData(uint8_t* &pData)
{
	LOG_DBG("VbaZipIo::GetEntryData()\n");

	ZipEntry entry = _zipMap[_dir.top()][_currentDirIndex];
	//_curFex->rewind();
	_curFex->seek_arc(entry.pos);
	_curFex->stat();

	int size = _curFex->size();

	//
	int res = size;
	//int res = 1;
	//while(res < size)
	//	res <<= 1;

	LOG_DBG("VbaZipIo::GetEntryData() - Getting Data for Entry (%s, %d, %d), \n", entry.name.c_str(), entry.pos, entry.type);
	//_curFex->((const void**)&pData);

	LOG_DBG("VbaZipIo::GetEntryData() -- size: %d\n", res);
	uint8_t	*data = (uint8_t *)SystemMalloc(res);

	_curFex->reader().read(data, res);

	// set out param
	pData = data;

	return res;
}