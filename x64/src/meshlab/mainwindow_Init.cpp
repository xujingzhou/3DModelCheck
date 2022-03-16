
#include "../common/interfaces.h"
#include "../common/xmlfilterinfo.h"
#include "../common/searcher.h"
#include "../common/mlapplication.h"

#include <QToolBar>
#include <QProgressBar>
#include <QFileOpenEvent>
#include <QFile>
#include <QtXml>
#include <QSysInfo>
#include <QDesktopServices>
#include <QStatusBar>
#include <QMenuBar>
#include <QWidgetAction>

#include "skin/titlebar_custom.h"
#include "mainwindow.h"
#include "saveSnapshotDialog.h"
#include "utils.h"

QProgressBar *MainWindow::qb;

MainWindow::MainWindow()
	:mwsettings(), gpumeminfo(NULL), xmlfiltertimer(), wama()
{
	_currviewcontainer = NULL;

	//xmlfiltertimer will be called repeatedly, so like Qt documentation suggests, the first time start function should be called.
	//Subsequently restart function will be invoked.
	xmlfiltertimer.start();

	mdiarea = new QMdiArea(this);

	layerDialog = new LayerDialog(this);
	connect(layerDialog, SIGNAL(toBeShow()), this, SLOT(updateLayerDialog()));
	layerDialog->setAllowedAreas(Qt::LeftDockWidgetArea);
	layerDialog->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
	addDockWidget(Qt::LeftDockWidgetArea, layerDialog);

	setCentralWidget(mdiarea);

	windowMapper = new QSignalMapper(this);
	// Permette di passare da una finestra all'altra e tenere aggiornato il workspace
	connect(windowMapper, SIGNAL(mapped(QWidget*)), this, SLOT(wrapSetActiveSubWindow(QWidget *)));
	// Quando si passa da una finestra all'altra aggiorna lo stato delle toolbar e dei menu
	connect(mdiarea, SIGNAL(subWindowActivated(QMdiSubWindow *)), this, SLOT(switchCurrentContainer(QMdiSubWindow *)));

	QIcon icon;
	icon.addPixmap(QPixmap(":images/eye48.png"));
	setWindowIcon(icon);

	PM.loadPlugins(defaultGlobalParams);

	QSettings settings;
	QVariant vers = settings.value(MeshLabApplication::versionRegisterKeyName());
	//should update those values only after I run MeshLab for the very first time or after I installed a new version
	if (!vers.isValid() || vers.toString() < MeshLabApplication::appVer())
	{
		settings.setValue(MeshLabApplication::pluginsPathRegisterKeyName(), PluginManager::getDefaultPluginDirPath());
		settings.setValue(MeshLabApplication::versionRegisterKeyName(), MeshLabApplication::appVer());
		settings.setValue(MeshLabApplication::wordSizeKeyName(), QSysInfo::WordSize);
		foreach(QString plfile, PM.pluginsLoaded)
			settings.setValue(PluginManager::osIndependentPluginName(plfile), MeshLabApplication::appVer());
	}

	// Now load from the registry the settings and  merge the hardwired values got from the PM.loadPlugins with the ones found in the registry.
	loadSettings();
	mwsettings.updateGlobalParameterSet(currentGlobalParams);

	createActions();
	createToolBars();
	createMenus();

	gpumeminfo = new vcg::QtThreadSafeMemoryInfo(mwsettings.maxgpumem);
	stddialog = 0;
	xmldialog = 0;
	setAcceptDrops(true);
	mdiarea->setAcceptDrops(true);

	setWindowTitle(MeshLabApplication::appName());

	setStatusBar(new QStatusBar(this));
	globalStatusBar() = statusBar();
	qb = new QProgressBar(this);
	qb->setMaximum(100);
	qb->setMinimum(0);
	qb->reset();
	statusBar()->addPermanentWidget(qb, 1);

	// Delete GPU usage by Johnny Xu, 2017/6/27
	/*libGPUProcess = new QLabel(this);
	libGPUProcess->setBaseSize(100, 20);
	QString qStr = QString::fromLocal8Bit("GPUʹ����: 10%");
	libGPUProcess->setText(qStr);
	statusBar()->addPermanentWidget(libGPUProcess, 0);*/

	newProject();
	updateCustomSettings();

	connect(this, SIGNAL(updateLayerTable()), this, SLOT(updateLayerDialog()));
	connect(layerDialog, SIGNAL(removeDecoratorRequested(QAction*)), this, SLOT(switchOffDecorator(QAction*)));

	// Added by Johnny Xu, 2017/7/26
	if (utils::runProgramCount() >= 1)
	{
		readSettings();
	}
}

