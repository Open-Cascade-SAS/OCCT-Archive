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

#ifndef _StepVisual_AnnotationLeaderCurveOccurrenceAndGeomReprItem_HeaderFile
#define _StepVisual_AnnotationLeaderCurveOccurrenceAndGeomReprItem_HeaderFile

#include <Standard.hxx>

#include <StepVisual_AnnotationCurveOccurrence.hxx>

//! Added for Dimensional Tolerances
//! Complex STEP entity AnnotationOccurrence & AnnotationOccurrence
//! & DraughtingAnnotationOccurrence & GeometricRepresentationItem
//! & LeaderCurve & RepresentationItem & StyledItem
class StepVisual_AnnotationLeaderCurveOccurrenceAndGeomReprItem : public StepVisual_AnnotationCurveOccurrence
{
public:

  Standard_EXPORT StepVisual_AnnotationLeaderCurveOccurrenceAndGeomReprItem();

  DEFINE_STANDARD_RTTIEXT(StepVisual_AnnotationLeaderCurveOccurrenceAndGeomReprItem, StepVisual_AnnotationCurveOccurrence)

};
#endif // _StepVisual_AnnotationLeaderCurveOccurrenceAndGeomReprItem_HeaderFile
