/*
 * VbaAudio.cpp
 *
 *  Created on: Nov 17, 2010
 *      Author: halsafar
 */

#include "VbaAudio.h"

namespace Internal {
   static void Emulator_Convert_Samples(float * out, unsigned, const int16_t *in, size_t frames)
   {
      for (size_t i = 0; i < frames * 2; i++)
      {
         out[i] = (float)(in[i]) / 0x3000;
      }
   }
}


VbaAudio::VbaAudio()
{
	LOG_DBG("VbaAudio()\n");

	_cellAudio = NULL;
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

	_cellAudio = new Audio::AudioPort<int16_t>(2, sampleRate);
	//_cellAudio->set_float_conv_func(Internal::Emulator_Convert_Samples);
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
	_cellAudio->write((s16*)finalWave, length / 2);
}