MainWindow::~MainWindow()
{
	delete gpumeminfo;

	// Added by Johnny Xu, 2017/7/26
	writeSettings();
}

// Added by Johnny Xu, 2017/7/17
void MainWindow::closeEvent(QCloseEvent *event)
{

#if defined (_CUSTOM_SKIN_)  

	if (SkinWindow *skin = static_cast<SkinWindow *>(this->parentWidget()))
	{
		if (skin)
		{
			skin->close();
		}
	}
#endif
}

void MainWindow::createActions()
{
	QTextCodec *codec = QTextCodec::codecForName("GBK");

	importMeshAct = new QAction(QIcon(":/images/import_mesh.png"), codec->toUnicode("&��������") /*tr("&Import Mesh...")*/, this);
	importMeshAct->setShortcutContext(Qt::ApplicationShortcut);
	importMeshAct->setShortcut(Qt::CTRL + Qt::Key_I);
	connect(importMeshAct, SIGNAL(triggered()), this, SLOT(importMeshWithLayerManagement()));

	exportMeshAct = new QAction(QIcon(":/images/save.png"), codec->toUnicode("&��������") /*tr("&Export Mesh...")*/, this);
	exportMeshAct->setShortcutContext(Qt::ApplicationShortcut);
	exportMeshAct->setShortcut(Qt::CTRL + Qt::Key_E);
	connect(exportMeshAct, SIGNAL(triggered()), this, SLOT(save()));

	exportMeshAsAct = new QAction(QIcon(":/images/save.png"), codec->toUnicode("&�������Ϊ...") /*tr("&Export Mesh As...")*/, this);
	exportMeshAsAct->setShortcutContext(Qt::ApplicationShortcut);
	exportMeshAsAct->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_E);
	connect(exportMeshAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

	reloadMeshAct = new QAction(QIcon(":/images/reload.png"), codec->toUnicode("&���¼�������") /*tr("&Reload")*/, this);
	reloadMeshAct->setShortcutContext(Qt::ApplicationShortcut);
	reloadMeshAct->setShortcut(Qt::ALT + Qt::Key_R);
	connect(reloadMeshAct, SIGNAL(triggered()), this, SLOT(reload()));

	reloadAllMeshAct = new QAction(codec->toUnicode("&���¼�����������") /*tr("&Reload All")*/, this);
	reloadAllMeshAct->setShortcutContext(Qt::ApplicationShortcut);
	reloadAllMeshAct->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_R);
	connect(reloadAllMeshAct, SIGNAL(triggered()), this, SLOT(reloadAllMesh()));

	saveSnapshotAct = new QAction(QIcon(":/images/snapshot.png"), codec->toUnicode("&�����ͼ") /*tr("Save snapsho&t")*/, this);
	connect(saveSnapshotAct, SIGNAL(triggered()), this, SLOT(saveSnapshot()));

	for (int i = 0; i < MAXRECENTFILES; ++i)
	{
		recentFileActs[i] = new QAction(this);
		recentFileActs[i]->setVisible(true);
		recentFileActs[i]->setEnabled(true);
		recentFileActs[i]->setShortcutContext(Qt::ApplicationShortcut);
		recentFileActs[i]->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_1 + i));
		connect(recentFileActs[i], SIGNAL(triggered()), this, SLOT(openRecentMesh()));
	}

	exitAct = new QAction(codec->toUnicode("�˳�") /*tr("E&xit")*/, this);
	exitAct->setShortcut(Qt::CTRL + Qt::Key_Q);
	connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

	//////////////Action Menu View ////////////////////////////////////////////////////////////////////////////
	fullScreenAct = new QAction(codec->toUnicode("&ȫ��") /*tr("&FullScreen")*/, this);
	fullScreenAct->setCheckable(true);
	fullScreenAct->setShortcutContext(Qt::ApplicationShortcut);
	fullScreenAct->setShortcut(Qt::ALT + Qt::Key_Return);
	connect(fullScreenAct, SIGNAL(triggered()), this, SLOT(fullScreen()));

	showToolbarStandardAct = new QAction(codec->toUnicode("��׼") /*tr("&Standard")*/, this);
	showToolbarStandardAct->setCheckable(true);
	showToolbarStandardAct->setChecked(true);
	connect(showToolbarStandardAct, SIGNAL(triggered()), this, SLOT(showToolbarFile()));

	// Added by Johnny xu, 2017/2/21
	showToolbarEditAct = new QAction (/*tr("&Edit")*/codec->toUnicode("�༭"), this);
	showToolbarEditAct->setCheckable(true);
	showToolbarEditAct->setChecked(true);
    connect(showToolbarEditAct, SIGNAL(triggered()), this, SLOT(showToolbarEdit()));

	// Added by Johnny xu, 2017/2/21
	showToolbarFilterAct = new QAction (/*tr("&Filter")*/codec->toUnicode("����"), this);
	showToolbarFilterAct->setCheckable(true);
	showToolbarFilterAct->setChecked(true);
    connect(showToolbarFilterAct, SIGNAL(triggered()), this, SLOT(showToolbarFilter()));

	// Added by Johnny xu, 2017/2/21
	showToolbarDecoratorAct = new QAction (/*tr("&Filter")*/codec->toUnicode("װ��"), this);
	showToolbarDecoratorAct->setCheckable(true);
	showToolbarDecoratorAct->setChecked(true);
    connect(showToolbarDecoratorAct, SIGNAL(triggered()), this, SLOT(showToolbarDecorator()));

	showInfoPaneAct = new QAction(codec->toUnicode("&��ʾ��Ϣ���") /*tr("Show Info &Panel")*/, this);
	showInfoPaneAct->setCheckable(true);
	connect(showInfoPaneAct, SIGNAL(triggered()), this, SLOT(showInfoPane()));

	showTrackBallAct = new QAction(codec->toUnicode("&��ʾ������") /*tr("Show &Trackball")*/, this);
	showTrackBallAct->setCheckable(true);
	connect(showTrackBallAct, SIGNAL(triggered()), this, SLOT(showTrackBall()));

	// Fix bug #22 by Johnny Xu, 2017/2/28
	resetTrackBallAct = new QAction(QIcon(":/images/view_raster.png"), codec->toUnicode("&����ģ��") /*tr("Reset &Trackball")*/, this);
	resetTrackBallAct->setShortcutContext(Qt::ApplicationShortcut);
