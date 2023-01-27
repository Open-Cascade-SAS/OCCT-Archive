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

//#58 rln 28.12.98 changing default values for Global Section
//sln 14.01.2002 OCC51 : verifying whether entry model of method ActorRead is IGESDatat_IGESModel 

#include <IGESAppli.hxx>
#include <IGESBasic_SubfigureDef.hxx>
#include <IGESControl_ActorWrite.hxx>
#include <IGESControl_AlgoContainer.hxx>
#include <IGESControl_Controller.hxx>
#include <IGESData_IGESModel.hxx>
#include <IGESData_Protocol.hxx>
#include <IGESSelect_AutoCorrect.hxx>
#include <IGESSelect_ComputeStatus.hxx>
#include <IGESSelect_EditDirPart.hxx>
#include <IGESSelect_EditHeader.hxx>
#include <IGESSelect_FloatFormat.hxx>
#include <IGESSelect_IGESName.hxx>
#include <IGESSelect_IGESTypeForm.hxx>
#include <IGESSelect_RemoveCurves.hxx>
#include <IGESSelect_SelectBasicGeom.hxx>
#include <IGESSelect_SelectBypassGroup.hxx>
#include <IGESSelect_SelectBypassSubfigure.hxx>
#include <IGESSelect_SelectFaces.hxx>
#include <IGESSelect_SelectPCurves.hxx>
#include <IGESSelect_SelectSubordinate.hxx>
#include <IGESSelect_SelectVisibleStatus.hxx>
#include <IGESSelect_SetGlobalParameter.hxx>
#include <IGESSelect_SetLabel.hxx>
#include <IGESSelect_SignColor.hxx>
#include <IGESSelect_SignLevelNumber.hxx>
#include <IGESSelect_SignStatus.hxx>
#include <IGESSelect_UpdateFileName.hxx>
#include <IGESSelect_WorkLibrary.hxx>
#include <IGESSolid.hxx>
#include <IGESToBRep.hxx>
#include <IGESToBRep_Actor.hxx>
#include <Interface_InterfaceModel.hxx>
#include <Interface_Macros.hxx>
#include <Interface_Static.hxx>
#include <Standard_Type.hxx>
#include <TCollection_HAsciiString.hxx>
#include <TopoDS_Shape.hxx>
#include <Transfer_ActorOfTransientProcess.hxx>
#include <XSAlgo.hxx>
#include <XSControl_SelectForTransfer.hxx>
#include <XSControl_WorkSession.hxx>

IMPLEMENT_STANDARD_RTTIEXT(IGESControl_Controller,XSControl_Controller)

//=======================================================================
//function : IGESControl_Controller
//purpose  : 
//=======================================================================

IGESControl_Controller::IGESControl_Controller (const Standard_Boolean mod)
: XSControl_Controller ((Standard_CString ) (mod ? "FNES" : "IGES") , (Standard_CString ) (mod ? "fnes" : "iges") ),
  themode (mod)
{
  static Standard_Boolean init = Standard_False;
  if (!init) {
    IGESSolid::Init();
    IGESAppli::Init();
    init = Standard_True;
  }

  myAdaptorLibrary  = new IGESSelect_WorkLibrary(themode);
  myAdaptorProtocol = IGESSelect_WorkLibrary::DefineProtocol();

  Handle(IGESToBRep_Actor) anactiges = new IGESToBRep_Actor;
  anactiges->SetContinuity(0);
  myAdaptorRead     = anactiges;

  myAdaptorWrite    = new IGESControl_ActorWrite;
}

//=======================================================================
//function : NewModel
//purpose  : 
//=======================================================================

Handle(Interface_InterfaceModel) IGESControl_Controller::NewModel () const
{
  //  On prend un modele qu on prepare avec les statiques enregistres
  DeclareAndCast(IGESData_IGESModel,igm,Interface_InterfaceModel::Template("iges"));
  IGESData_GlobalSection GS = igm->GlobalSection();

  GS.SetReceiveName (Interface_Static::Static("write.iges.header.receiver")->HStringValue());
  GS.SetUnitFlag    (Interface_Static::IVal("write.iges.unit"));
  GS.SetUnitName    (new TCollection_HAsciiString(Interface_Static::CVal("write.iges.unit")));
  GS.SetAuthorName  (Interface_Static::Static("write.iges.header.author")->HStringValue());
  GS.SetCompanyName (Interface_Static::Static("write.iges.header.company")->HStringValue());
  igm->SetGlobalSection (GS);
  return igm;
}

//=======================================================================
//function : ActorRead
//purpose  : 
//=======================================================================

Handle(Transfer_ActorOfTransientProcess) IGESControl_Controller::ActorRead (const Handle(Interface_InterfaceModel)& model) const
{
  DeclareAndCast(IGESToBRep_Actor,anactiges,myAdaptorRead);
  if (!anactiges.IsNull()) {
    // sln 14.01.2002 OCC51 : verifying whether entry model is IGESDatat_IGESModel,
    // if this condition is false new model is created
    Handle(Interface_InterfaceModel) aModel = (model->IsKind(STANDARD_TYPE(IGESData_IGESModel))? model : NewModel());
    anactiges->SetModel(GetCasted(IGESData_IGESModel,aModel));
    anactiges->SetContinuity(Interface_Static::IVal("read.iges.bspline.continuity"));
  }
  return myAdaptorRead;
}

//  ####    TRANSFERT (ECRITURE SHAPE)    ####
//  modetrans : 0  <5.1 (groupe de faces),  1 BREP-5.1

//=======================================================================
//function : TransferWriteShape
//purpose  : 
//=======================================================================

XSControl_ReturnStatus IGESControl_Controller::TransferWriteShape (const TopoDS_Shape& shape,
                                                                  const Handle(Transfer_FinderProcess)& FP,
                                                                  const Handle(Interface_InterfaceModel)& model,
                                                                  const Standard_Integer modetrans,
                                                                  const Message_ProgressRange& theProgress) const
{
  return XSControl_Controller::TransferWriteShape(shape, FP, model, modetrans, theProgress);
}

//=======================================================================
//function : Init
//purpose  : 
//=======================================================================

Standard_Boolean IGESControl_Controller::Init ()
{
  static Standard_Boolean inic = Standard_False;
  if (!inic) {
    Handle(IGESControl_Controller) ADIGES = new IGESControl_Controller(Standard_False);
    ADIGES->AutoRecord();
    XSAlgo::Init();
    IGESToBRep::Init();
    IGESToBRep::SetAlgoContainer (new IGESControl_AlgoContainer());
    inic = Standard_True;
  }
  return Standard_True;
}
