/**
* @Title: utils
* @Package
* @Description:
* @author Johnny Xu(xujingzhou2016@gmail.com)
* @date 2017/7/28
*/
#include <QtCore/QSharedMemory> 
#include "skin/head.h"
#include "utils.h"

utils* utils::pInstance = NULL;
utils *utils::getInstance()
{
	if (pInstance == NULL)
	{
		pInstance = new utils();
		atexit(release);
	}

	return pInstance;
}

void utils::release()
{
	if (pInstance)
	{
		delete pInstance;
		pInstance = NULL;
	}
}

utils::utils()
{
}

utils::~utils()
{
}

int utils::deskWidth()
{
	return qApp->desktop()->availableGeometry().width();
}

int utils::deskHeight()
{
	return qApp->desktop()->availableGeometry().height();
}

QString utils::appName()
{
	QString name = qApp->applicationFilePath();
	QStringList list = name.split("/");
	name = list.at(list.count() - 1).split(".").at(0);
	return name;
}

QString utils::appPath()
{
#ifdef __arm__
	return "/sdcard/Android/lys";
	//return QStandardPaths::writableLocation(QStandardPaths::DataLocation);
#else
	return qApp->applicationDirPath();
#endif
}

void utils::newDir(const QString &dirName)
{
	QString strDir = dirName;

	//如果路径中包含斜杠字符则说明是绝对路径
	//linux系统路径字符带有 /  windows系统 路径字符带有 :/
	if (!strDir.startsWith("/") && !strDir.contains(":/")) 
	{
		strDir = QString("%1/%2").arg(utils::appPath()).arg(strDir);
	}

	QDir dir(strDir);
	if (!dir.exists()) 
	{
		dir.mkpath(strDir);
	}
}

void utils::writeInfo(const QString &info, const QString &filePath)
{
	QString fileName = QString("%1/%2/%3_runinfo_%4.txt").arg(utils::appPath())
		.arg(filePath).arg(utils::appName()).arg(QDate::currentDate().year());

	QFile file(fileName);
	file.open(QIODevice::WriteOnly | QIODevice::Append | QFile::Text);
	QTextStream stream(&file);
	stream << DATETIME << info << NEWLINE;
	file.close();
}

void utils::setStyle(const QString &qssFile)
{
	if (qssFile.isEmpty() || qssFile.isNull())
	{
		qApp->setStyleSheet("");
		return;
	}

	QFile file(qssFile);
	if (file.open(QFile::ReadOnly))
	{
		QString qss = QLatin1String(file.readAll());
		qApp->setStyleSheet(qss);
		file.close();
	}
}

void utils::setStyle(const QString &qssFile, QString &paletteColor, QString &textColor)
{
	QFile file(qssFile);

	if (file.open(QFile::ReadOnly)) 
	{
		QString qss = QLatin1String(file.readAll());
		paletteColor = qss.mid(20, 7);
		textColor = qss.mid(49, 7);
		qApp->setPalette(QPalette(QColor(paletteColor)));
		qApp->setStyleSheet(qss);
		file.close();
	}
}

void utils::setStyle(const QString &qssFile, QString &textColor, QString &panelColor, QString &borderColor,
	QString &normalColorStart, QString &normalColorEnd,
	QString &darkColorStart, QString &darkColorEnd, QString &highColor)
{
	QFile file(qssFile);

	if (file.open(QFile::ReadOnly)) 
	{
		QString qss = QLatin1String(file.readAll());
		getQssColor(qss, textColor, panelColor, borderColor, normalColorStart, normalColorEnd, darkColorStart, darkColorEnd, highColor);
		qApp->setPalette(QPalette(QColor(panelColor)));
		qApp->setStyleSheet(qss);
		file.close();
	}
}

