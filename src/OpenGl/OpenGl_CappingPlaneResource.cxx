// Created on: 2013-08-15
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

#include <NCollection_AlignedAllocator.hxx>
#include <OpenGl_CappingPlaneResource.hxx>
#include <OpenGl_Context.hxx>
#include <OpenGl_Vec.hxx>
#include <OpenGl_ShaderManager.hxx>
#include <Precision.hxx>

IMPLEMENT_STANDARD_RTTIEXT(OpenGl_CappingPlaneResource,OpenGl_Resource)

namespace
{
  //! 12 plane vertices, interleaved:
  //!  - 4 floats, position
  //!  - 4 floats, normal
  //!  - 4 floats, UV texture coordinates
  static const GLfloat THE_CAPPING_PLN_VERTS[12 * (4 + 4 + 4)] =
  {
    0.0f, 0.0f, 0.0f, 1.0f,  0.0f,-1.0f, 0.0f, 0.0f,   0.0f, 0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f, 0.0f,  0.0f,-1.0f, 0.0f, 0.0f,   0.0f,-1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f, 0.0f,  0.0f,-1.0f, 0.0f, 0.0f,   1.0f, 0.0f, 0.0f, 0.0f,

    0.0f, 0.0f, 0.0f, 1.0f,  0.0f,-1.0f, 0.0f, 0.0f,   0.0f, 0.0f, 0.0f, 1.0f,
   -1.0f, 0.0f, 0.0f, 0.0f,  0.0f,-1.0f, 0.0f, 0.0f,  -1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,  0.0f,-1.0f, 0.0f, 0.0f,   0.0f,-1.0f, 0.0f, 0.0f,

    0.0f, 0.0f, 0.0f, 1.0f,  0.0f,-1.0f, 0.0f, 0.0f,   0.0f, 0.0f, 0.0f, 1.0f,
    0.0f, 0.0f,-1.0f, 0.0f,  0.0f,-1.0f, 0.0f, 0.0f,   0.0f, 1.0f, 0.0f, 0.0f,
   -1.0f, 0.0f, 0.0f, 0.0f,  0.0f,-1.0f, 0.0f, 0.0f,  -1.0f, 0.0f, 0.0f, 0.0f,

    0.0f, 0.0f, 0.0f, 1.0f,  0.0f,-1.0f, 0.0f, 0.0f,   0.0f, 0.0f, 0.0f, 1.0f,
    1.0f, 0.0f, 0.0f, 0.0f,  0.0f,-1.0f, 0.0f, 0.0f,   1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f,-1.0f, 0.0f,  0.0f,-1.0f, 0.0f, 0.0f,   0.0f, 1.0f, 0.0f, 0.0f
  };

  static const OpenGl_Matrix OpenGl_IdentityMatrix =
  {
    // mat[4][4]
    { { 1.0f, 0.0f, 0.0f, 0.0f },
      { 0.0f, 1.0f, 0.0f, 0.0f },
      { 0.0f, 0.0f, 1.0f, 0.0f },
      { 0.0f, 0.0f, 0.0f, 1.0f } }
  };

  Handle(Graphic3d_Aspects) defaultMaterial()
  {
    Handle(Graphic3d_AspectFillArea3d) anAspect;
    const Graphic3d_MaterialAspect aMaterial (Graphic3d_NOM_DEFAULT);
    anAspect = new Graphic3d_AspectFillArea3d();
    anAspect->SetDistinguishOff();
    anAspect->SetFrontMaterial (aMaterial);
    anAspect->SetInteriorStyle (Aspect_IS_SOLID);
    anAspect->SetInteriorColor (aMaterial.Color());
    anAspect->SetSuppressBackFaces (false);
    return anAspect;
  }
}


