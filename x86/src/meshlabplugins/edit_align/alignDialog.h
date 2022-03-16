
#ifndef ALIGN_DIALOG_H
#define ALIGN_DIALOG_H
#include <QMenu>
#include <QDockWidget>
#include "meshtree.h"
class GLArea;

#include "ui_alignDialog.h"
#include <QDialog>
class EditAlignPlugin;
class MeshModel;

class MeshTreeWidgetItem : public QTreeWidgetItem
{
public:
	MeshTreeWidgetItem(MeshNode *n);
	MeshTreeWidgetItem(MeshTree* meshTree,vcg::AlignPair::Result *A,MeshTreeWidgetItem *parent);

	MeshNode *n;
	vcg::AlignPair::Result *a;
};

class AlignDialog : public QDockWidget
{
	Q_OBJECT

public:
	AlignDialog(QWidget *parent, EditAlignPlugin *_edit);
	void rebuildTree();
	void updateButtons();
	void updateDialog();
	void updateMeshVisibilities() {emit updateMeshSetVisibilities();}
	void setTree(MeshTree *);
	void updateCurrentNodeBackground();
	void setCurrentArc(vcg::AlignPair::Result *currentArc);

	Ui::alignDialog ui;
	GLArea *gla;
private:
	EditAlignPlugin *edit;
public:

	MeshTree *meshTree;
	MeshNode *currentNode();
	vcg::AlignPair::Result *currentArc;

	QMap<MeshNode *,           MeshTreeWidgetItem *> M2T; // MeshNode to treeitem hash
	QMap<vcg::AlignPair::Result  *, MeshTreeWidgetItem *> A2Tf; // Arc to treeitem hash  (forward)
	QMap<vcg::AlignPair::Result  *, MeshTreeWidgetItem *> A2Tb; // Arc to treeitem hash  (backward)

	QMenu popupMenu;

	virtual void closeEvent ( QCloseEvent * event )	;
signals:
	void closing();
	void updateMeshSetVisibilities();

public slots:
	void onClickItem(QTreeWidgetItem * item, int column );
	void currentMeshChanged(int );
};

#endif
