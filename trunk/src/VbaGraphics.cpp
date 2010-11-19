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

#include "vba/System.h"


// FIXME: make this use the faster ABGR_SCE instead of the easy RGBA

VbaGraphics::VbaGraphics() : PSGLGraphics(), gl_buffer(NULL), vertex_buf(NULL)
{

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
   psglSwap();
}


void VbaGraphics::Draw() const
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_pitch/4.0, m_height, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, 0);

	glDrawArrays(GL_QUADS, 0, 4);
	glFlush();
	Swap();
}


void VbaGraphics::Draw(uint8_t *XBuf)
{
	//LOG("Draw(%d, %d)\n", m_width, m_height);

	Clear();

	uint32_t* texture = MapPixels();

	for(int i = 0; i != m_width * m_height; i ++)
	{
		//LOG(" %u ", XBuf[i]);
		texture[i] = XBuf[i] << 24 | XBuf[i] << 16 | XBuf[i] << 8 | 0xFF;
		//texture[i] = systemColorMap32[XBuf[i]] | 0xFF;
		//texture[i] = std::numeric_limits<uint32_t>::max();
		//texture[i] = ((pcpalette[XBuf[i]].r) << 24) | ((pcpalette[XBuf[i]].g) << 16) | (pcpalette[XBuf[i]].b << 8) | 0xFF;
		//texture[i] = ((pcpalette[XBuf[i]].b) << 24) | ((pcpalette[XBuf[i]].g) << 16) | (pcpalette[XBuf[i]].r << 8) | 0xFF;
	}
	UnmapPixels();
	//LOG("\n");
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
	glClearColor(0.25, 0.25, 0.25, 1);
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
	m_pitch = pitch;

	if (gl_buffer)
		free(gl_buffer);
	gl_buffer = (uint32_t*)memalign(128, m_height * m_pitch);
	memset(gl_buffer, 0, m_height * m_pitch);
	glBufferData(GL_PIXEL_UNPACK_BUFFER_ARB, m_height * m_pitch, gl_buffer, GL_STREAM_DRAW);
}


void VbaGraphics::SetRect(const Rect &view)
{
   unsigned pitch = m_pitch >> 2;
   GLfloat tex_coords[] = {
      (float)view.x / pitch, (float)(view.y + view.h) / m_height,            // Upper Left
      (float)view.x / pitch, (float)view.y / m_height,                       // Lower Left
      (float)(view.x + view.w) / pitch, (float)view.y / m_height,            // Lower Right
      (float)(view.x + view.w) / pitch, (float)(view.y + view.h) / m_height  // Upper Right
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


CellVideoOutState VbaGraphics::GetVideoOutState()
{
	return _videoOutState;
}


int32_t VbaGraphics::ChangeResolution(uint32_t resId, uint16_t refreshrateId)
{
	int32_t ret;
	CellVideoOutState video_state;
	CellVideoOutConfiguration video_config;
	CellVideoOutResolution resolution;
	uint32_t depth_pitch;

	cellVideoOutGetState(CELL_VIDEO_OUT_PRIMARY, 0, &video_state);
	video_state.displayMode.refreshRates = refreshrateId;
	video_state.displayMode.resolutionId = resId;
	cellVideoOutGetResolution(video_state.displayMode.resolutionId, &resolution);

	depth_pitch = resolution.width * 4;

	memset(&video_config, 0, sizeof(CellVideoOutConfiguration));

	video_config.resolutionId = video_state.displayMode.resolutionId;
	video_config.format = CELL_VIDEO_OUT_BUFFER_COLOR_FORMAT_X8R8G8B8;
	video_config.aspect = video_state.displayMode.aspect;
	video_config.pitch = resolution.width * 4;

	// just bring down it anyway
	this->DeinitDbgFont();
	this->DeInit();

	ret = cellVideoOutConfigure(CELL_VIDEO_OUT_PRIMARY, &video_config, NULL, 0);
	if (ret != CELL_OK)
	{
		return ret;
	}
}


void VbaGraphics::Init()
{
	LOG("VbaGraphics::Init()");
	PSGLGraphics::Init();

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_VSYNC_SCE);

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
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, pbo);
	SetSmooth(true);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glColor4f(1.0, 1.0, 1.0, 1.0);
	Clear();
	Swap();

	// snag the video out state
	cellVideoOutGetState(CELL_VIDEO_OUT_PRIMARY, 0, &_videoOutState);
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

	return LoadFragmentShader(DEFAULT_SHADER_FILE);
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


void VbaGraphics::SetSmooth(bool smooth)
{
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, smooth ? GL_LINEAR : GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, smooth ? GL_LINEAR : GL_NEAREST);
}


void VbaGraphics::set_aspect()
{
	bool overscan = false;
	float overscan_amount = 0.0f;

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

	if (overscan)
	{
		glOrthof(-overscan_amount/2, 1 + overscan_amount/2, -overscan_amount/2, 1 + overscan_amount/2, -1, 1);
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


