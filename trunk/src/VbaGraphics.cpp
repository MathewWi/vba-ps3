/*
 * VbaGraphics.cpp
 *
 *  Created on: Nov 4, 2010
 *      Author: halsafar
 */

#include "VbaGraphics.h"

#include <limits>
#include <assert.h>



#include "cellframework/logger/Logger.h"
#include "cellframework/utility/utils.h"

#include "vba/System.h"


// FIXME: make this use the faster ABGR_SCE instead of the easy RGBA

VbaGraphics::VbaGraphics() : PSGLGraphics(), gl_buffer(NULL), vertex_buf(NULL)
{
	m_smooth = true;
	m_pal60Hz = false;
	m_overscan = false;
	m_overscan_amount = 0.0f;
	m_ratio = 4.0/3.0;
}


VbaGraphics::~VbaGraphics()
{
	DeInit();
}


void VbaGraphics::DeInit()
{
	if (vertex_buf)
	{
		free(vertex_buf);
		vertex_buf = NULL;
	}

	if (gl_buffer)
	{
		free(gl_buffer);
		gl_buffer = NULL;
	}
}


void VbaGraphics::Swap() const
{
   // TODO: Make this a setting!
   m_frames++;

   if (m_frames >= 100)
   {
      uint64_t new_time = get_usec();
      m_fps = 100000000.0 / (new_time - last_time);

      last_time = new_time;
      m_frames = 0;
   }
   cellDbgFontPrintf(0.03, 0.03, 0.75, 0xffffffff, "FPS: %.2f\n", m_fps);
   cellDbgFontDraw();

   psglSwap();
}


void VbaGraphics::Draw() const
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_pitch/4.0, m_height, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, 0);
   // Dirty random hack is dirty. Check below ...

	glDrawArrays(GL_QUADS, 0, 4);
	glFlush();
	Swap();
}


void VbaGraphics::Draw(uint8_t *XBuf)
{
	//LOG("Draw(%d, %d)\n", m_width, m_height);

	Clear();

	// get ps3 texture
	//uint32_t* texture = MapPixels();


	// convert VBA buf to systemDepth, which is 32
	//u32 * palette = (u32 *)XBuf;
	//int palIndex = 0;
	//for (int i = 0; i < m_height; i++)
	//{
	//	for (int j = 0; j < m_width; j++)
	//	{
	//		palIndex = (i*pitch) + j;

			//if (Emula)
	//		texture[(i*m_width) + j] = palette[palIndex];
			//texture[(i*m_width) + j] = palette[palIndex] << 24 | palette[palIndex] << 16 | palette[palIndex] << 8 | 0xFF;
	//	}
	//}
   glBufferSubData(GL_TEXTURE_REFERENCE_BUFFER_SCE, 0, m_height * m_pitch, XBuf);
	//UnmapPixels();

	Draw();
}

void VbaGraphics::FlushDbgFont() const
{
	cellDbgFontDraw();
}


void VbaGraphics::Sleep(uint64_t usec) const
{
   sys_timer_usleep(usec);
}


void VbaGraphics::Clear() const
{
	glClear(GL_COLOR_BUFFER_BIT);
}


uint32_t* VbaGraphics::MapPixels()
{
   return (uint32_t*)glMapBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, GL_WRITE_ONLY);
}


void VbaGraphics::UnmapPixels()
{
   glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER_ARB);
}


void VbaGraphics::SetAspectRatio(float ratio)
{
   m_ratio = ratio;
   set_aspect();
}


void VbaGraphics::SetDimensions(unsigned width, unsigned height, unsigned pitch)
{
	assert(width > 0);
	assert(height > 0);
	assert(pitch > 0);

	m_width = width;
	m_height = height;
   SetAspectRatio(m_ratio);

	// calculate pitch for the VBA buffer, FIXME: verify the +1, which magically fixed it
	m_pitch = ((unsigned)((pitch / 4.0) + 1)) * 4;

	if (gl_buffer)
		free(gl_buffer);
	gl_buffer = (uint32_t*)memalign(128, m_height * m_pitch);
	memset(gl_buffer, 0, m_height * m_pitch);
	glBufferData(GL_TEXTURE_REFERENCE_BUFFER_SCE, m_height * m_pitch, gl_buffer, GL_STREAM_DRAW);
   glTextureReferenceSCE(GL_TEXTURE_2D, 1, m_width, m_height, 0, GL_ARGB_SCE, m_pitch, 0);
}