#if defined(Q_OS_MAC)
	resetTrackBallAct->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_H);
#else
	resetTrackBallAct->setShortcut(Qt::CTRL + Qt::Key_H);
#endif
	connect(resetTrackBallAct, SIGNAL(triggered()), this, SLOT(resetTrackBall()));

	showLayerDlgAct = new QAction(QIcon(":/images/layers.png"), codec->toUnicode("&��ʾͼ�㴰��") /*tr("Show Layer Dialog")*/, this);
	showLayerDlgAct->setCheckable(true);
	showLayerDlgAct->setChecked(true);
	showLayerDlgAct->setShortcut(Qt::CTRL + Qt::Key_L);
	connect(showLayerDlgAct, SIGNAL(triggered(bool)), this, SLOT(showLayerDlg(bool)));

	//////////////Action Menu EDIT /////////////////////////////////////////////////////////////////////////
	suspendEditModeAct = new QAction(QIcon(":/images/no_edit.png"), codec->toUnicode("�Ǳ༭ģʽ") /*tr("Not editing")*/, this);
	suspendEditModeAct->setShortcut(Qt::Key_Escape);
	suspendEditModeAct->setCheckable(true);
	suspendEditModeAct->setChecked(true);
	connect(suspendEditModeAct, SIGNAL(triggered()), this, SLOT(suspendEditMode()));

	////////////////Action Menu About ///////////////////////////////////////////////////////////////////////////
	aboutAct = new QAction(codec->toUnicode("&����") /*tr("&About")*/, this);
	connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

	onscreenHelpAct = new QAction(codec->toUnicode("&���ٰ���") /*tr("On screen quick help")*/, this);
	onscreenHelpAct->setShortcut(Qt::Key_F1);
	onscreenHelpAct->setShortcutContext(Qt::ApplicationShortcut);
	connect(onscreenHelpAct, SIGNAL(triggered()), this, SLOT(helpOnscreen()));
}

