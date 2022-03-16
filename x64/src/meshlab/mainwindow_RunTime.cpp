
#include <exception>
#include <QToolBar>
#include <QToolTip>
#include <QStatusBar>
#include <QMenuBar>
#include <QProgressBar>
#include <QDesktopServices>
#include <qdialogbuttonbox.h>
#include <qfileinfo.h>

#include "mainwindow.h"
#include "saveSnapshotDialog.h"
#include "ui_aboutDialog.h"
#include "savemaskexporter.h"
#include "alnParser.h"
#include "filterthread.h"
#include "ml_default_decorators.h"

#include "../common/scriptinterface.h"
#include "../common/meshlabdocumentxml.h"
#include "../common/meshlabdocumentbundler.h"
#include "../common/mlapplication.h"
#include "skin/qskinobject.h"
#include "skin/titlebar_dialog.h"
#include "skin/titlebar_custom.h"
#include "utils.h"

using namespace std;
using namespace vcg;

// Added by Johnny Xu, 2017/3/1
void MainWindow::showContextMenu(bool visible)
{
	if(visible)
		setContextMenuPolicy(Qt::DefaultContextMenu);
	else
		setContextMenuPolicy(Qt::NoContextMenu);
}

void MainWindow::updateRecentFileActions()
{
	// Modified by Johnny Xu, 2017/2/24
    bool activeDoc = true;/* (bool) !mdiarea->subWindowList().empty() && mdiarea->currentSubWindow();*/
    QSettings settings;
    QStringList files = settings.value("recentFileList").toStringList();

	// File validate, Added by Johnny Xu, 2017/8/9
	for (int i = 0; i < files.size(); ++i)
	{
		if (!utils::fileIsExist(files[i]))
		{
			files.removeAt(i);
		}
	}

    int numRecentFiles = qMin(files.size(), (int)MAXRECENTFILES);
    for (int i = 0; i < numRecentFiles; ++i)
    {
        QString text = tr("&%1 %2").arg(i + 1).arg(QFileInfo(files[i]).fileName());
        recentFileActs[i]->setText(text);
        recentFileActs[i]->setData(files[i]);
        recentFileActs[i]->setEnabled(activeDoc);
    }

    for (int j = numRecentFiles; j < MAXRECENTFILES; ++j)
        recentFileActs[j]->setVisible(false);
}

// creates the standard plugin window
void MainWindow::createStdPluginWnd(const QString &titlename)
{
    //checks if a MeshlabStdDialog is already open and closes it
    if (stddialog != 0)
    {
        stddialog->close();
        delete stddialog;
    }

#if defined (_CUSTOM_SKIN_)  

	stddialog = new MeshlabStdDialog();
	stddialog->setAllowedAreas(Qt::NoDockWidgetArea);
	stddialog->setFloating(true);

	// Cusctom titlebar by Johnny Xu, 2017/7/13
	SkinDialog *skin = new SkinDialog(nullptr);
	skin->setWindowTitle(titlename);
	stddialog->setTitleBarWidget(skin->getTitleBar());
	//stddialog->setAttribute(Qt::WA_TranslucentBackground);
	stddialog->show();

	// Added by Johnny Xu, 2017/7/13
	stddialog->setWindowFlags(stddialog->windowFlags() | Qt::WindowStaysOnTopHint);

#else 

	stddialog = new MeshlabStdDialog(this);
	stddialog->setAllowedAreas(Qt::NoDockWidgetArea);
	stddialog->setFloating(true);
	stddialog->hide();

#endif

    connect(GLA(), SIGNAL(glareaClosed()), this, SLOT(updateStdDialog()));
    connect(GLA(), SIGNAL(glareaClosed()), stddialog, SLOT(closeClick()));
}

void MainWindow::createXMLStdPluginWnd()
{
    //checks if a MeshlabStdDialog is already open and closes it
    if (xmldialog != 0)
	{
        xmldialog->close();
        delete xmldialog;
    }

    xmldialog = new MeshLabXMLStdDialog(this);
    connect(xmldialog, SIGNAL(filterParametersEvaluated(const QString&,const QMap<QString,QString>&)), meshDoc()->filterHistory, SLOT(addExecutedXMLFilter(const QString&,const QMap<QString,QString>& )));
    xmldialog->setAllowedAreas (Qt::NoDockWidgetArea);
    xmldialog->setFloating(true);
//    xmldialog->hide();

	// Modified by Johnny Xu, 2017/2/22
    connect(GLA(),SIGNAL(glareaClosed()),this,SLOT(updateXMLStdDialog() /*updateStdDialog()*/));
    connect(GLA(),SIGNAL(glareaClosed()),xmldialog,SLOT(closeClick()));
}

// When we switch the current model (and we change the active window)
// we have to close the stddialog.
// this one is called when user switch current window.
void MainWindow::updateStdDialog()
{
    if(stddialog != 0)
	{
        if(GLA() != 0)
		{
            if(stddialog->curModel != meshDoc()->mm())
			{
                stddialog->curgla = 0; // invalidate the curgla member that is no more valid.
                stddialog->close();
            }
        }
    }
}

// When we switch the current model (and we change the active window)
// we have to close the stddialog.
// this one is called when user switch current window.
void MainWindow::updateXMLStdDialog()
{
    if(xmldialog != 0)
	{
        if(GLA() != 0)
		{
            if(xmldialog->curModel != meshDoc()->mm())
			{
                xmldialog->resetPointers(); // invalidate the curgla member that is no more valid.
                xmldialog->close();
            }
        }
    }
}

void MainWindow::updateCustomSettings()
{
    mwsettings.updateGlobalParameterSet(currentGlobalParams);
    emit dispatchCustomSettings(currentGlobalParams);
}

void MainWindow::enableDocumentSensibleActionsContainer(const bool allowed)
{
    if (mainToolBar != NULL)
        mainToolBar->setEnabled(allowed);

    QAction* filtact = filterMenu->menuAction();
    if (filtact != NULL)
        filtact->setEnabled(allowed);

    if (filterToolBar != NULL)
        filterToolBar->setEnabled(allowed);

    QAction* editact = editMenu->menuAction();
    if (editact != NULL)
        editact->setEnabled(allowed);

    if (editToolBar)
        editToolBar->setEnabled(allowed);

	// Added by Johnny Xu, 2017/2/22
	QAction* renderact = renderMenu->menuAction();
    if (renderact != NULL)
        renderact->setEnabled(allowed);

    if (globrendtoolbar != NULL)
        globrendtoolbar->setEnabled(allowed);

	if (decoratorToolBar != NULL)
        decoratorToolBar->setEnabled(allowed);
}

//menu create is not enabled only in case of not valid/existing meshdocument
void MainWindow::updateSubFiltersMenu( const bool createmenuenabled, const bool validmeshdoc )
{
	filterMenuSelect->setEnabled(validmeshdoc);
    updateMenuItems(filterMenuSelect, validmeshdoc);

    filterMenuClean->setEnabled(validmeshdoc);
    updateMenuItems(filterMenuClean, validmeshdoc);

    filterMenuSmoothing->setEnabled(validmeshdoc);
    updateMenuItems(filterMenuSmoothing, validmeshdoc);

    filterMenuNormal->setEnabled(validmeshdoc);
    updateMenuItems(filterMenuNormal, validmeshdoc);

    filterMenuMeshLayer->setEnabled(validmeshdoc);
    updateMenuItems(filterMenuMeshLayer, validmeshdoc);

    filterMenuSampling->setEnabled(validmeshdoc);
    updateMenuItems(filterMenuSampling, validmeshdoc);
}

void MainWindow::updateMenuItems(QMenu* menu,const bool enabled)
{
    foreach(QAction* act, menu->actions())
        act->setEnabled(enabled);
}

void MainWindow::switchOffDecorator(QAction* decorator)
{
    if (GLA() != NULL)
    {
        int res = GLA()->iCurPerMeshDecoratorList().removeAll(decorator);
        if (res == 0)
            GLA()->iPerDocDecoratorlist.removeAll(decorator);

        updateMenus();

		// Delete decorator property by Johnny Xu, 2017/6/28
//		layerDialog->updateDecoratorParsView();

        GLA()->update();
    }
}

void MainWindow::updateLayerDialog()
{
	if (meshDoc() == NULL)
		return;

    MultiViewer_Container* mvc = currentViewContainer();
    if (mvc == NULL)
        return;

    MLSceneGLSharedDataContext* shared = mvc->sharedDataContext();
    if (shared == NULL)
        return;

    if(GLA())
    {
        MLSceneGLSharedDataContext::PerMeshRenderingDataMap dtf;
        shared->getRenderInfoPerMeshView(GLA()->context(), dtf);

		/*Add to the table the info for the per view global rendering of the Project*/
		MLRenderingData projdt;
		dtf[-1] = projdt;
        layerDialog->updateTable(dtf);
        layerDialog->updateLog(meshDoc()->Log);

		// Delete decorator property by Johnny Xu, 2017/6/28
//        layerDialog->updateDecoratorParsView();

        MLRenderingData dt;
        if (meshDoc()->mm() != NULL)
		{  
			MLSceneGLSharedDataContext::PerMeshRenderingDataMap::iterator it = dtf.find(meshDoc()->mm()->id());
			if (it != dtf.end())
				layerDialog->updateRenderingParametersTab(meshDoc()->mm()->id(), *it);         
        }

		if (globrendtoolbar != NULL)
		{
			shared->getRenderInfoPerMeshView(GLA()->context(), dtf);
			globrendtoolbar->statusConsistencyCheck(dtf);
		}
    }
}

void MainWindow::updateMenus()
{
    bool activeDoc = (bool) !mdiarea->subWindowList().empty() && mdiarea->currentSubWindow();
	// Modified for update action status by Johnny Xu, 2017/2/23
    bool notEmptyActiveDoc = activeDoc && (meshDoc() ? !meshDoc()->meshList.empty() : false);
	
	showContextMenu(activeDoc);

    exportMeshAct->setEnabled(notEmptyActiveDoc);
    exportMeshAsAct->setEnabled(notEmptyActiveDoc);
    reloadMeshAct->setEnabled(notEmptyActiveDoc);
    reloadAllMeshAct->setEnabled(notEmptyActiveDoc);
    saveSnapshotAct->setEnabled(activeDoc);
    updateRecentFileActions();

	// Added by Johnny Xu, 2017/2/23
	globrendtoolbar->setEnabled(notEmptyActiveDoc);

    filterMenu->setEnabled(!filterMenu->actions().isEmpty());
    if (!filterMenu->actions().isEmpty())
        updateSubFiltersMenu(GLA() != NULL, notEmptyActiveDoc);

    editMenu->setEnabled(!editMenu->actions().isEmpty());
    updateMenuItems(editMenu, activeDoc);

    renderMenu->setEnabled(!renderMenu->actions().isEmpty());
    updateMenuItems(renderMenu, activeDoc);

    fullScreenAct->setEnabled(activeDoc);
	showLayerDlgAct->setEnabled(activeDoc);
	showTrackBallAct->setEnabled(activeDoc);
	resetTrackBallAct->setEnabled(activeDoc);
	showInfoPaneAct->setEnabled(activeDoc);

    showToolbarStandardAct->setChecked(mainToolBar->isVisible());
	// Added by Johnny Xu, 2017/2/22
	showToolbarEditAct->setChecked(editToolBar->isVisible());
	showToolbarFilterAct->setChecked(filterToolBar->isVisible());
	showToolbarDecoratorAct->setChecked(decoratorToolBar->isVisible());
	
    if(activeDoc && GLA())
    {
        foreach (QAction *a, PM.editActionList)
        {
            a->setChecked(false);
            a->setEnabled( GLA()->getCurrentEditAction() == NULL );
        }

        suspendEditModeAct->setChecked(GLA()->suspendedEditor);
        suspendEditModeAct->setDisabled(GLA()->getCurrentEditAction() == NULL);

        if(GLA()->getCurrentEditAction())
        {
            GLA()->getCurrentEditAction()->setChecked(! GLA()->suspendedEditor);
            GLA()->getCurrentEditAction()->setEnabled(true);
        }

        showInfoPaneAct->setChecked(GLA()->infoAreaVisible);
        showTrackBallAct->setChecked(GLA()->isTrackBallVisible());

        // Decorator Menu Checking and unChecking
        // First uncheck and disable all the decorators
        foreach (QAction *a, PM.decoratorActionList)
        {
            a->setChecked(false);
            a->setEnabled(true);
        }

        // Check the decorator per Document of the current glarea
        foreach (QAction *a, GLA()->iPerDocDecoratorlist)
        { 
			a ->setChecked(true);
		}

        // Then check the decorator enabled for the current mesh.
        if(GLA()->mm())
		{
            foreach (QAction *a, GLA()->iCurPerMeshDecoratorList())
            a ->setChecked(true);
		}
    } // if active
    else
    {
        foreach (QAction *a, PM.editActionList)
        {
            a->setEnabled(false);
        }

        foreach (QAction *a, PM.decoratorActionList)
            a->setEnabled(false);
    }

	GLArea* tmp = GLA();
    if(tmp != NULL)
    {
        showLayerDlgAct->setChecked(layerDialog->isVisible());
    }
    else
    {
        foreach (QAction *a, PM.decoratorActionList)
        {
            a->setChecked(false);
            a->setEnabled(false);
        }

        layerDialog->setVisible(false);
    }
}

//set the split/unsplit menu that appears right clicking on a splitter's handle
void MainWindow::setHandleMenu(QPoint point, Qt::Orientation orientation, QSplitter *origin)
{
    // Remove useless code by Johnny Xu, 2017/2/22
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    //qDebug("dragEnterEvent: %s",event->format());
    event->accept();
}

void MainWindow::dropEvent( QDropEvent * event )
{
    const QMimeData * data = event->mimeData();
    if (data->hasUrls())
    {
        QList< QUrl > url_list = data->urls();
        bool layervis = false;
        if (layerDialog != NULL)
        {
            layervis = layerDialog->isVisible();
            showLayerDlg(false);
        }

        for (int i = 0, size = url_list.size(); i < size; i++)
        {
            QString path = url_list.at(i).toLocalFile();
            if( (event->keyboardModifiers () == Qt::ControlModifier ) || ( QApplication::keyboardModifiers () == Qt::ControlModifier ))
            {
                this->newProject();
            }

			// Modified by Johnny Xu, 2017/2/22
            importMesh(path);
        }

        showLayerDlg(layervis || meshDoc()->meshList.size() > 0);
    }
}

