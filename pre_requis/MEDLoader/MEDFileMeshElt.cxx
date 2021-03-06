// Copyright (C) 2007-2014  CEA/DEN, EDF R&D
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
// Author : Anthony Geay (CEA/DEN)

#include "MEDFileMeshElt.hxx"
#include "MEDFileMeshReadSelector.hxx"

#include "MEDCouplingUMesh.hxx"

#include "InterpKernelAutoPtr.hxx"
#include "CellModel.hxx"

#include <iostream>

extern med_geometry_type typmai3[34];

using namespace ParaMEDMEM;

MEDFileUMeshPerType *MEDFileUMeshPerType::New(med_idt fid, const char *mName, int dt, int it, int mdim, med_geometry_type geoElt, INTERP_KERNEL::NormalizedCellType geoElt2, MEDFileMeshReadSelector *mrs)
{
  med_entity_type whichEntity;
  if(!isExisting(fid,mName,dt,it,geoElt,whichEntity))
    return 0;
  return new MEDFileUMeshPerType(fid,mName,dt,it,mdim,geoElt,geoElt2,whichEntity,mrs);
}

std::size_t MEDFileUMeshPerType::getHeapMemorySizeWithoutChildren() const
{
  return 0;
}

std::vector<const BigMemoryObject *> MEDFileUMeshPerType::getDirectChildren() const
{
  std::vector<const BigMemoryObject *> ret;
  if((const MEDCoupling1GTUMesh *)_m)
    ret.push_back((const MEDCoupling1GTUMesh *)_m);
  if((const DataArrayInt *)_num)
    ret.push_back((const DataArrayInt *)_num);
  if((const DataArrayInt *)_fam)
    ret.push_back((const DataArrayInt *)_fam);
  if((const DataArrayAsciiChar *)_names)
    ret.push_back((const DataArrayAsciiChar *)_names);
  return ret;
}

bool MEDFileUMeshPerType::isExisting(med_idt fid, const char *mName, int dt, int it, med_geometry_type geoElt, med_entity_type& whichEntity)
{
  static const med_entity_type entities[3]={MED_CELL,MED_DESCENDING_FACE,MED_DESCENDING_EDGE};
  int nbOfElt=0;
  for(int i=0;i<3;i++)
    {
      med_bool changement,transformation;
      int tmp=MEDmeshnEntity(fid,mName,dt,it,entities[i],geoElt,MED_CONNECTIVITY,MED_NODAL,
          &changement,&transformation);
      if(tmp>nbOfElt)
        {
          nbOfElt=tmp;
          whichEntity=entities[i];
          if(i>0)
            std::cerr << "WARNING : MEDFile has been detected to be no compilant with MED 3 : Please change entity in MEDFile for geotype " <<  geoElt << std::endl;
        }
    }
  return nbOfElt>0;
}

int MEDFileUMeshPerType::getDim() const
{
  return _m->getMeshDimension();
}

MEDFileUMeshPerType::MEDFileUMeshPerType(med_idt fid, const char *mName, int dt, int it, int mdim, med_geometry_type geoElt, INTERP_KERNEL::NormalizedCellType type,
                                         med_entity_type entity, MEDFileMeshReadSelector *mrs):_entity(entity)
{
  med_bool changement,transformation;
  int curNbOfElem=MEDmeshnEntity(fid,mName,dt,it,entity,geoElt,MED_CONNECTIVITY,MED_NODAL,
      &changement,&transformation);
  const INTERP_KERNEL::CellModel& cm=INTERP_KERNEL::CellModel::GetCellModel(type);
  if(!cm.isDynamic())
    {
      loadFromStaticType(fid,mName,dt,it,mdim,curNbOfElem,geoElt,type,entity,mrs);
      return;
    }
  if(type==INTERP_KERNEL::NORM_POLYGON || type==INTERP_KERNEL::NORM_QPOLYG)
    {
      loadPolyg(fid,mName,dt,it,mdim,curNbOfElem,geoElt,entity,mrs);
      return;
    }
  //if(type==INTERP_KERNEL::NORM_POLYHED)
  loadPolyh(fid,mName,dt,it,mdim,curNbOfElem,geoElt,entity,mrs);
}

