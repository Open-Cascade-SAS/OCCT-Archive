// Copyright (c) 2022 OPEN CASCADE SAS
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

#include <DE_Provider.hxx>

#include <DE_ConfigurationNode.hxx>
#include <OSD_Directory.hxx>
#include <OSD_File.hxx>
#include <OSD_OpenFile.hxx>
#include <OSD_Protection.hxx>
#include <Message.hxx>

#include <stdio.h>

IMPLEMENT_STANDARD_RTTIEXT(DE_Provider, Standard_Transient)

namespace
{
  class DE_TemporaryFile
  {
  public:
    DE_TemporaryFile(const TCollection_AsciiString& theFolderPath,
                     const TCollection_AsciiString& theExtension);

    ~DE_TemporaryFile();

    TCollection_AsciiString Path() const { return myTempPath; }

    Standard_Boolean IsDone() const { return myIsCreated; }

  private:

    Standard_Boolean myIsCreated = Standard_False;
    TCollection_AsciiString myTempPath;
    OSD_File myFile;
  };
}

//=======================================================================
// function : DE_TemporaryFile
// purpose  :
//=======================================================================
DE_TemporaryFile::DE_TemporaryFile(const TCollection_AsciiString& theFolderPath,
                                   const TCollection_AsciiString& theExtension)
{
  Standard_Boolean anIsCreated = Standard_False;
  OSD_Directory aDirectory;
  if (!theFolderPath.IsEmpty())
  {
    OSD_Directory anInternalFolder(theFolderPath);
    if (!anInternalFolder.Failed())
    {
      aDirectory = anInternalFolder;
      anIsCreated = Standard_True;
    }
  }
  if (!anIsCreated)
  {
    aDirectory = OSD_Directory::BuildTemporary();
  }
  OSD_Path aPath;
  aDirectory.Path(aPath);
  TCollection_AsciiString aFullPath;
  aPath.SystemName(aFullPath);
  if (!anIsCreated)
  {
    Message::SendTrace() << "DE Provider : Using temporary folder from system : ["
      << aFullPath << "]";
  }
  if (aDirectory.Failed())
  {
    Message::SendFail() << "Error: DE Provider : Can't create folder by path : ["
      << aFullPath << "]";
  }
  TCollection_AsciiString aTempName(tempnam(aFullPath.ToCString(), nullptr));
  aTempName += ".";
  aTempName += theExtension;
  myFile = OSD_File(aTempName);
  myFile.Build(OSD_ReadWrite, OSD_Protection());
  if (myFile.Failed())
  {
    Message::SendFail() << "Error: DE Provider : Can't create tempolary file by path : ["
      << aTempName << "]";
    return;
  }
  myIsCreated = Standard_True;
  myTempPath = aTempName;
}

//=======================================================================
// function : DE_TemporaryFile
// purpose  :
//=======================================================================
DE_TemporaryFile::~DE_TemporaryFile()
{
  if (!myIsCreated)
  {
    return;
  }
  if (myFile.IsLocked())
  {
    myFile.UnLock();
  }
  myFile.Close();
  if (std::remove(myTempPath.ToCString()) != 0)
  {
    Message::SendFail() << "Error: DE Provider : Can't remove tempolary file by path : ["
      << myTempPath << "]";
  }
}

//=======================================================================
// function : DE_Provider
// purpose  :
//=======================================================================
DE_Provider::DE_Provider()
{}

//=======================================================================
// function : DE_Provider
// purpose  :
//=======================================================================
DE_Provider::DE_Provider(const Handle(DE_ConfigurationNode)& theNode)
  :myNode(theNode)
{}

//=======================================================================
// function : Read
// purpose  :
//=======================================================================
Standard_Boolean DE_Provider::Read(const TCollection_AsciiString& thePath,
                                   const Handle(TDocStd_Document)& theDocument,
                                   Handle(DE_WorkSession)& theWS,
                                   const Message_ProgressRange& theProgress)
{
  (void)thePath;
  (void)theDocument;
  (void)theWS;
  (void)theProgress;
  Message::SendFail() << "Error: provider " << GetFormat() <<
    " " << GetVendor() << " doesn't support read operation";
  return Standard_False;
}

