/**
* @Title: TitleBar
* @Package 
* @Description:
* @author �쾰��(Johnny Xu,xujingzhou2016@gmail.com)
* @date 2017/7/10
*/
#ifndef TITLE_BAR
#define TITLE_BAR

#include <QWidget>
#include <QVBoxLayout>
#include <QToolButton>
#include <QMainWindow>

class QLabel;
class QPushButton;

enum WidgetTitleBar 
{
	Lab_Ico = 0,                //���Ͻ�ͼ��
	BtnMenu = 1,                //�����˵���ť
	BtnMenu_Min = 2,            //��С����ť
	BtnMenu_Max = 3,            //��󻯰�ť
	BtnMenu_Normal = 4,         //��ԭ��ť
	BtnMenu_Close = 5           //�رհ�ť
};

//��ʽö��
enum SkinStyle
{
	Style_None = 0,
	Style_Silvery = 1,          //��ɫ��ʽ
	Style_Blue = 2,             //��ɫ��ʽ
	Style_Black = 3,			//��ɫ��ʽ
};

class TitleBar : public QWidget
{
	Q_OBJECT

	Q_PROPERTY(QString form READ getForm WRITE setForm)
	Q_PROPERTY(SkinStyle skinStyle READ getSkinStyle WRITE setSkinStyle)
	Q_ENUMS(SkinStyle)

public:
	explicit TitleBar(QWidget *parent = 0);
	virtual ~TitleBar();

	void setOnlyCloseBtn();

	void setForm(QString &value) { form = value; }
	QString getForm() const { return form; }

	void setSkinStyle(SkinStyle skinStyle)
	{
		this->skinStyle = skinStyle;
	}
	;
	SkinStyle getSkinStyle() const
	{
		return skinStyle;
	};

protected:

	virtual void mouseDoubleClickEvent(QMouseEvent *event);
	virtual void mousePressEvent(QMouseEvent *event);
	virtual bool eventFilter(QObject *obj, QEvent *event);

private slots:
	void onClicked();

private:
	void updateMaximize();

private slots:
	void changeStyle();

private:
	QLabel *m_pIconLabel;
	QLabel *m_pTitleLabel;
	QToolButton  *m_pMinimizeButton;
	QToolButton  *m_pMaximizeButton;
	QToolButton  *m_pCloseButton;
	QToolButton  *m_pMenuButton;

	QString form;
	SkinStyle skinStyle;
};

class TitleBar;
class SkinWindow : public QWidget
{
public:

	explicit SkinWindow(QWidget *wgtParent = 0);
	virtual ~SkinWindow();

	QWidget *getContentWidget() const 
	{ 
		return skinWidget;
	}

	TitleBar *getTitleBar() const 
	{ 
		return titleBar;
	}

	void startSkinning();

private:
	TitleBar *titleBar;
	QWidget *skinWidget;
};

#endif // TITLE_BAR