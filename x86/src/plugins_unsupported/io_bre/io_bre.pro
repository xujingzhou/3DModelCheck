include (../../shared.pri)

HEADERS       += io_bre.h \
				
SOURCES       += io_bre.cpp \

TARGET        = io_bre

win32-msvc2010:LIBS += -lGLU32 -lOpenGL32
