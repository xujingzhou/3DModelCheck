
#ifndef EditAlignPLUGIN_H
#define EditAlignPLUGIN_H

#include <common/interfaces.h>
#include "align/AlignPair.h"
#include "align/OccupancyGrid.h"
#include "meshtree.h"
#include <wrap/gui/trackball.h>
#include "alignDialog.h"

class EditAlignPlugin : public QObject, public MeshEditInterface
{
	Q_OBJECT
		Q_INTERFACES(MeshEditInterface)

		enum
	{
		ALIGN_IDLE = 0x01,
		ALIGN_INSPECT_ARC = 0x02,
		ALIGN_MOVE = 0x03
	};

	void Pick(MeshModel &m, GLArea * gla);

public:
	EditAlignPlugin();
	virtual ~EditAlignPlugin() {
	}

	static const QString Info();

	bool isSingleMeshEdit() const { return false; }
	void suggestedRenderingData(MeshModel &m, MLRenderingData& dt);
	bool StartEdit(MeshDocument &md, GLArea *parent, MLSceneGLSharedDataContext* cont);
	void Decorate(MeshModel &/*m*/, GLArea * /*parent*/);
	void EndEdit(MeshModel &/*m*/, GLArea * /*parent*/, MLSceneGLSharedDataContext* /*cont*/);
	void mousePressEvent(QMouseEvent *, MeshModel &, GLArea *);
	void mouseMoveEvent(QMouseEvent *, MeshModel &, GLArea *);
	void mouseReleaseEvent(QMouseEvent *event, MeshModel &/*m*/, GLArea *);

	QFont qFont;
	int mode;

	AlignDialog *alignDialog; //the widget with the list of the meshes.

	void toggleButtons();

	vcg::Trackball trackball;
	GLArea* _gla;
	MeshDocument* _md;
	MeshNode *currentNode() { return meshTree.find(_md->mm()); }
	vcg::AlignPair::Result *currentArc() { return  alignDialog->currentArc; }
	MeshTree meshTree;
public:
	vcg::AlignPair::Param defaultAP;  // default alignment parameters
	MeshTree::Param defaultMTP;  // default MeshTree parameters

	// this callback MUST be redefined because we are able to manage internally the layer change.
	void LayerChanged(MeshDocument & /*md*/, MeshModel & /*oldMeshModel*/, GLArea * /*parent*/, MLSceneGLSharedDataContext* )
	{
		// add code here to manage the external layer switching
	}

public slots:
	void process();
	void recalcCurrentArc();
	void glueHere();
	void glueHereVisible();
	void selectBadArc();
	void glueManual();
	void glueByPicking();
	void alignParam();
	void setAlignParamMM();
	void setAlignParamM();
	void meshTreeParam();
	void alignParamCurrent();
	void setBaseMesh();
	void hideRevealGluedMesh();

	void DrawArc(vcg::AlignPair::Result *A);
private:
	MLSceneGLSharedDataContext* _shared;
signals:
	void suspendEditToggle();

};

#endif
