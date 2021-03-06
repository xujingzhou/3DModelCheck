
#include "baseio.h"

#include <wrap/io_trimesh/import_ply.h>
#include <wrap/io_trimesh/import_stl.h>
#include <wrap/io_trimesh/import_obj.h>
#include <wrap/io_trimesh/import_off.h>
#include <wrap/io_trimesh/import_ptx.h>
#include <wrap/io_trimesh/import_vmi.h>
#include <wrap/io_trimesh/import_gts.h>

#include <wrap/io_trimesh/export_ply.h>
#include <wrap/io_trimesh/export_stl.h>
#include <wrap/io_trimesh/export_obj.h>
#include <wrap/io_trimesh/export_vrml.h>
#include <wrap/io_trimesh/export_dxf.h>
#include <wrap/io_trimesh/export_vmi.h>
#include <wrap/io_trimesh/export_gts.h>
#include <wrap/io_trimesh/export.h>

using namespace std;
using namespace vcg;

class PEdge;
class PFace;
class PVertex;
struct PUsedTypes : public UsedTypes<Use<PVertex>   ::AsVertexType,
                                        Use<PEdge>     ::AsEdgeType,
                                        Use<PFace>     ::AsFaceType> 
{
};

class PVertex  : public Vertex<PUsedTypes,  vertex::Coord3f, vertex::Normal3f, vertex::Qualityf, vertex::Color4b, vertex::BitFlags  >
{
};

class PEdge    : public Edge< PUsedTypes, edge::VertexRef, edge::BitFlags>
{
};

class PFace : public vcg::Face<
     PUsedTypes,
    face::PolyInfo, // this is necessary  if you use component in vcg/simplex/face/component_polygon.h
    face::PFVAdj,   // Pointer to the vertices (just like FVAdj )
    face::Color4b,
    face::BitFlags, // bit flags
    face::Normal3f, // normal
    face::WedgeTexCoord2f
> 
{
};

class PMesh    : public tri::TriMesh< vector<PVertex>, vector<PEdge>, vector<PFace>   > 
{
};

// initialize importing parameters
void BaseMeshIOPlugin::initPreOpenParameter(const QString &formatName, const QString &/*filename*/, RichParameterSet &parlst)
{
    if (formatName.toUpper() == tr("PTX"))
    {
        parlst.addParam(new RichInt("meshindex",0,"Index of Range Map to be Imported",
                        "PTX files may contain more than one range map. 0 is the first range map. If the number if higher than the actual mesh number, the import will fail"));
        parlst.addParam(new RichBool("anglecull",true,"Cull faces by angle","short"));
        parlst.addParam(new RichFloat("angle",85.0,"Angle limit for face culling","short"));
        parlst.addParam(new RichBool("usecolor",true,"import color","Read color from PTX, if color is not present, uses reflectance instead"));
        parlst.addParam(new RichBool("pointcull",true,"delete unsampled points","Deletes unsampled points in the grid that are normally located in [0,0,0]"));
        parlst.addParam(new RichBool("pointsonly",true,"Keep only points","Import points a point cloud only, with radius and normals, no triangulation involved, isolated points and points with normals with steep angles are removed."));
        parlst.addParam(new RichBool("switchside",false,"Swap rows/columns","On some PTX, the rows and columns number are switched over"));
        parlst.addParam(new RichBool("flipfaces",false,"Flip all faces","Flip the orientation of all the triangles"));
    }
}

