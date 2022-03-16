
#ifndef __VCGLIB_SAVEMASK_EXPORT
#define __VCGLIB_SAVEMASK_EXPORT

#include <wrap/io_trimesh/io_mask.h>

#include "stdpardialog.h"
#include "../common/filterparameter.h"
#include "glarea.h"

//
// Each file format exposes:
//  a capability bit vector with all the things that it can save (the bits are the one indicated in the IOM_XXX bit mask) ; 
//  a default bit vector that indicate what things are saved by defaults (eg. by default normals and flags are not saved in ply)
//  a vector of optional parameters (like for example a bool for ascii/binary, a bool for choosing what  
// This dialog allow to select what things actually save. 
// 
namespace Ui 
{
	class MaskExporterDialog;
} 

class SaveMaskExporterDialog : public QDialog
{
	Q_OBJECT
public:
	SaveMaskExporterDialog(QWidget *parent, MeshModel *m, int capability, int defaultBits, RichParameterSet *par, GLArea* glar = NULL);
	~SaveMaskExporterDialog();
	
	void InitDialog();
	void SetTextureName();
	int GetNewMask();
	void SetMaskCapability();
	void updateMask();

public slots:
	void on_check_help_stateChanged(int);
	void SlotOkButton();
	void SlotCancelButton();
	void SlotRenameTexture();
	void SlotSelectionTextureName();
	void SlotSelectionNoneButton();
	void SlotSelectionAllButton();

private:
	Ui::MaskExporterDialog* ui;
	MeshModel *m;
	int mask;
	int type;
	int capability;
	int defaultBits;
    RichParameterSet *parSet;
	StdParFrame *stdParFrame;
	GLArea* glar;
	
	void checkAndEnable(QCheckBox *qcb,int bit, int capabilityBits, int defaultBits);
	bool shouldBeEnabled(int bit, int capabilityBits, int defaultBits);
	bool shouldBeChecked(int bit, int capabilityBits, int defaultBits);
};//end class

#endif
