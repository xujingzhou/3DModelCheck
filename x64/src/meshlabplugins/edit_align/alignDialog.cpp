
#include <GL/glew.h>
#include <QDockWidget>
#include "edit_align.h"

#include "alignDialog.h"
#include <meshlab/glarea.h>

static QTextEdit *globalLogTextEdit = 0;

MeshNode *AlignDialog::currentNode() 
{ 
	return edit->currentNode(); 
}

// Global function to write on the log in the lower part of the window.
bool AlignCallBackPos(const int pos, const char * message)
{
	assert(globalLogTextEdit);

	// Modified by Johnny Xu,2017/2/17
	globalLogTextEdit->insertPlainText(QTextCodec::codecForName("GBK")->toUnicode(message) /*QString(message)*/);
	globalLogTextEdit->ensureCursorVisible();
	globalLogTextEdit->update();
	qApp->processEvents();

	return true;
}
// This signal is used to make a connection between the closure of the align dialog and the end of the editing in the GLArea
void AlignDialog::closeEvent(QCloseEvent * /*event*/)
{
	emit closing();
}

AlignDialog::AlignDialog(QWidget *parent, EditAlignPlugin *_edit) : QDockWidget(parent)
{
	// setWindowFlags( windowFlags() | Qt::WindowStaysOnTopHint | Qt::SubWindow);
	   //setVisible(false);
	AlignDialog::ui.setupUi(this);
	this->setWidget(ui.frame);
	this->setFeatures(QDockWidget::AllDockWidgetFeatures);
	this->setAllowedAreas(Qt::LeftDockWidgetArea);

	QPoint p = parent->mapToGlobal(QPoint(0, 0));
	this->setFloating(true);
	// Initialize to certer,modified by Johnny Xu,2017/2/17
	this->setGeometry(p.x() + (parent->width() - width())/2, p.y() + (parent->height() - height())/2, width(), height());

	this->edit = _edit;
	// The following connection is used to associate the click with the change of the current mesh.
	connect(ui.alignTreeWidget, SIGNAL(itemClicked(QTreeWidgetItem *, int)), this, SLOT(onClickItem(QTreeWidgetItem *, int)));
	globalLogTextEdit = ui.logTextEdit;
	globalLogTextEdit->append(QString::fromLocal8Bit("该功能需要选择两个模型进行拼接，具体操作如下:\r\n选择一个模型->黏合网格->设为基础网格->选中另一个模型->基于点黏合后，在两个模型的不同面上各选择四个点后确定，然后点击处理就可以将两个模型拼接起来。\r\n"));
	currentArc = 0;
	meshTree = 0;
}

void AlignDialog::setCurrentArc(vcg::AlignPair::Result *_currentArc)
{
	assert(meshTree);

	// First clear the backgrund of previously selected arc
	MeshTreeWidgetItem *oldArcF = A2Tf[currentArc];
	MeshTreeWidgetItem *oldArcB = A2Tb[currentArc];
	if (oldArcF != NULL)
	{
		assert(oldArcF->a == currentArc);
		oldArcF->setBackground(3, QBrush());
		oldArcB->setBackground(3, QBrush());
	}

	// if we clicked twice on the same arc deselect it
	if (_currentArc == currentArc) {
		currentArc = 0;
		return;
	}

	MeshTreeWidgetItem *newArcB = A2Tb[_currentArc];
	MeshTreeWidgetItem *newArcF = A2Tf[_currentArc];
	if (newArcB != NULL)
	{
		assert(newArcB->a == _currentArc);
		newArcB->setBackground(3, QBrush(QColor("#d0ffff")));
		newArcF->setBackground(3, QBrush(QColor("#c0ffff")));
	}
	newArcF->treeWidget()->expandItem(newArcF->parent());
	newArcB->treeWidget()->expandItem(newArcB->parent());
	currentArc = _currentArc;
	updateButtons();
}


