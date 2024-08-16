// Created on: 2000-01-19
// Created by: data exchange team
// Copyright (c) 2000-2014 OPEN CASCADE SAS
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

#include <XSAlgo_AlgoProcessShape.hxx>

#include <Interface_Static.hxx>
#include <Resource_Manager.hxx>
#include <ShapeAlgo.hxx>
#include <ShapeAlgo_AlgoContainer.hxx>
#include <ShapeAlgo_ToolContainer.hxx>
#include <ShapeBuild_ReShape.hxx>
#include <ShapeExtend_MsgRegistrator.hxx>
#include <ShapeFix_Shape.hxx>
#include <ShapeProcess.hxx>
#include <ShapeProcess_ShapeContext.hxx>

IMPLEMENT_STANDARD_RTTIEXT(XSAlgo_AlgoProcessShape, Standard_Transient)

//=======================================================================
//function : XSAlgo_AlgoProcessShape
//purpose  : 
//=======================================================================
XSAlgo_AlgoProcessShape::XSAlgo_AlgoProcessShape()
  : myPrscfile(""),
  myPseq(""),
  myPrecision(0.001),
  myMaxTol(1.),
  myNonManifold(false),
  myDetalisationLevel(TopAbs_VERTEX),
  myDEHealingParamsUsage(false),
  myProgressRange(Message_ProgressRange())
{
}

//=======================================================================
//function : ProcessShape
//purpose  :
//=======================================================================
TopoDS_Shape XSAlgo_AlgoProcessShape::ProcessShape()
{
  if (myShape.IsNull())
  {
    return myShape;
  }

  Standard_CString aRscfile = "";
  Handle(ShapeProcess_ShapeContext) aContext = Handle(ShapeProcess_ShapeContext)::DownCast(myInfo);
  if (aContext.IsNull())
  {
    aRscfile = Interface_Static::CVal(myPrscfile);
    aContext = new ShapeProcess_ShapeContext(myShape, aRscfile);
    if (myDEHealingParamsUsage)
    {
      // Use defined shape healing parameters
      FillMap(aContext->ResourceManager()->GetMap());
    }
    else if (!aContext->ResourceManager()->IsInitialized())
    {
      // If resource file wasn't found, use static values instead
      Interface_Static::FillMap(aContext->ResourceManager()->GetMap());
    }
  }
  aContext->SetDetalisation(myDetalisationLevel);
  aContext->SetNonManifold(myNonManifold);
  myInfo = aContext;

  Standard_CString aSeq = Interface_Static::CVal(myPseq);
  if (!aSeq) aSeq = myPseq;

  // if resource file is not loaded or does not define <seq>.exec.op, 
  // do default fixes
  Handle(Resource_Manager) aRsc = aContext->ResourceManager();
  TCollection_AsciiString aStr(aSeq);
  aStr += ".exec.op";
  if (strncmp(myPseq, "read.", 5) == 0)
  {
    aRsc->SetResource(aStr.ToCString(), "FixShape");
    try {
      OCC_CATCH_SIGNALS
      Handle(ShapeExtend_MsgRegistrator) aMsg = new ShapeExtend_MsgRegistrator;
      Handle(ShapeFix_Shape) aSfs = ShapeAlgo::AlgoContainer()->ToolContainer()->FixShape();
      aSfs->Init(myShape);
      aSfs->SetMsgRegistrator(aMsg);
      aSfs->SetPrecision(myPrecision);
      aSfs->SetMaxTolerance(myMaxTol);
      aSfs->FixFaceTool()->FixWireTool()->FixSameParameterMode() = Standard_False;
      if (strcmp(aRscfile, "IGES") == 0 || strcmp(aRscfile, "XT") == 0)
      {
        aSfs->FixSolidTool()->CreateOpenSolidMode() = Standard_True;
      }
      else
      {
        aSfs->FixSolidTool()->CreateOpenSolidMode() = Standard_False;
      }
      aSfs->SetContext(myReShape);
      aSfs->Perform(myProgressRange);

      TopoDS_Shape aShape = aSfs->Shape();
      if (!aShape.IsNull() && aShape != myShape)
      {
        aContext->RecordModification(aSfs->Context(), aMsg);
        aContext->SetResult(aShape);
      }
    }
    catch (Standard_Failure const& anException)
    {
#ifdef OCCT_DEBUG
      std::cout << "Error: XSAlgo_AlgoContainer::ProcessShape(): Exception in ShapeFix::Shape" << std::endl;
      anException.Print(std::cout); std::cout << std::endl;
#endif
      (void)anException;
    }
    return aContext->Result();
  }
  else if (strncmp(myPseq, "write.", 6) == 0)
  {
    if (strcmp(aRscfile, "STEP") == 0)
    {
      aRsc->SetResource(aStr.ToCString(), "SplitCommonVertex,DirectFaces");
    }
    else
    {
      aRsc->SetResource(aStr.ToCString(), "DirectFaces");
    }
  }

  // Define runtime tolerances and do Shape Processing 
  aRsc->SetResource("Runtime.Tolerance", myPrecision);
  aRsc->SetResource("Runtime.MaxTolerance", myMaxTol);

  if (!ShapeProcess::Perform(aContext, aSeq, myProgressRange))
    return myShape; // return original shape

  return aContext->Result();
}

