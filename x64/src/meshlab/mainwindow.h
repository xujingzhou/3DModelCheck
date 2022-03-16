
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

//None of this should happen if we are compiling c, not c++
#ifdef __cplusplus

#include <GL/glew.h>

#include "../common/pluginmanager.h"
#include "../common/scriptinterface.h"

#include <wrap/qt/qt_thread_safe_memory_info.h>

#include "glarea.h"
#include "layerDialog.h"
#include "ml_std_par_dialog.h"
#include "xmlstdpardialog.h"

#include "multiViewer_Container.h"
#include "ml_render_gui.h"

#include <QtScript>
#include <QDir>
#include <QMainWindow>
#include <QMdiArea>
#include <QStringList>
#include <QColorDialog>
#include <QMdiSubWindow>
#include <QSplitter>
#include <QCloseEvent>

#define MAXRECENTFILES 4

class QAction;
class QActionGroup;
class QMenu;
class QScrollArea;
class QSignalMapper;
class QProgressDialog;
class QNetworkAccessManager;
class QNetworkReply;
class QToolBar;

class MainWindowSetting
{
public:
    static void initGlobalParameterSet(RichParameterSet* gblset);
    void updateGlobalParameterSet( RichParameterSet& rps );

    std::ptrdiff_t maxgpumem;
    inline static QString maximumDedicatedGPUMem() 
	{
		return "MeshLab::System::maxGPUMemDedicatedToGeometry";
	}

    bool permeshtoolbar;
    inline static QString perMeshRenderingToolBar() 
	{
		return "MeshLab::GUI::perMeshToolBar";
	}

    bool highprecision;
    inline static QString highPrecisionRendering() 
	{
		return "MeshLab::System::highPrecisionRendering";
	}

    size_t perbatchprimitives;
    inline static QString perBatchPrimitives() 
	{
		return "MeshLab::System::perBatchPrimitives";
	}

	size_t minpolygonpersmoothrendering;
	inline static QString minPolygonNumberPerSmoothRendering() 
	{ 
		return "MeshLab::System::minPolygonNumberPerSmoothRendering";
	}

    std::ptrdiff_t maxTextureMemory;
    inline static QString maxTextureMemoryParam()  
	{
		return "MeshLab::System::maxTextureMemory";
	}
};



class MainWindow : public QMainWindow, public MainWindowInterface
{
    Q_OBJECT

public:

    // callback function to execute a filter
    void executeFilter(QAction *action, RichParameterSet &srcpar, bool isPreview = false);
    //parexpval is a string map containing the parameter expression values set in the filter's dialog.
    //These parameter expression values will be evaluated when the filter will start.
    void executeFilter(MeshLabXMLFilterContainer*,const QMap<QString,QString>& parexpval , bool  isPreview = false);

    MainWindow();
    ~MainWindow();

    static bool QCallBack(const int pos, const char * str);
    MainWindowSetting mwsettings;

	// Added by Johnny Xu, 2017/7/17
	void closeEvent(QCloseEvent *event);

signals:
    void dispatchCustomSettings(RichParameterSet& rps);
    void filterExecuted();
    void updateLayerTable();

protected:
	void paintEvent(QPaintEvent*);

private slots:
    void newProject(const QString& projName = QString());
    
    void meshAdded(int mid);
    void meshRemoved(int mid);

public slots:
    bool importMeshWithLayerManagement(QString fileName=QString());
    bool importRaster(const QString& fileImg = QString());

    void updateCustomSettings();
	void updateLayerDialog();
	bool addRenderingDataIfNewlyGeneratedMesh(int meshid);

	void updateRenderingDataAccordingToActions(int meshid, const QList<MLRenderingAction*>& acts);
	void updateRenderingDataAccordingToActionsToAllVisibleLayers(const QList<MLRenderingAction*>& acts);
	void updateRenderingDataAccordingToActions(int meshid, MLRenderingAction* act, QList<MLRenderingAction*>& acts);
	void updateRenderingDataAccordingToAction(int meshid, MLRenderingAction* act);
	void updateRenderingDataAccordingToActionToAllVisibleLayers(MLRenderingAction* act);
	void updateRenderingDataAccordingToAction(int meshid, MLRenderingAction* act,bool);
	void updateRenderingDataAccordingToActions(QList<MLRenderingGlobalAction*> actlist);



private:
	void updateRenderingDataAccordingToActionsCommonCode(int meshid, const QList<MLRenderingAction*>& acts);
	void updateRenderingDataAccordingToActionCommonCode(int meshid, MLRenderingAction* act);

	// Added by Johnny Xu, 2017/3/1
	void showContextMenu(bool visible);

