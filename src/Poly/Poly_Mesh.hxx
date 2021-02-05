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

#include <Poly_Element.hxx>
#include <Poly_Triangulation.hxx>

//! This class is extension for Poly_Triangulation.
//! It allows to store mesh with quad polygons as table of Poly_Element.
//! Keep in mind that when you add a triangle, it is also added to the table of elements
//! as Poly_Element. And it will have first index set to triangle index from Poly_Triangulation
//! and second index will be set to 0.
class Poly_Mesh : public Poly_Triangulation
{

public:

  //! Constructs an empty mesh.
  //! @param theHasUVNodes indicates whether 2D nodes will be associated with
  //!        3D ones, (i.e. to enable a 2D representation).
  Standard_EXPORT Poly_Mesh (const Standard_Boolean theHasUVNodes = Standard_False);

  //! Constructs a mesh from existing triangulation.
  //! @param theTriangulation source triangulation.
  Standard_EXPORT Poly_Mesh (const Handle(Poly_Triangulation)& theTriangulation);

  //! Creates full copy of current mesh
  Standard_EXPORT virtual Handle(Poly_Triangulation) Copy() const;

  //! Adds element to the mesh.
  //! @param theN1 index of the first node.
  //! @param theN2 index of the second node.
  //! @param theN3 index of the third node.
  //! @return index of the added element.
  Standard_EXPORT Standard_Integer AddElement (const Standard_Integer theN1,
                                               const Standard_Integer theN2,
                                               const Standard_Integer theN3);

  //! Adds element to the mesh.
  //! @param theN1 index of the first node.
  //! @param theN2 index of the second node.
  //! @param theN3 index of the third node.
  //! @param theN4 index of the fourth node.
  //! @return index of the added element.
  Standard_EXPORT Standard_Integer AddElement (const Standard_Integer theN1,
                                               const Standard_Integer theN2,
                                               const Standard_Integer theN3,
                                               const Standard_Integer theN4);

  //! @return the number of elements for this mesh.
  Standard_Integer NbElements() const { return myElements.Size(); }

  //! @return the number of quads for this mesh.
  Standard_Integer NbQuads() const { return myNbQuads; }

  //! @return element at the given index.
  //! Raises Standard_OutOfRange exception if theIndex is less than 1 or greater than NbElements.
  Standard_EXPORT const Poly_Element& Element (const Standard_Integer theIndex) const;

  //! @return nodes of the element at the given index.
  //! Raises Standard_OutOfRange exception if theIndex is less than 1 or greater than NbElements.
  Standard_EXPORT void Element (const Standard_Integer theIndex,
                                Standard_Integer& theN1,
                                Standard_Integer& theN2,
                                Standard_Integer& theN3,
                                Standard_Integer& theN4) const;

  //! Sets the element at the given index.
  //! Raises Standard_OutOfRange exception if theIndex is less than 1 or greater than NbElements.
  Standard_EXPORT void SetElement (const Standard_Integer theIndex, const Poly_Element& theElement);

protected:

  //! Adds element to the mesh.
  //! @param theElement element to add.
  //! @return index of the added element.
  Standard_EXPORT Standard_Integer addElement (const Poly_Element& theElement);

private:

  NCollection_Vector<Poly_Element> myElements;
  Standard_Integer                 myNbQuads;

public:

  DEFINE_STANDARD_RTTIEXT(Poly_Mesh, Poly_Triangulation)

};

DEFINE_STANDARD_HANDLE(Poly_Mesh, Poly_Triangulation)

#endif // _Poly_Mesh_HeaderFile
