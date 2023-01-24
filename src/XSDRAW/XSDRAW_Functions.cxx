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

#include <XSDRAW_Functions.hxx>

#include <IFSelect_Act.hxx>
#include <IFSelect_CheckCounter.hxx>
#include <IFSelect_SessionPilot.hxx>
#include <Interface_Static.hxx>
#include <Message.hxx>
#include <Message_Messenger.hxx>
#include <TColStd_HSequenceOfTransient.hxx>
#include <Transfer_ResultFromModel.hxx>
#include <Transfer_TransientProcess.hxx>
#include <XSControl_Controller.hxx>
#include <XSDRAW.hxx>
#include <XSDRAWBase.hxx>
#include <XSControl_SelectForTransfer.hxx>
#include <XSControl_TransferReader.hxx>
#include <XSControl_TransferWriter.hxx>
#include <XSControl_WorkSession.hxx>

//=======================================================================
//function : XSControl_xinit
//purpose  :
//=======================================================================
static Standard_Integer XSControl_xinit(Draw_Interpretor& theDI,
                                        Standard_Integer theNbArgs,
                                        const char** theArgVec)
{
  XSDRAW::StreamContainer aSSC(theDI);
  if (theNbArgs != 2 && theNbArgs != 1)
  {
    aSSC.SStream() << "Error:";
    return 1;
  }
  if (theNbArgs > 1)
  {
    if (!XSDRAWBase::Session()->SelectNorm(theArgVec[1]))
    {
      aSSC.SStream() << "Error:";
      return 1;
    }
  }
  else
  {
    aSSC.SStream() << "Selected Norm:";
    aSSC.SStream() << XSDRAWBase::Session()->SelectedNorm() << "\n";
  }
  return 0;
}

//=======================================================================
//function : XSControl_xnorm
//purpose  :
//=======================================================================
static Standard_Integer XSControl_xnorm(Draw_Interpretor& theDI,
                                        Standard_Integer theNbArgs,
                                        const char** theArgVec)
{
  XSDRAW::StreamContainer aSSC(theDI);
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  Handle(XSControl_Controller) control = WS->NormAdaptor();
  if (theNbArgs == 1)
  {
    aSSC.SStream() << "Current Norm. xnorm newnorm to change";
  }
  else
  {
    aSSC.SStream() << "Selected Norm:";
  }
  if (control.IsNull())
  {
    aSSC.SStream() << "no norm currently defined";
  }
  else
  {
    aSSC.SStream() << "  Long  Name (complete) : "
      << control->Name(Standard_False)
      << "  Short name (resource) : " << control->Name(Standard_True);
  }
  if (theNbArgs == 1)
  {
    return 0;
  }

  control = XSControl_Controller::Recorded(theArgVec[1]);
  if (control.IsNull())
  {
    aSSC.SStream() << " No norm named : " << theArgVec[1];
    return 1;
  }

  WS->SetController(control);
  aSSC.SStream() << "new norm : " << control->Name();
  return 0;
}

//=======================================================================
//function : XSControl_newmodel
//purpose  :
//=======================================================================
static Standard_Integer XSControl_newmodel(Draw_Interpretor& theDI,
                                           Standard_Integer theNbArgs,
                                           const char** theArgVec)
{
  (void)theDI;
  (void)theNbArgs;
  (void)theArgVec;
  return 0;
}

//=======================================================================
//function : XSControl_tpclear
//purpose  :
//=======================================================================
static Standard_Integer XSControl_tpclear(Draw_Interpretor& theDI,
                                          Standard_Integer theNbArgs,
                                          const char** theArgVec)
{
  XSDRAW::StreamContainer aSSC(theDI);
  (void)theNbArgs;
  const Standard_Boolean modew = (theArgVec[0][2] == 'w');
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  const Handle(Transfer_FinderProcess)& FP =
    WS->TransferWriter()->FinderProcess();
  const Handle(Transfer_TransientProcess)& TP =
    WS->TransferReader()->TransientProcess();
  if (modew)
  {
    if (!FP.IsNull()) FP->Clear();
    else aSSC.SStream() << "No Transfer Write";
  }
  else
  {
    if (!TP.IsNull())
      TP->Clear();
    else
      aSSC.SStream() << "No Transfer Read";
  }
  return 0;
}

