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

#include <Poly_Element.hxx>

//=======================================================================
//function : Poly_Element
//purpose  :
//=======================================================================

Poly_Element::Poly_Element()
{
  myTriangles[0] = myTriangles[1] = 0;
}

//=======================================================================
//function : Poly_Element
//purpose  :
//=======================================================================

Poly_Element::Poly_Element (const Standard_Integer theTriangle1,
                            const Standard_Integer theTriangle2)
{
  myTriangles[0] = theTriangle1;
  myTriangles[1] = theTriangle2;
}

//=======================================================================
//function : Set
//purpose  :
//=======================================================================

void Poly_Element::Set (const Standard_Integer theTriangle1,
                        const Standard_Integer theTriangle2)
{
  myTriangles[0] = theTriangle1;
  myTriangles[1] = theTriangle2;
}

//=======================================================================
//function : Get
//purpose  :
//=======================================================================

void Poly_Element::Get (Standard_Integer& theTriangle1,
                        Standard_Integer& theTriangle2) const
{
  theTriangle1 = myTriangles[0];
  theTriangle2 = myTriangles[1];
}