// =======================================================================
// function : BuildInfinitPlaneVertices
// purpose  :
// =======================================================================
OpenGl_PrimitiveArray* OpenGl_CappingPlaneResource::BuildInfinitPlaneVertices()
{
  OpenGl_PrimitiveArray* aPrimitives = NULL;
  // Fill primitive array
  Handle(NCollection_AlignedAllocator) anAlloc = new NCollection_AlignedAllocator (16);
  Handle(Graphic3d_Buffer) anAttribs = new Graphic3d_Buffer (anAlloc);
  Graphic3d_Attribute anAttribInfo[] =
  {
    { Graphic3d_TOA_POS,  Graphic3d_TOD_VEC4 },
    { Graphic3d_TOA_NORM, Graphic3d_TOD_VEC4 },
    { Graphic3d_TOA_UV,   Graphic3d_TOD_VEC4 }
  };
  if (anAttribs->Init (12, anAttribInfo, 3))
  {
    memcpy (anAttribs->ChangeData(), THE_CAPPING_PLN_VERTS, sizeof(THE_CAPPING_PLN_VERTS));

    aPrimitives = new OpenGl_PrimitiveArray (NULL);
    aPrimitives->InitBuffers (NULL, Graphic3d_TOPA_TRIANGLES, NULL, anAttribs, NULL);
  }
  return aPrimitives;
}

// =======================================================================
// function : OpenGl_CappingPlaneResource
// purpose  :
// =======================================================================
OpenGl_CappingPlaneResource::OpenGl_CappingPlaneResource (const Handle(Graphic3d_AspectFillCapping)& theAspect)
: myCappingAspect(),//defaultMaterial()),
  myHatchingAspect(),//defaultMaterial()),
  myHatchingState (0)
{
  myCappingAspect.SetAspect (defaultMaterial());
  myHatchingAspect.SetAspect (defaultMaterial());

  SetAspect (theAspect);
}

// =======================================================================
// function : OpenGl_CappingPlaneResource
// purpose  :
// =======================================================================
OpenGl_CappingPlaneResource::~OpenGl_CappingPlaneResource()
{
}

// =======================================================================
// function : SetAspect
// purpose  :
// =======================================================================
void OpenGl_CappingPlaneResource::SetAspect (const Handle(Graphic3d_AspectFillCapping)& theAspect)
{
  myAspect = theAspect;

  if (theAspect.IsNull())
  {
    return;
  }

  if (!theAspect->ToUseObjectMaterial()
   || !theAspect->ToUseObjectTexture()
   || !theAspect->ToUseObjectShader())
  {
    Handle(Graphic3d_Aspects) aFillAspect = myCappingAspect.Aspect();

    if (!theAspect->ToUseObjectMaterial())
    {
      aFillAspect->SetFrontMaterial (theAspect->Material());
      aFillAspect->SetInteriorColor (theAspect->Material().Color());
    }

    if (!theAspect->ToUseObjectTexture())
    {
      aFillAspect->SetTextureMap (theAspect->Texture());

      if (!theAspect->Texture().IsNull())
      {
        aFillAspect->SetTextureMapOn();
      }
      else
      {
        aFillAspect->SetTextureMapOff();
      }
    }
    else
    {
      aFillAspect->SetTextureMap (Handle(Graphic3d_TextureMap)());
      aFillAspect->SetTextureMapOff();
    }

    if (!theAspect->ToUseObjectShader())
    {
      aFillAspect->SetShaderProgram (theAspect->Shader());
    }

    myCappingAspect.SetAspect (aFillAspect);
  }

  if (theAspect->ToDrawHatch()
    && (theAspect->IsTextureHatch()
     || theAspect->IsStippleHatch()))
  {
    Handle(Graphic3d_Aspects) aFillAspect = myHatchingAspect.Aspect();

    aFillAspect->SetInteriorStyle (theAspect->IsStippleHatch() ? Aspect_IS_HATCH : Aspect_IS_SOLID);
    aFillAspect->SetHatchStyle    (theAspect->IsStippleHatch() ? theAspect->StippleHatch() : Handle(Graphic3d_HatchStyle)());
    aFillAspect->SetTextureMap    (theAspect->IsTextureHatch() ? theAspect->TextureHatch() : Handle(Graphic3d_TextureMap)());
    aFillAspect->SetFrontMaterial (theAspect->HatchMaterial());
    aFillAspect->SetInteriorColor (theAspect->HatchMaterial().Color());
    if (theAspect->IsTextureHatch())
    {
      aFillAspect->SetTextureMapOn();
    }
    else
    {
      aFillAspect->SetTextureMapOff();
    }

    myHatchingAspect.SetAspect (aFillAspect);
    myHatchingState = theAspect->HatchingState();
  }
}