void MainWindow::endEdit()
{
	MultiViewer_Container* mvc = currentViewContainer();
	if ((meshDoc() == NULL) || (GLA() == NULL) || (mvc == NULL))
		return;

	for (int ii = 0; ii < meshDoc()->meshList.size(); ++ii)
	{
		MeshModel* mm = meshDoc()->meshList[ii];
		if (mm != NULL)
			addRenderingDataIfNewlyGeneratedMesh(mm->id());
	}
	meshDoc()->meshDocStateData().clear();

    GLA()->endEdit();
	updateLayerDialog();
}

// Receives the action that wants to show a tooltip and display it
// on screen at the current mouse position.
// TODO: have the tooltip always display with fixed width at the right
//       hand side of the menu entry (not invasive)
void MainWindow::showTooltip(QAction* q)
{
    QString tip = q->toolTip();
    QToolTip::showText(QCursor::pos(), tip);
}

void MainWindow::startFilter()
{
    if(currentViewContainer() == NULL) 
		return;

    if(GLA() == NULL)
		return;

    // In order to avoid that a filter changes something assumed by the current editing tool,
    // before actually starting the filter we close the current editing tool (if any).
	if (GLA()->getCurrentEditAction() != NULL)
		endEdit();

    updateMenus();

    QStringList missingPreconditions;
    QAction *action = qobject_cast<QAction *>(sender());
    if (action == NULL)
        throw MLException("Invalid filter action value.");

    MeshFilterInterface *iFilter = qobject_cast<MeshFilterInterface *>(action->parent());
    if (meshDoc() == NULL)
        return;

    //OLD FILTER PHILOSOPHY
    if (iFilter != NULL)
    {
        //if(iFilter->getClass(action) == MeshFilterInterface::MeshCreation)
        //{
        //	qDebug("MeshCreation");
        //	GLA()->meshDoc->addNewMesh("",iFilter->filterName(action) );
        //}
        //else
        if (!iFilter->isFilterApplicable(action,(*meshDoc()->mm()), missingPreconditions))
        {
            QString enstr = missingPreconditions.join(",");

			QString strBox = QString("<font color=red>'" + iFilter->filterName(action) + "'</font>" + QTextCodec::codecForName("GBK")->toUnicode("没有被应用。<br>")
				+ QTextCodec::codecForName("GBK")->toUnicode("当前网格没有<i>") + enstr + QTextCodec::codecForName("GBK")->toUnicode("</i>。"));
			QMessageBox *box = new QMessageBox(QMessageBox::Warning, QString::fromLocal8Bit("启用前置条件失败"), strBox);
			box->setStandardButtons(QMessageBox::Ok);
			box->setButtonText(QMessageBox::Ok, QString::fromLocal8Bit("确定"));

#if defined (_CUSTOM_SKIN_)  

			// Cusctom titlebar by Johnny Xu, 2017/7/13
			QSkinObject *skin = new QSkinObject(box);
			skin->startSkinning();
#else  

			// Normal
#endif

			box->exec();
            return;
        }

        if(currentViewContainer())
        {
            iFilter->setLog(currentViewContainer()->LogPtr());
            currentViewContainer()->LogPtr()->SetBookmark();
        }

        // just to be sure...
        createStdPluginWnd(iFilter->filterName(action));

        if (xmldialog != NULL)
        {
            xmldialog->close();
            delete xmldialog;
            xmldialog = NULL;
        }

        // (2) Ask for filter parameters and eventally directly invoke the filter
        // showAutoDialog return true if a dialog have been created (and therefore the execution is demanded to the apply event)
        // if no dialog is created the filter must be executed immediately
        if(! stddialog->showAutoDialog(iFilter, meshDoc()->mm(), (meshDoc()), action, this, GLA()) )
        {
            RichParameterSet dummyParSet;
            executeFilter(action, dummyParSet, false);
        }
    }
    else // NEW XML PHILOSOPHY
    {
        try
        {
            MeshLabFilterInterface *iXMLFilter = qobject_cast<MeshLabFilterInterface *>(action->parent());
            QString fname = action->text();
            MeshLabXMLFilterContainer& filt  = PM.stringXMLFilterMap[fname];

            if ((iXMLFilter == NULL) || (filt.xmlInfo == NULL) || (filt.act == NULL))
                throw MLException(QTextCodec::codecForName("GBK")->toUnicode("一个无效的MLXMLPluginInfo处理被检测到!") 
				  /* "An invalid MLXMLPluginInfo handle has been detected in startFilter function."*/);

            QString filterClasses = filt.xmlInfo->filterAttribute(fname,MLXMLElNames::filterClass);
            QStringList filterClassesList = filterClasses.split(QRegExp("\\W+"), QString::SkipEmptyParts);
            if(filterClassesList.contains("MeshCreation"))
            {
                qDebug("MeshCreation");
                meshDoc()->addNewMesh("","untitled.ply");
            }
            else
            {
                QString preCond = filt.xmlInfo->filterAttribute(fname,MLXMLElNames::filterPreCond);
                QStringList preCondList = preCond.split(QRegExp("\\W+"), QString::SkipEmptyParts);
                int preCondMask = MeshLabFilterInterface::convertStringListToMeshElementEnum(preCondList);
                if (!MeshLabFilterInterface::arePreCondsValid(preCondMask,(*meshDoc()->mm()),missingPreconditions))
                {
                    QString enstr = missingPreconditions.join(",");

					QString strBox = QString("<font color=red>'" + iFilter->filterName(action) + "'</font>" + QTextCodec::codecForName("GBK")->toUnicode("没有被应用。<br>")
						+ QTextCodec::codecForName("GBK")->toUnicode("当前网格没有<i>") + enstr + QTextCodec::codecForName("GBK")->toUnicode("</i>。"));
					QMessageBox *box = new QMessageBox(QMessageBox::Warning, QString::fromLocal8Bit("启用前置条件失败"), strBox);
					box->setStandardButtons(QMessageBox::Ok);
					box->setButtonText(QMessageBox::Ok, QString::fromLocal8Bit("确定"));

#if defined (_CUSTOM_SKIN_)  

					// Cusctom titlebar by Johnny Xu, 2017/7/13
					QSkinObject *skin = new QSkinObject(box);
					skin->startSkinning();
#else  

					// Normal
#endif

					box->exec();

                    return;
                }
            }

            MLXMLPluginInfo::XMLMapList params = filt.xmlInfo->filterParametersExtendedInfo(fname);
            try
            {
                if(currentViewContainer())
                {
                    if (iXMLFilter)
                        iXMLFilter->setLog(currentViewContainer()->LogPtr());
                    currentViewContainer()->LogPtr()->SetBookmark();
                }

                // just to be sure...
                createXMLStdPluginWnd();

                if (stddialog != NULL)
                {
                    stddialog->close();
                    delete stddialog;
                    stddialog = NULL;
                }

                // (2) Ask for filter parameters and eventally directly invoke the filter
                // showAutoDialog return true if a dialog have been created (and therefore the execution is demanded to the apply event)
                // if no dialog is created the filter must be executed immediatel
                if(!xmldialog->showAutoDialog(filt,PM,meshDoc(), this, GLA()))
                {
                    /*Mock Parameters (there are no ones in the filter indeed) for the filter history.The filters with parameters are inserted by the applyClick of the XMLStdParDialog.
                    That is the only place where I can easily evaluate the parameter values without writing a long, boring and horrible if on the filter type for the correct evaluation of the expressions contained inside the XMLWidgets*/
                    QMap<QString,QString> mock;
                    meshDoc()->filterHistory->addExecutedXMLFilter(fname, mock);

                    executeFilter(&filt, mock, false);
                    meshDoc()->Log.Logf(GLLogStream::SYSTEM, "超出范围\n");
                }
                //delete env;
            }
            catch (MLException& e)
            {
                meshDoc()->Log.Logf(GLLogStream::SYSTEM,e.what());
            }
        }
        catch(ParsingException e)
        {
            meshDoc()->Log.Logf(GLLogStream::SYSTEM,e.what());
        }
    }
}

void MainWindow::updateSharedContextDataAfterFilterExecution(int postcondmask, int fclasses, bool& newmeshcreated)
{
    MultiViewer_Container* mvc = currentViewContainer();
    if ((meshDoc() != NULL) && (mvc != NULL))
    {
        if (GLA() == NULL)
            return;

        MLSceneGLSharedDataContext* shared = mvc->sharedDataContext();
        if (shared != NULL)
        {
            for(MeshModel* mm = meshDoc()->nextMesh();mm != NULL;mm = meshDoc()->nextMesh(mm))
            {
                if (mm == NULL)
                    continue;

                bool currentmeshnewlycreated = false;
                //Just to be sure that the filter author didn't forget to add changing tags to the postCondition field
                if ((mm->hasDataMask(MeshModel::MM_FACECOLOR)) && (fclasses & MeshFilterInterface::FaceColoring ))
                    postcondmask = postcondmask | MeshModel::MM_FACECOLOR;

                if ((mm->hasDataMask(MeshModel::MM_VERTCOLOR)) && (fclasses & MeshFilterInterface::VertexColoring ))
                    postcondmask = postcondmask | MeshModel::MM_VERTCOLOR;

                if ((mm->hasDataMask(MeshModel::MM_COLOR)) && (fclasses & MeshFilterInterface::MeshColoring ))
                    postcondmask = postcondmask | MeshModel::MM_COLOR;

                if ((mm->hasDataMask(MeshModel::MM_FACEQUALITY)) && (fclasses & MeshFilterInterface::Quality ))
                    postcondmask = postcondmask | MeshModel::MM_FACEQUALITY;

                if ((mm->hasDataMask(MeshModel::MM_VERTQUALITY)) && (fclasses & MeshFilterInterface::Quality ))
                    postcondmask = postcondmask | MeshModel::MM_VERTQUALITY;

                MLRenderingData dttoberendered;
                QMap<int,MeshModelStateData>::Iterator existit = meshDoc()->meshDocStateData().find(mm->id());
				if (existit != meshDoc()->meshDocStateData().end())
                {

                    shared->getRenderInfoPerMeshView(mm->id(),GLA()->context(),dttoberendered);
                    int updatemask = MeshModel::MM_NONE;
					bool connectivitychanged = false;
                    if ((mm->cm.VN() != existit->_nvert) || (mm->cm.FN() != existit->_nface) ||
                        bool(postcondmask & MeshModel::MM_UNKNOWN) || bool(postcondmask & MeshModel::MM_VERTNUMBER) || 
                        bool(postcondmask & MeshModel::MM_FACENUMBER) || bool(postcondmask & MeshModel::MM_FACEVERT) ||
                        bool(postcondmask & MeshModel::MM_VERTFACETOPO) || bool(postcondmask & MeshModel::MM_FACEFACETOPO))
                    {    
                        updatemask = MeshModel::MM_ALL;
                        connectivitychanged = true;
                    }
                    else
                    {
                        //masks differences bitwise operator (^) -> remove the attributes that didn't apparently change + the ones that for sure changed according to the postCondition function
                        //this operation has been introduced in order to minimize problems with filters that didn't declared properly the postCondition mask 
                        updatemask = (existit->_mask ^ mm->dataMask()) | postcondmask;
                        connectivitychanged = false;
                    }

                    MLRenderingData::RendAtts dttoupdate;
                    //1) we convert the meshmodel updating mask to a RendAtts structure 
                    MLPoliciesStandAloneFunctions::fromMeshModelMaskToMLRenderingAtts(updatemask, dttoupdate);
                    //2) The correspondent bos to the updated rendering attributes are set to invalid 
                    shared->meshAttributesUpdated(mm->id(), connectivitychanged, dttoupdate);

                    //3) we took the current rendering modality for the mesh in the active gla
                    MLRenderingData curr;
                    shared->getRenderInfoPerMeshView(mm->id(), GLA()->context(), curr);

                    //4) we add to the current rendering modality in the current GLArea just the minimum attributes having been updated
                    //   WARNING!!!! There are priority policies
                    //               ex1) suppose that the current rendering modality is PR_POINTS and ATT_VERTPOSITION, ATT_VERTNORMAL,ATT_VERTCOLOR
                    //               if i updated, for instance, just the ATT_FACECOLOR, we switch off in the active GLArea the per ATT_VERTCOLOR attribute
                    //               and turn on the ATT_FACECOLOR
                    //               ex2) suppose that the current rendering modality is PR_POINTS and ATT_VERTPOSITION, ATT_VERTNORMAL,ATT_VERTCOLOR
                    //               if i updated, for instance, both the ATT_FACECOLOR and the ATT_VERTCOLOR, we continue to render the updated value of the ATT_VERTCOLOR
                    //               ex3) suppose that in all the GLAreas the current rendering modality is PR_POINTS and we run a surface reconstruction filter
                    //               in the current GLA() we switch from the PR_POINTS to PR_SOLID primitive rendering modality. In the other GLArea we maintain the per points visualization
                    for(MLRenderingData::PRIMITIVE_MODALITY pm = MLRenderingData::PRIMITIVE_MODALITY(0);pm < MLRenderingData::PR_ARITY;pm = MLRenderingData::next(pm))
                    {
                        bool wasprimitivemodalitymeaningful = MLPoliciesStandAloneFunctions::isPrimitiveModalityCompatibleWithMeshInfo((existit->_nvert > 0),(existit->_nface > 0),(existit->_nedge > 0),existit->_mask,pm);
                        bool isprimitivemodalitymeaningful = MLPoliciesStandAloneFunctions::isPrimitiveModalityCompatibleWithMesh(mm,pm);
                        bool isworthtobevisualized = MLPoliciesStandAloneFunctions::isPrimitiveModalityWorthToBeActivated(pm,curr.isPrimitiveActive(pm),wasprimitivemodalitymeaningful,isprimitivemodalitymeaningful);


                        MLRenderingData::RendAtts rd;
						curr.get(pm, rd);
						MLPoliciesStandAloneFunctions::updatedRendAttsAccordingToPriorities(pm, dttoupdate, rd, rd);
						rd[MLRenderingData::ATT_NAMES::ATT_VERTPOSITION] = isworthtobevisualized;
                        MLPoliciesStandAloneFunctions::filterUselessUdpateAccordingToMeshMask(mm,rd);
                        curr.set(pm,rd);
                    }

					MLPerViewGLOptions opts;
					curr.get(opts);
					if (fclasses & MeshFilterInterface::MeshColoring)
					{
						bool hasmeshcolor = mm->hasDataMask(MeshModel::MM_COLOR);
						opts._perpoint_mesh_color_enabled = hasmeshcolor;
						opts._perwire_mesh_color_enabled = hasmeshcolor;
						opts._persolid_mesh_color_enabled = hasmeshcolor;

						for (MLRenderingData::PRIMITIVE_MODALITY pm = MLRenderingData::PRIMITIVE_MODALITY(0); pm < MLRenderingData::PR_ARITY; pm = MLRenderingData::next(pm))
						{
							MLRenderingData::RendAtts atts;
							curr.get(pm, atts);
							atts[MLRenderingData::ATT_NAMES::ATT_VERTCOLOR] = false;
							atts[MLRenderingData::ATT_NAMES::ATT_FACECOLOR] = false;
							curr.set(pm, atts);
						}
						curr.set(opts);
					}

					MLPoliciesStandAloneFunctions::setPerViewGLOptionsAccordindToWireModality(mm, curr);
                    MLPoliciesStandAloneFunctions::setPerViewGLOptionsPriorities(curr);
					if (mm == meshDoc()->mm())
					{
						/*HORRIBLE TRICK IN ORDER TO HAVE VALID ACTIONS ASSOCIATED WITH THE CURRENT WIRE RENDERING MODALITY*/
						MLRenderingFauxEdgeWireAction* fauxaction = new MLRenderingFauxEdgeWireAction(meshDoc()->mm()->id(), NULL);
						fauxaction->setChecked(curr.isPrimitiveActive(MLRenderingData::PR_WIREFRAME_EDGES));
						layerDialog->_tabw->switchWireModality(fauxaction);
						delete fauxaction;
						/****************************************************************************************************/
					}

                    shared->setRenderingDataPerMeshView(mm->id(),GLA()->context(),curr);
                    currentmeshnewlycreated = false;
                }
                else 
                {
                    //A new mesh has been created by the filter. I have to add it in the shared context data structure
                    newmeshcreated = true;
                    currentmeshnewlycreated = true;
                    MLPoliciesStandAloneFunctions::suggestedDefaultPerViewRenderingData(mm,dttoberendered,mwsettings.minpolygonpersmoothrendering);
					if (mm == meshDoc()->mm())
					{
						/*HORRIBLE TRICK IN ORDER TO HAVE VALID ACTIONS ASSOCIATED WITH THE CURRENT WIRE RENDERING MODALITY*/
						MLRenderingFauxEdgeWireAction* fauxaction = new MLRenderingFauxEdgeWireAction(meshDoc()->mm()->id(), NULL);
						fauxaction->setChecked(dttoberendered.isPrimitiveActive(MLRenderingData::PR_WIREFRAME_EDGES));
						layerDialog->_tabw->switchWireModality(fauxaction);
						delete fauxaction;
						/****************************************************************************************************/
					}

					foreach(GLArea* gla,mvc->viewerList)
                    {
                        if (gla != NULL)
                            shared->setRenderingDataPerMeshView(mm->id(), gla->context(), dttoberendered);
                    }
				}

				shared->manageBuffers(mm->id());
            }

			updateLayerDialog();
        }
    }
}