//=======================================================================
//function : XSControl_tpstat
//purpose  :
//=======================================================================
static Standard_Integer XSControl_tpstat(Draw_Interpretor& theDI,
                                         Standard_Integer theNbArgs,
                                         const char** theArgVec)
{
  XSDRAW::StreamContainer aSSC(theDI);
  const Standard_CString arg1 = theArgVec[1];
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  const Handle(Transfer_TransientProcess)& TP =
    WS->TransferReader()->TransientProcess();
  if (TP.IsNull())
  {
    aSSC.SStream() << "No Transfer Read";
    return 1;
  }
  //        ****    tpstat        ****

  Standard_Integer mod1 = -1;
  Standard_Integer mod2 = 0;
  //  g : general  c : check (compte) C (liste)   f : fails(compte)  F (liste)
  //  resultats racines :  n : n0s entites  s : status  b : binders
  //    t : compte par type  r : compte par resultat   l : liste(type-resultat)
  //   *n *s *b *t *r *l : idem sur tout
  //   ?n  etc.. : idem sur resultats anormaux
  //   ?  tout court pour help

  if (theNbArgs > 1)
  {
    char a2 = arg1[1]; if (a2 == '\0') a2 = '!';
    switch (arg1[0])
    {
      case 'g': mod1 = 0; break;
      case 'c': mod1 = 4; mod2 = 4; break;
      case 'C': mod1 = 4; mod2 = 2; break;
      case 'f': mod1 = 5; mod2 = 4; break;
      case 'F': mod1 = 5; mod2 = 2; break;
      case '*': mod1 = 2; break;
      case '?': mod1 = 3; break;
      default: mod1 = 1; if (theNbArgs > 2) mod1 = 2; a2 = arg1[0]; break;
    }
    if (mod1 < 1 || mod1 > 3) a2 = '!';
    switch (a2)
    {
      case 'n': mod2 = 0; break;
      case 's': mod2 = 1; break;
      case 'b': mod2 = 2; break;
      case 't': mod2 = 3; break;
      case 'r': mod2 = 4; break;
      case 'l': mod2 = 5; break;
      case 'L': mod2 = 6; break;
      case '!': break;
      case '?': mod1 = -1; break;
      default: mod1 = -2; break;
    }
  }
  //  A present help eventuel
  if (mod1 < -1)
    aSSC.SStream() << "Unknown Mode";
  if (mod1 < 0)
  {
    aSSC.SStream() << "Modes available :\n"
      << "g : general    c : checks (count)  C (list)\n"
      << "               f : fails  (count)  F (list)\n"
      << "  n : numbers of transferred entities (on TRANSFER ROOTS)\n"
      << "  s : their status (type entity-result , presence checks)\n"
      << "  b : detail of binders\n"
      << "  t : count per entity type    r : per type/status result\n"
      << "  l : count per couple  type entity/result\n"
      << "  L : list  per couple  type entity/result\n"
      << "  *n  *s  *b  *t  *r  *l  *L : idem on ALL recorded items\n"
      << "  ?n  ?s  ?b  ?t ... : idem on abnormal items\n"
      << "  n select : n applied on a selection   idem for  s b t r l";
    if (mod1 < -1)
      return 1;
    return 0;
  }

  if (!TP.IsNull())
  {
    aSSC.SStream() << "TransferRead :";
    if (TP->Model() != WS->Model()) aSSC.SStream() << "Model differs from the session";
    Handle(TColStd_HSequenceOfTransient) list =
      WS->GiveList(theArgVec[2]);
    XSControl_TransferReader::PrintStatsOnList(TP, aSSC.SStream(), list, mod1, mod2);
    //    TP->PrintStats (1,aSSC.SStream());
  }
  else aSSC.SStream() << "TransferRead : not defined";
  return 0;
}