	// Added by Johnny Xu, 2017/7/26
	void writeSettings();
	void readSettings();

private slots:
	void documentUpdateRequested();
    bool importMesh(QString fileName=QString(), bool isareload = false);
    void endEdit();

    void postFilterExecution(/*MeshLabXMLFilterContainer* mfc*/);

    void updateProgressBar(const int pos,const QString& text);
    void updateTexture(int meshid);

public:

    bool exportMesh(QString fileName,MeshModel* mod,const bool saveAllPossibleAttributes);
    bool loadMesh(const QString& fileName,MeshIOInterface *pCurrentIOPlugin,MeshModel* mm,int& mask,RichParameterSet* prePar,const Matrix44m &mtr=Matrix44m::Identity(), bool isareload = false);

	void computeRenderingDataOnLoading(MeshModel* mm,bool isareload);

	bool loadMeshWithStandardParams(QString& fullPath, MeshModel* mm, const Matrix44m &mtr = Matrix44m::Identity(),bool isareload = false);
    
    void defaultPerViewRenderingData(MLRenderingData& dt) const;
    void getRenderingData(int mid,MLRenderingData& dt) const;
    void setRenderingData(int mid,const MLRenderingData& dt);

	unsigned int viewsRequiringRenderingActions(int meshid,MLRenderingAction* act);

    void updateSharedContextDataAfterFilterExecution(int postcondmask,int fclasses,bool& newmeshcreated);

private slots:

	void closeCurrentDocument();

    //////////// Slot Menu File //////////////////////
    void reload();
    void reloadAllMesh();
    void openRecentMesh();

	// Delete save setup dialog by Johnny Xu, 2017/6/28.
    bool saveAs(QString fileName = QString(),const bool saveAllPossibleAttributes = true /*false*/);
    bool save(const bool saveAllPossibleAttributes = true /*false*/);

    bool saveSnapshot();
    void changeFileExtension(const QString&);

    ///////////Slot Menu Edit ////////////////////////
    void applyEditMode();
    void suspendEditMode();

    ///////////Slot Menu Filter ////////////////////////
    void startFilter();
	void showTooltip(QAction*);

    void applyRenderMode();
    void applyDecorateMode();

    void switchOffDecorator(QAction* );

    ///////////Slot Menu View ////////////////////////
    void fullScreen();

    void showToolbarFile();

	// Added by Johnny Xu, 2017/2/21
	void showToolbarEdit();
	void showToolbarFilter();
	void showToolbarDecorator();

    void showInfoPane();
    void showTrackBall();
    void resetTrackBall();
    void showLayerDlg(bool visible);

    ///////////Slot Menu Windows /////////////////////
    void updateMenus();
    void updateSubFiltersMenu(const bool createmenuenabled,const bool validmeshdoc);
    void updateMenuItems(QMenu* menu,const bool enabled);
    void updateStdDialog();
    void updateXMLStdDialog();
    void enableDocumentSensibleActionsContainer(const bool enable);

    ///////////Slot Menu Help ////////////////////////
    void about();
	void helpOnscreen();

    void dropEvent ( QDropEvent * event );
    void dragEnterEvent(QDragEnterEvent *);

    ///////////Solt Wrapper for QMdiArea //////////////////
    void wrapSetActiveSubWindow(QWidget* window);
	void switchCurrentContainer(QMdiSubWindow *);
    void scriptCodeExecuted(const QScriptValue& val,const int time,const QString& output);
    
    void updateGPUMemBar(int,int);
	void updateLog();

private:
    void addRenderingSystemLogInfo(unsigned mmid);

    void createStdPluginWnd(const QString &titlename); // this one is
    void createXMLStdPluginWnd();
    void initGlobalParameters();

    void createActions();
    void createMenus();

    void fillFilterMenu();
    void fillDecorateMenu();
	void fillEditMenu();
    
    void createToolBars();
    void loadSettings();

    void keyPressEvent(QKeyEvent *);

    void updateRecentFileActions();
    void saveRecentFileList(const QString &fileName);

    void addToMenu(QList<QAction *>, QMenu *menu, const char *slot);

    void initDocumentMeshRenderState(MeshLabXMLFilterContainer* mfc);
    void initDocumentRasterRenderState(MeshLabXMLFilterContainer* mfc);
	void setCurrentMeshBestTab();

    MeshlabStdDialog *stddialog;
    MeshLabXMLStdDialog* xmldialog;
    static QProgressBar *qb;

    QMdiArea *mdiarea;
    LayerDialog *layerDialog;

    QSignalMapper *windowMapper;
    vcg::QtThreadSafeMemoryInfo* gpumeminfo;
	QLabel			*libGPUProcess;
    RichParameterSet currentGlobalParams;
    RichParameterSet defaultGlobalParams;

