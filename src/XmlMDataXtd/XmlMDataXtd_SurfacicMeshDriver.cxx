// Created on: 2015-12-15
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

#include <XmlMDataXtd_SurfacicMeshDriver.hxx>
#include <Message_Messenger.hxx>
#include <NCollection_LocalArray.hxx>
#include <Standard_Type.hxx>
#include <TDF_Attribute.hxx>
#include <XmlObjMgt.hxx>
#include <XmlObjMgt_Persistent.hxx>
#include <TDataXtd_SurfacicMesh.hxx>
#include <LDOM_OSStream.hxx>
#include <BRepTools_ShapeSet.hxx>

IMPLEMENT_STANDARD_RTTIEXT(XmlMDataXtd_SurfacicMeshDriver,XmlMDF_ADriver)
IMPLEMENT_DOMSTRING (MeshString, "mesh")
IMPLEMENT_DOMSTRING (NullString, "null")
IMPLEMENT_DOMSTRING (ExistString, "exists")

//=======================================================================
//function : XmlMDataXtd_SurfacicMeshDriver
//purpose  : Constructor
//=======================================================================
XmlMDataXtd_SurfacicMeshDriver::XmlMDataXtd_SurfacicMeshDriver (const Handle(Message_Messenger)& theMsgDriver)
     : XmlMDF_ADriver (theMsgDriver, NULL)
{

}

//=======================================================================
//function : NewEmpty
//purpose  : 
//=======================================================================
Handle(TDF_Attribute) XmlMDataXtd_SurfacicMeshDriver::NewEmpty() const
{
  return new TDataXtd_SurfacicMesh();
}

//=======================================================================
//function : Paste
//purpose  : persistent -> transient (retrieve)
//=======================================================================
Standard_Boolean XmlMDataXtd_SurfacicMeshDriver::Paste (const XmlObjMgt_Persistent&  theSource,
                                                        const Handle(TDF_Attribute)& theTarget,
                                                        XmlObjMgt_RRelocationTable&  ) const
{
  const XmlObjMgt_Element& anElement = theSource;
  Handle(TDataXtd_SurfacicMesh) attrMesh = Handle(TDataXtd_SurfacicMesh)::DownCast (theTarget);

  // Read the FirstIndex; if the attribute is absent initialize to 1
  XmlObjMgt_DOMString aMeshStatus = anElement.getAttribute (::MeshString());
  if (aMeshStatus == NULL ||
      aMeshStatus.Type() != LDOMBasicString::LDOM_AsciiDoc ||
      strcmp (aMeshStatus.GetString(), ::ExistString().GetString())) 
  {
    // No mesh.
    return Standard_True;
  }

  // Get mesh as a string.
  const XmlObjMgt_DOMString& aData = XmlObjMgt::GetStringValue (anElement);
  std::stringstream aStream (std::string (aData.GetString()));

  // Read the mesh.
  BRepTools_ShapeSet aShapeSet;
  TColStd_IndexedMapOfTransient aMeshes;
  aShapeSet.ReadMeshes (aStream, aMeshes);

  // Set mesh.
  if (!aMeshes.IsEmpty()) {
    // We expect only one mesh.
    Handle(Poly_Mesh) aMesh = Handle(Poly_Mesh)::DownCast (aMeshes (1));
    if (!aMesh.IsNull())
      attrMesh->Set (aMesh);
  }

  return Standard_True;
}

//=======================================================================
//function : Paste
//purpose  : transient -> persistent (store)
//=======================================================================
void XmlMDataXtd_SurfacicMeshDriver::Paste (const Handle(TDF_Attribute)& theSource,
                                            XmlObjMgt_Persistent&        theTarget,
                                            XmlObjMgt_SRelocationTable&  ) const
{
  const Handle(TDataXtd_SurfacicMesh) meshAttr = Handle(TDataXtd_SurfacicMesh)::DownCast (theSource);
  if (meshAttr->Get().IsNull())
    theTarget.Element().setAttribute (::MeshString(), ::NullString());
  else
  {
    theTarget.Element().setAttribute (::MeshString(), ::ExistString());
    
    // Analyse the size of the mesh
    // (to allocate properly the string array).
    const Handle(Poly_Mesh)& aMesh = meshAttr->Get();
    Standard_Integer aSize = aMesh->NbNodes();
    aSize *= 3; // 3 coordinates for a node
    aSize *= 8; // 8 characters are used to represent a coordinate (double) in XML
    aSize += 3 * 5 * aMesh->NbTriangles(); // space for triangles   (3 integers of 5 symbols)
    aSize += 4 * 5 * aMesh->NbQuads();     // space for quadrangles (4 integers of 5 symbols)
    aSize *= 2; // just in case :-)
    if (!aSize)
      aSize = 1;

    // Allocate a string stream.
    LDOM_OSStream aStream (aSize);

    // Write the mesh.
    BRepTools_ShapeSet aShapeSet;
    TColStd_IndexedMapOfTransient aMeshes;
    aMeshes.Add (aMesh);
    aShapeSet.WriteMeshes (aStream, aMeshes, Standard_True/*compact*/);
    aStream << std::ends;

    Standard_Character* aDump = (Standard_Character*) aStream.str(); // copying! Don't forget to delete it.
    XmlObjMgt::SetStringValue (theTarget, aDump, Standard_True);
    delete[] aDump;
  }
}
