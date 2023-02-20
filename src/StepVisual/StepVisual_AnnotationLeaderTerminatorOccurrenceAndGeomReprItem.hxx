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

#ifndef _StepVisual_AnnotationLeaderTerminatorOccurrenceAndGeomReprItem_HeaderFile
#define _StepVisual_AnnotationLeaderTerminatorOccurrenceAndGeomReprItem_HeaderFile

#include <Standard.hxx>

#include <StepVisual_AnnotationCurveOccurrence.hxx>

//! Added for Dimensional Tolerances
//! Complex STEP entity AnnotationOccurrence & AnnotationSymbolOccurrence
//! & DraughtingAnnotationOccurrence & GeometricRepresentationItem
//! & LeaderTerminator & RepresentationItem & StyledItem
class StepVisual_AnnotationLeaderTerminatorOccurrenceAndGeomReprItem : public StepVisual_AnnotationOccurrence
{
public:

  Standard_EXPORT StepVisual_AnnotationLeaderTerminatorOccurrenceAndGeomReprItem();
  
  Standard_EXPORT void Init(const Handle(TCollection_HAsciiString)& theName,
                            const Handle(StepVisual_HArray1OfPresentationStyleAssignment)& theStyles,
                            const Handle(Standard_Transient)& theItem,
                            const Handle(StepVisual_AnnotationCurveOccurrence)& theAnnotatedTerminator);

  const Handle(StepVisual_AnnotationCurveOccurrence)& Terminator() const { return myAnnotatedTerminator; }

  void SetTerminator(const Handle(StepVisual_AnnotationCurveOccurrence)& theAnnotatedTerminator) { myAnnotatedTerminator = theAnnotatedTerminator; }

  DEFINE_STANDARD_RTTIEXT(StepVisual_AnnotationLeaderTerminatorOccurrenceAndGeomReprItem, StepVisual_AnnotationOccurrence)

private:

  Handle(StepVisual_AnnotationCurveOccurrence) myAnnotatedTerminator;
};
#endif // _StepVisual_AnnotationLeaderTerminatorOccurrenceAndGeomReprItem_HeaderFile
