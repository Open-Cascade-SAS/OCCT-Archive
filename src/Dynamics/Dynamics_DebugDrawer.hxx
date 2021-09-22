// Copyright (c) 2018-2021 OPEN CASCADE SAS
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

#ifndef _Dynamics_DebugDrawer_HeaderFile
#define _Dynamics_DebugDrawer_HeaderFile

#include <AIS_InteractiveContext.hxx>

#include <Dynamics_Internal.hxx>

//! Define the debugging drawer.
class Dynamics_DebugDrawer : public btIDebugDraw
{
public:

  //! Empty constructor.
  Standard_EXPORT Dynamics_DebugDrawer();

  //! Destructor.
  Standard_EXPORT virtual ~Dynamics_DebugDrawer();

  //! Return TRUE if temporary presentations have been displayed (and not cleared) in Interactive Context.
  bool IsEmpty() const { return myTmpPrsList.IsEmpty(); }

  //! Remove temporary presentations from context.
  Standard_EXPORT void FrameClear (AIS_InteractiveContext* theCtx);

  //! Begin frame rendering.
  Standard_EXPORT void FrameBegin (const Handle(AIS_InteractiveContext)& theCtx);

  //! End frame rendering.
  Standard_EXPORT void FrameEnd   (const Handle(AIS_InteractiveContext)& theCtx);

public:

  //! Draw the line segment.
  Standard_EXPORT virtual void drawLine (const btVector3& theFrom,
                                         const btVector3& theTo,
                                         const btVector3& theColor) Standard_OVERRIDE;

  //! Draw the line segment.
  Standard_EXPORT virtual void drawLine (const btVector3& theFrom,
                                         const btVector3& theTo,
                                         const btVector3& theFromColor,
                                         const btVector3& theToColor) Standard_OVERRIDE;

  //! Draw contact point.
  Standard_EXPORT virtual void drawContactPoint (const btVector3& thePointOnB,
                                                 const btVector3& theNormalOnB,
                                                 btScalar theDistance,
                                                 int theLifeTime,
                                                 const btVector3& theColor) Standard_OVERRIDE;

  //! Print message.
  Standard_EXPORT virtual void reportErrorWarning (const char* theWarningString) Standard_OVERRIDE;

  //! Draw text.
  Standard_EXPORT virtual void draw3dText (const btVector3& theLocation,
                                           const char* theTextString) Standard_OVERRIDE;

  //! Return debug mode.
  virtual int getDebugMode() const Standard_OVERRIDE { return myDebugMode; }

  //! Set debug mode.
  virtual void setDebugMode (int theDebugMode) Standard_OVERRIDE { myDebugMode = theDebugMode; }

private:

  class Dynamics_DebugPrs;

private:

  Handle(Dynamics_DebugPrs) myTmpPrs;
  NCollection_Sequence<Handle(AIS_InteractiveObject)> myTmpPrsList;
  Graphic3d_ZLayerId myZLayer;
  Standard_Integer   myDebugMode;

};

#endif // _Dynamics_DebugDrawer_HeaderFile