void MainWindow::createToolBars()
{
#if defined(Q_OS_MAC)
	this->setStyleSheet("QToolBar {spacing: 0px; } QToolButton {border-radius: 0px;} QToolButton:checked {background: darkgray}");
#endif

	QTextCodec *codec = QTextCodec::codecForName("GBK");

	mainToolBar = addToolBar(codec->toUnicode("��׼")/*tr("Standard")*/);
	mainToolBar->addAction(importMeshAct);
	mainToolBar->addAction(reloadMeshAct);
//  mainToolBar->addAction(reloadAllMeshAct);
	mainToolBar->addAction(exportMeshAct);
	mainToolBar->addAction(saveSnapshotAct);
	mainToolBar->addAction(showLayerDlgAct);
	// Fix bug #22 by Johnny Xu, 2017/2/28
	mainToolBar->addAction(resetTrackBallAct);

	mainToolBar->addSeparator();
	globrendtoolbar = new MLRenderingGlobalToolbar(this);
	connect(globrendtoolbar, SIGNAL(updateRenderingDataAccordingToActions(QList<MLRenderingGlobalAction*>)), this, SLOT(updateRenderingDataAccordingToActions(QList<MLRenderingGlobalAction*>)));
	mainToolBar->addWidget(globrendtoolbar);
	mainToolBar->setIconSize(QSize(32, 32));

	decoratorToolBar = addToolBar(codec->toUnicode("װ��") /*"Decorator"*/);
	foreach(MeshDecorateInterface *iDecorate, PM.meshDecoratePlugins())
	{
		foreach(QAction *decorateAction, iDecorate->actions())
		{
			if (!decorateAction->icon().isNull())
				decoratorToolBar->addAction(decorateAction);
		}
	}

	decoratorToolBar->setIconSize(QSize(32, 32));

	editToolBar = addToolBar(codec->toUnicode("�༭") /*tr("Edit")*/);
	editToolBar->addAction(suspendEditModeAct);
	foreach(MeshEditInterfaceFactory *iEditFactory, PM.meshEditFactoryPlugins())
	{
		foreach(QAction* editAction, iEditFactory->actions())
		{
			if (!editAction->icon().isNull())
			{
				editToolBar->addAction(editAction);
			}
			else qDebug() << "action was null";
		}
	}
	editToolBar->setIconSize(QSize(32, 32));

	filterToolBar = addToolBar(codec->toUnicode("����") /*tr("Filter")*/);
	filterToolBar->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
	foreach(MeshFilterInterface *iFilter, PM.meshFilterPlugins())
	{
		foreach(QAction* filterAction, iFilter->actions())
		{
			if (!filterAction->icon().isNull())
			{
				if (filterAction->priority() != QAction::LowPriority)
					filterToolBar->addAction(filterAction);
			} 
		}
	}
	filterToolBar->setIconSize(QSize(32, 32));

	// Added by Johnny Xu, 2017/5/3
	mainToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	globrendtoolbar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	decoratorToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	editToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	filterToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
}