bool BaseMeshIOPlugin::open(const QString &formatName, const QString &fileName, MeshModel &m, int& mask, const RichParameterSet &parlst, CallBackPos *cb, QWidget * /*parent*/)
{
    bool normalsUpdated = false;

    // initializing mask
	mask = 0;

    // initializing progress bar status
    if (cb != NULL)		
		(*cb)(0, qPrintable(QTextCodec::codecForName("GBK")->toUnicode("????????...")) /*"Loading..."*/);

    QString errorMsgFormat = QTextCodec::codecForName("GBK")->toUnicode("??????????????????") + "%1:\n%2"; /*"Error encountered while loading file:\n\"%1\"\n\nError details: %2";*/

    //string filename = fileName.toUtf8().data();
    string filename = QFile::encodeName(fileName).constData ();

	if (formatName.toUpper() == tr("PLY"))
    {
        tri::io::ImporterPLY<CMeshO>::LoadMask(filename.c_str(), mask);
        // small patch to allow the loading of per wedge color into faces.
        if(mask & tri::io::Mask::IOM_WEDGCOLOR) mask |= tri::io::Mask::IOM_FACECOLOR;
        m.Enable(mask);


        int result = tri::io::ImporterPLY<CMeshO>::Open(m.cm, filename.c_str(), mask, cb);
        if (result != 0) // all the importers return 0 on success
        {
            if(tri::io::ImporterPLY<CMeshO>::ErrorCritical(result) )
            {
                errorMessage = errorMsgFormat.arg(fileName, tri::io::ImporterPLY<CMeshO>::ErrorMsg(result));
                return false;
            }
        }
    }
    else if (formatName.toUpper() == tr("STL"))
	{
		if (!tri::io::ImporterSTL<CMeshO>::LoadMask(filename.c_str(), mask))
		{
		  errorMessage = errorMsgFormat.arg(fileName, tri::io::ImporterSTL<CMeshO>::ErrorMsg(tri::io::ImporterSTL<CMeshO>::E_CANTOPEN));
		  return false;
		}

		m.Enable(mask);
		int result = tri::io::ImporterSTL<CMeshO>::Open(m.cm, filename.c_str(), mask, cb);
		if (result != 0) // all the importers return 0 on success
		{
		  errorMessage = errorMsgFormat.arg(fileName, tri::io::ImporterSTL<CMeshO>::ErrorMsg(result));
		  return false;
		}
	 }
	else if( (formatName.toUpper() == tr("OBJ")) || (formatName.toUpper() == tr("QOBJ")) )
    {
		tri::io::ImporterOBJ<CMeshO>::Info oi;
        oi.cb = cb;
        if (!tri::io::ImporterOBJ<CMeshO>::LoadMask(filename.c_str(), oi))
            return false;

		m.Enable(oi.mask);

        int result = tri::io::ImporterOBJ<CMeshO>::Open(m.cm, filename.c_str(), oi);
        if (result != tri::io::ImporterOBJ<CMeshO>::E_NOERROR)
        {
            if (result & tri::io::ImporterOBJ<CMeshO>::E_NON_CRITICAL_ERROR)
                    errorMessage = errorMsgFormat.arg(fileName, tri::io::ImporterOBJ<CMeshO>::ErrorMsg(result));
            else
            {
                errorMessage = errorMsgFormat.arg(fileName, tri::io::ImporterOBJ<CMeshO>::ErrorMsg(result));
                return false;
            }
        }

        if(oi.mask & tri::io::Mask::IOM_WEDGNORMAL)
            normalsUpdated = true;

        m.Enable(oi.mask);
        if(m.hasDataMask(MeshModel::MM_POLYGONAL)) 
			qDebug("Mesh is Polygonal!");
        mask = oi.mask;
    }
    else if (formatName.toUpper() == tr("PTX"))
    {
        tri::io::ImporterPTX<CMeshO>::Info importparams;

		importparams.meshnum = parlst.getInt("meshindex");
		importparams.anglecull = parlst.getBool("anglecull");
		importparams.angle = parlst.getFloat("angle");
		importparams.savecolor = parlst.getBool("usecolor");
		importparams.pointcull = parlst.getBool("pointcull");
		importparams.pointsonly = parlst.getBool("pointsonly");
		importparams.switchside = parlst.getBool("switchside");
		importparams.flipfaces = parlst.getBool("flipfaces");

        // if color, add to mesh
        if(importparams.savecolor)
            importparams.mask |= tri::io::Mask::IOM_VERTCOLOR;
		if(importparams.pointsonly)
		  importparams.mask |= tri::io::Mask::IOM_VERTRADIUS;

        // reflectance is stored in quality
        importparams.mask |= tri::io::Mask::IOM_VERTQUALITY;

        m.Enable(importparams.mask);

        int result = tri::io::ImporterPTX<CMeshO>::Open(m.cm, filename.c_str(), importparams, cb);
        if (result == 1)
        {
            errorMessage = errorMsgFormat.arg(fileName, tri::io::ImporterPTX<CMeshO>::ErrorMsg(result));
            return false;
        }

        // update mask
        mask = importparams.mask;
    }
    else if (formatName.toUpper() == tr("OFF"))
    {
        int loadMask;
        if (!tri::io::ImporterOFF<CMeshO>::LoadMask(filename.c_str(),loadMask))
        {
          errorMessage = errorMsgFormat.arg(fileName, tri::io::ImporterOFF<CMeshO>::ErrorMsg(tri::io::ImporterOFF<CMeshO>::InvalidFile));
          return false;
        }
        m.Enable(loadMask);

        int result = tri::io::ImporterOFF<CMeshO>::Open(m.cm, filename.c_str(), mask, cb);
        if (result != 0)  // OFFCodes enum is protected
        {
            errorMessage = errorMsgFormat.arg(fileName, tri::io::ImporterOFF<CMeshO>::ErrorMsg(result));
            return false;
        }
    }
    else if (formatName.toUpper() == tr("VMI"))
    {
        int loadMask;
        if (!tri::io::ImporterVMI<CMeshO>::LoadMask(filename.c_str(),loadMask))
            return false;
        m.Enable(loadMask);

        int result = tri::io::ImporterVMI<CMeshO>::Open(m.cm, filename.c_str(), mask, cb);
        if (result != 0)
        {
            errorMessage = errorMsgFormat.arg(fileName, tri::io::ImporterOFF<CMeshO>::ErrorMsg(result));
            return false;
        }
    }
    else if (formatName.toUpper() == tr("GTS"))
	{
		int loadMask;
		if (!tri::io::ImporterGTS<CMeshO>::LoadMask(filename.c_str(),loadMask))
			return false;
		m.Enable(loadMask);

		tri::io::ImporterGTS<CMeshO>::Options opt;
		opt.flipFaces = true;

		int result = tri::io::ImporterGTS<CMeshO>::Open(m.cm, filename.c_str(), mask, opt, cb);
		if (result != 0)
		{
			errorMessage = errorMsgFormat.arg(fileName, vcg::tri::io::ImporterGTS<CMeshO>::ErrorMsg(result));
			return false;
		}
	}
    else
	{
		assert(0); // Unknown File type
		return false;
	}

    // verify if texture files are present
    QString missingTextureFilesMsg = "The following texture files were not found:\n";
    bool someTextureNotFound = false;
    for ( unsigned textureIdx = 0; textureIdx < m.cm.textures.size(); ++textureIdx)
    {
		if (!QFile::exists(m.cm.textures[textureIdx].c_str()))
        {
            missingTextureFilesMsg.append("\n");
            missingTextureFilesMsg.append(m.cm.textures[textureIdx].c_str());
            someTextureNotFound = true;
        }
    }

    if (someTextureNotFound)
        Log("??????????????: %s" /*"Missing texture files: %s"*/, qPrintable(missingTextureFilesMsg));

    if (cb != NULL)	
		(*cb)(99, qPrintable(QTextCodec::codecForName("GBK")->toUnicode("??????????")) /*"Done"*/);

    return true;
}

