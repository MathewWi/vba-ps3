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

bool VbaAudio::enable_network(const std::string& host)
{
   m_net = true;
   m_host = host;

}

bool VbaAudio::networked() const
{
   return m_net;
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

   if (m_net)
   {
      _cellAudio = new Audio::RSound<int16_t>(m_host, 2, m_rate);
      if (!_cellAudio->alive())
      {
         delete _cellAudio;
         _cellAudio = new Audio::AudioPort<int16_t>(2, m_rate);
         m_net = false;
      }
   }
   else
   {
      _cellAudio = new Audio::AudioPort<int16_t>(2, m_rate);
   }

   return true;
}


void VbaAudio::pause()
{
	LOG_DBG("pause()\n");
   _cellAudio->pause();
}


void VbaAudio::reset()
{
	LOG_DBG("reset()\n");
   init(m_rate);
}


void VbaAudio::resume()
{
	LOG_DBG("resume()\n");
   _cellAudio->unpause();
}


void VbaAudio::write(u16 * finalWave, int length)
{
	_cellAudio->write((s16*)finalWave, length / 2);
}
