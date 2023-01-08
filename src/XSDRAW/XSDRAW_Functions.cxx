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
#include <XSControl.hxx>
#include <XSControl_Controller.hxx>
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
  if (theNbArgs != 2 && theNbArgs != 1)
  {
    theDI << "Error:";
    return 1;
  }
  if (theNbArgs > 1)
  {
    if (!XSDRAWBase::Session()->SelectNorm(theArgVec[1]));
    {
      theDI << "Error:";
      return 1;
    }
  }
  else
  {
    Message::SendInfo() << "Selected Norm:";
    theDI << XSDRAWBase::Session()->SelectedNorm() << "\n";
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
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  Handle(XSControl_Controller) control = WS->NormAdaptor();
  if (theNbArgs == 1)
  {
    Message::SendInfo() << "Current Norm. xnorm newnorm to change";
  }
  else
  {
    Message::SendInfo() << "Selected Norm:";
  }
  if (control.IsNull())
  {
    Message::SendInfo() << "no norm currently defined";
  }
  else
  {
    Message::SendInfo() << "  Long  Name (complete) : "
      << control->Name(Standard_False) << std::endl
      << "  Short name (resource) : " << control->Name(Standard_True) << std::endl;
  }
  if (theNbArgs == 1)
  {
    return 0;
  }

  control = XSControl_Controller::Recorded(theArgVec[1]);
  if (control.IsNull())
  {
    Message::SendInfo() << " No norm named : " << theArgVec[1] << std::endl;
    return 1;
  }

  WS->SetController(control);
  Message::SendInfo() << "new norm : " << control->Name() << std::endl;
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
  if (!XSDRAWBase::Session()->NewModel().IsNull())
  {
    return 0;
  }
  Message::SendInfo() << "No new Model produced" << std::endl;
  return 1;
}

