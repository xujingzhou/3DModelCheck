/**
* @Title: QSkinObject
* @Package
* @Description:
* @author Ðì¾°ÖÜ(Johnny Xu,xujingzhou2016@gmail.com)
* @date 2017/7/10
*/
#ifndef QSkinObject_H
#define QSkinObject_H

#include <QtGui>
#include <QtSvg>

#include "NcFramelessHelper.h"

class  QSkinObject : public QObject
{
    Q_OBJECT

public:
    QSkinObject(QWidget* wgtParent);
	~QSkinObject();
	void setSkinPath(const QString & skinpath);
	QString getSkinPath();
	int customFrameWidth();

	// Added by Johnny Xu, 2017/7/19
	QWidget *getSkinWidget()
	{
		return skinWidget;
	}

	void setBackgroundColor(QColor backgroundColor);

public slots:
	void updateStyle();
	void updateButtons();
	void startSkinning(const QString &filename = "/skin/skin.dat");
	void stopSkinning();
	void setAlphaValue(double a){alpha = a; skinWidget->setWindowOpacity(alpha);}

protected:
	bool eventFilter(QObject *o, QEvent *e);
	//Events to filter
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *mouseEvent);
	void paintEvent(QPaintEvent *event);
	void resizeEvent(QResizeEvent *e);
	//void closeEvent(QCloseEvent *e);

	void loadSkinIni(const QString &filename);
	void manageRegions();
	void timerEvent ( QTimerEvent * event );

private:
    QPoint dragPosition;
	QPixmap widgetMask;//the pixmap, in which the ready frame is stored 
	QString skinPath;
	QFont titleFont;
	QColor titleColor;
	QColor backgroundColor;
    bool gotMousePress;	
	QRegion quitButton;
	QRegion maxButton;
	QRegion minButton;
	QRect contentsRect;
	NcFramelessHelper * resizeHandler;
	bool mousePress;
	QBasicTimer *skinTimer;
	QWidget *skinWidget;
	Qt::WindowFlags flags;

private:
	qreal alpha;
	QSvgRenderer *styleRenderer;
};

#endif

