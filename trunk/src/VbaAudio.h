/*
 * VbaAudio.h
 *
 *  Created on: Nov 17, 2010
 *      Author: halsafar
 */

#ifndef VBAAUDIO_H_
#define VBAAUDIO_H_

#include "vba/common/SoundDriver.h"

#include "sys/types.h"

#include "cellframework/logger/Logger.h"
#include "cellframework/audio/audioport.hpp"
#include "cellframework/audio/rsound.hpp"
#include <string>


class VbaAudio : public SoundDriver
{
public:
	VbaAudio();
	virtual ~VbaAudio();

	bool init(long sampleRate);   // initialize the primary and secondary sound buffer
	void pause();  // pause the secondary sound buffer
   bool enable_network(const std::string& host);
   bool networked() const;
	void reset();  // stop and reset the secondary sound buffer
	void resume(); // resume the secondary sound buffer
	void write(u16 * finalWave, int length);  // write the emulated sound to the secondary sound buffer
private:
	Audio::Stream<int16_t> *_cellAudio;
   bool m_net;
   unsigned m_rate;
   std::string m_host;
};


#endif /* VBAAUDIO_H_ */
