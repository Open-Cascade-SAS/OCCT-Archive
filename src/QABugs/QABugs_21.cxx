// Created on: 2025-06-01
// Copyright (c) 2025 OPEN CASCADE SAS
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

#include <QABugs.hxx>

#include <AIS_InteractiveContext.hxx>
#include <AIS_InteractiveObject.hxx>
#include <Graphic3d_ArrayOfTriangles.hxx>
#include <Graphic3d_Group.hxx>
#include <Graphic3d_ZLayerId.hxx>
#include <Prs3d_Presentation.hxx>
#include <Quantity_Color.hxx>
#include <Select3D_SensitiveBox.hxx>
#include <SelectMgr_EntityOwner.hxx>
#include <SelectMgr_Selection.hxx>
#include <ViewerTest.hxx>
#include <ViewerTest_DoubleMapOfInteractiveAndName.hxx>

#if !defined(_WIN32)
extern ViewerTest_DoubleMapOfInteractiveAndName& GetMapOfAIS();
#else
Standard_EXPORT ViewerTest_DoubleMapOfInteractiveAndName& GetMapOfAIS();
#endif

namespace
{
//! Presentation with two groups: Group 1 inherits the object Z-layer (Topmost),
//! Group 2 is explicitly placed in Default layer via SetZLayer().
class OCC33504_MixedLayerObject : public AIS_InteractiveObject
{
  DEFINE_STANDARD_RTTI_INLINE(OCC33504_MixedLayerObject, AIS_InteractiveObject)
public:
  OCC33504_MixedLayerObject() = default;

protected:

  void Compute (const Handle(PrsMgr_PresentationManager)& ,
                const Handle(Prs3d_Presentation)& thePresentation,
                const Standard_Integer) override
  {
    // Reproduce the client code: call SetZLayer on the object inside Compute().
    SetZLayer (Graphic3d_ZLayerId_Topmost);

    // Group 1 -- inherits Topmost from the object's layer (no explicit per-group override).
    Handle(Graphic3d_ArrayOfTriangles) aTriGreen =
      new Graphic3d_ArrayOfTriangles (3, 3, false, true);
    aTriGreen->AddVertex (gp_Pnt (  0.0, 0.0,   0.0), Quantity_Color (Quantity_NOC_GREEN));
    aTriGreen->AddVertex (gp_Pnt (100.0, 0.0,   0.0), Quantity_Color (Quantity_NOC_GREEN));
    aTriGreen->AddVertex (gp_Pnt (100.0, 0.0, 100.0), Quantity_Color (Quantity_NOC_GREEN));
    aTriGreen->AddEdge (1); aTriGreen->AddEdge (2); aTriGreen->AddEdge (3);
    thePresentation->CurrentGroup()->AddPrimitiveArray (aTriGreen);

    // Group 2 -- explicitly placed in Default layer via SetZLayer().
    Handle(Graphic3d_ArrayOfTriangles) aTriRed =
      new Graphic3d_ArrayOfTriangles (3, 3, false, true);
    aTriRed->AddVertex (gp_Pnt (  0.0, 0.0, 100.0), Quantity_Color (Quantity_NOC_RED));
    aTriRed->AddVertex (gp_Pnt (100.0, 0.0, 100.0), Quantity_Color (Quantity_NOC_RED));
    aTriRed->AddVertex (gp_Pnt (100.0, 0.0, 200.0), Quantity_Color (Quantity_NOC_RED));
    aTriRed->AddEdge (1); aTriRed->AddEdge (2); aTriRed->AddEdge (3);
    Handle(Graphic3d_Group) aGroup2 = thePresentation->NewGroup();
    aGroup2->AddPrimitiveArray (aTriRed);
    aGroup2->SetZLayer (Graphic3d_ZLayerId_Default, Standard_True);
  }

  void ComputeSelection (const Handle(SelectMgr_Selection)& theSel,
                         const Standard_Integer             theMode) override
  {
    // Mode 2 uses ComesFromDecomposition=true, triggering LocalDynamic highlight.
    Handle(SelectMgr_EntityOwner) anOwner = new SelectMgr_EntityOwner (this);
    anOwner->SetComesFromDecomposition (Standard_True);
    // Bounding box covers both triangles in the XZ plane (Y=0 with tolerance).
    Handle(Select3D_SensitiveBox) aBox =
      new Select3D_SensitiveBox (anOwner, 0.0, -1.0, 0.0, 100.0, 1.0, 200.0);
    theSel->Add (aBox);
  }
};

} // anonymous namespace

//==================================================================================================

static Standard_Integer OCC33504 (Draw_Interpretor& theDI,
                                   Standard_Integer  theArgC,
                                   const char**      theArgV)
{
  if (theArgC < 2)
  {
    theDI << "Syntax: " << theArgV[0] << " name\n"
          << "\t: The object calls SetZLayer(Topmost) inside Compute() and creates\n"
          << "\t:   Group 1 -- inherits Topmost (green triangle)\n"
          << "\t:   Group 2 -- explicitly set to Default layer (red triangle)\n"
          << "\t: The dynamic-highlight shadow must render both groups.\n";
    return 1;
  }

  Handle(AIS_InteractiveContext) aCtx = ViewerTest::GetAISContext();
  if (aCtx.IsNull())
  {
    theDI << "Error: no active viewer, call vinit first.\n";
    return 1;
  }

  Handle(OCC33504_MixedLayerObject) anObj = new OCC33504_MixedLayerObject();
  // Display without activating any selection mode (-1), then activate only mode 2
  // (ComesFromDecomposition=true owner) so that vmoveto always triggers LocalDynamic
  // highlight whose shadow is in Topmost and is composited over all geometry.
  aCtx->Display (anObj, 0, -1, false);
  aCtx->Activate (anObj, 2, false);
  GetMapOfAIS().Bind (anObj, theArgV[1]);
  aCtx->UpdateCurrentViewer();
  return 0;
}

//==================================================================================================

void QABugs::Commands_21 (Draw_Interpretor& theCommands)
{
  const char* aGroup = "QABugs";

  theCommands.Add ("OCC33504",
                   "OCC33504 name\n"
                   "\t\t: The object calls SetZLayer(Topmost) in Compute() and has\n"
                   "\t\t: one group inheriting Topmost (green triangle) and one group\n"
                   "\t\t: with explicit Default layer (red triangle).\n"
                   "\t\t: The dynamic-highlight shadow must render both groups.",
                   __FILE__, OCC33504, aGroup);
}
