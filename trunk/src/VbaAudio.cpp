/*
 * VbaAudio.cpp
 *
 *  Created on: Nov 17, 2010
 *      Author: halsafar
 */

#include "VbaAudio.h"
#include "VbaPs3.h"

VbaAudio::VbaAudio() : _cellAudio(NULL), m_host("0.0.0.0"), m_net(false), m_blocking(true)
{
	LOG_DBG("VbaAudio()\n");
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
   LOG_DBG("Enabling network. %s\n", host.c_str());
   m_net = true;
   m_host = host;
}

bool VbaAudio::networked() const
{
   return m_net;
}

void VbaAudio::nonblock(bool enable)
{
   m_blocking = !enable;
}

bool VbaAudio::init(long sampleRate)
{
	LOG_DBG("init(%ld)\n", sampleRate);

	if (_cellAudio)
	{
		delete _cellAudio;
		_cellAudio = NULL;
	}

   m_rate = sampleRate;

   if (m_net)
   {
      LOG_DBG("Starting RSound\n");
      _cellAudio = new Audio::RSound<int16_t>(m_host, 2, m_rate);
      if (!_cellAudio->alive())
      {
         LOG_DBG("Failed starting RSound.\n");
         delete _cellAudio;
         _cellAudio = new Audio::AudioPort<int16_t>(2, m_rate);
         m_net = false;
         VbaPs3::RSound_Error();
      }
   }
   else
   {
      LOG_DBG("Starting AudioPort\n");
      _cellAudio = new Audio::AudioPort<int16_t>(2, m_rate);
   }

   return true;
}


void VbaAudio::pause()
{
	LOG_DBG("pause()\n");
   if (_cellAudio)
      _cellAudio->pause();
}


void VbaAudio::reset()
{
	LOG_DBG("reset()\n");
   if (_cellAudio)
      init(m_rate);
}


void VbaAudio::resume()
{
	LOG_DBG("resume()\n");
   if (_cellAudio)
      _cellAudio->unpause();
}


void VbaAudio::write(u16 * finalWave, int length)
{
   if (_cellAudio)
   {
      if (m_blocking)
      {
         _cellAudio->write((s16*)finalWave, length / 2);
      }
      else
      {
         size_t avail = _cellAudio->write_avail();
         _cellAudio->write((s16*)finalWave, avail < (length / 2) ? avail : (length / 2));
      }
   }
}
