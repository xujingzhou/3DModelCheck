
#ifndef ML_STD_PAR_DIALOG
#define ML_STD_PAR_DIALOG

#include<QDockWidget>
#include "stdpardialog.h"

class MainWindow;
class MeshlabStdDialog : public QDockWidget
{
    Q_OBJECT

public:
    MeshlabStdDialog(QWidget *p = NULL);
    ~MeshlabStdDialog();

    void clearValues();
    void createFrame();
    void loadFrameContent(MeshDocument *mdPt=0);

    bool showAutoDialog(MeshFilterInterface *mfi, MeshModel *mm, MeshDocument * md, QAction *q, MainWindow *mwi, QWidget *gla=0);
    bool isPreviewable();

public slots:
    void closeClick();

private slots:
    void applyClick();
    void resetValues();
    void toggleHelp();
    void togglePreview();
    void applyDynamic();
    void changeCurrentMesh(int meshInd);

public:
    QFrame *qf;
    StdParFrame *stdParFrame;
    QAction *curAction;
    MeshModelState meshState;
    MeshModelState meshCacheState;
    QCheckBox *previewCB;

    void closeEvent ( QCloseEvent * event );

    uint curmask;
    MeshModel *curModel;
    MeshDocument * curMeshDoc;
    MeshFilterInterface *curmfi;
    MainWindow *curmwi;
    QWidget * curgla;
    RichParameterSet curParSet;
    RichParameterSet prevParSet;
    bool validcache;

};

#endif