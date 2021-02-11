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

#ifndef _Poly_Mesh_HeaderFile
#define _Poly_Mesh_HeaderFile

#include <Poly_Quad.hxx>
#include <Poly_Triangulation.hxx>

//! This class is extension for Poly_Triangulation.
//! It allows to store mesh with quad polygons.
class Poly_Mesh : public Poly_Triangulation
{

public:

  //! Constructs an empty mesh.
  Standard_EXPORT Poly_Mesh();

  //! Constructs a mesh.
  //! @param theNbNodes defines the number of nodes.
  //! @param theNbTriangles defines the number of triangles.
  //! @param theNbQuads defines the number of quadrangles.
  //! @param theHasUVNodes indicates whether 2D nodes will be associated with
  //!        3D ones, (i.e. to enable a 2D representation).
  //! @param theHasNormals defines allocation of normals for the nodes.
  Standard_EXPORT Poly_Mesh(const Standard_Integer theNbNodes,
                            const Standard_Integer theNbTriangles,
                            const Standard_Integer theNbQuads,
                            const Standard_Boolean theHasUVNodes = Standard_False,
                            const Standard_Boolean theHasNormals = Standard_False);

  //! Constructs a mesh from existing triangulation.
  //! @param theTriangulation source triangulation.
  Standard_EXPORT Poly_Mesh (const Handle(Poly_Triangulation)& theTriangulation);

  //! Creates full copy of current mesh
  Standard_EXPORT virtual Handle(Poly_Triangulation) Copy() const Standard_OVERRIDE;

  //! Sets a quadrangle to the mesh.
  //! @param theIndex is an index of the quadrangle.
  //! @param theN1 index of the first node.
  //! @param theN2 index of the second node.
  //! @param theN3 index of the third node.
  //! @param theN4 index of the fourth node.
  void SetQuad (const Standard_Integer theIndex,
                const Standard_Integer theN1,
                const Standard_Integer theN2,
                const Standard_Integer theN3,
                const Standard_Integer theN4)
  {
    myQuads.SetValue (theIndex, Poly_Quad (theN1, theN2, theN3, theN4));
  }

  //! @return the number of quadrangles in the mesh.
  Standard_Integer NbQuads() const {
    return myQuads.Size();
  }

  //! @return a quadrangle at the given index.
  const Poly_Quad& Quad (const Standard_Integer theIndex) const {
    return myQuads.Value (theIndex);
  }

  //! @return a reference to a quadrangle at the given index.
  Poly_Quad& ChangeQuad (const Standard_Integer theIndex) {
    return myQuads.ChangeValue (theIndex);
  }

protected:

  NCollection_Array1<Poly_Quad> myQuads;

public:

  DEFINE_STANDARD_RTTIEXT(Poly_Mesh, Poly_Triangulation)

};

DEFINE_STANDARD_HANDLE(Poly_Mesh, Poly_Triangulation)

#endif // _Poly_Mesh_HeaderFile