/*
callback function that actually start the chosen filter.
it is called once the parameters have been filled.
It can be called
from the automatic dialog
from the user defined dialog
*/
void MainWindow::executeFilter(QAction *action, RichParameterSet &params, bool isPreview)
{
    MeshFilterInterface *iFilter = qobject_cast<MeshFilterInterface *>(action->parent());
    qb->show();
    iFilter->setLog(&meshDoc()->Log);

    // Ask for filter requirements (eg a filter can need topology, border flags etc)
    // and statisfy them
    qApp->setOverrideCursor(QCursor(Qt::WaitCursor));
    MainWindow::globalStatusBar()->showMessage(QTextCodec::codecForName("GBK")->toUnicode("正在启用过滤...") /*"Starting Filter..."*/, 5000);

    int req = iFilter->getRequirements(action);
    if (!meshDoc()->meshList.isEmpty())
        meshDoc()->mm()->updateDataMask(req);
    qApp->restoreOverrideCursor();

    // (3) save the current filter and its parameters in the history
    if(!isPreview)
    {
        if (meshDoc()->filterHistory != NULL)
        {
            OldFilterNameParameterValuesPair* oldpair = new OldFilterNameParameterValuesPair();
            oldpair->pair = qMakePair(action->text(),params);
            meshDoc()->filterHistory->filtparlist.append(oldpair);
        }
        meshDoc()->Log.ClearBookmark();
    }
    else
        meshDoc()->Log.BackToBookmark();

    // (4) Apply the Filter
    bool ret;
    qApp->setOverrideCursor(QCursor(Qt::WaitCursor));
    QTime tt; tt.start();
    meshDoc()->setBusy(true);
    RichParameterSet mergedenvironment(params);
    mergedenvironment.join(currentGlobalParams);

    MLSceneGLSharedDataContext* shar = NULL;
    QGLWidget* filterWidget = NULL;
    if (currentViewContainer() != NULL)
    {
        shar = currentViewContainer()->sharedDataContext();
        //GLA() is only the parent
        filterWidget = new QGLWidget(NULL,shar);
        QGLFormat defForm = QGLFormat::defaultFormat();
        iFilter->glContext = new MLPluginGLContext(defForm,filterWidget->context()->device(),*shar);
        iFilter->glContext->create(filterWidget->context());

        MLRenderingData dt;
        MLRenderingData::RendAtts atts;
        atts[MLRenderingData::ATT_NAMES::ATT_VERTPOSITION] = true;
        atts[MLRenderingData::ATT_NAMES::ATT_VERTNORMAL] = true;

        if (iFilter->filterArity(action) == MeshFilterInterface::SINGLE_MESH)
        {
            MLRenderingData::PRIMITIVE_MODALITY pm = MLPoliciesStandAloneFunctions::bestPrimitiveModalityAccordingToMesh(meshDoc()->mm());
            if ((pm != MLRenderingData::PR_ARITY) && (meshDoc()->mm() != NULL))
            {
                dt.set(pm,atts);
                iFilter->glContext->initPerViewRenderingData(meshDoc()->mm()->id(),dt);
            }
        }
        else
        {
            for(int ii = 0;ii < meshDoc()->meshList.size();++ii)
            {
                MeshModel* mm = meshDoc()->meshList[ii];
                MLRenderingData::PRIMITIVE_MODALITY pm = MLPoliciesStandAloneFunctions::bestPrimitiveModalityAccordingToMesh(mm);
                if ((pm != MLRenderingData::PR_ARITY) && (mm != NULL))
                {
                    dt.set(pm,atts);
                    iFilter->glContext->initPerViewRenderingData(mm->id(),dt);
                }
            }
        }
    }

    bool newmeshcreated = false;
    try
    {
        meshDoc()->meshDocStateData().clear();
		meshDoc()->meshDocStateData().create(*meshDoc());
        ret = iFilter->applyFilter(action, *(meshDoc()), mergedenvironment, QCallBack);

		for (MeshModel* mm = meshDoc()->nextMesh(); mm != NULL; mm = meshDoc()->nextMesh(mm))
			vcg::tri::Allocator<CMeshO>::CompactEveryVector(mm->cm);

		if (shar != NULL)
		{
			shar->removeView(iFilter->glContext);
			delete filterWidget;
		}

        meshDoc()->setBusy(false);
        qApp->restoreOverrideCursor();

        // (5) Apply post filter actions (e.g. recompute non updated stuff if needed)

        if(ret)
        {
            meshDoc()->Log.Logf(GLLogStream::SYSTEM, "应用了过滤: %s，花费了%i秒", qPrintable(action->text()), tt.elapsed());

            if (meshDoc()->mm() != NULL)
                meshDoc()->mm()->meshModified() = true;

             MainWindow::globalStatusBar()->showMessage(QTextCodec::codecForName("GBK")->toUnicode("过滤成功完成") /*"Filter successfully completed..."*/, 2000);

            if(GLA())
            {
                GLA()->setLastAppliedFilter(action);
            }
        }
        else // filter has failed. show the message error.
        {
//			QMessageBox::warning(this, QTextCodec::codecForName("GBK")->toUnicode("过滤失败") /*tr("Filter Failure")*/, QTextCodec::codecForName("GBK")->toUnicode("失败的过滤") + QString("<font color=red>: '%1'</font><br><br>").arg(action->text())+iFilter->errorMsg()); // text
			QMessageBox *box = new QMessageBox(QMessageBox::Warning, QString::fromLocal8Bit("过滤失败"), QString::fromLocal8Bit("失败的过滤") + QString("<font color=red>: '%1'</font><br><br>").arg(action->text()) + iFilter->errorMsg());
			box->setStandardButtons(QMessageBox::Ok);
			box->setButtonText(QMessageBox::Ok, QString::fromLocal8Bit("确定"));

#if defined (_CUSTOM_SKIN_)  

			// Cusctom titlebar by Johnny Xu, 2017/7/13
			QSkinObject *skin = new QSkinObject(box);
			skin->startSkinning();
#else  

			// Normal
#endif

			box->exec();

            meshDoc()->Log.Logf(GLLogStream::SYSTEM, "过滤失败: %s", qPrintable(iFilter->errorMsg()));
            MainWindow::globalStatusBar()->showMessage(QTextCodec::codecForName("GBK")->toUnicode("过滤启用失败") /*"Filter failed..."*/, 2000);
        }

        MeshFilterInterface::FILTER_ARITY arity = iFilter->filterArity(action);
        QList<MeshModel*> tmp;
        switch(arity)
        {
			case (MeshFilterInterface::SINGLE_MESH):
				{
					tmp.push_back(meshDoc()->mm());
					break;
				}
			case (MeshFilterInterface::FIXED):
				{
					for(int ii = 0;ii < mergedenvironment.paramList.size();++ii)
					{
						if (mergedenvironment.paramList[ii]->val->isMesh())
						{
							MeshModel* mm = mergedenvironment.paramList[ii]->val->getMesh();
							if (mm != NULL)
								tmp.push_back(mm);
						}
					}
					break;
				}
			case (MeshFilterInterface::VARIABLE):
				{
					for(MeshModel* mm = meshDoc()->nextMesh();mm != NULL;mm=meshDoc()->nextMesh(mm))
					{
						if (mm->isVisible())
							tmp.push_back(mm);
					}
					break;
				}
			default:
				break;
        }

        if(iFilter->getClass(action) & MeshFilterInterface::MeshCreation )
            GLA()->resetTrackBall();

        for(int jj = 0; jj < tmp.size(); ++jj)
        {
            MeshModel* mm = tmp[jj];
            if (mm != NULL)
            {
                // at the end for filters that change the color, or selection set the appropriate rendering mode
                if(iFilter->getClass(action) & MeshFilterInterface::FaceColoring ) 
                    mm->updateDataMask(MeshModel::MM_FACECOLOR);

                if(iFilter->getClass(action) & MeshFilterInterface::VertexColoring )
                    mm->updateDataMask(MeshModel::MM_VERTCOLOR);

                if((iFilter->getClass(action) & MeshFilterInterface::MeshColoring ) || (iFilter->postCondition(action) & MeshModel::MM_COLOR))
                    mm->updateDataMask(MeshModel::MM_COLOR);

                if(iFilter->postCondition(action) & MeshModel::MM_CAMERA)
                    mm->updateDataMask(MeshModel::MM_CAMERA);

                if(iFilter->getClass(action) & MeshFilterInterface::Texture )
                    updateTexture(mm->id());
            }
        }
        
        int fclasses =	iFilter->getClass(action);
        //MLSceneGLSharedDataContext* sharedcont = GLA()->getSceneGLSharedContext();
        int postCondMask = iFilter->postCondition(action);
        updateSharedContextDataAfterFilterExecution(postCondMask,fclasses,newmeshcreated);
        meshDoc()->meshDocStateData().clear();
    }
    catch (std::bad_alloc& bdall)
    {
        meshDoc()->setBusy(false);
        qApp->restoreOverrideCursor();

		QMessageBox *box = new QMessageBox(QMessageBox::Warning, QString::fromLocal8Bit("过滤失败"), QTextCodec::codecForName("GBK")->toUnicode("操作系统未分配足够的内存。"));
		box->setStandardButtons(QMessageBox::Ok);
		box->setButtonText(QMessageBox::Ok, QString::fromLocal8Bit("确定"));

#if defined (_CUSTOM_SKIN_)  

		// Cusctom titlebar by Johnny Xu, 2017/7/13
		QSkinObject *skin = new QSkinObject(box);
		skin->startSkinning();
#else  

		// Normal
#endif

		box->exec();

		MainWindow::globalStatusBar()->showMessage(QTextCodec::codecForName("GBK")->toUnicode("过滤启用失败."), 2000);
    }

    qb->reset();
    layerDialog->setVisible(layerDialog->isVisible() || ((newmeshcreated) && (meshDoc()->size() > 0)));
    updateLayerDialog();
    updateMenus();

    MultiViewer_Container* mvc = currentViewContainer();
    if(mvc)
        mvc->updateAllViewer();
}

void MainWindow::initDocumentMeshRenderState(MeshLabXMLFilterContainer* /*mfc*/)
{
}

void MainWindow::initDocumentRasterRenderState(MeshLabXMLFilterContainer* /*mfc*/)
{
}

