// Created on: 2016-10-19
// Created by: Irina KRYLOVA
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

#include <XCAFDoc_View.hxx>

#include <Standard_GUID.hxx>
#include <TDataStd_AsciiString.hxx>
#include <TDataStd_Integer.hxx>
#include <TDataStd_Real.hxx>
#include <TDataStd_RealArray.hxx>
#include <TDataStd_ByteArray.hxx>
#include <TDataXtd_Axis.hxx>
#include <TDataXtd_Geometry.hxx>
#include <TDataXtd_Plane.hxx>
#include <TDataXtd_Point.hxx>
#include <TDF_ChildIterator.hxx>
#include <TColStd_HArray1OfReal.hxx>
#include <XCAFDoc.hxx>
#include <XCAFView_Object.hxx>
#include <TDataStd_IntegerArray.hxx>

IMPLEMENT_STANDARD_RTTIEXT(XCAFDoc_View, TDF_Attribute)

enum ChildLab
{
  ChildLab_Name = 1,
  ChildLab_Type,
  ChildLab_ProjectionPoint,
  ChildLab_ViewDirection,
  ChildLab_UpDirection,
  ChildLab_ZoomFactor,
  ChildLab_WindowHorizontalSize,
  ChildLab_WindowVerticalSize,
  ChildLab_FrontPlaneDistance,
  ChildLab_BackPlaneDistance,
  ChildLab_ViewVolumeSidesClipping,
  ChildLab_ClippingExpression,
  ChildLab_GDTPoints,
  ChildLab_Image,
  ChildLab_EnabledShapes,
  ChildLab_NotePoints
};

//=======================================================================
//function : XCAFDoc_View
//purpose  : 
//=======================================================================
XCAFDoc_View::XCAFDoc_View()
{
}


//=======================================================================
//function : GetID
//purpose  : 
//=======================================================================
const Standard_GUID& XCAFDoc_View::GetID() 
{
  static Standard_GUID ViewID ("efd213e8-6dfd-11d4-b9c8-0060b0ee281b");
  return ViewID; 
}

//=======================================================================
//function : Set
//purpose  : 
//=======================================================================
Handle(XCAFDoc_View) XCAFDoc_View::Set(const TDF_Label& theLabel) 
{
  Handle(XCAFDoc_View) A;
  if (!theLabel.FindAttribute(XCAFDoc_View::GetID(), A)) {
    A = new XCAFDoc_View();
    theLabel.AddAttribute(A);
  }
  return A;
}

//=======================================================================
//function : SetObject
//purpose  : 
//=======================================================================
void XCAFDoc_View::SetObject (const Handle(XCAFView_Object)& theObject)
{
  Backup();

  TDF_ChildIterator anIter(Label());
  for(;anIter.More(); anIter.Next())
  {
    anIter.Value().ForgetAllAttributes();
  }

  // Name
  TDataStd_AsciiString::Set(Label().FindChild(ChildLab_Name), theObject->Name()->String());

  // Type
  TDataStd_Integer::Set(Label().FindChild(ChildLab_Type), theObject->Type());

  // Projection point
  TDataXtd_Point::Set(Label().FindChild(ChildLab_ProjectionPoint), theObject->ProjectionPoint());

  // View direction
  gp_Ax1 aViewDir(gp_Pnt(), theObject->ViewDirection());
  TDataXtd_Axis::Set(Label().FindChild(ChildLab_ViewDirection), aViewDir);

  // Up direction
  gp_Ax1 anUpDir(gp_Pnt(), theObject->UpDirection());
  TDataXtd_Axis::Set(Label().FindChild(ChildLab_UpDirection), anUpDir);
  
  // Zoom factor
  TDataStd_Real::Set(Label().FindChild(ChildLab_ZoomFactor), theObject->ZoomFactor());

  // Window horizontal size
  TDataStd_Real::Set(Label().FindChild(ChildLab_WindowHorizontalSize), theObject->WindowHorizontalSize());

  // Window vertical size
  TDataStd_Real::Set(Label().FindChild(ChildLab_WindowVerticalSize), theObject->WindowVerticalSize());

  // Front plane clipping
  if (theObject->HasFrontPlaneClipping())
  {
    TDataStd_Real::Set(Label().FindChild(ChildLab_FrontPlaneDistance), theObject->FrontPlaneDistance());
  }

  // Back plane clipping
  if (theObject->HasBackPlaneClipping())
  {
    TDataStd_Real::Set(Label().FindChild(ChildLab_BackPlaneDistance), theObject->BackPlaneDistance());
  }

  // View volume sides clipping
  Standard_Integer aValue = theObject->HasViewVolumeSidesClipping() ? 1 : 0;
  TDataStd_Integer::Set(Label().FindChild(ChildLab_ViewVolumeSidesClipping), aValue);

  // Clipping Expression
  if (!theObject->ClippingExpression().IsNull())
    TDataStd_AsciiString::Set(Label().FindChild(ChildLab_ClippingExpression), theObject->ClippingExpression()->String());

  // GDT points
  if (theObject->HasGDTPoints())
  {
    TDF_Label aPointsLabel = Label().FindChild(ChildLab_GDTPoints);
    for (Standard_Integer i = 1; i <= theObject->NbGDTPoints(); i++) {
      TDataXtd_Point::Set(aPointsLabel.FindChild(i), theObject->GDTPoint(i));
    }
  }
  //Image
  if (theObject->HasImage())
  {
    Handle(TColStd_HArray1OfByte) image = theObject->Image();
    Handle(TDataStd_ByteArray) arr = TDataStd_ByteArray::Set(Label().FindChild(ChildLab_Image), image->Lower(), image->Upper());
    for (Standard_Integer i = image->Lower(); i <= image->Upper(); i++) {
      arr->SetValue(i, image->Value(i));
    }
  }
  //shapes transparency
  if (theObject->HasEnabledShapes())
  {
    TDF_Label aShapeTranspLabel = Label().FindChild(ChildLab_EnabledShapes);
    Handle(TDataStd_IntegerArray) array = TDataStd_IntegerArray::Set(aShapeTranspLabel, 1, theObject->NbEnabledShapes());	
    array->ChangeArray(theObject->GetEnabledShapes(), Standard_False);	
  }
  //note points
  if (theObject->HasNotePoints())
  {
    TDF_Label aPointsLabel = Label().FindChild(ChildLab_NotePoints);
    for (Standard_Integer i = 1; i <= theObject->NbNotePoints(); i++) {
      TDataXtd_Point::Set(aPointsLabel.FindChild(i), theObject->NotePoint(i));
    }
  }
}

