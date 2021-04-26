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

#ifndef _VInspector_PresentationLight_HeaderFile
#define _VInspector_PresentationLight_HeaderFile

#include <AIS_InteractiveObject.hxx>

#include <Standard_Transient.hxx>

class Graphic3d_ArrayOfTriangles;
class Graphic3d_CLight;

//! Interactive object class to visualize light element
class VInspector_PresentationLight : public AIS_InteractiveObject
{
public:
  //! Constructor
  Standard_EXPORT VInspector_PresentationLight();

  virtual ~VInspector_PresentationLight() {}

  //! Fill presentation by the light element
  Standard_EXPORT void SetLight (const Handle(Graphic3d_CLight)& theLight);

  //! Sets indent for directional light
  Standard_EXPORT void SetIndent (const Standard_Integer theIndent) { myIndent = theIndent; }

  //! Sets translate mode
  void SetTranslateMode (const Standard_Integer theTranslateMode) { myTranslateMode = theTranslateMode; }

public:
  //! Fills presentation.
  //! \param thePrsMgr manager of presentations
  //! \param thePrs presentation to fill
  //! \param theMode mode of display
  Standard_EXPORT virtual void Compute (const Handle(PrsMgr_PresentationManager)& thePrsMgr,
                                        const Handle(Prs3d_Presentation)& thePrs,
                                        const Standard_Integer theMode = 0) Standard_OVERRIDE;

  //! Computes selection sensitive zones (triangulation) for manipulator.
  //! \param theSelection selection item to have own selected entities
  //! \param theMode selection mode
  Standard_EXPORT virtual void ComputeSelection (const Handle(SelectMgr_Selection)& theSelection,
                                                 const Standard_Integer theMode) Standard_OVERRIDE;

public:
  //! Drag object in the viewer.
  //! @param theCtx      [in] interactive context
  //! @param theView     [in] active View
  //! @param theOwner    [in] the owner of detected entity
  //! @param theDragFrom [in] drag start point
  //! @param theDragTo   [in] drag end point
  //! @param theAction   [in] drag action
  //! @return FALSE if object rejects dragging action (e.g. AIS_DragAction_Start)
  Standard_EXPORT virtual Standard_Boolean ProcessDragging (const Handle(AIS_InteractiveContext)& theCtx,
                                                            const Handle(V3d_View)& theView,
                                                            const Handle(SelectMgr_EntityOwner)& theOwner,
                                                            const Graphic3d_Vec2i& theDragFrom,
                                                            const Graphic3d_Vec2i& theDragTo,
                                                            const AIS_DragAction theAction);
  //! @return true if this drag item participates in dragging
  Standard_EXPORT virtual Standard_Boolean HasActiveMode() const;

  //! Init start (reference) transformation.
  //! @warning It is used in chain with StartTransform-Transform(gp_Trsf)-StopTransform
  Standard_EXPORT virtual void StartTransform (const Standard_Integer theX,
                                               const Standard_Integer theY,
                                               const Handle(V3d_View)& theView);

  //! Reset start (reference) transformation.
  //! @param theToApply [in] option to apply or to cancel the started transformation.
  Standard_EXPORT virtual void StopTransform (const Standard_Boolean theToApply = Standard_True);

  //! Apply transformation made from mouse moving from start position
  Standard_EXPORT virtual gp_Trsf Transform (const Standard_Integer theX,
                                             const Standard_Integer theY,
                                             const Handle(V3d_View)& theView);

public:
  DEFINE_STANDARD_RTTIEXT(VInspector_PresentationLight, AIS_InteractiveObject)

private:
  //! Converts pixel coordinate to point. If translation is not started, save this point as current,
  //! else change direction of the light to the point
  Standard_EXPORT Standard_Boolean doTranslate (const Standard_Integer theX,
                                                const Standard_Integer theY,
                                                const Handle(V3d_View)& theView);

  //! Fills presentation with directional light elements
  Standard_EXPORT void drawDirectionalLight (const Handle(Prs3d_Presentation)& thePrs);

  //! Fills selection presentation with directional light elements
  Standard_EXPORT void selectDirectionalLight (const Handle(SelectMgr_Selection)& theSelection);

  //! Fills presentation with positional light elements
  Standard_EXPORT void drawPositionalLight (const Handle(Prs3d_Presentation)& thePrs);

  //! Fills presentation with spot light elements
  Standard_EXPORT void drawSpotLigth (const Handle(Prs3d_Presentation)& thePrs);

  //! Creates array for arrow presentation
  //! \param theDir direction from the origin
  //! \param theIndent a space to span from the origin
  Standard_EXPORT Handle(Graphic3d_ArrayOfTriangles) createArrow (const gp_Dir& theDir,
                                                                  const gp_Pnt& theAttachPoint,
                                                                  const Standard_Integer theIndent);

  //! Draws sphere in attached point
  Standard_EXPORT void drawPosition (const gp_Pnt& theAttachPoint, const Handle(Prs3d_Presentation)& thePrs);

  //! Returns true if light has defined direction
  Standard_Boolean hasDirection (gp_Dir& theDir);

  //! Returns true if light has defined position
  Standard_Boolean hasPosition (gp_Pnt& thePosition);

private:
  Standard_Integer myIndent; //!< indent for directional light
  Handle(Graphic3d_CLight) myLight; //!< light element
  gp_Pnt myStartPick; //!< 3d point corresponding to start mouse pick.

  Standard_Integer myTranslateMode; //!< Translate mode: position or direction change.
  Standard_Boolean myHasStartedTransformation; //!< to move position
};

#endif // _VInspector_PresentationLight_HeaderFile