//=======================================================================
//function : XSControl_tpent
//purpose  :
//=======================================================================
static Standard_Integer XSControl_tpent(Draw_Interpretor& theDI,
                                        Standard_Integer theNbArgs,
                                        const char** theArgVec)
{
  XSDRAW::StreamContainer aSSC(theDI);
  const Standard_CString arg1 = theArgVec[1];
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  const Handle(Transfer_TransientProcess)& TP =
    WS->TransferReader()->TransientProcess();
  //        ****    tpent        ****
  if (TP.IsNull())
  {
    aSSC.SStream() << "No Transfer Read";
    return 1;
  }
  Handle(Interface_InterfaceModel) model = TP->Model();
  if (model.IsNull())
    return 1;

  if (theNbArgs < 2)
  {
    aSSC.SStream() << "Give ENTITY NUMBER (IN MODEL TransferProcess)";
    return 1;
  }
  Standard_Integer num = atoi(arg1);
  if (num <= 0 || num > model->NbEntities())
  {
    aSSC.SStream() << "Number not in [1 - "
      << model->NbEntities() << "]";
    return 1;
  }
  Handle(Standard_Transient) ent = model->Value(num);
  Standard_Integer index = TP->MapIndex(ent);
  if (index == 0)
    aSSC.SStream() << "Entity " << num << "  not recorded in transfer";
  else
    WS->PrintTransferStatus(index, Standard_False, aSSC.SStream());
  return 0;
}

//=======================================================================
//function : XSControl_tpitem
//purpose  :
//=======================================================================
static Standard_Integer XSControl_tpitem(Draw_Interpretor& theDI,
                                         Standard_Integer theNbArgs,
                                         const char** theArgVec)
{
  XSDRAW::StreamContainer aSSC(theDI);
  const Standard_CString arg1 = theArgVec[1];
  //        ****    tpitem/tproot/twitem/twroot        ****
  if (theNbArgs < 2)
  {
    aSSC.SStream() << "Give ITEM NUMBER (in TransferProcess)";
    return 1;
  }
  Standard_Integer num = atoi(arg1);
  if (theArgVec[0][3] == 'r') num = -num;
  Standard_Boolean modew = Standard_False;
  if (theArgVec[0][2] == 'w') modew = Standard_True;
  Handle(Transfer_Binder) binder;
  Handle(Transfer_Finder) finder;
  Handle(Standard_Transient) ent;
  if (!XSDRAWBase::Session()->PrintTransferStatus(num, modew, aSSC.SStream()))
  {
    aSSC.SStream() << " - Num=" << num << " incorrect";
  }
  return 0;
}

//=======================================================================
//function : XSControl_trecord
//purpose  :
//=======================================================================
static Standard_Integer XSControl_trecord(Draw_Interpretor& theDI,
                                          Standard_Integer theNbArgs,
                                          const char** theArgVec)
{
  XSDRAW::StreamContainer aSSC(theDI);
  const Standard_CString arg1 = theArgVec[1];
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  const Handle(Transfer_TransientProcess)& TP = WS->TransferReader()->TransientProcess();
  //        ****    trecord : TransferReader        ****
  Standard_Boolean tous = (theNbArgs == 1);
  Standard_Integer num = -1;
  const Handle(Interface_InterfaceModel)& mdl = WS->Model();
  const Handle(XSControl_TransferReader)& TR = WS->TransferReader();
  Handle(Standard_Transient) ent;
  if (mdl.IsNull() || TR.IsNull() || TP.IsNull())
  {
    aSSC.SStream() << " init not done";
    return 1;
  }
  if (!tous)
    num = atoi(arg1);
  //    Enregistrer les racines
  if (tous)
  {
    Standard_Integer nb = TP->NbRoots();
    aSSC.SStream() << " Recording " << nb << " Roots";
    for (Standard_Integer i = 1; i <= nb; i++)
    {
      ent = TP->Root(i);
      if (TR->RecordResult(ent))
        aSSC.SStream() << " Root n0." << i;
      else
        aSSC.SStream() << " Root n0." << i << " not recorded";
    }
  }
  else
  {
    if (num < 1 || num > mdl->NbEntities())
      aSSC.SStream() << "incorrect number:" << num;
    else if (TR->RecordResult(mdl->Value(num)))
      aSSC.SStream() << " Entity n0." << num;
    else
      aSSC.SStream() << " Entity n0." << num << " not recorded";
  }
  return 0;
}