void utils::getQssColor(const QString &qss, QString &textColor, QString &panelColor, QString &borderColor,
	QString &normalColorStart, QString &normalColorEnd,
	QString &darkColorStart, QString &darkColorEnd, QString &highColor)
{
	QString str = qss;

	QString flagTextColor = "TextColor:";
	int indexTextColor = str.indexOf(flagTextColor);
	if (indexTextColor >= 0) 
	{
		textColor = str.mid(indexTextColor + flagTextColor.length(), 7);
	}

	QString flagPanelColor = "PanelColor:";
	int indexPanelColor = str.indexOf(flagPanelColor);
	if (indexPanelColor >= 0) 
	{
		panelColor = str.mid(indexPanelColor + flagPanelColor.length(), 7);
	}

	QString flagBorderColor = "BorderColor:";
	int indexBorderColor = str.indexOf(flagBorderColor);
	if (indexBorderColor >= 0) 
	{
		borderColor = str.mid(indexBorderColor + flagBorderColor.length(), 7);
	}

	QString flagNormalColorStart = "NormalColorStart:";
	int indexNormalColorStart = str.indexOf(flagNormalColorStart);
	if (indexNormalColorStart >= 0)
	{
		normalColorStart = str.mid(indexNormalColorStart + flagNormalColorStart.length(), 7);
	}

	QString flagNormalColorEnd = "NormalColorEnd:";
	int indexNormalColorEnd = str.indexOf(flagNormalColorEnd);
	if (indexNormalColorEnd >= 0)
	{
		normalColorEnd = str.mid(indexNormalColorEnd + flagNormalColorEnd.length(), 7);
	}

	QString flagDarkColorStart = "DarkColorStart:";
	int indexDarkColorStart = str.indexOf(flagDarkColorStart);
	if (indexDarkColorStart >= 0) 
	{
		darkColorStart = str.mid(indexDarkColorStart + flagDarkColorStart.length(), 7);
	}

	QString flagDarkColorEnd = "DarkColorEnd:";
	int indexDarkColorEnd = str.indexOf(flagDarkColorEnd);
	if (indexDarkColorEnd >= 0) 
	{
		darkColorEnd = str.mid(indexDarkColorEnd + flagDarkColorEnd.length(), 7);
	}

	QString flagHighColor = "HighColor:";
	int indexHighColor = str.indexOf(flagHighColor);
	if (indexHighColor >= 0) 
	{
		highColor = str.mid(indexHighColor + flagHighColor.length(), 7);
	}
}

void utils::setFormInCenter(QWidget *frm)
{
	int frmX = frm->width();
	int frmY = frm->height();
	QDesktopWidget w;
	int deskWidth = w.availableGeometry().width();
	int deskHeight = w.availableGeometry().height();
	QPoint movePoint(deskWidth / 2 - frmX / 2, deskHeight / 2 - frmY / 2);
	frm->move(movePoint);
}

void utils::centerParent(QWidget* parent, QWidget* child)
{
	QPoint centerparent(
		parent->x() + ((parent->frameGeometry().width() - child->frameGeometry().width()) / 2),
		parent->y() + ((parent->frameGeometry().height() - child->frameGeometry().height()) / 2));

	QDesktopWidget * pDesktop = QApplication::desktop();
	QRect sgRect = pDesktop->screenGeometry(pDesktop->screenNumber(parent));
	QRect childFrame = child->frameGeometry();

	if (centerparent.x() < sgRect.left())
		centerparent.setX(sgRect.left());
	else if ((centerparent.x() + childFrame.width()) > sgRect.right())
		centerparent.setX(sgRect.right() - childFrame.width());

	if (centerparent.y() < sgRect.top())
		centerparent.setY(sgRect.top());
	else if ((centerparent.y() + childFrame.height()) > sgRect.bottom())
		centerparent.setY(sgRect.bottom() - childFrame.height());

	child->move(centerparent);
}

void utils::setTranslator(const QString &qmFile)
{
	QTranslator *translator = new QTranslator(qApp);
	translator->load(qmFile);
	qApp->installTranslator(translator);
}

