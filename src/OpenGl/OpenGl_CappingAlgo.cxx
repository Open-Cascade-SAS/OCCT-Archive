// Created on: 2013-09-05
// Created by: Anton POLETAEV
// Copyright (c) 2013-2014 OPEN CASCADE SAS
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

#include <OpenGl_CappingAlgo.hxx>

#include <OpenGl_ClippingIterator.hxx>
#include <OpenGl_Workspace.hxx>
#include <OpenGl_Context.hxx>
#include <OpenGl_PrimitiveArray.hxx>
#include <OpenGl_CappingPlaneResource.hxx>
#include <OpenGl_Vec.hxx>
#include <OpenGl_View.hxx>
#include <OpenGl_Structure.hxx>
#include <OpenGl_ShaderManager.hxx>

namespace
{
  static const OpenGl_CappingPlaneResource* THE_DEFAULT_ASPECT = new OpenGl_CappingPlaneResource (new Graphic3d_AspectFillCapping);
  static const TCollection_AsciiString THE_QUAD_PARRAY = "OpenGl_CappingAlgo_Quad";
  static const TCollection_AsciiString THE_PLANE_STYLE = "OpenGl_CappingAlgo_CappingStyle_";

  //! Auxiliary sentry object managing stencil test.
  struct StencilTestSentry
  {
    StencilTestSentry() : myDepthFuncPrev (0) {}

    //! Restore previous application state.
    ~StencilTestSentry()
    {
      if (myDepthFuncPrev != 0)
      {
        glClear (GL_STENCIL_BUFFER_BIT);
        glDepthFunc (myDepthFuncPrev);
        glStencilFunc (GL_ALWAYS, 0, 0xFF);
        glDisable (GL_STENCIL_TEST);
      }
    }

    //! Prepare for rendering the clip planes.
    void Init()
    {
      if (myDepthFuncPrev == 0)
      {
        glEnable (GL_STENCIL_TEST);
        glGetIntegerv (GL_DEPTH_FUNC, &myDepthFuncPrev);
        glDepthFunc (GL_LESS);
      }
    }

  private:
    GLint myDepthFuncPrev;
  };

  class OpenGl_SharedElement : public OpenGl_Resource
  {
    public:
      OpenGl_SharedElement (OpenGl_Element* theGlElement) : myGlElement (theGlElement) {}
      virtual void Release (OpenGl_Context* theGlCtx) Standard_OVERRIDE
      {
        OpenGl_Element::Destroy (theGlCtx, myGlElement);
      }
      OpenGl_Element* GlElement() const { return myGlElement; }

      //! Returns estimated GPU memory usage for holding data without considering overheads and allocation alignment rules.
      Standard_Size EstimatedDataSize() const Standard_OVERRIDE { return 0; }

    private:
      OpenGl_Element* myGlElement;

    public:

      DEFINE_STANDARD_RTTI_INLINE (OpenGl_SharedElement, OpenGl_Resource)
  };

  //! Iitializes and returns vertex buffer for plane section
  OpenGl_PrimitiveArray* initQuad (const Handle(OpenGl_Context)& theContext)
  {
    Handle(OpenGl_SharedElement) aSharedResource;

    if (!theContext->GetResource (THE_QUAD_PARRAY, aSharedResource))
    {
      aSharedResource = new OpenGl_SharedElement (OpenGl_CappingPlaneResource::BuildInfinitPlaneVertices());
      theContext->ShareResource (THE_QUAD_PARRAY, aSharedResource);
    }

    return dynamic_cast<OpenGl_PrimitiveArray*> (aSharedResource->GlElement());
  }