void MainWindow::createMenus()
{
	QTextCodec *codec = QTextCodec::codecForName("GBK");

	//////////////////// Menu File ////////////////////////////////////////////////////////////////////////////
	fileMenu = menuBar()->addMenu(codec->toUnicode("�ļ�") /*tr("&File")*/);
	fileMenu->addAction(importMeshAct);
	fileMenu->addAction(exportMeshAct);
	fileMenu->addAction(exportMeshAsAct);
	fileMenu->addAction(reloadMeshAct);
	fileMenu->addAction(reloadAllMeshAct);
	fileMenu->addSeparator();

	fileMenu->addAction(saveSnapshotAct);
	separatorAct = fileMenu->addSeparator();
	recentFileMenu = fileMenu->addMenu(codec->toUnicode("&���ʹ���ļ�") /*tr("Recent Files")*/);

	for (int i = 0; i < MAXRECENTFILES; ++i)
	{
		recentFileMenu->addAction(recentFileActs[i]);
	}

	fileMenu->addSeparator();
	fileMenu->addAction(exitAct);

	//////////////////// Menu Edit //////////////////////////////////////////////////////////////////////////
	editMenu = menuBar()->addMenu(codec->toUnicode("�༭") /*tr("&Edit")*/);
	editMenu->addAction(suspendEditModeAct);

	//////////////////// Menu Filter //////////////////////////////////////////////////////////////////////////
	filterMenu = menuBar()->addMenu(codec->toUnicode("����") /*tr("Fi&lters")*/);

	//////////////////// Menu Render //////////////////////////////////////////////////////////////////////////
	renderMenu = menuBar()->addMenu(codec->toUnicode("��Ⱦ") /*tr("&Render")*/);

	//////////////////// Menu View ////////////////////////////////////////////////////////////////////////////
	viewMenu = menuBar()->addMenu(codec->toUnicode("��ͼ") /*tr("&View")*/);
	viewMenu->addAction(fullScreenAct);
	viewMenu->addSeparator();
	viewMenu->addAction(showLayerDlgAct);
	viewMenu->addSeparator();
	//remove by Bangui Ruan
//	viewMenu->addAction(showTrackBallAct);
	viewMenu->addAction(resetTrackBallAct);
	viewMenu->addSeparator();
	viewMenu->addAction(showInfoPaneAct);
	viewMenu->addSeparator();

	toolBarMenu = viewMenu->addMenu(codec->toUnicode("������") /*tr("&ToolBars")*/);
	toolBarMenu->addAction(showToolbarStandardAct);

	// Added by Johnny Xu, 2017/2/21
	toolBarMenu->addAction(showToolbarDecoratorAct);
	toolBarMenu->addAction(showToolbarEditAct);
	toolBarMenu->addAction(showToolbarFilterAct);

	//////////////////// Menu Help ////////////////////////////////////////////////////////////////
	helpMenu = menuBar()->addMenu(codec->toUnicode("����") /*tr("&Help")*/);
	helpMenu->addAction(aboutAct);
	helpMenu->addAction(onscreenHelpAct);

	fillFilterMenu();
	fillEditMenu();
	fillDecorateMenu();
}

QString MainWindow::getDecoratedFileName(const QString& name)
{
	return  QString("<br><b><i>(") + name + ")</i></b>";
}