void utils::setCode()
{
#if (QT_VERSION <= QT_VERSION_CHECK(5,0,0))
#if _MSC_VER
	QTextCodec *codec = QTextCodec::codecForName("gbk");
#else
	QTextCodec *codec = QTextCodec::codecForName("utf-8");
#endif
	QTextCodec::setCodecForLocale(codec);
	QTextCodec::setCodecForCStrings(codec);
	QTextCodec::setCodecForTr(codec);
#else
	QTextCodec *codec = QTextCodec::codecForName("utf-8");
	QTextCodec::setCodecForLocale(codec);
#endif
}

void utils::sleep(int sec)
{
	QTime dieTime = QTime::currentTime().addMSecs(sec);
	while (QTime::currentTime() < dieTime) 
	{
		QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
	}
}

void utils::setSystemDateTime(const QString &year, const QString &month, const QString &day, const QString &hour, const QString &min, const QString &sec)
{
#ifdef Q_OS_WIN
	QProcess p(0);
	p.start("cmd");
	p.waitForStarted();
	p.write(QString("date %1-%2-%3\n").arg(year).arg(month).arg(day).toLatin1());
	p.closeWriteChannel();
	p.waitForFinished(1000);
	p.close();
	p.start("cmd");
	p.waitForStarted();
	p.write(QString("time %1:%2:%3.00\n").arg(hour).arg(min).arg(sec).toLatin1());
	p.closeWriteChannel();
	p.waitForFinished(1000);
	p.close();
#else
	QString cmd = QString("date %1%2%3%4%5.%6").arg(month).arg(day).arg(hour).arg(min).arg(year).arg(sec);
	system(cmd.toLatin1());
	system("hwclock -w");
#endif
}