//=======================================================================
//function : XSControl_trstat
//purpose  :
//=======================================================================
static Standard_Integer XSControl_trstat(Draw_Interpretor& theDI,
                                         Standard_Integer theNbArgs,
                                         const char** theArgVec)
{
  XSDRAW::StreamContainer aSSC(theDI);
  const Standard_CString arg1 = theArgVec[1];
  //        ****    trstat : TransferReader        ****
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  const Handle(XSControl_TransferReader)& TR = WS->TransferReader();
  if (TR.IsNull())
  {
    aSSC.SStream() << " init not done";
    return 1;
  }
  Handle(Interface_InterfaceModel)  mdl = TR->Model();
  if (mdl.IsNull())
  {
    aSSC.SStream() << " No model";
    return 1;
  }
  aSSC.SStream() << " Statistics : FileName : " << TR->FileName();
  if (theNbArgs == 1)
  {
    // stats generales
    TR->PrintStats(aSSC.SStream(), 10, 0);
  }
  else
  {
    // stats unitaires
    Standard_Integer num = atoi(arg1);
    if (num < 1 || num > mdl->NbEntities())
    {
      aSSC.SStream() << " incorrect number:" << arg1;
      return 1;
    }
    Handle(Standard_Transient) ent = mdl->Value(num);
    if (!TR->IsRecorded(ent))
    {
      aSSC.SStream() << " Entity " << num << " not recorded";
      return 1;
    }
    Handle(Transfer_ResultFromModel) RM = TR->FinalResult(ent);
    Handle(TColStd_HSequenceOfTransient) list = TR->CheckedList(ent);
    Standard_Integer i, nb = list->Length();
    if (nb > 0)
      aSSC.SStream() << " Entities implied by Check/Result :" << nb << " i.e.:";
    for (i = 1; i <= nb; i++)
    {
      aSSC.SStream() << "  "; mdl->Print(list->Value(i), aSSC.SStream());
    }
    if (RM.IsNull())
    {
      aSSC.SStream() << " no other info";
      return 0;
    }
    Interface_CheckIterator chl = RM->CheckList(Standard_False);
    WS->PrintCheckList(aSSC.SStream(), chl, Standard_False, IFSelect_EntitiesByItem);
  }
  return 0;
}

//=======================================================================
//function : XSControl_trbegin
//purpose  :
//=======================================================================
static Standard_Integer XSControl_trbegin(Draw_Interpretor& theDI,
                                          Standard_Integer theNbArgs,
                                          const char** theArgVec)
{
  XSDRAW::StreamContainer aSSC(theDI);
  //        ****    trbegin : TransferReader        ****
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  Handle(XSControl_TransferReader) TR = WS->TransferReader();
  Standard_Boolean init = TR.IsNull();
  if (theNbArgs > 1)
  {
    if (theArgVec[1][0] == 'i') init = Standard_True;
  }
  if (init)
  {
    WS->InitTransferReader(0);
    TR = WS->TransferReader();
    if (TR.IsNull())
    {
      aSSC.SStream() << " init not done or failed";
      return 1;
    }
  }
  TR->BeginTransfer();
  return 0;
}

