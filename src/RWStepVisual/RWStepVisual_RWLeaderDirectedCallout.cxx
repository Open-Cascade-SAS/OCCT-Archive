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

#include <RWStepVisual_RWLeaderDirectedCallout.hxx>

#include <Interface_Check.hxx>
#include <Interface_EntityIterator.hxx>
#include <StepData_StepReaderData.hxx>
#include <StepData_StepWriter.hxx>
#include <StepVisual_LeaderDirectedCallout.hxx>

//=======================================================================
//function : RWStepVisual_RWLeaderDirectedCallout
//purpose  :
//=======================================================================
RWStepVisual_RWLeaderDirectedCallout::RWStepVisual_RWLeaderDirectedCallout() {}

//=======================================================================
//function : ReadStep
//purpose  :
//=======================================================================
void RWStepVisual_RWLeaderDirectedCallout::ReadStep(const Handle(StepData_StepReaderData)& theData,
                                                    const Standard_Integer theNum,
                                                    Handle(Interface_Check)& theCheck,
                                                    const Handle(StepVisual_LeaderDirectedCallout)& theEnitity) const
{
  if (!theData->CheckNbParams(theNum, 2, theCheck, "draughting_callout"))
  {
    return;
  }

  // Inherited field : name
  Handle(TCollection_HAsciiString) aName;
  theData->ReadString(theNum, 1, "name", theCheck, aName);

  // Own field: contents
  Handle(StepVisual_HArray1OfDraughtingCalloutElement) aContents;
  StepVisual_DraughtingCalloutElement anEnt;
  Standard_Integer aNbSub;
  if (theData->ReadSubList(theNum, 2, "contents", theCheck, aNbSub))
  {
    Standard_Integer aNbElements = theData->NbParams(aNbSub);
    aContents = new StepVisual_HArray1OfDraughtingCalloutElement(1, aNbElements);
    for (Standard_Integer anInd = 1; anInd <= aNbElements; anInd++)
    {
      if (theData->ReadEntity(aNbSub, anInd, "content", theCheck, anEnt))
      {
        aContents->SetValue(anInd, anEnt);
      }
    }
  }

  // Initialisation of the read entity
  theEnitity->Init(aName, aContents);
}

//=======================================================================
//function : WriteStep
//purpose  :
//=======================================================================
void RWStepVisual_RWLeaderDirectedCallout::WriteStep(StepData_StepWriter& theSW,
                                                     const Handle(StepVisual_LeaderDirectedCallout)& theEnitity) const
{
  // Inherited field: name
  theSW.Send(theEnitity->Name());

  // Own field: contents
  theSW.OpenSub();
  for (Standard_Integer anInd = 1; anInd <= theEnitity->NbContents(); anInd++)
  {
    theSW.Send(theEnitity->ContentsValue(anInd).Value());
  }
  theSW.CloseSub();
}

//=======================================================================
//function : Share
//purpose  :
//=======================================================================
void RWStepVisual_RWLeaderDirectedCallout::Share(const Handle(StepVisual_LeaderDirectedCallout)& theEnitity,
                                                 Interface_EntityIterator& theIter) const
{
  // Own field: contents
  const Standard_Integer aNb = theEnitity->NbContents();
  for (Standard_Integer anInd = 1; anInd <= aNb; anInd++)
  {
    theIter.AddItem(theEnitity->ContentsValue(anInd).Value());
  }
}
