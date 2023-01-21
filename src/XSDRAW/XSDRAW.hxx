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

#ifndef _XSDRAW_HeaderFile
#define _XSDRAW_HeaderFile

#include <Standard.hxx>
#include <Standard_Handle.hxx>

#include <Draw_Interpretor.hxx>

//! Basic package to work functions of X-STEP (IFSelect & Co)
//! under control of DRAW
//!
//! Works with some "static" data : a SessionPilot (used to run)
//! with its WorkSession and Model and TransferReader, a
//! FinderProcess
class XSDRAW
{
  DEFINE_STANDARD_ALLOC
public:

  //! Loads all Draw commands of XSDRAWDEWrapper. Used for plugin.
  Standard_EXPORT static void Factory(Draw_Interpretor& theDI);

public:

  class StreamContainer
  {
    DEFINE_STANDARD_ALLOC
  public:
    StreamContainer(Draw_Interpretor& theDI) : myDI(&theDI) {}
    Standard_SStream& SStream() { return myStream; }
    ~StreamContainer() { *myDI << myStream; }
  private:
    Draw_Interpretor* myDI;
    Standard_SStream myStream;
  };

};

#endif // _XSDRAW_HeaderFile
