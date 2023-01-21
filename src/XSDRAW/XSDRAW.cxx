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

#include <XSDRAW.hxx>

#include <Draw_PluginMacro.hxx>
#include <XSDRAW_Functions.hxx>
#include <XSDRAW_FunctionsSession.hxx>
#include <XSDRAW_FunctionsShape.hxx>
#include <XSAlgo.hxx>

void XSDRAW::Factory(Draw_Interpretor& theDI)
{
  static Standard_Boolean aIsActivated = Standard_False;
  if (aIsActivated)
  {
    return;
  }
  aIsActivated = Standard_True;
  XSAlgo::Init();
  XSDRAW_Functions::Init(theDI);
  XSDRAW_FunctionsSession::Init(theDI);
  XSDRAW_FunctionsShape::Init(theDI);
}

// Declare entry point PLUGINFACTORY
DPLUGIN(XSDRAW)
