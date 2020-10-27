// Copyright (c) 2020 OPEN CASCADE SAS
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

#include <Message_AttributeMeter.hxx>
#include <Message_AlertExtended.hxx>
#include <Message.hxx>
#include <Message_Report.hxx>
#include <OSD_Chronometer.hxx>
#include <OSD_Timer.hxx>

#include <Precision.hxx>
#include <Standard_Dump.hxx>

IMPLEMENT_STANDARD_RTTIEXT(Message_AttributeMeter, Message_Attribute)

//=======================================================================
//function : SetValues
//purpose  : 
//=======================================================================
Message_AttributeMeter::Message_AttributeMeter (const TCollection_AsciiString& theName)
: Message_Attribute(theName)
{
}

//=======================================================================
//function : HasMetric
//purpose  : 
//=======================================================================
Standard_Boolean Message_AttributeMeter::HasMetric (const Message_MetricType& theMetric) const
{
  return myMetrics.IsBound (theMetric);
}

//=======================================================================
//function : IsMetricValid
//purpose  : 
//=======================================================================
Standard_Boolean Message_AttributeMeter::IsMetricValid (const Message_MetricType& theMetric) const
{
  return Abs (StartValue (theMetric) - UndefinedMetricValue()) > Precision::Confusion() &&
         Abs (StopValue (theMetric) - UndefinedMetricValue()) > Precision::Confusion();
}

//=======================================================================
//function : StartValue
//purpose  : 
//=======================================================================
Standard_Real Message_AttributeMeter::StartValue (const Message_MetricType& theMetric) const
{
  if (!HasMetric (theMetric))
    return UndefinedMetricValue();

  return myMetrics.Seek (theMetric)->first;
}

//=======================================================================
//function : SetStartValue
//purpose  : 
//=======================================================================
void Message_AttributeMeter::SetStartValue (const Message_MetricType& theMetric, const Standard_Real theValue)
{
  //std::cout << GetName() << ": " << theValue << " (start)" << std::endl;

  if (!HasMetric (theMetric))
  {
    myMetrics.Bind (theMetric, std::make_pair (theValue, UndefinedMetricValue()));
  }
  myMetrics.ChangeFind (theMetric).first = theValue;
}

//=======================================================================
//function : StopValue
//purpose  : 
//=======================================================================
Standard_Real Message_AttributeMeter::StopValue (const Message_MetricType& theMetric) const
{
  if (!HasMetric (theMetric))
    return UndefinedMetricValue();

  return myMetrics.Find (theMetric).second;
}

//=======================================================================
//function : SetStopValue
//purpose  : 
//=======================================================================
void Message_AttributeMeter::SetStopValue (const Message_MetricType& theMetric, const Standard_Real theValue)
{
  //std::cout << GetName() << ": " << theValue << " (stop)" << std::endl;
  if (!HasMetric (theMetric))
  {
    // start value should be already set
    return;
  }
  myMetrics.ChangeFind (theMetric).second = theValue;
}

//=======================================================================
//function : SetAlertMetrics
//purpose  :
//=======================================================================
void Message_AttributeMeter::SetAlertMetrics (const Handle(Message_AlertExtended)& theAlert,
                                              const Standard_Boolean theStartValue)
{
  if (theAlert.IsNull())
    return;

  Handle(Message_AttributeMeter) aMeterAttribute = Handle(Message_AttributeMeter)::DownCast (theAlert->Attribute());
  if (aMeterAttribute.IsNull())
    return;

  Handle(Message_Report) aReport = Message::DefaultReport (Standard_True);
  const NCollection_Map<Message_MetricType>& anActiveMetrics = aReport->ActiveMetrics();

  // time metrics
  if (anActiveMetrics.Contains (Message_MetricType_WallClock))
  {
    OSD_Timer aTimer;
    aTimer.Start();
    Standard_Real aTime = aTimer.StartTime();
    if (theStartValue)
      aMeterAttribute->SetStartValue (Message_MetricType_WallClock, aTime);
    else
      aMeterAttribute->SetStopValue (Message_MetricType_WallClock, aTime);
  }
  if (anActiveMetrics.Contains (Message_MetricType_UserTimeCPU) ||
      anActiveMetrics.Contains (Message_MetricType_SystemTimeInfo))
  {
    Standard_Real aUserSeconds, aSystemSeconds;
    OSD_Chronometer::GetProcessCPU (aUserSeconds, aSystemSeconds);

    if (anActiveMetrics.Contains (Message_MetricType_UserTimeCPU))
    {
      if (theStartValue)
        aMeterAttribute->SetStartValue (Message_MetricType_UserTimeCPU, aUserSeconds);
      else
        aMeterAttribute->SetStopValue (Message_MetricType_UserTimeCPU, aUserSeconds);
    }
    if (anActiveMetrics.Contains (Message_MetricType_SystemTimeInfo))
    {
      if (theStartValue)
        aMeterAttribute->SetStartValue (Message_MetricType_SystemTimeInfo, aSystemSeconds);
      else
        aMeterAttribute->SetStopValue (Message_MetricType_SystemTimeInfo, aSystemSeconds);
    }
  }
  // memory metrics
  OSD_MemInfo aMemInfo (Standard_False);
  aMemInfo.SetActive (Standard_False);
  NCollection_Map<OSD_MemInfo::Counter> aCounters;
  Standard_Boolean isCounterFound = Standard_False;
  for (NCollection_Map<Message_MetricType>::Iterator anIterator (anActiveMetrics); anIterator.More(); anIterator.Next())
  {
    OSD_MemInfo::Counter anInfoCounter;
    if (!Message::ToOSDMetric (anIterator.Value(), anInfoCounter))
      continue;

    aCounters.Add (anInfoCounter);
    aMemInfo.SetActive (anInfoCounter, Standard_True);
    isCounterFound = Standard_True;
  }
  if (aCounters.IsEmpty())
    return;

  aMemInfo.Update();
  Message_MetricType aMetricType;
  for (NCollection_Map<OSD_MemInfo::Counter>::Iterator anIterator (aCounters); anIterator.More(); anIterator.Next())
  {
    if (!Message::ToMessageMetric (anIterator.Value(), aMetricType))
      continue;

    if (theStartValue)
      aMeterAttribute->SetStartValue (aMetricType, (Standard_Real)aMemInfo.ValuePreciseMiB (anIterator.Value()));
    else
      aMeterAttribute->SetStopValue (aMetricType, (Standard_Real)aMemInfo.ValuePreciseMiB (anIterator.Value()));
  }
}

//=======================================================================
//function : DumpJson
//purpose  :
//=======================================================================
void Message_AttributeMeter::DumpJson (Standard_OStream& theOStream,
                                       Standard_Integer theDepth) const
{
  OCCT_DUMP_TRANSIENT_CLASS_BEGIN (theOStream)
  OCCT_DUMP_BASE_CLASS (theOStream, theDepth, Message_Attribute)

  for (NCollection_DataMap<Message_MetricType, StartToStopValue>::Iterator anIterator (myMetrics);
       anIterator.More(); anIterator.Next())
  {
    OCCT_DUMP_VECTOR_CLASS (theOStream, Message::MetricToString (anIterator.Key()),
                            2, anIterator.Value().first, anIterator.Value().second)
  }
}
