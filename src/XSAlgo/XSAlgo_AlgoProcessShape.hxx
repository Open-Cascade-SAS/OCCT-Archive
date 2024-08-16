// Copyright (c) 2024 OPEN CASCADE SAS
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

#ifndef _XSAlgo_AlgoProcessShape_HeaderFile
#define _XSAlgo_AlgoProcessShape_HeaderFile

#include <Standard.hxx>
#include <Standard_Type.hxx>

#include <DE_ShapeFixParameters.hxx>
#include <Message_ProgressRange.hxx>
#include <NCollection_DataMap.hxx>
#include <Standard_Integer.hxx>
#include <Standard_Transient.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <TopoDS_Shape.hxx>

class ShapeBuild_ReShape;
class XSAlgo_ToolContainer;
class TopoDS_Edge;
class TopoDS_Face;
class Transfer_TransientProcess;
class Transfer_FinderProcess;

class XSAlgo_AlgoProcessShape;
DEFINE_STANDARD_HANDLE(XSAlgo_AlgoProcessShape, Standard_Transient)

//! Class for shape processing
class XSAlgo_AlgoProcessShape : public Standard_Transient
{

public:

  //! Empty constructor
  Standard_EXPORT XSAlgo_AlgoProcessShape();
  
  //! Shape processing with specified tolerances
  Standard_EXPORT TopoDS_Shape ProcessShape();

  //! Fill map from DE shape healing parameters
  void FillMap(NCollection_DataMap<TCollection_AsciiString, TCollection_AsciiString>& theMap) const;

  //! Get shape for processing
  const TopoDS_Shape& GetShape() { return myShape; }

  //! Set shape for processing
  void SetShape(const TopoDS_Shape& theShape) { myShape = theShape; }

  //! Get name of the resource file
  const Standard_CString& GetPrscfile() { return myPrscfile; }

  //! Set name of the resource file
  void SetPrscfile(const Standard_CString& thePrscfile) { myPrscfile = thePrscfile; }

  //! Get name of the sequence of operators defined in the resource file for Shape Processing
  const Standard_CString& GetPseq() { return myPseq; }

  //! Set name of the sequence of operators defined in the resource file for Shape Processing
  void SetPseq(const Standard_CString& thePseq) { myPseq = thePseq; }

  //! Get information to be recorded in the translation map
  const Handle(Standard_Transient)& GetInfo() { return myInfo; }

  //! Set information to be recorded in the translation map
  void SetInfo(const Handle(Standard_Transient)& theInfo) { myInfo = theInfo; }

  //! Get flag of healing parameters
  const bool GetHealingParamsFlag() { return myDEHealingParamsUsage; }

  //! Set flag of healing parameters
  //! Uses defined DE healing parameters (if true) or parameters from resource file/Static_Interface
  void SetHealingParamsFlag(const bool theHealingParamsFlag) { myDEHealingParamsUsage = theHealingParamsFlag; }

  //! Get healing parameters
  const DE_ShapeFixParameters& GetHealingParameters() { return myHealingParameters; }

  //! Set healing parameters
  void SetHealingParameters(const DE_ShapeFixParameters& theHealingParams) { myHealingParameters = theHealingParams; }

  //! Get flag to proceed with non-manifold topology
  const bool GetNonManifold() { return myNonManifold; }

  //! Set flag to proceed with non-manifold topology
  void SetNonManifold(const bool theNonManifold) { myNonManifold = theNonManifold; }

  //! Get detalisation level
  const TopAbs_ShapeEnum GetDetalisationLevel() { return myDetalisationLevel; }

  //! Set detalisation level
  void SetDetalisationLevel(const TopAbs_ShapeEnum theDetalisationLevel) { myDetalisationLevel = theDetalisationLevel; }

  //! Get progress indicator
  const Message_ProgressRange& GetProgressRange() { return myProgressRange; }

  //! Set progress indicator
  void SetProgressRange(const Message_ProgressRange& theProgressRange) { myProgressRange = theProgressRange; }

  //! Get maximum allowed tolerance
  const double GetMaxTol() { return myMaxTol; }

  //! Set maximum allowed tolerance
  void SetMaxTol(const double theMaxTol) { myMaxTol = theMaxTol; }

  //! Get basic precision
  const double GetPrecision() { return myPrecision; }

  //! Set basic precision
  void SetPrecision(const double thePrecision) { myPrecision = thePrecision; }

  //! Get tool to record the modifications of input shape
  const Handle(ShapeBuild_ReShape)& GetReShape() { return myReShape; }

  //! Set tool to record the modifications of input shape
  void SetReShape(const Handle(ShapeBuild_ReShape)& theReShape) { myReShape = theReShape; }

  DEFINE_STANDARD_RTTIEXT(XSAlgo_AlgoProcessShape, Standard_Transient)

private:

  TopoDS_Shape     myShape;
  Standard_CString myPrscfile;
  Standard_CString myPseq;
  double           myPrecision;
  double           myMaxTol;
  bool             myNonManifold;
  TopAbs_ShapeEnum myDetalisationLevel;
  bool myDEHealingParamsUsage;
  DE_ShapeFixParameters myHealingParameters;
  Message_ProgressRange myProgressRange;
  Handle(Standard_Transient) myInfo;
  Handle(ShapeBuild_ReShape) myReShape;

};

#endif // _XSAlgo_AlgoProcessShape_HeaderFile