void MainWindow::executeFilter(MeshLabXMLFilterContainer* mfc,const QMap<QString,QString>& parexpval , bool  ispreview)
{
    if (mfc == NULL)
        return;

    MeshLabFilterInterface *iFilter = mfc->filterInterface;
    bool jscode = (mfc->xmlInfo->filterScriptCode(mfc->act->text()) != "");
    bool filtercpp = (iFilter != NULL) && (!jscode);

    if ((!filtercpp) && (!jscode))
        throw MLException(QTextCodec::codecForName("GBK")->toUnicode("一个非C++或非JavaScript的过滤被打开。") 
		  /* "A not-C++ and not-JaveScript filter has been invoked.There is something really wrong in MeshLab."*/);

    QString fname = mfc->act->text();
    QString postCond = mfc->xmlInfo->filterAttribute(fname,MLXMLElNames::filterPostCond);
    QStringList postCondList = postCond.split(QRegExp("\\W+"), QString::SkipEmptyParts);
    int postCondMask = MeshLabFilterInterface::convertStringListToMeshElementEnum(postCondList);

    if(!ispreview)
        meshDoc()->Log.ClearBookmark();
    else
        meshDoc()->Log.BackToBookmark();

    qb->show();
    if (filtercpp)
        iFilter->setLog(&meshDoc()->Log);

    //// Ask for filter requirements (eg a filter can need topology, border flags etc)
    //// and statisfy them
    qApp->setOverrideCursor(QCursor(Qt::WaitCursor));
	MainWindow::globalStatusBar()->showMessage(QTextCodec::codecForName("GBK")->toUnicode("正在启用过滤...") /*"Starting Filter..."*/, 5000);
    //int req=iFilter->getRequirements(action);
    meshDoc()->mm()->updateDataMask(postCondMask);
    qApp->restoreOverrideCursor();

    bool ret = true;
    qApp->setOverrideCursor(QCursor(Qt::WaitCursor));
    bool isinter = (mfc->xmlInfo->filterAttribute(fname,MLXMLElNames::filterIsInterruptible) == "true");
	if(!isinter) 
		meshDoc()->setBusy(true);

    //RichParameterSet MergedEnvironment(params);
    //MergedEnvironment.join(currentGlobalParams);

    ////GLA() is only the parent
    xmlfiltertimer.restart();

    try
    {
        MLXMLPluginInfo::XMLMapList ml = mfc->xmlInfo->filterParametersExtendedInfo(fname);
        QString funcall = "Plugins." + mfc->xmlInfo->pluginAttribute(MLXMLElNames::pluginScriptName) + "." + mfc->xmlInfo->filterAttribute(fname,MLXMLElNames::filterScriptFunctName) + "(";
        if (mfc->xmlInfo->filterAttribute(fname,MLXMLElNames::filterArity) == MLXMLElNames::singleMeshArity && !jscode)
        {
            funcall = funcall + QString::number(meshDoc()->mm()->id());
            if (ml.size() != 0)
                funcall = funcall + ",";
        }

        for(int ii = 0;ii < ml.size();++ii)
        {
            funcall = funcall + parexpval[ml[ii][MLXMLElNames::paramName]];
            if (ii != ml.size() - 1)
                funcall = funcall + ",";
        }
        funcall = funcall + ");";
        
        meshDoc()->xmlhistory << funcall;
		meshDoc()->meshDocStateData().clear();
		meshDoc()->meshDocStateData().create(*meshDoc());

        if (filtercpp)
        {
            enableDocumentSensibleActionsContainer(false);
            FilterThread* ft = new FilterThread(fname,parexpval,PM,*(meshDoc()),this);

            connect(ft,SIGNAL(finished()),this,SLOT(postFilterExecution()));
            connect(ft,SIGNAL(threadCB(const int, const QString&)),this,SLOT(updateProgressBar(const int,const QString&)));
            connect(xmldialog,SIGNAL(filterInterrupt(const bool)),PM.stringXMLFilterMap[fname].filterInterface,SLOT(setInterrupt(const bool)));
            
            ft->start();
        }
        else
        {
            QTime t;
            t.start();
            Env env;
            env.loadMLScriptEnv(*meshDoc(),PM,currentGlobalPars());
            QScriptValue result = env.evaluate(funcall);
            scriptCodeExecuted(result,t.elapsed(),"");
            postFilterExecution();

        }
    }
    catch(MLException& e)
    {
        meshDoc()->Log.Logf(GLLogStream::SYSTEM,e.what());
        ret = false;
    }
}

void MainWindow::postFilterExecution()
{
    emit filterExecuted();

    qApp->restoreOverrideCursor();
    qb->reset();

    enableDocumentSensibleActionsContainer(true);

    FilterThread* obj = qobject_cast<FilterThread*>(QObject::sender());
    if (obj == NULL)
        return;

    QMap<QString,MeshLabXMLFilterContainer>::const_iterator mfc = PM.stringXMLFilterMap.find(obj->filterName());
    if (mfc == PM.stringXMLFilterMap.constEnd())
        return;

    QString fname = mfc->act->text();
    // at the end for filters that change the color, or selection set the appropriate rendering mode
    QString filterClasses = mfc->xmlInfo->filterAttribute(fname,MLXMLElNames::filterClass);
    QStringList filterClassesList = filterClasses.split(QRegExp("\\W+"), QString::SkipEmptyParts);
    int fclasses =	MeshLabFilterInterface::convertStringListToCategoryEnum(filterClassesList);
    bool newmeshcreated = false;
    if (mfc->filterInterface != NULL)
    {
        mfc->filterInterface->setInterrupt(false);
        QString postCond = mfc->xmlInfo->filterAttribute(fname,MLXMLElNames::filterPostCond);
        QStringList postCondList = postCond.split(QRegExp("\\W+"), QString::SkipEmptyParts);
        int postCondMask = MeshLabFilterInterface::convertStringListToMeshElementEnum(postCondList);
        updateSharedContextDataAfterFilterExecution(postCondMask,fclasses,newmeshcreated);
        MultiViewer_Container* mvc = currentViewContainer();
        if(mvc)
            mvc->updateAllViewer();
    }

    meshDoc()->meshDocStateData().clear();
    meshDoc()->setBusy(false);

    //// (5) Apply post filter actions (e.g. recompute non updated stuff if needed)
    if(obj->succeed())
    {
		meshDoc()->Log.Logf(GLLogStream::SYSTEM, "应用了过滤: %s - %i秒\n", qPrintable(fname), xmlfiltertimer.elapsed());
        MainWindow::globalStatusBar()->showMessage(QTextCodec::codecForName("GBK")->toUnicode("过滤成功完成") /*"Filter successfully completed..."*/, 2000);

        if(GLA())
        {
            GLA()->setLastAppliedFilter(mfc->act);
        }
    }
    else // filter has failed. show the message error.
    {
		MeshLabFilterInterface *iFilter = mfc->filterInterface;

		QString strBox = QTextCodec::codecForName("GBK")->toUnicode("失败的过滤") + QString("<font color=red>: '%1'</font><br><br>").arg(fname) + iFilter->errorMsg();
		QMessageBox *box = new QMessageBox(QMessageBox::Warning, QString::fromLocal8Bit("过滤失败"), strBox);
		box->setStandardButtons(QMessageBox::Ok);
		box->setButtonText(QMessageBox::Ok, QString::fromLocal8Bit("确定"));

#if defined (_CUSTOM_SKIN_)  

		// Cusctom titlebar by Johnny Xu, 2017/7/13
		QSkinObject *skin = new QSkinObject(box);
		skin->startSkinning();
#else  

		// Normal
#endif

		box->exec();

		meshDoc()->Log.Logf(GLLogStream::SYSTEM, "过滤失败: %s", qPrintable(iFilter->errorMsg()));
        MainWindow::globalStatusBar()->showMessage(QTextCodec::codecForName("GBK")->toUnicode("过滤启用失败") /*"Filter failed..."*/, 2000);
    }

    if(fclasses & MeshFilterInterface::FaceColoring ) 
	{
//GLA()->setColorMode(vcg::GLW::CMPerFace);
        meshDoc()->mm()->updateDataMask(MeshModel::MM_FACECOLOR);
    }

    if(fclasses & MeshFilterInterface::VertexColoring )
	{
/*GLA()->setColorMode(vcg::GLW::CMPerVert);*/
        meshDoc()->mm()->updateDataMask(MeshModel::MM_VERTCOLOR);
    }

    if(fclasses & MeshModel::MM_COLOR)
    {
/*GLA()->setColorMode(vcg::GLW::CMPerMesh);*/
        meshDoc()->mm()->updateDataMask(MeshModel::MM_COLOR);
    }

    if(fclasses & MeshFilterInterface::MeshCreation )
        GLA()->resetTrackBall();

    if(fclasses & MeshFilterInterface::Texture )
    {
//WARNING!!!!! This should be replaced in some way
//WARNING!!!!! HERE IT SHOULD BE A CHECK IF THE FILTER IS FOR MESH OR FOR DOCUMENT (IN THIS CASE I SHOULD ACTIVATE ALL THE TEXTURE MODE FOR EVERYONE...)
//NOW WE HAVE JUST TEXTURE FILTERS WORKING ON SINGLE MESH
//QMap<int,RenderMode>::iterator it = GLA()->rendermodemap.find(meshDoc()->mm()->id());
//if (it != GLA()->rendermodemap.end())
//    it.value().setTextureMode(GLW::TMPerWedgeMulti);
//////////////////////////////////////////////////////////////////////////////
        updateTexture(meshDoc()->mm()->id());
    }

    layerDialog->setVisible((layerDialog->isVisible() || ((newmeshcreated) && (meshDoc()->size() > 0))));
    updateLayerDialog();
    updateMenus();
    delete obj;
}

void MainWindow::scriptCodeExecuted( const QScriptValue& val,const int time,const QString& output )
{
    if (val.isError())
    {
        meshDoc()->Log.Logf(GLLogStream::SYSTEM, "解释器错误: 行%i: %s", val.property("lineNumber").toInt32(), qPrintable(val.toString()));
        layerDialog->updateLog(meshDoc()->Log);
    }
    else
    {
        meshDoc()->Log.Logf(GLLogStream::SYSTEM, "代码已成功执行，花费了: %d毫秒.\n结果: \n%s", time, qPrintable(output));
		//bool res;
		//updateSharedContextDataAfterFilterExecution((int)MeshModel::MM_ALL, (int)MeshFilterInterface::Generic, res);
        GLA()->update();
    }
}

// Edit Mode Managment
// At any point there can be a single editing plugin active.
// When a plugin is active it intercept the mouse actions.
// Each active editing tools
//
void MainWindow::suspendEditMode()
{
    // return if no window is open
    if(!GLA()) 
		return;

    // return if no editing action is currently ongoing
    if(!GLA()->getCurrentEditAction()) 
		return;

    GLA()->suspendEditToggle();
    updateMenus();
    GLA()->update();
}

void MainWindow::applyEditMode()
{
    if(!GLA()) 
	{ 
		//prevents crash without mesh
        QAction *action = qobject_cast<QAction *>(sender());
        action->setChecked(false);
        return;
    }

    QAction *action = qobject_cast<QAction *>(sender());
    if(GLA()->getCurrentEditAction()) //prevents multiple buttons pushed
    {
        if(action == GLA()->getCurrentEditAction()) // We have double pressed the same action and that means disable that actioon
        {
            if(GLA()->suspendedEditor)
            {
                suspendEditMode();
                return;
            }

			endEdit();
            updateMenus();
            return;
        }

        assert(0); // it should be impossible to start an action without having ended the previous one.
        return;
    }

    //if this GLArea does not have an instance of this action's MeshEdit tool then give it one
    if(!GLA()->editorExistsForAction(action))
    {
        MeshEditInterfaceFactory *iEditFactory = qobject_cast<MeshEditInterfaceFactory *>(action->parent());
        MeshEditInterface *iEdit = iEditFactory->getMeshEditInterface(action);
        GLA()->addMeshEditor(action, iEdit);
    }

	meshDoc()->meshDocStateData().create(*meshDoc());
    GLA()->setCurrentEditAction(action);
    updateMenus();
    GLA()->update();
}

void MainWindow::applyRenderMode()
{
    QAction *action = qobject_cast<QAction *>(sender());		// find the action which has sent the signal
    if ((GLA()!= NULL) && (GLA()->getRenderer() != NULL))
    {
        GLA()->getRenderer()->Finalize(GLA()->getCurrentShaderAction(), meshDoc(), GLA());
        GLA()->setRenderer(NULL,NULL);
    }

    // Make the call to the plugin core
    MeshRenderInterface *iRenderTemp = qobject_cast<MeshRenderInterface *>(action->parent());
    bool initsupport = false;
	if (currentViewContainer() == NULL)
		return;

	MLSceneGLSharedDataContext* shared = currentViewContainer()->sharedDataContext();
    if ((shared != NULL) && (iRenderTemp != NULL))
    {
		MLSceneGLSharedDataContext::PerMeshRenderingDataMap rdmap;
		shared->getRenderInfoPerMeshView(GLA()->context(), rdmap);
        iRenderTemp->Init(action,*(meshDoc()),rdmap, GLA());
        initsupport = iRenderTemp->isSupported();
        if (initsupport)
            GLA()->setRenderer(iRenderTemp,action);
        else
        {
            if (!initsupport)
            {
                QString msg = "机器硬件不支持选中的着色器!";
                GLA()->Logf(GLLogStream::SYSTEM,qPrintable(msg));
            }

            iRenderTemp->Finalize(action,meshDoc(),GLA());
        }
    }

    /*I clicked None in renderMenu */
    if ((action->parent() == this) || (!initsupport))
    {
        QString msg("没有着色器");
        GLA()->Logf(GLLogStream::SYSTEM,qPrintable(msg));
        GLA()->setRenderer(0,0); //default opengl pipeline or vertex and fragment programs not supported
    }

    GLA()->update();
}

void MainWindow::applyDecorateMode()
{
    if(GLA()->mm() == 0) 
		return;

    QAction *action = qobject_cast<QAction *>(sender());		// find the action which has sent the signal
    MeshDecorateInterface *iDecorateTemp = qobject_cast<MeshDecorateInterface *>(action->parent());
    GLA()->toggleDecorator(iDecorateTemp->decorationName(action));

	updateMenus();

	// Delete decorator property by Johnny Xu, 2017/6/28
//    layerDialog->updateDecoratorParsView();

    layerDialog->updateLog(meshDoc()->Log);
    layerDialog->update();
    GLA()->update();
}

void MainWindow::setCurrentMeshBestTab()
{
	if (layerDialog == NULL)
		return;

	MultiViewer_Container* mvc = currentViewContainer();
	if (mvc != NULL)
	{
		MLSceneGLSharedDataContext* cont = mvc->sharedDataContext();
		if ((GLA() != NULL) && (meshDoc() != NULL) && (meshDoc()->mm() != NULL))
		{
			MLRenderingData dt;
			cont->getRenderInfoPerMeshView(meshDoc()->mm()->id(), GLA()->context(), dt);
			layerDialog->setCurrentTab(dt);
		}
	}
}

void MainWindow::newProject(const QString& projName)
{
    if (gpumeminfo == NULL)
        return;

    MultiViewer_Container *mvcont = new MultiViewer_Container(*gpumeminfo, mwsettings.highprecision, mwsettings.perbatchprimitives, mwsettings.minpolygonpersmoothrendering, mdiarea);
    connect(&mvcont->meshDoc, SIGNAL(meshAdded(int)), this, SLOT(meshAdded(int)));
    connect(&mvcont->meshDoc, SIGNAL(meshRemoved(int)), this, SLOT(meshRemoved(int)));
	connect(&mvcont->meshDoc, SIGNAL(documentUpdated()), this, SLOT(documentUpdateRequested()));
	connect(mvcont, SIGNAL(closingMultiViewerContainer()), this, SLOT(closeCurrentDocument()));
    mdiarea->addSubWindow(mvcont);

    connect(mvcont, SIGNAL(updateMainWindowMenus()), this, SLOT(updateMenus()));
    connect(mvcont, SIGNAL(updateDocumentViewer()), this, SLOT(updateLayerDialog()));
	connect(&mvcont->meshDoc.Log, SIGNAL(logUpdated()), this, SLOT(updateLog()));

    filterMenu->setEnabled(!filterMenu->actions().isEmpty());
    if (!filterMenu->actions().isEmpty())
        updateSubFiltersMenu(true,false);

    GLArea *gla = new GLArea(this, mvcont, &currentGlobalParams);
    mvcont->addView(gla, Qt::Horizontal);

    if (projName.isEmpty())
    {
        static int docCounter = 1;
        mvcont->meshDoc.setDocLabel(QTextCodec::codecForName("GBK")->toUnicode("工程_") + QString::number(docCounter));
        ++docCounter;
    }
    else
        mvcont->meshDoc.setDocLabel(projName);

    mvcont->setWindowTitle(mvcont->meshDoc.docLabel());
	if (layerDialog != NULL)
		layerDialog->reset();

    updateLayerDialog();
    mvcont->showMaximized();

	// Delete GPU usage by Johnny Xu, 2017/6/27
//    connect(mvcont->sharedDataContext(), SIGNAL(currentAllocatedGPUMem(int,int)), this, SLOT(updateGPUMemBar(int,int)));
}

