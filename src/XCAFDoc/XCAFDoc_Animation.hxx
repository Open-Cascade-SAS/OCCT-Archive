// Created on: 2017-10-02
// Created by: Elena MOZOKHINA
// Copyright (c) 2016 OPEN CASCADE SAS
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

#ifndef _XCAFDoc_Animation_HeaderFile
#define _XCAFDoc_Animation_HeaderFile

#include <Standard.hxx>
#include <Standard_Type.hxx>

#include <OSD_File.hxx>
#include <TDF_Attribute.hxx>
class Standard_GUID;
class TDF_Label;
class TDF_Attribute;
class TDF_RelocationTable;
class XCAFView_Object;

class XCAFDoc_Animation;
class TCollection_HAsciiString;
class TColStd_HArray1OfByte;

DEFINE_STANDARD_HANDLE(XCAFDoc_Animation, TDF_Attribute)

//! attribute to store animation
class XCAFDoc_Animation : public TDF_Attribute
{

public:

  Standard_EXPORT XCAFDoc_Animation();
  
  Standard_EXPORT static const Standard_GUID& GetID();
  
  Standard_EXPORT static Handle(XCAFDoc_Animation) Set(const TDF_Label& label, const Handle(TCollection_HAsciiString)& theName, const Handle(TColStd_HArray1OfByte)& theImage, const Handle(TColStd_HArray1OfByte)& theAnimation);
  
  Standard_EXPORT static Handle(XCAFDoc_Animation) Set(const TDF_Label& theLabel, const Handle(TCollection_HAsciiString)& theName, OSD_File& theImageFile, OSD_File& theAnimationFile);

  Standard_EXPORT void Set(const Handle(TCollection_HAsciiString)& theName, const Handle(TColStd_HArray1OfByte)& theImage, const Handle(TColStd_HArray1OfByte)& theAnimation);
 
  Standard_EXPORT Standard_Boolean Set(const Handle(TCollection_HAsciiString)& aName, OSD_File&  theImageFile, OSD_File& theAnimationFile);

  Standard_EXPORT void Set(const Handle(TCollection_HAsciiString)& theName);

  Standard_EXPORT Handle(TCollection_HAsciiString) GetName() const;

  Standard_EXPORT Handle(TColStd_HArray1OfByte) GetImage() const;

  Standard_EXPORT Handle(TColStd_HArray1OfByte) GetAnimation() const;
  
  Standard_EXPORT const Standard_GUID& ID() const Standard_OVERRIDE;

  Standard_EXPORT void Restore(const Handle(TDF_Attribute)& With) Standard_OVERRIDE;

  Standard_EXPORT Handle(TDF_Attribute) NewEmpty() const Standard_OVERRIDE;

  Standard_EXPORT void Paste(const Handle(TDF_Attribute)& Into, const Handle(TDF_RelocationTable)& RT) const Standard_OVERRIDE;

  


  DEFINE_STANDARD_RTTIEXT(XCAFDoc_Animation, TDF_Attribute)

private:

  Handle(TCollection_HAsciiString) myName;
  Handle(TColStd_HArray1OfByte) myImage;
  Handle(TColStd_HArray1OfByte) myAnimation;
};

#endif
