/****************************************************************************
* VCGLib                                                            o o     *
* Visual and Computer Graphics Library                            o     o   *
*                                                                _   O  _   *
* Copyright(C) 2004                                                \/)\/    *
* Visual Computing Lab                                            /\/|      *
* ISTI - Italian National Research Council                           |      *
*                                                                    \      *
* All rights reserved.                                                      *
*                                                                           *
* This program is free software; you can redistribute it and/or modify      *
* it under the terms of the GNU General Public License as published by      *
* the Free Software Foundation; either version 2 of the License, or         *
* (at your option) any later version.                                       *
*                                                                           *
* This program is distributed in the hope that it will be useful,           *
* but WITHOUT ANY WARRANTY; without even the implied warranty of            *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
* GNU General Public License (http://www.gnu.org/licenses/gpl.txt)          *
* for more details.                                                         *
*                                                                           *
****************************************************************************/

#include <time.h>
#include <list>
#include <utility>
#include <bitset>

#include "OccupancyGrid.h"

#include <vcg/complex/algorithms/clean.h>
#include <wrap/io_trimesh/import.h>
#include <wrap/ply/plystuff.h>

using namespace std;
using namespace vcg;


// Nota che il bbox viene automaticamento inflatato dalla G.SetBBox();
bool OccupancyGrid::Init(int _mn, Box3d bb, int size)
{
	mn=_mn; // the number of meshes (including all the unused ones; eg it is the range of the possible id)
	if(mn>MeshCounter::MaxVal()) return false;
	MeshCounter MC;
	MC.Clear();
	G.Create(bb,size,MC);
	VM.clear();
	return true;
}

void OccupancyGrid::Add(const char *MeshName, Matrix44d &Tr, int id)
{
	A2Mesh M;
	tri::io::Importer<A2Mesh>::Open(M,MeshName);
	tri::Clean<A2Mesh>::RemoveUnreferencedVertex(M);
	AddMesh(M,Tr,id);
}

void OccupancyGrid::AddMeshes(std::vector<string> &names, std::vector<Matrix44d> &trv, int size )
{
	unsigned int i;

	Box3d bb,totalbb;

	bb.SetNull();
	totalbb.SetNull();

	printf("OG::AddMesh:Scanning BBoxex\n");
	for(i=0;i<names.size();++i)
	{
		ply::ScanBBox(names[i].c_str(),bb);
		totalbb.Add( trv[i], bb);
	}
	Init(names.size(),totalbb,size);

	for(i=0;i<names.size();++i)
	{
		printf("OG::AddMesh:Adding Mesh %i '%s'\n",i,names[i].c_str());
		Add(names[i].c_str(),trv[i],i);
	}
}