void utils::runWithSystem(const QString &strName, const QString &strPath, bool autoRun)
{
#ifdef Q_OS_WIN
	QSettings reg("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);

	if (autoRun) 
	{
		reg.setValue(strName, strPath);
	}
	else 
	{
		reg.setValue(strName, "");
	}
#endif
}

bool utils::isIP(const QString &ip)
{
	QRegExp RegExp("((2[0-4]\\d|25[0-5]|[01]?\\d\\d?)\\.){3}(2[0-4]\\d|25[0-5]|[01]?\\d\\d?)");
	return RegExp.exactMatch(ip);
}

bool utils::isMac(const QString &mac)
{
	QRegExp RegExp("^[A-F0-9]{2}(-[A-F0-9]{2}){5}$");
	return RegExp.exactMatch(mac);
}

bool utils::isTel(const QString &tel)
{
	if (tel.length() != 11) 
	{
		return false;
	}

	if (!tel.startsWith("13") && !tel.startsWith("14") && !tel.startsWith("15") && !tel.startsWith("18")) 
	{
		return false;
	}

	return true;
}

bool utils::isEmail(const QString &email)
{
	if (!email.contains("@") || !email.contains(".com")) 
	{
		return false;
	}

	return true;
}

int utils::strHexToDecimal(const QString &strHex)
{
	bool ok;
	return strHex.toInt(&ok, 16);
}

int utils::strDecimalToDecimal(const QString &strDecimal)
{
	bool ok;
	return strDecimal.toInt(&ok, 10);
}

int utils::strBinToDecimal(const QString &strBin)
{
	bool ok;
	return strBin.toInt(&ok, 2);
}

QString utils::strHexToStrBin(const QString &strHex)
{
	uchar decimal = strHexToDecimal(strHex);
	QString bin = QString::number(decimal, 2);
	uchar len = bin.length();

	if (len < 8) 
	{
		for (int i = 0; i < 8 - len; i++) 
		{
			bin = "0" + bin;
		}
	}

	return bin;
}

QString utils::decimalToStrBin1(int decimal)
{
	QString bin = QString::number(decimal, 2);
	uchar len = bin.length();

	if (len <= 8) 
	{
		for (int i = 0; i < 8 - len; i++) 
		{
			bin = "0" + bin;
		}
	}

	return bin;
}

QString utils::decimalToStrBin2(int decimal)
{
	QString bin = QString::number(decimal, 2);
	uchar len = bin.length();

	if (len <= 16) 
	{
		for (int i = 0; i < 16 - len; i++) 
		{
			bin = "0" + bin;
		}
	}

	return bin;
}

QString utils::decimalToStrHex(int decimal)
{
	QString temp = QString::number(decimal, 16);

	if (temp.length() == 1) 
	{
		temp = "0" + temp;
	}

	return temp;
}

QByteArray utils::intToByte(int i)
{
	QByteArray result;
	result.resize(4);
	result[3] = (uchar)(0x000000ff & i);
	result[2] = (uchar)((0x0000ff00 & i) >> 8);
	result[1] = (uchar)((0x00ff0000 & i) >> 16);
	result[0] = (uchar)((0xff000000 & i) >> 24);
	return result;
}

int utils::byteToInt(const QByteArray &data)
{
	int i = data.at(3) & 0x000000FF;
	i |= ((data.at(2) << 8) & 0x0000FF00);
	i |= ((data.at(1) << 16) & 0x00FF0000);
	i |= ((data.at(0) << 24) & 0xFF000000);
	return i;
}

QByteArray utils::ushortToByte(ushort i)
{
	QByteArray result;
	result.resize(2);
	result[1] = (uchar)(0x000000ff & i);
	result[0] = (uchar)((0x0000ff00 & i) >> 8);
	return result;
}

int utils::byteToUShort(const QByteArray &data)
{
	int i = data.at(1) & 0x000000FF;
	i |= ((data.at(0) << 8) & 0x0000FF00);
	return i;
}

QString utils::getXorEncryptDecrypt(const QString &str, char key)
{
	QByteArray data = str.toLatin1();
	int size = data.size();

	for (int i = 0; i < size; i++) 
	{
		data[i] = data.at(i) ^ key;
	}

	return QLatin1String(data);
}

uchar utils::getOrCode(const QByteArray &data)
{
	int len = data.length();
	uchar result = 0;

	for (int i = 0; i < len; i++) 
	{
		result ^= data.at(i);
	}

	return result;
}

uchar utils::getCheckCode(const QByteArray &data)
{
	int len = data.length();
	uchar temp = 0;

	for (uchar i = 0; i < len; i++) 
	{
		temp += data.at(i);
	}

	return temp % 256;
}

QString utils::byteArrayToAsciiStr(const QByteArray &data)
{
	QString temp;
	int len = data.size();

	for (int i = 0; i < len; i++) 
	{
		//0x20为空格,空格以下都是不可见字符
		char b = data.at(i);

		if (0x00 == b) 
		{
			temp += QString("\\NUL");
		}
		else if (0x01 == b) 
		{
			temp += QString("\\SOH");
		}
		else if (0x02 == b) 
		{
			temp += QString("\\STX");
		}
		else if (0x03 == b) 
		{
			temp += QString("\\ETX");
		}
		else if (0x04 == b) 
		{
			temp += QString("\\EOT");
		}
		else if (0x05 == b) 
		{
			temp += QString("\\ENQ");
		}
		else if (0x06 == b)
		{
			temp += QString("\\ACK");
		}
		else if (0x07 == b) 
		{
			temp += QString("\\BEL");
		}
		else if (0x08 == b) 
		{
			temp += QString("\\BS");
		}
		else if (0x09 == b) 
		{
			temp += QString("\\HT");
		}
		else if (0x0A == b) 
		{
			temp += QString("\\LF");
		}
		else if (0x0B == b) 
		{
			temp += QString("\\VT");
		}
		else if (0x0C == b) 
		{
			temp += QString("\\FF");
		}
		else if (0x0D == b) 
		{
			temp += QString("\\CR");
		}
		else if (0x0E == b) 
		{
			temp += QString("\\SO");
		}
		else if (0x0F == b) 
		{
			temp += QString("\\SI");
		}
		else if (0x10 == b) 
		{
			temp += QString("\\DLE");
		}
		else if (0x11 == b)
		{
			temp += QString("\\DC1");
		}
		else if (0x12 == b) 
		{
			temp += QString("\\DC2");
		}
		else if (0x13 == b)
		{
			temp += QString("\\DC3");
		}
		else if (0x14 == b)
		{
			temp += QString("\\DC4");
		}
		else if (0x15 == b) 
		{
			temp += QString("\\NAK");
		}
		else if (0x16 == b)
		{
			temp += QString("\\SYN");
		}
		else if (0x17 == b)
		{
			temp += QString("\\ETB");
		}
		else if (0x18 == b) 
		{
			temp += QString("\\CAN");
		}
		else if (0x19 == b) 
		{
			temp += QString("\\EM");
		}
		else if (0x1A == b)
		{
			temp += QString("\\SUB");
		}
		else if (0x1B == b)
		{
			temp += QString("\\ESC");
		}
		else if (0x1C == b)
		{
			temp += QString("\\FS");
		}
		else if (0x1D == b)
		{
			temp += QString("\\GS");
		}
		else if (0x1E == b) 
		{
			temp += QString("\\RS");
		}
		else if (0x1F == b) 
		{
			temp += QString("\\US");
		}
		else if (0x7F == b)
		{
			temp += QString("\\x7F");
		}
		else if (0x5C == b)
		{
			temp += QString("\\x5C");
		}
		else if (0x20 >= b) 
		{
			temp += QString("\\x%1").arg(decimalToStrHex((quint8)b));
		}
		else 
		{
			temp += QString("%1").arg(b);
		}
	}

	return temp.trimmed();
}

QByteArray utils::hexStrToByteArray(const QString &str)
{
	QByteArray senddata;
	int hexdata, lowhexdata;
	int hexdatalen = 0;
	int len = str.length();
	senddata.resize(len / 2);
	char lstr, hstr;

	for (int i = 0; i < len;) 
	{
		hstr = str.at(i).toLatin1();

		if (hstr == ' ') 
		{
			i++;
			continue;
		}

		i++;

		if (i >= len) 
		{
			break;
		}

		lstr = str.at(i).toLatin1();
		hexdata = convertHexChar(hstr);
		lowhexdata = convertHexChar(lstr);

		if ((hexdata == 16) || (lowhexdata == 16)) 
		{
			break;
		}
		else 
		{
			hexdata = hexdata * 16 + lowhexdata;
		}

		i++;
		senddata[hexdatalen] = (char)hexdata;
		hexdatalen++;
	}

	senddata.resize(hexdatalen);
	return senddata;
}

char utils::convertHexChar(char ch)
{
	if ((ch >= '0') && (ch <= '9')) 
	{
		return ch - 0x30;
	}
	else if ((ch >= 'A') && (ch <= 'F')) 
	{
		return ch - 'A' + 10;
	}
	else if ((ch >= 'a') && (ch <= 'f')) 
	{
		return ch - 'a' + 10;
	}
	else 
	{
		return (-1);
	}
}

QByteArray utils::asciiStrToByteArray(const QString &str)
{
	QByteArray buffer;
	int len = str.length();
	QString letter;
	QString hex;

	for (int i = 0; i < len; i++) 
	{
		letter = str.at(i);

		if (letter == "\\") 
		{
			i++;
			letter = str.mid(i, 1);

			if (letter == "x") 
			{
				i++;
				hex = str.mid(i, 2);
				buffer.append(strHexToDecimal(hex));
				i++;
				continue;
			}
			else if (letter == "N") 
			{
				i++;
				hex = str.mid(i, 1);

				if (hex == "U") 
				{
					i++;
					hex = str.mid(i, 1);

					if (hex == "L") 
					{           //NUL=0x00
						buffer.append((char)0x00);
						continue;
					}
				}
				else if (hex == "A") 
				{
					i++;
					hex = str.mid(i, 1);

					if (hex == "K") 
					{           //NAK=0x15
						buffer.append(0x15);
						continue;
					}
				}
			}
			else if (letter == "S") 
			{
				i++;
				hex = str.mid(i, 1);

				if (hex == "O") 
				{
					i++;
					hex = str.mid(i, 1);

					if (hex == "H") 
					{           //SOH=0x01
						buffer.append(0x01);
						continue;
					}
					else 
					{                    //SO=0x0E
						buffer.append(0x0E);
						i--;
						continue;
					}
				}
				else if (hex == "T") 
				{
					i++;
					hex = str.mid(i, 1);

					if (hex == "X") 
					{           //STX=0x02
						buffer.append(0x02);
						continue;
					}
				}
				else if (hex == "I")
				{        //SI=0x0F
					buffer.append(0x0F);
					continue;
				}
				else if (hex == "Y") 
				{
					i++;
					hex = str.mid(i, 1);

					if (hex == "N") 
					{           //SYN=0x16
						buffer.append(0x16);
						continue;
					}
				}
				else if (hex == "U") 
				{
					i++;
					hex = str.mid(i, 1);

					if (hex == "B") 
					{           //SUB=0x1A
						buffer.append(0x1A);
						continue;
					}
				}
			}
			else if (letter == "E") 
			{
				i++;
				hex = str.mid(i, 1);

				if (hex == "T") 
				{
					i++;
					hex = str.mid(i, 1);

					if (hex == "X")
					{           //ETX=0x03
						buffer.append(0x03);
						continue;
					}
					else if (hex == "B")
					{    //ETB=0x17
						buffer.append(0x17);
						continue;
					}
				}
				else if (hex == "O") 
				{
					i++;
					hex = str.mid(i, 1);

					if (hex == "T") 
					{           //EOT=0x04
						buffer.append(0x04);
						continue;
					}
				}
				else if (hex == "N") 
				{
					i++;
					hex = str.mid(i, 1);

					if (hex == "Q") 
					{           //ENQ=0x05
						buffer.append(0x05);
						continue;
					}
				}
				else if (hex == "M") {        //EM=0x19
					buffer.append(0x19);
					continue;
				}
				else if (hex == "S") 
				{
					i++;
					hex = str.mid(i, 1);

					if (hex == "C") 
					{           //ESC=0x1B
						buffer.append(0x1B);
						continue;
					}
				}
			}
			else if (letter == "A") 
			{
				i++;
				hex = str.mid(i, 1);

				if (hex == "C") 
				{
					i++;
					hex = str.mid(i, 1);

					if (hex == "K") 
					{           //ACK=0x06
						buffer.append(0x06);
						continue;
					}
				}
			}
			else if (letter == "B") 
			{
				i++;
				hex = str.mid(i, 1);

				if (hex == "E") 
				{
					i++;
					hex = str.mid(i, 1);

					if (hex == "L") 
					{           //BEL=0x07
						buffer.append(0x07);
						continue;
					}
				}
				else if (hex == "S") 
				{        //BS=0x08
					buffer.append(0x08);
					continue;
				}
			}
			else if (letter == "C") 
			{
				i++;
				hex = str.mid(i, 1);

				if (hex == "R") 
				{               //CR=0x0D
					buffer.append(0x0D);
					continue;
				}
				else if (hex == "A")
				{
					i++;
					hex = str.mid(i, 1);

					if (hex == "N") 
					{           //CAN=0x18
						buffer.append(0x18);
						continue;
					}
				}
			}
			else if (letter == "D")
			{
				i++;
				hex = str.mid(i, 1);

				if (hex == "L") 
				{
					i++;
					hex = str.mid(i, 1);

					if (hex == "E") 
					{           //DLE=0x10
						buffer.append(0x10);
						continue;
					}
				}
				else if (hex == "C")
				{
					i++;
					hex = str.mid(i, 1);

					if (hex == "1") 
					{           //DC1=0x11
						buffer.append(0x11);
						continue;
					}
					else if (hex == "2") 
					{    //DC2=0x12
						buffer.append(0x12);
						continue;
					}
					else if (hex == "3")
					{    //DC3=0x13
						buffer.append(0x13);
						continue;
					}
					else if (hex == "4") 
					{    //DC2=0x14
						buffer.append(0x14);
						continue;
					}
				}
			}
			else if (letter == "F") 
			{
				i++;
				hex = str.mid(i, 1);

				if (hex == "F")
				{               //FF=0x0C
					buffer.append(0x0C);
					continue;
				}
				else if (hex == "S") 
				{        //FS=0x1C
					buffer.append(0x1C);
					continue;
				}
			}
			else if (letter == "H")
			{
				i++;
				hex = str.mid(i, 1);

				if (hex == "T")
				{               //HT=0x09
					buffer.append(0x09);
					continue;
				}
			}
			else if (letter == "L")
			{
				i++;
				hex = str.mid(i, 1);

				if (hex == "F") 
				{               //LF=0x0A
					buffer.append(0x0A);
					continue;
				}
			}
			else if (letter == "G")
			{
				i++;
				hex = str.mid(i, 1);

				if (hex == "S") 
				{               //GS=0x1D
					buffer.append(0x1D);
					continue;
				}
			}
			else if (letter == "R") 
			{
				i++;
				hex = str.mid(i, 1);

				if (hex == "S") 
				{               //RS=0x1E
					buffer.append(0x1E);
					continue;
				}
			}
			else if (letter == "U")
			{
				i++;
				hex = str.mid(i, 1);

				if (hex == "S") 
				{               //US=0x1F
					buffer.append(0x1F);
					continue;
				}
			}
			else if (letter == "V") 
			{
				i++;
				hex = str.mid(i, 1);

				if (hex == "T") 
				{               //VT=0x0B
					buffer.append(0x0B);
					continue;
				}
			}
			else if (letter == "\\") 
			{
				//如果连着的是多个\\则对应添加\对应的16进制0x5C
				buffer.append(0x5C);
				continue;
			}
			else 
			{
				//将对应的\[前面的\\也要加入
				buffer.append(0x5C);
				buffer.append(letter.toLatin1());
				continue;
			}
		}

		buffer.append(str.mid(i, 1).toLatin1());
	}

	return buffer;
}

QString utils::byteArrayToHexStr(const QByteArray &data)
{
	QString temp = "";
	QString hex = data.toHex();

	for (int i = 0; i < hex.length(); i = i + 2)
	{
		temp += hex.mid(i, 2) + " ";
	}

	return temp.trimmed().toUpper();
}

QString utils::getFileName(const QString &filter, QString defaultDir)
{
	return QFileDialog::getOpenFileName(0, "选择文件", defaultDir, filter);
}

QStringList utils::getFileNames(const QString &filter, QString defaultDir)
{
	return QFileDialog::getOpenFileNames(0, "选择文件", defaultDir, filter);
}

QString utils::getFolderName()
{
	return QFileDialog::getExistingDirectory();;
}

QString utils::getFileSuffix(const QString &strFilePath)
{
	QFileInfo fileInfo(strFilePath);
	return fileInfo.suffix();
}

QString utils::getFilePath(const QString &strFilePath)
{
	QFileInfo fileInfo(strFilePath);
	return fileInfo.path();
}

QString utils::getFileNameWithNotExtension(const QString &strFilePath)
{
	QFileInfo fileInfo(strFilePath);
	return fileInfo.baseName();
}

QString utils::getCompleteFileNameWithNotExtension(const QString &strFilePath)
{
	QFileInfo fileInfo(strFilePath);
	return fileInfo.completeBaseName();
}

QString utils::getFileNameWithExtension(const QString &strFilePath)
{
	QFileInfo fileInfo(strFilePath);
	return fileInfo.fileName();
}

QStringList utils::getFolderFileNames(const QStringList &filter)
{
	QStringList fileList;
	QString strFolder = QFileDialog::getExistingDirectory();

	if (strFolder.length() != 0) 
	{
		QDir myFolder(strFolder);
		if (myFolder.exists()) 
		{
			fileList = myFolder.entryList(filter);
		}
	}

	return fileList;
}

bool utils::folderIsExist(const QString &strFolder)
{
	QDir tempFolder(strFolder);
	return tempFolder.exists();
}

bool utils::fileIsExist(const QString &strFile)
{
	QFile tempFile(strFile);
	return tempFile.exists();
}

bool utils::copyFile(const QString &sourceFile, const QString &targetFile)
{
	bool ok;
	ok = QFile::copy(sourceFile, targetFile);
	// 将复制过去的文件只读属性取消
	ok = QFile::setPermissions(targetFile, QFile::WriteOwner);
	return ok;
}

void utils::deleteDirectory(const QString &path)
{
	QDir dir(path);

	if (!dir.exists()) 
	{
		return;
	}

	dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
	QFileInfoList fileList = dir.entryInfoList();

	foreach(QFileInfo fi, fileList) 
	{
		if (fi.isFile())
		{
			fi.dir().remove(fi.fileName());
		}
		else 
		{
			deleteDirectory(fi.absoluteFilePath());
			dir.rmdir(fi.absoluteFilePath());
		}
	}
}

bool utils::ipLive(const QString &ip, int port, int timeout)
{
	QTcpSocket tcpClient;
	tcpClient.abort();
	tcpClient.connectToHost(ip, port);
	//超时没有连接上则判断不在线
	return tcpClient.waitForConnected(timeout);
}

QString utils::getHtml(const QString &url)
{
	QNetworkAccessManager *manager = new QNetworkAccessManager();
	QNetworkReply *reply = manager->get(QNetworkRequest(QUrl(url)));
	QByteArray responseData;
	QEventLoop eventLoop;
	QObject::connect(manager, SIGNAL(finished(QNetworkReply *)), &eventLoop, SLOT(quit()));
	eventLoop.exec();
	responseData = reply->readAll();
	return QString(responseData);
}

QString utils::getNetIP(const QString &webCode)
{
	QString web = webCode;
	web = web.replace(' ', "");
	web = web.replace("\r", "");
	web = web.replace("\n", "");
	QStringList list = web.split("<br/>");
	QString tar = list.at(3);
	QStringList ip = tar.split("=");
	return ip.at(1);
}

QString utils::getLocalIP()
{
	QString ip;
	QList<QHostAddress> addrs = QNetworkInterface::allAddresses();

	foreach(QHostAddress addr, addrs) 
	{
		ip = addr.toString();

		if (ip != "127.0.0.1")
		{
			break;
		}
	}

	return ip;
}

QString utils::urlToIP(const QString &url)
{
	QHostInfo host = QHostInfo::fromName(url);
	return host.addresses().at(0).toString();
}

bool utils::isWebOk()
{
	// 接通百度IP
	return ipLive("115.239.211.112", 80);
}

int utils::assumeSingleInstance(const char* program)
{
	static QSharedMemory shm(program);
	if (shm.create(100) == false)
	{
		return -1;
	}

	return 0;
}

int utils::runProgramCount()
{
	QString filename = utils::appPath() + "/runCount.ini";
	QString key = "count";
	QSettings settings(filename, QSettings::IniFormat);
	settings.setIniCodec(QTextCodec::codecForName("gb2312"));
	int count = settings.value(key).toInt();
	if (count > 0)
	{
		settings.setValue(key, count+1);
	}
	else
	{
		settings.setValue(key, 1);
	}

	settings.sync();
	qDebug("Run count is %d", count);

	return count;
}

