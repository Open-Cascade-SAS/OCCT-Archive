// Copyright (c) 2017-2021 OPEN CASCADE SAS
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

#ifdef HAVE_BULLET

#include <Dynamics_DebugDrawer.hxx>

#include <AIS_TextLabel.hxx>
#include <Message.hxx>
#include <Message_Messenger.hxx>
#include <Prs3d_LineAspect.hxx>
#include <Prs3d_PointAspect.hxx>
#include <V3d_Viewer.hxx>
#include <V3d_View.hxx>

namespace
{
  //! Convert vec3 into vec4ub color.
  Graphic3d_Vec4ub colorBt2vec4ub (const btVector3& theColor)
  {
    return Graphic3d_Vec4ub ((unsigned char)(int)(theColor.x() * 255.0f),
                             (unsigned char)(int)(theColor.y() * 255.0f),
                             (unsigned char)(int)(theColor.z() * 255.0f),
                             255);
  }
}

//! Auxiliary presentation of line segments and points.
class Dynamics_DebugDrawer::Dynamics_DebugPrs : public AIS_InteractiveObject
{
  DEFINE_STANDARD_RTTI_INLINE(Dynamics_DebugPrs, AIS_InteractiveObject)
public:
  //! Empty constructor.
  Dynamics_DebugPrs()
  {
    SetMutable (true);
    SetInfiniteState (true);
    mySegments = new ColoredNodeBuffer();
    myPoints   = new ColoredNodeBuffer();
    myDrawer->SetLineAspect (new Prs3d_LineAspect (Quantity_NOC_RED, Aspect_TOL_SOLID, 1.0f));
    myDrawer->SetPointAspect(new Prs3d_PointAspect(Aspect_TOM_PLUS,  Quantity_NOC_RED, 1.0f));
  }

  //! Colored node.
  struct ColoredNode
  {
    Graphic3d_Vec3   Position;
    Graphic3d_Vec4ub Color;
  };

  //! Return vector of colored segments.
  std::vector<ColoredNode>& ChangeSegments() { return mySegments->Nodes; }

  //! Return vector of colored points.
  std::vector<ColoredNode>& ChangePoints() { return myPoints->Nodes; }

  //! Clear previous data.
  void Clear()
  {
    if (!mySegments->Nodes.empty()
     || !myPoints->Nodes.empty())
    {
      mySegments->Nodes.clear();
      myPoints  ->Nodes.clear();
      //SetToUpdate();
    }
  }

  //! Setup view bounding box to be used instead of min/max range of points in this presentation.
  //! This would allow avoiding issues with infinite presentations passed from Bullet (e.g. for btStaticPlaneShape).
  void SetViewBox (const Bnd_Box& theBox) { myViewBox = theBox; }

protected:

  //! Return TRUE for supported display modes.
  virtual Standard_Boolean AcceptDisplayMode (const Standard_Integer theMode) const Standard_OVERRIDE { return theMode == 0; }

  //! Compute presentation.
  virtual void Compute (const Handle(PrsMgr_PresentationManager)& ,
                        const Handle(Prs3d_Presentation)& thePrs,
                        const Standard_Integer theMode) Standard_OVERRIDE
  {
    thePrs->SetInfiniteState (true);
    if (theMode != 0)
    {
      return;
    }

    Graphic3d_Vec3d aMinMax[2];
    bool toCalcMinMax = myViewBox.IsVoid();
    if (!toCalcMinMax)
    {
      aMinMax[0].SetValues (myViewBox.CornerMin().X(), myViewBox.CornerMin().Y(), myViewBox.CornerMin().Z());
      aMinMax[1].SetValues (myViewBox.CornerMax().X(), myViewBox.CornerMax().Y(), myViewBox.CornerMax().Z());
    }

    if (mySegments->Init())
    {
      const Handle(Graphic3d_Group)& aGroup = thePrs->NewGroup();
      aGroup->SetGroupPrimitivesAspect (myDrawer->LineAspect()->Aspect());
      aGroup->AddPrimitiveArray (Graphic3d_TOPA_SEGMENTS, Handle(Graphic3d_IndexBuffer)(), mySegments, Handle(Graphic3d_BoundBuffer)(), toCalcMinMax);
      if (!toCalcMinMax)
      {
        aGroup->SetMinMaxValues (aMinMax[0].x(), aMinMax[0].y(), aMinMax[0].z(),
                                 aMinMax[1].x(), aMinMax[1].y(), aMinMax[1].z());
      }
    }
    if (myPoints->Init())
    {
      const Handle(Graphic3d_Group)& aGroup = thePrs->NewGroup();
      aGroup->SetGroupPrimitivesAspect (myDrawer->PointAspect()->Aspect());
      aGroup->AddPrimitiveArray (Graphic3d_TOPA_POINTS, Handle(Graphic3d_IndexBuffer)(), myPoints, Handle(Graphic3d_BoundBuffer)(), toCalcMinMax);
      if (!toCalcMinMax)
      {
        aGroup->SetMinMaxValues (aMinMax[0].x(), aMinMax[0].y(), aMinMax[0].z(),
                                 aMinMax[1].x(), aMinMax[1].y(), aMinMax[1].z());
      }
    }
  }

