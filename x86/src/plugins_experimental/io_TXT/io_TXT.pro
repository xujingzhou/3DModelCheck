include (../../shared.pri)

HEADERS       += io_txt.h \
				
SOURCES       += io_txt.cpp \
				
TARGET        = io_txt

win32-msvc2010:LIBS += -lGLU32 -lOpenGL32