bool BaseMeshIOPlugin::save(const QString &formatName,const QString &fileName, MeshModel &m, const int mask, const RichParameterSet & par, CallBackPos *cb, QWidget */*parent*/)
{
    QString errorMsgFormat = QTextCodec::codecForName("GBK")->toUnicode("??????????????????") + "%1:\n%2"; /*"Error encountered while exportering file %1:\n%2";*/

	string filename = QFile::encodeName(fileName).constData ();
	//string filename = fileName.toUtf8().data();
    string ex = formatName.toUtf8().data();
    bool binaryFlag = false;
    if(formatName.toUpper() == tr("STL") || formatName.toUpper() == tr("PLY"))
		binaryFlag = par.findParameter("Binary")->val->getBool();

    if(formatName.toUpper() == tr("PLY"))
    {
        int result = tri::io::ExporterPLY<CMeshO>::Save(m.cm, filename.c_str(), mask, binaryFlag, cb);
        if(result!=0)
        {
            errorMessage = errorMsgFormat.arg(fileName, tri::io::ExporterPLY<CMeshO>::ErrorMsg(result));
            return false;
        }

        return true;
    }

    if(formatName.toUpper() == tr("STL"))
    {
      bool magicsFlag =  par.getBool("ColorMode");

        int result = tri::io::ExporterSTL<CMeshO>::Save(m.cm, filename.c_str(), binaryFlag, mask, "STL", magicsFlag);
        if(result!=0)
        {
            errorMessage = errorMsgFormat.arg(fileName, tri::io::ExporterSTL<CMeshO>::ErrorMsg(result));
            return false;
        }

        return true;
    }

    if(formatName.toUpper() == tr("WRL"))
    {
        int result = tri::io::ExporterWRL<CMeshO>::Save(m.cm,filename.c_str(),mask,cb);
        if(result!=0)
        {
            errorMessage = errorMsgFormat.arg(fileName, tri::io::ExporterWRL<CMeshO>::ErrorMsg(result));
            return false;
        }

        return true;
    }

    if( formatName.toUpper() == tr("OFF"))
    {
		if(mask && tri::io::Mask::IOM_BITPOLYGONAL)
			m.updateDataMask(MeshModel::MM_FACEFACETOPO);

		int result = tri::io::Exporter<CMeshO>::Save(m.cm,filename.c_str(),mask,cb);
		if(result!=0)
		{
			errorMessage = errorMsgFormat.arg(fileName, tri::io::Exporter<CMeshO>::ErrorMsg(result));
			return false;
		}

		return true;
    }

	if( formatName.toUpper() == tr("OBJ"))
	{
		tri::Allocator<CMeshO>::CompactEveryVector(m.cm);
		int result;
		if(mask & tri::io::Mask::IOM_BITPOLYGONAL)
		{
			m.updateDataMask(MeshModel::MM_FACEFACETOPO);
			PMesh pm;
			tri::PolygonSupport<CMeshO,PMesh>::ImportFromTriMesh(pm, m.cm);
			result = tri::io::ExporterOBJ<PMesh>::Save(pm, filename.c_str(), mask, cb);
		}
		else
			result = tri::io::ExporterOBJ<CMeshO>::Save(m.cm, filename.c_str(), mask, cb);

		if(result!=0)
		{
			errorMessage = errorMsgFormat.arg(fileName, tri::io::Exporter<CMeshO>::ErrorMsg(result));
			return false;
		}

		return true;
	}

    if( formatName.toUpper() == tr("DXF") )
	{
        int result = tri::io::Exporter<CMeshO>::Save(m.cm,filename.c_str(),mask,cb);
		if(result!=0)
		{
            errorMessage = errorMsgFormat.arg(fileName, tri::io::Exporter<CMeshO>::ErrorMsg(result));
			return false;
		}
		return true;
	}

    if( formatName.toUpper() == tr("GTS") )
    {
        int result = vcg::tri::io::ExporterGTS<CMeshO>::Save(m.cm,filename.c_str(),mask);
        if(result!=0)
        {
            errorMessage =  errorMsgFormat.arg(fileName, vcg::tri::io::ExporterGTS<CMeshO>::ErrorMsg(result));
            return false;
        }
        return true;
    }

	assert(0); // unknown format
    return false;
}

