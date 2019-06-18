// Created on: 2017-04-14
// Created by: Anton POLETAEV
// Copyright (c) 2017 OPEN CASCADE SAS
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

#include <Graphic3d_AspectFillCapping.hxx>

IMPLEMENT_STANDARD_RTTIEXT(Graphic3d_AspectFillCapping, Standard_Transient)

// =======================================================================
// function : Graphic3d_AspectFillCapping
// purpose  :
// =======================================================================
Graphic3d_AspectFillCapping::Graphic3d_AspectFillCapping()
: myFlags (Flags_None),
  myHatchingState (0)
{
  Graphic3d_MaterialAspect aMaterial;
  aMaterial.SetColor             (Quantity_NOC_BLACK);
  aMaterial.SetReflectionModeOff (Graphic3d_TOR_AMBIENT);
  aMaterial.SetReflectionModeOff (Graphic3d_TOR_DIFFUSE);
  aMaterial.SetReflectionModeOff (Graphic3d_TOR_SPECULAR);
  aMaterial.SetReflectionModeOff (Graphic3d_TOR_EMISSION);
  aMaterial.SetMaterialType      (Graphic3d_MATERIAL_ASPECT);
  SetHatchStyle    (Aspect_HS_HORIZONTAL);
  SetHatchMaterial (aMaterial);
}

// =======================================================================
// function : SetHatchStyle
// purpose  :
// =======================================================================
void Graphic3d_AspectFillCapping::SetHatchStyle (const Aspect_HatchStyle theStyle)
{
  myStippleHatch = new Graphic3d_HatchStyle (theStyle);
  myTextureHatch.Nullify();
  myHatchingState++;
}

// =======================================================================
// function : SetHatchStyle
// purpose  :
// =======================================================================
void Graphic3d_AspectFillCapping::SetHatchStyle (const Handle(Graphic3d_HatchStyle)& theStyle)
{
  myStippleHatch = theStyle;
  myTextureHatch.Nullify();
  myHatchingState++;
}

// =======================================================================
// function : SetHatchStyle
// purpose  :
// =======================================================================
void Graphic3d_AspectFillCapping::SetHatchStyle (const Handle(Graphic3d_TextureMap)& theTexture)
{
  myStippleHatch.Nullify();
  myTextureHatch = theTexture;
  myHatchingState++;
}

// =======================================================================
// function : SetHatchMaterial
// purpose  :
// =======================================================================
void Graphic3d_AspectFillCapping::SetHatchMaterial (const Graphic3d_MaterialAspect& theMaterial)
{
  myHatchMaterial = theMaterial;
  myHatchingState++;
}

// =======================================================================
// function : SetToDrawHatch
// purpose  :
// =======================================================================
void Graphic3d_AspectFillCapping::SetToDrawHatch (const Standard_Boolean theToDraw)
{
  setFlag (theToDraw, Flags_DrawHatching);
  myHatchingState++;
}

// =======================================================================
// function : SetHatchZoomPeristent
// purpose  :
// =======================================================================
void Graphic3d_AspectFillCapping::SetHatchZoomPeristent (const Standard_Boolean theToSet) 
{
  setFlag (theToSet, Flags_HatchZoomPersistent);
  myHatchingState++;
}

// =======================================================================
// function : SetHatchRotationPeristent
// purpose  :
// =======================================================================
void Graphic3d_AspectFillCapping::SetHatchRotationPeristent (const Standard_Boolean theToSet)
{
  setFlag (theToSet, Flags_HatchRotationPersistent);
  myHatchingState++;
}
