// Copyright (c) 2017-2018 OPEN CASCADE SAS
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

#include <XCAFDoc_NoteBinData.hxx>

#include <OSD_File.hxx>
#include <TDataStd_AsciiString.hxx>
#include <TDataStd_ByteArray.hxx>
#include <TDataStd_Name.hxx>
#include <TDataStd_ReferenceList.hxx>
#include <TDF_ChildIterator.hxx>
#include <TDF_Label.hxx>
#include <XCAFDoc_Note.hxx>

IMPLEMENT_STANDARD_RTTIEXT(XCAFDoc_NoteBinDataContainer, Standard_Transient)
IMPLEMENT_STANDARD_RTTIEXT(XCAFDoc_NoteBinData, Standard_Transient)

// =======================================================================
// function : getID
// purpose  :
// =======================================================================
const Standard_GUID& 
XCAFDoc_NoteBinDataContainer::getID()
{
  static Standard_GUID s_ID("91CFB5D9-737C-4ab9-933A-15E28DBBD1CF");
  return s_ID;
}

// =======================================================================
// function : XCAFDoc_NoteBinDataContainer
// purpose  :
// =======================================================================
XCAFDoc_NoteBinDataContainer::XCAFDoc_NoteBinDataContainer(const Handle(TDataStd_ReferenceList)& theList)
  : myList(theList)
{

}

// =======================================================================
// function : Get
// purpose  :
// =======================================================================
Handle(XCAFDoc_NoteBinDataContainer)
XCAFDoc_NoteBinDataContainer::Get(const TDF_Label& theLabel)
{
  if (!XCAFDoc_Note::IsMine(theLabel))
  {
    return NULL;
  }

  TDF_Label aLabel = theLabel.FindChild(XCAFDoc_Note::ChildLab_BinDataContainer, Standard_False);
  Handle(TDataStd_ReferenceList) aList;
  if (!aLabel.IsNull() && aLabel.FindAttribute(getID(), aList) && !aList.IsNull())
  {
    return new XCAFDoc_NoteBinDataContainer(aList);
  }

  return NULL;
}

// =======================================================================
// function : Set
// purpose  :
// =======================================================================
Handle(XCAFDoc_NoteBinDataContainer)
XCAFDoc_NoteBinDataContainer::Set(const TDF_Label& theLabel)
{
  if (!XCAFDoc_Note::IsMine(theLabel))
  {
    return NULL;
  }

  TDF_Label aLabel = theLabel.FindChild(XCAFDoc_Note::ChildLab_BinDataContainer);
  return new XCAFDoc_NoteBinDataContainer(TDataStd_ReferenceList::Set(aLabel, getID()));
}

// =======================================================================
// function : Size
// purpose  :
// =======================================================================
Standard_Integer
XCAFDoc_NoteBinDataContainer::Size() const
{
  return myList->List().Size();
}

// =======================================================================
// function : Content
// purpose  :
// =======================================================================
const TDF_LabelList&
XCAFDoc_NoteBinDataContainer::Content() const
{
  return myList->List();
}

// =======================================================================
// function : Add
// purpose  :
// =======================================================================
Handle(XCAFDoc_NoteBinData)
XCAFDoc_NoteBinDataContainer::Add(const TCollection_ExtendedString& theTitle,
                                  const TCollection_AsciiString&    theMIMEtype,
                                  OSD_File&                         theFile)
{
  TDF_Label aLabel;
  for (TDF_ChildIterator anIt(myList->Label()); anIt.More(); anIt.Next())
  {
    if (!anIt.Value().HasAttribute())
    {
      aLabel = anIt.Value();
      break;
    }
  }
  if (aLabel.IsNull())
  {
    aLabel = myList->Label().NewChild();
  }
  myList->Append(aLabel);
  return XCAFDoc_NoteBinData::Set(aLabel, theTitle, theMIMEtype, theFile);
}