  //! Empty selection builder.
  virtual void ComputeSelection (const Handle(SelectMgr_Selection)& theSel, const Standard_Integer theMode) Standard_OVERRIDE
  {
    (void )theSel;
    (void )theMode;
  }

protected:

  //! Buffer holding colored nodes.
  //! This implementation wraps std::vector array, so that NCollection_Buffer wraps external memory and initialized with NULL allocator.
  class ColoredNodeBuffer : public Graphic3d_Buffer
  {
  public:
    //! Data array.
    std::vector<ColoredNode> Nodes;

    //! Empty constructor; passes a NULL allocator.
    ColoredNodeBuffer() : Graphic3d_Buffer (Handle(NCollection_BaseAllocator)()) {}

    //! Wraps buffer from std::vector.
    bool Init()
    {
      release();
      if (Nodes.empty())
      {
        return false;
      }

      // Graphic3d_Buffer defines attributes definition at the end of memory buffer,
      // so that we are extending std::vector capacity to cover this memory block.
      const size_t aReserve = (sizeof(Nodes[0]) + sizeof(Graphic3d_Attribute) - 1) / sizeof(Graphic3d_Attribute);
      Nodes.reserve (Nodes.size() + aReserve);

      static const Graphic3d_Attribute anAttribs[2] =
      {
        { Graphic3d_TOA_POS,   Graphic3d_TOD_VEC3 },
        { Graphic3d_TOA_COLOR, Graphic3d_TOD_VEC4UB }
      };

      NbAttributes = 2;
      Stride       = anAttribs[0].Stride() + anAttribs[1].Stride();
      NbElements   = (int )Nodes.size();
      myData       = (Standard_Byte* )&Nodes[0];
      mySize       = size_t(Stride) * size_t(NbElements);
      ChangeAttribute (0) = anAttribs[0];
      ChangeAttribute (1) = anAttribs[1];
      return true;
    }
  };

private:

  Handle(ColoredNodeBuffer) mySegments;
  Handle(ColoredNodeBuffer) myPoints;
  Bnd_Box myViewBox;

};

// =======================================================================
// function : Dynamics_DebugDrawer
// purpose  :
// =======================================================================
Dynamics_DebugDrawer::Dynamics_DebugDrawer()
: myZLayer (Graphic3d_ZLayerId_Topmost),
  myDebugMode (btIDebugDraw::DBG_DrawWireframe | btIDebugDraw::DBG_DrawContactPoints | btIDebugDraw::DBG_DrawText | btIDebugDraw::DBG_DrawFeaturesText)
{
  //myDebugMode = -1;
}

// =======================================================================
// function : ~Dynamics_DebugDrawer
// purpose  :
// =======================================================================
Dynamics_DebugDrawer::~Dynamics_DebugDrawer()
{
  FrameClear (nullptr);
}

// =======================================================================
// function : FrameClear
// purpose  :
// =======================================================================
void Dynamics_DebugDrawer::FrameClear (AIS_InteractiveContext* theCtx)
{
  for (NCollection_Sequence<Handle(AIS_InteractiveObject)>::Iterator aPrsIter (myTmpPrsList); aPrsIter.More(); aPrsIter.Next())
  {
    if (aPrsIter.Value()->HasInteractiveContext())
    {
      if (theCtx == nullptr)
      {
        throw Standard_ProgramError ("Internal Error: Dynamics_DebugDrawer::FrameClear() is called with NULL context");
      }
      theCtx->Remove (aPrsIter.Value(), false);
    }
  }
  myTmpPrsList.Clear();
  if (!myTmpPrs.IsNull())
  {
    myTmpPrs->Clear();
  }
}