//=======================================================================
//function : XSControl_tpclear
//purpose  :
//=======================================================================
static Standard_Integer XSControl_tpclear(Draw_Interpretor& theDI,
                                          Standard_Integer theNbArgs,
                                          const char** theArgVec)
{
  const Standard_Boolean modew = (theArgVec[0][2] == 'w');
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  const Handle(Transfer_FinderProcess)& FP =
    WS->TransferWriter()->FinderProcess();
  const Handle(Transfer_TransientProcess)& TP =
    WS->TransferReader()->TransientProcess();
  if (modew)
  {
    if (!FP.IsNull()) FP->Clear();
    else Message::SendInfo() << "No Transfer Write" << std::endl;
  }
  else
  {
    if (!TP.IsNull())
      TP->Clear();
    else
      Message::SendInfo() << "No Transfer Read" << std::endl;
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
  const Standard_CString arg1 = theArgVec[1];
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  const Handle(Transfer_TransientProcess)& TP =
    WS->TransferReader()->TransientProcess();
  if (TP.IsNull())
  {
    Message::SendInfo() << "No Transfer Read" << std::endl; return 1;
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
    Message::SendInfo() << "Unknown Mode";
  if (mod1 < 0)
  {
    Message::SendInfo() << "Modes available :\n"
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
    if (mod1 < -1) return 1;
    return 0;
  }

  if (!TP.IsNull())
  {
    Message::SendInfo() << "TransferRead :";
    if (TP->Model() != WS->Model()) Message::SendInfo() << "Model differs from the session";
    Handle(TColStd_HSequenceOfTransient) list =
      IFSelect_Functions::GiveList(pilot->Session(), pilot->CommandPart(2));
    XSControl_TransferReader::PrintStatsOnList(TP, list, mod1, mod2);
    //    TP->PrintStats (1,Message::SendInfo());
  }
  else Message::SendInfo() << "TransferRead : not defined" << std::endl;
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
  const Standard_CString arg1 = theArgVec[1];
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  const Handle(Transfer_TransientProcess)& TP =
    WS->TransferReader()->TransientProcess();
  //        ****    tpent        ****
  if (TP.IsNull())
  {
    Message::SendInfo() << "No Transfer Read";
    return 1;
  }
  Handle(Interface_InterfaceModel) model = TP->Model();
  if (model.IsNull())
    return 1;

  if (theNbArgs < 2)
  {
    Message::SendInfo() << "Give ENTITY NUMBER (IN MODEL TransferProcess)";
    return 1;
  }
  Standard_Integer num = atoi(arg1);
  if (num <= 0 || num > model->NbEntities())
  {
    Message::SendInfo() << "Number not in [1 - "
      << model->NbEntities() << "]";
    return 1;
  }
  Handle(Standard_Transient) ent = model->Value(num);
  Standard_Integer index = TP->MapIndex(ent);
  if (index == 0)
    Message::SendInfo() << "Entity " << num << "  not recorded in transfer";
  else
    WS->PrintTransferStatus(index, Standard_False, Message::SendInfo());
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
  const Standard_CString arg1 = theArgVec[1];
  //        ****    tpitem/tproot/twitem/twroot        ****
  if (theNbArgs < 2)
  {
    Message::SendInfo() << "Give ITEM NUMBER (in TransferProcess)";
    return 1;
  }
  Standard_Integer num = atoi(arg1);
  if (theArgVec[0][3] == 'r') num = -num;
  Standard_Boolean modew = Standard_False;
  if (theArgVec[0][2] == 'w') modew = Standard_True;
  Handle(Transfer_Binder) binder;
  Handle(Transfer_Finder) finder;
  Handle(Standard_Transient) ent;
  if (!XSDRAWBase::Session()->PrintTransferStatus(num, modew, Message::SendInfo()))
  {
    Message::SendInfo() << " - Num=" << num << " incorrect";
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
  const Standard_CString arg1 = theArgVec[1];
  const Handle(Transfer_TransientProcess)& TP = XSControl::Session(pilot)->TransferReader()->TransientProcess();
  //        ****    trecord : TransferReader        ****
  Standard_Boolean tous = (theNbArgs == 1);
  Standard_Integer num = -1;
  const Handle(Interface_InterfaceModel)& mdl = XSControl::Session(pilot)->Model();
  const Handle(XSControl_TransferReader)& TR = XSControl::Session(pilot)->TransferReader();
  Handle(Standard_Transient) ent;
  Message_Messenger::StreamBuffer Message::SendInfo() = Message::SendInfo();
  if (mdl.IsNull() || TR.IsNull() || TP.IsNull())
  {
    Message::SendInfo() << " init not done" << std::endl; return 1;
  }
  if (!tous) num = atoi(arg1);
  //    Enregistrer les racines
  if (tous)
  {
    Standard_Integer nb = TP->NbRoots();
    Message::SendInfo() << " Recording " << nb << " Roots" << std::endl;
    for (Standard_Integer i = 1; i <= nb; i++)
    {
      ent = TP->Root(i);
      if (TR->RecordResult(ent)) Message::SendInfo() << " Root n0." << i << std::endl;
      else Message::SendInfo() << " Root n0." << i << " not recorded" << std::endl;
    }
  }
  else
  {
    if (num < 1 || num > mdl->NbEntities()) Message::SendInfo() << "incorrect number:" << num << std::endl;
    else if (TR->RecordResult(mdl->Value(num))) Message::SendInfo() << " Entity n0." << num << std::endl;
    else Message::SendInfo() << " Entity n0." << num << " not recorded" << std::endl;
  }
  return IFSelect_RetDone;
}

//=======================================================================
//function : XSControl_trstat
//purpose  :
//=======================================================================
static Standard_Integer XSControl_trstat(Draw_Interpretor& theDI,
                                         Standard_Integer theNbArgs,
                                         const char** theArgVec)
{
  Standard_Integer theNbArgs = pilot->NbWords();
  const Standard_CString arg1 = pilot->Arg(1);
  Message_Messenger::StreamBuffer Message::SendInfo() = Message::SendInfo();
  //        ****    trstat : TransferReader        ****
  const Handle(XSControl_TransferReader)& TR = XSControl::Session(pilot)->TransferReader();
  if (TR.IsNull())
  {
    Message::SendInfo() << " init not done" << std::endl; return 1;
  }
  Handle(Interface_InterfaceModel)  mdl = TR->Model();
  if (mdl.IsNull())
  {
    Message::SendInfo() << " No model" << std::endl; return 1;
  }
  Message::SendInfo() << " Statistics : FileName : " << TR->FileName() << std::endl;
  if (theNbArgs == 1)
  {
    // stats generales
    TR->PrintStats(Message::SendInfo(), 10, 0);
  }
  else
  {
    // stats unitaires
    Standard_Integer num = atoi(arg1);
    if (num < 1 || num > mdl->NbEntities())
    {
      Message::SendInfo() << " incorrect number:" << arg1 << std::endl; return 1;
    }
    Handle(Standard_Transient) ent = mdl->Value(num);
    if (!TR->IsRecorded(ent))
    {
      Message::SendInfo() << " Entity " << num << " not recorded" << std::endl; return 1;
    }
    Handle(Transfer_ResultFromModel) RM = TR->FinalResult(ent);
    Handle(TColStd_HSequenceOfTransient) list = TR->CheckedList(ent);
    Standard_Integer i, nb = list->Length();
    if (nb > 0) Message::SendInfo() << " Entities implied by Check/Result :" << nb << " i.e.:";
    for (i = 1; i <= nb; i++)
    {
      Message::SendInfo() << "  "; mdl->Print(list->Value(i), Message::SendInfo());
    }
    Message::SendInfo() << std::endl;
    if (RM.IsNull())
    {
      Message::SendInfo() << " no other info" << std::endl; return 0;
    }
    Interface_CheckIterator chl = RM->CheckList(Standard_False);
    pilot->Session()->PrintCheckList(Message::SendInfo(), chl, Standard_False, IFSelect_EntitiesByItem);
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
  //        ****    trbegin : TransferReader        ****
  Handle(XSControl_TransferReader) TR = XSControl::Session(pilot)->TransferReader();
  Standard_Boolean init = TR.IsNull();
  if (pilot->NbWords() > 1)
  {
    if (pilot->Arg(1)[0] == 'i') init = Standard_True;
  }
  if (init)
  {
    XSControl::Session(pilot)->InitTransferReader(0);
    TR = XSControl::Session(pilot)->TransferReader();
    if (TR.IsNull())
    {
      Message_Messenger::StreamBuffer Message::SendInfo() = Message::SendInfo();
      Message::SendInfo() << " init not done or failed" << std::endl;
      return 1;
    }
  }
  TR->BeginTransfer();
  return IFSelect_RetDone;
}

//=======================================================================
//function : XSControl_tread
//purpose  :
//=======================================================================
static Standard_Integer XSControl_tread(Draw_Interpretor& theDI,
                                        Standard_Integer theNbArgs,
                                        const char** theArgVec)
{
  Standard_Integer theNbArgs = pilot->NbWords();
  //const Standard_CString arg1 = pilot->Arg(1);
  //        ****    tread : TransferReader        ****
  Message_Messenger::StreamBuffer Message::SendInfo() = Message::SendInfo();
  const Handle(XSControl_TransferReader)& TR = XSControl::Session(pilot)->TransferReader();
  if (TR.IsNull())
  {
    Message::SendInfo() << " init not done" << std::endl; return 1;
  }
  const Handle(Interface_InterfaceModel)& mdl = TR->Model();
  if (mdl.IsNull())
  {
    Message::SendInfo() << " No model" << std::endl; return 1;
  }
  if (theNbArgs < 2)
  {
    //      DeclareAndCast(IFSelect_Selection,sel,pilot->Session()->NamedItem("xst-model-roots"));
    Handle(Standard_Transient) sel = pilot->Session()->NamedItem("xst-model-roots");
    if (sel.IsNull())
    {
      Message::SendInfo() << "Select Roots absent" << std::endl; return 1;
    }
    Handle(TColStd_HSequenceOfTransient) list = pilot->Session()->GiveList(sel);
    Message::SendInfo() << " Transferring all roots i.e. : " << TR->TransferList(list) << std::endl;
  }
  else
  {
    Handle(TColStd_HSequenceOfTransient) list =
      IFSelect_Functions::GiveList(pilot->Session(), pilot->CommandPart(1));
    Message::SendInfo() << " Transfer of " << list->Length() << " entities" << std::endl;
    Standard_Integer nb = TR->TransferList(list);
    Message::SendInfo() << " Gives " << nb << " results" << std::endl;
  }
  return IFSelect_RetDone;
}

//=======================================================================
//function : XSControl_trtp
//purpose  :
//=======================================================================
static Standard_Integer XSControl_trtp(Draw_Interpretor& theDI,
                                       Standard_Integer theNbArgs,
                                       const char** theArgVec)
{
  //        ****    TReader -> TProcess         ****
  const Handle(XSControl_TransferReader)& TR = XSControl::Session(pilot)->TransferReader();
  Message_Messenger::StreamBuffer Message::SendInfo() = Message::SendInfo();
  if (TR.IsNull()) Message::SendInfo() << " No TransferReader" << std::endl;
  else if (TR->TransientProcess().IsNull()) Message::SendInfo() << " Transfer Reader without Process" << std::endl;
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
  //        ****    TProcess -> TReader         ****
  XSControl::Session(pilot)->InitTransferReader(3);
  return IFSelect_RetDone;
}

//=======================================================================
//function : XSControl_twmode
//purpose  :
//=======================================================================
static Standard_Integer XSControl_twmode(Draw_Interpretor& theDI,
                                         Standard_Integer theNbArgs,
                                         const char** theArgVec)
{
  Standard_Integer theNbArgs = pilot->NbWords();
  const Standard_CString arg1 = pilot->Arg(1);
  //        ****    twmode         ****
  Handle(XSControl_TransferWriter) TW = XSControl::Session(pilot)->TransferWriter();
  Handle(XSControl_Controller) control = XSControl::Session(pilot)->NormAdaptor();
  Standard_Integer modemin, modemax;
  Message_Messenger::StreamBuffer Message::SendInfo() = Message::SendInfo();
  if (control->ModeWriteBounds(modemin, modemax))
  {
    Message::SendInfo() << "Write Mode : allowed values  " << modemin << " to " << modemax << std::endl;
    for (Standard_Integer modd = modemin; modd <= modemax; modd++)
    {
      Message::SendInfo() << modd << "	: " << control->ModeWriteHelp(modd) << std::endl;
    }
  }
  Message::SendInfo() << "Write Mode : actual = " << TW->TransferMode() << std::endl;
  if (theNbArgs <= 1) return 0;
  Standard_Integer mod = atoi(arg1);
  Message::SendInfo() << "New value -> " << arg1 << std::endl;
  TW->SetTransferMode(mod);
  if (!control->IsModeWrite(mod)) Message::SendInfo() << "Warning : this new value is not supported" << std::endl;
  return IFSelect_RetDone;
}

//=======================================================================
//function : XSControl_twstat
//purpose  :
//=======================================================================
static Standard_Integer XSControl_twstat(Draw_Interpretor& theDI,
                                         Standard_Integer theNbArgs,
                                         const char** theArgVec)
{
  //Standard_Integer theNbArgs = pilot->NbWords();
  //const Standard_CString arg1 = pilot->Arg(1);
  //const Standard_CString arg2 = pilot->Arg(2);
  const Handle(Transfer_FinderProcess)& FP = XSControl::Session(pilot)->TransferWriter()->FinderProcess();
  //        ****    twstat        ****
  //  Pour Write
  Message_Messenger::StreamBuffer Message::SendInfo() = Message::SendInfo();
  if (!FP.IsNull())
  {
    Message::SendInfo() << "TransferWrite:";
    //    XSControl_TransferWriter::PrintStatsProcess (FP,mod1,mod2);
    FP->PrintStats(1, Message::SendInfo());
  }
  else Message::SendInfo() << "TransferWrite: not defined" << std::endl;
  return 0;
}

//=======================================================================
//function : XSControl_settransfert
//purpose  :
//=======================================================================
static Standard_Integer XSControl_settransfert(Draw_Interpretor& theDI,
                                               Standard_Integer theNbArgs,
                                               const char** theArgVec)
{
  //        ****    SelectForTransfer           ****
  return pilot->RecordItem(new XSControl_SelectForTransfer(XSControl::Session(pilot)->TransferReader()));
}


//=======================================================================
//function : Init
//purpose  :
//=======================================================================
void XSDRAW_Functions::Init(Draw_Interpretor& theDI)
{
  static Standard_Boolean initactor = Standard_False;
  if (initactor)
  {
    return;
  }
  initactor = Standard_True;
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

  theDI.Add("selecttransfer",
            "selection (recognize from transfer actor)", __FILE__, XSControl_settransfert);
}