//=======================================================================
//function : FillMap
//purpose  :
//=======================================================================
void XSAlgo_AlgoProcessShape::FillMap(NCollection_DataMap<TCollection_AsciiString, TCollection_AsciiString>& theMap) const
{
  theMap.Clear();
  theMap.Bind("Tolerance3d", TCollection_AsciiString(myHealingParameters.Tolerance3d));
  theMap.Bind("MinTolerance3d", TCollection_AsciiString(myHealingParameters.MinTolerance3d));
  theMap.Bind("MaxTolerance3d", TCollection_AsciiString(myHealingParameters.MaxTolerance3d));
  theMap.Bind("FixFreeShellMode", TCollection_AsciiString(static_cast<int>(myHealingParameters.FixFreeShellMode)));
  theMap.Bind("FixFreeFaceMode", TCollection_AsciiString(static_cast<int>(myHealingParameters.FixFreeFaceMode)));
  theMap.Bind("FixFreeFaceMode", TCollection_AsciiString(static_cast<int>(myHealingParameters.FixFreeFaceMode)));
  theMap.Bind("FixFreeWireMode", TCollection_AsciiString(static_cast<int>(myHealingParameters.FixFreeWireMode)));
  theMap.Bind("FixSameParameterMode", TCollection_AsciiString(static_cast<int>(myHealingParameters.FixSameParameterMode)));
  theMap.Bind("FixSolidMode", TCollection_AsciiString(static_cast<int>(myHealingParameters.FixSolidMode)));
  theMap.Bind("FixShellOrientationMode", TCollection_AsciiString(static_cast<int>(myHealingParameters.FixShellOrientationMode)));
  theMap.Bind("CreateOpenSolidMode", TCollection_AsciiString(static_cast<int>(myHealingParameters.CreateOpenSolidMode)));
  theMap.Bind("FixShellMode", TCollection_AsciiString(static_cast<int>(myHealingParameters.FixShellMode)));
  theMap.Bind("FixFaceOrientationMode", TCollection_AsciiString(static_cast<int>(myHealingParameters.FixFaceOrientationMode)));
  theMap.Bind("FixFaceMode", TCollection_AsciiString(static_cast<int>(myHealingParameters.FixFaceMode)));
  theMap.Bind("FixWireMode", TCollection_AsciiString(static_cast<int>(myHealingParameters.FixWireMode)));
  theMap.Bind("FixOrientationMode", TCollection_AsciiString(static_cast<int>(myHealingParameters.FixOrientationMode)));
  theMap.Bind("FixAddNaturalBoundMode", TCollection_AsciiString(static_cast<int>(myHealingParameters.FixAddNaturalBoundMode)));
  theMap.Bind("FixMissingSeamMode", TCollection_AsciiString(static_cast<int>(myHealingParameters.FixMissingSeamMode)));
  theMap.Bind("FixSmallAreaWireMode", TCollection_AsciiString(static_cast<int>(myHealingParameters.FixSmallAreaWireMode)));
  theMap.Bind("RemoveSmallAreaFaceMode", TCollection_AsciiString(static_cast<int>(myHealingParameters.RemoveSmallAreaFaceMode)));
  theMap.Bind("FixIntersectingWiresMode", TCollection_AsciiString(static_cast<int>(myHealingParameters.FixIntersectingWiresMode)));
  theMap.Bind("FixLoopWiresMode", TCollection_AsciiString(static_cast<int>(myHealingParameters.FixLoopWiresMode)));
  theMap.Bind("FixSplitFaceMode", TCollection_AsciiString(static_cast<int>(myHealingParameters.FixSplitFaceMode)));
  theMap.Bind("AutoCorrectPrecisionMode", TCollection_AsciiString(static_cast<int>(myHealingParameters.AutoCorrectPrecisionMode)));
  theMap.Bind("ModifyTopologyMode", TCollection_AsciiString(static_cast<int>(myHealingParameters.ModifyTopologyMode)));
  theMap.Bind("ClosedWireMode", TCollection_AsciiString(static_cast<int>(myHealingParameters.ClosedWireMode)));
  theMap.Bind("PreferencePCurveMode", TCollection_AsciiString(static_cast<int>(myHealingParameters.PreferencePCurveMode)));
  theMap.Bind("FixReorderMode", TCollection_AsciiString(static_cast<int>(myHealingParameters.FixReorderMode)));
  theMap.Bind("FixSmallMode", TCollection_AsciiString(static_cast<int>(myHealingParameters.FixSmallMode)));
  theMap.Bind("FixConnectedMode", TCollection_AsciiString(static_cast<int>(myHealingParameters.FixConnectedMode)));
  theMap.Bind("FixEdgeCurvesMode", TCollection_AsciiString(static_cast<int>(myHealingParameters.FixEdgeCurvesMode)));
  theMap.Bind("FixDegeneratedMode", TCollection_AsciiString(static_cast<int>(myHealingParameters.FixDegeneratedMode)));
  theMap.Bind("FixLackingMode", TCollection_AsciiString(static_cast<int>(myHealingParameters.FixLackingMode)));
  theMap.Bind("FixSelfIntersectionMode", TCollection_AsciiString(static_cast<int>(myHealingParameters.FixSelfIntersectionMode)));
  theMap.Bind("RemoveLoopMode", TCollection_AsciiString(static_cast<int>(myHealingParameters.RemoveLoopMode)));
  theMap.Bind("FixReversed2dMode", TCollection_AsciiString(static_cast<int>(myHealingParameters.FixReversed2dMode)));
  theMap.Bind("FixRemovePCurveMode", TCollection_AsciiString(static_cast<int>(myHealingParameters.FixRemovePCurveMode)));
  theMap.Bind("FixRemoveCurve3dMode", TCollection_AsciiString(static_cast<int>(myHealingParameters.FixRemoveCurve3dMode)));
  theMap.Bind("FixAddPCurveMode", TCollection_AsciiString(static_cast<int>(myHealingParameters.FixAddPCurveMode)));
  theMap.Bind("FixAddCurve3dMode", TCollection_AsciiString(static_cast<int>(myHealingParameters.FixAddCurve3dMode)));
  theMap.Bind("FixSeamMode", TCollection_AsciiString(static_cast<int>(myHealingParameters.FixSeamMode)));
  theMap.Bind("FixShiftedMode", TCollection_AsciiString(static_cast<int>(myHealingParameters.FixShiftedMode)));
  theMap.Bind("FixEdgeSameParameterMode", TCollection_AsciiString(static_cast<int>(myHealingParameters.FixEdgeSameParameterMode)));
  theMap.Bind("FixNotchedEdgesMode", TCollection_AsciiString(static_cast<int>(myHealingParameters.FixNotchedEdgesMode)));
  theMap.Bind("FixTailMode", TCollection_AsciiString(static_cast<int>(myHealingParameters.FixTailMode)));
  theMap.Bind("MaxTailAngle", TCollection_AsciiString(static_cast<int>(myHealingParameters.MaxTailAngle)));
  theMap.Bind("MaxTailWidth", TCollection_AsciiString(static_cast<int>(myHealingParameters.MaxTailWidth)));
  theMap.Bind("FixSelfIntersectingEdgeMode", TCollection_AsciiString(static_cast<int>(myHealingParameters.FixSelfIntersectingEdgeMode)));
  theMap.Bind("FixIntersectingEdgesMode", TCollection_AsciiString(static_cast<int>(myHealingParameters.FixIntersectingEdgesMode)));
  theMap.Bind("FixNonAdjacentIntersectingEdgesMode", TCollection_AsciiString(static_cast<int>(myHealingParameters.FixNonAdjacentIntersectingEdgesMode)));
  theMap.Bind("FixVertexPositionMode", TCollection_AsciiString(static_cast<int>(myHealingParameters.FixVertexPositionMode)));
  theMap.Bind("FixVertexToleranceMode", TCollection_AsciiString(static_cast<int>(myHealingParameters.FixVertexToleranceMode)));
}
