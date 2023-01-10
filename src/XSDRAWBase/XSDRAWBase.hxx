// Copyright (c) 2023 OPEN CASCADE SAS
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

#ifndef _XSDRAWBase_HeaderFile
#define _XSDRAWBase_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>

#include <Draw_Interpretor.hxx>
#include <TColStd_HSequenceOfTransient.hxx>
#include <TopTools_HSequenceOfShape.hxx>

class IFSelect_SessionPilot;
class Interface_Protocol;
class Interface_InterfaceModel;
class Standard_Transient;
class TCollection_AsciiString;
class TDocStd_Document;
class Transfer_TransientProcess;
class Transfer_FinderProcess;
class XSControl_Controller;
class XSControl_TransferReader;
class XSControl_WorkSession;

//! Basic package to work functions of X-STEP (IFSelect & Co)
//! under control of DRAW
//!
//! Works with some "static" data : a SessionPilot (used to run)
//! with its WorkSession and Model and TransferReader, a
//! FinderProcess
class XSDRAWBase
{
  DEFINE_STANDARD_ALLOC
public:

  //! Returns the WorkSession defined in AddDraw (through Pilot)
  //! It is from XSControl, it brings functionalities for Transfers
  Standard_EXPORT static Handle(XSControl_WorkSession) Session();

  //! Evaluates and returns a list of entity, from :
  //! keyboard if <first> and <second> are empty, see below
  //! first if second is empty : can be a number/label of an entity
  //! or the name of a selection to be evaluated (standard)
  //! first : name of a selection, evaluated from a list defined by
  //! second
  //! In case of failure, returns a Null Handle
  Standard_EXPORT static Handle(TColStd_HSequenceOfTransient) GetList(const Standard_CString first = "", const Standard_CString second = "");

  //!
  Standard_EXPORT static Standard_Real GetLengthUnit(const Handle(TDocStd_Document)& theDoc = nullptr);

  //!
  Standard_EXPORT static XSControl_WorkSessionMap& WorkSessionList();

  //!
  Standard_EXPORT static void CollectActiveWorkSessions(const Handle(XSControl_WorkSession)& theWS,
                                                        const TCollection_AsciiString& theName,
                                                        XSControl_WorkSessionMap& theMap,
                                                        const Standard_Boolean theIsFirst = Standard_True);
};

#endif // _XSDRAWBase_HeaderFile
