/*
 * VbaZipIo.h
 *
 *  Created on: Nov 24, 2010
 *      Author: halsafar
 */

#ifndef VBAZIPIO_H_
#define VBAZIPIO_H_

#include <stack>
#include <string>
#include <map>
#include <vector>

#include "utils/fex/fex.h"
//#include "utils/fex/Zip_Extractor.h"
//#include "utils/fex/Zip7_Extractor.h"
//#include "utils/fex/Rar_Extractor.h"
//#include "utils/fex/Gzip_Extractor.h"

#define ZIPIO_TYPE_DIR 0
#define ZIPIO_TYPE_FILE 1

struct ZipEntry
{
	std::string name;
	fex_pos_t pos;
	int type;
};

class VbaZipIo
{
public:
	VbaZipIo();
	~VbaZipIo();

	void Open(std::string filename);
	void PushDir(std::string dir);
	void PopDir();
	size_t GetCurrentEntryCount();
	void SetCurrentEntryPosition(size_t index);

	int GetEntryData(const void** pData);

	ZipEntry GetCurrentEntry()
	{
		return _zipMap[_dir.top()][_currentDirIndex];
	}

	size_t GetCurrentEntryIndex()
	{
		return _currentDirIndex;
	}

	//int GetCurrentEntrySize();

	size_t GetDirStackCount()
	{
		return _dir.size();
	}

	ZipEntry operator[](uint32_t i)
	{
		return _zipMap[_dir.top()][i];
	}
private:
	fex_t* _curFex;

	std::stack<std::string> _dir;
	std::size_t _currentDirIndex;
	std::map<std::string, std::vector<struct ZipEntry> > _zipMap;
};

#endif /* VBAZIPIO_H_ */