// =======================================================================
// function : Release
// purpose  :
// =======================================================================
void OpenGl_CappingPlaneResource::Release (OpenGl_Context* theContext)
{
  myCappingAspect .Release (theContext);
  myHatchingAspect.Release (theContext);
}

// =======================================================================
// function : CappingFaceAspect
// purpose  :
// =======================================================================
const OpenGl_Aspects* OpenGl_CappingPlaneResource::CappingFaceAspect (const OpenGl_Aspects* theObjectAspect) const
{
  if (myAspect.IsNull())
  {
    return NULL;
  }

  Handle(Graphic3d_Aspects) aFillAspect = myCappingAspect.Aspect();

  if (myAspect->ToUseObjectMaterial() && theObjectAspect != NULL)
  {
    // only front material currently supported by capping rendering
    aFillAspect->SetFrontMaterial (theObjectAspect->Aspect()->FrontMaterial());
    aFillAspect->SetInteriorColor (theObjectAspect->Aspect()->InteriorColor());
  }
  else
  {
    aFillAspect->SetFrontMaterial (myAspect->Material());
    aFillAspect->SetInteriorColor (myAspect->Material().Color());
  }

  if (myAspect->ToUseObjectTexture() && theObjectAspect != NULL)
  {
    if (theObjectAspect->Aspect()->ToMapTexture())
    {
      aFillAspect->SetTextureMap (theObjectAspect->Aspect()->TextureMap());
      aFillAspect->SetTextureMapOn();
    }
    else
    {
      aFillAspect->SetTextureMapOff();
    }
  }
  else
  {
    aFillAspect->SetTextureMap (myAspect->Texture());
    if (!myAspect->Texture().IsNull())
    {
      aFillAspect->SetTextureMapOn();
    }
    else
    {
      aFillAspect->SetTextureMapOff();
    }
  }

  if (myAspect->ToUseObjectShader() && theObjectAspect != NULL)
  {
    aFillAspect->SetShaderProgram (theObjectAspect->Aspect()->ShaderProgram());
  }
  else
  {
    aFillAspect->SetShaderProgram (myAspect->Shader());
  }

  myCappingAspect.SetAspect (aFillAspect);

  return &myCappingAspect;
}

// =======================================================================
// function : HatchingFaceAspect
// purpose  :
// =======================================================================
const OpenGl_Aspects* OpenGl_CappingPlaneResource::HatchingFaceAspect() const
{
  if (myAspect.IsNull())
  {
    return NULL;
  }

  const Standard_Size aHatchingState = myAspect->HatchingState();
  if (myHatchingState != aHatchingState)
  {
    if (myAspect->ToDrawHatch())
    {
      Handle(Graphic3d_Aspects) aFillAspect = myHatchingAspect.Aspect();

      aFillAspect->SetInteriorStyle (myAspect->IsStippleHatch() ? Aspect_IS_HATCH : Aspect_IS_SOLID);
      aFillAspect->SetHatchStyle    (myAspect->IsStippleHatch() ? myAspect->StippleHatch() : Handle(Graphic3d_HatchStyle)());
      aFillAspect->SetTextureMap    (myAspect->IsTextureHatch() ? myAspect->TextureHatch() : Handle(Graphic3d_TextureMap)());
      aFillAspect->SetFrontMaterial (myAspect->HatchMaterial());
      aFillAspect->SetInteriorColor (myAspect->HatchMaterial().Color());
      if (myAspect->IsTextureHatch())
      {
        aFillAspect->SetTextureMapOn();
      }
      else
      {
        aFillAspect->SetTextureMapOff();
      }
      myHatchingAspect.SetAspect (aFillAspect);
      myHatchingState = aHatchingState;
    }
  }

  return &myHatchingAspect;
}
