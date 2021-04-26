// Created on: 2021-04-27
// Created by: Natalia ERMOLAEVA
// Copyright (c) 2021 OPEN CASCADE SAS
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

#include <inspector/VInspector_PresentationLight.hxx>
#include <inspector/View_DisplayPreview.hxx>
#include <inspector/View_PreviewParameters.hxx>

#include <Extrema_ExtElC.hxx>
#include <Geom_Line.hxx>
#include <GeomAdaptor_Curve.hxx>
#include <Graphic3d_ArrayOfPoints.hxx>
#include <Graphic3d_ArrayOfTriangles.hxx>
#include <Graphic3d_CLight.hxx>

#include <AIS_InteractiveContext.hxx>
#include <PrsDim.hxx>
#include <Prs3d_Arrow.hxx>
#include <Prs3d_PointAspect.hxx>
#include <Prs3d_Root.hxx>
#include <Prs3d_ShadingAspect.hxx>
#include <Prs3d_ToolSphere.hxx>
#include <SelectMgr_EntityOwner.hxx>
#include <Select3D_SensitiveSegment.hxx>
#include <Select3D_SensitiveTriangulation.hxx>
#include <StdPrs_Curve.hxx>
#include <V3d_View.hxx>

IMPLEMENT_STANDARD_RTTIEXT(VInspector_PresentationLight, AIS_InteractiveObject)

static const Standard_Integer THE_NB_ARROW_FACETTES = 20;

class VInspector_PresentationLightOwner : public SelectMgr_EntityOwner
{
  DEFINE_STANDARD_RTTIEXT(VInspector_PresentationLightOwner, SelectMgr_EntityOwner)

public:
  VInspector_PresentationLightOwner (const Handle(SelectMgr_SelectableObject)& theSelectable,
                                     const Standard_Integer theMode,
                                     const Standard_Integer thePriority = 0)
  : SelectMgr_EntityOwner (theSelectable, thePriority), myMode (theMode) {}

  //! Highlights selectable object's presentation with display mode in presentation manager with given highlight style.
  Standard_EXPORT virtual void HilightWithColor (const Handle(PrsMgr_PresentationManager)& thePrsMgr,
                                                 const Handle(Prs3d_Drawer)& theStyle,
                                                 const Standard_Integer theMode) Standard_OVERRIDE
  {
    Handle(VInspector_PresentationLight) aLightPrs = Handle(VInspector_PresentationLight)::DownCast(Selectable());
    if (!aLightPrs.IsNull())
    {
      aLightPrs->SetTranslateMode (myMode);
    }
    SelectMgr_EntityOwner::HilightWithColor (thePrsMgr, theStyle, theMode);
  }

private:
  Standard_Integer myMode; //!< selection mode
};

IMPLEMENT_STANDARD_RTTIEXT(VInspector_PresentationLightOwner, SelectMgr_EntityOwner)

// =======================================================================
// function : Constructor
// purpose :
// =======================================================================
VInspector_PresentationLight::VInspector_PresentationLight()
: AIS_InteractiveObject(),
  myIndent (200), myTranslateMode (0/*position*/), myHasStartedTransformation (Standard_False)
{
}

// =======================================================================
// function : SetLight
// purpose :
// =======================================================================
void VInspector_PresentationLight::SetLight (const Handle(Graphic3d_CLight)& theLight)
{
  myLight = theLight;
}

// =======================================================================
// function : Compute
// purpose :
// =======================================================================
void VInspector_PresentationLight::Compute (const Handle(PrsMgr_PresentationManager)&,
                                            const Handle(Prs3d_Presentation)& thePrs,
                                            const Standard_Integer)
{
  if (myLight.IsNull())
    return;

  switch (myLight->Type())
  {
    case Graphic3d_TOLS_AMBIENT:     break;
    case Graphic3d_TOLS_DIRECTIONAL: drawDirectionalLight (thePrs); break;
    case Graphic3d_TOLS_POSITIONAL:  drawPositionalLight (thePrs); break;
    case Graphic3d_TOLS_SPOT:        drawSpotLigth (thePrs); break;
    default: break;
  }
}