void MEDFileUMeshPerType::loadFromStaticType(med_idt fid, const char *mName, int dt, int it, int mdim, int curNbOfElem, med_geometry_type geoElt, INTERP_KERNEL::NormalizedCellType type,
                                             med_entity_type entity, MEDFileMeshReadSelector *mrs)
{
  _m=MEDCoupling1SGTUMesh::New(mName,type);
  MEDCoupling1SGTUMesh *mc(dynamic_cast<MEDCoupling1SGTUMesh *>((MEDCoupling1GTUMesh *)_m));
  MEDCouplingAutoRefCountObjectPtr<DataArrayInt> conn(DataArrayInt::New());
  int nbOfNodesPerCell=mc->getNumberOfNodesPerCell();
  conn->alloc(nbOfNodesPerCell*curNbOfElem,1);
  MEDmeshElementConnectivityRd(fid,mName,dt,it,entity,geoElt,MED_NODAL,MED_FULL_INTERLACE,conn->getPointer());
  std::transform(conn->begin(),conn->end(),conn->getPointer(),std::bind2nd(std::plus<int>(),-1));
  mc->setNodalConnectivity(conn);
  loadCommonPart(fid,mName,dt,it,mdim,curNbOfElem,geoElt,entity,mrs);
}

void MEDFileUMeshPerType::loadCommonPart(med_idt fid, const char *mName, int dt, int it, int mdim, int curNbOfElem, med_geometry_type geoElt,
                                         med_entity_type entity, MEDFileMeshReadSelector *mrs)
{
  med_bool changement,transformation;
  _fam=0;
  if(MEDmeshnEntity(fid,mName,dt,it,entity,geoElt,MED_FAMILY_NUMBER,MED_NODAL,&changement,&transformation)>0)
    {    
      if(!mrs || mrs->isCellFamilyFieldReading())
        {
          _fam=DataArrayInt::New();
          _fam->alloc(curNbOfElem,1);
          if(MEDmeshEntityFamilyNumberRd(fid,mName,dt,it,entity,geoElt,_fam->getPointer())!=0)
            std::fill(_fam->getPointer(),_fam->getPointer()+curNbOfElem,0);
        }
    }
  _num=0;
  if(MEDmeshnEntity(fid,mName,dt,it,entity,geoElt,MED_NUMBER,MED_NODAL,&changement,&transformation)>0)
    {
      if(!mrs || mrs->isCellNumFieldReading())
        {
          _num=DataArrayInt::New();
          _num->alloc(curNbOfElem,1);
          if(MEDmeshEntityNumberRd(fid,mName,dt,it,entity,geoElt,_num->getPointer())!=0)
            _num=0;
        }
    }
  _names=0;
  if(MEDmeshnEntity(fid,mName,dt,it,entity,geoElt,MED_NAME,MED_NODAL,&changement,&transformation)>0)
    {
      if(!mrs || mrs->isCellNameFieldReading())
        {
          _names=DataArrayAsciiChar::New();
          _names->alloc(curNbOfElem+1,MED_SNAME_SIZE);//not a bug to avoid the memory corruption due to last \0 at the end
          if(MEDmeshEntityNameRd(fid,mName,dt,it,entity,geoElt,_names->getPointer())!=0)
            _names=0;
          else
            _names->reAlloc(curNbOfElem);//not a bug to avoid the memory corruption due to last \0 at the end
        }
    }
}

void MEDFileUMeshPerType::loadPolyg(med_idt fid, const char *mName, int dt, int it, int mdim, int arraySize, med_geometry_type geoElt,
                                    med_entity_type entity, MEDFileMeshReadSelector *mrs)
{
  med_bool changement,transformation;
  med_int curNbOfElem=MEDmeshnEntity(fid,mName,dt,it,entity,geoElt,MED_INDEX_NODE,MED_NODAL,&changement,&transformation)-1;
  _m=MEDCoupling1DGTUMesh::New(mName,geoElt==MED_POLYGON?INTERP_KERNEL::NORM_POLYGON:INTERP_KERNEL::NORM_QPOLYG);
  MEDCouplingAutoRefCountObjectPtr<MEDCoupling1DGTUMesh> mc(DynamicCast<MEDCoupling1GTUMesh,MEDCoupling1DGTUMesh>(_m));
  MEDCouplingAutoRefCountObjectPtr<DataArrayInt> conn(DataArrayInt::New()),connI(DataArrayInt::New());
  conn->alloc(arraySize,1); connI->alloc(curNbOfElem+1,1);
  MEDmeshPolygon2Rd(fid,mName,dt,it,MED_CELL,geoElt,MED_NODAL,connI->getPointer(),conn->getPointer());
  std::transform(conn->begin(),conn->end(),conn->getPointer(),std::bind2nd(std::plus<int>(),-1));
  std::transform(connI->begin(),connI->end(),connI->getPointer(),std::bind2nd(std::plus<int>(),-1));
  mc->setNodalConnectivity(conn,connI);
  loadCommonPart(fid,mName,dt,it,mdim,curNbOfElem,geoElt,entity,mrs);
}

