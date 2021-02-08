// Created on: 2015-12-10
// Created by: Vlad Romashko
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

#include <TDataXtd_SurfacicMesh.hxx>
#include <Standard_GUID.hxx>
#include <Standard_Type.hxx>
#include <TDF_Attribute.hxx>
#include <TDF_Label.hxx>
#include <TDF_RelocationTable.hxx>

//=======================================================================
//function : GetID
//purpose  : 
//=======================================================================
const Standard_GUID& TDataXtd_SurfacicMesh::GetID() 
{ 
  static Standard_GUID TDataXtd_SurfacicMeshID ("D7E3F1CF-38A4-4DCA-94F4-51C31F3FCBA5");
  return TDataXtd_SurfacicMeshID; 
}

//=======================================================================
//function : SetAttr
//purpose  : 
//=======================================================================
static Handle(TDataXtd_SurfacicMesh) SetAttr (const TDF_Label& theLabel, 
                                      const Standard_GUID& theID)
{
  Handle(TDataXtd_SurfacicMesh) hMesh;
  if (!theLabel.FindAttribute (theID, hMesh)) {
    hMesh = new TDataXtd_SurfacicMesh();
    hMesh->SetID (theID);
    theLabel.AddAttribute (hMesh);
  }
  return hMesh;
}

//=======================================================================
//function : Set
//purpose  : 
//=======================================================================
Handle(TDataXtd_SurfacicMesh) TDataXtd_SurfacicMesh::Set (const TDF_Label& theLabel)
{
  return SetAttr (theLabel, GetID());
}

//=======================================================================
//function : Set
//purpose  : 
//=======================================================================
Handle(TDataXtd_SurfacicMesh) TDataXtd_SurfacicMesh::Set (const TDF_Label& theLabel,
                                          const Standard_GUID& theID)
{
  return SetAttr (theLabel, theID);
}

//=======================================================================
//function : Set
//purpose  : 
//=======================================================================
Handle(TDataXtd_SurfacicMesh) TDataXtd_SurfacicMesh::Set (const TDF_Label& theLabel,
                                          const Handle(Poly_Mesh)& theMesh)
{
  Handle(TDataXtd_SurfacicMesh) hMesh = Set (theLabel);
  hMesh->Set (theMesh);
  return hMesh;
}

//=======================================================================
//function : Set
//purpose  : 
//=======================================================================
Handle(TDataXtd_SurfacicMesh) TDataXtd_SurfacicMesh::Set (const TDF_Label& theLabel,
                                          const Standard_GUID& theID,
                                          const Handle(Poly_Mesh)& theMesh)
{
  Handle(TDataXtd_SurfacicMesh) hMesh = Set (theLabel, theID);
  hMesh->Set (theMesh);
  return hMesh;
}

//=======================================================================
//function : TDataXtd_SurfacicMesh
//purpose  : 
//=======================================================================
TDataXtd_SurfacicMesh::TDataXtd_SurfacicMesh():myID (GetID())
{

}

//=======================================================================
//function : TDataXtd_SurfacicMesh
//purpose  : 
//=======================================================================
void TDataXtd_SurfacicMesh::Set (const Handle(Poly_Mesh)& theMesh)
{
  Backup();
  myMesh = theMesh;
}

//=======================================================================
//function : TDataXtd_SurfacicMesh
//purpose  : 
//=======================================================================
const Handle(Poly_Mesh)& TDataXtd_SurfacicMesh::Get() const
{
  return myMesh;
}

// Poly_Mesh methods

//=======================================================================
//function : Deflection
//purpose  : 
//=======================================================================
Standard_Real TDataXtd_SurfacicMesh::Deflection() const
{
  return myMesh->Deflection();
}

//=======================================================================
//function : Deflection
//purpose  : 
//=======================================================================
void TDataXtd_SurfacicMesh::Deflection (const Standard_Real theDeflection)
{
  Backup();
  myMesh->Deflection (theDeflection);
}

//=======================================================================
//function : RemoveUVNodes
//purpose  : 
//=======================================================================
void TDataXtd_SurfacicMesh::RemoveUVNodes()
{
  Backup();
  myMesh->RemoveUVNodes();
}

//=======================================================================
//function : NbNodes
//purpose  : 
//=======================================================================
Standard_Integer TDataXtd_SurfacicMesh::NbNodes() const
{
  return myMesh->NbNodes();
}

//=======================================================================
//function : NbTriangles
//purpose  : 
//=======================================================================
Standard_Integer TDataXtd_SurfacicMesh::NbTriangles() const
{
  return myMesh->NbTriangles();
}

