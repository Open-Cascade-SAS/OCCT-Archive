// Created on: 2017-06-16
// Created by: Natalia ERMOLAEVA
// Copyright (c) 2017 OPEN CASCADE SAS
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

#include <inspector/MessageView_VisibilityState.hxx>
#include <inspector/MessageModel_ItemAlert.hxx>

#include <Message_AlertExtended.hxx>
#include <Message_AttributeStream.hxx>

#include <TopoDS_AlertAttribute.hxx>

// =======================================================================
// function : CanBeVisible
// purpose :
// =======================================================================
bool MessageView_VisibilityState::CanBeVisible (const QModelIndex& theIndex) const
{
  MessageModel_ItemAlertPtr anAlertItem = getAlertItem (theIndex);
  if (anAlertItem)
  {
    NCollection_List<Handle(Standard_Transient)> aPresentations;
    anAlertItem->Presentations (aPresentations);
    if (!aPresentations.IsEmpty())
      return true;
  }

  return !Shape (theIndex).IsNull();// || hasTableValues (theIndex);
}

// =======================================================================
// function : SetVisible
// purpose :
// =======================================================================
bool MessageView_VisibilityState::SetVisible (const QModelIndex& theIndex, const bool theState, const bool toEmitDataChanged)
{
  TopoDS_Shape aShape = Shape (theIndex);
  if (aShape.IsNull())
    return false;

  myDisplayer->SetVisible (aShape, theState, myPresentationType);

  if (!theState) {
    MessageModel_ItemAlertPtr anAlertItem = getAlertItem (theIndex);
    if (anAlertItem && !anAlertItem->GetCustomShape().IsNull())
      anAlertItem->SetCustomShape (TopoDS_Shape());
  }

  if (toEmitDataChanged)
  {
    QModelIndex anIndex = theIndex;
    if (theIndex.column() != TreeModel_ColumnType_Visibility)
      anIndex = theIndex.model()->index(theIndex.row(), TreeModel_ColumnType_Visibility, theIndex.parent());

    getModel()->EmitDataChanged (anIndex, anIndex);
  }
  return true;
}

// =======================================================================
// function : IsVisible
// purpose :
// =======================================================================
bool MessageView_VisibilityState::IsVisible (const QModelIndex& theIndex) const
{
  return myDisplayer->IsVisible (Shape (theIndex), myPresentationType);
}

// =======================================================================
// function : OnClicked
// purpose :
// =======================================================================
void MessageView_VisibilityState::OnClicked (const QModelIndex& theIndex)
{
  processClicked (theIndex);
  emit itemClicked (theIndex);
}

// =======================================================================
// function : getAlertItem
// purpose :
// =======================================================================
MessageModel_ItemAlertPtr MessageView_VisibilityState::getAlertItem (const QModelIndex& theIndex) const
{
  TreeModel_ItemBasePtr anItemBase = TreeModel_ModelBase::GetItemByIndex (theIndex);
  if (!anItemBase)
    return MessageModel_ItemAlertPtr();

  MessageModel_ItemAlertPtr anAlertItem = itemDynamicCast<MessageModel_ItemAlert>(anItemBase);
  return anAlertItem;
}

// =======================================================================
// function : Shape
// purpose :
// =======================================================================
TopoDS_Shape MessageView_VisibilityState::Shape (const QModelIndex& theIndex) const
{
  MessageModel_ItemAlertPtr anAlertItem = getAlertItem (theIndex);
  if (!anAlertItem)
    return TopoDS_Shape();

  /*if (!anAlertItem->GetCustomShape().IsNull())
    return anAlertItem->GetCustomShape();

  Handle(Message_Alert) anAlert = anAlertItem->GetAlert();
  if (anAlert.IsNull())
    return TopoDS_Shape();

  Handle(Message_AlertExtended) anAlertExtended = Handle(Message_AlertExtended)::DownCast(anAlert);
  if (anAlertExtended.IsNull())
    return TopoDS_Shape();

  Handle(TopoDS_AlertAttribute) aShapeAttribute = Handle(TopoDS_AlertAttribute)::DownCast (anAlertExtended->Attribute());
  if (!aShapeAttribute.IsNull())
    return aShapeAttribute->GetShape();
  */
  return TopoDS_Shape();
}

// =======================================================================
// function : hasTableValues
// purpose :
// =======================================================================
bool MessageView_VisibilityState::hasTableValues (const QModelIndex& theIndex) const
{
  MessageModel_ItemAlertPtr anAlertItem = getAlertItem (theIndex);
  if (!anAlertItem)
    return false;

  Handle(Message_Alert) anAlert = anAlertItem->GetAlert();
  if (anAlert.IsNull())
    return false;

  if (anAlert->IsKind (STANDARD_TYPE (Message_AttributeStream)))
    return true;

  return false;
}