void VbaGraphics::SetRect(const Rect &view)
{
#if old_pitchypitch
   unsigned pitch = m_pitch >> 2;
   GLfloat tex_coords[] = {
      (float)view.x / pitch, (float)(view.y + view.h) / m_height,            // Upper Left
      (float)view.x / pitch, (float)view.y / m_height,                       // Lower Left
      (float)(view.x + view.w) / pitch, (float)view.y / m_height,            // Lower Right
      (float)(view.x + view.w) / pitch, (float)(view.y + view.h) / m_height  // Upper Right
   };
#endif
   GLfloat tex_coords[] = {
      (float)view.x / m_width, (float)(view.y + view.h) / m_height,            // Upper Left
      (float)view.x / m_width, (float)view.y / m_height,                       // Lower Left
      (float)(view.x + view.w) / m_width, (float)view.y / m_height,            // Lower Right
      (float)(view.x + view.w) / m_width, (float)(view.y + view.h) / m_height  // Upper Right
   };
   memcpy(vertex_buf + 128, tex_coords, 8 * sizeof(GLfloat));
	glBufferData(GL_ARRAY_BUFFER, 256, vertex_buf, GL_STATIC_DRAW);
}


void VbaGraphics::Printf(float x, float y, const char *fmt, ...) const
{
   va_list ap;
   va_start(ap, fmt);
   cellDbgFontVprintf(x, y, 1.0, 0xffffffff, fmt, ap);
   va_end(ap);
   cellDbgFontDraw();
}


void VbaGraphics::DbgPrintf(const char *fmt, ...) const
{
   //Clear();
   va_list ap;
   va_start(ap, fmt);
   cellDbgFontVprintf(0.1, 0.1, 1.0, 0xffffffff, fmt, ap);
   va_end(ap);
   cellDbgFontDraw();
   Swap();
   Sleep(600000);
}

int VbaGraphics::CheckResolution(uint32_t resId)
{
	return cellVideoOutGetResolutionAvailability(CELL_VIDEO_OUT_PRIMARY, resId, CELL_VIDEO_OUT_ASPECT_AUTO,0);
}

int VbaGraphics::AddResolution(uint32_t resId)
{
	supportedResolutions.push_back(resId);
}

void VbaGraphics::NextResolution()
{
	LOG("VbaGraphics::NextResolution()\n");
	LOG("supportedResolutions size: %d\n", supportedResolutions.size());
	if(currentResolution+1 < supportedResolutions.size())
	{
		currentResolution++;
		LOG("currentResolution: %d\n", currentResolution);
	}
}

void VbaGraphics::PreviousResolution()
{
	LOG("VbaGraphics::NextResolution()\n");
	LOG("supportedResolutions size: %d\n", supportedResolutions.size());
	if(currentResolution > 0)
	{
		currentResolution--;
		LOG("currentResolution: %d\n", currentResolution);
	}
}

void VbaGraphics::SwitchResolution()
{
	if(CheckResolution(supportedResolutions[currentResolution]))
	{
		ChangeResolution(supportedResolutions[currentResolution]);
	}
}

void VbaGraphics::SwitchResolution(uint32_t resId)
{
	if(CheckResolution(resId))
	{
		ChangeResolution(resId);
	}
}

uint32_t VbaGraphics::GetInitialResolution()
{
	return initialResolution;
}

uint32_t VbaGraphics::GetCurrentResolution()
{
	return supportedResolutions[currentResolution];
}