void MainWindow::documentUpdateRequested()
{
	if (meshDoc() == NULL)
		return;

	for (int ii = 0; ii < meshDoc()->meshList.size(); ++ii)
	{
		MeshModel* mm = meshDoc()->meshList[ii];
		if (mm != NULL)
		{
			addRenderingDataIfNewlyGeneratedMesh(mm->id());
			updateLayerDialog();
			if (currentViewContainer() != NULL)
			{
				currentViewContainer()->resetAllTrackBall();
				currentViewContainer()->updateAllViewer();
			}
		}
	}
}

void MainWindow::updateGPUMemBar(int allmem,int currentallocated)
{
	if (libGPUProcess)
	{
		if (allmem> 0&& currentallocated> 0)
		{
			int allmb = allmem/1024;
			float remainingmb = (100.0* (allmem-currentallocated))/(1024*allmb);
			int pro= remainingmb;
			QString qStr= QString::fromLocal8Bit("GPU使用率: %1%").arg(pro);
			libGPUProcess->setText(qStr);
		}
	}

//     if (nvgpumeminfo != NULL)
//     {
//         nvgpumeminfo->setFormat( QTextCodec::codecForName("GBK")->toUnicode(" GPU使用率: %p%") /*"Mem %p% %v/%m MB"*/ );
//         int allmb = allmem/1024;
//         nvgpumeminfo->setRange(0, allmb);
//         int remainingmb = (allmem-currentallocated)/1024;
//         nvgpumeminfo->setValue(remainingmb);
//         nvgpumeminfo->setFixedWidth(110);
//     }
}

//WARNING!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//Temporary disgusting inequality between open (slot) - importMesh (function)
//and importRaster (slot). It's not also difficult to remove the problem because
//addNewRaster add a raster on a document and open the file, instead addNewMesh add a new mesh layer
//without loading the model.
bool MainWindow::importRaster(const QString& fileImg)
{
    QStringList filters;
    filters.push_back("Images (*.jpg *.png *.xpm)");
    filters.push_back("*.jpg");
    filters.push_back("*.png");
    filters.push_back("*.xpm");

    QStringList fileNameList;
    if (fileImg.isEmpty())
        fileNameList = QFileDialog::getOpenFileNames(this, QTextCodec::codecForName("GBK")->toUnicode("打开文件")/*tr("Open File")*/, lastUsedDirectory.path(), filters.join(";;"));
    else
        fileNameList.push_back(fileImg);

    foreach(QString fileName,fileNameList)
    {
        QFileInfo fi(fileName);
        if( fi.suffix().toLower()=="png" || fi.suffix().toLower()=="xpm" || fi.suffix().toLower()=="jpg")
        {
            qb->show();

            if(!fi.exists()) 	
			{
                QString errorMsgFormat = "Unable to open file:\n\"%1\"\n\nError details: file %1 does not exist.";

				QMessageBox *box = new QMessageBox(QMessageBox::Critical, QString::fromLocal8Bit("打开错误"), errorMsgFormat.arg(fileName));
				box->setStandardButtons(QMessageBox::Ok);
				box->setButtonText(QMessageBox::Ok, QString::fromLocal8Bit("确定"));

#if defined (_CUSTOM_SKIN_)  

				// Cusctom titlebar by Johnny Xu, 2017/7/13
				QSkinObject *skin = new QSkinObject(box);
				skin->startSkinning();
#else  

				// Normal
#endif

				box->exec();

				return false;
            }

            if(!fi.isReadable()) 	
			{
                QString errorMsgFormat = "Unable to open file:\n\"%1\"\n\nError details: file %1 is not readable.";

				QMessageBox *box = new QMessageBox(QMessageBox::Critical, QString::fromLocal8Bit("打开错误"), errorMsgFormat.arg(fileName));
				box->setStandardButtons(QMessageBox::Ok);
				box->setButtonText(QMessageBox::Ok, QString::fromLocal8Bit("确定"));

#if defined (_CUSTOM_SKIN_)  

				// Cusctom titlebar by Johnny Xu, 2017/7/13
				QSkinObject *skin = new QSkinObject(box);
				skin->startSkinning();
#else  

				// Normal
#endif

				box->exec();

                return false;
            }

            this->meshDoc()->setBusy(true);
            RasterModel *rm= meshDoc()->addNewRaster();
            rm->setLabel(fileImg);
            rm->addPlane(new Plane(fileName,Plane::RGBA));
            meshDoc()->setBusy(false);
            showLayerDlg(true);

			/// Intrinsics extraction from EXIF
            ///	If no CCD Width value is provided, the intrinsics are extracted using the Equivalent 35mm focal
            /// If no or invalid EXIF info is found, the Intrinsics are initialized as a "plausible" 35mm sensor, with 50mm focal
			
            ::ResetJpgfile();
            FILE * pFile = fopen(qPrintable(fileName), "rb");

            int ret = ::ReadJpegSections (pFile, READ_METADATA);
            fclose(pFile);
            if (!ret || (ImageInfo.CCDWidth==0.0f && ImageInfo.FocalLength35mmEquiv==0.0f))
            {
                rm->shot.Intrinsics.ViewportPx = vcg::Point2i(rm->currentPlane->image.width(), rm->currentPlane->image.height());
                rm->shot.Intrinsics.CenterPx   = Point2m(float(rm->currentPlane->image.width()/2.0), float(rm->currentPlane->image.width()/2.0));
                rm->shot.Intrinsics.PixelSizeMm[0] = 36.0f/(float)rm->currentPlane->image.width();
                rm->shot.Intrinsics.PixelSizeMm[1] = rm->shot.Intrinsics.PixelSizeMm[0];
                rm->shot.Intrinsics.FocalMm = 50.0f;
            }
            else if (ImageInfo.CCDWidth!=0)
            {
                rm->shot.Intrinsics.ViewportPx = vcg::Point2i(ImageInfo.Width, ImageInfo.Height);
                rm->shot.Intrinsics.CenterPx   = Point2m(float(ImageInfo.Width/2.0), float(ImageInfo.Height/2.0));
                float ratio;
                if (ImageInfo.Width>ImageInfo.Height)
                    ratio = (float)ImageInfo.Width/(float)ImageInfo.Height;
                else
                    ratio = (float)ImageInfo.Height/(float)ImageInfo.Width;
                rm->shot.Intrinsics.PixelSizeMm[0] = ImageInfo.CCDWidth/(float)ImageInfo.Width;
                rm->shot.Intrinsics.PixelSizeMm[1] = ImageInfo.CCDWidth/((float)ImageInfo.Height*ratio);
                rm->shot.Intrinsics.FocalMm = ImageInfo.FocalLength;
            }
            else
            {
                rm->shot.Intrinsics.ViewportPx = vcg::Point2i(ImageInfo.Width, ImageInfo.Height);
                rm->shot.Intrinsics.CenterPx   = Point2m(float(ImageInfo.Width/2.0), float(ImageInfo.Height/2.0));
                float ratioFocal = ImageInfo.FocalLength/ImageInfo.FocalLength35mmEquiv;
                rm->shot.Intrinsics.PixelSizeMm[0] = (36.0f*ratioFocal)/(float)ImageInfo.Width;
                rm->shot.Intrinsics.PixelSizeMm[1] = (24.0f*ratioFocal)/(float)ImageInfo.Height;
                rm->shot.Intrinsics.FocalMm = ImageInfo.FocalLength;
            }

			// End of EXIF reading

			//// Since no extrinsic are available, the current trackball is reset (except for the FOV) and assigned to the raster
			GLA()->resetTrackBall();
			GLA()->fov = rm->shot.GetFovFromFocal();
			rm->shot = GLA()->shotFromTrackball().first;
           

            //			if(mdiarea->isVisible()) GLA()->mvc->showMaximized();
            updateMenus();
            updateLayerDialog();
        }
        else
            return false;
    }

    return true;
}

bool MainWindow::loadMesh(const QString& fileName, MeshIOInterface *pCurrentIOPlugin, MeshModel* mm, int& mask,RichParameterSet* prePar, const Matrix44m &mtr, bool isareload)
{
    if ((GLA() == NULL) || (mm == NULL))
        return false;

    QFileInfo fi(fileName);
    QString extension = fi.suffix();
    if(!fi.exists())
    {
        QString errorMsgFormat = "%1" + QTextCodec::codecForName("GBK")->toUnicode("文件不存在!");

		QMessageBox *box = new QMessageBox(QMessageBox::Critical, QString::fromLocal8Bit("打开错误"), errorMsgFormat.arg(fileName));
		box->setStandardButtons(QMessageBox::Ok);
		box->setButtonText(QMessageBox::Ok, QString::fromLocal8Bit("确定"));

#if defined (_CUSTOM_SKIN_)  

		// Cusctom titlebar by Johnny Xu, 2017/7/13
		QSkinObject *skin = new QSkinObject(box);
		skin->startSkinning();
#else  

		// Normal
#endif

		box->exec();

        return false;
    }

    if(!fi.isReadable())
    {
        QString errorMsgFormat = "%1" + QTextCodec::codecForName("GBK")->toUnicode("文件不可读!") /*"Unable to open file:\n\"%1\"\n\nError details: file %1 is not readable."*/;

		QMessageBox *box = new QMessageBox(QMessageBox::Critical, QString::fromLocal8Bit("打开错误"), errorMsgFormat.arg(fileName));
		box->setStandardButtons(QMessageBox::Ok);
		box->setButtonText(QMessageBox::Ok, QString::fromLocal8Bit("确定"));

#if defined (_CUSTOM_SKIN_)  

		// Cusctom titlebar by Johnny Xu, 2017/7/13
		QSkinObject *skin = new QSkinObject(box);
		skin->startSkinning();
#else  

		// Normal
#endif

		box->exec();
		return false;
    }

    // this change of dir is needed for subsequent textures/materials loading
    QDir::setCurrent(fi.absoluteDir().absolutePath());

    // retrieving corresponding IO plugin
    if (pCurrentIOPlugin == 0)
    {
        QString errorMsgFormat = "%1" + QTextCodec::codecForName("GBK")->toUnicode("文件类型不被支持!") /*"Error encountered while opening file:\n\"%1\"\n\nError details: The \"%2\" file extension does not correspond to any supported format."*/;

		QMessageBox *box = new QMessageBox(QMessageBox::Critical, QString::fromLocal8Bit("打开错误"), errorMsgFormat.arg(fileName));
		box->setStandardButtons(QMessageBox::Ok);
		box->setButtonText(QMessageBox::Ok, QString::fromLocal8Bit("确定"));

#if defined (_CUSTOM_SKIN_)  

		// Cusctom titlebar by Johnny Xu, 2017/7/13
		QSkinObject *skin = new QSkinObject(box);
		skin->startSkinning();
#else  

		// Normal
#endif

		box->exec();
        return false;
    }

    meshDoc()->setBusy(true);
    pCurrentIOPlugin->setLog(&meshDoc()->Log);
    if (!pCurrentIOPlugin->open(extension, fileName, *mm ,mask,*prePar,QCallBack,this /*gla*/))
    {

		QMessageBox *box = new QMessageBox(QMessageBox::Warning, QString::fromLocal8Bit("打开失败"), QString("'%1'\n\n").arg(fileName) + pCurrentIOPlugin->errorMsg());
		box->setStandardButtons(QMessageBox::Ok);
		box->setButtonText(QMessageBox::Ok, QString::fromLocal8Bit("确定"));

#if defined (_CUSTOM_SKIN_)  

		// Cusctom titlebar by Johnny Xu, 2017/7/13
		QSkinObject *skin = new QSkinObject(box);
		skin->startSkinning();
#else  

		// Normal
#endif

		box->exec();

		pCurrentIOPlugin->clearErrorString();
        meshDoc()->setBusy(false);
        return false;
    }

    QString err = pCurrentIOPlugin->errorMsg();
    if (!err.isEmpty())
    {
		QMessageBox *box = new QMessageBox(QMessageBox::Warning, QString::fromLocal8Bit("打开失败"), QString("'%1'\n\n").arg(fileName) + pCurrentIOPlugin->errorMsg());
		box->setStandardButtons(QMessageBox::Ok);
		box->setButtonText(QMessageBox::Ok, QString::fromLocal8Bit("确定"));

#if defined (_CUSTOM_SKIN_)  

		// Cusctom titlebar by Johnny Xu, 2017/7/12
		QSkinObject *skin = new QSkinObject(box);
		skin->startSkinning();
#else  

		// Normal
#endif

		box->exec();
		pCurrentIOPlugin->clearErrorString();
    }

    saveRecentFileList(fileName);

    if (!(mm->cm.textures.empty()))
        updateTexture(mm->id());

    // In case of polygonal meshes the normal should be updated accordingly
    if( mask & vcg::tri::io::Mask::IOM_BITPOLYGONAL)
    {
        mm->updateDataMask(MeshModel::MM_POLYGONAL); // just to be sure. Hopefully it should be done in the plugin...
        int degNum = tri::Clean<CMeshO>::RemoveDegenerateFace(mm->cm);
        if(degNum)
            GLA()->Logf(0, "已删除退化面: %i.", degNum);

        mm->updateDataMask(MeshModel::MM_FACEFACETOPO);
        vcg::tri::UpdateNormal<CMeshO>::PerBitQuadFaceNormalized(mm->cm);
        vcg::tri::UpdateNormal<CMeshO>::PerVertexFromCurrentFaceNormal(mm->cm);
    } // standard case
    else
    {
        vcg::tri::UpdateNormal<CMeshO>::PerFaceNormalized(mm->cm);
        if(!( mask & vcg::tri::io::Mask::IOM_VERTNORMAL) )
            vcg::tri::UpdateNormal<CMeshO>::PerVertexAngleWeighted(mm->cm);
    }

    vcg::tri::UpdateBounding<CMeshO>::Box(mm->cm);					// updates bounding box
    if(mm->cm.fn==0 && mm->cm.en==0)
    {
        if(mask & vcg::tri::io::Mask::IOM_VERTNORMAL)
            mm->updateDataMask(MeshModel::MM_VERTNORMAL);
    }

    if(mm->cm.fn==0 && mm->cm.en>0)
    {
        if (mask & vcg::tri::io::Mask::IOM_VERTNORMAL)
            mm->updateDataMask(MeshModel::MM_VERTNORMAL);
    }

    updateMenus();
    int delVertNum = vcg::tri::Clean<CMeshO>::RemoveDegenerateVertex(mm->cm);
    int delFaceNum = vcg::tri::Clean<CMeshO>::RemoveDegenerateFace(mm->cm);
    tri::Allocator<CMeshO>::CompactEveryVector(mm->cm);
	if (delVertNum > 0 || delFaceNum > 0)
	{
		QString strBox = QTextCodec::codecForName("GBK")->toUnicode("已修正了网格中的非数值顶点: ") + QString("%1 ").arg(delVertNum) + QTextCodec::codecForName("GBK")->toUnicode("退化的面: ") + QString("%1").arg(delFaceNum);
		QMessageBox *box = new QMessageBox(QMessageBox::Warning, QString::fromLocal8Bit("警告"), strBox);
		box->setStandardButtons(QMessageBox::Ok);
		box->setButtonText(QMessageBox::Ok, QString::fromLocal8Bit("确定"));

#if defined (_CUSTOM_SKIN_)  

		// Cusctom titlebar by Johnny Xu, 2017/7/12
		QSkinObject *skin = new QSkinObject(box);
		skin->startSkinning();
#else  

		// Normal
#endif

		box->exec();

	}

    mm->cm.Tr = mtr;
	computeRenderingDataOnLoading(mm, isareload);
	updateLayerDialog();

    meshDoc()->setBusy(false);

    return true;
}