// This function is called after we have <added> all the mesh to the OG
// to collect the information about the interferences between the various meshes.
void OccupancyGrid::Compute()
{
	// Analisi della griglia
	// Si deve trovare l'insieme degli archi piu'plausibili
	// un arco ha "senso" in una cella se entrambe le mesh compaiono in quell'arco
	// Si considera tutti gli archi possibili e si conta in quante celle ha senso un arco
	std::vector<int> VA; // virtual arcs
	VA.resize(mn*mn,0);
	std::map< std::pair<int,int>, int > VAMap;

	// First Loop:
	// Scan the grid and update possible arc count
	for(int i=0;i<G.siz[0];++i)
	{
		for(int j=0;j<G.siz[1];++j)
		{
			for(int k=0;k<G.siz[2];++k)
			{
				vector<int > vv;
				G.Grid(i,j,k).Pack(vv);
				size_t meshInCell = vv.size();
				for( size_t ii=0; ii< vv.size(); ++ii)
				{ 
					OGMeshInfo & omi_ii = VM[vv[ii]];
					++omi_ii.area; // compute mesh area
					if(meshInCell>omi_ii.densityDistribution.size())
						omi_ii.densityDistribution.resize(meshInCell);
					++(omi_ii.densityDistribution[meshInCell-1]);
				}

				for(size_t ii=0;ii<vv.size();++ii)
				{
					for(size_t jj=ii+1;jj<vv.size();++jj)
					{
						//              ++VA[vv[ii]+vv[jj]*mn]; // count intersections of all mesh pairs
						++VAMap[std::make_pair(vv[ii],vv[jj])]; // count intersections of all mesh pairs
					}
				}
			}
		}
	}
	// Find all the arcs, e.g. all the pair of meshes 
	SVA.clear();
	for(auto vi=VAMap.begin();vi!=VAMap.end();++vi)
	{
		if(vi->second > 0) 
		{
			int m_s = vi->first.first;
			int m_t = vi->first.second;
			int area = vi->second;
			SVA.push_back( OGArcInfo (m_s,m_t,area,float(area)/float(min(VM[m_s].area,VM[m_t].area)) ));
		}     
	}

	//  SVA.clear();
	//  for(int i=0;i<mn-1;++i)
	//    if(VM[i].used)
	//      for(int j=i+1;j<mn;++j)
	//        if(VM[j].used && VA[i+j*mn]>0)
	//          SVA.push_back( OGArcInfo(i,j, VA[i+j*mn], VA[i+j*mn]/float( min(VM[i].area,VM[j].area)) ));


	// Compute Mesh Coverage
	for(size_t i=0;i<SVA.size();++i)
	{
		VM[SVA[i].s].coverage += SVA[i].area;
		VM[SVA[i].t].coverage += SVA[i].area;
	}

	sort(SVA.begin(),SVA.end());
	reverse(SVA.begin(),SVA.end());
}


void OccupancyGrid::ComputeTotalArea()
{
	int ccnt=0;
	MaxCount=0;
	int sz=G.size();
	for(int i=0;i<sz;++i)
	{
		if(!G.grid[i].Empty())
		{
			ccnt++;
			if(G.grid[i].Count()>MaxCount) MaxCount=G.grid[i].Count();
		}
	}
	TotalArea=ccnt;
}
/*
Ordinare le RangeMap in base a quanto sono utili.
Una RangeMap e' utile se copre parti non ancora viste

Per ogni cella della og c'e' un bit che dice quali range map ci passano
per ogni range map si conosce l'area (espressa in celle della og)
Si considera un voxel visto se ci sono almeno <K> range map che lo coprono.
Inizialmente si moltiplica *1,2, ..K l'area di tutte le rm

Si parte dalla rm con area maggiore e si diminuisce di uno l'area
di tutte le altre rm che tocca in voxel visti meno di due volte.

*/
void OccupancyGrid::ComputeUsefulMesh(FILE *elfp)
{
	vector<int> UpdArea(mn);
	vector<int> UpdCovg(mn);

	//    Use.clear();
	int i,j,m,mcnt=0;
	for(m=0;m<mn;++m) 
	{
		if(VM[m].used && VM[m].area>0) 
		{
			mcnt++;
			UpdCovg[m]=VM[m].coverage;
			UpdArea[m]=VM[m].area;
		}
	}

	int sz=G.size();
	if(elfp) 
	{
		fprintf(elfp,"\n\nComputing Usefulness of Meshes of %i(on %i) meshes\n Og with %i / %i fill ratio %i max mesh per cell\n\n",mcnt,mn,TotalArea,sz,MaxCount);
		fprintf(elfp,"\n");

	}
	int CumArea=0;
	for(m=0;m<mn-1;++m)
	{
		int best = max_element(UpdArea.begin(),UpdArea.end())-UpdArea.begin();
		CumArea+=UpdArea[best];
		if(UpdCovg[best]<0) break;
		if(VM[best].area==0) continue; // se era una mesh fuori del working group si salta tutto.

		if(elfp) fprintf(elfp,"%3i %3i %7i (%7i) %7i %5.2f %7i(%7i)\n",
			m, best, UpdArea[best],VM[best].area, TotalArea-CumArea, 100.0-100*float(CumArea)/TotalArea, UpdCovg[best],VM[best].coverage);

		UpdArea[best]=-1;
		UpdCovg[best]=-1;

		for(i=0;i<sz;++i)
		{
			MeshCounter &mc=G.grid[i];
			if(mc.IsSet(best))	
			{
				mc.UnSet(best);
				for(j=0;j<mn;++j)
				{
					if(mc.IsSet(j)) 
					{
						--UpdArea[j];
						UpdCovg[j]-=mc.Count();
					}
				}
				mc.Clear();
			}
		}
	}
}