//=======================================================================
//function : GetObject
//purpose  : 
//=======================================================================
Handle(XCAFView_Object) XCAFDoc_View::GetObject()  const
{
  Handle(XCAFView_Object) anObj = new XCAFView_Object();

  TDF_ChildIterator it(Label(), false);
  for ( ; it.More(); it.Next())
  {
    TDF_Label aLabel = it.Value();
    ChildLab aTag = static_cast<ChildLab>(aLabel.Tag());
    if (aTag == ChildLab_Name)
    {
      // Name
      Handle(TDataStd_AsciiString) aName;
      if (aLabel.FindAttribute(TDataStd_AsciiString::GetID(), aName))
      {
        anObj->SetName(new TCollection_HAsciiString(aName->Get()));
      }
    }
    else if (aTag == ChildLab_Type)
    {
      // Type
      Handle(TDataStd_Integer) aType;
      if (aLabel.FindAttribute(TDataStd_Integer::GetID(), aType))
      {
        anObj->SetType((XCAFView_ProjectionType)aType->Get());
      }
    }
    else if (aTag == ChildLab_ProjectionPoint)
    {
      // Projection point
      Handle(TDataXtd_Point) aPointAttr;
      if (aLabel.FindAttribute(TDataXtd_Point::GetID(), aPointAttr))
      {
        gp_Pnt aPoint;
        TDataXtd_Geometry::Point(aPointAttr->Label(), aPoint);
        anObj->SetProjectionPoint(aPoint);
      }
    }
    else if (aTag == ChildLab_ViewDirection)
    {
      // View direction
      Handle(TDataXtd_Axis) aViewDirAttr;
      if (aLabel.FindAttribute(TDataXtd_Axis::GetID(), aViewDirAttr))
      {
        gp_Ax1 aDir;
        TDataXtd_Geometry::Axis(aViewDirAttr->Label(), aDir);
        anObj->SetViewDirection(aDir.Direction());
      }
    }
    else if (aTag == ChildLab_UpDirection)
    {
      // Up direction
      Handle(TDataXtd_Axis) anUpDirAttr;
      if (aLabel.FindAttribute(TDataXtd_Axis::GetID(), anUpDirAttr))
      {
        gp_Ax1 aDir;
        TDataXtd_Geometry::Axis(anUpDirAttr->Label(), aDir);
        anObj->SetUpDirection(aDir.Direction());
      }
    }
    else if (aTag == ChildLab_ZoomFactor)
    {
      // Zoom factor
      Handle(TDataStd_Real) aZoomFactor;
      if (aLabel.FindAttribute(TDataStd_Real::GetID(), aZoomFactor))
      {
        anObj->SetZoomFactor(aZoomFactor->Get());
      }
    }
    else if (aTag == ChildLab_WindowHorizontalSize)
    {
      // Window horizontal size
      Handle(TDataStd_Real) aWindowHorizontalSize;
      if (aLabel.FindAttribute(TDataStd_Real::GetID(), aWindowHorizontalSize))
      {
        anObj->SetWindowHorizontalSize(aWindowHorizontalSize->Get());
      }
    }
    else if (aTag == ChildLab_WindowVerticalSize)
    {
      // Window vertical size
      Handle(TDataStd_Real) aWindowVerticalSize;
      if (aLabel.FindAttribute(TDataStd_Real::GetID(), aWindowVerticalSize))
      {
        anObj->SetWindowVerticalSize(aWindowVerticalSize->Get());
      }
    }
    else if (aTag == ChildLab_FrontPlaneDistance)
    {
      // Front plane clipping
      Handle(TDataStd_Real) aFrontPlaneDistance;
      if (aLabel.FindAttribute(TDataStd_Real::GetID(), aFrontPlaneDistance))
      {
        anObj->SetFrontPlaneDistance(aFrontPlaneDistance->Get());
      }
    }
    else if (aTag == ChildLab_BackPlaneDistance)
    {
      // Back plane clipping
      Handle(TDataStd_Real) aBackPlaneDistance;
      if (aLabel.FindAttribute(TDataStd_Real::GetID(), aBackPlaneDistance))
      {
        anObj->SetBackPlaneDistance(aBackPlaneDistance->Get());
      }
    }
    else if (aTag == ChildLab_ViewVolumeSidesClipping)
    {
      // View volume sides clipping
      Handle(TDataStd_Integer) aViewVolumeSidesClipping;
      if (aLabel.FindAttribute(TDataStd_Integer::GetID(), aViewVolumeSidesClipping))
      {
        Standard_Boolean aValue = (aViewVolumeSidesClipping->Get() == 1);
        anObj->SetViewVolumeSidesClipping(aValue);
      }
    }
    else if (aTag == ChildLab_ClippingExpression)
    {
      // Name
      Handle(TDataStd_AsciiString) aClippingExpression;
      if (aLabel.FindAttribute(TDataStd_AsciiString::GetID(), aClippingExpression))
      {
        anObj->SetClippingExpression(new TCollection_HAsciiString(aClippingExpression->Get()));
      }
    }
    else if (aTag == ChildLab_GDTPoints)
    {
      // GDT Points
      if (!aLabel.IsNull())
      {
        anObj->CreateGDTPoints(aLabel.NbChildren());
        for (Standard_Integer i = 1; i <= aLabel.NbChildren(); i++)
        {
          gp_Pnt aPoint;
          Handle(TDataXtd_Point) aGDTPointAttr;
          aLabel.FindChild(i).FindAttribute(TDataXtd_Point::GetID(), aGDTPointAttr);
          TDataXtd_Geometry::Point(aGDTPointAttr->Label(), aPoint);
          anObj->SetGDTPoint(i, aPoint);
        }
      }
    }
    else if (aTag == ChildLab_Image)
    {
      //Image
      Handle(TDataStd_ByteArray) anArr;
      if (aLabel.FindAttribute(TDataStd_ByteArray::GetID(), anArr))
      {
        anObj->SetImage(anArr->InternalArray());
      }
    }
    else if (aTag == ChildLab_EnabledShapes)
    {
      // Shapes transparency
      Handle(TDataStd_IntegerArray) anArr;
      if (aLabel.FindAttribute(TDataStd_IntegerArray::GetID(), anArr))
      {
        anObj->SetEnabledShapes(anArr->Array());
      }	  	
    }
    else if (aTag == ChildLab_NotePoints)
    {
      // Note Points
      anObj->CreateNotePoints(aLabel.NbChildren());
      TDF_ChildIterator itPoints(aLabel);
      int index = 1;
      for (; itPoints.More(); itPoints.Next(), ++index)
      {
        gp_Pnt aPoint;
        Handle(TDataXtd_Point) aPointAttr;
        itPoints.Value().FindAttribute(TDataXtd_Point::GetID(), aPointAttr);
        TDataXtd_Geometry::Point(aPointAttr->Label(), aPoint);
        anObj->SetNotePoint(index, aPoint);
      }
    }
  }
  return anObj;
}

//=======================================================================
//function : ID
//purpose  : 
//=======================================================================
const Standard_GUID& XCAFDoc_View::ID() const
{
  return GetID();
}

//=======================================================================
//function : Restore
//purpose  : 
//=======================================================================
void XCAFDoc_View::Restore(const Handle(TDF_Attribute)& /*With*/) 
{
}


//=======================================================================
//function : NewEmpty
//purpose  : 
//=======================================================================
Handle(TDF_Attribute) XCAFDoc_View::NewEmpty() const
{
  return new XCAFDoc_View();
}


//=======================================================================
//function : Paste
//purpose  : 
//=======================================================================
void XCAFDoc_View::Paste(const Handle(TDF_Attribute)& /*Into*/,
                         const Handle(TDF_RelocationTable)& /*RT*/) const
{
}

//=======================================================================
//function : DumpJson
//purpose  : 
//=======================================================================
void XCAFDoc_View::DumpJson (Standard_OStream& theOStream, Standard_Integer theDepth) const
{
  OCCT_DUMP_TRANSIENT_CLASS_BEGIN (theOStream)

  OCCT_DUMP_BASE_CLASS (theOStream, theDepth, TDF_Attribute)
}