//=======================================================================
//function : XSControl_tread
//purpose  :
//=======================================================================
static Standard_Integer XSControl_tread(Draw_Interpretor& theDI,
                                        Standard_Integer theNbArgs,
                                        const char** theArgVec)
{
  XSDRAW::StreamContainer aSSC(theDI);
  //const Standard_CString arg1 = pilot->Arg(1);
  //        ****    tread : TransferReader        ****
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  Handle(XSControl_TransferReader) TR = WS->TransferReader();
  if (TR.IsNull())
  {
    aSSC.SStream() << " init not done";
    return 1;
  }
  const Handle(Interface_InterfaceModel)& mdl = TR->Model();
  if (mdl.IsNull())
  {
    aSSC.SStream() << " No model";
    return 1;
  }
  if (theNbArgs < 2)
  {
    //      DeclareAndCast(IFSelect_Selection,sel,pilot->Session()->NamedItem("xst-model-roots"));
    Handle(Standard_Transient) sel = WS->NamedItem("xst-model-roots");
    if (sel.IsNull())
    {
      aSSC.SStream() << "Select Roots absent";
      return 1;
    }
    Handle(TColStd_HSequenceOfTransient) list = WS->GiveList(sel);
    aSSC.SStream() << " Transferring all roots i.e. : " << TR->TransferList(list);
  }
  else
  {
    Handle(TColStd_HSequenceOfTransient) list =
      WS->GiveList(theArgVec[1]);
    aSSC.SStream() << " Transfer of " << list->Length() << " entities";
    Standard_Integer nb = TR->TransferList(list);
    aSSC.SStream() << " Gives " << nb << " results";
  }
  return 0;
}

//=======================================================================
//function : XSControl_trtp
//purpose  :
//=======================================================================
static Standard_Integer XSControl_trtp(Draw_Interpretor& theDI,
                                       Standard_Integer theNbArgs,
                                       const char** theArgVec)
{
  XSDRAW::StreamContainer aSSC(theDI);
  (void)theNbArgs;
  (void)theArgVec;
  //        ****    TReader -> TProcess         ****
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  const Handle(XSControl_TransferReader)& TR = WS->TransferReader();
  if (TR.IsNull())
    aSSC.SStream() << " No TransferReader";
  else if (TR->TransientProcess().IsNull())
    aSSC.SStream() << " Transfer Reader without Process";
  return 0;
}

//=======================================================================
//function : XSControl_tptr
//purpose  :
//=======================================================================
static Standard_Integer XSControl_tptr(Draw_Interpretor& theDI,
                                       Standard_Integer theNbArgs,
                                       const char** theArgVec)
{
  XSDRAW::StreamContainer aSSC(theDI);
  (void)theNbArgs;
  (void)theArgVec;
  //        ****    TProcess -> TReader         ****
  XSDRAWBase::Session()->InitTransferReader(3);
  return 0;
}

//=======================================================================
//function : XSControl_twmode
//purpose  :
//=======================================================================
static Standard_Integer XSControl_twmode(Draw_Interpretor& theDI,
                                         Standard_Integer theNbArgs,
                                         const char** theArgVec)
{
  XSDRAW::StreamContainer aSSC(theDI);
  const Standard_CString arg1 = theArgVec[1];
  //        ****    twmode         ****
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  Handle(XSControl_TransferWriter) TW = WS->TransferWriter();
  Handle(XSControl_Controller) control = WS->NormAdaptor();
  Standard_Integer modemin, modemax;
  if (control->ModeWriteBounds(modemin, modemax))
  {
    aSSC.SStream() << "Write Mode : allowed values  " << modemin << " to " << modemax;
    for (Standard_Integer modd = modemin; modd <= modemax; modd++)
    {
      aSSC.SStream() << modd << "	: " << control->ModeWriteHelp(modd);
    }
  }
  aSSC.SStream() << "Write Mode : actual = " << TW->TransferMode();
  if (theNbArgs <= 1)
    return 0;
  Standard_Integer mod = atoi(arg1);
  aSSC.SStream() << "New value -> " << arg1;
  TW->SetTransferMode(mod);
  if (!control->IsModeWrite(mod))
    aSSC.SStream() << "Warning : this new value is not supported";
  return 0;
}

