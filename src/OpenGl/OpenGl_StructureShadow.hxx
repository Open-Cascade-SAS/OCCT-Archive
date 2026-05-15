// Created on: 2014-09-01
// Created by: Ivan SAZONOV
// Copyright (c) 2014 OPEN CASCADE SAS
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

#ifndef OpenGl_StructureShadow_Header
#define OpenGl_StructureShadow_Header

#include <OpenGl_Structure.hxx>

//! Dummy structure which just redirects to groups of another structure.
class OpenGl_StructureShadow : public OpenGl_Structure
{

public:

  //! Create empty structure
  Standard_EXPORT OpenGl_StructureShadow (const Handle(Graphic3d_StructureManager)& theManager,
                                          const Handle(OpenGl_Structure)&           theStructure);

public:

  //! Raise exception on API misuse.
  Standard_EXPORT virtual void Connect (Graphic3d_CStructure& ) Standard_OVERRIDE;

  //! Raise exception on API misuse.
  Standard_EXPORT virtual void Disconnect (Graphic3d_CStructure& ) Standard_OVERRIDE;

  //! Renders all groups of the parent structure without Z-layer filtering.
  //! The shadow Z-layer is already enforced at the structure level; applying
  //! per-group Z-layer filtering would prevent groups from appearing in the
  //! target highlight layer when they carry a different individual layer.
  Standard_EXPORT virtual void renderGeometry (const Handle(OpenGl_Workspace)& theWorkspace,
                                               bool&                           theHasClosed) const Standard_OVERRIDE;

private:

  Handle(OpenGl_Structure) myParent;

public:

  DEFINE_STANDARD_RTTIEXT(OpenGl_StructureShadow,OpenGl_Structure) // Type definition

};

DEFINE_STANDARD_HANDLE(OpenGl_StructureShadow, OpenGl_Structure)

#endif // OpenGl_StructureShadow_Header
