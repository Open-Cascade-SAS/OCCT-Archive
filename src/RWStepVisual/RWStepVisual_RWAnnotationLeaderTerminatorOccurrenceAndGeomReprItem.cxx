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

#include <RWStepVisual_RWAnnotationLeaderTerminatorOccurrenceAndGeomReprItem.hxx>

#include <Interface_Check.hxx>
#include <Interface_EntityIterator.hxx>
#include <StepData_StepReaderData.hxx>
#include <StepData_StepWriter.hxx>
#include <StepVisual_AnnotationLeaderTerminatorOccurrenceAndGeomReprItem.hxx>

//=======================================================================
//function : ReadStep
//purpose  :
//=======================================================================
void RWStepVisual_RWAnnotationLeaderTerminatorOccurrenceAndGeomReprItem::ReadStep(
  const Handle(StepData_StepReaderData)& theData,
  const Standard_Integer theNum,
  Handle(Interface_Check)& theCheck,
  const Handle(StepVisual_AnnotationLeaderTerminatorOccurrenceAndGeomReprItem)& theEntity) const
{
  Standard_Integer aNum = 0;
  theData->NamedForComplex("REPRESENTATION_ITEM", "RPRITM", theNum, aNum, theCheck);
  // Inherited field : name 
  Handle(TCollection_HAsciiString) aName;
  theData->ReadString(aNum, 1, "name", theCheck, aName);

  theData->NamedForComplex("STYLED_ITEM", "STYITM", theNum, aNum, theCheck);
  // Inherited field : styles
  Handle(StepVisual_HArray1OfPresentationStyleAssignment) aStyles;
  Handle(StepVisual_PresentationStyleAssignment) anEnt;
  Standard_Integer aSubNum = 0;
  if (theData->ReadSubList(aNum, 1, "styles", theCheck, aSubNum))
  {
    Standard_Integer aNbParams = theData->NbParams(aSubNum);
    aStyles = new StepVisual_HArray1OfPresentationStyleAssignment(1, aNbParams);
    for (Standard_Integer aParamInd = 1; aParamInd <= aNbParams; aParamInd++)
    {
      if (theData->ReadEntity(aSubNum, aParamInd, "presentation_style_assignment", theCheck,
          STANDARD_TYPE(StepVisual_PresentationStyleAssignment), anEnt))
      {
        aStyles->SetValue(aParamInd, anEnt);
      }
    }
  }

  // Inherited field : item
  Handle(Standard_Transient) aItem;
  theData->ReadEntity(aNum, 2, "item", theCheck, STANDARD_TYPE(Standard_Transient), aItem);

  theData->NamedForComplex("TERMINATOR_SYMBOL", "STYITM", theNum, aNum, theCheck);

  Handle(StepVisual_AnnotationCurveOccurrence) aTermonator;
  theData->ReadEntity(aNum, 1, "annotated_curve", theCheck, STANDARD_TYPE(StepVisual_AnnotationCurveOccurrence), aTermonator);

  // Initialization of the read entity
  theEntity->Init(aName, aStyles, aItem, aTermonator);
}

//=======================================================================
//function : WriteStep
//purpose  :
//=======================================================================
void RWStepVisual_RWAnnotationLeaderTerminatorOccurrenceAndGeomReprItem::WriteStep(
  StepData_StepWriter& theSW,
  const Handle(StepVisual_AnnotationLeaderTerminatorOccurrenceAndGeomReprItem)& theEntity) const
{
  theSW.StartEntity("ANNOTATION_CURVE_OCCURRENCE");
  theSW.StartEntity("ANNOTATION_SYMBOL_OCCURRENCE");
  theSW.StartEntity("DRAUGHTING_ANNOTATION_OCCURRENCE");
  theSW.StartEntity("GEOMETRIC_REPRESENTATION_ITEM");
  theSW.StartEntity("LEADER_TERMINATOR");
  theSW.StartEntity("REPRESENTATION_ITEM");
  //Inherited field : name
  theSW.Send(theEntity->Name());

  theSW.StartEntity("STYLED_ITEM");
  // Inherited field : styles
  theSW.OpenSub();
  for (StepVisual_HArray1OfPresentationStyleAssignment::Iterator anIter(theEntity->Styles()->Array1());
       anIter.More(); anIter.Next())
  {
    theSW.Send(anIter.Value());
  }
  theSW.CloseSub();

  // Inherited field : item
  theSW.Send(theEntity->Item());

  theSW.StartEntity("TERMINATOR_SYMBOL");
  theSW.Send(theEntity->Terminator());
}

//=======================================================================
//function : Share
//purpose  :
//=======================================================================
void RWStepVisual_RWAnnotationLeaderTerminatorOccurrenceAndGeomReprItem::Share(
  const Handle(StepVisual_AnnotationLeaderTerminatorOccurrenceAndGeomReprItem)& theEntity,
  Interface_EntityIterator& theIter) const
{
  for (StepVisual_HArray1OfPresentationStyleAssignment::Iterator anIter(theEntity->Styles()->Array1());
       anIter.More(); anIter.Next())
  {
    theIter.GetOneItem(anIter.Value());
  }
  theIter.GetOneItem(theEntity->Item());
  theIter.GetOneItem(theEntity->Terminator());
}