void AlignDialog::updateCurrentNodeBackground()
{
	static MeshNode *lastCurrentNode = 0;
	assert(meshTree);

	if (lastCurrentNode && M2T[lastCurrentNode])
		M2T[lastCurrentNode]->setBackground(3, QBrush());

	MeshTreeWidgetItem *newNodeItem = M2T[currentNode()];
	if (newNodeItem != NULL)
	{
		newNodeItem->setBackground(3, QBrush(QColor(Qt::lightGray)));
		lastCurrentNode = currentNode();
	}
}

void AlignDialog::setTree(MeshTree *_meshTree)
{
	assert(meshTree == 0);
	meshTree = _meshTree;
	meshTree->cb = AlignCallBackPos;
	rebuildTree();
}

void AlignDialog::updateDialog()
{
	assert(meshTree != 0);
	assert(currentNode() == meshTree->find(currentNode()->m));
	updateButtons();
}

void AlignDialog::updateButtons()
{
	if (currentNode() == NULL)
		return;

	if(currentNode()->glued)
         ui.glueHereButton->setText(QTextCodec::codecForName("GBK")->toUnicode("撤销黏合")/*"Unglue Mesh   "*/);
    else 
		ui.glueHereButton->setText(QTextCodec::codecForName("GBK")->toUnicode("黏合网格")/*"Glue Here Mesh"*/);

	ui.pointBasedAlignButton->setDisabled(currentNode()->glued);
//	ui.manualAlignButton->setDisabled(currentNode()->glued);
//	ui.recalcButton->setDisabled(currentArc == 0);
//	ui.icpParamCurrentButton->setDisabled(currentArc == 0);
	ui.baseMeshButton->setDisabled(!currentNode()->glued);
}

MeshTreeWidgetItem::MeshTreeWidgetItem(MeshNode *meshNode)
{
	QString meshName = meshNode->m->label();

	QString labelText;
	setText(0, QString::number(meshNode->Id()));
	if (meshNode->glued)  	
		setText(2, "*");
	if (meshNode->m->visible)  
		setIcon(1, QIcon(":/layer_eye_open.png"));
	else 
		setIcon(1, QIcon(":/layer_eye_close.png"));

	// Commented and modified by Johnny Xu,2017/2/9
	setText(3, meshName);
	/*labelText.sprintf("%s", qPrintable(meshName));
	setText(3, labelText);*/

	n = meshNode;
	a = 0;
}

MeshTreeWidgetItem::MeshTreeWidgetItem(MeshTree* /*meshTree*/, vcg::AlignPair::Result *A, MeshTreeWidgetItem *parent)
{
	n = 0;
	a = A;
	parent->addChild(this);

	QString buf = QTextCodec::codecForName("GBK")->toUnicode("弧:") + QString(" %1 -> %2 ").arg((*A).FixName).arg((*A).MovName) + QTextCodec::codecForName("GBK")->toUnicode("区域:")
			+ QString(" %1 ").arg((*A).area, 6,'f',3) + QTextCodec::codecForName("GBK")->toUnicode("错误:") + QString(" %1 ").arg((*A).err,  6,'f',3)
			+ QTextCodec::codecForName("GBK")->toUnicode("采样#") + QString(" %1 (%2) ").arg((*A).ap.SampleNum,6).arg((*A).as.LastSampleUsed())
			/*QString("Arc: %1 -> %2 Area: %3 Err: %4 Sample# %5 (%6)")
			.arg((*A).FixName)
			.arg((*A).MovName)
			.arg((*A).area, 6,'f',3)
			.arg((*A).err,  6,'f',3)
			.arg((*A).ap.SampleNum,6)
			.arg((*A).as.LastSampleUsed())*/;
	setText(3, buf);

	QFont fixedFont("宋体" /*"Courier"*/);
	std::vector<vcg::AlignPair::Stat::IterInfo> &I = (*A).as.I;
	QTreeWidgetItem *itemArcIter;
	//buf.sprintf("Iter - MinD -  Error - Sample - Used - DistR - BordR - AnglR  ");
	//          " 12   123456  1234567   12345  12345   12345   12345   12345
	buf = QTextCodec::codecForName("GBK")->toUnicode("标识 - 最短距离 - 错误 - 已测采样 - 已用采样 - 废弃距离 - 废弃边界 - 废弃角度");
	itemArcIter = new QTreeWidgetItem(this);
	itemArcIter->setFont(3, fixedFont);
	itemArcIter->setText(3, buf);
	for (int qi = 0; qi < I.size(); ++qi)
	{
		buf.sprintf(" %03i  %8.2f  %7.4f   %10i  %10i  %12i  %12i  %12i",
									qi, I[qi].MinDistAbs, I[qi].pcl50,
									I[qi].SampleTested,I[qi].SampleUsed,I[qi].DistanceDiscarded,I[qi].BorderDiscarded,I[qi].AngleDiscarded );
		itemArcIter = new QTreeWidgetItem(this);
		itemArcIter->setFont(3, fixedFont);
		itemArcIter->setText(3, buf);
	}
}

