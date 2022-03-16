include (../../shared.pri)

HEADERS       += io_u3d.h \
        $$VCGDIR/wrap/io_trimesh/export_u3d.h
SOURCES       += io_u3d.cpp

TARGET        = io_u3d

CONFIG += STL

win32-msvc2010:LIBS += -lGLU32 -lOpenGL32