//=======================================================================
//function : XSControl_twstat
//purpose  :
//=======================================================================
static Standard_Integer XSControl_twstat(Draw_Interpretor& theDI,
                                         Standard_Integer theNbArgs,
                                         const char** theArgVec)
{
  XSDRAW::StreamContainer aSSC(theDI);
  (void)theNbArgs;
  (void)theArgVec;
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  const Handle(Transfer_FinderProcess)& FP = WS->TransferWriter()->FinderProcess();
  //        ****    twstat        ****
  //  Pour Write
  if (!FP.IsNull())
  {
    aSSC.SStream() << "TransferWrite:";
    //    XSControl_TransferWriter::PrintStatsProcess (FP,mod1,mod2);
    FP->PrintStats(1, aSSC.SStream());
  }
  else
    aSSC.SStream() << "TransferWrite: not defined";
  return 0;
}

//=======================================================================
//function : Init
//purpose  :
//=======================================================================
void XSDRAW_Functions::Init(Draw_Interpretor& theDI)
{
  static Standard_Boolean aIsActivated = Standard_False;
  if (aIsActivated)
  {
    return;
  }
  aIsActivated = Standard_True;
  Standard_CString aGroup = "DE: General";

  theDI.Add("xinit"
            "[norm:string to change norme] reinitialises according to the norm", __FILE__, XSControl_xinit, aGroup);
  theDI.Add("xnorm",
            "displays current norm   +norm : changes it", __FILE__, XSControl_xnorm, aGroup);

  theDI.Add("newmodel",
            "produces a new empty model, for the session", __FILE__, XSControl_newmodel, aGroup);

  theDI.Add("tpclear",
            "Clears  TransferProcess (READ)", __FILE__, XSControl_tpclear, aGroup);
  theDI.Add("twclear",
            "Clears  TransferProcess (WRITE)", __FILE__, XSControl_tpclear, aGroup);

  theDI.Add("tpstat",
            "Statistics on TransferProcess (READ)", __FILE__, XSControl_tpstat, aGroup);

  theDI.Add("tpent",
            "[num:integer] Statistics on an entity of the model (READ)", __FILE__, XSControl_tpent, aGroup);

  theDI.Add("tpitem",
            "[num:integer] Statistics on ITEM of transfer (READ)", __FILE__, XSControl_tpitem, aGroup);
  theDI.Add("tproot",
            "[num:integer] Statistics on a ROOT of transfert (READ)", __FILE__, XSControl_tpitem, aGroup);
  theDI.Add("twitem",
            "[num:integer] Statistics on an ITEM of transfer (WRITE)", __FILE__, XSControl_tpitem, aGroup);
  theDI.Add("twroot",
            "[num:integer] Statistics on a ROOT of transfer (WRITE)", __FILE__, XSControl_tpitem, aGroup);

  theDI.Add("trecord",
            "record : all root results; or num : for entity n0.num", __FILE__, XSControl_trecord, aGroup);
  theDI.Add("trstat",
            "general statistics;  or num : stats on entity n0 num", __FILE__, XSControl_trstat, aGroup);
  theDI.Add("trbegin",
            "begin-transfer-reader [init]", __FILE__, XSControl_trbegin, aGroup);
  theDI.Add("tread",
            "transfers all roots, or num|sel|sel num : entity list, by transfer-reader", __FILE__, XSControl_tread, aGroup);

  theDI.Add("trtp",
            "feeds commands tp... with results from tr...", __FILE__, XSControl_trtp, aGroup);
  theDI.Add("tptr",
            "feeds tr... from tp... (may be incomplete)", __FILE__, XSControl_tptr, aGroup);

  theDI.Add("twmode",
            "displays mode transfer write, + num  changes it", __FILE__, XSControl_twmode, aGroup);
  theDI.Add("twstat",
            "Statistics on TransferProcess (WRITE)", __FILE__, XSControl_twstat, aGroup);
}