    QByteArray toolbarState;								//stato delle toolbar e dockwidgets

    QDir lastUsedDirectory;  //This will hold the last directory that was used to load/save a file/project in

public:
    PluginManager PM;

    MeshDocument *meshDoc() 
	{
        if (currentViewContainer() != NULL)
			return &currentViewContainer()->meshDoc;
		return NULL;
    }

    inline vcg::QtThreadSafeMemoryInfo* memoryInfoManager() const {return gpumeminfo;} 
    const RichParameterSet& currentGlobalPars() const { return currentGlobalParams; }
    RichParameterSet& currentGlobalPars() { return currentGlobalParams; }
    const RichParameterSet& defaultGlobalPars() const { return defaultGlobalParams; }

    GLArea *GLA() const 
	{
        //	  if(mdiarea->currentSubWindow()==0) return 0;
        MultiViewer_Container *mvc = currentViewContainer();
        if(!mvc) 
			return 0;
        return  mvc->currentgla;
    }

    MultiViewer_Container* currentViewContainer() const 
	{
        return _currviewcontainer;
    }
    
    const PluginManager& pluginManager() const 
	{ 
		return PM; 
	}

    static QStatusBar *&globalStatusBar()
    {
        static QStatusBar *_qsb=0;
        return _qsb;
    }
    
    void setHandleMenu(QPoint point, Qt::Orientation orientation, QSplitter *origin);

    QMenu* meshLayerMenu() 
	{ 
		return filterMenuMeshLayer; 
	}

private:
    //the xml filters run in a different thread. The xmlfiltertimer starts on executeFilter and stops on postFilterExecution
    //function linked to the thread finished signal.
    QTime xmlfiltertimer;
    WordActionsMapAccessor wama;

    //////// ToolBars ///////////////
    QToolBar *mainToolBar;
    QToolBar *decoratorToolBar;
    QToolBar *editToolBar;
    QToolBar *filterToolBar;
	MLRenderingGlobalToolbar* globrendtoolbar;

    ///////// Menus ///////////////
    QMenu *fileMenu;
    QMenu* recentFileMenu;

	QMenu *filterMenu;
    QMenu *filterMenuSelect;
    QMenu *filterMenuClean;
	QMenu *filterMenuSmoothing;
	QMenu *filterMenuMeshLayer;
	QMenu *filterMenuNormal;
	QMenu *filterMenuSampling;

    QMenu *editMenu;
    QMenu *renderMenu;

    //View Menu and SubMenu //////
    QMenu *viewMenu;
    QMenu *toolBarMenu;
	QMenu *helpMenu;

    //////////// Actions Menu File ///////////////////////
    QAction *importMeshAct, *exportMeshAct, *exportMeshAsAct;
    QAction *reloadMeshAct;
    QAction *reloadAllMeshAct;
    QAction *saveSnapshotAct;
    QAction *recentFileActs[MAXRECENTFILES];
    QAction *separatorAct;
    QAction *exitAct;

    /////////// Actions Menu Edit  /////////////////////
    QAction *suspendEditModeAct;
   
    ///////////Actions Menu View ////////////////////////
    QAction *fullScreenAct;
    QAction *showToolbarStandardAct;

	// Added by Johnny Xu, 2017/2/21
	QAction *showToolbarEditAct;
	QAction *showToolbarFilterAct;
	QAction *showToolbarDecoratorAct;

    QAction *showInfoPaneAct;
    QAction *showTrackBallAct;
    QAction *resetTrackBallAct;
    QAction *showLayerDlgAct;

public:

private:

    ///////////Actions Menu Help ////////////////////////
    QAction *aboutAct;
	QAction *onscreenHelpAct;
    ////////////////////////////////////////////////////

    static QString getDecoratedFileName(const QString& name);

	MultiViewer_Container* _currviewcontainer;
};

/// Event filter that is installed to intercept the open events sent directly by the Operative System
class FileOpenEater : public QObject
{
    Q_OBJECT

public:
    FileOpenEater(MainWindow *_mainWindow)
    {
        mainWindow= _mainWindow;
        noEvent=true;
    }

    MainWindow *mainWindow;
    bool noEvent;

protected:

    bool eventFilter(QObject *obj, QEvent *event)
    {
        if (event->type() == QEvent::FileOpen) 
		{
            noEvent = false;
            QFileOpenEvent *fileEvent = static_cast<QFileOpenEvent*>(event);
            mainWindow->importMeshWithLayerManagement(fileEvent->file());

            qDebug("event fileopen %s", qPrintable(fileEvent->file()));
            return true;
        } 
		else 
		{
            // standard event processing
            return QObject::eventFilter(obj, event);
        }
    }
};

#endif
#endif