void MainWindow::fillFilterMenu()
{
	QTextCodec *codec = QTextCodec::codecForName("GBK");

	filterMenu->clear();
	filterMenuSelect = new MenuWithToolTip(codec->toUnicode("ѡ��") /*tr("Selection")*/, this);
	filterMenu->addMenu(filterMenuSelect);

	filterMenuClean = new MenuWithToolTip(codec->toUnicode("����") /*tr("Cleaning and Repairing")*/, this);
	filterMenu->addMenu(filterMenuClean);

	filterMenuSmoothing = new MenuWithToolTip(codec->toUnicode("ƽ��") /*tr("Smoothing, Fairing and Deformation")*/, this);
	filterMenu->addMenu(filterMenuSmoothing);

	filterMenuNormal = new MenuWithToolTip(codec->toUnicode("����") /*tr("Normals, Curvatures and Orientation")*/, this);
	// Delete orientation menu by Johnny Xu, 2017/6/28
	// filterMenu->addMenu(filterMenuNormal);

	filterMenuMeshLayer = new MenuWithToolTip(codec->toUnicode("����ͼ��") /*tr("Mesh Layer")*/, this);
	filterMenu->addMenu(filterMenuMeshLayer);

	filterMenuSampling = new MenuWithToolTip(codec->toUnicode("����") /*tr("Sampling")*/, this);
	// Delete Sampling menu by Johnny Xu, 2017/6/28
	// filterMenu->addMenu(filterMenuSampling);

	QMap<QString, MeshFilterInterface *>::iterator msi;
	for (msi = PM.stringFilterMap.begin(); msi != PM.stringFilterMap.end(); ++msi)
	{
		MeshFilterInterface * iFilter = msi.value();
		QAction *filterAction = iFilter->AC((msi.key()));

		// Commented by Johnny Xu, 2017/2/21
		//QString tooltip = iFilter->filterInfo(filterAction) + "<br>" + getDecoratedFileName(filterAction->data().toString());
		//filterAction->setToolTip(tooltip);

		connect(filterAction, SIGNAL(triggered()), this, SLOT(startFilter()));

		// Add 'else' condition by Johnny Xu, 2017/2/21
		int filterClass = iFilter->getClass(filterAction);
		if( filterClass & MeshFilterInterface::Selection )
        {
            filterMenuSelect->addAction(filterAction);
        }
		else if( filterClass & MeshFilterInterface::Cleaning )
        {
            filterMenuClean->addAction(filterAction);
        }
		else if( filterClass & MeshFilterInterface::Smoothing )
        {
            filterMenuSmoothing->addAction(filterAction);
        }
        else if( filterClass & MeshFilterInterface::Normal )
        {
            filterMenuNormal->addAction(filterAction);
        }
		else if( filterClass & MeshFilterInterface::Layer )
        {
            filterMenuMeshLayer->addAction(filterAction);
        }
		else if( filterClass & MeshFilterInterface::Sampling )
        {
            filterMenuSampling->addAction(filterAction);
        }

		//  MeshFilterInterface::Generic
		if (filterClass == 0)
		{
			filterMenu->addAction(filterAction);
		}
	}

	QMap<QString, MeshLabXMLFilterContainer>::iterator xmlit;
	for (xmlit = PM.stringXMLFilterMap.begin(); xmlit != PM.stringXMLFilterMap.end(); ++xmlit)
	{
		try
		{
			//MeshLabFilterInterface * iFilter= xmlit.value().filterInterface;
			QAction *filterAction = xmlit.value().act;
			if (filterAction == NULL)
				throw MLException(QTextCodec::codecForName("GBK")->toUnicode("��Ч�Ĺ���ֵ!") /*"Invalid filter action value."*/);

			MLXMLPluginInfo* info = xmlit.value().xmlInfo;
			if (filterAction == NULL)
				throw MLException(QTextCodec::codecForName("GBK")->toUnicode("��Ч�Ĺ���ֵ!") /*"Invalid filter info value."*/);

			QString filterName = xmlit.key();
			QString help = info->filterHelp(filterName);

			// Commented by Johnny Xu,2017/2/21
//			filterAction->setToolTip(help + getDecoratedFileName(filterAction->data().toString()));

			connect(filterAction, SIGNAL(triggered()), this, SLOT(startFilter()));
			QString filterClasses = info->filterAttribute(filterName, MLXMLElNames::filterClass);
			QStringList filterClassesList = filterClasses.split(QRegExp("\\W+"), QString::SkipEmptyParts);
			
			foreach(QString nameClass, filterClassesList)
			{
				// Add 'else' condition by Johnny Xu, 2017/2/22
				if( nameClass == QString("Selection"))
                {
                    filterMenuSelect->addAction(filterAction);
                }
                else if( nameClass == QString("Cleaning"))
                {
                    filterMenuClean->addAction(filterAction);
                }
				else if( nameClass == QString("Smoothing"))
                {
                    filterMenuSmoothing->addAction(filterAction);
                }
                else if( nameClass == QString("Normal"))
                {
                    filterMenuNormal->addAction(filterAction);
                }
				else if( nameClass == QString("Layer"))
                {
                    filterMenuMeshLayer->addAction(filterAction);
                }
				else if( nameClass == QString("Sampling"))
                {
                    filterMenuSampling->addAction(filterAction);
                }

                //  MeshFilterInterface::Generic
                if(	nameClass == QString("Generic"))
                {
                    filterMenu->addAction(filterAction);
                }
			}
		}
		catch (ParsingException e)
		{
			meshDoc()->Log.Logf(GLLogStream::SYSTEM, e.what());
		}
	}
}

void MainWindow::fillDecorateMenu()
{
	foreach(MeshDecorateInterface *iDecorate, PM.meshDecoratePlugins())
	{
		foreach(QAction *decorateAction, iDecorate->actions())
		{
			connect(decorateAction, SIGNAL(triggered()), this, SLOT(applyDecorateMode()));
			decorateAction->setToolTip(iDecorate->decorationInfo(decorateAction));
			renderMenu->addAction(decorateAction);
		}
	}

	connect(renderMenu, SIGNAL(hovered(QAction*)), this, SLOT(showTooltip(QAction*)));
}

void MainWindow::fillEditMenu()
{
	foreach(MeshEditInterfaceFactory *iEditFactory, PM.meshEditFactoryPlugins())
	{
		foreach(QAction* editAction, iEditFactory->actions())
		{
			editMenu->addAction(editAction);
			connect(editAction, SIGNAL(triggered()), this, SLOT(applyEditMode()));
		}
	}
}