// =======================================================================
// function : Add
// purpose  :
// =======================================================================
Handle(XCAFDoc_NoteBinData)
XCAFDoc_NoteBinDataContainer::Add(const TCollection_ExtendedString&    theTitle,
                                  const TCollection_AsciiString&       theMIMEtype,
                                  const Handle(TColStd_HArray1OfByte)& theData)
{
  TDF_Label aLabel;
  for (TDF_ChildIterator anIt(myList->Label()); anIt.More(); anIt.Next())
  {
    if (!anIt.Value().HasAttribute())
    {
      aLabel = anIt.Value();
      break;
    }
  }
  if (aLabel.IsNull())
  {
    aLabel = myList->Label().NewChild();
  }
  myList->Append(aLabel);
  return XCAFDoc_NoteBinData::Set(aLabel, theTitle, theMIMEtype, theData);
}

// =======================================================================
// function : Remove
// purpose  :
// =======================================================================
Standard_Boolean
XCAFDoc_NoteBinDataContainer::Remove(TDF_Label& theLabel)
{
  theLabel.ForgetAllAttributes();
  return myList->Remove(theLabel);
}

// =======================================================================
// function : Clear
// purpose  :
// =======================================================================
void
XCAFDoc_NoteBinDataContainer::Clear()
{
  const TDF_LabelList& aList = myList->List();
  for (TDF_LabelList::Iterator anIt(aList); anIt.More(); anIt.Next())
  {
    anIt.ChangeValue().ForgetAllAttributes();
  }
  myList->Clear();
}

// =======================================================================
// function : Label
// purpose  :
// =======================================================================
TDF_Label
XCAFDoc_NoteBinDataContainer::Label() const
{
  return !myList.IsNull() ? myList->Label() : TDF_Label();
}

// =======================================================================
// function : Get
// purpose  :
// =======================================================================
Handle(XCAFDoc_NoteBinData)
XCAFDoc_NoteBinData::Get(const TDF_Label& theLabel)
{
  if (theLabel.IsNull() || theLabel.Father().IsNull() || theLabel.Father().Father().IsNull() ||
      !XCAFDoc_Note::IsMine(theLabel.Father().Father()))
    return NULL;

  Handle(TDataStd_Name) aTitle;
  Handle(TDataStd_AsciiString) aMIMEType;
  Handle(TDataStd_ByteArray) aData;
  if (!theLabel.FindAttribute(TDataStd_Name::GetID(), aTitle) ||
      !theLabel.FindAttribute(TDataStd_AsciiString::GetID(), aMIMEType) || 
      !theLabel.FindAttribute(TDataStd_ByteArray::GetID(), aData))
    return NULL;

  return new XCAFDoc_NoteBinData(aTitle, aMIMEType, aData);
}

// =======================================================================
// function : Set
// purpose  :
// =======================================================================
Handle(XCAFDoc_NoteBinData)
XCAFDoc_NoteBinData::Set(const TDF_Label&                  theLabel,
                         const TCollection_ExtendedString& theTitle,
                         const TCollection_AsciiString&    theMIMEtype,
                         OSD_File&                         theFile)
{
  if (theLabel.IsNull() || theLabel.Father().IsNull() || theLabel.Father().Father().IsNull() ||
      !XCAFDoc_Note::IsMine(theLabel.Father().Father()))
    return NULL;

  if (!theFile.IsOpen() || !theFile.IsReadable() || theFile.Size() > (Standard_Size)IntegerLast())
    return NULL;

  Handle(TDataStd_ByteArray) aData = TDataStd_ByteArray::Set(theLabel, 1, (Standard_Integer)theFile.Size());
  Standard_Integer nbReadBytes = 0;
  theFile.Read((Standard_Address)&aData->InternalArray()->First(), aData->Length(), nbReadBytes);
  if (nbReadBytes < aData->Length())
  {
    theLabel.ForgetAttribute(aData);
    return NULL;
  }

  return new XCAFDoc_NoteBinData(TDataStd_Name::Set(theLabel, theTitle),
                                 TDataStd_AsciiString::Set(theLabel, theMIMEtype),
                                 aData);
}

