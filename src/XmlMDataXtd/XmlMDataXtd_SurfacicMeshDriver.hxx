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

#ifndef _XmlMDataXtd_SurfacicMeshDriver_HeaderFile
#define _XmlMDataXtd_SurfacicMeshDriver_HeaderFile

#include <Standard.hxx>
#include <XmlMDF_ADriver.hxx>
#include <XmlObjMgt_RRelocationTable.hxx>
#include <XmlObjMgt_SRelocationTable.hxx>

class Message_Messenger;
class TDF_Attribute;
class XmlObjMgt_Persistent;

DEFINE_STANDARD_HANDLE(XmlMDataXtd_SurfacicMeshDriver, XmlMDF_ADriver)

//! TDataXtd_SurfacicMesh attribute XML-driver.
class XmlMDataXtd_SurfacicMeshDriver : public XmlMDF_ADriver
{

public:
  
  Standard_EXPORT XmlMDataXtd_SurfacicMeshDriver (const Handle(Message_Messenger)& theMessageDriver);
  
  Standard_EXPORT Handle(TDF_Attribute) NewEmpty() const Standard_OVERRIDE;
  
  Standard_EXPORT Standard_Boolean Paste (const XmlObjMgt_Persistent& theSource, 
                                          const Handle(TDF_Attribute)& theTarget, 
                                          XmlObjMgt_RRelocationTable& theRelocTable) const Standard_OVERRIDE;
  
  Standard_EXPORT void Paste (const Handle(TDF_Attribute)& theSource, XmlObjMgt_Persistent& theTarget, 
                              XmlObjMgt_SRelocationTable& RelocTable) const Standard_OVERRIDE;

  DEFINE_STANDARD_RTTIEXT(XmlMDataXtd_SurfacicMeshDriver, XmlMDF_ADriver)
};

#endif // _XmlMDataXtd_SurfacicMeshDriver_HeaderFile
