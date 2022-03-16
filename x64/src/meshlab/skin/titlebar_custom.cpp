/**
* @Title: TitleBar
* @Package
* @Description:
* @author 徐景周(Johnny Xu,xujingzhou2016@gmail.com)
* @date 2017/7/10
*/
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QTextCodec>
#include <QStyle>
#include <QDesktopWidget>
#include "utils.h"
#include "titlebar_custom.h"

#if (QT_VERSION > QT_VERSION_CHECK(5,0,0))
#include <QtWidgets>
#endif

#ifdef Q_OS_WIN
#pragma comment(lib, "user32.lib")
#include <qt_windows.h>
#endif

TitleBar::TitleBar(QWidget *parent)
	: QWidget(parent)
{
	setFixedHeight(33);

	m_pIconLabel = new QLabel(this);
	m_pTitleLabel = new QLabel(this);
	m_pMinimizeButton = new QToolButton(this);
	m_pMaximizeButton = new QToolButton(this);
	m_pCloseButton = new QToolButton(this);
	m_pMenuButton = new QToolButton(this);

	m_pIconLabel->setFixedSize(25, 25);
	m_pIconLabel->setScaledContents(true);

	QFont lbl_font;
	lbl_font.setPointSize(10); 
	lbl_font.setBold(true);
	m_pTitleLabel->setFont(lbl_font);
	m_pTitleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	m_pMinimizeButton->setFixedSize(30, 25);
	m_pMaximizeButton->setFixedSize(30, 25);
	m_pCloseButton->setFixedSize(30, 25);
	m_pMenuButton->setFixedSize(30, 25);

	QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
	sizePolicy.setHorizontalStretch(0);
	sizePolicy.setVerticalStretch(0);
	sizePolicy.setHeightForWidth(m_pMenuButton->sizePolicy().hasHeightForWidth());
	m_pMenuButton->setSizePolicy(sizePolicy);

	QSizePolicy sizePolicy1(QSizePolicy::Minimum, QSizePolicy::Expanding);
	sizePolicy1.setHorizontalStretch(0);
	sizePolicy1.setVerticalStretch(0);
	sizePolicy1.setHeightForWidth(m_pMinimizeButton->sizePolicy().hasHeightForWidth());
	m_pMinimizeButton->setSizePolicy(sizePolicy1);

	sizePolicy.setHeightForWidth(m_pMaximizeButton->sizePolicy().hasHeightForWidth());
	m_pMaximizeButton->setSizePolicy(sizePolicy);

	sizePolicy.setHeightForWidth(m_pCloseButton->sizePolicy().hasHeightForWidth());
	m_pCloseButton->setSizePolicy(sizePolicy);

	// Move to QSS style by Johnny Xu,2017/7/18
	m_pMinimizeButton->setObjectName(QString::fromUtf8("btnMenu_Min"));
	m_pMaximizeButton->setObjectName(QString::fromUtf8("btnMenu_Max"));
	m_pCloseButton->setObjectName(QString::fromUtf8("btnMenu_Close"));
	m_pMenuButton->setObjectName(QString::fromUtf8("btnMenu"));

	QPixmap pix = style()->standardPixmap(QStyle::SP_TitleBarCloseButton);
	m_pCloseButton->setIcon(pix);

	pix = style()->standardPixmap(QStyle::SP_TitleBarMaxButton);
	m_pMaximizeButton->setIcon(pix);

	pix = style()->standardPixmap(QStyle::SP_TitleBarMinButton);
	m_pMinimizeButton->setIcon(pix);

	QPixmap pixmap(":/qss/skin1.png");
	m_pMenuButton->setIcon(pixmap);

	m_pMinimizeButton->setCursor(QCursor(Qt::ArrowCursor));
	m_pMaximizeButton->setCursor(QCursor(Qt::ArrowCursor));
	m_pCloseButton->setCursor(QCursor(Qt::ArrowCursor));

	m_pMinimizeButton->setToolTip(QString::fromLocal8Bit("最小化"));
	m_pMaximizeButton->setToolTip(QString::fromLocal8Bit("最大化"));
	m_pCloseButton->setToolTip(QString::fromLocal8Bit("关闭"));
	m_pMenuButton->setToolTip(QString::fromLocal8Bit("皮肤"));

	m_pMinimizeButton->setCheckable(false);
	m_pMaximizeButton->setCheckable(false);
	m_pCloseButton->setCheckable(false);
	m_pMenuButton->setCheckable(false);

	QHBoxLayout *pLayout = new QHBoxLayout(this);
	pLayout->addWidget(m_pIconLabel);
	pLayout->addSpacing(5);
	pLayout->addWidget(m_pTitleLabel);
	pLayout->addWidget(m_pMenuButton);
	pLayout->addWidget(m_pMinimizeButton);
	pLayout->addWidget(m_pMaximizeButton);
	pLayout->addWidget(m_pCloseButton);
	pLayout->setSpacing(0);
	pLayout->setContentsMargins(5, 0, 5, 0);

	setLayout(pLayout);

	m_pMenuButton->setFocusPolicy(Qt::NoFocus);
	m_pMenuButton->setPopupMode(QToolButton::InstantPopup);
	QStringList name;
	name << QString::fromLocal8Bit("原型") << QString::fromLocal8Bit("蓝色") << QString::fromLocal8Bit("银色");
	foreach(QString str, name) 
	{
		QAction *action = new QAction(str, this);
		m_pMenuButton->addAction(action);
		connect(action, SIGNAL(triggered(bool)), this, SLOT(changeStyle()));
	}

	connect(m_pMinimizeButton, SIGNAL(clicked(bool)), this, SLOT(onClicked()));
	connect(m_pMaximizeButton, SIGNAL(clicked(bool)), this, SLOT(onClicked()));
	connect(m_pCloseButton, SIGNAL(clicked(bool)), this, SLOT(onClicked()));

	this->setProperty("form", "top");
	setSkinStyle(SkinStyle::Style_Blue);

	// Test by Johnny Xu, 2017/7/28
	//m_pMenuButton->setVisible(false);
}

