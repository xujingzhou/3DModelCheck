include (../../shared.pri)

HEADERS       += decorate_background.h cubemap.h

SOURCES      += decorate_background.cpp\
                cubemap.cpp

TARGET        = decorate_background

RESOURCES +=  decorate_background.qrc

win32-msvc2010:LIBS += -lGLU32 -lOpenGL32
