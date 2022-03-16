#include "iconhelper.h"

QChar IconHelper::IconMenu = QChar(0xf0d7);
QChar IconHelper::IconMin = QChar(0xf068);
QChar IconHelper::IconMax = QChar(0xf2d2);
QChar IconHelper::IconNormal = QChar(0xf2d0);
QChar IconHelper::IconClose = QChar(0xf00d);

#ifdef __arm__
QString IconHelper::FontName = "Droid Sans Fallback";
int IconHelper::FontSize = 13;
#else
QString IconHelper::FontName = "Microsoft Yahei";
int IconHelper::FontSize = 9;
#endif


IconHelper *IconHelper::self = 0;
IconHelper::IconHelper(QObject *) : QObject(qApp)
{
    int fontId = QFontDatabase::addApplicationFont(":/qss/fontawesome-webfont.ttf");
    QStringList fontName = QFontDatabase::applicationFontFamilies(fontId);

    if (fontName.count() > 0) 
	{
        iconFont = QFont(fontName.at(0));
    } 
	else 
	{
        qDebug() << "load fontawesome-webfont.ttf error";
    }
}

void IconHelper::setIcon(QLabel *lab, QChar c, quint32 size)
{
    iconFont.setPointSize(size);
    lab->setFont(iconFont);
    lab->setText(c);
}

void IconHelper::setIcon(QAbstractButton *btn, QChar c, quint32 size)
{
    iconFont.setPointSize(size);
    btn->setFont(iconFont);
    btn->setText(c);
}

QPixmap IconHelper::getPixmap(const QString &color, QChar c, quint32 size,
                              quint32 pixWidth, quint32 pixHeight)
{
    QPixmap pix(pixWidth, pixHeight);
    pix.fill(Qt::transparent);

    QPainter painter;
    painter.begin(&pix);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    painter.setPen(QColor(color));
    painter.setBrush(QColor(color));

    iconFont.setPointSize(size);
    painter.setFont(iconFont);
    painter.drawText(pix.rect(), Qt::AlignCenter, c);
    painter.end();

    return pix;
}

QPixmap IconHelper::getPixmap(QToolButton *btn, bool normal)
{
    QPixmap pix;
    int index = btns.indexOf(btn);

    if (index >= 0) 
	{
        if (normal) 
		{
            pix = pixNormal.at(index);
        } 
		else 
		{
            pix = pixDark.at(index);
        }
    }

    return pix;
}

void IconHelper::setStyle(QWidget *widget, const QString &type, int borderWidth, const QString &borderColor,
                          const QString &normalBgColor, const QString &darkBgColor,
                          const QString &normalTextColor, const QString &darkTextColor)
{
    QString strBorder;
    if (type == "top") 
	{
        strBorder = QString("border-width:%1px 0px 0px 0px;padding:%1px %2px %2px %2px;")
                    .arg(borderWidth).arg(borderWidth * 2);
    } 
	else if (type == "right") 
	{
        strBorder = QString("border-width:0px %1px 0px 0px;padding:%2px %1px %2px %2px;")
                    .arg(borderWidth).arg(borderWidth * 2);
    } 
	else if (type == "bottom") 
	{
        strBorder = QString("border-width:0px 0px %1px 0px;padding:%2px %2px %1px %2px;")
                    .arg(borderWidth).arg(borderWidth * 2);
    } 
	else if (type == "left") 
	{
        strBorder = QString("border-width:0px 0px 0px %1px;padding:%2px %2px %2px %1px;")
                    .arg(borderWidth).arg(borderWidth * 2);
    }

    QStringList qss;
    qss.append(QString("QWidget[flag=\"%1\"] QAbstractButton{border-style:none;border-radius:0px;padding:5px;color:%2;background:%3;}")
               .arg(type).arg(normalTextColor).arg(normalBgColor));

    qss.append(QString("QWidget[flag=\"%1\"] QAbstractButton:hover,"
                       "QWidget[flag=\"%1\"] QAbstractButton:pressed,"
                       "QWidget[flag=\"%1\"] QAbstractButton:checked{"
                       "border-style:solid;%2border-color:%3;color:%4;background:%5;}")
               .arg(type).arg(strBorder).arg(borderColor).arg(darkTextColor).arg(darkBgColor));

    widget->setStyleSheet(qss.join(""));
}

