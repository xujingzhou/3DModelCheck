include (../../shared.pri)

HEADERS += io_json.h

SOURCES += io_json.cpp

TARGET   = io_json

win32-msvc2010:LIBS += -lGLU32 -lOpenGL32