// =======================================================================
// function : ComputeSelection
// purpose :
// =======================================================================
void VInspector_PresentationLight::ComputeSelection (const Handle(SelectMgr_Selection)& theSelection,
                                                     const Standard_Integer theMode)
{
  if (theMode != View_DisplayPreview::PreviewSelectionMode())
    return;

  if (myLight.IsNull())
    return;

  switch (myLight->Type())
  {
    case Graphic3d_TOLS_DIRECTIONAL:
    case Graphic3d_TOLS_POSITIONAL:
    case Graphic3d_TOLS_SPOT:
    {
      selectDirectionalLight (theSelection);
      break;
    }
    case Graphic3d_TOLS_AMBIENT:
    default: break;
  }
}

//=======================================================================
//function : ProcessDragging
//purpose  :
//=======================================================================
Standard_Boolean VInspector_PresentationLight::ProcessDragging (const Handle(AIS_InteractiveContext)&,
                                                                const Handle(V3d_View)& theView,
                                                                const Handle(SelectMgr_EntityOwner)&,
                                                                const Graphic3d_Vec2i& theDragFrom,
                                                                const Graphic3d_Vec2i& theDragTo,
                                                                const AIS_DragAction theAction)
{
  switch (theAction)
  {
    case AIS_DragAction_Start:
    {
      if (HasActiveMode())
      {
        StartTransform (theDragFrom.x(), theDragFrom.y(), theView);
        return Standard_True;
      }
      break;
    }
    case AIS_DragAction_Update:
    {
      Transform (theDragTo.x(), theDragTo.y(), theView);
      return Standard_True;
    }
    case AIS_DragAction_Abort:
    case AIS_DragAction_Stop:
    {
      StopTransform (false);
      return Standard_True;
    }
    break;
  }
  return Standard_False;
}

// =======================================================================
// function : HasActiveMode
// purpose :
// =======================================================================
Standard_Boolean VInspector_PresentationLight::HasActiveMode() const
{
  return Standard_True;
}

// =======================================================================
// function : StartTransform
// purpose :
// =======================================================================
void VInspector_PresentationLight::StartTransform (const Standard_Integer theX,
                                                   const Standard_Integer theY,
                                                   const Handle(V3d_View)& theView)
{
  if (myHasStartedTransformation)
  {
    return;
  }
  if (doTranslate (theX, theY, theView))
    myHasStartedTransformation = Standard_True;

  if (GetContext())
  {
    GetContext()->Deactivate (this, View_DisplayPreview::PreviewSelectionMode());
  }
}

// =======================================================================
// function : StopTransform
// purpose :
// =======================================================================
void VInspector_PresentationLight::StopTransform (const Standard_Boolean)
{
  myHasStartedTransformation = Standard_False;
  if (GetContext())
  {
    GetContext()->Activate (this, View_DisplayPreview::PreviewSelectionMode());
  }
}

// =======================================================================
// function : Transform
// purpose :
// =======================================================================
gp_Trsf VInspector_PresentationLight::Transform (const Standard_Integer theX,
                                                 const Standard_Integer theY,
                                                 const Handle(V3d_View)& theView)
{
  if (myLight.IsNull() || !myHasStartedTransformation)
    return gp_Trsf();

  doTranslate (theX, theY, theView);
  return gp_Trsf();
}