//=======================================================================
// function : Read
// purpose  :
//=======================================================================
bool DE_Provider::Read(std::istream& theIStream,
                       const Handle(TDocStd_Document)& theDocument,
                       const TCollection_AsciiString theName,
                       Handle(DE_WorkSession)& theWS,
                       const Message_ProgressRange& theProgress)
{
  (void)theName;
  if (myNode.IsNull() ||
      myNode->GetFormat() != GetFormat() ||
      myNode->GetVendor() != GetVendor())
  {
    Message::SendFail() << "Error: provider " << GetFormat() <<
      " " << GetVendor() << " : Incorrect Configuration node";
    return Standard_False;
  }
  if (!myNode->IsImportSupported())
  {
    Message::SendFail() << "Error: provider " << GetFormat() <<
      " " << GetVendor() << " doesn't support read operation";
    return Standard_False;
  }
  TColStd_ListOfAsciiString anExtns = myNode->GetExtensions();
  TCollection_AsciiString anExt("tmp");
  if (!anExtns.IsEmpty())
  {
    anExt = anExtns.First();
  }
  DE_TemporaryFile aTempFile(myTempFolder, anExt);
  if (!aTempFile.IsDone())
  {
    return Standard_False;
  }
  std::ofstream aStream;
  OSD_OpenStream(aStream, aTempFile.Path(), std::ios::out | std::ios::binary);
  aStream << theIStream.rdbuf();
  return Read(aTempFile.Path(), theDocument, theWS, theProgress);
}

//=======================================================================
// function : Write
// purpose  :
//=======================================================================
Standard_Boolean DE_Provider::Write(const TCollection_AsciiString& thePath,
                                    const Handle(TDocStd_Document)& theDocument,
                                    Handle(DE_WorkSession)& theWS,
                                    const Message_ProgressRange& theProgress)
{
  (void)thePath;
  (void)theDocument;
  (void)theWS;
  (void)theProgress;
  Message::SendFail() << "Error: provider " << GetFormat() <<
    " " << GetVendor() << " doesn't support write operation";
  return Standard_False;
}

//=======================================================================
// function : Write
// purpose  :
//=======================================================================
bool DE_Provider::Write(std::ostream& theOStream,
                        const Handle(TDocStd_Document)& theDocument,
                        Handle(DE_WorkSession)& theWS,
                        const Message_ProgressRange& theProgress)
{
  if (myNode.IsNull() ||
      myNode->GetFormat() != GetFormat() ||
      myNode->GetVendor() != GetVendor())
  {
    Message::SendFail() << "Error: provider " << GetFormat() <<
      " " << GetVendor() << " : Incorrect Configuration node";
    return Standard_False;
  }
  if (!myNode->IsExportSupported())
  {
    Message::SendFail() << "Error: provider " << GetFormat() <<
      " " << GetVendor() << " doesn't support write operation";
    return Standard_False;
  }
  TColStd_ListOfAsciiString anExtns = myNode->GetExtensions();
  TCollection_AsciiString anExt("tmp");
  if (!anExtns.IsEmpty())
  {
    anExt = anExtns.First();
  }
  DE_TemporaryFile aTempFile(myTempFolder, anExt);
  if (!aTempFile.IsDone())
  {
    return Standard_False;
  }
  if (!Write(aTempFile.Path(), theDocument, theWS, theProgress))
  {
    return Standard_False;
  }
  std::ifstream aStream;
  OSD_OpenStream(aStream, aTempFile.Path().ToCString(), std::ios::in | std::ios::binary);
  theOStream << aStream.rdbuf();
  return Standard_True;
}

