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

#ifndef _Graphic3d_AspectFillCapping_HeaderFile
#define _Graphic3d_AspectFillCapping_HeaderFile

#include <Aspect_HatchStyle.hxx>
#include <Graphic3d_Aspects.hxx>
#include <Graphic3d_HatchStyle.hxx>
#include <Graphic3d_MaterialAspect.hxx>
#include <Graphic3d_ShaderProgram.hxx>
#include <Graphic3d_TextureMap.hxx>
#include <Standard_Transient.hxx>

//! Defines graphical attributes for drawing section planes on solids resulted from clipping (cutting) planes.
class Graphic3d_AspectFillCapping : public Graphic3d_Aspects
{
public:

  //! Default constructor.
  Standard_EXPORT Graphic3d_AspectFillCapping();

public:

  //! Sets material for filling section created by clipping.
  void SetMaterial (const Graphic3d_MaterialAspect& theMaterial) { myMaterial = theMaterial; }

  //! Returns material for filling section created by clipping.
  const Graphic3d_MaterialAspect& Material() const { return myMaterial; }

  //! Sets flag indicating whether object's material (instead of defined by this aspect) should be used for filling section.
  void SetUseObjectMaterial (const Standard_Boolean theToUse) { setFlag (theToUse, Flags_UseObjectMaterial); }

  //! Returns flag indicating whether object's material (instead of defined by this aspect) should be used for filling section.
  Standard_Boolean ToUseObjectMaterial() const { return (myFlags & Flags_UseObjectMaterial) != 0; }

  //! Sets texture for filling section created by clipping.
  void SetTexture (const Handle(Graphic3d_TextureMap)& theTexture) { myTexture = theTexture; }

  //! Returns texture for filling section created by clipping.
  const Handle(Graphic3d_TextureMap)& Texture() const { return myTexture; }

  //! Sets flag indicating whether object's texture (instead of defined by this aspect) should be used for filling section.
  void SetUseObjectTexture (const Standard_Boolean theToUse) { setFlag (theToUse, Flags_UseObjectTexture); }

  //! Returns flag indicating whether object's texture (instead of defined by this aspect) should be used for filling section.
  Standard_Boolean ToUseObjectTexture() const { return (myFlags & Flags_UseObjectTexture) != 0; }

  //! Sets OpenGL/GLSL shader program.
  void SetShader (const Handle(Graphic3d_ShaderProgram)& theShader) { myShader = theShader; }

  //! Returns OpenGL/GLSL shader program.
  const Handle(Graphic3d_ShaderProgram)& Shader() const { return myShader; }

  //! Sets flag indicating whether object's shader (instead of defined by this aspect) should be used for filling section.
  void SetUseObjectShader (const Standard_Boolean theToUse) { setFlag (theToUse, Flags_UseObjectShader); }

  //! Returns flag indicating whether object's shader (instead of defined by this aspect) should be used for filling section.
  Standard_Boolean ToUseObjectShader() const { return (myFlags & Flags_UseObjectShader) != 0; }

public:

  //! Sets style of hatch defined by predefined stipple mask.
  Standard_EXPORT void SetHatchStyle (const Aspect_HatchStyle theStyle);

  //! Sets style of hatch defined by custom stipple mask.
  Standard_EXPORT void SetHatchStyle (const  Handle(Graphic3d_HatchStyle)& theStyle);

  //! Sets style of hatch defined by texture map (decal texture with alpha channel should be used).
  Standard_EXPORT void SetHatchStyle (const Handle(Graphic3d_TextureMap)& theTexture);

  //! Sets material style for hatch lines (texture).
  Standard_EXPORT void SetHatchMaterial (const Graphic3d_MaterialAspect& theMaterial);

  //! Returns material style for hatch lines (texture).
  const Graphic3d_MaterialAspect& HatchMaterial() const { return myHatchMaterial; }

  //! Sets boolean flag indicating whether the hatch layer should be drawn or not.
  Standard_EXPORT void SetToDrawHatch (const Standard_Boolean theToDraw);

  //! Returns boolean flag indicating whether the hatch layer should be drawn or not.
  Standard_Boolean ToDrawHatch() const { return (myFlags & Flags_DrawHatching) != 0; }

  //! Sets flag controlling behavior of hatch texture mapping on zooming.
  //! @param theToSet [in] if passed TRUE the texture will keep constant screen-scale independent of zooming.
  Standard_EXPORT void SetHatchZoomPeristent (const Standard_Boolean theToSet);

  //! Returns value of flag controlling behavior of hatch texture mapping on zooming.
  Standard_Boolean IsHatchZoomPersistent() { return (myFlags & Flags_HatchZoomPersistent) != 0; }

  //! Sets flag controlling behavior of hatch texture mapping on camera rotation around heading vector.
  Standard_EXPORT void SetHatchRotationPeristent (const Standard_Boolean theToSet);

  //! Returns value of flag controlling behavior of hatch texture mapping on camera rotation around heading vector.
  Standard_Boolean IsHatchRotationPersistent() { return (myFlags & Flags_HatchRotationPersistent) != 0; }

  //! Returns true if hatch is defined by texture.
  Standard_Boolean IsTextureHatch() const { return !myTextureHatch.IsNull(); }

  //! Returns texture map defining the hatch.
  const Handle(Graphic3d_TextureMap)& TextureHatch() const { return myTextureHatch; }

  //! Returns true if hatch is defined by stipple mask.
  Standard_Boolean IsStippleHatch() const { return !myStippleHatch.IsNull(); }

  //! Returns the stipple mask.
  const Handle(Graphic3d_HatchStyle)& StippleHatch() const { return myStippleHatch; }

  //! Returns modification counter for hatching state.
  Standard_Size HatchingState() const { return myHatchingState; }

private:

  enum Flags
  {
    Flags_None                    = 0x00, //!< no flags
    Flags_UseObjectMaterial       = 0x01, //!< use object material
    Flags_UseObjectTexture        = 0x02, //!< use object texture
    Flags_UseObjectShader         = 0x04, //!< use object GLSL program
    Flags_HatchZoomPersistent     = 0x08, //!< zoom-persistent texturing
    Flags_HatchRotationPersistent = 0x10, //!< rotation-persistent texturing
    Flags_DrawHatching            = 0x20, //!< draw hatching
    Flags_UseObjectProperties     =       //!< use entire fill area aspect from object
        Flags_UseObjectMaterial
      | Flags_UseObjectTexture
      | Flags_UseObjectShader
  };

  void setFlag (const Standard_Boolean theToUse, const unsigned int theFlag)
  {
    myFlags = theToUse ? myFlags | theFlag : myFlags & ~theFlag;
  }

private:

  Graphic3d_MaterialAspect        myMaterial;
  Handle(Graphic3d_TextureMap)    myTexture;
  Handle(Graphic3d_ShaderProgram) myShader;
  Handle(Graphic3d_HatchStyle)    myStippleHatch;
  Handle(Graphic3d_TextureMap)    myTextureHatch;
  Graphic3d_MaterialAspect        myHatchMaterial;
  unsigned int                    myFlags;
  Standard_Size                   myHatchingState;

public:

  DEFINE_STANDARD_RTTIEXT(Graphic3d_AspectFillCapping, Graphic3d_Aspects)
};

DEFINE_STANDARD_HANDLE (Graphic3d_AspectFillCapping, Graphic3d_Aspects)

#endif // _Graphic3d_AspectFillCapping_HeaderFile
