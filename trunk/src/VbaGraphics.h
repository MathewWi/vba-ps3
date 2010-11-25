/*
 * VbaGraphics.h
 *
 *  Created on: Nov 4, 2010
 *      Author: halsafar
 */

#ifndef VbaGraphics_H_
#define VbaGraphics_H_

#include <string>
#include <vector>
using namespace std;

#include <sys/types.h>
#include <PSGL/psgl.h>
#include <PSGL/psglu.h>

#include "cellframework/graphics/GraphicsTypes.h"
#include "cellframework/graphics/PSGLGraphics.h"

// FIXME: Old copy-paste cruft from FCEU
#define FCEU_RENDER_TEXTURE_WIDTH (256.0)
#define FCEU_RENDER_TEXTURE_HEIGHT (256.0)

#define NES_ASPECT_RATIO SCREEN_4_3_ASPECT_RATIO

#define SCREEN_16_9_ASPECT_RATIO (16.0/9)
#define SCREEN_4_3_ASPECT_RATIO (4.0/3)

#define SCREEN_RENDER_TEXTURE_WIDTH FCEU_RENDER_TEXTURE_WIDTH
#define SCREEN_RENDER_TEXTURE_HEIGHT FCEU_RENDER_TEXTURE_HEIGHT

#define SCREEN_REAL_ASPECT_RATIO NES_ASPECT_RATIO
#define SCREEN_RENDER_TEXTURE_PITCH FCEU_SCREEN_PITCH

#ifdef EMUDEBUG
	#ifdef PSGL
	#define EMU_DBG(fmt, args...) do {\
	   gl_dprintf(0.1, 0.1, 1.0, fmt, ##args);\
	   sys_timer_usleep(EMU_DBG_DELAY);\
	   } while(0)
	#else
	#define EMU_DBG(fmt, args...) do {\
	   cellDbgFontPrintf(0.1f, 0.1f, DEBUG_FONT_SIZE, RED, fmt, ##args);\
	   sys_timer_usleep(EMU_DBG_DELAY);\
	   } while(0)
	#endif
#else
#define EMU_DBG(fmt, args...) ((void)0)
#endif

// default shader sources
#define DEFAULT_SHADER_FILE  "/dev_hdd0/game/VBAM90000/USRDIR/shaders/stock.cg"


// PC Palette
struct pcpal {
    unsigned char r;
    unsigned char g;
    unsigned char b;
};

class VbaGraphics : public PSGLGraphics
{
public:
	VbaGraphics();
	~VbaGraphics();

	void SetDimensions(unsigned width, unsigned height, unsigned pitch);
	void SetRect(const Rect &view);
	uint32_t *MapPixels();
	void UnmapPixels();

	void SetAspectRatio(float ratio);
   void SetStretched(bool stretch);
	void SetSmooth(bool smooth);
	void SetPAL60Hz(bool pal60Hz);
	void Clear() const;
	void Draw() const;
	void Draw(uint8_t *XBuf);
	void FlushDbgFont() const;
	void Swap() const;
	void Init();
	void Init(uint32_t resId);
	void DeInit();
	void Printf(float x, float y, const char *fmt, ...) const;
	void DbgPrintf(const char *fmt, ...) const;
	void Sleep(uint64_t usec) const;

	string GetFragmentShaderPath() { return _curFragmentShaderPath; }
	CellVideoOutState GetVideoOutState();

	int CheckResolution(uint32_t resId);
	void SwitchResolution();
	void SwitchResolution(uint32_t resId);
	uint32_t GetInitialResolution();
	uint32_t GetCurrentResolution();
	void PreviousResolution();
	void NextResolution();
	int AddResolution(uint32_t resId);
	void SetOverscan(bool overscan, float amount = 0.0f);
	int32_t InitCg();
	int32_t LoadFragmentShader(string shaderPath);
	void UpdateCgParams(unsigned width, unsigned height, unsigned tex_width, unsigned tex_height);
	void UpdateCgParams();

	pcpal Palette[256];
private:
	void PSGLInit();
	CellVideoOutState stored_video_state;
	CellVideoOutState _videoOutState;
	void GetAllAvailableResolutions();
	int32_t ChangeResolution(uint32_t resId);
	std::vector<uint32_t> supportedResolutions;
	int currentResolution;
	uint32_t initialResolution;
	unsigned m_width;
	unsigned m_height;
	unsigned m_pitch;
	bool m_overscan;
	float m_overscan_amount;
	uint32_t *m_pixels;
	float m_ratio;
	bool m_smooth;
	bool m_pal60Hz;
	GLuint pbo, vbo;
	uint32_t *gl_buffer;
	uint8_t *vertex_buf;
   mutable float m_fps;
   mutable uint64_t m_frames;
   mutable uint64_t last_time;

	string _curFragmentShaderPath;

	CGcontext		_cgContext;

	CGprogram 		_vertexProgram;
	CGprogram 		_fragmentProgram;

	CGparameter  	_cgpModelViewProj;
	CGparameter		_cgpDiffuseMap;

	CGparameter _cgpVideoSize;
	CGparameter _cgpTextureSize;
	CGparameter _cgpOutputSize;

	GLuint _cgViewWidth;
	GLuint _cgViewHeight;

	void set_aspect();
};



#endif /* VbaGraphics_H_ */
