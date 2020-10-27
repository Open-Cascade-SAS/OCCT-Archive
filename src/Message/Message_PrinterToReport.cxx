// Created on: 2001-01-06
// Created by: OCC Team
// Copyright (c) 2001-2014 OPEN CASCADE SAS
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

#include <Message_PrinterToReport.hxx>

#include <Message.hxx>
#include <Message_AlertExtended.hxx>
#include <Message_Attribute.hxx>
#include <Message_AttributeMeter.hxx>
#include <Message_AttributeObject.hxx>
#include <Message_AttributeStream.hxx>
#include <Message_Report.hxx>

#include <Standard_Dump.hxx>
#include <TCollection_ExtendedString.hxx>

IMPLEMENT_STANDARD_RTTIEXT(Message_PrinterToReport, Message_Printer)

//=======================================================================
//function : Destructor
//purpose  : 
//=======================================================================
Message_PrinterToReport::~Message_PrinterToReport()
{
  if (!myReport.IsNull())
  {
    myReport->UpdateActiveInMessenger();
  }
}

//=======================================================================
//function : Report
//purpose  : 
//=======================================================================
const Handle(Message_Report)& Message_PrinterToReport::Report() const
{
  if (!myReport.IsNull())
    return myReport;

  return Message::DefaultReport (Standard_True);
}

//=======================================================================
//function : Send
//purpose  :
//=======================================================================
void Message_PrinterToReport::SetReport (const Handle(Message_Report)& theReport)
{
  myReport = theReport;
  myReport->UpdateActiveInMessenger();
}

//=======================================================================
//function : Send
//purpose  :
//=======================================================================
void Message_PrinterToReport::Send (const Standard_SStream& theStream,
                                    const Message_Gravity   theGravity) const
{
  const Handle(Message_Report)& aReport = Report();
  if (!aReport->ActiveMetrics().IsEmpty())
  {
    sendMetricAlert (theStream.str().c_str(), theGravity);
    return;
  }
  if (Standard_Dump::HasChildKey(Standard_Dump::Text (theStream)))
  {
    Message_AlertExtended::AddAlert (aReport, new Message_AttributeStream (theStream, myName), theGravity);
    myName.Clear();
  }
  else
  {
    if (!myName.IsEmpty())
    {
      TCollection_AsciiString aName = myName;
      myName.Clear();
      send (aName, theGravity);
    }
    myName = Standard_Dump::Text (theStream);
  }
}

//=======================================================================
//function : Send
//purpose  : 
//=======================================================================
void Message_PrinterToReport::Send (const Handle(Standard_Transient)& /*theObject*/,
                                    const Message_Gravity /*theGravity*/) const
{
  //const Handle(Message_Report)& aReport = Report();
  //if (!aReport->ActiveMetrics().IsEmpty())
  //{
  //  sendMetricAlert (myValue, theGravity);
  //  return;
  //}

  //Message_AlertExtended::AddAlert (aReport, new Message_AttributeObject (theObject, myValue), theGravity);
}

//=======================================================================
//function : send
//purpose  : 
//=======================================================================
void Message_PrinterToReport::send (const TCollection_AsciiString& theString,
                                    const Message_Gravity theGravity) const
{
  if (!myName.IsEmpty())
  {
    send (myName, theGravity);
    myName.Clear();
  }

  const Handle(Message_Report)& aReport = Report();
  if (!aReport->ActiveMetrics().IsEmpty())
  {
    sendMetricAlert (theString, theGravity);
    return;
  }
  Message_AlertExtended::AddAlert (aReport, new Message_Attribute (theString), theGravity);
}

//=======================================================================
//function : sendMetricAlert
//purpose  : 
//=======================================================================
void Message_PrinterToReport::sendMetricAlert (const TCollection_AsciiString theValue,
                                               const Message_Gravity theGravity) const
{
  Message_AlertExtended::AddAlert (Report(), new Message_AttributeMeter (theValue), theGravity);
}

//=======================================================================
//function : DumpJson
//purpose  :
//=======================================================================
void Message_PrinterToReport::DumpJson (Standard_OStream& theOStream, Standard_Integer) const
{
  OCCT_DUMP_TRANSIENT_CLASS_BEGIN (theOStream)
}