  //! Render section plane using the given aspects.
  void renderSection (const Handle(OpenGl_Workspace)& theWorkspace,
                      const OpenGl_PrimitiveArray*    theQuad,
                      const OpenGl_Aspects*           theCappingAspect,
                      const OpenGl_Aspects*           theHatchAspect,
                      const OpenGl_Mat4&              theCappingMatrix,
                      const Standard_ShortReal        theHatchScale,
                      const Standard_ShortReal        theHatchRotate)
  {
    const Handle(OpenGl_Context)& aContext = theWorkspace->GetGlContext();
    const bool wasCullAllowed = theWorkspace->SetAllowFaceCulling (true);

    const Standard_Boolean isTextureHatch =
        theHatchAspect != NULL
    &&  theHatchAspect->Aspect()->TextureMapState();

    aContext->ModelWorldState.Push();
    aContext->ModelWorldState.SetCurrent (theCappingMatrix);
    aContext->ApplyModelViewMatrix();

    theWorkspace->SetAspects (theCappingAspect);
    theWorkspace->ApplyAspects();

    theQuad->Render (theWorkspace);

    if (theHatchAspect != NULL)
    {
      Graphic3d_Vec2     aPrevScale;
      Standard_ShortReal aPrevRotate = 0.0;

      if (isTextureHatch)
      {
        glEnable (GL_BLEND);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        if ((theHatchScale != 1.0 || theHatchRotate != 0.0) && !theHatchAspect->TextureSet(aContext)->IsEmpty())
        {
          Handle(OpenGl_Texture) aTexture = theHatchAspect->TextureSet(aContext)->First();
          const Handle(Graphic3d_TextureParams)& aTexParams = aTexture->Sampler()->Parameters();

          aPrevScale  = aTexParams->Scale();
          aPrevRotate = aTexParams->Rotation();

          const Standard_Boolean   isMirror = aPrevScale.x() * aPrevScale.y() < 0.0;
          aTexParams->SetScale    (aPrevScale * theHatchScale);
          aTexParams->SetRotation (isMirror ? aPrevRotate - theHatchRotate : aPrevRotate + theHatchRotate);
        }
      }

      theWorkspace->SetAspects (theHatchAspect);
      theWorkspace->ApplyAspects();

      glDepthFunc (GL_LEQUAL);

      theQuad->Render (theWorkspace);

      glDepthFunc (GL_LESS);

      if (isTextureHatch)
      {
        glDisable (GL_BLEND);

        if (theHatchScale != 1.0 || theHatchRotate != 0.0)
        {
          Handle(OpenGl_Texture) aTexture = theHatchAspect->TextureSet(aContext)->First();
          const Handle(Graphic3d_TextureParams)& aTexParams = aTexture->Sampler()->Parameters();

          aTexParams->SetScale (aPrevScale);
          aTexParams->SetRotation (aPrevRotate);
        }
      }
    }

    aContext->ModelWorldState.Pop();
    aContext->ApplyModelViewMatrix();

    theWorkspace->SetAllowFaceCulling (wasCullAllowed);
  }

