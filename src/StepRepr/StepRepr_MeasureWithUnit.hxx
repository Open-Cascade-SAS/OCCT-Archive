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

#ifndef _StepRepr_MeasureWithUnit_HeaderFile
#define _StepRepr_MeasureWithUnit_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>

#include <StepData_SelectType.hxx>

class StepBasic_MeasureWithUnit;
class StepRepr_MeasureRepresentationItem;
class StepRepr_ReprItemAndMeasureWithUnit;

//! Representation of STEP SELECT type MeasureWithUnit
class StepRepr_MeasureWithUnit : public StepData_SelectType
{
public:
  DEFINE_STANDARD_ALLOC

  //! Empty constructor
  Standard_EXPORT StepRepr_MeasureWithUnit();

  //! Recognizes a kind of MeasureWithUnit select type
  //! 1 -> MeasureWithUnit from StepBasic
  //! 2 -> MeasureRepresentationItem from StepRepr
  //! 3 -> ReprItemAndMeasureWithUnit from StepRepr
  //! 0 else
  Standard_EXPORT Standard_Integer CaseNum(const Handle(Standard_Transient)& ent) const;

  //! returns value as a BasicMeasureWithUnit (Null if another type)
  Standard_EXPORT Handle(StepBasic_MeasureWithUnit) BasicMeasureWithUnit() const;

  //! returns Value as a MeasureRepresentationItem (Null if another type)
  Standard_EXPORT Handle(StepRepr_MeasureRepresentationItem) MeasureRepresentationItem() const;

  //! returns Value as a ReprItemAndMeasureWithUnit (Null if another type)
  Standard_EXPORT Handle(StepRepr_ReprItemAndMeasureWithUnit) ReprItemAndMeasureWithUnit() const;

};
#endif // _StepRepr_MeasureWithUnit_HeaderFile
