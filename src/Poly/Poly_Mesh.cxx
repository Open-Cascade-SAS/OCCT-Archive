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

Poly_Mesh::Poly_Mesh (const Standard_Boolean theHasUVNodes)
: Poly_Triangulation (0, 0, theHasUVNodes),
  myNbQuads (0)
{}

//=======================================================================
//function : Poly_Mesh
//purpose  :
//=======================================================================

Poly_Mesh::Poly_Mesh (const Handle(Poly_Triangulation)& theTriangulation)
: Poly_Triangulation ( theTriangulation ),
  myNbQuads (0)
{
  const Standard_Integer aNbTris = theTriangulation->NbTriangles();
  
  // Fill collection of elements
  if ( aNbTris )
    myElements.SetValue( aNbTris - 1, Poly_Element() );

  // Populate elements with triangles
  for ( Standard_Integer i = 1; i <= aNbTris; ++i )
  {
    myElements(i - 1).Set(i, 0);
  }
}

//=======================================================================
//function : Copy
//purpose  :
//=======================================================================

Handle(Poly_Triangulation) Poly_Mesh::Copy() const
{
  const Standard_Boolean hasUV = HasUVNodes();
  Handle(Poly_Mesh) aCopy = new Poly_Mesh(hasUV);
  // Copy nodes
  Standard_Integer aNbNodes = NbNodes();
  for ( Standard_Integer i = 1; i <= aNbNodes; ++i )
  {
    aCopy->AddNode(Node(i));
    if ( hasUV )
      aCopy->ChangeUVNode(i) = UVNode(i);
  }
  // Copy triangles
  Standard_Integer aNbTriangles = NbTriangles();
  const Standard_Boolean hasNormals = HasNormals();
  for ( Standard_Integer i = 1; i <= aNbTriangles; ++i )
  {
    aCopy->AddTriangle(Triangle(i));
    // Pass normal vector (if any)
    if ( hasNormals )
      aCopy->SetNormal(i, Normal(i));
  }
  // Copy quads
  aCopy->myNbQuads = myNbQuads;
  aCopy->myElements = myElements;
  return aCopy;
}

//=======================================================================
//function : AddElement
//purpose  :
//=======================================================================

Standard_Integer Poly_Mesh::AddElement (const Standard_Integer theN1,
                                        const Standard_Integer theN2,
                                        const Standard_Integer theN3)
{
  Standard_Integer anIndex = Poly_Triangulation::AddTriangle( Poly_Triangle(theN1, theN2, theN3) );
  return addElement( Poly_Element(anIndex, 0) );
}

//=======================================================================
//function : AddElement
//purpose  :
//=======================================================================

Standard_Integer Poly_Mesh::AddElement (const Standard_Integer theN1,
                                        const Standard_Integer theN2,
                                        const Standard_Integer theN3,
                                        const Standard_Integer theN4)
{
  Standard_Integer anIndex1 = Poly_Triangulation::AddTriangle( Poly_Triangle(theN1, theN2, theN3) );
  Standard_Integer anIndex2 = Poly_Triangulation::AddTriangle( Poly_Triangle(theN1, theN3, theN4) );
  return addElement( Poly_Element(anIndex1, anIndex2) );
}

//=======================================================================
//function : Element
//purpose  :
//=======================================================================

const Poly_Element& Poly_Mesh::Element (const Standard_Integer theIndex) const
{
  if ( theIndex < 1 || theIndex > myElements.Size() )
  {
    Standard_OutOfRange::Raise("Poly_Mesh::Element : index out of range");
  }

  return myElements.Value(theIndex - 1);
}

//=======================================================================
//function : Element
//purpose  :
//=======================================================================

void Poly_Mesh::Element (const Standard_Integer theIndex,
                         Standard_Integer& theN1,
                         Standard_Integer& theN2,
                         Standard_Integer& theN3,
                         Standard_Integer& theN4) const
{
  if ( theIndex < 1 || theIndex > myElements.Size() )
  {
    Standard_OutOfRange::Raise("Poly_Mesh::Element : index out of range");
  }

  const Poly_Element& anElem = Element(theIndex);
  Standard_Integer aTriIdx1, aTriIdx2;
  anElem.Get(aTriIdx1, aTriIdx2);

  // Get node indices for the first triangle
  const Poly_Triangle& aTri1 = Poly_Triangulation::Triangle(aTriIdx1);
  aTri1.Get(theN1, theN2, theN3);

  // If the second triangle exists, take its node indices for quad
  if ( aTriIdx2 )
  {
    const Poly_Triangle& aTri2 = Poly_Triangulation::Triangle(aTriIdx2);
    aTri2.Get(theN1, theN3, theN4);
  }
  else
    theN4 = 0;
}

//=======================================================================
//function : SetElement
//purpose  :
//=======================================================================

void Poly_Mesh::SetElement (const Standard_Integer theIndex, const Poly_Element& theElement)
{
  if ( theIndex < 1 || theIndex > myElements.Size() )
  {
    Standard_OutOfRange::Raise("Poly_Mesh::SetElement : index out of range");
  }

  if ( myElements.Value(theIndex - 1).Value(2) == 0 && theElement.Value(2) != 0 )
  {
    myNbQuads++;
  }
  else if ( myElements.Value(theIndex - 1).Value(2) != 0 && theElement.Value(2) == 0 )
  {
    myNbQuads--;
  }

  myElements.SetValue(theIndex - 1, theElement);
}

//=======================================================================
//function : addElement
//purpose  :
//=======================================================================

Standard_Integer Poly_Mesh::addElement(const Poly_Element& theElement)
{
  myElements.Append(theElement);
  if ( theElement.Value(2) != 0 )
  {
    myNbQuads++;
  }
  return myElements.Size();
}