void MainWindow::computeRenderingDataOnLoading(MeshModel* mm,bool isareload)
{
	MultiViewer_Container* mv = currentViewContainer();
	if (mv != NULL)
	{
		MLSceneGLSharedDataContext* shared = mv->sharedDataContext();
		if ((shared != NULL) && (mm != NULL))
		{
			MLRenderingData defdt;
			MLPoliciesStandAloneFunctions::suggestedDefaultPerViewRenderingData(mm, defdt,mwsettings.minpolygonpersmoothrendering);
			for (int glarid = 0; glarid < mv->viewerCounter(); ++glarid)
			{
				GLArea* ar = mv->getViewer(glarid);
				if (ar != NULL)
				{					
					if (isareload)
					{
						MLRenderingData currentdt;
						shared->getRenderInfoPerMeshView(mm->id(), ar->context(), currentdt);
						MLRenderingData newdt;
						MLPoliciesStandAloneFunctions::computeRequestedRenderingDataCompatibleWithMeshSameGLOpts(mm, currentdt, newdt);
						MLPoliciesStandAloneFunctions::setPerViewGLOptionsAccordindToWireModality(mm,newdt);
						MLPoliciesStandAloneFunctions::setBestWireModality(mm, newdt);
						shared->setRenderingDataPerMeshView(mm->id(), ar->context(), newdt);
						shared->meshAttributesUpdated(mm->id(), true, MLRenderingData::RendAtts(true));
					}
					else
						shared->setRenderingDataPerMeshView(mm->id(), ar->context(), defdt);
				}
			}

			shared->manageBuffers(mm->id());
		}
	}
}


void MainWindow::paintEvent(QPaintEvent*)
{
	QPainter pt(this);
	QColor c(241,241,241);
//	c.setAlpha(100);
	pt.fillRect(rect(), c);
}

bool MainWindow::importMeshWithLayerManagement(QString fileName)
{
    bool layervisible = false;
    if (layerDialog != NULL)
    {
        layervisible = layerDialog->isVisible();
        showLayerDlg(false);
    }

	globrendtoolbar->setEnabled(false);
    bool res = importMesh(fileName, false);
	globrendtoolbar->setEnabled(true);

	if (layerDialog != NULL)
	{
		// Fix issue after cancel dialog. Modified by Johnny Xu, 2017/2/23
		showLayerDlg(layervisible || (meshDoc() ? meshDoc()->meshList.size() : false));
	}

	// Modified by Johnny Xu, 2017/2/23
	if(res)
	{
		setCurrentMeshBestTab();;
	}

    return res;
}

// Opening files in a transparent form (IO plugins contribution is hidden to user)
bool MainWindow::importMesh(QString fileName, bool isareload)
{
    QStringList fileNameList;
	// Fixed skin issue by Johnny Xu, 2017/7/11
	if (fileName.isEmpty())
	{
		QFileDialog *openDialog = new QFileDialog(nullptr, QTextCodec::codecForName("GBK")->toUnicode("导入网格"), lastUsedDirectory.path(), PM.inpFilters.join(";;"));
#if defined(Q_OS_WIN)
		openDialog->setOption(QFileDialog::DontUseNativeDialog);
#endif

#if defined (_CUSTOM_SKIN_)  

		// Test cusctom titlebar by Johnny Xu, 2017/7/12
		QSkinObject *skin = new QSkinObject(openDialog);
		skin->startSkinning();
		openDialog->setSizeGripEnabled(false);
#else  

		// Normal
#endif
		
		openDialog->setFileMode(QFileDialog::ExistingFiles);
		openDialog->setWindowFlags(windowFlags()&~Qt::WindowMinMaxButtonsHint&~Qt::WindowContextHelpButtonHint);
		if (QDialogButtonBox *btnBox = openDialog->findChild<QDialogButtonBox *>())
		{
			if (QPushButton *btnCancel = btnBox->button(QDialogButtonBox::Cancel))
				btnCancel->setText(QString::fromLocal8Bit("取消"));
		}

		// Forbid dialog scale-down by Johnny Xu,2017/7/18
		openDialog->setFixedSize(openDialog->size());

		int dialogRet = openDialog->exec();
		if (dialogRet == QDialog::Rejected)
			return false;

		fileNameList = openDialog->selectedFiles();
	}
    else
        fileNameList.push_back(fileName);


    if (fileNameList.isEmpty())	
		return false;
    else
    {
        //save path away so we can use it again
        QString path = fileNameList.first();
        path.truncate(path.lastIndexOf("/"));
        lastUsedDirectory.setPath(path);
    }

	// Moved by Johnny Xu, 2017/2/22
    if (!GLA())
    {
        this->newProject();
        if(!GLA())
            return false;
    }

    QTime allFileTime;
    allFileTime.start();
    foreach(fileName,fileNameList)
    {
        QFileInfo fi(fileName);
        QString extension = fi.suffix();
        MeshIOInterface *pCurrentIOPlugin = PM.allKnowInputFormats[extension.toLower()];
        //pCurrentIOPlugin->setLog(gla->log);
        if (pCurrentIOPlugin == NULL)
        {
            QString errorMsgFormat(QTextCodec::codecForName("GBK")->toUnicode("不支持文件类型: ") + extension);
			QMessageBox *box = new QMessageBox(QMessageBox::Warning, QString::fromLocal8Bit("打开错误"), errorMsgFormat.arg(fileName));
			box->setStandardButtons(QMessageBox::Ok);
			box->setButtonText(QMessageBox::Ok, QString::fromLocal8Bit("确定"));

#if defined (_CUSTOM_SKIN_)  

			// Cusctom titlebar by Johnny Xu, 2017/7/12
			QSkinObject *skin = new QSkinObject(box);
			skin->startSkinning();
#else  

			// Normal
#endif

			box->exec();

            return false;
        }

        RichParameterSet prePar;
        pCurrentIOPlugin->initPreOpenParameter(extension, fileName,prePar);
        if(!prePar.isEmpty())
        {
            GenericParamDialog preOpenDialog(this, &prePar, QTextCodec::codecForName("GBK")->toUnicode("预先打开选项") /*tr("Pre-Open Options")*/);
            preOpenDialog.setFocus();
            preOpenDialog.exec();
        }

        int mask = 0;
        //MeshModel *mm= new MeshModel(gla->meshDoc);

        QFileInfo info(fileName);
		// Remove "qPrintable()" for chinese display. Modified by Johnny Xu, 2017/2/20
        MeshModel *mm = meshDoc()->addNewMesh(fileName, info.fileName());
  
        qb->show();
        QTime t;t.start();
		Matrix44m mtr;
		mtr.SetIdentity();

        bool open = loadMesh(fileName,pCurrentIOPlugin,mm,mask,&prePar,mtr,isareload);
        if(open)
        {
            GLA()->Logf(0, "打开网格: %s 花费了%i毫秒", qPrintable(fileName), t.elapsed());
            RichParameterSet par;
            pCurrentIOPlugin->initOpenParameter(extension, *mm, par);

			// Delete import setup dialog by Johnny Xu, 2017/6/27
            /*if(!par.isEmpty())
            {
                GenericParamDialog postOpenDialog(this, &par, QTextCodec::codecForName("GBK")->toUnicode("后置打开选项"));
                postOpenDialog.setFocus();
                postOpenDialog.exec();
                pCurrentIOPlugin->applyOpenParameter(extension, *mm, par);
            }*/
        }
        else
        {
            meshDoc()->delMesh(mm);
            GLA()->Logf(0, "提示: 网格%s不能被打开", qPrintable(fileName));
        }
    }// end foreach file of the input list

    GLA()->Logf(0, "打开所有文件花费了: %i毫秒", allFileTime.elapsed());
    this->currentViewContainer()->resetAllTrackBall();
    qb->reset();

    return true;
}

void MainWindow::openRecentMesh()
{
	// Commented by Johnny Xu, 2017/2/24
    /*if(!GLA()) 
		return;

    if(meshDoc()->isBusy())
		return;*/

    QAction *action = qobject_cast<QAction *>(sender());
    if (action)	
		importMeshWithLayerManagement(action->data().toString());
}

bool MainWindow::loadMeshWithStandardParams(QString& fullPath, MeshModel* mm, const Matrix44m &mtr, bool isreload)
{
    if ((meshDoc() == NULL) || (mm == NULL))
        return false;

    bool ret = false;
    mm->Clear();
    QFileInfo fi(fullPath);
    QString extension = fi.suffix();
    MeshIOInterface *pCurrentIOPlugin = PM.allKnowInputFormats[extension.toLower()];
    if(pCurrentIOPlugin != NULL)
    {
        RichParameterSet prePar;
        pCurrentIOPlugin->initPreOpenParameter(extension, fullPath,prePar);
        int mask = 0;
        QTime t;t.start();
        bool open = loadMesh(fullPath,pCurrentIOPlugin,mm,mask,&prePar,mtr,isreload);
        if(open)
        {
            GLA()->Logf(0, "打开网格%s花费了%i秒.", qPrintable(fullPath), t.elapsed());
            RichParameterSet par;
            pCurrentIOPlugin->initOpenParameter(extension, *mm, par);
            pCurrentIOPlugin->applyOpenParameter(extension, *mm, par);
            ret = true;
        }
        else
            GLA()->Logf(0, "提示: 网格%s不能被打开.", qPrintable(fullPath));
    }
    else
        GLA()->Logf(0, "提示: 网格%s不能被打开. 该程序版本没有插件支持文件格式: %s", qPrintable(fullPath), qPrintable(extension));
    
	return ret;
}

void MainWindow::reloadAllMesh()
{
    // Discards changes and reloads current file
    // save current file name
    qb->show();
    foreach(MeshModel *mmm,meshDoc()->meshList)
    {
        QString fileName = mmm->fullName();
		Matrix44m mat;
		mat.SetIdentity();
        loadMeshWithStandardParams(fileName,mmm,mat,true);
    }
    qb->reset();

    update();
    if (GLA() != NULL)
        GLA()->update();
}

void MainWindow::reload()
{
    if ((meshDoc() == NULL) || (meshDoc()->mm() == NULL))
        return;

    // Discards changes and reloads current file
    // save current file name
    qb->show();
    QString fileName = meshDoc()->mm()->fullName();
	if (fileName.isEmpty())
	{
		QMessageBox *box = new QMessageBox(QMessageBox::Warning, QString::fromLocal8Bit("错误"), QString::fromLocal8Bit("不能加载一个没保存的网格"));
		box->setStandardButtons(QMessageBox::Ok);
		box->setButtonText(QMessageBox::Ok, QString::fromLocal8Bit("确定"));

#if defined (_CUSTOM_SKIN_)  

		// Cusctom titlebar by Johnny Xu, 2017/7/12
		QSkinObject *skin = new QSkinObject(box);
		skin->startSkinning();
#else  

		// Normal
#endif

		box->exec();

		return;
	}

	Matrix44m mat;
	mat.SetIdentity();
    loadMeshWithStandardParams(fileName,meshDoc()->mm(),mat,true);
    qb->reset();

    update();
    if (GLA() != NULL)
        GLA()->update();
}