void MEDFileUMeshPerType::loadPolyh(med_idt fid, const char *mName, int dt, int it, int mdim, int connFaceLgth, med_geometry_type geoElt,
                                    med_entity_type entity, MEDFileMeshReadSelector *mrs)
{
  med_bool changement,transformation;
  med_int indexFaceLgth=MEDmeshnEntity(fid,mName,dt,it,MED_CELL,MED_POLYHEDRON,MED_INDEX_NODE,MED_NODAL,&changement,&transformation);
  int curNbOfElem=MEDmeshnEntity(fid,mName,dt,it,MED_CELL,MED_POLYHEDRON,MED_INDEX_FACE,MED_NODAL,&changement,&transformation)-1;
  _m=MEDCoupling1DGTUMesh::New(mName,INTERP_KERNEL::NORM_POLYHED);
  MEDCouplingAutoRefCountObjectPtr<MEDCoupling1DGTUMesh> mc(DynamicCastSafe<MEDCoupling1GTUMesh,MEDCoupling1DGTUMesh>(_m));
  INTERP_KERNEL::AutoPtr<int> index=new int[curNbOfElem+1];
  INTERP_KERNEL::AutoPtr<int> indexFace=new int[indexFaceLgth];
  INTERP_KERNEL::AutoPtr<int> locConn=new int[connFaceLgth];
  MEDmeshPolyhedronRd(fid,mName,dt,it,MED_CELL,MED_NODAL,index,indexFace,locConn);
  MEDCouplingAutoRefCountObjectPtr<DataArrayInt> conn(DataArrayInt::New()),connI(DataArrayInt::New());
  int arraySize=connFaceLgth;
  for(int i=0;i<curNbOfElem;i++)
    arraySize+=index[i+1]-index[i]-1;
  conn=DataArrayInt::New();
  conn->alloc(arraySize,1);
  int *wFinalConn=conn->getPointer();
  connI->alloc(curNbOfElem+1,1);
  int *finalIndex(connI->getPointer());
  finalIndex[0]=0;
  for(int i=0;i<curNbOfElem;i++)
    {
      finalIndex[i+1]=finalIndex[i]+index[i+1]-index[i]-1+indexFace[index[i+1]-1]-indexFace[index[i]-1];
      wFinalConn=std::transform(locConn+indexFace[index[i]-1]-1,locConn+indexFace[index[i]]-1,wFinalConn,std::bind2nd(std::plus<int>(),-1));
      for(int j=index[i];j<index[i+1]-1;j++)
        {
          *wFinalConn++=-1;
          wFinalConn=std::transform(locConn+indexFace[j]-1,locConn+indexFace[j+1]-1,wFinalConn,std::bind2nd(std::plus<int>(),-1));
        }
    }
  mc->setNodalConnectivity(conn,connI);
  loadCommonPart(fid,mName,dt,it,mdim,curNbOfElem,MED_POLYHEDRON,entity,mrs);
}

