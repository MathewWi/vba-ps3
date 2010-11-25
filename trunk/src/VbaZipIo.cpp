/*
 * VbaZipIo.cpp
 *
 *  Created on: Nov 24, 2010
 *      Author: halsafar
 */
#include "VbaZipIo.h"

#include "vba/System.h"

#include "cellframework/logger/Logger.h"



VbaZipIo::VbaZipIo()
{

}


VbaZipIo::~VbaZipIo()
{

}


void VbaZipIo::Open(std::string filename)
{
	LOG_DBG("VbaZipIo::Open(%s)\n", filename.c_str());

	// clear old
	while (!_dir.empty()) { _dir.pop(); }
	_zipMap.clear();
	_currentDirIndex = 0;

	_zip7.open(filename.c_str());

	while (!_zip7.done())
	{
		struct ZipEntry entry;
		std::string name = _zip7.name();
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

			// rip out the file's dir for the mapping
			path = name.substr(0, slashIndex);

			// path does not yet exist, add dir for it in entries
			if (_zipMap.find(path) == _zipMap.end())
			{
				LOG_DBG("ZipIO: adding dir: %s\n", path.c_str());
				entry.name = path;
				entry.pos = -1;
				entry.type = ZIPIO_TYPE_DIR;
				_zipMap[path].push_back(entry);
			}
		}

		entry.name = name;
		entry.pos = _zip7.tell_arc();
		entry.type = ZIPIO_TYPE_FILE;
		_zipMap[path].push_back(entry);

		_zip7.next();
	}

	_dir.push("");
}


void VbaZipIo::PushDir(std::string dir)
{
	LOG_DBG("VbaZipIo::PushDir(%s)\n", dir.c_str());

	// check if key exists
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
	}
}

size_t VbaZipIo::GetCurrentEntryCount()
{
	LOG_DBG("VbaZipIo::GetCurrentEntryCount()\n");

	return _zipMap[_dir.top()].size();
}


void VbaZipIo::SetCurrentEntryPosition(size_t index)
{
	LOG_DBG("VbaZipIo::SetCurrentEntryPosition(%u)\n", index);

	_currentDirIndex = index;
}


/*(int VbaZipIo::GetCurrentEntrySize()
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


uint8_t* VbaZipIo::GetEntryData()
{
	LOG_DBG("VbaZipIo::GetEntryData()\n");

	_zip7.seek_arc(_zipMap[_dir.top()][_currentDirIndex].pos);
	_zip7.stat();

	int size = _zip7.size();

	// wtf?
	int res = 1;
	while(res < size)
		res <<= 1;

	LOG_DBG("VbaZipIo::GetEntryData() -- res %d\n", res);
	uint8_t	*data = (uint8_t *)SystemMalloc(res);

	int read = res;//size;
	_zip7.reader().read(data, read);

	return data;
}
