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
	fex_close(_curFex);
	_curFex = NULL;
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

	fex_err_t err = NULL;

	if (_curFex != NULL)
	{
		fex_close(_curFex);
		_curFex = NULL;
	}

	// clear old
	while (!_dir.empty()) { _dir.pop(); }
	_zipMap.clear();

	// reset index!
	_currentDirIndex = 0;

	// open using fex, handles extension detection and all
	fex_open(&_curFex, filename.c_str());

	// parse the zip data
	while (!fex_done(_curFex))
	{
		// stat the new file now
		fex_stat(_curFex);

		struct ZipEntry entry;
		std::string name = fex_name(_curFex);
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
			//LOG_DBG("ZipIO: dir found!\n");

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

				//LOG_DBG("ZipIO: adding dir: %s\n", path.c_str());
				//LOG_DBG("ZipIO: previous path: %s\n", prevPath.c_str());
				entry.name = path;
				entry.pos = -1;
				entry.type = ZIPIO_TYPE_DIR;
				entry.crc = 0;
				entry.len = 0;
				_zipMap[prevPath].push_back(entry);
			}
		}

		entry.name = fn;
		entry.pos = fex_tell_arc(_curFex);
		entry.type = ZIPIO_TYPE_FILE;
		entry.len = fex_size(_curFex);
		entry.crc = fex_crc32(_curFex);

		_zipMap[path].push_back(entry);

		LOG_DBG("ZipIO: Added Entry (name:%s, pos:%d, type:%d, len:%d, crc32:%d)\n", entry.name.c_str(), entry.pos, entry.type, entry.len, entry.crc);

		err = fex_next(_curFex);
		if (err != NULL)
		{
			LOG_WRN("VbaZipIO:GetEntryData() -- ERROR COULD NOT NEXT \t %s\n;", fex_err_str( err ));
		}
	}

	err = fex_rewind(_curFex);
	if (err != NULL)
	{
		LOG_WRN("VbaZipIO:GetEntryData() -- ERROR COULD NOT REWIND \t %s\n;", fex_err_str( err ));
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


//int VbaZipIo::GetEntryData(void** pData)
int VbaZipIo::GetEntryData(const void** pData)
{
	LOG_DBG("VbaZipIo::GetEntryData()\n");

	ZipEntry entry = _zipMap[_dir.top()][_currentDirIndex];
	fex_err_t err = NULL;

	/*err = fex_rewind(_curFex);
	if (err != NULL)
	{
		LOG_WRN("VbaZipIO:GetEntryData() -- ERROR 1: \t %s\n;", fex_err_str( err ));
		return 0;
	}

	int nCurrFile = 1;
	int nEntry = entry.pos;
	int nLength = entry.len;

	// Now step through to the file we need
	LOG_WRN("VbaZipIO:GetEntryData() -- SEEK ENTRY : %d\n", nEntry);
	while (nCurrFile < nEntry)
	{
			err = fex_next(_curFex);
			if (err != NULL || fex_done(_curFex))
			{
				LOG_WRN("VbaZipIO:GetEntryData() -- ERROR 1: \t %s\n;", fex_err_str( err ));
				return 0;
			}
			nCurrFile++;
	}

	LOG_WRN("VbaZipIO:GetEntryData() -- MEMORY ALLOC %d\n", nLength);
	*pData = SystemMalloc(nLength);
	if (*pData == NULL)
	{
		LOG_WRN("VbaZipIO:GetEntryData() -- MEMORY ERROR\n;");
		return 0;
	}

	LOG_WRN("VbaZipIO:GetEntryData() -- FEX READ %d\n", nLength);
    err = fex_read(_curFex, *pData, nLength);
    if (err != NULL)
    {
    	free(*pData);
    	LOG_WRN("VbaZipIO:GetEntryData() -- READ ERROR 1: \t %s\n;", fex_err_str( err ));
        return 0;
    }

    return nLength;*/



	LOG_WRN("VbaZipIO:GetEntryData() -- SEEK BEGIN (pos:%d)\n;", entry.pos);
	err = fex_seek_arc(_curFex, entry.pos);
	if (err != NULL)
	{
		LOG_WRN("VbaZipIO:GetEntryData() -- SEEK FAILED \t %s\n;", fex_err_str( err ));
		return 0;
	}
	LOG_WRN("VbaZipIO:GetEntryData() -- SEEK END\n;");

	//
	//int res = size;
	//int res = 1;
	//while(res < size)
	//	res <<= 1;

	LOG_DBG("VbaZipIo::GetEntryData() - Getting Data for Entry (%s, %d, %d), \n", entry.name.c_str(), entry.pos, entry.type);


	err = fex_data(_curFex, pData);
	LOG_DBG("VbaZipIo::GetEntryData() - fex_data err: %s\n", fex_err_str( err ));

	//fex_stat(_curFex);
	int size = fex_size(_curFex);
	LOG_DBG("VbaZipIo::GetEntryData() -- size: %d\n", size);

	return size;
}