void OccupancyGrid::Dump(FILE *fp)
{
	fprintf(fp,"Occupancy Grid\n");
	fprintf(fp,"grid of ~%i kcells: %d x %d x %d\n",G.size(),G.siz[0],G.siz[1],G.siz[2]);
	fprintf(fp,"grid voxel size of %f %f %f\n",G.voxel[0],G.voxel[1],G.voxel[2]);

	fprintf(fp,"Computed %lu arcs for %i meshes\n",SVA.size(),mn);
	for(size_t i=0;i<VM.size();++i)
	{
		if(VM[i].used)
		{
			fprintf(fp,"mesh %3lu area %6i covg %7i (%5.2f%%) DensDistr:",i,VM[i].area,VM[i].coverage,float(VM[i].coverage)/float(VM[i].area));
			for(size_t j=0;j<std::min(size_t(8),VM[i].densityDistribution.size());++j)
			{
				fprintf(fp," %3i ", VM[i].densityDistribution[j]);
			}
			fprintf(fp,"\n");
		}
		else
			fprintf(fp,"mesh %3lu ---- UNUSED\n",i);
	}
	fprintf(fp,"Computed %lu Arcs :\n",SVA.size());
	for(size_t i=0;i<SVA.size() && SVA[i].norm_area > .1; ++i)
	{
		fprintf(fp,"%4i -> %4i Area:%5i NormArea:%5.3f\n",SVA[i].s,SVA[i].t,SVA[i].area,SVA[i].norm_area);
	}
	fprintf(fp,"End OG Dump\n");
}

//// sceglie gli archi da fare che abbiano una sovrapposizione di almeno <normarea>
//// e restituisce la lista di nodi isolati;
//void OccupancyGrid::ChooseArcs(vector<pair<int,int> > &AV, vector<int> &BNV, vector<int> &adjcnt, float normarea)
//{
//    AV.clear();
//    BNV.clear();
//    size_t i=0;
//    adjcnt.clear();
//    adjcnt.resize(mn,0);

//    while(SVA[i].norm_area>normarea && i<SVA.size())
//    {
//            AV.push_back(make_pair( SVA[i].s, SVA[i].t) );

//            ++adjcnt[SVA[i].s];
//            ++adjcnt[SVA[i].t];
//            ++i;
//    }

//  // Second loop to add some more constraints we add also all the arc with area > normarea/3
//    // and that connects meshes poorly connected (e.g. with zero or one adjacent)
//  normarea/=3.0;
//    while(SVA[i].norm_area>normarea && i<SVA.size())
//    {
//          if(adjcnt[SVA[i].s]<=1 || adjcnt[SVA[i].t]<=1 )
//            {
//                AV.push_back(make_pair( SVA[i].s, SVA[i].t) );

//                ++adjcnt[SVA[i].s];
//                ++adjcnt[SVA[i].t];
//            }
//            ++i;
//    }

//    for(i=0;i<mn;++i) if(VM[i].used && adjcnt[i]==0) BNV.push_back(i);
//}

void OccupancyGrid::RemoveMesh(int id)
{
	MeshCounter *GridEnd=G.grid+G.size();
	MeshCounter *ig;
	for(ig=G.grid;ig!=GridEnd;++ig)
	{
		ig->UnSet(id);
	}
}
