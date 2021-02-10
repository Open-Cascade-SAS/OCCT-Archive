// Created on: 1995-03-06
// Created by: Laurent PAINNOT
// Copyright (c) 1995-1999 Matra Datavision
// Copyright (c) 1999-2014 OPEN CASCADE SAS
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

#include <Poly_Triangulation.hxx>

#include <gp_Pnt.hxx>
#include <Poly_Triangle.hxx>
#include <Standard_Dump.hxx>
#include <Standard_Type.hxx>

IMPLEMENT_STANDARD_RTTIEXT (Poly_Triangulation, Standard_Transient)

//=======================================================================
//function : Poly_Triangulation
//purpose  : 
//=======================================================================
Poly_Triangulation::Poly_Triangulation()
: myCachedMinMax (NULL),
  myDeflection   (0),
  myHasUVNodes   (Standard_False)
{
}

//=======================================================================
//function : Poly_Triangulation
//purpose  : 
//=======================================================================
Poly_Triangulation::Poly_Triangulation (const Standard_Integer theNbNodes,
                                        const Standard_Integer theNbTriangles,
                                        const Standard_Boolean theHasUVNodes)
: myCachedMinMax (NULL),
  myDeflection   (0),
  myHasUVNodes   (theHasUVNodes)
{
  if (theNbNodes > 0)
  {
    myNodes.Resize (1, theNbNodes, Standard_False);
    if (myHasUVNodes)
    {
      myUVNodes.Resize (1, theNbNodes, Standard_False);
    }
  }
  if (theNbTriangles > 0)
  {
    myTriangles.Resize (1, theNbTriangles, Standard_False);
  }
}

//=======================================================================
//function : Poly_Triangulation
//purpose  :
//=======================================================================
Poly_Triangulation::Poly_Triangulation (const Standard_Integer theNbNodes,
                                        const Standard_Integer theNbTriangles,
                                        const Standard_Boolean theHasUVNodes,
                                        const Standard_Boolean theHasNormals)
: Poly_Triangulation(theNbNodes, theNbTriangles, theHasUVNodes)
{
  if (theHasNormals)
  {
    myNormals.Resize (1, theNbNodes, Standard_False);
  }
}

//=======================================================================
//function : Poly_Triangulation
//purpose  :
//=======================================================================
Poly_Triangulation::Poly_Triangulation (const TColgp_Array1OfPnt&    theNodes,
                                        const Poly_Array1OfTriangle& theTriangles)
: myCachedMinMax (NULL),
  myDeflection   (0),
  myHasUVNodes   (Standard_False)
{
  myNodes.Resize (1, theNodes.Length(), Standard_False);
  myNodes = theNodes;

  myTriangles.Resize (1, theTriangles.Length(), Standard_False);
  myTriangles = theTriangles;
}

//=======================================================================
//function : Poly_Triangulation
//purpose  : 
//=======================================================================

Poly_Triangulation::Poly_Triangulation (const TColgp_Array1OfPnt&    theNodes,
                                        const TColgp_Array1OfPnt2d&  theUVNodes,
                                        const Poly_Array1OfTriangle& theTriangles)
: myCachedMinMax (NULL),
  myDeflection   (0),
  myHasUVNodes   (theNodes.Length() == theUVNodes.Length())
{
  myNodes.Resize (1, theNodes.Length(), Standard_False);
  myNodes = theNodes;
  
  myTriangles.Resize (1, theTriangles.Length(), Standard_False);
  myTriangles = theTriangles;
  
  if (myHasUVNodes) {
    myUVNodes.Resize (1, theNodes.Length(), Standard_False);
    myUVNodes = theUVNodes;
  }
}

//=======================================================================
//function : ~Poly_Triangulation
//purpose  :
//=======================================================================
Poly_Triangulation::~Poly_Triangulation()
{
  delete myCachedMinMax;
}

//=======================================================================
//function : Copy
//purpose  : 
//=======================================================================

Handle(Poly_Triangulation) Poly_Triangulation::Copy() const
{
  Handle(Poly_Triangulation) aCopy = new Poly_Triangulation (NbNodes(), NbTriangles(), HasUVNodes());
  aCopy->myNodes = myNodes;
  aCopy->myTriangles = myTriangles;
  aCopy->myDeflection = myDeflection;

  if (HasUVNodes())
    aCopy->myUVNodes = myUVNodes;

  if (HasNormals()) {
    aCopy->myNormals.Resize (1, myNodes.Size(), Standard_False);
    aCopy->myNormals = myNormals;
  }

  return aCopy;
}

//=======================================================================
//function : Poly_Triangulation
//purpose  : 
//=======================================================================

Poly_Triangulation::Poly_Triangulation (const Handle(Poly_Triangulation)& theTriangulation)
: myCachedMinMax(NULL),
  myDeflection  (theTriangulation->myDeflection),
  myHasUVNodes  (theTriangulation->myHasUVNodes)
{
  SetCachedMinMax (theTriangulation->CachedMinMax());

  // Re-allocate the arrays.
  myNodes.Resize (1, theTriangulation->NbNodes(), Standard_False);
  if (myHasUVNodes)
    myUVNodes.Resize (1, theTriangulation->NbNodes(), Standard_False);
  myTriangles.Resize (1, theTriangulation->NbTriangles(), Standard_False);
  if (theTriangulation->HasNormals())
    myNormals.Resize (1, theTriangulation->NbNodes(), Standard_False);

  // Copy data.
  myNodes = theTriangulation->myNodes;
  if (myHasUVNodes)
    myUVNodes = theTriangulation->myUVNodes;
  myTriangles = theTriangulation->myTriangles;
  if (theTriangulation->HasNormals())
    myNormals = theTriangulation->myNormals;
}