  //! Render capping for specific structure.
  static void renderCappingForStructure (StencilTestSentry& theStencilSentry,
                                         const Handle(OpenGl_Workspace)& theWorkspace,
                                         const OpenGl_Structure&         theStructure,
                                         const Handle(Graphic3d_ClipPlane)& theClipChain,
                                         const Standard_Integer          theSubPlaneIndex,
                                         const Handle(OpenGl_CappingPlaneResource)& thePlane,
                                         const OpenGl_PrimitiveArray*    theQuad)
  {
    const Standard_Integer aPrevFilter = theWorkspace->RenderFilter();
    const Standard_Integer anAnyFilter = aPrevFilter & ~(Standard_Integer )(OpenGl_RenderFilter_OpaqueOnly | OpenGl_RenderFilter_TransparentOnly);

    const Handle(Graphic3d_ClipPlane)& aPlane = theClipChain;

    const Handle(OpenGl_Context)&      aContext     = theWorkspace->GetGlContext();
    const Handle(Graphic3d_Camera) aCamera    = theWorkspace->View() != NULL
                                              ? theWorkspace->View()->Camera()
                                              : Handle(Graphic3d_Camera)();
    const OpenGl_Mat4& aPlaneMat    = OpenGl_Mat4::Map (aPlane->OrientationMatrix());
    Standard_ShortReal aRotateAngle = 0.0;
    Standard_ShortReal aViewScale   = ShortRealLast();
    OpenGl_Mat4        aRotateZoomMat;

    for (OpenGl_Structure::GroupIterator aGroupIter (theStructure.Groups()); aGroupIter.More(); aGroupIter.Next())
    {
      if (!aGroupIter.Value()->IsClosed())
      {
        continue;
      }

      // clear stencil only if something has been actually drawn
      theStencilSentry.Init();

      // suppress only opaque/transparent filter since for filling stencil the whole geometry should be drawn
      theWorkspace->SetRenderFilter (anAnyFilter);

      // enable only the rendering plane to generate stencil mask
      aContext->ChangeClipping().DisableAllExcept (theClipChain, theSubPlaneIndex);
      aContext->ShaderManager()->UpdateClippingState();

      glClear (GL_STENCIL_BUFFER_BIT);
      glColorMask (GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

      // override aspects, disable culling
      theWorkspace->SetAspects (&theWorkspace->NoneCulling());
      theWorkspace->ApplyAspects();

      // evaluate number of pair faces
      if (theWorkspace->UseZBuffer())
      {
        glDisable (GL_DEPTH_TEST);
      }
      if (theWorkspace->UseDepthWrite())
      {
        glDepthMask (GL_FALSE);
      }
      glStencilFunc (GL_ALWAYS, 1, 0x01);
      glStencilOp (GL_KEEP, GL_INVERT, GL_INVERT);

      // render closed primitives
      aGroupIter.Value()->Render (theWorkspace);

      // override material, cull back faces
      theWorkspace->SetAspects (&theWorkspace->FrontCulling());
      theWorkspace->ApplyAspects();

      // enable all clip plane except the rendered one
      aContext->ChangeClipping().EnableAllExcept (theClipChain, theSubPlaneIndex);
      aContext->ShaderManager()->UpdateClippingState();

      // render capping plane using the generated stencil mask
      glColorMask (GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
      if (theWorkspace->UseDepthWrite())
      {
        glDepthMask (GL_TRUE);
      }
      glStencilFunc (GL_EQUAL, 1, 0x01);
      glStencilOp (GL_KEEP, GL_KEEP, GL_KEEP);
      if (theWorkspace->UseZBuffer())
      {
        glEnable (GL_DEPTH_TEST);
      }

      const OpenGl_Aspects*              aGroupAspectFace    = aGroupIter.Value()->GlAspects();
      const OpenGl_CappingPlaneResource* aGroupAspectCapping = aGroupIter.Value()->AspectFillCapping();
      const OpenGl_CappingPlaneResource* anAspectCapping =
          thePlane && (!aGroupAspectCapping || aGroupAspectCapping->Aspect().IsNull() || aPlane->ToOverrideCappingAspect())
        ? thePlane.get()
        : aGroupAspectCapping;

      if (anAspectCapping == NULL)
      {
        anAspectCapping = THE_DEFAULT_ASPECT;
      }

      const OpenGl_Aspects*  anAspectFace     = anAspectCapping->CappingFaceAspect (aGroupAspectFace);
      const Standard_Boolean hasHatch         = anAspectCapping->Aspect()->ToDrawHatch();
      const OpenGl_Aspects*  anAspectHatching = hasHatch ? anAspectCapping->HatchingFaceAspect() : NULL;
      const Standard_Boolean hasTextureHatch  = hasHatch && !anAspectCapping->Aspect()->TextureHatch().IsNull();
      const Standard_Boolean isRotatePers     = hasTextureHatch && !aCamera.IsNull() && anAspectCapping->Aspect()->IsHatchRotationPersistent();
      const Standard_Boolean isZoomPers       = hasTextureHatch && !aCamera.IsNull() && anAspectCapping->Aspect()->IsHatchZoomPersistent();

      Standard_ShortReal aHatchScale = 1.0;
      Standard_ShortReal aHatchAngle = 0.0;

      if (isRotatePers || isZoomPers)
      {

        if (isRotatePers)
        {
          if (aRotateAngle == 0.0)
          {
            const gp_Dir aPlaneSide (aPlaneMat.GetValue (0, 0), aPlaneMat.GetValue (1, 0), aPlaneMat.GetValue (2, 0));
            const gp_Dir aPlaneUp   (aPlaneMat.GetValue (0, 2), aPlaneMat.GetValue (1, 2), aPlaneMat.GetValue (2, 2));
            const gp_Dir& aCameraUp  = aCamera->Up();
            const gp_Vec  aCameraPln = aPlaneSide.Dot (aCameraUp) * aPlaneSide + aPlaneUp.Dot (aCameraUp) * aPlaneUp;
            if (aCameraPln.Magnitude() > Precision::Confusion())
            {
              const gp_Dir& aCameraDir   = aCamera->Direction();
              aRotateAngle = static_cast<Standard_ShortReal> (aCameraPln.AngleWithRef (aPlaneUp, aCameraDir) / M_PI * 180.0);
            }
          }

          aHatchAngle = aRotateAngle;
        }

        if (isZoomPers)
        {
          if (aViewScale == ShortRealLast())
          {
            const Standard_Real aFocus = aCamera->IsOrthographic()
                                        ? aCamera->Distance()
                                        : (aCamera->ZFocusType() == Graphic3d_Camera::FocusType_Relative
                                        ? Standard_Real(aCamera->ZFocus() * aCamera->Distance())
                                        : Standard_Real(aCamera->ZFocus()));

            const gp_XYZ aViewDim = aCamera->ViewDimensions (aFocus);
            aViewScale = static_cast<Standard_ShortReal> (aViewDim.Y() / aContext->Viewport()[3]);
          }

          if (!anAspectHatching->TextureSet(aContext)->IsEmpty())
            aHatchScale = 1.0f / (aViewScale * anAspectHatching->TextureSet(aContext)->First()->SizeY());
        }
      }
    
      renderSection (theWorkspace, theQuad, anAspectFace, hasHatch ? anAspectCapping->HatchingFaceAspect() : NULL, aPlaneMat, aHatchScale, aHatchAngle);

      // turn on the current plane to restore initial state
      aContext->ChangeClipping().ResetCappingFilter();
      aContext->ShaderManager()->RevertClippingState();
      aContext->ShaderManager()->RevertClippingState();
    }

    if (theStructure.InstancedStructure() != NULL)
    {
      renderCappingForStructure (theStencilSentry, theWorkspace, *theStructure.InstancedStructure(), theClipChain, theSubPlaneIndex, thePlane, theQuad);
    }
  }
}

// =======================================================================
// function : RenderCapping
// purpose  :
// =======================================================================
void OpenGl_CappingAlgo::RenderCapping (const Handle(OpenGl_Workspace)& theWorkspace,
                                        const OpenGl_Structure&         theStructure)
{
  const Handle(OpenGl_Context)& aContext = theWorkspace->GetGlContext();
  if (!aContext->Clipping().IsCappingOn())
  {
    // do not perform algorithm if there is nothing to render
    return;
  }

  const OpenGl_PrimitiveArray* aCappingQuad = initQuad (aContext);
  if (!aCappingQuad)
  {
    return;
  }

  // remember current aspect face defined in workspace
  const OpenGl_Aspects* aFaceAsp = theWorkspace->Aspects();

  // only filled primitives should be rendered
  const Standard_Integer aPrevFilter = theWorkspace->RenderFilter();
  theWorkspace->SetRenderFilter (aPrevFilter | OpenGl_RenderFilter_FillModeOnly);
  StencilTestSentry aStencilSentry;

  GLboolean aPrevBlend = glIsEnabled (GL_BLEND);
  GLint     aPrevBlendSrc = GL_ONE;
  GLint     aPrevBlendDst = GL_ZERO;
  if (aPrevBlend == GL_TRUE)
  {
    glGetIntegerv (GL_BLEND_SRC_ALPHA, &aPrevBlendSrc);
    glGetIntegerv (GL_BLEND_DST_ALPHA, &aPrevBlendDst);
    glDisable (GL_BLEND);
  }

  // generate capping for every clip plane
  for (OpenGl_ClippingIterator aCappingIt (aContext->Clipping()); aCappingIt.More(); aCappingIt.Next())
  {
    // get plane being rendered
    const Handle(Graphic3d_ClipPlane)& aClipChain = aCappingIt.Value();
    if (!aClipChain->IsCapping()
      || aCappingIt.IsDisabled())
    {
      continue;
    }

    Standard_Integer aSubPlaneIndex = 1;
    for (const Graphic3d_ClipPlane* aSubPlaneIter = aClipChain.get(); aSubPlaneIter != NULL; aSubPlaneIter = aSubPlaneIter->ChainNextPlane().get(), ++aSubPlaneIndex)
    {
      // get resource for the plane
      const TCollection_AsciiString& aResId = THE_PLANE_STYLE + aSubPlaneIter->GetId();
      Handle(OpenGl_CappingPlaneResource) aPlaneRes;
      if (!aContext->GetResource (aResId, aPlaneRes))
      {
        // share and register for release once the resource is no longer used
        aPlaneRes = new OpenGl_CappingPlaneResource (aSubPlaneIter->CappingSectionStyle());
        aContext->ShareResource (aResId, aPlaneRes);
      }

      renderCappingForStructure (aStencilSentry, theWorkspace, theStructure, aClipChain, aSubPlaneIndex, aPlaneRes, aCappingQuad);

      // set delayed resource release
      aPlaneRes.Nullify();
      if (!aResId.IsEmpty())
      {
        // schedule release of resource if not used
        aContext->ReleaseResource (aResId, Standard_True);
      } 
    }
  }

  if (aPrevBlend == GL_TRUE)
  {
    glEnable (GL_BLEND);
    glBlendFunc (aPrevBlendSrc, aPrevBlendDst);
  }

  // restore rendering aspects
  theWorkspace->SetAspects (aFaceAsp);
  theWorkspace->SetRenderFilter (aPrevFilter);
}
