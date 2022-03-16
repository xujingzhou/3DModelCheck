
#include "edit_align.h"
#include <QGLWidget>
#include "AlignPairDialog.h"
#include <QLabel>
#include <QDialogButtonBox>
#include <QDesktopWidget>

AlignPairDialog::AlignPairDialog (GLArea* gla,QWidget * parent) : QDialog(parent)
{
	// Added by Johnny Xu
	setWindowFlags(windowFlags()&~Qt::WindowMinMaxButtonsHint&~Qt::WindowContextHelpButtonHint);

    QVBoxLayout *layout = new QVBoxLayout();
	
	alignPairDialog = new AlignPairWidget(gla,this);
	connect(this, SIGNAL(finished(int)), alignPairDialog, SLOT(cleanDataOnClosing(int)));
	alignPairDialog->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok  | QDialogButtonBox::Cancel);
	// Added by Johnny Xu, 2017/2/17
	buttonBox->button(QDialogButtonBox::Ok)->setText(QTextCodec::codecForName("GBK")->toUnicode("ȷ��"));
	buttonBox->button(QDialogButtonBox::Cancel)->setText(QTextCodec::codecForName("GBK")->toUnicode("ȡ��"));

	setWindowTitle(QTextCodec::codecForName("GBK")->toUnicode("��ƴ��"));

	QHBoxLayout* checklayout = new QHBoxLayout();
	
	QCheckBox* fakecolor = new QCheckBox(QTextCodec::codecForName("GBK")->toUnicode("ʹ��α��ɫ") /*"use False Color"*/,this);
	fakecolor->setChecked(true);
	checklayout->addWidget(fakecolor);
	setFakeColorFlag(fakecolor->isChecked());
	connect(fakecolor, SIGNAL(clicked(bool)), this, SLOT(setFakeColorFlag(bool)));


	QCheckBox* pointrend = new QCheckBox(QTextCodec::codecForName("GBK")->toUnicode("ʹ�õ���Ⱦ") /*"use Point Rendering"*/, this);
    pointrend->setChecked(false);
	checklayout->addWidget(pointrend);
	setPointRenderingFlag(pointrend->isChecked());
	connect(pointrend, SIGNAL(clicked(bool)), this, SLOT(setPointRenderingFlag(bool)));

	QCheckBox* allowScalingCB = new QCheckBox(QTextCodec::codecForName("GBK")->toUnicode("��������") /*"Allow Scaling"*/, this);
	allowScalingCB->setChecked(false);
	checklayout->addWidget(allowScalingCB);
	setScalingFlag(allowScalingCB->isChecked());
	connect(allowScalingCB, SIGNAL(clicked(bool)), this, SLOT(setScalingFlag(bool)));

    QLabel *helpLabel = new QLabel(QTextCodec::codecForName("GBK")->toUnicode("�ڶ��������ϣ����ٸ�ѡȡ�ĸ��㡣�������ϵ��ѡȡ˳����Ҫһ�£�˫�����һ���µ㡣") /*"Choose at least 4 matching pair of points on the two meshes. <br>Double Click over each mesh to add new points. Choose points in consistent order"*/);
    helpLabel->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
    layout->addWidget(helpLabel);
    layout->addWidget(alignPairDialog);
	layout->addLayout(checklayout);
	layout->addWidget(buttonBox);

    setLayout(layout);
    adjustSize();

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QRect rr = QApplication::desktop()->screenGeometry ( this );
    setMinimumSize(rr.width()*0.8, rr.width()*0.5);

	buttonBox->button(QDialogButtonBox::Ok)->setMinimumWidth(80);
	buttonBox->button(QDialogButtonBox::Cancel)->setMinimumWidth(80);
}

void AlignPairDialog::setScalingFlag(bool checked)
{
	alignPairDialog->allowscaling = checked;
	alignPairDialog->update();
}

void AlignPairDialog::setPointRenderingFlag(bool checked)
{
	alignPairDialog->usePointRendering = checked;
	alignPairDialog->update();
}

void AlignPairDialog::setFakeColorFlag(bool checked)
{
	alignPairDialog->isUsingVertexColor = !checked;
	alignPairDialog->update();
}

