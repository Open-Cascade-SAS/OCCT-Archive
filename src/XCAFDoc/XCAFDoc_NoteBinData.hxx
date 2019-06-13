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

#ifndef _XCAFDoc_NoteBinData_HeaderFile
#define _XCAFDoc_NoteBinData_HeaderFile

#include <Standard_Handle.hxx>
#include <Standard_Transient.hxx>
#include <TDF_Label.hxx>
#include <TDF_LabelList.hxx>

class OSD_File;
class Standard_GUID;
class TColStd_HArray1OfByte;
class TDataStd_AsciiString;
class TDataStd_ByteArray; 
class TDataStd_Name;
class TDataStd_ReferenceList;
class XCAFDoc_NoteBinData;

class XCAFDoc_NoteBinDataContainer : public Standard_Transient
{
public:

  DEFINE_STANDARD_RTTIEXT(XCAFDoc_NoteBinDataContainer, Standard_Transient)

  //! Finds a reference attribute on the given label and returns a proxy instance if it is found
  Standard_EXPORT static Handle(XCAFDoc_NoteBinDataContainer) Get(const TDF_Label& theLabel);

  //! Create (if not exist) a binary data container on the given note label.
  //! \param [in] theLabel   - note label.
  Standard_EXPORT static Handle(XCAFDoc_NoteBinDataContainer) Set(const TDF_Label& theLabel);

  Standard_EXPORT Standard_Integer Size() const;
  Standard_EXPORT const TDF_LabelList& Content() const;

  Standard_EXPORT Handle(XCAFDoc_NoteBinData) Add(const TCollection_ExtendedString& theTitle,
                                                  const TCollection_AsciiString&    theMIMEtype,
                                                  OSD_File&                         theFile);

  Standard_EXPORT Handle(XCAFDoc_NoteBinData) Add(const TCollection_ExtendedString&    theTitle,
                                                  const TCollection_AsciiString&       theMIMEtype,
                                                  const Handle(TColStd_HArray1OfByte)& theData);

  Standard_EXPORT Standard_Boolean Remove(TDF_Label& theLabel);

  Standard_EXPORT void Clear();

  Standard_EXPORT TDF_Label Label() const;

private:

  XCAFDoc_NoteBinDataContainer(const Handle(TDataStd_ReferenceList)& theList);

  static const Standard_GUID& getID();

  Handle(TDataStd_ReferenceList) myList;
};

DEFINE_STANDARD_HANDLE(XCAFDoc_NoteBinDataContainer, Standard_Transient)

class XCAFDoc_NoteBinData : public Standard_Transient
{
  friend class XCAFDoc_NoteBinDataContainer;

public:

  DEFINE_STANDARD_RTTIEXT(XCAFDoc_NoteBinData, Standard_Transient)

  //! Finds a binary data attribute on the given label and returns it, if it is found
  Standard_EXPORT static Handle(XCAFDoc_NoteBinData) Get(const TDF_Label& theLabel);

  //! @name Set attribute data functions.
  //! @{

  //! Sets title, MIME type and data from a binary file.
  //! \param [in] theTitle    - file title.
  //! \param [in] theMIMEtype - MIME type of the file.
  //! \param [in] theFile     - input binary file.
  Standard_EXPORT Standard_Boolean Set(const TCollection_ExtendedString& theTitle,
                                       const TCollection_AsciiString&    theMIMEtype,
                                       OSD_File&                         theFile);

  //! Sets title, MIME type and data from a byte array.
  //! \param [in] theTitle    - data title.
  //! \param [in] theMIMEtype - MIME type of data.
  //! \param [in] theData     - byte data array.
  Standard_EXPORT void Set(const TCollection_ExtendedString&    theTitle,
                           const TCollection_AsciiString&       theMIMEtype,
                           const Handle(TColStd_HArray1OfByte)& theData);

  //! @}

  //! Returns the note title.
  Standard_EXPORT const TCollection_ExtendedString& Title() const;

  //! Returns data MIME type.
  Standard_EXPORT const TCollection_AsciiString& MIMEtype() const;

  //! Size of data in bytes.
  Standard_EXPORT Standard_Integer Size() const;

  //! Returns byte data array.
  Standard_EXPORT const Handle(TColStd_HArray1OfByte)& Data() const;

  //! Returns label
  Standard_EXPORT TDF_Label Label() const;

private:

  XCAFDoc_NoteBinData(const Handle(TDataStd_Name)& theTitle,
                      const Handle(TDataStd_AsciiString)& theMIMEType,
                      const Handle(TDataStd_ByteArray)& theData);

  static Handle(XCAFDoc_NoteBinData) Set(const TDF_Label&                  theLabel,
                                         const TCollection_ExtendedString& theTitle,
                                         const TCollection_AsciiString&    theMIMEtype,
                                         OSD_File&                         theFile);

  static Handle(XCAFDoc_NoteBinData) Set(const TDF_Label&                     theLabel,
                                         const TCollection_ExtendedString&    theTitle,
                                         const TCollection_AsciiString&       theMIMEtype,
                                         const Handle(TColStd_HArray1OfByte)& theData);

private:

  Handle(TDataStd_Name)        myTitle;    ///< Data title.
  Handle(TDataStd_AsciiString) myMIMEtype; ///< MIME type of data.
  Handle(TDataStd_ByteArray)   myData;     ///< Byte data array.

};

DEFINE_STANDARD_HANDLE(XCAFDoc_NoteBinData, Standard_Transient)

#endif // _XCAFDoc_NoteBinData_HeaderFile