// =======================================================================
// function : Set
// purpose  :
// =======================================================================
Handle(XCAFDoc_NoteBinData)
XCAFDoc_NoteBinData::Set(const TDF_Label&                     theLabel,
                         const TCollection_ExtendedString&    theTitle,
                         const TCollection_AsciiString&       theMIMEtype,
                         const Handle(TColStd_HArray1OfByte)& theData)
{
  if (theLabel.IsNull() || theLabel.Father().IsNull() || theLabel.Father().Father().IsNull() ||
      !XCAFDoc_Note::IsMine(theLabel.Father().Father()))
    return NULL;

  Handle(TDataStd_ByteArray) aData = TDataStd_ByteArray::Set(theLabel, 1, 2);
  aData->ChangeArray(theData);
  return new XCAFDoc_NoteBinData(TDataStd_Name::Set(theLabel, theTitle),
                                 TDataStd_AsciiString::Set(theLabel, theMIMEtype),
                                 aData);
}

// =======================================================================
// function : XCAFDoc_NoteBinData
// purpose  :
// =======================================================================
XCAFDoc_NoteBinData::XCAFDoc_NoteBinData(const Handle(TDataStd_Name)& theTitle,
                                         const Handle(TDataStd_AsciiString)& theMIMEType,
                                         const Handle(TDataStd_ByteArray)& theData)
  : myTitle(theTitle)
  , myMIMEtype(theMIMEType)
  , myData(theData)
{
}

// =======================================================================
// function : Set
// purpose  :
// =======================================================================
Standard_Boolean
XCAFDoc_NoteBinData::Set(const TCollection_ExtendedString& theTitle,
                         const TCollection_AsciiString&    theMIMEtype,
                         OSD_File&                         theFile)
{
  if (!theFile.IsOpen() || !theFile.IsReadable() || theFile.Size() > (Standard_Size)IntegerLast())
    return Standard_False;

  Handle(TColStd_HArray1OfByte) aData(new TColStd_HArray1OfByte(1, (Standard_Integer)theFile.Size()));
  Standard_Integer nbReadBytes = 0;
  theFile.Read((Standard_Address)&aData->First(), aData->Length(), nbReadBytes);
  if (nbReadBytes < aData->Length())
    return Standard_False;

  myData->ChangeArray(aData);
  myTitle->Set(theTitle);
  myMIMEtype->Set(theMIMEtype);

  return Standard_True;
}

// =======================================================================
// function : Set
// purpose  :
// =======================================================================
void
XCAFDoc_NoteBinData::Set(const TCollection_ExtendedString&    theTitle, 
                         const TCollection_AsciiString&       theMIMEtype,
                         const Handle(TColStd_HArray1OfByte)& theData)
{
  myTitle->Set(theTitle);
  myMIMEtype->Set(theMIMEtype);
  myData->ChangeArray(theData);
}

// =======================================================================
// function : Title
// purpose  :
// =======================================================================
const TCollection_ExtendedString&
XCAFDoc_NoteBinData::Title() const
{
  return myTitle->Get();
}

// =======================================================================
// function : MIMEtype
// purpose  :
// =======================================================================
const TCollection_AsciiString&
XCAFDoc_NoteBinData::MIMEtype() const
{
  return myMIMEtype->Get();
}

// =======================================================================
// function : Size
// purpose  :
// =======================================================================
Standard_Integer
XCAFDoc_NoteBinData::Size() const
{
  return myData->Length();
}

// =======================================================================
// function : Data
// purpose  :
// =======================================================================
const Handle(TColStd_HArray1OfByte)&
XCAFDoc_NoteBinData::Data() const
{
  return myData->InternalArray();
}

// =======================================================================
// function : Label
// purpose  :
// =======================================================================
TDF_Label
XCAFDoc_NoteBinData::Label() const
{
  return !myData.IsNull() ? myData->Label() : TDF_Label();
}
