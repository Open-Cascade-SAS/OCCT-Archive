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

#include <XSDRAWBase.hxx>

#include <XCAFDoc_DocumentTool.hxx>
#include <XSAlgo.hxx>
#include <XSAlgo_AlgoContainer.hxx>
#include <TDocStd_Document.hxx>
#include <UnitsMethods.hxx>

#include <memory>

namespace
{
  //=======================================================================
  //function : collectActiveWorkSessions
  //purpose  :
  //=======================================================================
  static void collectActiveWorkSessions(const Handle(XSControl_WorkSession)& theWS,
                                 const TCollection_AsciiString& theName,
                                 XSControl_WorkSessionMap& theMap,
                                 const Standard_Boolean theIsFirst)
  {
    if (theIsFirst)
    {
      theMap.Clear();
    }
    if (theMap.IsBound(theName))
    {
      return;
    }
    theMap.Bind(theName, theWS);
    for (XSControl_WorkSessionMap::Iterator anIter(theWS->ReferenceWS());
         anIter.More(); anIter.Next())
    {
      collectActiveWorkSessions(anIter.Value(), anIter.Key(), theMap, Standard_False);
    }
  }
}

//=======================================================================
//function : Session
//purpose  :
//=======================================================================
Handle(XSControl_WorkSession)& XSDRAWBase::Session()
{
  static Handle(XSControl_WorkSession) THE_SINGLETON_SESSION;
  if (THE_SINGLETON_SESSION.IsNull())
  {
    THE_SINGLETON_SESSION = new XSControl_WorkSession;
  }
  return THE_SINGLETON_SESSION;
}

//=======================================================================
//function : GetLengthUnit
//purpose  :
//=======================================================================
Standard_Real XSDRAWBase::GetLengthUnit(const Handle(TDocStd_Document)& theDoc)
{
  if (!theDoc.IsNull())
  {
    Standard_Real aUnit = 1.;
    if (XCAFDoc_DocumentTool::GetLengthUnit(theDoc, aUnit,
        UnitsMethods_LengthUnit_Millimeter))
    {
      return aUnit;
    }
  }
  XSAlgo::AlgoContainer()->PrepareForTransfer();
  return UnitsMethods::GetCasCadeLengthUnit();
}

//=======================================================================
//function : WorkSessionList
//purpose  :
//=======================================================================
XSControl_WorkSessionMap& XSDRAWBase::WorkSessionList()
{
  static std::shared_ptr<XSControl_WorkSessionMap> THE_PREVIOUS_WORK_SESSIONS;
  if (THE_PREVIOUS_WORK_SESSIONS == nullptr)
  {
    THE_PREVIOUS_WORK_SESSIONS =
      std::make_shared<XSControl_WorkSessionMap>();
  }
  return *THE_PREVIOUS_WORK_SESSIONS;
}

//=======================================================================
//function : CollectActiveWorkSessions
//purpose  :
//=======================================================================
void XSDRAWBase::CollectActiveWorkSessions(const Handle(XSControl_WorkSession)& theWS,
                                           const TCollection_AsciiString& theName,
                                           XSControl_WorkSessionMap& theMap)
{
  collectActiveWorkSessions(theWS, theName, theMap, Standard_True);
}

//=======================================================================
//function : CollectActiveWorkSessions
//purpose  :
//=======================================================================
void XSDRAWBase::CollectActiveWorkSessions(const TCollection_AsciiString& theName)
{
  collectActiveWorkSessions(Session(), theName, WorkSessionList(), Standard_True);
}