// =======================================================================
// function : doTranslate
// purpose :
// =======================================================================
Standard_Boolean VInspector_PresentationLight::doTranslate (const Standard_Integer theX,
                                                            const Standard_Integer theY,
                                                            const Handle(V3d_View)& theView)
{
  gp_Pnt aStartPosition (gp::Origin());
  hasPosition(aStartPosition);

  // Get 3d point with projection vector
  Graphic3d_Vec3d anInputPoint, aProj;
  theView->ConvertWithProj (theX, theY, anInputPoint.x(), anInputPoint.y(), anInputPoint.z(), aProj.x(), aProj.y(), aProj.z());

  if (!myHasStartedTransformation)
  {
    gp_Dir aDir;
    gp_Pnt aNewPosition;
    if (!hasDirection(aDir))
    {
      if (!hasPosition(aNewPosition))
      {
        return Standard_False;
      }
      myStartPick = aNewPosition;
    }
    else
    {
      const gp_Lin anInputLine (gp_Pnt (anInputPoint.x(), anInputPoint.y(), anInputPoint.z()), gp_Dir (aProj.x(), aProj.y(), aProj.z()));
      aDir = myLight->Direction();
      const gp_Lin aLine (aStartPosition, aDir);
      Extrema_ExtElC anExtrema (anInputLine, aLine, Precision::Angular());
      if (!anExtrema.IsDone()
          || anExtrema.IsParallel()
          || anExtrema.NbExt() != 1)
        {
          // translation cannot be done co-directed with camera
          return Standard_False;
        }

      Extrema_POnCurv anExPnts[2];
      anExtrema.Points (1, anExPnts[0], anExPnts[1]);
      aNewPosition = anExPnts[1].Value();

      myStartPick = aNewPosition;
    }
    return Standard_True;
  }

  gp_Ax1 aLine1 (gp_Pnt (anInputPoint.x(), anInputPoint.y(), anInputPoint.z()), gp_Dir (aProj.x(), aProj.y(), aProj.z()));
  gp_Pnt aProjectedPoint = PrsDim::ProjectPointOnLine (myStartPick, aLine1);
  if (aProjectedPoint.Distance (myStartPick) < Precision::Confusion())
  {
    return Standard_False;
  }

  if (myTranslateMode == 0)
  {
    myLight->SetPosition (aProjectedPoint);
    myStartPick = aProjectedPoint;
  }
  else
  {
    gp_Dir aNewDir (gp_Vec (aStartPosition, aProjectedPoint));
    myLight->SetDirection (aNewDir);
  }
  // update the current presentation in context
  SetToUpdate();
  if (GetContext())
  {
    GetContext()->Redisplay (this, Standard_True);
  }
  return Standard_True;
}

// =======================================================================
// function : drawDirectionalLight
// purpose :
// =======================================================================
void VInspector_PresentationLight::drawDirectionalLight (const Handle(Prs3d_Presentation)& thePrs)
{
  gp_Pnt anAttachPoint (gp::Origin());

  gp_Dir aDir = myLight->Direction();

  // draw arrow
  Handle(Graphic3d_ArrayOfTriangles) aTriangleArray = createArrow (aDir, anAttachPoint, myIndent);
  Handle(Graphic3d_Group) aGroup = thePrs->CurrentGroup();
  aGroup->SetGroupPrimitivesAspect (Attributes()->ShadingAspect()->Aspect());
  aGroup->AddPrimitiveArray (aTriangleArray);

  // draw line
  Handle(Geom_Line) aGeomLine = new Geom_Line (gp_Lin (anAttachPoint, aDir));
  GeomAdaptor_Curve aCurve (aGeomLine);
  StdPrs_Curve::Add (thePrs, aCurve, myDrawer);

  thePrs->SetInfiniteState (Standard_True);
}

// =======================================================================
// function : selectDirectionalLight
// purpose :
// =======================================================================
void VInspector_PresentationLight::selectDirectionalLight (const Handle(SelectMgr_Selection)& theSelection)
{
  gp_Dir aDir;
  Standard_Boolean aHasDirection = hasDirection (aDir);
  gp_Pnt anAttachPoint;
  Standard_Boolean aHasPosition = hasPosition (anAttachPoint);

  if (!aHasDirection && !aHasPosition)
    return;

  if (aHasDirection)
  {
    Handle(SelectMgr_EntityOwner) anOwner = new VInspector_PresentationLightOwner (this, 1/*translation mode*/, 5);

    gp_Pnt aStartPoint = anAttachPoint.Translated (2 * myIndent * aDir.XYZ());
    gp_Pnt anEndPoint = anAttachPoint.Translated ((-2) * myIndent * aDir.XYZ());
    Handle(Select3D_SensitiveSegment) aSensitive = new Select3D_SensitiveSegment (anOwner, aStartPoint, anEndPoint);

    theSelection->Add (aSensitive);
  }
  if (aHasPosition)
  {
    Handle(SelectMgr_EntityOwner) anOwner = new VInspector_PresentationLightOwner (this, 0/*position*/, 6);

    Prs3d_ToolSphere aTool (10/*Radius*/, 20 /*SlicesNb*/, 20 /*StacksNb*/);
    gp_Trsf aTrsf;
    aTrsf.SetTranslation (gp_Vec(gp::Origin(), anAttachPoint));
    Handle(Poly_Triangulation) aTriangulation = aTool.CreatePolyTriangulation (aTrsf);

    Handle(Select3D_SensitiveTriangulation) aSensitive = new Select3D_SensitiveTriangulation (anOwner, aTriangulation,
      TopLoc_Location(), Standard_True);
    theSelection->Add (aSensitive);
  }
}

