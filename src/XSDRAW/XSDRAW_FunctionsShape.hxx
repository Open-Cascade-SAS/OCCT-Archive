// Copyright (c) 2023 OPEN CASCADE SAS
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

#ifndef _XSDRAW_FunctionsShape_HeaderFile
#define _XSDRAW_FunctionsShape_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>

#include <Draw_Interpretor.hxx>

//! Defines additional commands for XSControl to :
//! - control of initialisation (xinit, xnorm, newmodel)
//! - analyse of the result of a transfer (recorded in a
//! TransientProcess for Read, FinderProcess for Write) :
//! statistics, various lists (roots,complete,abnormal), what
//! about one specific entity, producing a model with the
//! abnormal result
//!
//! This appendix of XSControl is compiled separately to distinguish
//! basic features from user callable forms
class XSDRAW_FunctionsShape 
{
  DEFINE_STANDARD_ALLOC
public:
  //! Defines and loads all functions which work on shapes for XSControl (as ActFunc)
  Standard_EXPORT static void Init(Draw_Interpretor& theDI);
};

#endif // _XSDRAW_FunctionsShape_HeaderFile