void TitleBar::changeStyle()
{
	QAction *act = (QAction *)sender();
	QString name = act->text();
	QString qssFile = "";

	if (name == QString::fromLocal8Bit("银色")) 
	{
		qssFile = ":/qss/Style_Silvery.qss";
		qDebug("qssFile: '%s'", qPrintable(qssFile));

		setSkinStyle(SkinStyle::Style_Silvery);
	}
	else if (name == QString::fromLocal8Bit("蓝色")) 
	{
		qssFile = ":/qss/Style_LightBlue.qss";
		qDebug("qssFile: '%s'", qPrintable(qssFile));

		setSkinStyle(SkinStyle::Style_Blue);
	}
	else if (name == QString::fromLocal8Bit("黑色"))
	{
		qssFile = ":/qss/Style_Black.qss";
		qDebug("qssFile: '%s'", qPrintable(qssFile));

		setSkinStyle(SkinStyle::Style_Black);
	}
	else
	{
		utils::setStyle(qssFile);
		m_pMenuButton->setStyleSheet("QToolButton::menu-indicator{ image:None;} QToolButton{ color:#386487;border-style:none;border-radius:3px;padding:3px;background:none; } QToolButton:hover{ background-color:rgba(51,127,209,230); color: white;margin:1px 1px 2px 1px; }");
		m_pMinimizeButton->setStyleSheet("QToolButton{ color:#386487;border-style:none;border-radius:3px;padding:3px;background:none; } QToolButton:hover{ background-color:rgba(51,127,209,230); color: white;margin:1px 1px 2px 1px; }");
		m_pMaximizeButton->setStyleSheet("QToolButton{ color:#386487;border-style:none;border-radius:3px;padding:3px;background:none; } QToolButton:hover{ background-color:rgba(51,127,209,230); color: white;margin:1px 1px 2px 1px; }");
		m_pCloseButton->setStyleSheet("QToolButton{ color:#386487;border-style:none;border-radius:3px;padding:3px;background:none; } QToolButton:hover{ margin:1px 1px 2px 1px;background-color:rgba(238,0,0,128); color: white; }");
		qDebug("qssFile: '%s'", qPrintable(qssFile));

		setSkinStyle(SkinStyle::Style_None);
	}

	utils::setStyle(qssFile);
}

