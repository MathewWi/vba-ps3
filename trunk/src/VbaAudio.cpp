/*
 * VbaAudio.cpp
 *
 *  Created on: Nov 17, 2010
 *      Author: halsafar
 */

#include "VbaAudio.h"



VbaAudio::VbaAudio()
{
	LOG_DBG("VbaAudio()\n");

	_cellAudio = NULL;
	//CellAudio = new Audio::AudioPort<int32_t>(1, AUDIO_INPUT_RATE);
	//CellAudio->set_float_conv_func(Emulator_Convert_Samples);
}


VbaAudio::~VbaAudio()
{
	LOG_DBG("~VbaAudio()\n");
	if (_cellAudio)
	{
		delete _cellAudio;
		_cellAudio = NULL;
	}
}


bool VbaAudio::init(long sampleRate)
{
	LOG_DBG("init(%d)\n", sampleRate);

	if (_cellAudio)
	{
		delete _cellAudio;
		_cellAudio = NULL;
	}

	//_cellAudio = new Audio::AudioPort<u16>(1, sampleRate);
	//_cellAudio->set_float_conv_func(Emulator_Convert_Samples);
}


void VbaAudio::pause()
{
	LOG_DBG("pause()\n");
}


void VbaAudio::reset()
{
	LOG_DBG("reset()\n");
}


void VbaAudio::resume()
{
	LOG_DBG("resume()\n");
}


void VbaAudio::write(u16 * finalWave, int length)
{
	//_cellAudio->write((u16*)finalWave, length);
}