bool MainWindow::exportMesh(QString fileName,MeshModel* mod,const bool saveAllPossibleAttributes)
{
    QStringList& suffixList = PM.outFilters;

    QFileInfo fi(fileName);
    QString defaultExt = "*." + fi.suffix().toLower();
    if(defaultExt == "*.")
        defaultExt = "*.ply";

    if (mod == NULL)
        return false;

    mod->meshModified() = false;
    QString laylabel = QTextCodec::codecForName("GBK")->toUnicode("保存 ") + "\"" + mod->label() + "\"";
    QString ss = fi.absoluteFilePath();
    QFileDialog* saveDialog = new QFileDialog(nullptr, laylabel, fi.absolutePath());

	// Can't remove the blow code for 'save as' by Johnny Xu, 2017/2/7
#if defined(Q_OS_WIN)
    saveDialog->setOption(QFileDialog::DontUseNativeDialog);
#endif

#if defined (_CUSTOM_SKIN_)  

	// Cusctom titlebar by Johnny Xu, 2017/7/12
	QSkinObject *skin = new QSkinObject(saveDialog);
	skin->startSkinning();
	saveDialog->setSizeGripEnabled(false);
#else  

	// Normal
#endif

	// Delete help button by Johnny Xu, 2017/6/29
	saveDialog->setWindowFlags(windowFlags()&~Qt::WindowMinMaxButtonsHint&~Qt::WindowContextHelpButtonHint);

	// Forbid dialog scale-down by Johnny Xu,2017/7/18
	saveDialog->setFixedSize(saveDialog->size());

    saveDialog->setNameFilters(suffixList);
    saveDialog->setAcceptMode(QFileDialog::AcceptSave);
    saveDialog->setFileMode(QFileDialog::AnyFile);
    saveDialog->selectFile(fileName);
    QStringList matchingExtensions = suffixList.filter(defaultExt);
    if(!matchingExtensions.isEmpty())
        saveDialog->selectNameFilter(matchingExtensions.last());

    connect(saveDialog,SIGNAL(filterSelected(const QString&)),this,SLOT(changeFileExtension(const QString&)));


	if (QDialogButtonBox *btnBox = saveDialog->findChild<QDialogButtonBox *>())
	{
		if (QPushButton *btnCancel = btnBox->button(QDialogButtonBox::Cancel))
			btnCancel->setText(QString::fromLocal8Bit("取消"));
	}

    if (fileName.isEmpty())
	{
        saveDialog->selectFile(meshDoc()->mm()->fullName());
        int dialogRet = saveDialog->exec();
        if(dialogRet == QDialog::Rejected	)
            return false;

        fileName = saveDialog->selectedFiles ().first();
        QFileInfo fni(fileName);
        if(fni.suffix().isEmpty())
        {
            QString ext = saveDialog->selectedNameFilter();
			if (ext.length() > 6)
			{
				ext = ".ply";
			}
			else
			{
				ext.chop(1);
				ext = ext.right(4);
			}
			fileName = fileName + ext;
			qDebug("File without extension adding it by hand '%s'", qPrintable(fileName));
        }
		else
		{
			QString name= fni.fileName();
			if (0 == name.indexOf("."))
			{

				QMessageBox *box = new QMessageBox(QMessageBox::Warning, QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("##文件名称不能为空##"));
				box->setStandardButtons(QMessageBox::Ok);
				box->setButtonText(QMessageBox::Ok, QString::fromLocal8Bit("确定"));

#if defined (_CUSTOM_SKIN_)  

				// Cusctom titlebar by Johnny Xu, 2017/7/12
				QSkinObject *skin = new QSkinObject(box);
				skin->startSkinning();
#else  

				// Normal
#endif

				box->exec();

				return false;
			}
		}
    }

    bool ret = false;
    QStringList fs = fileName.split(".");
    if(!fileName.isEmpty() && fs.size() < 2)
    {

		QString strBox = QTextCodec::codecForName("GBK")->toUnicode("你必须指定一个文件格式类型!");
		QMessageBox box(QMessageBox::Warning, QString::fromLocal8Bit("保存错误"), strBox);
		box.setStandardButtons(QMessageBox::Ok);
		box.setButtonText(QMessageBox::Ok, QString::fromLocal8Bit("确定"));
		box.exec();
		return ret;
    }

    if (!fileName.isEmpty())
    {
        //save path away so we can use it again
        QString path = fileName;
        path.truncate(path.lastIndexOf("/"));
        lastUsedDirectory.setPath(path);

        QString extension = fileName;
        extension.remove(0, fileName.lastIndexOf('.')+1);

        QStringListIterator itFilter(suffixList);
        MeshIOInterface *pCurrentIOPlugin = PM.allKnowOutputFormats[extension.toLower()];
        if (pCurrentIOPlugin == 0)
		{
			QString strBox = QTextCodec::codecForName("GBK")->toUnicode("文件格式类型不被支持!");
			QMessageBox *box = new QMessageBox(QMessageBox::Warning, QString::fromLocal8Bit("未知类型"), strBox);
			box->setStandardButtons(QMessageBox::Ok);
			box->setButtonText(QMessageBox::Ok, QString::fromLocal8Bit("确定"));

#if defined (_CUSTOM_SKIN_)  

			// Cusctom titlebar by Johnny Xu, 2017/7/12
			QSkinObject *skin = new QSkinObject(box);
			skin->startSkinning();
#else  

			// Normal
#endif

			box->exec();

			return false;
        }

        //MeshIOInterface* pCurrentIOPlugin = meshIOPlugins[idx-1];
        pCurrentIOPlugin->setLog(&meshDoc()->Log);

        int capability = 0, defaultBits = 0;
        pCurrentIOPlugin->GetExportMaskCapability(extension, capability, defaultBits);

        // optional saving parameters (like ascii/binary encoding)
        RichParameterSet savePar;
        pCurrentIOPlugin->initSaveParameter(extension, *(mod), savePar);

        SaveMaskExporterDialog maskDialog(new QWidget(), mod, capability, defaultBits, &savePar, this->GLA());
        if (!saveAllPossibleAttributes)
            maskDialog.exec();
        else
        {
            maskDialog.SlotSelectionAllButton();
            maskDialog.updateMask();
        }

        int mask = maskDialog.GetNewMask();
        if (!saveAllPossibleAttributes)
        {
            maskDialog.close();
            if(maskDialog.result() == QDialog::Rejected)
                return false;
        }

        if(mask == -1)
            return false;

        qApp->setOverrideCursor(QCursor(Qt::WaitCursor));
        qb->show();
        QTime tt; 
		tt.start();
        ret = pCurrentIOPlugin->save(extension, fileName, *mod ,mask,savePar,QCallBack,this);
        qb->reset();
        GLA()->Logf(GLLogStream::SYSTEM, "保存网格%s花费了%i秒", qPrintable(fileName), tt.elapsed());
        qApp->restoreOverrideCursor();

		// Fix bug #2. Commented by Johnny Xu,2017/2/22
//        mod->setFileName(fileName);

        QSettings settings;
        int savedMeshCounter = settings.value("savedMeshCounter", 0).toInt();
        settings.setValue("savedMeshCounter", savedMeshCounter+1);
		updateLayerDialog();

		if (ret)
			QDir::setCurrent(fi.absoluteDir().absolutePath()); //set current dir
    }

    return ret;
}

void MainWindow::changeFileExtension(const QString& st)
{
    QFileDialog* fd = qobject_cast<QFileDialog*>(sender());
    if (fd == NULL)
        return;

    QRegExp extlist("\\*.\\w+");
    int start = st.indexOf(extlist);
    (void)start;
    QString ext = extlist.cap().remove("*");
    QStringList stlst = fd->selectedFiles();
    if (!stlst.isEmpty())
    {
        QFileInfo fi(stlst[0]);
        fd->selectFile(fi.baseName() + ext);
    }
}

bool MainWindow::save(const bool saveAllPossibleAttributes)
{
    return exportMesh(meshDoc()->mm()->fullName(), meshDoc()->mm(), saveAllPossibleAttributes);
}

bool MainWindow::saveAs(QString fileName,const bool saveAllPossibleAttributes)
{
    return exportMesh(fileName, meshDoc()->mm(), saveAllPossibleAttributes);
}

bool MainWindow::saveSnapshot()
{
    SaveSnapshotDialog *dialog = new SaveSnapshotDialog();

#if defined (_CUSTOM_SKIN_)  

	// Cusctom titlebar by Johnny Xu, 2017/7/12
	QSkinObject *skin = new QSkinObject(dialog);
	skin->startSkinning();
	dialog->setSizeGripEnabled(false);
	dialog->setWindowFlags(Qt::WindowStaysOnTopHint);
#else  

	// Normal
#endif

    dialog->setValues(GLA()->ss);

    if (dialog->exec() == QDialog::Accepted)
    {
        GLA()->ss = dialog->getValues();
		QFileInfo  fi (dialog->getValues().outdir);
		if (!fi.isDir())
		{
			QString strTip = dialog->getValues().outdir+ QString::fromLocal8Bit("不是一个文件路径！");

			QMessageBox *box = new QMessageBox(QMessageBox::Warning, QString::fromLocal8Bit("提示"), strTip);
			box->setStandardButtons(QMessageBox::Ok);
			box->setButtonText(QMessageBox::Ok, QString::fromLocal8Bit("确 定"));

#if defined (_CUSTOM_SKIN_)  

			// Cusctom titlebar by Johnny Xu, 2017/7/12
			QSkinObject *skin = new QSkinObject(box);
			skin->startSkinning();
#else  

			// Normal
#endif

			box->exec();

			return false;
		}

        GLA()->saveSnapshot();

        // if user ask to add the snapshot to raster layers
        /*
        if(dialog.addToRasters())
        {
        QString savedfile = QString("%1/%2%3.png")
        .arg(GLA()->ss.outdir).arg(GLA()->ss.basename)
        .arg(GLA()->ss.counter,2,10,QChar('0'));

        importRaster(savedfile);
        }
        */
        return true;
    }

    return false;
}

void MainWindow::about()
{
    QDialog *about_dialog = new QDialog();
    Ui::aboutDialog temp;
    temp.setupUi(about_dialog);

#if defined (_CUSTOM_SKIN_)  

	// Cusctom titlebar by Johnny Xu, 2017/7/12
	QSkinObject *skin = new QSkinObject(about_dialog);
	skin->startSkinning();
	about_dialog->setSizeGripEnabled(false);
	about_dialog->setWindowFlags(Qt::WindowStaysOnTopHint);
#else  

	// Normal
#endif

    // Modified by Johnny Xu, 2017/2/21
	about_dialog->setWindowFlags(Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

	// Forbid dialog scale-down by Johnny Xu,2017/7/18
	about_dialog->setFixedSize(about_dialog->size());

    temp.labelMLName->setText(MeshLabApplication::completeVersion());
    about_dialog->show();
}

void MainWindow::helpOnscreen()
{
    if(GLA()) 
		GLA()->toggleHelpVisible();
}

void MainWindow::showToolbarFile()
{
    mainToolBar->setVisible(!mainToolBar->isVisible());
}

// Added by Johnny Xu, 2017/2/22
void MainWindow::showToolbarEdit()
{
	editToolBar->setVisible(!editToolBar->isVisible());
}

// Added by Johnny Xu, 2017/2/22
void MainWindow::showToolbarFilter()
{
	filterToolBar->setVisible(!filterToolBar->isVisible());
}

// Added by Johnny Xu, 2017/2/22
void MainWindow::showToolbarDecorator()
{
	decoratorToolBar->setVisible(!decoratorToolBar->isVisible());
}

void MainWindow::showInfoPane()  
{
	if(GLA() != 0)	
		GLA()->infoAreaVisible =!GLA()->infoAreaVisible;
}

void MainWindow::showTrackBall() 
{
	if(GLA() != 0) 	
		GLA()->showTrackBall(!GLA()->isTrackBallVisible());
}

void MainWindow::resetTrackBall()
{
	if(GLA() != 0)
		GLA()->resetTrackBall();
}

void MainWindow::showLayerDlg(bool visible)
{
    if ((GLA() != 0) && (layerDialog != NULL))
	{
        layerDialog->setVisible(visible);
        showLayerDlgAct->setChecked(visible);
    }
}

void MainWindow::fullScreen()
{
	if(!globalStatusBar()->isHidden()) //   if(!isFullScreen()) // Fixed issue on custom titlebar by Johnny Xu,2017/7/17
    {
        toolbarState = saveState();
        menuBar()->hide();
        mainToolBar->hide();
        globalStatusBar()->hide();

#if defined (_CUSTOM_SKIN_)  

		// Add by Johnny Xu, 2017/7/14
		if (SkinWindow *skin = static_cast<SkinWindow *>(this->parentWidget()))
		{
			if (!skin->getTitleBar()->isHidden())
			{
				skin->getTitleBar()->hide();
			}
		}
#endif

        setWindowState(windowState()^Qt::WindowFullScreen);
        bool found = true;
        //Caso di piu' finestre aperte in tile:
        if((mdiarea->subWindowList()).size() > 1)
		{
            foreach(QWidget *w,mdiarea->subWindowList())
			{
				if(w->isMaximized()) 
					found = false;
			}

            if (found)
				mdiarea->tileSubWindows();
        }
    }
    else
    {
		restoreState(toolbarState);
        menuBar()->show();
		mainToolBar->show();
        globalStatusBar()->show();

#if defined (_CUSTOM_SKIN_)  

		// Add by Johnny Xu, 2017/7/14
		if (SkinWindow *skin = static_cast<SkinWindow *>(this->parentWidget()))
		{
			if (skin->getTitleBar()->isHidden())
			{
				skin->getTitleBar()->show();
			}
		}
#endif

        setWindowState(windowState()^ Qt::WindowFullScreen);
        bool found = true;
        //Caso di piu' finestre aperte in tile:
        if((mdiarea->subWindowList()).size() > 1)
		{
            foreach(QWidget *w,mdiarea->subWindowList())
			{
				if(w->isMaximized()) 
					found = false;
			}

            if (found)
			{
				mdiarea->tileSubWindows();
			}
        }

        fullScreenAct->setChecked(false);
    }
}

void MainWindow::keyPressEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_Return && e->modifiers() == Qt::AltModifier)
    {
        fullScreen();
        e->accept();
    }
	// Add escape key to exit fullscreen by Johnny Xu, 2017/2/20
	else if(e->key() == Qt::Key_Escape)
    {
		fullScreen();
        e->accept();
	}
    else 
		e->ignore();
}

bool MainWindow::QCallBack(const int pos, const char * str)
{
    int static lastPos = -1;
    if(pos == lastPos) 
		return true;
    lastPos = pos;

    static QTime currTime = QTime::currentTime();
    if(currTime.elapsed() < 100) 
		return true;

    currTime.start();

	// Commented for some translate issue from VCGLib by Johnny Xu, 2017/2/20
 //  MainWindow::globalStatusBar()->showMessage(str,5000);

    qb->show();
    qb->setEnabled(true);
    qb->setValue(pos);
    MainWindow::globalStatusBar()->update();
    qApp->processEvents();
    return true;
}

void MainWindow::updateTexture(int meshid)
{
    MultiViewer_Container* mvc = currentViewContainer();
    if ((mvc == NULL) || (meshDoc() == NULL)) 
        return;

    MLSceneGLSharedDataContext* shared = mvc->sharedDataContext();
    if (shared == NULL)
        return;

    MeshModel* mymesh = meshDoc()->getMesh(meshid);
    if (mymesh  == NULL)
        return;

    shared->deAllocateTexturesPerMesh(mymesh->id());

    int textmemMB = int(mwsettings.maxTextureMemory / ((float) 1024 * 1024));

    size_t totalTextureNum = 0;
    foreach (MeshModel *mp, meshDoc()->meshList)
        totalTextureNum+=mp->cm.textures.size();

    int singleMaxTextureSizeMpx = int(textmemMB/((totalTextureNum != 0)? totalTextureNum : 1));
    bool sometextfailed = false;
    QString unexistingtext = QTextCodec::codecForName("GBK")->toUnicode("网格文件") + mymesh->fullName() +  QTextCodec::codecForName("GBK")->toUnicode(": 载入纹理失败!") + "\n"; /*"In mesh file <i>" + mymesh->fullName() + "</i> : Failure loading textures:<br>";*/
    for(size_t i =0; i< mymesh->cm.textures.size();++i)
    {
        QImage img;
        QFileInfo fi(mymesh->cm.textures[i].c_str());
        QString filename = fi.absoluteFilePath();
        bool res = img.load(filename);
        sometextfailed = sometextfailed || !res;
        if(!res)
        {
            res = img.load(filename);
            if(!res)
            {
                QString errmsg = QTextCodec::codecForName("GBK")->toUnicode("载入纹理失败: ") + fi.fileName(); /*QString("Failure of loading texture %1").arg(fi.fileName());*/
                meshDoc()->Log.Log(GLLogStream::WARNING, qPrintable(errmsg));
                unexistingtext += "<font color=red>" + filename + "</font><br>";
            }
        }

        if (!res)
            res = img.load(":/images/dummy.png");

        GLuint textid = shared->allocateTexturePerMesh(meshid,img,singleMaxTextureSizeMpx);
   
		// Commented by Johnny Xu, 2017/2/21
        //if (sometextfailed)
        //    QMessageBox::warning(this, QTextCodec::codecForName("GBK")->toUnicode("纹理加载失败") /*"Texture files has not been correctly loaded"*/, unexistingtext);

        for(int tt = 0;tt < mvc->viewerCounter();++tt)
        {
            GLArea* ar = mvc->getViewer(tt);
            if (ar != NULL)
                ar->setupTextureEnv(textid);
        }
    }

	if (sometextfailed)
	{

		QMessageBox *box = new QMessageBox(QMessageBox::Warning, QString::fromLocal8Bit("纹理加载失败"), unexistingtext);
		box->setStandardButtons(QMessageBox::Ok);
		box->setButtonText(QMessageBox::Ok, QString::fromLocal8Bit("确 定"));

#if defined (_CUSTOM_SKIN_)  

		// Cusctom titlebar by Johnny Xu, 2017/7/12
		QSkinObject *skin = new QSkinObject(box);
		skin->startSkinning();
#else  

		// Normal
#endif

		box->exec();
	}
}

