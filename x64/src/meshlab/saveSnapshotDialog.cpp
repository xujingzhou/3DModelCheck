
#include "ui_savesnapshotDialog.h"
#include "saveSnapshotDialog.h"
#include <QTextCodec>
#include <QDialogButtonBox>
#include <skin/QSkinObject.h>

SaveSnapshotDialog::SaveSnapshotDialog(QWidget * parent):QDialog(parent)
{
	ui = new Ui::SSDialog();
	SaveSnapshotDialog::ui->setupUi(this);
	connect(ui->saveButton, SIGNAL(clicked()), this, SLOT(accept()));
	connect(ui->cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
	connect(ui->browseDir, SIGNAL(clicked()),this, SLOT(browseDir()));
//	ui->outDirLineEdit->setReadOnly(true);
//	setFixedSize(250,130);

	// Added by Johnny Xu,2017/2/21
//	ui->addToRastersCheckBox->setEnabled(false);

	// Delete help button by Johnny Xu, 2017/6/29
	this->setWindowFlags(windowFlags()&~Qt::WindowMinMaxButtonsHint&~Qt::WindowContextHelpButtonHint);

	// Added by Johnny Xu, 2017/7/13
	this->setFixedSize(this->size());
}

void SaveSnapshotDialog::setValues(const SnapshotSetting& ss)
{
	settings=ss;
	ui->baseNameLineEdit->setText(settings.basename);
	ui->outDirLineEdit->setText(settings.outdir);
// 	ui->resolutionSpinBox->setValue(settings.resolution);
// 	ui->counterSpinBox->setValue(settings.counter);		
}

SnapshotSetting SaveSnapshotDialog::getValues()
{
	settings.basename=ui->baseNameLineEdit->text();
	settings.outdir=ui->outDirLineEdit->text();
// 	settings.counter=ui->counterSpinBox->value();
// 	settings.resolution=ui->resolutionSpinBox->value();
	settings.background = ui->backgroundComboBox->currentIndex();
	settings.snapAllLayers=ui->alllayersCheckBox->isChecked();
// 	settings.tiledSave=ui->tiledSaveCheckBox->isChecked();
// 	settings.addToRasters=ui->addToRastersCheckBox->isChecked();
	return settings;
}

void SaveSnapshotDialog::browseDir()
{
	QFileDialog *fd = new QFileDialog(nullptr, QTextCodec::codecForName("GBK")->toUnicode("选择保存目录") /*"Choose output directory"*/);
	fd->setFileMode(QFileDialog::DirectoryOnly);

#if defined (_CUSTOM_SKIN_)  

	// Cusctom titlebar by Johnny Xu, 2017/7/12
	QSkinObject *skin = new QSkinObject(fd);
	skin->startSkinning();
	fd->setSizeGripEnabled(false);
#else  

	// Normal
#endif

	// Add skin by Johnny Xu, 2017/7/11
#if defined(Q_OS_WIN)
	fd->setOption(QFileDialog::DontUseNativeDialog);
#endif

	fd->setWindowFlags(windowFlags()&~Qt::WindowMinMaxButtonsHint&~Qt::WindowContextHelpButtonHint);
	if (QDialogButtonBox *btnBox = fd->findChild<QDialogButtonBox *>())
	{
		if (QPushButton *btnCancel = btnBox->button(QDialogButtonBox::Cancel))
			btnCancel->setText(QString::fromLocal8Bit("取消"));
	}

	// Forbid dialog scale-down by Johnny Xu,2017/7/18
	fd->setFixedSize(fd->size());

	QStringList selection;
	if (fd->exec())
	{
		selection = fd->selectedFiles();
		ui->outDirLineEdit->setText(selection.at(0));;	
	}
}

SaveSnapshotDialog::~SaveSnapshotDialog()
{
	delete ui;
}