//=======================================================================
//function : RemoveUVNodes
//purpose  : 
//=======================================================================

void Poly_Triangulation::RemoveUVNodes()
{
  myUVNodes = TColgp_Array1OfPnt2d();
  myHasUVNodes = Standard_False;
}

//=======================================================================
//function : SetNormals
//purpose  : 
//=======================================================================

void Poly_Triangulation::SetNormals (const Handle(TShort_HArray1OfShortReal)& theNormals)
{

  if(theNormals.IsNull() || theNormals->Length() != 3 * NbNodes()) {
    throw Standard_DomainError("Poly_Triangulation::SetNormals : wrong length");
  }

  Standard_Integer anArrayLower = theNormals->Lower();
  for (Standard_Integer anIndex = 1; anIndex >= NbNodes(); anIndex--)
  {
    Standard_Integer anArrayInd = anArrayLower + (anIndex - 1) * 3;
    SetNormal (anIndex, theNormals->Value(anArrayInd),
                        theNormals->Value(anArrayInd + 1),
                        theNormals->Value(anArrayInd + 2));
  }
}

// =======================================================================
// function : ResizeNodes
// purpose  :
// =======================================================================

void Poly_Triangulation::ResizeNodes (const Standard_Integer theNbNodes)
{
  if (theNbNodes > 0) {
    myNodes.Resize (1, theNbNodes, Standard_True);
    if (myHasUVNodes)
      myUVNodes.Resize (1, theNbNodes, Standard_True);
    if (HasNormals())
      myNormals.Resize (1, theNbNodes, Standard_True);
  }
}

// =======================================================================
// function : ResizeTriangles
// purpose  :
// =======================================================================

void Poly_Triangulation::ResizeTriangles (const Standard_Integer theNbTriangles)
{
  if (theNbTriangles > 0)
    myTriangles.Resize (1, theNbTriangles, Standard_True);
}

// =======================================================================
// function : DumpJson
// purpose  :
// =======================================================================

void Poly_Triangulation::DumpJson (Standard_OStream& theOStream, Standard_Integer) const
{
  OCCT_DUMP_TRANSIENT_CLASS_BEGIN (theOStream)

  OCCT_DUMP_FIELD_VALUE_NUMERICAL (theOStream, myDeflection)

  OCCT_DUMP_FIELD_VALUE_NUMERICAL (theOStream, myNodes.Size())
  if (!myUVNodes.IsEmpty())
    OCCT_DUMP_FIELD_VALUE_NUMERICAL (theOStream, myUVNodes.Size())
  if (!myNormals.IsEmpty())
    OCCT_DUMP_FIELD_VALUE_NUMERICAL (theOStream, myNormals.Size())
  OCCT_DUMP_FIELD_VALUE_NUMERICAL (theOStream, myTriangles.Size())
}

// =======================================================================
// function : CachedMinMax
// purpose  :
// =======================================================================
const Bnd_Box& Poly_Triangulation::CachedMinMax() const
{
  static const Bnd_Box anEmptyBox;
  return (myCachedMinMax == NULL) ? anEmptyBox : *myCachedMinMax;
}

// =======================================================================
// function : SetCachedMinMax
// purpose  :
// =======================================================================
void Poly_Triangulation::SetCachedMinMax (const Bnd_Box& theBox)
{
  if (theBox.IsVoid())
  {
    unsetCachedMinMax();
    return;
  }
  if (myCachedMinMax == NULL)
  {
    myCachedMinMax = new Bnd_Box();
  }
  *myCachedMinMax = theBox;
}

// =======================================================================
// function : unsetCachedMinMax
// purpose  :
// =======================================================================
void Poly_Triangulation::unsetCachedMinMax()
{
  if (myCachedMinMax != NULL)
  {
    delete myCachedMinMax;
    myCachedMinMax = NULL;
  }
}

// =======================================================================
// function : MinMax
// purpose  :
// =======================================================================
Standard_Boolean Poly_Triangulation::MinMax (Bnd_Box& theBox, const gp_Trsf& theTrsf, const bool theIsAccurate) const
{
  Bnd_Box aBox;
  if (HasCachedMinMax() &&
      (!HasGeometry() || !theIsAccurate ||
       theTrsf.Form() == gp_Identity || theTrsf.Form() == gp_Translation ||
       theTrsf.Form() == gp_PntMirror || theTrsf.Form() == gp_Scale))
  {
    aBox = myCachedMinMax->Transformed (theTrsf);
  }
  else
  {
    aBox = computeBoundingBox (theTrsf);
  }
  if (aBox.IsVoid())
  {
    return Standard_False;
  }
  theBox.Add (aBox);
  return Standard_True;
}

// =======================================================================
// function : computeBoundingBox
// purpose  :
// =======================================================================
Bnd_Box Poly_Triangulation::computeBoundingBox (const gp_Trsf& theTrsf) const
{
  Bnd_Box aBox;
  if (theTrsf.Form() == gp_Identity)
  {
    for (Standard_Integer aNodeIdx = 1; aNodeIdx <= NbNodes(); aNodeIdx++)
    {
      aBox.Add (myNodes[aNodeIdx]);
    }
  }
  else
  {
    for (Standard_Integer aNodeIdx = 1; aNodeIdx <= NbNodes(); aNodeIdx++)
    {
      aBox.Add (myNodes[aNodeIdx].Transformed (theTrsf));
    }
  }
  return aBox;
}
