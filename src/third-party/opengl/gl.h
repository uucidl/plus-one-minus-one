#ifndef THIRD_PARTY_OPENGL_GL_H
#define THIRD_PARTY_OPENGL_GL_H

#if defined(MACOSX)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#if defined(WIN32)
#include <GL/glext.h>
#endif
#endif

#endif