void VbaGraphics::GetAllAvailableResolutions()
{
	if(CheckResolution(CELL_VIDEO_OUT_RESOLUTION_576))
	{
		AddResolution(CELL_VIDEO_OUT_RESOLUTION_576);
		initialResolution = CELL_VIDEO_OUT_RESOLUTION_576;
	}
	if(CheckResolution(CELL_VIDEO_OUT_RESOLUTION_480))
	{
		AddResolution(CELL_VIDEO_OUT_RESOLUTION_480);
		initialResolution = CELL_VIDEO_OUT_RESOLUTION_480;
	}
	if(CheckResolution(CELL_VIDEO_OUT_RESOLUTION_720))
	{
		AddResolution(CELL_VIDEO_OUT_RESOLUTION_720);
		initialResolution = CELL_VIDEO_OUT_RESOLUTION_720;
	}
	if(CheckResolution(CELL_VIDEO_OUT_RESOLUTION_960x1080))
	{
		AddResolution(CELL_VIDEO_OUT_RESOLUTION_960x1080);
		initialResolution = CELL_VIDEO_OUT_RESOLUTION_960x1080;
	}
	if(CheckResolution(CELL_VIDEO_OUT_RESOLUTION_1280x1080))
	{
		AddResolution(CELL_VIDEO_OUT_RESOLUTION_1280x1080);
		initialResolution = CELL_VIDEO_OUT_RESOLUTION_1280x1080;
	}
	if(CheckResolution(CELL_VIDEO_OUT_RESOLUTION_1440x1080))
	{
		AddResolution(CELL_VIDEO_OUT_RESOLUTION_1440x1080);
		initialResolution = CELL_VIDEO_OUT_RESOLUTION_1440x1080;
	}
	if(CheckResolution(CELL_VIDEO_OUT_RESOLUTION_1600x1080))
	{
		AddResolution(CELL_VIDEO_OUT_RESOLUTION_1600x1080);
		initialResolution = CELL_VIDEO_OUT_RESOLUTION_1600x1080;
	}
	if(CheckResolution(CELL_VIDEO_OUT_RESOLUTION_1080))
	{
		AddResolution(CELL_VIDEO_OUT_RESOLUTION_1080);
		initialResolution = CELL_VIDEO_OUT_RESOLUTION_1080;
	}
	currentResolution = supportedResolutions.size()-1;
}

CellVideoOutState VbaGraphics::GetVideoOutState()
{
	return _videoOutState;
}

int32_t VbaGraphics::ChangeResolution(uint32_t resId)
{
	LOG("VbaGraphics::ChangeResolution(%d)\n", resId);
	int32_t ret;
	CellVideoOutState video_state;
	CellVideoOutConfiguration video_config;
	CellVideoOutResolution resolution;
	
	cellVideoOutGetState(CELL_VIDEO_OUT_PRIMARY, 0, &video_state);
	video_state.displayMode.resolutionId = resId;
	cellVideoOutGetResolution(video_state.displayMode.resolutionId, &resolution);
	
	memset(&video_config, 0, sizeof(CellVideoOutConfiguration));
	
	video_config.resolutionId = resId;
	video_config.format = CELL_VIDEO_OUT_BUFFER_COLOR_FORMAT_X8R8G8B8;
	video_config.aspect = video_state.displayMode.aspect;
	video_config.pitch = resolution.width << 2;
	
	Deinit();
	
	ret = cellVideoOutConfigure(CELL_VIDEO_OUT_PRIMARY, &video_config, NULL, 0);
	if(ret != CELL_OK)
	{
		return ret;
	}
	
	Init(resId);
	PSGLGraphics::InitDbgFont();
	cellVideoOutGetState(CELL_VIDEO_OUT_PRIMARY, 0, &stored_video_state);
}