TitleBar::~TitleBar()
{

}

void TitleBar::mouseDoubleClickEvent(QMouseEvent *event)
{
	Q_UNUSED(event);

	emit m_pMaximizeButton->clicked();
}

void TitleBar::mousePressEvent(QMouseEvent *event)
{
#ifdef Q_OS_WIN
	if (ReleaseCapture())
	{
		QWidget *pWindow = this->window();
		if (pWindow->isTopLevel())
		{
			SendMessage(HWND(pWindow->winId()), WM_SYSCOMMAND, SC_MOVE + HTCAPTION, 0);
		}
	}
	event->ignore();
#else
#endif
}

bool TitleBar::eventFilter(QObject *obj, QEvent *event)
{
	switch (event->type())
	{
	case QEvent::WindowTitleChange:
	{
		QWidget *pWidget = qobject_cast<QWidget *>(obj);
		if (pWidget)
		{
			m_pTitleLabel->setText(pWidget->windowTitle());
			return true;
		}
	}
	case QEvent::WindowIconChange:
	{
		QWidget *pWidget = qobject_cast<QWidget *>(obj);
		if (pWidget)
		{
			QIcon icon = pWidget->windowIcon();
			m_pIconLabel->setPixmap(icon.pixmap(m_pIconLabel->size()));
			return true;
		}
	}
	case QEvent::WindowStateChange:
	case QEvent::Resize:
		updateMaximize();
		return true;
	}

	return QWidget::eventFilter(obj, event);
}

void TitleBar::onClicked()
{
	QToolButton *pButton = qobject_cast<QToolButton *>(sender());
	QWidget *pWindow = this->window();
	if (pWindow->isTopLevel())
	{
		if (pButton == m_pMinimizeButton)
		{
			pWindow->showMinimized();
		}
		else if (pButton == m_pMaximizeButton)
		{
			pWindow->isMaximized() ? pWindow->showNormal() : pWindow->showMaximized();
		}
		else if (pButton == m_pCloseButton)
		{
			pWindow->close();
		}
	}
}

void TitleBar::updateMaximize()
{
	QWidget *pWindow = this->window();
	if (pWindow->isTopLevel())
	{
		bool bMaximize = pWindow->isMaximized();
		if (bMaximize)
		{
			QPixmap pix = style()->standardPixmap(QStyle::SP_TitleBarNormalButton);
			m_pMaximizeButton->setIcon(pix);
			m_pMaximizeButton->setToolTip(QString::fromLocal8Bit("还原"));
			m_pMaximizeButton->setProperty("maximizeProperty", "restore");
		}
		else
		{
			QPixmap pix = style()->standardPixmap(QStyle::SP_TitleBarMaxButton);
			m_pMaximizeButton->setIcon(pix);
			m_pMaximizeButton->setProperty("maximizeProperty", "maximize");
			m_pMaximizeButton->setToolTip(QString::fromLocal8Bit("最大化"));
		}

		m_pMaximizeButton->setStyle(QApplication::style());
	}
}

void TitleBar::setOnlyCloseBtn()
{
	this->m_pMinimizeButton->setVisible(false);
	this->m_pMaximizeButton->setVisible(false);
}

SkinWindow::SkinWindow(QWidget* wgtParent)
{
	this->setWindowFlags(Qt::FramelessWindowHint |
		Qt::WindowSystemMenuHint |
		Qt::WindowMinMaxButtonsHint);
	setMouseTracking(true);

	titleBar = new TitleBar(this);
	installEventFilter(titleBar);
	skinWidget = wgtParent;

	QVBoxLayout *vbox = new QVBoxLayout(this);
	vbox->addWidget(titleBar);

	QVBoxLayout *layout = new QVBoxLayout;
	layout->addWidget(skinWidget);
	vbox->addLayout(layout);

	vbox->setSpacing(0);
	vbox->setContentsMargins(0, 0, 0, 0);
}

void SkinWindow::startSkinning()
{
	//skinWidget->showMaximized();
};

SkinWindow::~SkinWindow()
{

}

