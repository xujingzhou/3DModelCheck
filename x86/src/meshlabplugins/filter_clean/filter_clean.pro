include (../../shared.pri)

HEADERS       += cleanfilter.h

SOURCES       += cleanfilter.cpp	

TARGET        = filter_clean

win32-msvc2010:LIBS += -lGLU32 -lOpenGL32
