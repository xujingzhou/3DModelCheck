
#include <common/mlapplication.h>
#include <QMessageBox>
#include "mainwindow.h"
#include <QString>
#include <QSplashScreen>
#include "skin/titlebar_custom.h"

/////////////// - Added by Johnny Xu, 2017/3/2 - /////////////////////
#if defined(Q_OS_WIN)
	#include <Shlobj.h>
#endif

#if defined(Q_OS_WIN)

bool correlationFileTypeWithProgram(char *argv[], const QString extType)
{
	if(*argv == "" || extType.isEmpty())
		return false;

	// 1.
	QString regPath = QString("HKEY_CLASSES_ROOT\\\\.%1").arg(extType);
	QString programName = MeshLabApplication::programName();
	QSettings *envFileReg = new QSettings(regPath, QSettings::NativeFormat);  
	QString currentValue = envFileReg->value("Default").toString();
	if (currentValue.isEmpty() || currentValue != programName)
	{
		envFileReg->setValue("Default", programName);
	}

	QSettings *envIconReg = new QSettings("HKEY_CLASSES_ROOT\\scientree\\DefaultIcon\\", QSettings::NativeFormat);
	if (envIconReg->value("Default").toString().isEmpty())
	{
		envIconReg->setValue("Default", QString(QString(argv[0]) + ",0").replace("/", "\\"));
	}
	delete envIconReg;

	QSettings *envProgramReg = new QSettings("HKEY_CLASSES_ROOT\\scientree\\shell\\open\\command\\", QSettings::NativeFormat);
	if (envProgramReg->value("Default").toString().isEmpty())
	{
		envProgramReg->setValue("Default", QString(QString(argv[0]) + " %1").replace("/", "\\")); 
	}
	delete envProgramReg; 
	
	regPath = QString("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\\\.%1").arg(extType);
	QSettings *envExtReg = new QSettings(regPath, QSettings::NativeFormat);
	currentValue = envExtReg->value("Progid").toString();
	if (currentValue.isEmpty() || currentValue != programName)
	{
		envExtReg->setValue("Progid", programName);
	}
    delete envExtReg;
	delete envFileReg;  

	// 2.
	QString registerPath = QString("HKEY_CURRENT_USER\\Software\\Classes\\\\.%1\\DefaultIcon").arg(extType);
	QSettings *envCurrentUserIconReg = new QSettings(registerPath, QSettings::NativeFormat);
	QString currentRegValue = envCurrentUserIconReg->value("Default").toString();
	if (envCurrentUserIconReg->value("Default").toString().isEmpty())
	{
		envCurrentUserIconReg->setValue("Default", QString(QString(argv[0]) + ",0").replace("/", "\\")); 
	}
	delete envCurrentUserIconReg; 

	registerPath = QString("HKEY_CURRENT_USER\\Software\\Classes\\\\.%1\\shell\\open\\command").arg(extType);
	QSettings *envCurrentUserOpenReg = new QSettings(registerPath, QSettings::NativeFormat);
	currentRegValue = envCurrentUserOpenReg->value("Default").toString();
	if (currentRegValue.isEmpty())
	{
		envCurrentUserOpenReg->setValue("Default", QString(QString(argv[0]) + " %1").replace("/", "\\"));
	}
	delete envCurrentUserOpenReg;

	SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, 0, 0);

	return true;
}

#endif
/////////////// - End - /////////////////////

