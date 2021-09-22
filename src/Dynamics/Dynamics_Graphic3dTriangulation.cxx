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

#include <Dynamics_Graphic3dTriangulation.hxx>

#ifdef HAVE_BULLET

// =======================================================================
// function : Dynamics_Graphic3dTriangulation
// purpose  :
// =======================================================================
Dynamics_Graphic3dTriangulation::Dynamics_Graphic3dTriangulation (const Handle(Graphic3d_ArrayOfTriangles)& theTris)
: myPosOffset (0)
{
  if (!theTris.IsNull())
  {
    myVerts   = theTris->Attributes();
    myIndices = theTris->Indices();
  }

  if (myVerts.IsNull()
   || myIndices.IsNull()
   || !(myIndices->Stride == 2 || myIndices->Stride == 4))
  {
    throw Standard_ProgramError ("Dynamics_Graphic3dTriangulation constructor called with NULL object!");
  }

  bool hasPos = false;
  myPosOffset = 0;
  for (Standard_Integer anAttribIter = 0; anAttribIter < myVerts->NbAttributes; ++anAttribIter)
  {
    const Graphic3d_Attribute& anAttrib = myVerts->Attribute (anAttribIter);
    if (anAttrib.Id == Graphic3d_TOA_POS)
    {
      if (anAttrib.DataType != Graphic3d_TOD_VEC3
       && anAttrib.DataType != Graphic3d_TOD_VEC4)
      {
        throw Standard_ProgramError ("Dynamics_Graphic3dTriangulation constructor called with unsupported data!");
      }
      hasPos = true;
      break;
    }

    myPosOffset += Graphic3d_Attribute::Stride (anAttrib.DataType);
  }
  if (!hasPos)
  {
    throw Standard_ProgramError ("Dynamics_Graphic3dTriangulation constructor called with NULL object!");
  }
}

// =======================================================================
// function : ~Dynamics_Graphic3dTriangulation
// purpose  :
// =======================================================================
Dynamics_Graphic3dTriangulation::~Dynamics_Graphic3dTriangulation()
{
  //
}

// =======================================================================
// function : getLockedVertexIndexBase
// purpose  :
// =======================================================================
void Dynamics_Graphic3dTriangulation::getLockedVertexIndexBase (unsigned char** theVertexBase,
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

  theType        = PHY_FLOAT;
  theNbVerts     = myVerts->NbElements;
  theStride      = myVerts->Stride;
  *theVertexBase = myVerts->changeValue (0) + myPosOffset;

  theIndicesType = myIndices->Stride == 2 ? PHY_SHORT : PHY_INTEGER;
  *theIndexBase  = myIndices->changeValue (0);
  theNbFaces     = myIndices->NbElements / 3;
  theIndexStride = myIndices->Stride * 3;
}

// =======================================================================
// function : getLockedReadOnlyVertexIndexBase
// purpose  :
// =======================================================================
void Dynamics_Graphic3dTriangulation::getLockedReadOnlyVertexIndexBase (const unsigned char** theVertexBase,
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

  theType        = PHY_FLOAT;
  theNbVerts     = myVerts->NbElements;
  theStride      = myVerts->Stride;
  *theVertexBase = myVerts->value (0) + myPosOffset;

  theIndicesType = myIndices->Stride == 2 ? PHY_SHORT : PHY_INTEGER;
  *theIndexBase  = myIndices->value (0);
  theNbFaces     = myIndices->NbElements / 3;
  theIndexStride = myIndices->Stride * 3;
}

#endif // HAVE_BULLET