void MEDFileUMeshPerType::Write(med_idt fid, const std::string& mname, int mdim, const MEDCoupling1GTUMesh *m, const DataArrayInt *fam, const DataArrayInt *num, const DataArrayAsciiChar *names)
{
  int nbOfCells=m->getNumberOfCells();
  if(nbOfCells<1)
    return ;
  int dt,it;
  double timm=m->getTime(dt,it);
  INTERP_KERNEL::NormalizedCellType ikt=m->getTypeOfCell(0);
  const INTERP_KERNEL::CellModel& cm=INTERP_KERNEL::CellModel::GetCellModel(ikt);
  med_geometry_type curMedType=typmai3[(int)ikt];
  if(!cm.isDynamic())
    {
      const MEDCoupling1SGTUMesh *m0(dynamic_cast<const MEDCoupling1SGTUMesh *>(m));
      if(!m0)
        throw INTERP_KERNEL::Exception("MEDFileUMeshPerType::Write : internal error #1 !");
      MEDCouplingAutoRefCountObjectPtr<DataArrayInt> arr(m0->getNodalConnectivity()->deepCpy());
      std::transform(arr->begin(),arr->end(),arr->getPointer(),std::bind2nd(std::plus<int>(),1));
      MEDmeshElementConnectivityWr(fid,mname.c_str(),dt,it,timm,MED_CELL,curMedType,MED_NODAL,MED_FULL_INTERLACE,nbOfCells,arr->begin());
    }
  else
    {
      const MEDCoupling1DGTUMesh *m0(dynamic_cast<const MEDCoupling1DGTUMesh *>(m));
      if(!m0)
        throw INTERP_KERNEL::Exception("MEDFileUMeshPerType::Write : internal error #2 !");
      if(ikt==INTERP_KERNEL::NORM_POLYGON || ikt==INTERP_KERNEL::NORM_QPOLYG)
        {
          MEDCouplingAutoRefCountObjectPtr<DataArrayInt> arr(m0->getNodalConnectivity()->deepCpy()),arrI(m0->getNodalConnectivityIndex()->deepCpy());
          std::transform(arr->begin(),arr->end(),arr->getPointer(),std::bind2nd(std::plus<int>(),1));
          std::transform(arrI->begin(),arrI->end(),arrI->getPointer(),std::bind2nd(std::plus<int>(),1));
          MEDmeshPolygon2Wr(fid,mname.c_str(),dt,it,timm,MED_CELL,ikt==INTERP_KERNEL::NORM_POLYGON?MED_POLYGON:MED_POLYGON2,MED_NODAL,nbOfCells+1,arrI->begin(),arr->begin());
        }
      else
        {
          const int *conn(m0->getNodalConnectivity()->begin()),*connI(m0->getNodalConnectivityIndex()->begin());
          int meshLgth=m0->getNodalConnectivityLength();
          int nbOfFaces=std::count(conn,conn+meshLgth,-1)+nbOfCells;
          INTERP_KERNEL::AutoPtr<int> tab1=new int[nbOfCells+1];
          int *w1=tab1; *w1=1;
          INTERP_KERNEL::AutoPtr<int> tab2=new int[nbOfFaces+1];
          int *w2=tab2; *w2=1;
          INTERP_KERNEL::AutoPtr<int> bigtab=new int[meshLgth];
          int *bt=bigtab;
          for(int i=0;i<nbOfCells;i++,w1++)
            {
              int nbOfFaces2=0;
              for(const int *w=conn+connI[i];w!=conn+connI[i+1];w2++)
                {
                  const int *wend=std::find(w,conn+connI[i+1],-1);
                  bt=std::transform(w,wend,bt,std::bind2nd(std::plus<int>(),1));
                  int nbOfNode=std::distance(w,wend);
                  w2[1]=w2[0]+nbOfNode;
                  if(wend!=conn+connI[i+1])
                    w=wend+1;
                  else
                    w=wend;
                  nbOfFaces2++;
                }
              w1[1]=w1[0]+nbOfFaces2;
            }
          MEDmeshPolyhedronWr(fid,mname.c_str(),dt,it,timm,MED_CELL,MED_NODAL,nbOfCells+1,tab1,nbOfFaces+1,tab2,bigtab);
        }
    }
  if(fam)
    MEDmeshEntityFamilyNumberWr(fid,mname.c_str(),dt,it,MED_CELL,curMedType,nbOfCells,fam->getConstPointer());
  if(num)
    MEDmeshEntityNumberWr(fid,mname.c_str(),dt,it,MED_CELL,curMedType,nbOfCells,num->getConstPointer());
  if(names)
    {
      if(names->getNumberOfComponents()!=MED_SNAME_SIZE)
        {
          std::ostringstream oss; oss << "MEDFileUMeshPerType::write : expected a name field on cells with number of components set to " << MED_SNAME_SIZE;
          oss << " ! The array has " << names->getNumberOfComponents() << " components !";
          throw INTERP_KERNEL::Exception(oss.str().c_str());
        }
      MEDmeshEntityNameWr(fid,mname.c_str(),dt,it,MED_CELL,curMedType,nbOfCells,names->getConstPointer());
    }
}
