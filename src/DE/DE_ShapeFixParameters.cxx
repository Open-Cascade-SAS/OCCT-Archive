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

#include <DE_ShapeFixParameters.hxx>
#include <NCollection_DataMap.hxx>
#include <NCollection_Shared.hxx>
#include <TCollection_AsciiString.hxx>

//! Fill healing parameters map
void DE_ShapeFixParameters::FillParamsMap(DE_ShapeFixParameters::HealingParamMap& theMap)
{
  theMap.Bind("Tolerance3d", TCollection_AsciiString(Tolerance3d));
  theMap.Bind("MinTolerance3d", TCollection_AsciiString(MinTolerance3d));
  theMap.Bind("MaxTolerance3d", TCollection_AsciiString(MaxTolerance3d));
  theMap.Bind("FixFreeShellMode", TCollection_AsciiString(static_cast<int>(FixFreeShellMode)));
  theMap.Bind("FixFreeFaceMode", TCollection_AsciiString(static_cast<int>(FixFreeFaceMode)));
  theMap.Bind("FixFreeFaceMode", TCollection_AsciiString(static_cast<int>(FixFreeFaceMode)));
  theMap.Bind("FixFreeWireMode", TCollection_AsciiString(static_cast<int>(FixFreeWireMode)));
  theMap.Bind("FixSameParameterMode", TCollection_AsciiString(static_cast<int>(FixSameParameterMode)));
  theMap.Bind("FixSolidMode", TCollection_AsciiString(static_cast<int>(FixSolidMode)));
  theMap.Bind("FixShellOrientationMode", TCollection_AsciiString(static_cast<int>(FixShellOrientationMode)));
  theMap.Bind("CreateOpenSolidMode", TCollection_AsciiString(static_cast<int>(CreateOpenSolidMode)));
  theMap.Bind("FixShellMode", TCollection_AsciiString(static_cast<int>(FixShellMode)));
  theMap.Bind("FixFaceOrientationMode", TCollection_AsciiString(static_cast<int>(FixFaceOrientationMode)));
  theMap.Bind("FixFaceMode", TCollection_AsciiString(static_cast<int>(FixFaceMode)));
  theMap.Bind("FixWireMode", TCollection_AsciiString(static_cast<int>(FixWireMode)));
  theMap.Bind("FixOrientationMode", TCollection_AsciiString(static_cast<int>(FixOrientationMode)));
  theMap.Bind("FixAddNaturalBoundMode", TCollection_AsciiString(static_cast<int>(FixAddNaturalBoundMode)));
  theMap.Bind("FixMissingSeamMode", TCollection_AsciiString(static_cast<int>(FixMissingSeamMode)));
  theMap.Bind("FixSmallAreaWireMode", TCollection_AsciiString(static_cast<int>(FixSmallAreaWireMode)));
  theMap.Bind("RemoveSmallAreaFaceMode", TCollection_AsciiString(static_cast<int>(RemoveSmallAreaFaceMode)));
  theMap.Bind("FixIntersectingWiresMode", TCollection_AsciiString(static_cast<int>(FixIntersectingWiresMode)));
  theMap.Bind("FixLoopWiresMode", TCollection_AsciiString(static_cast<int>(FixLoopWiresMode)));
  theMap.Bind("FixSplitFaceMode", TCollection_AsciiString(static_cast<int>(FixSplitFaceMode)));
  theMap.Bind("AutoCorrectPrecisionMode", TCollection_AsciiString(static_cast<int>(AutoCorrectPrecisionMode)));
  theMap.Bind("ModifyTopologyMode", TCollection_AsciiString(static_cast<int>(ModifyTopologyMode)));
  theMap.Bind("ClosedWireMode", TCollection_AsciiString(static_cast<int>(ClosedWireMode)));
  theMap.Bind("PreferencePCurveMode", TCollection_AsciiString(static_cast<int>(PreferencePCurveMode)));
  theMap.Bind("FixReorderMode", TCollection_AsciiString(static_cast<int>(FixReorderMode)));
  theMap.Bind("FixSmallMode", TCollection_AsciiString(static_cast<int>(FixSmallMode)));
  theMap.Bind("FixConnectedMode", TCollection_AsciiString(static_cast<int>(FixConnectedMode)));
  theMap.Bind("FixEdgeCurvesMode", TCollection_AsciiString(static_cast<int>(FixEdgeCurvesMode)));
  theMap.Bind("FixDegeneratedMode", TCollection_AsciiString(static_cast<int>(FixDegeneratedMode)));
  theMap.Bind("FixLackingMode", TCollection_AsciiString(static_cast<int>(FixLackingMode)));
  theMap.Bind("FixSelfIntersectionMode", TCollection_AsciiString(static_cast<int>(FixSelfIntersectionMode)));
  theMap.Bind("RemoveLoopMode", TCollection_AsciiString(static_cast<int>(RemoveLoopMode)));
  theMap.Bind("FixReversed2dMode", TCollection_AsciiString(static_cast<int>(FixReversed2dMode)));
  theMap.Bind("FixRemovePCurveMode", TCollection_AsciiString(static_cast<int>(FixRemovePCurveMode)));
  theMap.Bind("FixRemoveCurve3dMode", TCollection_AsciiString(static_cast<int>(FixRemoveCurve3dMode)));
  theMap.Bind("FixAddPCurveMode", TCollection_AsciiString(static_cast<int>(FixAddPCurveMode)));
  theMap.Bind("FixAddCurve3dMode", TCollection_AsciiString(static_cast<int>(FixAddCurve3dMode)));
  theMap.Bind("FixSeamMode", TCollection_AsciiString(static_cast<int>(FixSeamMode)));
  theMap.Bind("FixShiftedMode", TCollection_AsciiString(static_cast<int>(FixShiftedMode)));
  theMap.Bind("FixEdgeSameParameterMode", TCollection_AsciiString(static_cast<int>(FixEdgeSameParameterMode)));
  theMap.Bind("FixNotchedEdgesMode", TCollection_AsciiString(static_cast<int>(FixNotchedEdgesMode)));
  theMap.Bind("FixTailMode", TCollection_AsciiString(static_cast<int>(FixTailMode)));
  theMap.Bind("MaxTailAngle", TCollection_AsciiString(static_cast<int>(MaxTailAngle)));
  theMap.Bind("MaxTailWidth", TCollection_AsciiString(static_cast<int>(MaxTailWidth)));
  theMap.Bind("FixSelfIntersectingEdgeMode", TCollection_AsciiString(static_cast<int>(FixSelfIntersectingEdgeMode)));
  theMap.Bind("FixIntersectingEdgesMode", TCollection_AsciiString(static_cast<int>(FixIntersectingEdgesMode)));
  theMap.Bind("FixNonAdjacentIntersectingEdgesMode", TCollection_AsciiString(static_cast<int>(FixNonAdjacentIntersectingEdgesMode)));
  theMap.Bind("FixVertexPositionMode", TCollection_AsciiString(static_cast<int>(FixVertexPositionMode)));
  theMap.Bind("FixVertexToleranceMode", TCollection_AsciiString(static_cast<int>(FixVertexToleranceMode)));
}