void MainWindow::updateProgressBar( const int pos, const QString& text )
{
    this->QCallBack(pos, qPrintable(text));
}

void MainWindow::meshAdded(int mid)
{
    MultiViewer_Container* mvc = currentViewContainer();
    if (mvc != NULL)
    {
        MLSceneGLSharedDataContext* shared = mvc->sharedDataContext();
        if (shared != NULL)
        {
            shared->meshInserted(mid);
            QList<QGLContext*> contlist;
            for(int glarid = 0; glarid < mvc->viewerCounter(); ++glarid)
            {
                GLArea* ar = mvc->getViewer(glarid);
                if (ar != NULL)
                    contlist.push_back(ar->context());
            }

            MLRenderingData defdt;
            if (meshDoc() != NULL)
            {
                MeshModel* mm = meshDoc()->getMesh(mid);
                if (mm != NULL)
                {
                    for(int glarid = 0; glarid < mvc->viewerCounter(); ++glarid)
                    {
                        GLArea* ar = mvc->getViewer(glarid);
                        if (ar != NULL)
                            shared->setRenderingDataPerMeshView(mid,ar->context(),defdt);
                    }

                    shared->manageBuffers(mid);
                }

				layerDialog->setVisible(meshDoc()->meshList.size() > 0);
				updateLayerDialog();
            }
        }
    }
}

void MainWindow::meshRemoved(int mid)
{
    MultiViewer_Container* mvc = currentViewContainer();
    if (mvc != NULL)
    {
        MLSceneGLSharedDataContext* shared = mvc->sharedDataContext();
        if (shared != NULL)
            shared->meshRemoved(mid);
    }

    updateLayerDialog();
}

void MainWindow::getRenderingData( int mid, MLRenderingData& dt) const
{
	if (mid == -1)
	{
		//if (GLA() != NULL)
			//GLA()->getPerDocGlobalRenderingData(dt);
	}
	else
	{
		MultiViewer_Container* cont = currentViewContainer();
		if (cont != NULL)
		{
			MLSceneGLSharedDataContext* share = cont->sharedDataContext();
			if ((share != NULL) && (GLA() != NULL))
				share->getRenderInfoPerMeshView(mid, GLA()->context(), dt);
		}
	}
}

void MainWindow::setRenderingData(int mid, const MLRenderingData& dt)
{
	if (mid == -1)
	{
		/*if (GLA() != NULL)
			GLA()->setPerDocGlobalRenderingData(dt);*/
	}
	else
	{
		MultiViewer_Container* cont = currentViewContainer();
		if (cont != NULL)
		{
			MLSceneGLSharedDataContext* share = cont->sharedDataContext();
			if ((share != NULL) && (GLA() != NULL))
			{
				share->setRenderingDataPerMeshView(mid, GLA()->context(), dt);
				share->manageBuffers(mid);
				//addRenderingSystemLogInfo(mid);
				if (globrendtoolbar != NULL)
				{
					MLSceneGLSharedDataContext::PerMeshRenderingDataMap mp;
					share->getRenderInfoPerMeshView(GLA()->context(), mp);
					globrendtoolbar->statusConsistencyCheck(mp);
				}
			}
		}
	}
}


void MainWindow::addRenderingSystemLogInfo(unsigned mmid)
{
    MultiViewer_Container* cont = currentViewContainer();
    if (cont != NULL)
    {
        MLRenderingData::DebugInfo deb;
        MLSceneGLSharedDataContext* share = cont->sharedDataContext();
        if ((share != NULL) && (GLA() != NULL))
        {
            share->getLog(mmid, deb);
            MeshModel* mm = meshDoc()->getMesh(mmid);
            if (mm != NULL)
            {
                QString data = QString(deb._currentlyallocated.c_str()) + "\n" + QString(deb._tobedeallocated.c_str()) + "\n" + QString(deb._tobeallocated.c_str()) + "\n" + QString(deb._tobeupdated.c_str()) + "\n";
                for(std::vector<std::string>::iterator it = deb._perviewdata.begin();it != deb._perviewdata.end();++it)
                    data += QString((*it).c_str()) + "<br>";
                meshDoc()->Log.Logf(0,data.toAscii());
            }
        }
    }
}

void MainWindow::updateRenderingDataAccordingToActionsCommonCode(int meshid, const QList<MLRenderingAction*>& acts)
{
	if (meshDoc() == NULL)
		return;

	MLRenderingData olddt;
	getRenderingData(meshid, olddt);
	MLRenderingData dt(olddt);
	foreach(MLRenderingAction* act, acts)
	{
		if (act != NULL)
			act->updateRenderingData(dt);
	}

	MeshModel* mm = meshDoc()->getMesh(meshid);
	if (mm != NULL)
	{
		MLPoliciesStandAloneFunctions::setBestWireModality(mm, dt);
		MLPoliciesStandAloneFunctions::computeRequestedRenderingDataCompatibleWithMeshSameGLOpts(mm, dt, dt);
	}

	setRenderingData(meshid, dt);

	/*if (meshid == -1)
	{
		foreach(MeshModel* mm, meshDoc()->meshList)
		{
			if (mm != NULL)
			{
				MLDefaultMeshDecorators dec(this);
				dec.updateMeshDecorationData(*mm, olddt, dt);
			}
		}
	}
	else
	{*/
		if (mm != NULL)
		{
			MLDefaultMeshDecorators dec(this);
			dec.updateMeshDecorationData(*mm, olddt, dt);
		}
	/*}*/

}

void MainWindow::updateRenderingDataAccordingToActions(int meshid,const QList<MLRenderingAction*>& acts)
{
	updateRenderingDataAccordingToActionsCommonCode(meshid, acts);
	if (GLA() != NULL)
		GLA()->update();
}

void MainWindow::updateRenderingDataAccordingToActionsToAllVisibleLayers(const QList<MLRenderingAction*>& acts)
{
	if (meshDoc() == NULL)
		return;

	for (int ii = 0; ii < meshDoc()->meshList.size(); ++ii)
	{
		MeshModel* mm = meshDoc()->meshList[ii];
		if ((mm != NULL) && (mm->isVisible()))
		{
			updateRenderingDataAccordingToActionsCommonCode(mm->id(), acts);
		}
	}

	//updateLayerDialog();
	if (GLA() != NULL)
		GLA()->update();
}

void MainWindow::updateRenderingDataAccordingToActions(int /*meshid*/, MLRenderingAction* act, QList<MLRenderingAction*>& acts)
{
	if ((meshDoc() == NULL) || (act == NULL))
		return;

	QList<MLRenderingAction*> tmpacts;
	for (int ii = 0; ii < acts.size(); ++ii)
	{
		if (acts[ii] != NULL)
		{
			MLRenderingAction* sisteract = NULL;
			acts[ii]->createSisterAction(sisteract, NULL);
			sisteract->setChecked(acts[ii] == act);
			tmpacts.push_back(sisteract);
		}
	}

	for (int hh = 0; hh < meshDoc()->meshList.size(); ++hh)
	{
		if (meshDoc()->meshList[hh] != NULL)
			updateRenderingDataAccordingToActionsCommonCode(meshDoc()->meshList[hh]->id(), tmpacts);
	}

	for (int ii = 0; ii < tmpacts.size(); ++ii)
		delete tmpacts[ii];
	tmpacts.clear();

	if (GLA() != NULL)
		GLA()->update();

	updateLayerDialog();
}

void MainWindow::updateRenderingDataAccordingToActionCommonCode(int meshid, MLRenderingAction* act)
{
	if ((meshDoc() == NULL) || (act == NULL))
		return;

	if (meshid != -1)
	{
		MLRenderingData olddt;
		getRenderingData(meshid, olddt);
		MLRenderingData dt(olddt);
		act->updateRenderingData(dt);
		MeshModel* mm = meshDoc()->getMesh(meshid);
		if (mm != NULL)
		{
			MLPoliciesStandAloneFunctions::setBestWireModality(mm, dt);
			MLPoliciesStandAloneFunctions::computeRequestedRenderingDataCompatibleWithMeshSameGLOpts(mm, dt, dt);
		}

		setRenderingData(meshid, dt);
		if (mm != NULL)
		{
			MLDefaultMeshDecorators dec(this);
			dec.updateMeshDecorationData(*mm, olddt, dt);
		}
	}
}

void MainWindow::updateRenderingDataAccordingToAction( int meshid,MLRenderingAction* act)
{
	updateRenderingDataAccordingToActionCommonCode(meshid, act);
	if (GLA() != NULL)
		GLA()->update();
}

void MainWindow::updateRenderingDataAccordingToActionToAllVisibleLayers(MLRenderingAction* act)
{
	if (meshDoc() == NULL)
		return;

	for (int ii = 0; ii < meshDoc()->meshList.size(); ++ii)
	{
		MeshModel* mm = meshDoc()->meshList[ii];
		if ((mm != NULL) && (mm->isVisible()))
		{
			updateRenderingDataAccordingToActionCommonCode(mm->id(), act);
		}
	}

	updateLayerDialog();

	if (GLA() != NULL)
		GLA()->update();
}

void  MainWindow::updateRenderingDataAccordingToActions(QList<MLRenderingGlobalAction*> actlist)
{
	if (meshDoc() == NULL) 
		return;

	for (int ii = 0; ii < meshDoc()->meshList.size(); ++ii)
	{
		MeshModel* mm = meshDoc()->meshList[ii];
		if (mm != NULL)
		{
			foreach(MLRenderingGlobalAction* act, actlist)
			{
				foreach(MLRenderingAction* ract, act->mainActions())
					updateRenderingDataAccordingToActionCommonCode(mm->id(), ract);

				foreach(MLRenderingAction* ract, act->relatedActions())
					updateRenderingDataAccordingToActionCommonCode(mm->id(), ract);
			}
		}
	}

	updateLayerDialog();

	if (GLA() != NULL)
		GLA()->update();
}

void MainWindow::updateRenderingDataAccordingToAction(int /*meshid*/, MLRenderingAction* act, bool check)
{
	MLRenderingAction* sisteract = NULL;
	act->createSisterAction(sisteract, NULL);
	sisteract->setChecked(check);

	foreach(MeshModel* mm, meshDoc()->meshList)
	{
		if (mm != NULL)
			updateRenderingDataAccordingToActionCommonCode(mm->id(), sisteract);
	}

	delete sisteract;
	if (GLA() != NULL)
		GLA()->update();

	updateLayerDialog();
}

bool MainWindow::addRenderingDataIfNewlyGeneratedMesh(int meshid)
{
	MultiViewer_Container* mvc = currentViewContainer();
	if (mvc == NULL)
		return false;

	MLSceneGLSharedDataContext* shared = mvc->sharedDataContext();
	if (shared != NULL)
	{
		MeshModel* mm = meshDoc()->getMesh(meshid);
		if ((meshDoc()->meshDocStateData().find(meshid) == meshDoc()->meshDocStateData().end()) && (mm != NULL))
		{
			MLRenderingData dttoberendered;
			MLPoliciesStandAloneFunctions::suggestedDefaultPerViewRenderingData(mm, dttoberendered,mwsettings.minpolygonpersmoothrendering);
			foreach(GLArea* gla, mvc->viewerList)
			{
				if (gla != NULL)
					shared->setRenderingDataPerMeshView(meshid, gla->context(), dttoberendered);
			}

			shared->manageBuffers(meshid);
			return true;
		}
	}

	return false;
}

unsigned int MainWindow::viewsRequiringRenderingActions(int meshid, MLRenderingAction* act)
{
	unsigned int res = 0;
	MultiViewer_Container* cont = currentViewContainer();
	if (cont != NULL)
	{
		MLSceneGLSharedDataContext* share = cont->sharedDataContext();
		if (share != NULL)
		{
			foreach(GLArea* area,cont->viewerList)
			{
				MLRenderingData dt;
				share->getRenderInfoPerMeshView(meshid, area->context(), dt);

				if (act->isRenderingDataEnabled(dt))
					++res;
			}
		}
	}

	return res;
}

void MainWindow::updateLog()
{
	GLLogStream* senderlog = qobject_cast<GLLogStream*>(sender());
	if ((senderlog != NULL) && (layerDialog != NULL))
		layerDialog->updateLog(*senderlog);
}

void MainWindow::switchCurrentContainer(QMdiSubWindow * subwin)
{
	if (subwin == NULL)
	{
		if (globrendtoolbar != NULL)
			globrendtoolbar->reset();

		// Added for update action status by Johnny Xu, 2017/2/23
		updateMenus();		
		updateStdDialog();
		updateXMLStdDialog();

		return;
	}

	if (mdiarea->currentSubWindow() != 0)
	{
		MultiViewer_Container* split = qobject_cast<MultiViewer_Container*>(mdiarea->currentSubWindow()->widget());
		if (split != NULL)
			_currviewcontainer = split;
	}

	if (_currviewcontainer != NULL)
	{
		updateLayerDialog();
		updateMenus();		
		updateStdDialog();
		updateXMLStdDialog();
	}
}

void MainWindow::closeCurrentDocument()
{
	_currviewcontainer = NULL;
	layerDialog->setVisible(false);

	if (mdiarea != NULL)
		mdiarea->closeActiveSubWindow();
}