// =======================================================================
// function : FrameBegin
// purpose  :
// =======================================================================
void Dynamics_DebugDrawer::FrameBegin (const Handle(AIS_InteractiveContext)& theCtx)
{
  FrameClear (theCtx.get());
  if (myTmpPrs.IsNull())
  {
    myTmpPrs = new Dynamics_DebugPrs();
    myTmpPrs->SetZLayer (myZLayer);
  }
  {
    Handle(V3d_View) aView = theCtx->CurrentViewer()->ActiveViewIterator().Value();
    const Bnd_Box aViewBox = aView->View()->MinMaxValues();
    myTmpPrs->SetViewBox (aViewBox);
  }
  myTmpPrsList.Append (myTmpPrs);
}

// =======================================================================
// function : FrameEnd
// purpose  :
// =======================================================================
void Dynamics_DebugDrawer::FrameEnd (const Handle(AIS_InteractiveContext)& theCtx)
{
  theCtx->RecomputePrsOnly (myTmpPrs, false);
  for (NCollection_Sequence<Handle(AIS_InteractiveObject)>::Iterator aPrsIter (myTmpPrsList); aPrsIter.More(); aPrsIter.Next())
  {
    theCtx->Display (aPrsIter.Value(), 0, -1, false);
  }
}

// =======================================================================
// function : drawLine
// purpose  :
// =======================================================================
void Dynamics_DebugDrawer::drawLine (const btVector3& theFrom,
                                        const btVector3& theTo,
                                        const btVector3& theColor)
{
  Dynamics_DebugPrs::ColoredNode aNode;
  aNode.Color = colorBt2vec4ub (theColor);
  aNode.Position.SetValues (theFrom.x(), theFrom.y(), theFrom.z());
  myTmpPrs->ChangeSegments().push_back (aNode);
  aNode.Position.SetValues (theTo.x(), theTo.y(), theTo.z());
  myTmpPrs->ChangeSegments().push_back (aNode);
}

// =======================================================================
// function : drawLine
// purpose  :
// =======================================================================
void Dynamics_DebugDrawer::drawLine (const btVector3& theFrom,
                                        const btVector3& theTo,
                                        const btVector3& theFromColor,
                                        const btVector3& theToColor)
{
  Dynamics_DebugPrs::ColoredNode aNode;
  aNode.Color = colorBt2vec4ub (theFromColor);
  aNode.Position.SetValues (theFrom.x(), theFrom.y(), theFrom.z());
  myTmpPrs->ChangeSegments().push_back (aNode);
  aNode.Color = colorBt2vec4ub (theToColor);
  aNode.Position.SetValues (theTo.x(), theTo.y(), theTo.z());
  myTmpPrs->ChangeSegments().push_back (aNode);
}

// =======================================================================
// function : drawContactPoint
// purpose  :
// =======================================================================
void Dynamics_DebugDrawer::drawContactPoint (const btVector3& thePointOnB,
                                                const btVector3& theNormalOnB,
                                                btScalar theDistance,
                                                int theLifeTime,
                                                const btVector3& theColor)
{
  Dynamics_DebugPrs::ColoredNode aNode;
  aNode.Color = colorBt2vec4ub (theColor);
  aNode.Position.SetValues (thePointOnB.x(), thePointOnB.y(), thePointOnB.z());
  myTmpPrs->ChangePoints().push_back (aNode);
  (void )theNormalOnB;
  (void )theDistance;
  (void )theLifeTime;
}

// =======================================================================
// function : reportErrorWarning
// purpose  :
// =======================================================================
void Dynamics_DebugDrawer::reportErrorWarning (const char* theWarningString)
{
  Message::SendWarning (theWarningString);
}

// =======================================================================
// function : draw3dText
// purpose  :
// =======================================================================
void Dynamics_DebugDrawer::draw3dText (const btVector3& theLocation,
                                          const char* theTextString)
{
  Handle(AIS_TextLabel) aLabel = new AIS_TextLabel();
  aLabel->SetMutable(true);
  aLabel->SetZLayer (myZLayer);
  aLabel->SetInfiniteState (true);
  aLabel->SetText (theTextString);
  aLabel->SetPosition (gp_Pnt (theLocation.x(), theLocation.y(), theLocation.z()));
  myTmpPrsList.Append (aLabel);
}

#endif
