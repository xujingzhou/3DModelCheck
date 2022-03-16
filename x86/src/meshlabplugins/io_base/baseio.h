
#ifndef BASEIOPLUGIN_H
#define BASEIOPLUGIN_H

#include <common/interfaces.h>

class BaseMeshIOPlugin : public QObject, public MeshIOInterface
{
Q_OBJECT
  MESHLAB_PLUGIN_IID_EXPORTER(MESH_IO_INTERFACE_IID)
  Q_INTERFACES(MeshIOInterface)

  
public:
	
  BaseMeshIOPlugin() : MeshIOInterface() {}

  QList<Format> importFormats() const;
  QList<Format> exportFormats() const;

  void GetExportMaskCapability(QString &format, int &capability, int &defaultBits) const;

  bool open(const QString &formatName, const QString &fileName, MeshModel &m, int& mask,const RichParameterSet & par, vcg::CallBackPos *cb=0, QWidget *parent=0);
  bool save(const QString &formatName, const QString &fileName, MeshModel &m, const int mask, const RichParameterSet & par, vcg::CallBackPos *cb=0, QWidget *parent= 0);
  void initOpenParameter(const QString &format, MeshModel &/*m*/, RichParameterSet & par);
  void applyOpenParameter(const QString &format, MeshModel &m, const RichParameterSet &par);
  void initPreOpenParameter(const QString &formatName, const QString &filename, RichParameterSet &parlst);
  void initSaveParameter(const QString &format, MeshModel &/*m*/, RichParameterSet & par);
};

#endif
