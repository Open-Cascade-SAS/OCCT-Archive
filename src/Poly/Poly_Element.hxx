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

#ifndef _Poly_Element_HeaderFile
#define _Poly_Element_HeaderFile

#include <Standard_DefineAlloc.hxx>
#include <Standard_Integer.hxx>
#include <Standard_OutOfRange.hxx>

//! Describes an element on mesh.
//! It can be defined as triangle index (in this case second index will be 0)
//! or as a pair of triangles indices that make up the quad.
class Poly_Element
{

public:

  DEFINE_STANDARD_ALLOC

  //! Constructs an element and sets all indices to zero.
  Standard_EXPORT Poly_Element();

  //! Constructs an element and sets it indices.
  Standard_EXPORT Poly_Element (const Standard_Integer theTriangle1,
                                const Standard_Integer theTriangle2);

  //! Sets the value of triangles indices.
  Standard_EXPORT void Set (const Standard_Integer theTriangle1,
                            const Standard_Integer theTriangle2);

  //! Returns the triangles indices of this element in theTriangle1, theTriangle2.
  Standard_EXPORT void Get (Standard_Integer& theTriangle1,
                            Standard_Integer& theTriangle2) const;

  //! @return the triangle index of given element theIndex.
  //! Raises OutOfRange from Standard if theIndex is not in 1,2.
  Standard_Integer Value (const Standard_Integer theIndex) const
  {
    Standard_OutOfRange_Raise_if (theIndex < 1 || theIndex > 2, NULL);
    return myTriangles[theIndex - 1];
  }

  Standard_Integer operator() (const Standard_Integer theIndex) const { return Value (theIndex); }

  //! @return the triangle index of given element theIndex.
  //! Raises OutOfRange from Standard if theIndex is not in 1,2.
  Standard_Integer& ChangeValue (const Standard_Integer theIndex)
  {
    Standard_OutOfRange_Raise_if (theIndex < 1 || theIndex > 2, NULL);
    return myTriangles[theIndex - 1];
  }

  Standard_Integer& operator() (const Standard_Integer theIndex) { return ChangeValue (theIndex); }

  //! @return Standard_True if the first element index > 0 and the second index == 0.
  Standard_Boolean IsTriangle() const
  {
    return (myTriangles[0] > 0 && myTriangles[1] == 0);
  }

  //! @return Standard_True if the first and the second element indices > 0.
  Standard_Boolean IsQuad() const
  {
    return (myTriangles[0] > 0 && myTriangles[1] > 0);
  }

private:

  Standard_Integer myTriangles[2];

};

#endif // _Poly_Element_HeaderFile
