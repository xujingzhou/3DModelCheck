#ifndef ICONHELPER_H
#define ICONHELPER_H

#include <QtCore>
#include <QtGui>

#if (QT_VERSION > QT_VERSION_CHECK(5,0,0))
#include <QtWidgets>
#endif

class IconHelper : public QObject
{
    Q_OBJECT

public:
    explicit IconHelper(QObject *parent = 0);
    static IconHelper *Instance()
    {
        static QMutex mutex;

        if (!self) 
		{
            QMutexLocker locker(&mutex);

            if (!self) 
			{
                self = new IconHelper;
            }
        }

        return self;
    }

    void setIcon(QLabel *lab, QChar c, quint32 size = 9);
    void setIcon(QAbstractButton *btn, QChar c, quint32 size = 9);
    
    QPixmap getPixmap(const QString &color, QChar c, quint32 size = 9,
                      quint32 pixWidth = 10, quint32 pixHeight = 10);

    //根据按钮获取该按钮对应的图标
    QPixmap getPixmap(QToolButton *btn, bool normal);

    //指定导航面板样式,不带图标
    static void setStyle(QWidget *widget, const QString &type = "left", int borderWidth = 3,
                         const QString &borderColor = "#029FEA",
                         const QString &normalBgColor = "#292F38",
                         const QString &darkBgColor = "#1D2025",
                         const QString &normalTextColor = "#54626F",
                         const QString &darkTextColor = "#FDFDFD");

    //指定导航面板样式,带图标和效果切换
    void setStyle(QWidget *widget, QList<QToolButton *> btns, QList<int> pixChar,
                  quint32 iconSize = 9, quint32 iconWidth = 10, quint32 iconHeight = 10,
                  const QString &type = "left", int borderWidth = 3,
                  const QString &borderColor = "#029FEA",
                  const QString &normalBgColor = "#292F38",
                  const QString &darkBgColor = "#1D2025",
                  const QString &normalTextColor = "#54626F",
                  const QString &darkTextColor = "#FDFDFD");

    //指定导航按钮样式,带图标和效果切换
    void setStyle(QFrame *frame, QList<QToolButton *> btns, QList<int> pixChar,
                  quint32 iconSize = 9, quint32 iconWidth = 10, quint32 iconHeight = 10,
                  const QString &normalBgColor = "#2FC5A2",
                  const QString &darkBgColor = "#3EA7E9",
                  const QString &normalTextColor = "#EEEEEE",
                  const QString &darkTextColor = "#FFFFFF");

protected:
    bool eventFilter(QObject *watched, QEvent *event);

private:
    static IconHelper *self;    //对象自身
    QFont iconFont;             //图形字体
    QList<QToolButton *> btns;  //按钮队列
    QList<QPixmap> pixNormal;   //正常图片队列
    QList<QPixmap> pixDark;     //加深图片队列

public:
	static QChar IconMenu;          //下拉菜单图标
    static QChar IconMin;           //最小化图标
    static QChar IconMax;           //最大化图标
    static QChar IconNormal;        //还原图标
    static QChar IconClose;         //关闭图标

    static QString FontName;        //全局字体名称
    static int FontSize;            //全局字体大小
};

#endif // ICONHELPER_H