void AlignDialog::rebuildTree()
{
	currentArc = 0;
	gla = edit->_gla;
	ui.alignTreeWidget->clear();
	M2T.clear();
	A2Tf.clear();
	A2Tb.clear();
    //  QList<MeshNode*> &meshList = meshTree->nodeList;
    //	for (int i = 0; i < meshList.size(); ++i)
    for(auto ni=meshTree->nodeMap.begin();ni!=meshTree->nodeMap.end();++ni)
      { 
        MeshNode *mn=ni->second;
//		MeshTreeWidgetItem *item = new MeshTreeWidgetItem(meshList.value(i));
        MeshTreeWidgetItem *item = new MeshTreeWidgetItem(mn);
//		 if(meshList.value(i)==currentNode) item->setBackground(0,QBrush(QColor(Qt::lightGray)));
//        M2T[meshList.value(i)] = item;
        M2T[mn] = item;
		ui.alignTreeWidget->addTopLevelItem(item);
	}

	// Second part add the arcs to the tree
	vcg::AlignPair::Result *A;
	MeshTreeWidgetItem *parent;
	MeshTreeWidgetItem *item;
	for (int i = 0; i < meshTree->resultList.size(); ++i)
	{
		A = &(meshTree->resultList[i]);
		// Forward arc
//		parent = M2T[meshList.at((*A).FixName)];
        parent = M2T[meshTree->nodeMap[(*A).FixName] ];
		item = new MeshTreeWidgetItem(meshTree, A, parent);
		A2Tf[A] = item;
		// Backward arc
//		parent = M2T[meshList.at((*A).MovName)];
        parent = M2T[meshTree->nodeMap [(*A).MovName] ];
		item = new MeshTreeWidgetItem(meshTree, A, parent);
		A2Tb[A] = item;
	}
	ui.alignTreeWidget->resizeColumnToContents(0);
	ui.alignTreeWidget->resizeColumnToContents(1);
	ui.alignTreeWidget->resizeColumnToContents(2);
	assert(currentNode());
	updateCurrentNodeBackground();
	updateButtons();
}
void AlignDialog::currentMeshChanged(int)
{
	this->updateDialog();
	this->rebuildTree();
}

// Called when a user click over the tree;
void AlignDialog::onClickItem(QTreeWidgetItem * item, int column)
{
	//  int row = item->data(1,Qt::DisplayRole).toInt();
	MeshTreeWidgetItem *mItem = dynamic_cast<MeshTreeWidgetItem *>(item);
	if (!mItem) 
		return; // user clicked on a iteration info (neither a node nor an arc)

	MeshNode * nn = mItem->n;
	if (nn) {
		if (column == 1)
		{
			nn->m->visible = !nn->m->visible;
			emit updateMeshSetVisibilities();
			if (nn->m->visible) mItem->setIcon(1, QIcon(":/layer_eye_open.png"));
			else mItem->setIcon(1, QIcon(":/layer_eye_close.png"));
		}
		else {
			this->edit->_md->setCurrentMesh(nn->Id());
			updateCurrentNodeBackground();
		}
	}
	else {
		assert(mItem->a);
		setCurrentArc(mItem->a);
	}
	gla->update();
	updateButtons();
}
