// Author: Ilya Khramov
// Copyright (c) 2019 OPEN CASCADE SAS
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

#include <Graphic3d_CubeMap.hxx>

IMPLEMENT_STANDARD_RTTIEXT(Graphic3d_CubeMap, Graphic3d_TextureMap)

// =======================================================================
// function : Graphic3d_CubeMap
// purpose  :
// =======================================================================
Graphic3d_CubeMap::Graphic3d_CubeMap (const TCollection_AsciiString& theFileName,
                                      Standard_Boolean theToGenerateMipmaps)
: Graphic3d_TextureMap (theFileName, Graphic3d_TypeOfTexture_CUBEMAP),
  myCurrentSide  (Graphic3d_CMS_POS_X),
  myEndIsReached (false),
  myZIsInverted  (false)
{
  myHasMipmaps = theToGenerateMipmaps;
}

// =======================================================================
// function : Graphic3d_CubeMap
// purpose  :
// =======================================================================
Graphic3d_CubeMap::Graphic3d_CubeMap (const Handle(Image_PixMap)& thePixmap,
                                      Standard_Boolean theToGenerateMipmaps)
: Graphic3d_TextureMap (thePixmap, Graphic3d_TypeOfTexture_CUBEMAP),
  myCurrentSide  (Graphic3d_CMS_POS_X),
  myEndIsReached (false),
  myZIsInverted  (false)
{
  myHasMipmaps = theToGenerateMipmaps;
}

// =======================================================================
// function : ~Graphic3d_CubeMap
// purpose  :
// =======================================================================
Graphic3d_CubeMap::~Graphic3d_CubeMap()
{
  //
}

// =======================================================================
// function : GetCubeDirection
// purpose  :
// =======================================================================
gp_Dir Graphic3d_CubeMap::GetCubeDirection (Graphic3d_CubeMapSide theFace)
{
  const Standard_Integer aDiv2 = theFace / 2;
  const Standard_Integer aMod2 = theFace % 2;
  return gp_Dir (aDiv2 == 0 ? aMod2 == 0 ? 1.0 : -1.0 : 0.0,
                 aDiv2 == 1 ? aMod2 == 0 ? 1.0 : -1.0 : 0.0,
                 aDiv2 == 2 ? aMod2 == 0 ? 1.0 : -1.0 : 0.0);
}

// =======================================================================
// function : GetCubeUp
// purpose  :
// =======================================================================
gp_Dir Graphic3d_CubeMap::GetCubeUp (Graphic3d_CubeMapSide theFace)
{
  const Standard_Integer aDiv2 = theFace / 2;
  const Standard_Integer aMod2 = theFace % 2;
  return gp_Dir (0.0,
                 aDiv2 == 0 ? 1.0 : aDiv2 == 2 ? -1.0 : 0.0,
                 aDiv2 == 1 ? aMod2 == 0 ? 1.0 : -1.0 : 0.0);
}