/*
    returns the list of the file's type which can be imported
*/
QList<MeshIOInterface::Format> BaseMeshIOPlugin::importFormats() const
{
    QList<Format> formatList;
    formatList << Format(""	, tr("PLY"));
    formatList << Format(""				      , tr("STL"));
    formatList << Format(""				, tr("OBJ"));

	//modify by Bangui Ruan
//     formatList << Format("Quad Object"				,						tr("QOBJ"));
//     formatList << Format("Object File Format"						, tr("OFF"));
//     formatList << Format("PTX File Format"							, tr("PTX"));
//     formatList << Format("VCG Dump File Format"							, tr("VMI"));

    return formatList;
}

/*
    returns the list of the file's type which can be exported
*/
QList<MeshIOInterface::Format> BaseMeshIOPlugin::exportFormats() const
{
    QList<Format> formatList;
    formatList << Format("", tr("PLY"));
    formatList << Format("",              tr("STL"));
	formatList << Format("",       tr("OBJ"));
	//modify by Bangui Ruan
//     formatList << Format("Object File Format",           tr("OFF"));
//     formatList << Format("VRML File Format",             tr("WRL"));
//     formatList << Format("DXF File Format",              tr("DXF"));

    return formatList;
}

/*
    returns the mask on the basis of the file's type.
    otherwise it returns 0 if the file format is unknown
*/
void BaseMeshIOPlugin::GetExportMaskCapability(QString &format, int &capability, int &defaultBits) const
{
    if(format.toUpper() == tr("PLY"))
	{
        capability = tri::io::ExporterPLY<CMeshO>::GetExportMaskCapability();
        // For the default bits of the ply format disable flags and normals that usually are not useful.
        defaultBits = capability;

		// Fix bug #21 by Johnny Xu, 2017/2/28
        //defaultBits &= (~tri::io::Mask::IOM_FLAGS);
        //defaultBits &= (~tri::io::Mask::IOM_VERTNORMAL);
    }

    if(format.toUpper() == tr("STL"))
	{
        capability = tri::io::ExporterSTL<CMeshO>::GetExportMaskCapability();
        defaultBits = capability;
    }

    if(format.toUpper() == tr("OBJ"))
	{
		capability = defaultBits= tri::io::ExporterOBJ<CMeshO>::GetExportMaskCapability() | tri::io::Mask::IOM_BITPOLYGONAL;
    }

    if(format.toUpper() == tr("OFF"))
	{
		capability = defaultBits= tri::io::ExporterOFF<CMeshO>::GetExportMaskCapability();
	}

    if(format.toUpper() == tr("WRL"))
	{
		capability = defaultBits = tri::io::ExporterWRL<CMeshO>::GetExportMaskCapability();
	}
}