//=======================================================================
//function : HasUVNodes
//purpose  : 
//=======================================================================
Standard_Boolean TDataXtd_SurfacicMesh::HasUVNodes() const
{
  return myMesh->HasUVNodes();
}

//=======================================================================
//function : AddNode
//purpose  : 
//=======================================================================
Standard_Integer TDataXtd_SurfacicMesh::AddNode (const gp_Pnt& theNode)
{
  Backup();
  return myMesh->AddNode (theNode);
}

//=======================================================================
//function : Node
//purpose  : 
//=======================================================================
const gp_Pnt& TDataXtd_SurfacicMesh::Node (const Standard_Integer theIndex) const
{
  return myMesh->Node (theIndex);
}

//=======================================================================
//function : SetNode
//purpose  : 
//=======================================================================
void TDataXtd_SurfacicMesh::SetNode (const Standard_Integer theIndex, const gp_Pnt& theNode)
{
  Backup();
  myMesh->ChangeNode (theIndex) = theNode;
}

//=======================================================================
//function : UVNode
//purpose  : 
//=======================================================================
const gp_Pnt2d& TDataXtd_SurfacicMesh::UVNode (const Standard_Integer theIndex) const
{
  return myMesh->UVNode (theIndex);
}

//=======================================================================
//function : SetUVNode
//purpose  : 
//=======================================================================
void TDataXtd_SurfacicMesh::SetUVNode (const Standard_Integer theIndex, const gp_Pnt2d& theUVNode)
{
  Backup();
  myMesh->ChangeUVNode (theIndex) = theUVNode;
}

//=======================================================================
//function : AddTriangle
//purpose  : 
//=======================================================================
Standard_Integer TDataXtd_SurfacicMesh::AddTriangle (const Poly_Triangle& theTriangle)
{
  Backup();
  return myMesh->AddTriangle (theTriangle);
}

//=======================================================================
//function : Triangle
//purpose  : 
//=======================================================================
const Poly_Triangle& TDataXtd_SurfacicMesh::Triangle (const Standard_Integer theIndex) const
{
  return myMesh->Triangle (theIndex);
}

//=======================================================================
//function : SetTriangle
//purpose  : 
//=======================================================================
void TDataXtd_SurfacicMesh::SetTriangle (const Standard_Integer theIndex, const Poly_Triangle& theTriangle)
{
  Backup();
  myMesh->ChangeTriangle (theIndex) = theTriangle;
}

//=======================================================================
//function : SetNormal
//purpose  : 
//=======================================================================
void TDataXtd_SurfacicMesh::SetNormal (const Standard_Integer theIndex,
                               const gp_Dir&          theNormal)
{
  Backup();
  myMesh->SetNormal (theIndex, theNormal);
}

//=======================================================================
//function : HasNormals
//purpose  : 
//=======================================================================
Standard_Boolean TDataXtd_SurfacicMesh::HasNormals() const
{
  return myMesh->HasNormals();
}

//=======================================================================
//function : Normal
//purpose  : 
//=======================================================================
const gp_Dir TDataXtd_SurfacicMesh::Normal (const Standard_Integer theIndex) const
{
  return myMesh->Normal (theIndex);
}

//=======================================================================
//function : AddElement
//purpose  :
//=======================================================================
Standard_Integer TDataXtd_SurfacicMesh::AddElement (const Standard_Integer theN1,
                                            const Standard_Integer theN2,
                                            const Standard_Integer theN3)
{
  Backup();
  return myMesh->AddElement (theN1, theN2, theN3);
}

//=======================================================================
//function : AddElement
//purpose  :
//=======================================================================
Standard_Integer TDataXtd_SurfacicMesh::AddElement (const Standard_Integer theN1,
                                            const Standard_Integer theN2,
                                            const Standard_Integer theN3,
                                            const Standard_Integer theN4)
{
  Backup();
  return myMesh->AddElement (theN1, theN2, theN3, theN4);
}

//=======================================================================
//function : NbElements
//purpose  : 
//=======================================================================
Standard_Integer TDataXtd_SurfacicMesh::NbElements() const
{
  return myMesh->NbElements();
}

//=======================================================================
//function : NbQuads
//purpose  : 
//=======================================================================
Standard_Integer TDataXtd_SurfacicMesh::NbQuads() const
{
  return myMesh->NbQuads();
}

//=======================================================================
//function : Element
//purpose  : 
//=======================================================================
const Poly_Element& TDataXtd_SurfacicMesh::Element (const Standard_Integer theIndex) const
{
  return myMesh->Element (theIndex);
}

