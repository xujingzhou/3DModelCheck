
#include "edit_select_factory.h"
#include "edit_select.h"

EditSelectFactory::EditSelectFactory()
{
	QTextCodec *codec = QTextCodec::codecForName("GBK");
    editSelect = new QAction(QIcon(":/images/select_face.png"), codec->toUnicode("选择面") /*"Select Faces in a rectagular region"*/, this);
//    editSelectConnected = new QAction(QIcon(":/images/select_face_connected.png"), codec->toUnicode("选择连接部分在一个区域") /*"Select Connected Components in a region"*/, this);
    editSelectVert = new QAction(QIcon(":/images/select_vertex.png"), codec->toUnicode("选择点集") /*"Select Vertexes"*/, this);

    actionList << editSelectVert;
    actionList << editSelect;
	// Modified by Johnny Xu, 2017/2/20
//    actionList << editSelectConnected;
	
	foreach(QAction *editAction, actionList)
		editAction->setCheckable(true); 	
}
	
//gets a list of actions available from this plugin
QList<QAction *> EditSelectFactory::actions() const
{
	return actionList;
}

//get the edit tool for the given action
MeshEditInterface* EditSelectFactory::getMeshEditInterface(QAction *action)
{
	if(action == editSelect)
      return new EditSelectPlugin(EditSelectPlugin::SELECT_FACE_MODE);
	else if(action == editSelectConnected)
      return new EditSelectPlugin(EditSelectPlugin::SELECT_CONN_MODE);
	else if(action == editSelectVert)
      return new EditSelectPlugin(EditSelectPlugin::SELECT_VERT_MODE);
	 
	assert(0); //should never be asked for an action that isnt here
}

QString EditSelectFactory::getEditToolDescription(QAction */*a*/)
{
  return EditSelectPlugin::Info();
}

MESHLAB_PLUGIN_NAME_EXPORTER(EditSelectFactory)
