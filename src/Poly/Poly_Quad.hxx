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

#ifndef _Poly_Quad_HeaderFile
#define _Poly_Quad_HeaderFile

#include <Standard_DefineAlloc.hxx>
#include <Standard_Integer.hxx>
#include <Poly_Triangle.hxx>

//! Describes a quadric element of a mesh.
class Poly_Quad : public Poly_Triangle
{

public:

  DEFINE_STANDARD_ALLOC

  //! Constructs an empty Quad object.
  Poly_Quad():myNode4 (0) {
    myNodes[0] = myNodes[1] = myNodes[2] = 0;
  }

  //! Constructs a quadric element.
  Poly_Quad (const Standard_Integer theNode1,
             const Standard_Integer theNode2,
             const Standard_Integer theNode3,
             const Standard_Integer theNode4)
  {
    Set (theNode1, theNode2, theNode3, theNode4);
  }

  //! Sets indices of nodes.
  void Set (const Standard_Integer theNode1,
            const Standard_Integer theNode2,
            const Standard_Integer theNode3,
            const Standard_Integer theNode4)
  {
    myNodes[0] = theNode1;
    myNodes[1] = theNode2;
    myNodes[2] = theNode3;
    myNode4 = theNode4;
  }

  //! Returns indices of nodes of the quadric element.
  void Get (Standard_Integer& theNode1,
            Standard_Integer& theNode2,
            Standard_Integer& theNode3,
            Standard_Integer& theNode4) const
  {
    theNode1 = myNodes[0];
    theNode2 = myNodes[1];
    theNode3 = myNodes[2];
    theNode4 = myNode4;
  }

protected:

  Standard_Integer myNode4;

};

#endif // _Poly_Quad_HeaderFile