//=======================================================================
//function : Element
//purpose  : 
//=======================================================================
void TDataXtd_SurfacicMesh::Element (const Standard_Integer theIndex,
                             Standard_Integer& theN1,
                             Standard_Integer& theN2,
                             Standard_Integer& theN3,
                             Standard_Integer& theN4) const
{
  myMesh->Element (theIndex, theN1, theN2, theN3, theN4);
}

//=======================================================================
//function : SetElement
//purpose  : 
//=======================================================================
void TDataXtd_SurfacicMesh::SetElement (const Standard_Integer theIndex, const Poly_Element& theElement)
{
  Backup();
  myMesh->SetElement (theIndex, theElement);
}

//=======================================================================
//function : SetID
//purpose  :
//=======================================================================
void TDataXtd_SurfacicMesh::SetID (const Standard_GUID& theID)
{
  if (myID != theID) {
    Backup();
    myID = theID;
  }
}

//=======================================================================
//function : SetID
//purpose  : 
//=======================================================================
void TDataXtd_SurfacicMesh::SetID()
{
  Backup();
  myID = GetID();
}

//=======================================================================
//function : ID
//purpose  : 
//=======================================================================
const Standard_GUID& TDataXtd_SurfacicMesh::ID () const 
{ 
  return myID; 
}

//=======================================================================
//function : NewEmpty
//purpose  : 
//=======================================================================
Handle(TDF_Attribute) TDataXtd_SurfacicMesh::NewEmpty () const
{  
  return new TDataXtd_SurfacicMesh(); 
}

//=======================================================================
//function : Restore
//purpose  : 
//=======================================================================
void TDataXtd_SurfacicMesh::Restore (const Handle(TDF_Attribute)& theWithMesh) 
{
  myMesh.Nullify();
  Handle(TDataXtd_SurfacicMesh) withMesh = Handle(TDataXtd_SurfacicMesh)::DownCast (theWithMesh);
  if (!withMesh->myMesh.IsNull()) {
    Handle(Poly_Triangulation) withTris = withMesh->myMesh->Copy();
    if (!withTris.IsNull())
      myMesh = Handle(Poly_Mesh)::DownCast (withTris);
  }
}

//=======================================================================
//function : Paste
//purpose  : 
//=======================================================================
void TDataXtd_SurfacicMesh::Paste (const Handle(TDF_Attribute)& theIntoMesh,
                           const Handle(TDF_RelocationTable)& ) const
{
  Handle(TDataXtd_SurfacicMesh) intoMesh = Handle(TDataXtd_SurfacicMesh)::DownCast (theIntoMesh);
  intoMesh->myMesh.Nullify();
  if (!myMesh.IsNull()) {
    Handle(Poly_Triangulation) aTris = myMesh->Copy();
    if (!aTris.IsNull())
      intoMesh->myMesh = Handle(Poly_Mesh)::DownCast (aTris);
  }
}

//=======================================================================
//function : Dump
//purpose  : 
//=======================================================================
Standard_OStream& TDataXtd_SurfacicMesh::Dump (Standard_OStream& theOS) const
{  
  theOS << "Mesh: ";

  Standard_Character aStrID[Standard_GUID_SIZE_ALLOC];
  myID.ToCString (aStrID);
  theOS << aStrID;

  if (!myMesh.IsNull()) {
    theOS << "\n\tDeflection: " << myMesh->Deflection();
    theOS << "\n\tNodes: " << myMesh->NbNodes();
    theOS << "\n\tTriangles: " << myMesh->NbTriangles();
    if (myMesh->HasUVNodes())
      theOS << "\n\tHas UV-Nodes";
    else
      theOS << "\n\tNo UV-Nodes";
    if (myMesh->HasNormals())
      theOS << "\n\tHas normals";
    else
      theOS << "\n\tNo normals";
  }

  theOS << "\nAttribute fields: ";
  TDF_Attribute::Dump (theOS);
  return theOS;
}

//=======================================================================
//function : DumpJson
//purpose  : 
//=======================================================================
void TDataXtd_SurfacicMesh::DumpJson (Standard_OStream& theOStream, Standard_Integer theDepth) const
{
  OCCT_DUMP_TRANSIENT_CLASS_BEGIN(theOStream)
  OCCT_DUMP_BASE_CLASS(theOStream, theDepth, TDF_Attribute)
  if (!myMesh.IsNull())
    myMesh->DumpJson (theOStream, theDepth);
}