int main(int argc, char *argv[])
{
	MeshLabApplication app(argc, argv);
	app.setFont(QFont("黑体", 10));

	// Add custom skin by Johnny Xu, 2017/7/5
	QFile styleFile(":/qss/Style_LightBlue.qss");
	styleFile.open(QIODevice::ReadOnly);
	QString styleSheet(QLatin1String(styleFile.readAll()));
	app.setStyleSheet(styleSheet);

	// Added by Johnn Xu, 2017/3/2 -- need run at administrator identity(in the right key popup menu of click mouse)
//#if defined(Q_OS_WIN)
//	correlationFileTypeWithProgram(argv, "ply");
//#endif

	// Modified by Johnny Xu, 2017/2/20
	QLocale cnLocale(QLocale::Chinese, QLocale::SimplifiedChineseScript, QLocale::China);
    QLocale::setDefault(cnLocale);

	// Load chinese translate to QT UI. Added by Johnny Xu,2017/2/10
#ifndef QT_NO_TRANSLATION
	// 1.
	/*QDir baseDir(PluginManager::getBaseDirPath());
	QDir translationsDir(baseDir.absolutePath());
	if(translationsDir.exists("translations"))
	{
		translationsDir.cd("translations");
		QTranslator translator_TextViewMenu;
		if (translator_TextViewMenu.load(translationsDir.absolutePath() + QString("qt_widgets")))
		{
			app.installTranslator(&translator_TextViewMenu);
			qDebug("\nLoad 'qt_widgets' success!\n");
		}
		else
		{
			qDebug(translationsDir.absolutePath().toStdString().c_str());
			qDebug("\nLoad 'qt_widgets' failed!\n");
		}
	}
	else
	{
		qDebug(translationsDir.absolutePath().toStdString().c_str());
		qDebug("\nInitialization error: Unable to find the translations directory.\n");
	}*/

	// 2.
	QDir baseDir(PluginManager::getBaseDirPath());
	QDir translationsDir(baseDir.absolutePath());
	if(translationsDir.exists("translations"))
	{
		translationsDir.cd("translations");
		QTranslator *translator = new QTranslator();
		if (translator->load(translationsDir.absolutePath() + QString("/qt_zh_CN.qm")))
		{
			app.installTranslator(translator);
			qDebug("\nLoad 'qt_zh_CN' success!\n");
		}
		else
		{
			qDebug(translationsDir.absolutePath().toStdString().c_str());
			qDebug("\nLoad 'qt_zh_CN' failed!\n");
		}
	}
	else
	{
		qDebug(translationsDir.absolutePath().toStdString().c_str());
		qDebug("\nInitialization error: Unable to find the translations directory.\n");
	}

    /*QString translatorFileName = QLatin1String("qt_");
    translatorFileName += QLocale::system().name();
    QTranslator *translator = new QTranslator();
    if (translator->load(translatorFileName, QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
	{
        app.installTranslator(translator);
		qDebug((QLibraryInfo::location(QLibraryInfo::TranslationsPath)).toStdString().c_str());
		qDebug("Load translator success!");
	}
    else
	{
		qDebug(translatorFileName.toStdString().c_str());
        qDebug("Load translator failed!");
	}*/
#endif

    QCoreApplication::setOrganizationName(MeshLabApplication::organization());
#if QT_VERSION >= 0x050100
    // Enable support for highres images (added in Qt 5.1, but off by default)
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    QCoreApplication::setApplicationName(MeshLabApplication::appArchitecturalName(MeshLabApplication::HW_ARCHITECTURE(QSysInfo::WordSize)));
   
	// Add splash by Johnny Xu, 2017/6/29
	QPixmap pixmap(":/images/splash.png");
	QSplashScreen *splash = new QSplashScreen(pixmap);
	splash->show();

#if defined (_CUSTOM_SKIN_)  
 
	// Cusctom titlebar by Johnny Xu, 2017/7/12
	MainWindow window;
	SkinWindow *skin = new SkinWindow(&window);
	skin->setWindowTitle(window.windowTitle());
	skin->setWindowIcon(window.windowIcon());
	skin->showMaximized();
#else  
	// Normal
	MainWindow window;
	window.showMaximized();
#endif

	splash->finish(&window);
	delete splash;

    // This event filter is installed to intercept the open events sent directly by the Operative System.
    FileOpenEater *filterObj = new FileOpenEater(&window);
    app.installEventFilter(filterObj);
    app.processEvents();

    if(argc>1)
    {
		// Modify by RBG  文件路径中有汉字的模型打开
		window.importMeshWithLayerManagement(QString::fromLocal8Bit(argv[1]));
    }

    return app.exec();
}
