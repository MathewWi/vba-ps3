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



class VbaAudio : public SoundDriver
{
private:


public:
	VbaAudio();
	virtual ~VbaAudio();

	bool init(long sampleRate);   // initialize the primary and secondary sound buffer
	void pause();  // pause the secondary sound buffer
	void reset();  // stop and reset the secondary sound buffer
	void resume(); // resume the secondary sound buffer
	void write(u16 * finalWave, int length);  // write the emulated sound to the secondary sound buffer
};


#endif /* VBAAUDIO_H_ */
