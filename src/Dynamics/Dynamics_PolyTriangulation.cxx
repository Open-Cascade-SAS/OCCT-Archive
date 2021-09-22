// Copyright (c) 2017-2021 OPEN CASCADE SAS
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

#include <Dynamics_PolyTriangulation.hxx>

#ifdef HAVE_BULLET

// =======================================================================
// function : Dynamics_PolyTriangulation
// purpose  :
// =======================================================================
Dynamics_PolyTriangulation::Dynamics_PolyTriangulation (const Handle(Poly_Triangulation)& thePolyTri)
: myPolyTri (thePolyTri)
{
  if (myPolyTri.IsNull())
  {
    throw Standard_ProgramError ("Dynamics_PolyTriangulation constructor called with NULL object!");
  }
}

// =======================================================================
// function : ~Dynamics_PolyTriangulation
// purpose  :
// =======================================================================
Dynamics_PolyTriangulation::~Dynamics_PolyTriangulation()
{
  //
}

// =======================================================================
// function : getLockedVertexIndexBase
// purpose  :
// =======================================================================
void Dynamics_PolyTriangulation::getLockedVertexIndexBase (unsigned char** theVertexBase,
                                                           int& theNbVerts,
                                                           PHY_ScalarType& theType,
                                                           int& theStride,
                                                           unsigned char** theIndexBase,
                                                           int& theIndexStride,
                                                           int& theNbFaces,
                                                           PHY_ScalarType& theIndicesType,
                                                           int theSubPart)
{
  if (theSubPart != 0)
  {
    return;
  }

  theType        = myPolyTri->InternalNodes().IsDoublePrecision() ? PHY_DOUBLE : PHY_FLOAT;
  theNbVerts     = myPolyTri->NbNodes();
  theStride      = myPolyTri->InternalNodes().Stride();
  *theVertexBase = reinterpret_cast<unsigned char* >(myPolyTri->InternalNodes().changeValue (0))
                 - theStride; // step back since indexation starts with 1 in Poly_Triangulation

  theIndicesType = PHY_INTEGER;
  *theIndexBase  = reinterpret_cast<unsigned char* >(&myPolyTri->InternalTriangles().ChangeFirst());
  theNbFaces     = myPolyTri->NbTriangles();
  theIndexStride = sizeof(Poly_Triangle);
}

// =======================================================================
// function : getLockedReadOnlyVertexIndexBase
// purpose  :
// =======================================================================
void Dynamics_PolyTriangulation::getLockedReadOnlyVertexIndexBase (const unsigned char** theVertexBase,
                                                                   int& theNbVerts,
                                                                   PHY_ScalarType& theType,
                                                                   int& theStride,
                                                                   const unsigned char** theIndexBase,
                                                                   int& theIndexStride,
                                                                   int& theNbFaces,
                                                                   PHY_ScalarType& theIndicesType,
                                                                   int theSubPart) const
{
  if (theSubPart != 0)
  {
    return;
  }

  theType        = myPolyTri->InternalNodes().IsDoublePrecision() ? PHY_DOUBLE : PHY_FLOAT;
  theNbVerts     = myPolyTri->NbNodes();
  theStride      = myPolyTri->InternalNodes().Stride();
  *theVertexBase = reinterpret_cast<const unsigned char* >(myPolyTri->InternalNodes().changeValue (0))
                 - theStride; // step back since indexation starts with 1 in Poly_Triangulation

  theIndicesType = PHY_INTEGER;
  *theIndexBase  = reinterpret_cast<const unsigned char* >(&myPolyTri->InternalTriangles().First());
  theNbFaces     = myPolyTri->NbTriangles();
  theIndexStride = sizeof(Poly_Triangle);
}

#endif // HAVE_BULLET