void VbaGraphics::PSGLInit()
{
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_VSYNC_SCE);

	uint32_t ret = InitCg();
	if (ret != CELL_OK)
	{
		LOG("Failed to InitCg: %d", __LINE__);
	}

	//FIXME: Change to SetViewports?
	set_aspect();

	vertex_buf = (uint8_t*)memalign(128, 256);
	assert(vertex_buf);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glColor4f(1.0, 1.0, 1.0, 1.0);
	Clear();
	Swap();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glGenBuffers(1, &pbo);
	glGenBuffers(1, &vbo);

	// Vertexes
	GLfloat vertexes[] = {
	  0, 0, 0,
	  0, 1, 0,
	  1, 1, 0,
	  1, 0, 0,
	  0, 1,
	  0, 0,
	  1, 0,
	  1, 1
	};

	memcpy(vertex_buf, vertexes, 12 * sizeof(GLfloat));
	memcpy(vertex_buf + 128, vertexes + 12, 8 * sizeof(GLfloat));

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, 256, vertex_buf, GL_STATIC_DRAW);
	/////

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, 0);
	glTexCoordPointer(2, GL_FLOAT, 0, (void*)128);

	glEnable(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glBindBuffer(GL_TEXTURE_REFERENCE_BUFFER_SCE, pbo);
	SetSmooth(m_smooth);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glColor4f(1.0, 1.0, 1.0, 1.0);
	Clear();
	Swap();
}

void VbaGraphics::Init()
{
	LOG("VbaGraphics::Init()");
	PSGLGraphics::Init(NULL, m_pal60Hz);
	PSGLInit();

	// snag the video out state
	cellVideoOutGetState(CELL_VIDEO_OUT_PRIMARY, 0, &_videoOutState);
	GetAllAvailableResolutions();
}

//FIXME: Ported over verbatim from Fceu. Might require additional changes
void VbaGraphics::Init(uint32_t resId)
{
	LOG("VbaGraphics::Init(%d, %d)", resId, m_pal60Hz);
	PSGLGraphics::Init(resId, m_pal60Hz);
	PSGLInit();

	SetDimensions(240, 160, (240)* 4);

	Rect r;
	r.x = 0;
	r.y = 0;
	r.w = 240;
	r.h = 160;
	SetRect(r);
	SetAspectRatio(m_ratio);
}


CGerror CheckCgError(int line)
{
	CGerror err = cgGetError ();

	if (err != CG_NO_ERROR)
	{
		LOG ("CG error:%d at line %d %s\n", err, line, cgGetErrorString (err));
	}

	return err;
}


CGprogram LoadShaderFromFile(CGcontext cgtx, CGprofile target, const char* filename, const char* entry)
{
	CGprogram id = cgCreateProgramFromFile(cgtx, CG_BINARY, filename, target, "main", NULL);
	if(!id)
	{
		LOG("Failed to load shader program >>%s<<\nExiting\n", filename);
		CheckCgError(__LINE__);
	}

	return id;
}


CGprogram LoadShaderFromSource(CGcontext cgtx, CGprofile target, const char* filename, const char* entry)
{
	CGprogram id = cgCreateProgramFromFile(cgtx, CG_SOURCE, filename, target, entry, NULL);
	if(!id)
	{
		LOG("Failed to load shader program >>%s<< \nExiting\n", filename);
		CheckCgError(__LINE__);
	}

	return id;
}


int32_t VbaGraphics::InitCg()
{
	LOG("VbaGraphics::InitCg()\n");

	cgRTCgcInit();

	LOG("VbaGraphics::InitCg() - About to create CgContext\n");
	_cgContext = cgCreateContext();
	if (_cgContext == NULL)
	{
		LOG("Error Creating Cg Context\n");
		return 1;
	}
	if (strlen(_curFragmentShaderPath.c_str()) > 0)
	{
		return LoadFragmentShader(_curFragmentShaderPath.c_str());
	}
	else
	{
		_curFragmentShaderPath = DEFAULT_SHADER_FILE;
		return LoadFragmentShader(_curFragmentShaderPath.c_str());
	}
}