void MainWindow::loadSettings()
{
	MainWindowSetting::initGlobalParameterSet(&defaultGlobalParams);
	GLArea::initGlobalParameterSet(&defaultGlobalParams);

	// Fix bug #18 to comment code by Johnny xu, 2017/2/22
	//QSettings settings;
	//QStringList klist = settings.allKeys();
	//for (int ii = 0; ii < klist.size(); ++ii)
	//{
	//	QDomDocument doc;
	//	doc.setContent(settings.value(klist.at(ii)).toString());

	//	QString st = settings.value(klist.at(ii)).toString();
	//	QDomElement docElem = doc.firstChild().toElement();

	//	RichParameter* rpar = NULL;
	//	if (!docElem.isNull())
	//	{
	//		bool ret = RichParameterFactory::create(docElem, &rpar);
	//		if (!ret)
	//		{
	//			//  qDebug("Warning Ignored parameter '%s' = '%s'. Malformed.", qPrintable(docElem.attribute("name")),qPrintable(docElem.attribute("value")));
	//			continue;
	//		}
	//		if (!defaultGlobalParams.hasParameter(rpar->name))
	//		{
	//			//  qDebug("Warning Ignored parameter %s. In the saved parameters there are ones that are not in the HardWired ones. "
	//			//         "It happens if you are running MeshLab with only a subset of the plugins. ",qPrintable(rpar->name));
	//		}
	//		else
	//			currentGlobalParams.addParam(rpar);
	//	}
	//}

	// eventually fill missing values with the hardwired defaults
	for (int ii = 0; ii < defaultGlobalParams.paramList.size(); ++ii)
	{
		//		qDebug("Searching param[%i] %s of the default into the loaded settings. ",ii,qPrintable(defaultGlobalParams.paramList.at(ii)->name));
		if (!currentGlobalParams.hasParameter(defaultGlobalParams.paramList.at(ii)->name))
		{
			qDebug("Warning! a default param was not found in the saved settings. This should happen only on the first run...");
			RichParameterCopyConstructor v;
			defaultGlobalParams.paramList.at(ii)->accept(v);
			currentGlobalParams.paramList.push_back(v.lastCreated);

			QDomDocument doc("MeshLabSettings");
			RichParameterXMLVisitor vxml(doc);
			v.lastCreated->accept(vxml);
			doc.appendChild(vxml.parElem);
			QString docstring = doc.toString();
			QSettings setting;
			setting.setValue(v.lastCreated->name, QVariant(docstring));
		}
	}

	//emit dispatchCustomSettings(currentGlobalParams);
}

void MainWindow::addToMenu(QList<QAction *> actionList, QMenu *menu, const char *slot)
{
	foreach(QAction *a, actionList)
	{
		connect(a, SIGNAL(triggered()), this, slot);
		menu->addAction(a);
	}
}

// this function update the app settings with the current recent file list
// and update the loaded mesh counter
void MainWindow::saveRecentFileList(const QString &fileName)
{
	QSettings settings;
	QStringList files = settings.value("recentFileList").toStringList();
	files.removeAll(fileName);
	files.prepend(fileName);
	while (files.size() > MAXRECENTFILES)
		files.removeLast();

	//avoid the slash/back-slash path ambiguity
	for (int ii = 0; ii < files.size(); ++ii)
		files[ii] = QDir::fromNativeSeparators(files[ii]);
	settings.setValue("recentFileList", files);

	foreach(QWidget *widget, QApplication::topLevelWidgets()) 
	{
		MainWindow *mainWin = qobject_cast<MainWindow *>(widget);
		if (mainWin) 
			mainWin->updateRecentFileActions();
	}

	settings.setValue("totalKV", settings.value("totalKV", 0).toInt() + (GLA()->mm()->cm.vn) / 1000);
	settings.setValue("loadedMeshCounter", settings.value("loadedMeshCounter", 0).toInt() + 1);
}

void MainWindow::wrapSetActiveSubWindow(QWidget* window) 
{
	QMdiSubWindow* subwindow;
	subwindow = dynamic_cast<QMdiSubWindow*>(window);
	if (subwindow != NULL)
	{
		mdiarea->setActiveSubWindow(subwindow);
	}
	else 
	{
		qDebug("Type of window is not a QMdiSubWindow*");
	}
}

