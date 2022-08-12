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

#include <StepRepr_MeasureWithUnit.hxx>

#include <Interface_Macros.hxx>

#include <StepBasic_MeasureWithUnit.hxx>
#include <StepRepr_MeasureRepresentationItem.hxx>
#include <StepRepr_ReprItemAndMeasureWithUnit.hxx>


//=======================================================================
//function : StepDimTol_MeasureWithUnit
//purpose  : 
//=======================================================================
StepRepr_MeasureWithUnit::StepRepr_MeasureWithUnit()
{
}

//=======================================================================
//function : CaseNum
//purpose  : 
//=======================================================================
Standard_Integer StepRepr_MeasureWithUnit::CaseNum(const Handle(Standard_Transient)& ent) const
{
  if (ent.IsNull()) return 0;
  if (ent->IsKind(STANDARD_TYPE(StepBasic_MeasureWithUnit))) return 1;
  if (ent->IsKind(STANDARD_TYPE(StepRepr_MeasureRepresentationItem))) return 2;
  if (ent->IsKind(STANDARD_TYPE(StepRepr_ReprItemAndMeasureWithUnit))) return 3;
  return 0;
}

//=======================================================================
//function : BasicMeasureWithUnit
//purpose  : 
//=======================================================================
Handle(StepBasic_MeasureWithUnit) StepRepr_MeasureWithUnit::BasicMeasureWithUnit() const
{
  return GetCasted(StepBasic_MeasureWithUnit, Value());
}

//=======================================================================
//function : MeasureRepresentationItem
//purpose  : 
//=======================================================================
Handle(StepRepr_MeasureRepresentationItem) StepRepr_MeasureWithUnit::MeasureRepresentationItem() const
{
  return GetCasted(StepRepr_MeasureRepresentationItem, Value());
}

//=======================================================================
//function : ReprItemAndMeasureWithUnit
//purpose  : 
//=======================================================================
Handle(StepRepr_ReprItemAndMeasureWithUnit) StepRepr_MeasureWithUnit::ReprItemAndMeasureWithUnit() const
{
  return GetCasted(StepRepr_ReprItemAndMeasureWithUnit, Value());
}
