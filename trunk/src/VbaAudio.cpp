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

bool VbaAudio::enable_network(bool enable, const std::string& host)
{
   m_net = enable;
   m_host = host;
   if (_cellAudio)
   {
      delete _cellAudio;
      _cellAudio = NULL;
   }

   if (m_net)
   {
      _cellAudio = new Audio::RSound<int16_t>(m_host, 2, m_rate);
      if (!_cellAudio->alive())
      {
         delete _cellAudio;
         _cellAudio = new Audio::AudioPort<int16_t>(2, m_rate);
         return false;
      }
   }
   _cellAudio = new Audio::AudioPort<int16_t>(2, m_rate);
   return true;
}


bool VbaAudio::init(long sampleRate)
{
	LOG_DBG("init(%d)\n", sampleRate);

	if (_cellAudio)
	{
		delete _cellAudio;
		_cellAudio = NULL;
	}

   m_rate = sampleRate;
   return enable_network(m_net, m_host);
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
