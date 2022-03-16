/**
* @Title: utils
* @Package
* @Description:
* @author Johnny Xu(xujingzhou2016@gmail.com)
* @date 2017/7/28
*/
#ifndef UTILS_H
#define UTILS_H

class utils
{

public:
	static utils* getInstance();
	static void release();

	~utils();

private:
	utils();

public:

	static int assumeSingleInstance(const char* program);
	static int runProgramCount();

	static int deskWidth();
	static int deskHeight();

	static QString appName();
	static QString appPath();

	static void newDir(const QString &dirName);

	static void writeInfo(const QString &info, const QString &filePath = "log");

	static void setStyle(const QString &qssFile);
	static void setStyle(const QString &qssFile, QString &paletteColor, QString &textColor);
	static void setStyle(const QString &qssFile, QString &textColor,
		QString &panelColor, QString &borderColor,
		QString &normalColorStart, QString &normalColorEnd,
		QString &darkColorStart, QString &darkColorEnd,
		QString &highColor);

	static void getQssColor(const QString &qss, QString &textColor,
		QString &panelColor, QString &borderColor,
		QString &normalColorStart, QString &normalColorEnd,
		QString &darkColorStart, QString &darkColorEnd,
		QString &highColor);

	static void setFormInCenter(QWidget *frm);
	static void centerParent(QWidget* parent, QWidget* child);

	static void setTranslator(const QString &qmFile = ":/image/qt_zh_CN.qm");
	static void setCode();
	static void sleep(int sec);

	static void setSystemDateTime(const QString &year, const QString &month, const QString &day,
		const QString &hour, const QString &min, const QString &sec);
	static void runWithSystem(const QString &strName, const QString &strPath, bool autoRun = true);

	static bool isIP(const QString &ip);
	static bool isMac(const QString &mac);
	static bool isTel(const QString &tel);
	static bool isEmail(const QString &email);

	static int strHexToDecimal(const QString &strHex);
	static int strDecimalToDecimal(const QString &strDecimal);
	static int strBinToDecimal(const QString &strBin);
	static QString strHexToStrBin(const QString &strHex);
	static QString decimalToStrBin1(int decimal);
	static QString decimalToStrBin2(int decimal);
	static QString decimalToStrHex(int decimal);

	static QByteArray intToByte(int i);
	static int byteToInt(const QByteArray &data);
	static QByteArray ushortToByte(ushort i);
	static int byteToUShort(const QByteArray &data);

	static QString getXorEncryptDecrypt(const QString &str, char key);
	static uchar getOrCode(const QByteArray &data);
	static uchar getCheckCode(const QByteArray &data);

	static QString byteArrayToAsciiStr(const QByteArray &data);

	static QByteArray hexStrToByteArray(const QString &str);
	static char convertHexChar(char ch);

	static QByteArray asciiStrToByteArray(const QString &str);

	static QString byteArrayToHexStr(const QByteArray &data);

	static QString getFileName(const QString &filter, QString defaultDir = QCoreApplication::applicationDirPath());

	static QStringList getFileNames(const QString &filter, QString defaultDir = QCoreApplication::applicationDirPath());

	static QString getFolderName();

	static QString getFileSuffix(const QString &strFilePath);
	static QString getFilePath(const QString &strFilePath);
	static QString getFileNameWithNotExtension(const QString &strFilePath);
	static QString getCompleteFileNameWithNotExtension(const QString &strFilePath);
	static QString getFileNameWithExtension(const QString &strFilePath);

	static QStringList getFolderFileNames(const QStringList &filter);

	static bool folderIsExist(const QString &strFolder);

	static bool fileIsExist(const QString &strFile);

	static bool copyFile(const QString &sourceFile, const QString &targetFile);

	static void deleteDirectory(const QString &path);

	static bool ipLive(const QString &ip, int port, int timeout = 1000);

	static QString getHtml(const QString &url);

	static QString getNetIP(const QString &webCode);

	static QString getLocalIP();

	static QString urlToIP(const QString &url);

	static bool isWebOk();

private:
	static utils *pInstance;
};

#endif // UTILS_H