void MainWindowSetting::initGlobalParameterSet(RichParameterSet* glbset)
{
	glbset->addParam(new RichInt(maximumDedicatedGPUMem(), 350, QTextCodec::codecForName("GBK")->toUnicode("GPU�ڴ�ռ��ֵ"), "" 
		/* "Maximum GPU Memory Dedicated to MeshLab (Mb)", "Maximum GPU Memory Dedicated to MeshLab (megabyte) for the storing of the geometry attributes. The dedicated memory must NOT be all the GPU memory presents on the videocard."*/));
	
	glbset->addParam(new RichInt(perBatchPrimitives(), 100000, QTextCodec::codecForName("GBK")->toUnicode("GPU�����������ԭ��"), "" 
		/* "Per batch primitives loaded in GPU", "Per batch primitives (vertices and faces) loaded in the GPU memory. It's used in order to do not overwhelm the system memory with an entire temporary copy of a mesh."*/));
	
	glbset->addParam(new RichInt(minPolygonNumberPerSmoothRendering(), 50000, QTextCodec::codecForName("GBK")->toUnicode("ƽ����Ⱦ��Ĭ������"), "" 
		/* "Default Face number per smooth rendering", "Minimum number of faces in order to automatically render a newly created mesh layer with the per vertex normal attribute activated."*/));

	glbset->addParam(new RichBool(perMeshRenderingToolBar(), true, QTextCodec::codecForName("GBK")->toUnicode("��ʾ��Ⱦ������"), "" 
		/*"Show Per-Mesh Rendering Side ToolBar", "If true the per-mesh rendering side toolbar will be redendered inside the layerdialog."*/));

	//WARNING!!!! REMOVE THIS LINE AS SOON AS POSSIBLE! A plugin global variable has been introduced by MeshLab Core!
	glbset->addParam(new RichString("MeshLab::Plugins::sketchFabKeyCode", "0000000", QTextCodec::codecForName("GBK")->toUnicode("SketchFab����") /*"SketchFab KeyCode"*/, ""));
	/****************************************************************************************************************/

	if (MeshLabScalarTest<Scalarm>::doublePrecision())
		glbset->addParam(new RichBool(highPrecisionRendering(), false, QTextCodec::codecForName("GBK")->toUnicode("�߾�����Ⱦ"), "" 
		/*"High Precision Rendering", "If true all the models in the scene will be rendered at the center of the world"*/));

	glbset->addParam(new RichInt(maxTextureMemoryParam(), 256, QTextCodec::codecForName("GBK")->toUnicode("��������ڴ�(��)"), "" 
		/*"Max Texture Memory (in MB)", "The maximum quantity of texture memory allowed to load mesh textures"*/));
}

void MainWindowSetting::updateGlobalParameterSet(RichParameterSet& rps)
{
	maxgpumem = (std::ptrdiff_t)rps.getInt(maximumDedicatedGPUMem()) * (float)(1024 * 1024);
	perbatchprimitives = (size_t)rps.getInt(perBatchPrimitives());
	minpolygonpersmoothrendering = (size_t)rps.getInt(minPolygonNumberPerSmoothRendering());
	permeshtoolbar = rps.getBool(perMeshRenderingToolBar());
	highprecision = false;

	if (MeshLabScalarTest<Scalarm>::doublePrecision())
		highprecision = rps.getBool(highPrecisionRendering());
	maxTextureMemory = (std::ptrdiff_t) rps.getInt(this->maxTextureMemoryParam()) * (float)(1024 * 1024);
}

void MainWindow::defaultPerViewRenderingData(MLRenderingData& dt) const
{
	MLRenderingData::RendAtts tmpatts;
	tmpatts[MLRenderingData::ATT_NAMES::ATT_VERTPOSITION] = true;
	tmpatts[MLRenderingData::ATT_NAMES::ATT_VERTNORMAL] = true;
	tmpatts[MLRenderingData::ATT_NAMES::ATT_VERTCOLOR] = true;
	tmpatts[MLRenderingData::ATT_NAMES::ATT_WEDGETEXTURE] = true;
	dt.set(MLRenderingData::PR_SOLID, tmpatts);
	MLPerViewGLOptions opts;
	dt.set(opts);
}

// Added by Johnny Xu, 2017/7/26
void MainWindow::writeSettings()
{
	QSettings settings("Scientree Inc.", "Model Editor");
	settings.beginGroup("mainWindow");
	settings.setValue("size", size());
	settings.setValue("state", saveState());
	settings.endGroup();
}

// Added by Johnny Xu, 2017/7/26
void MainWindow::readSettings()
{
	QSettings settings("Scientree Inc.", "Model Editor");
	settings.beginGroup("mainWindow");
	resize(settings.value("size").toSize());
	restoreState(settings.value("state").toByteArray());
	settings.endGroup();
}