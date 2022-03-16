/**
* @Title: TitleBarDialog
* @Package 
* @Description:
* @author Ðì¾°ÖÜ(Johnny Xu,xujingzhou2016@gmail.com)
* @date 2017/7/10
*/
#ifndef TITLE_BAR_DIALOG
#define TITLE_BAR_DIALOG

#include <QWidget>
#include <QVBoxLayout>
#include <QToolButton>
#include <QDialog>

class QLabel;
class QPushButton;

class TitleBarDialog : public QWidget
{
	Q_OBJECT

public:
	explicit TitleBarDialog(QWidget *parent = 0);
	virtual ~TitleBarDialog();

	void setBackgroundColor(QColor backgroundColor);

protected:

	virtual void mouseDoubleClickEvent(QMouseEvent *event);
	virtual void mousePressEvent(QMouseEvent *event);
	virtual bool eventFilter(QObject *obj, QEvent *event);

private:
	void paintEvent(QPaintEvent *event);

private slots:
	void onClicked();

private:
	QLabel *m_pIconLabel;
	QLabel *m_pTitleLabel;
	QToolButton  *m_pCloseButton;

	QColor backgroundColor;
};

class TitleBarDialog;
class SkinDialog : public QDialog
{
public:

	explicit SkinDialog(QWidget *wgtParent = 0);
	virtual ~SkinDialog();

	QWidget *getContentWidget() const 
	{ 
		return skinWidget;
	}

	TitleBarDialog *getTitleBar() const
	{ 
		return titleBar;
	}

	void startSkinning();

private:
	TitleBarDialog *titleBar;
	QWidget *skinWidget;
};

#endif // TITLE_BAR_DIALOG