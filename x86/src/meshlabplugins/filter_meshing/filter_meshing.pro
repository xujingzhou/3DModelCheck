include (../../shared.pri)

HEADERS       += $$VCGDIR/vcg/complex/algorithms/clean.h\
		quadric_simp.h \ 
		quadric_tex_simp.h \ 
		meshfilter.h 

SOURCES       += meshfilter.cpp \
		quadric_simp.cpp \ 
        quadric_tex_simp.cpp

TARGET        = filter_meshing

win32-msvc2010:LIBS += -lGLU32 -lOpenGL32