// =======================================================================
// function : drawPositionalLight
// purpose :
// =======================================================================
void VInspector_PresentationLight::drawPositionalLight (const Handle(Prs3d_Presentation)& thePrs)
{
  gp_Pnt anAttachPoint = myLight->Position();
  
  // draw position
  drawPosition (anAttachPoint, thePrs);
}

// =======================================================================
// function : drawSpotLigth
// purpose :
// =======================================================================
void VInspector_PresentationLight::drawSpotLigth (const Handle(Prs3d_Presentation)& thePrs)
{
  const gp_Pnt& anAttachPoint = myLight->Position();
  gp_Dir aDir = myLight->Direction();

  // draw position
  drawPosition (anAttachPoint, thePrs);

  // draw arrow
  Handle(Graphic3d_ArrayOfTriangles) aTriangleArray = createArrow (aDir, anAttachPoint, myIndent);
  Handle(Graphic3d_Group) aGroup = thePrs->CurrentGroup();
  aGroup->SetGroupPrimitivesAspect (Attributes()->ShadingAspect()->Aspect());
  aGroup->AddPrimitiveArray (aTriangleArray);

  // draw line
  Handle(Geom_Line) aGeomLine = new Geom_Line (gp_Lin (anAttachPoint, aDir));
  GeomAdaptor_Curve aCurve (aGeomLine);
  StdPrs_Curve::Add (thePrs, aCurve, myDrawer);

  thePrs->SetInfiniteState (Standard_True);
}

// =======================================================================
// function : createArrow
// purpose :
// =======================================================================
Handle(Graphic3d_ArrayOfTriangles) VInspector_PresentationLight::createArrow (const gp_Dir& theDir,
                                                                              const gp_Pnt& theAttachPoint,
                                                                              const Standard_Integer theIndent)
{
  Standard_Real aLength = theIndent * 0.2;
  Standard_Real aTubeRadius = 0;
  Standard_Real aConeRadius = aLength * 0.25;
  Standard_Real aConeLength = aLength;

  gp_Pnt anAttachPoint (theAttachPoint);
  anAttachPoint.Translate (myIndent * theDir.XYZ());
  return Prs3d_Arrow::DrawShaded (gp_Ax1(anAttachPoint, theDir), aTubeRadius, aLength, aConeRadius,
                                  aConeLength, THE_NB_ARROW_FACETTES);
}

// =======================================================================
// function : drawPosition
// purpose :
// =======================================================================
void VInspector_PresentationLight::drawPosition (const gp_Pnt& theAttachPoint,
                                                 const Handle(Prs3d_Presentation)& thePrs)
{
  Prs3d_ToolSphere aTool (10/*Radius*/, 20 /*SlicesNb*/, 20 /*StacksNb*/);
  gp_Trsf aTrsf;
  aTrsf.SetTranslation (gp_Vec(gp::Origin(), theAttachPoint));
  Handle(Graphic3d_ArrayOfTriangles) aSphereArray;
  aTool.FillArray (aSphereArray, aTrsf);

  Handle(Graphic3d_Group) aGroup = thePrs->CurrentGroup();
  aGroup->SetGroupPrimitivesAspect (Attributes()->ShadingAspect()->Aspect());
  aGroup->AddPrimitiveArray (aSphereArray);
}

// =======================================================================
// function : hasDirection
// purpose :
// =======================================================================
Standard_Boolean VInspector_PresentationLight::hasDirection (gp_Dir& theDir)
{
  if (myLight.IsNull())
    return Standard_False;

  if (myLight->Type() != Graphic3d_TOLS_DIRECTIONAL && myLight->Type() != Graphic3d_TOLS_SPOT)
    return Standard_False;

  theDir = myLight->Direction();
  return Standard_True;
}

// =======================================================================
// function : hasPosition
// purpose :
// =======================================================================
Standard_Boolean VInspector_PresentationLight::hasPosition (gp_Pnt& thePosition)
{
  if (myLight.IsNull())
    return Standard_False;

  if (myLight->Type() != Graphic3d_TOLS_POSITIONAL && myLight->Type() != Graphic3d_TOLS_SPOT)
    return Standard_False;

  thePosition = myLight->Position();
  return Standard_True;
}
