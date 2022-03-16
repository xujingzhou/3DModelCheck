include (../../shared.pri)

HEADERS       += io_tri.h 
				
SOURCES       += io_tri.cpp\
		
 
				
TARGET        = io_tri

win32-msvc2010:LIBS += -lGLU32 -lOpenGL32
