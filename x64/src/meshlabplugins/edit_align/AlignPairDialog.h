
#ifndef __VCG_ALIGNPAIRDIALOG
#define __VCG_ALIGNPAIRDIALOG

class QCheckBox;

#include <common/interfaces.h>
#include "AlignPairWidget.h"

#include <QCheckBox>

class AlignPairDialog: public QDialog
{
    Q_OBJECT
public:
    AlignPairWidget *alignPairDialog;

    AlignPairDialog (GLArea* gla, QWidget * parent = 0);


private slots:
	void setScalingFlag(bool);
	void setPointRenderingFlag(bool);
	void setFakeColorFlag(bool);
	
};
#endif
