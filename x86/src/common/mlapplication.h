#ifndef ML_APPLICATION_H
#define ML_APPLICATION_H

#include <QApplication>
#include <QString>
#include <wrap/gl/gl_mesh_attributes_info.h>
#include "ml_mesh_type.h"

template<typename T>
struct MeshLabScalarTest
{

};

template<>
struct MeshLabScalarTest<float>
{
	static const QString floatingPointPrecision() 
	{
		return QString("fp");
	}

    static const char* floatingPointPrecisionIOToken() 
	{
		return "%f";
	}

	static bool doublePrecision() 
	{
		return false;
	}
};

template<>
struct MeshLabScalarTest<double>
{
	static const QString floatingPointPrecision() 
	{
		return QString("dp");
	}

    static const char* floatingPointPrecisionIOToken() 
	{
		return "%lf";
	
	}
	static bool doublePrecision() 
	{
		return true;
	}
};


class MeshLabApplication : public QApplication
{
public:

    enum HW_ARCHITECTURE {HW_32BIT = 32, HW_64BIT = 64};

    MeshLabApplication(int &argc, char *argv[]):QApplication(argc,argv)
	{
	}

    ~MeshLabApplication()
	{
	}

    bool notify(QObject * rec, QEvent * ev);

    static const QString appName()
	{
		return QTextCodec::codecForName("GBK")->toUnicode("三维数据查验系统");
	}

    static const QString architecturalSuffix(const HW_ARCHITECTURE hw) 
	{
		return "_" + QString::number(int(hw)) + QTextCodec::codecForName("GBK")->toUnicode("位");
	}

    static const QString appArchitecturalName(const HW_ARCHITECTURE hw) 
	{
		return appName() + architecturalSuffix(hw) + "_" + MeshLabScalarTest<MESHLAB_SCALAR>::floatingPointPrecision();
	}

    static const QString appVer() 
	{ 
		return QTextCodec::codecForName("GBK")->toUnicode("1.0.0");  
	}

	static const QString shortName() 
	{ 
		return appName() + " " + appVer();
	}

	// Added by Johnny Xu, 2017/3/2
	static const QString programName()
	{
		return tr("scientree");
	}

	// Add by Johnny Xu,2017/2/20
	static const QString completeVersion()
	{
		return QTextCodec::codecForName("GBK")->toUnicode(" 版本:") + appVer();
	}

    static const QString completeName(const HW_ARCHITECTURE hw)
	{
		return appArchitecturalName(hw) + QTextCodec::codecForName("GBK")->toUnicode("版本")+ appVer();
	}

    static const QString organization()
	{
		return QTextCodec::codecForName("GBK")->toUnicode("算筹科技有限公司");
	}

    static const QString organizationHost() 
	{
		return tr("http://www.scientree.com/");
	}

    static const QString webSite() 
	{
		return tr("http://www.scientree.com/");
	}

    static const QString downloadSite() 
	{
		return tr("http://www.scientree.com//#download");
	}

    static const QString pluginsPathRegisterKeyName() 
	{
		return tr("pluginsPath");
	}

    static const QString versionRegisterKeyName() 
	{
		return tr("version");
	}

    static const QString wordSizeKeyName() 
	{
		return tr("wordSize");
	}
};

#endif
