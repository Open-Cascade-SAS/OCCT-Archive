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
#include <StepAP214.hxx>
#include <StepAP214_Protocol.hxx>
#include <StepData_WriterLib.hxx>

DEFINE_STANDARD_HANDLE(RWStepAP214_Mgr, Standard_Transient)

class RWStepAP214_Mgr : public Standard_Transient
{
public:
  RWStepAP214_Mgr()
  {
    myProtocol = StepAP214::Protocol();
    myGeneralModule = new RWStepAP214_GeneralModule;
    myRWModule = new RWStepAP214_ReadWriteModule;
    Interface_GeneralLib::SetGlobal(myGeneralModule, myProtocol);
    Interface_ReaderLib::SetGlobal(myRWModule, myProtocol);
    StepData_WriterLib::SetGlobal(myRWModule, myProtocol);
  }

  virtual ~RWStepAP214_Mgr()
  {
    Interface_GeneralLib::ReleaseGlobal(myGeneralModule);
    Interface_ReaderLib::ReleaseGlobal(myRWModule);
    StepData_WriterLib::ReleaseGlobal(myRWModule);
  }

  DEFINE_STANDARD_RTTI_INLINE(RWStepAP214_Mgr, Standard_Transient)

private:
  Handle(StepAP214_Protocol) myProtocol;
  Handle(RWStepAP214_ReadWriteModule) myRWModule;
  Handle(RWStepAP214_GeneralModule) myGeneralModule;
};

static Handle(RWStepAP214_Mgr) THE_MGR;

void RWStepAP214::Init()
{
  if (THE_MGR.IsNull())
  {
    RWHeaderSection::Init();
    THE_MGR = new RWStepAP214_Mgr;
  }
}
