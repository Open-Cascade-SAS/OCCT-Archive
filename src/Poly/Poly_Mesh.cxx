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

#include <Poly_Mesh.hxx>
#include <Standard_DefineHandle.hxx>

IMPLEMENT_STANDARD_RTTIEXT (Poly_Mesh, Poly_Triangulation)

//=======================================================================
//function : Poly_Mesh
//purpose  :
//=======================================================================

Poly_Mesh::Poly_Mesh () : Poly_Triangulation (0, 0, Standard_False)
{
}

//=======================================================================
//function : Poly_Mesh
//purpose  :
//=======================================================================

Poly_Mesh::Poly_Mesh (const Standard_Integer theNbNodes,
                      const Standard_Integer theNbTriangles,
                      const Standard_Integer theNbQuads,
                      const Standard_Boolean theHasUVNodes,
                      const Standard_Boolean theHasNormals)
: Poly_Triangulation (theNbNodes, theNbTriangles, theHasUVNodes, theHasNormals)
{
  if (theNbQuads > 0)
    myQuads.Resize (1, theNbQuads, Standard_False);
}

//=======================================================================
//function : Poly_Mesh
//purpose  :
//=======================================================================

Poly_Mesh::Poly_Mesh (const Handle(Poly_Triangulation)& theTriangulation)
: Poly_Triangulation ( theTriangulation )
{
  // No quadrangles.
}

//=======================================================================
//function : Copy
//purpose  :
//=======================================================================

Handle(Poly_Triangulation) Poly_Mesh::Copy() const
{
  Handle(Poly_Triangulation) aCopiedTriangulation = Poly_Triangulation::Copy();
  Handle(Poly_Mesh) aCopiedMesh = new Poly_Mesh (aCopiedTriangulation);
  aCopiedMesh->myQuads = myQuads;
  return aCopiedMesh;
}
