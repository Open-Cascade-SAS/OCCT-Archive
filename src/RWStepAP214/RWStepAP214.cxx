// Copyright (c) 1999-2014 OPEN CASCADE SAS
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


#include <Interface_GeneralLib.hxx>
#include <Interface_ReaderLib.hxx>
#include <RWHeaderSection.hxx>
#include <RWStepAP214.hxx>
#include <RWStepAP214_GeneralModule.hxx>
#include <RWStepAP214_ReadWriteModule.hxx>
#include <StepData_WriterLib.hxx>
#include <Standard_Mutex.hxx>

static int THE_RWStepAP214_init = 0;

void RWStepAP214::Init(const Handle(StepAP214_Protocol)& theProto)
{
  static Standard_Mutex aPars;
  {

    if (THE_RWStepAP214_init)
    {
      return;
    }
    THE_RWStepAP214_init = 1;
    RWHeaderSection::Init();
    Interface_GeneralLib::SetGlobal(new RWStepAP214_GeneralModule, theProto);
    Interface_ReaderLib::SetGlobal(new RWStepAP214_ReadWriteModule, theProto);
    StepData_WriterLib::SetGlobal(new RWStepAP214_ReadWriteModule, theProto);
  }
}
