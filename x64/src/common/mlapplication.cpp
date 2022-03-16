#include "mlapplication.h"
#include "mlexception.h"
#include <vcg/complex/complex.h>

bool MeshLabApplication::notify( QObject * rec, QEvent * ev )
{
	try
	{
		return QApplication::notify(rec, ev);
	}
	catch (MLException& e)
	{
		qCritical("%s", e.what());
	}
	catch (vcg::MissingComponentException &e )
	{
		qCritical("%s", e.what());
		abort();
	}
	catch (...)
	{
		qCritical(qPrintable(QTextCodec::codecForName("GBK")->toUnicode("存在某些异常情况!!!")) /*"Something really bad happened!!!!!!!!!!!!!"*/);
		abort();
	}

	return false;
}