//=======================================================================
// function : Read
// purpose  :
//=======================================================================
Standard_Boolean DE_Provider::Read(const TCollection_AsciiString& thePath,
                                   TopoDS_Shape& theShape,
                                   Handle(DE_WorkSession)& theWS,
                                   const Message_ProgressRange& theProgress)
{
  (void)thePath;
  (void)theShape;
  (void)theWS;
  (void)theProgress;
  Message::SendFail() << "Error: provider " << GetFormat() <<
    " " << GetVendor() << " doesn't support read operation";
  return Standard_False;
}

//=======================================================================
// function : Read
// purpose  :
//=======================================================================
bool DE_Provider::Read(std::istream& theIStream,
                       TopoDS_Shape& theShape,
                       const TCollection_AsciiString theName,
                       Handle(DE_WorkSession)& theWS,
                       const Message_ProgressRange& theProgress)
{
  (void)theName;
  if (myNode.IsNull() ||
      myNode->GetFormat() != GetFormat() ||
      myNode->GetVendor() != GetVendor())
  {
    Message::SendFail() << "Error: provider " << GetFormat() <<
      " " << GetVendor() << " : Incorrect Configuration node";
    return Standard_False;
  }
  if (!myNode->IsImportSupported())
  {
    Message::SendFail() << "Error: provider " << GetFormat() <<
      " " << GetVendor() << " doesn't support read operation";
    return Standard_False;
  }
  TColStd_ListOfAsciiString anExtns = myNode->GetExtensions();
  TCollection_AsciiString anExt("tmp");
  if (!anExtns.IsEmpty())
  {
    anExt = anExtns.First();
  }
  DE_TemporaryFile aTempFile(myTempFolder, anExt);
  if (!aTempFile.IsDone())
  {
    return Standard_False;
  }
  std::ofstream aStream;
  OSD_OpenStream(aStream, aTempFile.Path(), std::ios::out | std::ios::binary);
  aStream << theIStream.rdbuf();
  return Read(aTempFile.Path(), theShape, theWS, theProgress);
}

//=======================================================================
// function : Write
// purpose  :
//=======================================================================
Standard_Boolean DE_Provider::Write(const TCollection_AsciiString& thePath,
                                    const TopoDS_Shape& theShape,
                                    Handle(DE_WorkSession)& theWS,
                                    const Message_ProgressRange& theProgress)
{
  (void)thePath;
  (void)theShape;
  (void)theWS;
  (void)theProgress;
  Message::SendFail() << "Error: provider " << GetFormat() <<
    " " << GetVendor() << " doesn't support write operation";
  return Standard_False;
}

//=======================================================================
// function : Write
// purpose  :
//=======================================================================
bool DE_Provider::Write(std::ostream& theOStream,
                        const TopoDS_Shape& theShape,
                        Handle(DE_WorkSession)& theWS,
                        const Message_ProgressRange& theProgress)
{
  if (myNode.IsNull() ||
      myNode->GetFormat() != GetFormat() ||
      myNode->GetVendor() != GetVendor())
  {
    Message::SendFail() << "Error: provider " << GetFormat() <<
      " " << GetVendor() << " : Incorrect Configuration node";
    return Standard_False;
  }
  if (!myNode->IsExportSupported())
  {
    Message::SendFail() << "Error: provider " << GetFormat() <<
      " " << GetVendor() << " doesn't support write operation";
    return Standard_False;
  }
  TColStd_ListOfAsciiString anExtns = myNode->GetExtensions();
  TCollection_AsciiString anExt("tmp");
  if (!anExtns.IsEmpty())
  {
    anExt = anExtns.First();
  }
  DE_TemporaryFile aTempFile(myTempFolder, anExt);
  if (!aTempFile.IsDone())
  {
    return Standard_False;
  }
  if (!Write(aTempFile.Path(), theShape, theWS, theProgress))
  {
    return Standard_False;
  }
  std::ifstream aStream;
  OSD_OpenStream(aStream, aTempFile.Path().ToCString(), std::ios::in | std::ios::binary);
  theOStream << aStream.rdbuf();
  return Standard_True;
}
