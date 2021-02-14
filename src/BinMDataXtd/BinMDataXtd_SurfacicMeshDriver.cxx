// Created on: 2015-12-17
// Created by: Vlad Romashko
// Copyright (c) 2015 OPEN CASCADE SAS
//
// This file is part of Open CASCADE Technology software library.
//
// This library is free software; you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License version 2.1 as published
// by the Free Software Foundation, with special exception defined in the file
// OCCT_LGPL_EXCEPTION.txt. Consult the file LICENSE_LGPL_21.txt included in OCCT
// distribution for complete text of the license and disclaimer of any warranty.
//
// Alternatively, this file may be used under the terms of Open CASCADE
// commercial license or contractual agreement.

#include <BinMDataXtd_SurfacicMeshDriver.hxx>
#include <BinObjMgt_Persistent.hxx>
#include <Standard_Type.hxx>
#include <TDataXtd_SurfacicMesh.hxx>
#include <TDF_Attribute.hxx>

IMPLEMENT_STANDARD_RTTIEXT(BinMDataXtd_SurfacicMeshDriver,BinMDF_ADriver)

//=======================================================================
//function : BinMDataXtd_SurfacicMeshDriver
//purpose  : Constructor
//=======================================================================
BinMDataXtd_SurfacicMeshDriver::BinMDataXtd_SurfacicMeshDriver (const Handle(Message_Messenger)& theMsgDriver)
     : BinMDF_ADriver (theMsgDriver, STANDARD_TYPE(TDataXtd_SurfacicMesh)->Name())
{

}

//=======================================================================
//function : NewEmpty
//purpose  : 
//=======================================================================
Handle(TDF_Attribute) BinMDataXtd_SurfacicMeshDriver::NewEmpty() const
{
  return new TDataXtd_SurfacicMesh();
}

//=======================================================================
//function : Paste
//purpose  : persistent -> transient (retrieve)
//=======================================================================
Standard_Boolean BinMDataXtd_SurfacicMeshDriver::Paste (const BinObjMgt_Persistent&  theSource,
                                                        const Handle(TDF_Attribute)& theTarget,
                                                        BinObjMgt_RRelocationTable&  ) const
{
  Handle(TDataXtd_SurfacicMesh) attrMesh = Handle(TDataXtd_SurfacicMesh)::DownCast (theTarget);

  Standard_Integer i;
  Standard_Real deflection, x, y, z;
  Standard_Integer n1, n2, n3, n4;
  Standard_Integer nbNodes (0), nbTriangles (0), nbQuads (0);
  Standard_Boolean hasUV (Standard_False);
  gp_Pnt p;

  theSource >> nbNodes;
  theSource >> nbTriangles;
  theSource >> nbQuads;
  theSource >> hasUV;
  theSource >> deflection;

  // allocate the mesh
  Handle(Poly_Mesh) aMesh = new Poly_Mesh (nbNodes, nbTriangles, nbQuads, hasUV);

  // deflection
  aMesh->Deflection (deflection);

  // read nodes
  for (i = 1; i <= nbNodes; i++)
  {
    theSource >> x;
    theSource >> y;
    theSource >> z;
    p.SetCoord (x, y, z);
    aMesh->ChangeNode (i) = p;
  }
      
  // read 2d nodes
  if (hasUV)
  {
    for (i = 1; i <= nbNodes; i++)
    {
      theSource >> x;
      theSource >> y;
      aMesh->ChangeUVNode (i).SetCoord (x,y);
    }
  }

  // read triangles
  for (i = 1; i <= nbTriangles; i++)
  {
      theSource >> n1;
      theSource >> n2;
      theSource >> n3;
      aMesh->ChangeTriangle (i).Set (n1, n2, n3);
  }

  // read quadrangles
  for (i = 1; i <= nbQuads; i++)
  {
      theSource >> n1;
      theSource >> n2;
      theSource >> n3;
      theSource >> n4;
      aMesh->ChangeQuad (i).Set (n1, n2, n3, n4);
  }

  // Set mesh to Ocaf attribute
  attrMesh->Set (aMesh);
  return !aMesh.IsNull();
}

//=======================================================================
//function : Paste
//purpose  : transient -> persistent (store)
//=======================================================================
void BinMDataXtd_SurfacicMeshDriver::Paste (const Handle(TDF_Attribute)& theSource,
                                            BinObjMgt_Persistent&        theTarget,
                                            BinObjMgt_SRelocationTable&  ) const
{
  const Handle(TDataXtd_SurfacicMesh) attrMesh = Handle(TDataXtd_SurfacicMesh)::DownCast (theSource);
  const Handle(Poly_Mesh)& aMesh = attrMesh->Get();
  if (!aMesh.IsNull())
  {
    Standard_Integer nbNodes = aMesh->NbNodes();
    Standard_Integer nbTriangles = aMesh->NbTriangles();
    Standard_Integer nbQuads = aMesh->NbQuads();

    // write number of elements
    theTarget << nbNodes;
    theTarget << nbTriangles;
    theTarget << nbQuads;
    theTarget << (aMesh->HasUVNodes() ? 1 : 0);
    // write the deflection
    theTarget << aMesh->Deflection();

    // write 3d nodes
    Standard_Integer i;
    for (i = 1; i <= nbNodes; i++)
    {
      const gp_Pnt& aNode = aMesh->Node (i);
      theTarget << aNode.X();
      theTarget << aNode.Y();
      theTarget << aNode.Z();
    }

    // write 2d nodes
    if (aMesh->HasUVNodes())
    {
      for (i = 1; i <= nbNodes; i++)
      {
        const gp_Pnt2d& aUVNode = aMesh->UVNode (i);
        theTarget << aUVNode.X();
        theTarget << aUVNode.Y();
      }
    }

    // write triangles
    Standard_Integer n1, n2, n3;
    for (i = 1; i <= nbTriangles; i++)
    {
      aMesh->Triangle (i).Get (n1, n2, n3);
      theTarget << n1;
      theTarget << n2;
      theTarget << n3;
    }

    // write quadrangles
    Standard_Integer n4;
    for (i = 1; i <= nbQuads; i++)
    {
      aMesh->Quad (i).Get (n1, n2, n3, n4);
      theTarget << n1;
      theTarget << n2;
      theTarget << n3;
      theTarget << n4;
    }
  }
}
