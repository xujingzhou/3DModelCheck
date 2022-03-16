include (../../shared.pri)

HEADERS       += filter_layer.h
SOURCES       += filter_layer.cpp
TARGET        =  filter_layer

win32-msvc2010:LIBS += -lGLU32 -lOpenGL32
