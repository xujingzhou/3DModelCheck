/**
* @Title: TitleBar
* @Package
* @Description:
* @author Ðì¾°ÖÜ(Johnny Xu,xujingzhou2016@gmail.com)
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
#include <QPainter>
#include <QDesktopWidget>
#include "titlebar_dialog.h"

#ifdef Q_OS_WIN
#pragma comment(lib, "user32.lib")
#include <qt_windows.h>
#endif

TitleBarDialog::TitleBarDialog(QWidget *parent)
	: QWidget(parent)
{
	setFixedHeight(22);

	//this->setAttribute(Qt::WA_TranslucentBackground);
	backgroundColor = QColor(69, 141, 210);

	m_pIconLabel = new QLabel(this);
	m_pTitleLabel = new QLabel(this);
	m_pCloseButton = new QToolButton(this);

	m_pIconLabel->setFixedSize(20, 20);
	m_pIconLabel->setScaledContents(true);
	m_pIconLabel->setStyleSheet("QLabel{ background-color:transparent; }; color: white; }");

	QFont lbl_font;
	lbl_font.setPointSize(10); 
	lbl_font.setBold(true);
	m_pTitleLabel->setFont(lbl_font);
	m_pTitleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	m_pTitleLabel->setStyleSheet("QLabel{ background-color:transparent; }; color: white; }");

	m_pCloseButton->setFixedSize(25, 20);
	m_pCloseButton->setObjectName(QString::fromUtf8("btnMenu_Close"));   // Move to QSS style by Johnny Xu,2017/7/18
	m_pCloseButton->setStyleSheet("QToolButton{ color:#386487;border-style:none;border-radius:3px;padding:3px;background:none; } QToolButton:hover{ margin:1px 1px 2px 1px;background-color:rgba(238,0,0,128); color: white; }");

	QPixmap pix = style()->standardPixmap(QStyle::SP_TitleBarCloseButton);
	m_pCloseButton->setIcon(pix);

	m_pCloseButton->setToolTip(QTextCodec::codecForName("GBK")->toUnicode("¹Ø±Õ"));

	QHBoxLayout *pLayout = new QHBoxLayout(this);
	//pLayout->addWidget(m_pIconLabel);
	//pLayout->addSpacing(5);
	pLayout->addWidget(m_pTitleLabel);
	pLayout->addWidget(m_pCloseButton);
	pLayout->setSpacing(0);
	pLayout->setContentsMargins(5, 0, 5, 0);

	setLayout(pLayout);

	connect(m_pCloseButton, SIGNAL(clicked(bool)), this, SLOT(onClicked()));
}

TitleBarDialog::~TitleBarDialog()
{

}

void TitleBarDialog::setBackgroundColor(QColor backgroundColor)
{
	this->backgroundColor = backgroundColor;
	
	update();
}

void TitleBarDialog::paintEvent(QPaintEvent *event)
{
	QWidget::paintEvent(event);

	QPainter painter(this);
	QPainterPath pathBack;
	pathBack.setFillRule(Qt::WindingFill);
	pathBack.addRoundedRect(QRect(0, 0, this->width(), this->height()), 1, 1);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.fillPath(pathBack, QBrush(this->backgroundColor));

	if (this->width() != this->parentWidget()->width())
	{
		this->setFixedWidth(this->parentWidget()->width());
	}
}

void TitleBarDialog::mouseDoubleClickEvent(QMouseEvent *event)
{
	Q_UNUSED(event);
}

void TitleBarDialog::mousePressEvent(QMouseEvent *event)
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

bool TitleBarDialog::eventFilter(QObject *obj, QEvent *event)
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
		return true;
	}

	return QWidget::eventFilter(obj, event);
}

void TitleBarDialog::onClicked()
{
	QToolButton *pButton = qobject_cast<QToolButton *>(sender());
	QWidget *pWindow = this->window();
	if (pWindow->isTopLevel())
	{
		if (pButton == m_pCloseButton)
		{
			pWindow->close();
		}
	}
}

SkinDialog::SkinDialog(QWidget* wgtParent)
{
	this->setWindowFlags(Qt::FramelessWindowHint);
	setMouseTracking(true);

	titleBar = new TitleBarDialog(this);
	installEventFilter(titleBar);
	skinWidget = wgtParent;

	QVBoxLayout *vbox = new QVBoxLayout(this);
	vbox->addWidget(titleBar);
	vbox->setMargin(0);
	vbox->setSpacing(0);

	QVBoxLayout *layout = new QVBoxLayout;
	layout->addWidget(skinWidget);
	layout->setMargin(2);
	layout->setSpacing(0);
	vbox->addLayout(layout);
}

void SkinDialog::startSkinning()
{
};

SkinDialog::~SkinDialog()
{
}
