include (../../shared.pri)

HEADERS       += io_pdb.h \
		$$VCGDIR/wrap/ply/plylib.h 
				
SOURCES       += io_pdb.cpp \
		$$VCGDIR//wrap/ply/plylib.cpp\ 
		

TARGET        = io_pdb

win32-msvc2010:LIBS += -lGLU32 -lOpenGL32