void BaseMeshIOPlugin::initOpenParameter(const QString &format, MeshModel &/*m*/, RichParameterSet &par)
{
    if(format.toUpper() == tr("STL"))
        par.addParam(new RichBool("Unify", true, QTextCodec::codecForName("GBK")->toUnicode("????????????"), "" 
		                        /* "Unify Duplicated Vertices",
                                "The STL format is not an vertex-indexed format. Each triangle is composed by independent vertices, so, usually, duplicated vertices should be unified"*/));
}

void BaseMeshIOPlugin::initSaveParameter(const QString &format, MeshModel &/*m*/, RichParameterSet &par)
{
    if(format.toUpper() == tr("STL") || format.toUpper() == tr("PLY"))
        par.addParam(new RichBool("Binary", true, QTextCodec::codecForName("GBK")->toUnicode("??????"), ""
		                        /* "Binary encoding",
                                "Save the mesh using a binary encoding. If false the mesh is saved in a plain, readable ascii format"*/));

    if(format.toUpper() == tr("STL") )
		par.addParam(new RichBool("ColorMode", true, QTextCodec::codecForName("GBK")->toUnicode("??????????????"), "" 
	                          /* "Materialise Color Encoding",
                              "Save the color using a binary encoding according to the Materialise's Magic style (e.g. RGB coding instead of BGR coding)"*/));
}

void BaseMeshIOPlugin::applyOpenParameter(const QString &format, MeshModel &m, const RichParameterSet &par)
{
    if(format.toUpper() == tr("STL"))
		if (par.findParameter("Unify")->val->getBool())
		{
			tri::Clean<CMeshO>::RemoveDuplicateVertex(m.cm);
			tri::Allocator<CMeshO>::CompactEveryVector(m.cm);
		}
}

MESHLAB_PLUGIN_NAME_EXPORTER(BaseMeshIOPlugin)
