include (../../shared.pri)

HEADERS       += io_expe.h \
                 import_expe.h \
                 import_xyz.h \
                 export_xyz.h 
				 
SOURCES       += io_expe.cpp 

TARGET        = io_expe

win32-msvc2010:LIBS += -lGLU32 -lOpenGL32