void IconHelper::setStyle(QWidget *widget, QList<QToolButton *> btns, QList<int> pixChar,
                          quint32 iconSize, quint32 iconWidth, quint32 iconHeight,
                          const QString &type, int borderWidth, const QString &borderColor,
                          const QString &normalBgColor, const QString &darkBgColor,
                          const QString &normalTextColor, const QString &darkTextColor)
{
    int btnCount = btns.count();
    int charCount = pixChar.count();
    if (btnCount <= 0 || charCount <= 0 || btnCount != charCount) 
	{
        return;
    }

    QString strBorder;
    if (type == "top") 
	{
        strBorder = QString("border-width:%1px 0px 0px 0px;padding:%1px %2px %2px %2px;")
                    .arg(borderWidth).arg(borderWidth * 2);
    } 
	else if (type == "right") 
	{
        strBorder = QString("border-width:0px %1px 0px 0px;padding:%2px %1px %2px %2px;")
                    .arg(borderWidth).arg(borderWidth * 2);
    } 
	else if (type == "bottom") 
	{
        strBorder = QString("border-width:0px 0px %1px 0px;padding:%2px %2px %1px %2px;")
                    .arg(borderWidth).arg(borderWidth * 2);
    } 
	else if (type == "left") 
	{
        strBorder = QString("border-width:0px 0px 0px %1px;padding:%2px %2px %2px %1px;")
                    .arg(borderWidth).arg(borderWidth * 2);
    }

    QStringList qss;
    qss.append(QString("QWidget[flag=\"%1\"] QAbstractButton{border-style:none;border-radius:0px;padding:5px;color:%2;background:%3;}")
               .arg(type).arg(normalTextColor).arg(normalBgColor));

    qss.append(QString("QWidget[flag=\"%1\"] QAbstractButton:hover,"
                       "QWidget[flag=\"%1\"] QAbstractButton:pressed,"
                       "QWidget[flag=\"%1\"] QAbstractButton:checked{"
                       "border-style:solid;%2border-color:%3;color:%4;background:%5;}")
               .arg(type).arg(strBorder).arg(borderColor).arg(darkTextColor).arg(darkBgColor));

    qss.append(QString("QWidget#%1{background:%2;}").arg(widget->objectName()).arg(normalBgColor));

    qss.append(QString("QToolButton{border-width:0px;}"));
    qss.append(QString("QToolButton{background-color:%1;color:%2;}")
               .arg(normalBgColor).arg(normalTextColor));
    qss.append(QString("QToolButton:hover,QToolButton:pressed,QToolButton:checked{background-color:%1;color:%2;}")
               .arg(darkBgColor).arg(darkTextColor));

    widget->setStyleSheet(qss.join(""));

    for (int i = 0; i < btnCount; i++) 
	{
        //�洢��Ӧ��ť����,�����������ȥ��ʱ���л�ͼƬ
        QPixmap pixNormal = getPixmap(normalTextColor, QChar(pixChar.at(i)), iconSize, iconWidth, iconHeight);
        QPixmap pixDark = getPixmap(darkTextColor, QChar(pixChar.at(i)), iconSize, iconWidth, iconHeight);

        btns.at(i)->setIcon(QIcon(pixNormal));
        btns.at(i)->setIconSize(QSize(iconWidth, iconHeight));
        btns.at(i)->installEventFilter(this);

        this->btns.append(btns.at(i));
        this->pixNormal.append(pixNormal);
        this->pixDark.append(pixDark);
    }
}

void IconHelper::setStyle(QFrame *frame, QList<QToolButton *> btns, QList<int> pixChar,
                          quint32 iconSize, quint32 iconWidth, quint32 iconHeight,
                          const QString &normalBgColor, const QString &darkBgColor,
                          const QString &normalTextColor, const QString &darkTextColor)
{
    int btnCount = btns.count();
    int charCount = pixChar.count();
    if (btnCount <= 0 || charCount <= 0 || btnCount != charCount) 
	{
        return;
    }

    QStringList qss;
    qss.append(QString("QToolButton{border-width:0px;}"));
    qss.append(QString("QToolButton{background-color:%1;color:%2;}")
               .arg(normalBgColor).arg(normalTextColor));
    qss.append(QString("QToolButton:hover,QToolButton:pressed,QToolButton:checked{background-color:%1;color:%2;}")
               .arg(darkBgColor).arg(darkTextColor));

    frame->setStyleSheet(qss.join(""));

    for (int i = 0; i < btnCount; i++) 
	{
        //�洢��Ӧ��ť����,�����������ȥ��ʱ���л�ͼƬ
        QPixmap pixNormal = getPixmap(normalTextColor, QChar(pixChar.at(i)), iconSize, iconWidth, iconHeight);
        QPixmap pixDark = getPixmap(darkTextColor, QChar(pixChar.at(i)), iconSize, iconWidth, iconHeight);

        btns.at(i)->setIcon(QIcon(pixNormal));
        btns.at(i)->setIconSize(QSize(iconWidth, iconHeight));
        btns.at(i)->installEventFilter(this);

        this->btns.append(btns.at(i));
        this->pixNormal.append(pixNormal);
        this->pixDark.append(pixDark);
    }
}

bool IconHelper::eventFilter(QObject *watched, QEvent *event)
{
    if (watched->inherits("QToolButton")) 
	{
        QToolButton *btn = (QToolButton *)watched;
        int index = btns.indexOf(btn);
        if (index >= 0) 
		{
            if (event->type() == QEvent::Enter) 
			{
                btn->setIcon(QIcon(pixDark.at(index)));
            } 
			else if (event->type() == QEvent::Leave) 
			{
                if (btn->isChecked()) 
				{
                    btn->setIcon(QIcon(pixDark.at(index)));
                } 
				else 
				{
                    btn->setIcon(QIcon(pixNormal.at(index)));
                }
            }
        }
    }

    return QObject::eventFilter(watched, event);
}