int32_t VbaGraphics::LoadFragmentShader(string shaderPath)
{
	LOG("LoadFragmentShader(%s)\n", shaderPath.c_str());

	// store the cur path
	_curFragmentShaderPath = shaderPath;

	_vertexProgram = LoadShaderFromSource(_cgContext, CG_PROFILE_SCE_VP_RSX, shaderPath.c_str(), "main_vertex");
	if (_vertexProgram <= 0)
	{
		LOG("Error loading vertex shader...");
		return 1;
	}

	_fragmentProgram = LoadShaderFromSource(_cgContext, CG_PROFILE_SCE_FP_RSX, shaderPath.c_str(), "main_fragment");
	if (_fragmentProgram <= 0)
	{
		LOG("Error loading fragment shader...");
		return 1;
	}

	// bind and enable the vertex and fragment programs
	cgGLEnableProfile(CG_PROFILE_SCE_VP_RSX);
	cgGLEnableProfile(CG_PROFILE_SCE_FP_RSX);
	cgGLBindProgram(_vertexProgram);
	cgGLBindProgram(_fragmentProgram);

	// acquire mvp param from v shader
	_cgpModelViewProj = cgGetNamedParameter(_vertexProgram, "modelViewProj");
	if (CheckCgError (__LINE__) != CG_NO_ERROR)
	{
		// FIXME: WHY DOES THIS GIVE ERROR ON OTHER LOADS
		//return 1;
	}

    _cgpVideoSize = cgGetNamedParameter(_fragmentProgram, "IN.video_size");
    _cgpTextureSize = cgGetNamedParameter(_fragmentProgram, "IN.texture_size");
    _cgpOutputSize = cgGetNamedParameter(_fragmentProgram, "IN.output_size");

    LOG("SUCCESS - LoadFragmentShader(%s)\n", shaderPath.c_str());
	return CELL_OK;
}


void VbaGraphics::UpdateCgParams(unsigned width, unsigned height, unsigned tex_width, unsigned tex_height)
{
	cgGLSetStateMatrixParameter(_cgpModelViewProj, CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_IDENTITY);
	cgGLSetParameter2f(_cgpVideoSize, width, height);
	cgGLSetParameter2f(_cgpTextureSize, tex_width, tex_height);
	cgGLSetParameter2f(_cgpOutputSize, _cgViewWidth, _cgViewHeight);
}

void VbaGraphics::UpdateCgParams()
{
   UpdateCgParams(m_width, m_height, m_width, m_height);
}

void VbaGraphics::SetPAL60Hz(bool pal60Hz)
{
	m_pal60Hz = pal60Hz;
}

void VbaGraphics::SetSmooth(bool smooth)
{
	m_smooth = smooth;
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, smooth ? GL_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, smooth ? GL_LINEAR : GL_NEAREST);
}

void VbaGraphics::SetOverscan(bool will_overscan, float amount)
{
	LOG("VbaGraphics::SetOverscan(%, %f)", will_overscan, amount);
	m_overscan_amount = amount;
	m_overscan = will_overscan;
	set_aspect();
}


void VbaGraphics::set_aspect()
{
	float device_aspect = this->GetDeviceAspectRatio();
	GLuint width = this->GetResolutionWidth();
	GLuint height = this->GetResolutionHeight();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// calculate the glOrtho matrix needed to transform the texture to the desired aspect ratio
	float desired_aspect = m_ratio;

	GLuint real_width = width, real_height = height;

	// If the aspect ratios of screen and desired aspect ratio are sufficiently equal (floating point stuff),
	// assume they are actually equal.
	if ( (int)(device_aspect*1000) > (int)(desired_aspect*1000) )
	{
		float delta = (desired_aspect / device_aspect - 1.0) / 2.0 + 0.5;
		glViewport(width * (0.5 - delta), 0, 2.0 * width * delta, height);
		real_width = (int)(2.0 * width * delta);
	}

	else if ( (int)(device_aspect*1000) < (int)(desired_aspect*1000) )
	{
		float delta = (device_aspect / desired_aspect - 1.0) / 2.0 + 0.5;
		glViewport(0, height * (0.5 - delta), width, 2.0 * height * delta);
		real_height = (int)(2.0 * height * delta);
	}
	else
	{
		glViewport(0, 0, width, height);
	}

	if (m_overscan)
	{
		glOrthof(-m_overscan_amount/2, 1 + m_overscan_amount/2, -m_overscan_amount/2, 1 + m_overscan_amount/2, -1, 1);
	}
	else
	{
		glOrthof(0, 1, 0, 1, -1, 1);
	}

	_cgViewWidth = real_width;
	_cgViewHeight = real_height;

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}


