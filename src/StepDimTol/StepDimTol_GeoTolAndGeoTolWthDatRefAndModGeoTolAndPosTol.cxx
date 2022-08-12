// Copyright (c) 1999-2014 OPEN CASCADE SAS
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


#include <StepDimTol_GeometricToleranceWithDatumReference.hxx>
#include <StepDimTol_GeoTolAndGeoTolWthDatRefAndModGeoTolAndPosTol.hxx>
#include <StepDimTol_ModifiedGeometricTolerance.hxx>
#include <StepDimTol_PositionTolerance.hxx>
#include <StepRepr_ShapeAspect.hxx>
#include <TCollection_HAsciiString.hxx>

IMPLEMENT_STANDARD_RTTIEXT(StepDimTol_GeoTolAndGeoTolWthDatRefAndModGeoTolAndPosTol,StepDimTol_GeometricTolerance)

//=======================================================================
//function : StepDimTol_GeoTolAndGeoTolWthDatRefAndModGeoTolAndPosTol
//purpose  : 
//=======================================================================
StepDimTol_GeoTolAndGeoTolWthDatRefAndModGeoTolAndPosTol::StepDimTol_GeoTolAndGeoTolWthDatRefAndModGeoTolAndPosTol()
{
}


//=======================================================================
//function : Init
//purpose  : 
//=======================================================================

void StepDimTol_GeoTolAndGeoTolWthDatRefAndModGeoTolAndPosTol::Init
  (const Handle(TCollection_HAsciiString)& theName,
   const Handle(TCollection_HAsciiString)& theDescription,
   const Handle(Standard_Transient)& theMagnitude,
   const Handle(StepRepr_ShapeAspect)& theTolerancedShapeAspect,
   const Handle(StepDimTol_GeometricToleranceWithDatumReference)& theGTWDR,
   const Handle(StepDimTol_ModifiedGeometricTolerance)& theMGT)
{
  SetName(theName);
  SetDescription(theDescription);
  SetMagnitude(theMagnitude);
  SetTolerancedShapeAspect(theTolerancedShapeAspect);
  myGeometricToleranceWithDatumReference = theGTWDR;
  myModifiedGeometricTolerance = theMGT;
}

//=======================================================================
//function : Init
//purpose  : 
//=======================================================================

void StepDimTol_GeoTolAndGeoTolWthDatRefAndModGeoTolAndPosTol::Init
  (const Handle(TCollection_HAsciiString)& theName,
   const Handle(TCollection_HAsciiString)& theDescription,
   const Handle(Standard_Transient)& theMagnitude,
   const StepDimTol_GeometricToleranceTarget& theTolerancedShapeAspect,
   const Handle(StepDimTol_GeometricToleranceWithDatumReference)& theGTWDR,
   const Handle(StepDimTol_ModifiedGeometricTolerance)& theMGT)
{
  SetName(theName);
  SetDescription(theDescription);
  SetMagnitude(theMagnitude);
  SetTolerancedShapeAspect(theTolerancedShapeAspect);
  myGeometricToleranceWithDatumReference = theGTWDR;
  myModifiedGeometricTolerance = theMGT;
}


//=======================================================================
//function : SetGeometricToleranceWithDatumReference
//purpose  : 
//=======================================================================

void StepDimTol_GeoTolAndGeoTolWthDatRefAndModGeoTolAndPosTol::SetGeometricToleranceWithDatumReference
  (const Handle(StepDimTol_GeometricToleranceWithDatumReference)& theGTWDR)
{
  myGeometricToleranceWithDatumReference = theGTWDR;
}


//=======================================================================
//function : GetGeometricToleranceWithDatumReference
//purpose  : 
//=======================================================================

Handle(StepDimTol_GeometricToleranceWithDatumReference) StepDimTol_GeoTolAndGeoTolWthDatRefAndModGeoTolAndPosTol::GetGeometricToleranceWithDatumReference() const
{
  return myGeometricToleranceWithDatumReference;
}


//=======================================================================
//function : SetModifiedGeometricTolerance
//purpose  : 
//=======================================================================

void StepDimTol_GeoTolAndGeoTolWthDatRefAndModGeoTolAndPosTol::SetModifiedGeometricTolerance
  (const Handle(StepDimTol_ModifiedGeometricTolerance)& theMGT)
{
  myModifiedGeometricTolerance = theMGT;
}


//=======================================================================
//function : GetModifiedGeometricTolerance
//purpose  : 
//=======================================================================

Handle(StepDimTol_ModifiedGeometricTolerance) StepDimTol_GeoTolAndGeoTolWthDatRefAndModGeoTolAndPosTol::GetModifiedGeometricTolerance() const
{
  return myModifiedGeometricTolerance;
}


//=======================================================================
//function : SetPositionTolerance
//purpose  : 
//=======================================================================

void StepDimTol_GeoTolAndGeoTolWthDatRefAndModGeoTolAndPosTol::SetPositionTolerance
  (const Handle(StepDimTol_PositionTolerance)& thePT)
{
  myPositionTolerance = thePT;
}


//=======================================================================
//function : GetPositionTolerance
//purpose  : 
//=======================================================================

Handle(StepDimTol_PositionTolerance) StepDimTol_GeoTolAndGeoTolWthDatRefAndModGeoTolAndPosTol::GetPositionTolerance() const
{
  return myPositionTolerance;
}

