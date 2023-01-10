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

#include <XSDRAW_FunctionsSession.hxx>

#include <IFSelect_Act.hxx>
#include <IFSelect_CheckCounter.hxx>
#include <IFSelect_DispGlobal.hxx>
#include <IFSelect_DispPerCount.hxx>
#include <IFSelect_DispPerFiles.hxx>
#include <IFSelect_DispPerOne.hxx>
#include <IFSelect_DispPerSignature.hxx>
#include <IFSelect_EditForm.hxx>
#include <IFSelect_Editor.hxx>
#include <XSDRAW_FunctionsSession.hxx>
#include <IFSelect_GraphCounter.hxx>
#include <IFSelect_IntParam.hxx>
#include <IFSelect_ListEditor.hxx>
#include <IFSelect_ModifReorder.hxx>
#include <IFSelect_SelectDiff.hxx>
#include <IFSelect_SelectEntityNumber.hxx>
#include <IFSelect_SelectErrorEntities.hxx>
#include <IFSelect_SelectIncorrectEntities.hxx>
#include <IFSelect_SelectIntersection.hxx>
#include <IFSelect_Selection.hxx>
#include <IFSelect_SelectModelEntities.hxx>
#include <IFSelect_SelectModelRoots.hxx>
#include <IFSelect_SelectPointed.hxx>
#include <IFSelect_SelectRange.hxx>
#include <IFSelect_SelectRoots.hxx>
#include <IFSelect_SelectShared.hxx>
#include <IFSelect_SelectSharing.hxx>
#include <IFSelect_SelectSignature.hxx>
#include <IFSelect_SelectSuite.hxx>
#include <IFSelect_SelectUnion.hxx>
#include <IFSelect_SelectUnknownEntities.hxx>
#include <IFSelect_SessionFile.hxx>
#include <IFSelect_SessionPilot.hxx>
#include <IFSelect_ShareOut.hxx>
#include <IFSelect_SignatureList.hxx>
#include <IFSelect_SignCounter.hxx>
#include <IFSelect_SignType.hxx>
#include <IFSelect_Transformer.hxx>
#include <IFSelect_WorkLibrary.hxx>
#include <XSControl_WorkSession.hxx>
#include <Interface_Category.hxx>
#include <Interface_CheckIterator.hxx>
#include <Interface_EntityIterator.hxx>
#include <Interface_InterfaceModel.hxx>
#include <Interface_Macros.hxx>
#include <Interface_MSG.hxx>
#include <Interface_Static.hxx>
#include <Interface_Version.hxx>
#include <Message.hxx>
#include <Message_Messenger.hxx>
#include <Standard_Transient.hxx>
#include <TCollection_AsciiString.hxx>
#include <TCollection_HAsciiString.hxx>
#include <TColStd_HSequenceOfAsciiString.hxx>
#include <TColStd_HSequenceOfHAsciiString.hxx>
#include <TColStd_HSequenceOfTransient.hxx>
#include <XSDRAWBase.hxx>

//=======================================================================
//function : SplitFileName
//purpose  : Decomposition of a file name in its parts : prefix, root, suffix
//=======================================================================
static void SplitFileName(const Standard_CString filename,
                          TCollection_AsciiString& prefix,
                          TCollection_AsciiString& fileroot,
                          TCollection_AsciiString& suffix)
{
  Standard_Integer nomdeb, nomfin, nomlon;
  TCollection_AsciiString resfile(filename);
  nomlon = resfile.Length();
  nomdeb = resfile.SearchFromEnd("/");
  if (nomdeb <= 0) nomdeb = resfile.SearchFromEnd("\\");  // pour NT
  if (nomdeb < 0) nomdeb = 0;
  nomfin = resfile.SearchFromEnd(".");
  if (nomfin < nomdeb) nomfin = nomlon + 1;

  if (nomdeb > 0) prefix = resfile.SubString(1, nomdeb);
  fileroot = resfile.SubString(nomdeb + 1, nomfin - 1);
  if (nomfin <= nomlon) suffix = resfile.SubString(nomfin, nomlon);
}

//=======================================================================
//function : GiveList
//purpose  :
//=======================================================================
Handle(TColStd_HSequenceOfTransient) GiveList(const Handle(XSControl_WorkSession)& WS,
                                              const Standard_CString first,
                                              const Standard_CString second)
{
  return WS->GiveList(first, second);
}

//  Function which returns an EVALUATED DISPATCH
//   (could be added in WorkSession.cdl ...)
//  Two modes : returns dispatch as it is, or return with edition
//  Dispatch Name can be : an immediate name of already recorded Dispatch
//  Or a name of dispatch + a parameter :  dispatch-name(param-value)
//  According to type of Dispatch : integer , signature name

//=======================================================================
//function : GiveDispatch
//purpose  :
//=======================================================================
Handle(IFSelect_Dispatch) GiveDispatch(const Handle(XSControl_WorkSession)& WS,
                                       const Standard_CString name,
                                       const Standard_Boolean mode)
{
  DeclareAndCast(IFSelect_Dispatch, disp, WS->NamedItem(name));
  if (!disp.IsNull()) return disp;    // OK as it is given

//   Else, let s try special cases
  TCollection_AsciiString nam(name);
  Standard_Integer paro = nam.Location(1, '(', 1, nam.Length());
  Standard_Integer parf = nam.Location(1, ')', 1, nam.Length());
  nam.SetValue(paro, '\0'); nam.SetValue(parf, '\0');
  if (paro <= 0 && parf <= 0) return disp;
  disp = GetCasted(IFSelect_Dispatch, WS->NamedItem(nam.ToCString()));
  if (disp.IsNull()) return disp;     // KO anyway

//  According to the type of dispatch :
  DeclareAndCast(IFSelect_DispPerCount, dc, disp);
  if (!dc.IsNull())
  {
    Standard_Integer nb = atoi(&(nam.ToCString())[paro]);
    if (nb <= 0)
    {
      Message::SendInfo() << " DispPerCount, count is not positive";
      disp.Nullify();
      return disp;
    }
    if (mode)
    {
      Handle(IFSelect_IntParam) val = new IFSelect_IntParam;
      val->SetValue(nb);
      dc->SetCount(val);
    }
    return dc;
  }
  DeclareAndCast(IFSelect_DispPerFiles, dp, disp);
  if (!dp.IsNull())
  {
    Standard_Integer nb = atoi(&(nam.ToCString())[paro]);
    if (nb <= 0)
    {
      Message::SendInfo() << " DispPerFiles, count is not positive";
      disp.Nullify();
      return disp;
    }
    if (mode)
    {
      Handle(IFSelect_IntParam) val = new IFSelect_IntParam;
      val->SetValue(nb);
      dp->SetCount(val);
    }
    return dp;
  }
  DeclareAndCast(IFSelect_DispPerSignature, ds, disp);
  if (!ds.IsNull())
  {
    DeclareAndCast(IFSelect_Signature, sg, WS->NamedItem(&(nam.ToCString())[paro]));
    if (sg.IsNull())
    {
      Message::SendInfo() << "DispPerSignature " << nam << " , Signature not valid : " << &(nam.ToCString())[paro];
      disp.Nullify();
      return disp;
    }
    if (mode) ds->SetSignCounter(new IFSelect_SignCounter(sg));
    return ds;
  }
  Message::SendInfo() << "Dispatch : " << name << " , Parameter : " << &(nam.ToCString())[paro];
  return disp;
}

//  Functions definit un certain nombre de commandes
//  enregistrees dans le Dictionnaire de Activator (par des Act unitaires)
//  Les actions elles-memes sont regroupees en fin de fichier

//  Les definitions

//=======================================================================
//function : funstatus
//purpose  :
//=======================================================================
static Standard_Integer funstatus(Draw_Interpretor& theDI,
                                  Standard_Integer theNbArgs,
                                  const char** theArgVec)
{
  //        ****    Version & cie     ****
    //#58 rln
  Message::SendInfo() << "Processor Version : " << XSTEP_PROCESSOR_VERSION;
  Message::SendInfo() << "OL Version        : " << XSTEP_SYSTEM_VERSION;
  Message::SendInfo() << "Configuration     : " << XSTEP_Config;
  Message::SendInfo() << "UL Names          : " << XSTEP_ULNames;
  return 0;
}

//=======================================================================
//function : fun1
//purpose  :
//=======================================================================
static Standard_Integer fun1(Draw_Interpretor& theDI,
                             Standard_Integer theNbArgs,
                             const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  //        ****    ToggleHandler     ****
  Standard_Boolean hand = !WS->ErrorHandle();
  if (hand) Message::SendInfo() << " --  Mode Catch Error now Active";
  else      Message::SendInfo() << " --  Mode Catch Error now Inactive";
  WS->SetErrorHandle(hand);
  return 0;
}

//=======================================================================
//function : fun3
//purpose  :
//=======================================================================
static Standard_Integer fun3(Draw_Interpretor& theDI,
                             Standard_Integer theNbArgs,
                             const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  Standard_Integer theNbArgs = theNbArgs;
  const Standard_CString arg1 = theArgVec[1];
  //        ****    XRead / Load         ****
  if (theNbArgs < 2)
  {
    Message::SendInfo() << "Read/Load : give file name !";
    return 1;
  }
  if (WS->Protocol().IsNull())
  {
    Message::SendInfo() << "Protocol not defined";
    return 1;
  }
  if (WS->WorkLibrary().IsNull())
  {
    Message::SendInfo() << "WorkLibrary not defined";
    return 1;
  }

  IFSelect_ReturnStatus status = WS->ReadFile(arg1);
  // status : 0 OK, 1 erreur lecture, 2 Fail(try/catch),
  //          -1 fichier non trouve, -2 lecture faite mais resultat vide
  switch (status)
  {
    case IFSelect_RetVoid: Message::SendInfo() << "file:" << arg1 << " gives empty result"; break;
    case IFSelect_RetError: Message::SendInfo() << "file:" << arg1 << " could not be opened"; break;
    case IFSelect_RetDone: Message::SendInfo() << "file:" << arg1 << " read"; break;
    case IFSelect_RetFail: Message::SendInfo() << "file:" << arg1 << " : error while reading"; break;
    case IFSelect_RetStop: Message::SendInfo() << "file:" << arg1 << " : EXCEPTION while reading"; break;
    default: Message::SendInfo() << "file:" << arg1 << " could not be read"; break;
  }
  if (status != IFSelect_RetDone) return status;
  //      Message::SendInfo()<<" - clearing list of already written files"<<std::endl;
  WS->BeginSentFiles(Standard_True);
  return status;
}

//=======================================================================
//function : fun4
//purpose  :
//=======================================================================
static Standard_Integer fun4(Draw_Interpretor& theDI,
                             Standard_Integer theNbArgs,
                             const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  Standard_Integer theNbArgs = theNbArgs;
  const Standard_CString arg1 = theArgVec[1];
  //        ****    Write All         ****
  if (theNbArgs < 2)
  {
    Message::SendInfo() << "Write All : give file name !";
    return 1;
  }
  return WS->SendAll(arg1);
}

//=======================================================================
//function : fun5
//purpose  :
//=======================================================================
static Standard_Integer fun5(Draw_Interpretor& theDI,
                             Standard_Integer theNbArgs,
                             const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  Standard_Integer theNbArgs = theNbArgs;
  const Standard_CString arg1 = theArgVec[1];
  //  const Standard_CString arg2 = theArgVec[2];
  //        ****    Write Selected         ****
  if (theNbArgs < 3)
  {
    Message::SendInfo() << "Write Selected : give file name + givelist !";
    return 1;
  }
  Handle(TColStd_HSequenceOfTransient) result =
    XSDRAW_FunctionsSession::GiveList(WS, pilot->CommandPart(2));
  if (result.IsNull())
  {
    Message::SendInfo() << "No entity selected";
    return 1;
  }
  else Message::SendInfo() << "Nb Entities selected : " << result->Length();
  Handle(IFSelect_SelectPointed) sp = new IFSelect_SelectPointed;
  sp->SetList(result);
  return WS->SendSelected(arg1, sp);
}

//=======================================================================
//function : fun6
//purpose  :
//=======================================================================
static Standard_Integer fun6(Draw_Interpretor& theDI,
                             Standard_Integer theNbArgs,
                             const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  Standard_Integer theNbArgs = theNbArgs;
  const Standard_CString arg1 = theArgVec[1];
  //        ****    Write Entite(s)         ****
  if (theNbArgs < 3)
  {
    Message::SendInfo() << "Write Entitie(s) : give file name + n0s entitie(s)!";
    return 1;
  }
  int ko = 0;
  Handle(IFSelect_SelectPointed) sp = new IFSelect_SelectPointed;
  for (Standard_Integer ia = 2; ia < theNbArgs; ia++)
  {
    Standard_Integer id = pilot->Number(pilot->Arg(ia));
    if (id > 0)
    {
      Handle(Standard_Transient) item = WS->StartingEntity(id);
      if (sp->Add(item)) Message::SendInfo() << "Added:no." << id;
      else
      {
        Message::SendInfo() << " Fail Add n0." << id; ko++;
      }
    }
    else
    {
      Message::SendInfo() << "Not an entity number:" << pilot->Arg(ia); ko++;
    }
  }
  if (ko > 0)
  {
    Message::SendInfo() << ko << " bad arguments, abandon";
    return 1;
  }
  return WS->SendSelected(arg1, sp);
}

//=======================================================================
//function : fun7
//purpose  :
//=======================================================================
static Standard_Integer fun7(Draw_Interpretor& theDI,
                             Standard_Integer theNbArgs,
                             const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  Standard_Integer theNbArgs = theNbArgs;
  const Standard_CString arg1 = theArgVec[1];
  //        ****    Entity Label       ****
  if (theNbArgs < 2)
  {
    Message::SendInfo() << "Give entity number";
    return 1;
  }
  if (!WS->HasModel())
  {
    Message::SendInfo() << "No loaded model, abandon";
    return 1;
  }
  Standard_Integer nument = WS->NumberFromLabel(arg1);
  if (nument <= 0 || nument > WS->NbStartingEntities())
  {
    Message::SendInfo() << "Not a suitable number: " << arg1;
    return 1;
  }
  Message::SendInfo() << "N0." << nument << " ->Label in Model : ";
  WS->Model()->PrintLabel(WS->StartingEntity(nument), Message::SendInfo());
  Message::SendInfo();
  return 0;
}

//=======================================================================
//function : fun8
//purpose  :
//=======================================================================
static Standard_Integer fun8(Draw_Interpretor& theDI,
                             Standard_Integer theNbArgs,
                             const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  Standard_Integer theNbArgs = theNbArgs;
  const Standard_CString arg1 = theArgVec[1];
  //        ****    Entity Number      ****
  if (theNbArgs < 2)
  {
    Message::SendInfo() << "Give label to search";
    return 1;
  }
  if (!WS->HasModel())
  {
    Message::SendInfo() << "No loaded model, abandon";
    return 1;
  }
  const Handle(Interface_InterfaceModel)& model = WS->Model();
  Standard_Integer i, cnt = 0;
  Standard_Boolean exact = Standard_False;
  Message::SendInfo() << " **  Search Entity Number for Label : " << arg1;
  for (i = model->NextNumberForLabel(arg1, 0, exact); i != 0;
       i = model->NextNumberForLabel(arg1, i, exact))
  {
    cnt++;
    Message::SendInfo() << " **  Found n0/id:";
    model->Print(model->Value(i), Message::SendInfo());
    Message::SendInfo();
  }

  if (cnt == 0) Message::SendInfo() << " **  No Match";
  else if (cnt == 1) Message::SendInfo() << " **  1 Match";
  else Message::SendInfo() << cnt << " Matches";
  return 0;
}

//=======================================================================
//function : fun9
//purpose  :
//=======================================================================
static Standard_Integer fun9(Draw_Interpretor& theDI,
                             Standard_Integer theNbArgs,
                             const char** theArgVec)
{
  //        ****    List Types         ****
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  Handle(IFSelect_Signature) signtype = WS->SignType();
  if (signtype.IsNull()) signtype = new IFSelect_SignType;
  Handle(IFSelect_SignCounter) counter =
    new IFSelect_SignCounter(signtype, Standard_False);
  return pilot->ExecuteCounter(counter, 1);
}

//=======================================================================
//function : funcount
//purpose  :
//=======================================================================
static Standard_Integer funcount(Draw_Interpretor& theDI,
                                 Standard_Integer theNbArgs,
                                 const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  Standard_Integer theNbArgs = theNbArgs;
  const Standard_CString arg0 = theArgVec[0];
  const Standard_CString arg1 = theArgVec[1];
  Standard_Boolean listmode = (arg0[0] == 'l');
  //        ****    List Counter         ****

  if (theNbArgs < 2)
  {
    Message::SendInfo() << "Designer signature ou compteur, + facultatif selection + facultatif entite";
    Message::SendInfo() << " signature/compteur seul -> tout le modele"
      << " sign/compteur + selection -> cette selection, evaluation normale"
      << " sign/compteur + sel + num -> cette selection evaluee sur entite n0 num";
    return 1;
  }
  DeclareAndCast(IFSelect_SignCounter, counter, WS->NamedItem(arg1));
  if (counter.IsNull())
  {
    DeclareAndCast(IFSelect_Signature, signa, WS->NamedItem(arg1));
    if (!signa.IsNull()) counter = new IFSelect_SignCounter(signa, Standard_False, listmode);
  }
  //  Handle(IFSelect_Selection) sel;
  //  Standard_Integer n3 = 0;  if (theNbArgs > 3) n3 = WS->NumberFromLabel(arg3);
  //  if (theNbArgs > 2) sel = GetCasted(IFSelect_Selection,WS->NamedItem(arg2));
  //  if (counter.IsNull() || (theNbArgs > 2 && n3 <= 0 && sel.IsNull()) ) {
  //    Message::SendInfo()<<"Nom:"<<arg1; if (theNbArgs > 2) Message::SendInfo()<<" et/ou "<<arg2;
  //    Message::SendInfo()<<" incorrect (demande: compteur ou signature [selection])"<<std::endl;
  //    return 1;
  //  }

  //  Ajout : si Selection, on applique un GraphCounter
  //   Et en ce cas, on peut en avoir plusieurs : la limite est le mot-cle "on"
  Standard_Integer onflag = 0;
  Standard_Integer i; // svv Jan11 2000 : porting on DEC
  for (i = 2; i < theNbArgs; i++)
  {
    if (!strcmp(theArgVec[i], "on"))
    {
      onflag = i; break;
    }
  }

  Handle(IFSelect_Selection) sel = WS->GiveSelection(arg1);
  DeclareAndCast(IFSelect_SelectDeduct, seld, sel);
  if (!seld.IsNull())
  {
    //  Si onflag, faire une SelectSuite
    if (onflag > 2)
    {
      Handle(IFSelect_SelectSuite) suite = new IFSelect_SelectSuite;
      for (i = 1; i < onflag; i++)
      {
        sel = WS->GiveSelection(theArgVec[i]);
        if (!suite->AddInput(sel))
        {
          Message::SendInfo() << "Incorrect definition for applied selection";
          return 1;
        }
      }
      seld = suite;
    }

    Handle(IFSelect_GraphCounter) gc = new IFSelect_GraphCounter(Standard_False, listmode);
    gc->SetApplied(seld);
    counter = gc;
  }

  if (counter.IsNull())
  {
    Message::SendInfo() << "Neither Counter nor Signature : " << arg1;
    return 1;
  }

  if (onflag == 0) onflag = 1;
  IFSelect_PrintCount pcm = IFSelect_ListByItem;
  if (arg0[0] == 'c') pcm = IFSelect_CountByItem;
  if (arg0[0] == 's') pcm = IFSelect_CountSummary;
  return pilot->ExecuteCounter(counter, onflag + 1, pcm);
}

//=======================================================================
//function : funsigntype
//purpose  :
//=======================================================================
static Standard_Integer funsigntype(Draw_Interpretor& theDI,
                                    Standard_Integer theNbArgs,
                                    const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  Standard_Integer theNbArgs = theNbArgs;
  const Standard_CString arg1 = theArgVec[1];
  //        ****    Sign Type              ****
  Handle(IFSelect_Signature) signtype = WS->SignType();
  if (signtype.IsNull()) Message::SendInfo() << "signtype actually undefined";
  else
  {
    Handle(TCollection_HAsciiString) str = WS->Name(signtype);
    Standard_Integer id = WS->ItemIdent(signtype);
    Message::SendInfo() << signtype->Label();
    if (str.IsNull())
    {
      if (id > 0) Message::SendInfo() << "signtype : item n0 " << id;
    }
    else
    {
      Message::SendInfo() << "signtype : also named as " << str->ToCString();
    }
  }
  if (theNbArgs < 2) Message::SendInfo() << "signtype newitem  to change, signtype . to clear";
  else
  {
    if (arg1[0] == '.' && arg1[1] == '\0')
    {
      signtype.Nullify();
      Message::SendInfo() << "signtype now cleared";
    }
    else
    {
      signtype = GetCasted(IFSelect_Signature, WS->NamedItem(arg1));
      if (signtype.IsNull())
      {
        Message::SendInfo() << "Not a Signature : " << arg1;
        return 1;
      }
      else Message::SendInfo() << "signtype now set to " << arg1;
    }
    WS->SetSignType(signtype);
    return 0;
  }
  return 0;
}

//=======================================================================
//function : funsigncase
//purpose  :
//=======================================================================
static Standard_Integer funsigncase(Draw_Interpretor& theDI,
                                    Standard_Integer theNbArgs,
                                    const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  const Standard_CString arg1 = theArgVec[1];
  //        ****    Sign Case              ****
  Handle(IFSelect_Signature) signcase = GetCasted(IFSelect_Signature, WS->NamedItem(arg1));
  if (signcase.IsNull()) Message::SendInfo() << "Not a Signature : " << arg1;
  else
  {
    Standard_Boolean hasmin, hasmax;  Standard_Integer valmin, valmax;
    if (signcase->IsIntCase(hasmin, valmin, hasmax, valmax))
    {
      Message::SendInfo() << "Signature " << arg1 << " : Integer Case";
      if (hasmin) Message::SendInfo() << " - Mini:" << valmin;
      if (hasmax) Message::SendInfo() << " - Maxi:" << valmax;
      Message::SendInfo();
    }
    Handle(TColStd_HSequenceOfAsciiString) caselist = signcase->CaseList();
    if (caselist.IsNull()) Message::SendInfo() << "Signature " << arg1 << " : no predefined case, see command  count " << arg1;
    else
    {
      Standard_Integer i, nb = caselist->Length();
      Message::SendInfo() << "Signature " << arg1 << " : " << nb << " basic cases :";
      for (i = 1; i <= nb; i++) Message::SendInfo() << "  " << caselist->Value(i);
      Message::SendInfo();
    }
  }
  return 0;
}

//=======================================================================
//function : fun10
//purpose  :
//=======================================================================
static Standard_Integer fun10(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  Standard_Integer theNbArgs = theNbArgs;
  const Standard_CString arg1 = theArgVec[1];
  //        ****    Entity Status          ****
  Standard_Integer i, nb;
  if (theNbArgs < 2)
  {
    nb = Interface_Category::NbCategories();
    Message::SendInfo() << " Categories defined :" << nb << " i.e. :\n";
    for (i = 0; i <= nb; i++)
      Message::SendInfo() << "Cat." << i << "  : " << Interface_Category::Name(i) << "\n";
    Message::SendInfo() << " On a given entity : give its number";
    return 0;
  }
  Standard_Integer num = pilot->Number(arg1);
  if (num <= 0 || num > WS->NbStartingEntities())
  {
    Message::SendInfo() << "Not a suitable entity number : " << arg1;
    return 1;
  }
  Handle(Standard_Transient) ent = WS->StartingEntity(num);
  WS->PrintEntityStatus(ent, Message::SendInfo());
  return 0;
}

//=======================================================================
//function : fun11
//purpose  :
//=======================================================================
static Standard_Integer fun11(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  //  Standard_Integer theNbArgs = theNbArgs;
  const Standard_CString arg1 = theArgVec[1];
  //        ****    DumpModel (Data)  ****
  Standard_Integer niv = 0;
  //  char arg10 = arg1[0];
  //  if (theNbArgs < 2) arg10 = '?';
  switch (arg1[0])
  {
    case '?':
      Message::SendInfo() << "? for this help, else give a listing mode (first letter suffices) :\n"
        << " general    General Statistics\n roots    Roots\n"
        << " entities   All Entities\n"
        << " listfails  CheckList (fails)    per entity\n"
        << " messages   CheckList (complete) per entity\n"
        << " fails      CheckList (fails)    per message (counting)\n"
        << " check      CheckList (complete) per message (counting)\n"
        << " totalcheck CheckList (complete) per message (listing n0 ents)\n"
        << " FAILS      CheckList (fails)    per message (listing complete)\n"
        << " TOTALCHECK CheckList (complete) per message (listing complete)";
      return 0;
    case 'g': niv = 0; break;
    case 'r': niv = 1; break;
    case 'e': niv = 2; break;
    case 'l': niv = 3; break;
    case 'm': niv = 4; break;
    case 'c': niv = 5; break;
    case 't': niv = 6; break;
    case 'T': niv = 7; break;
    case 'f': niv = 8; break;
    case 'F': niv = 10; break;
    default: Message::SendInfo() << "Unknown Mode .  data tout court pour help";
      return 1;
  }
  WS->TraceDumpModel(niv);
  return 0;
}

//=======================================================================
//function : fundumpent
//purpose  :
//=======================================================================
static Standard_Integer fundumpent(Draw_Interpretor& theDI,
                                   Standard_Integer theNbArgs,
                                   const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  Standard_Integer theNbArgs = theNbArgs;
  Handle(IFSelect_WorkLibrary) WL = WS->WorkLibrary();
  Standard_Integer levdef = 0, levmax = 10, level;
  WL->DumpLevels(levdef, levmax);
  if (theNbArgs < 2 || (theNbArgs == 2 && levmax < 0))
  {
    Message::SendInfo() << "Give n0 or id of entity";
    if (levmax < 0) Message::SendInfo() << "  and dump level";
    else Message::SendInfo() << "  + optional, dump level in [0 - " << levmax << "] , default = " << levdef;
    for (level = 0; level <= levmax; level++)
    {
      Standard_CString help = WL->DumpHelp(level);
      if (help[0] != '\0') Message::SendInfo() << level << " : " << help;
    }
    return 1;
  }

  const Standard_CString arg1 = theArgVec[1];
  const Standard_CString arg2 = theArgVec[2];
  Standard_Integer num = pilot->Number(arg1);
  if (num == 0)
    return 1;
  level = levdef;
  if (theNbArgs > 2) level = atoi(arg2);
  Handle(Standard_Transient) ent = WS->StartingEntity(num);
  if (ent.IsNull())
  {
    Message::SendInfo() << "No entity with given id " << arg1 << " (" << num << ") is found in the current model";
  }
  else
  {
    Message::SendInfo() << "  --   DUMP  Entity n0 " << num << "  level " << level;
    WL->DumpEntity(WS->Model(), WS->Protocol(), ent, Message::SendInfo(), level);

    Interface_CheckIterator chl = WS->CheckOne(ent);
    if (!chl.IsEmpty(Standard_False)) chl.Print(Message::SendInfo(), WS->Model(), Standard_False);
  }
  //  Message::SendInfo() << std::flush;

  return 0;
}

//=======================================================================
//function : funsign
//purpose  :
//=======================================================================
static Standard_Integer funsign(Draw_Interpretor& theDI,
                                Standard_Integer theNbArgs,
                                const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  Standard_Integer theNbArgs = theNbArgs;
  const Standard_CString arg1 = theArgVec[1];
  const Standard_CString arg2 = theArgVec[2];
  if (theNbArgs < 3)
  {
    Message::SendInfo() << " Give signature name + n0 or id of entity";
    return 1;
  }
  DeclareAndCast(IFSelect_Signature, sign, WS->NamedItem(arg1));
  if (sign.IsNull())
  {
    Message::SendInfo() << "Not a signature : " << arg1;
    return 1;
  }
  Standard_Integer num = pilot->Number(arg2);
  Handle(Standard_Transient) ent = WS->StartingEntity(num);
  if (num == 0)
    return 1;
  Message::SendInfo() << "Entity n0 " << num << " : " << WS->SignValue(sign, ent);
  return 0;
}

//=======================================================================
//function : funqp
//purpose  :
//=======================================================================
static Standard_Integer funqp(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  Standard_Integer theNbArgs = theNbArgs;
  const Standard_CString arg1 = theArgVec[1];
  const Standard_CString arg2 = theArgVec[2];
  if (theNbArgs < 3)
  {
    Message::SendInfo() << " Give 2 numeros or labels : dad son";
    return 1;
  }
  Standard_Integer n1 = WS->NumberFromLabel(arg1);
  Standard_Integer n2 = WS->NumberFromLabel(arg2);
  Message::SendInfo() << "QueryParent for dad:" << arg1 << ":" << n1 << " and son:" << arg2 << ":" << n2;
  Standard_Integer qp = WS->QueryParent(WS->StartingEntity(n1), WS->StartingEntity(n2));
  if (qp < 0) Message::SendInfo() << arg1 << " is not super-entity of " << arg2;
  else if (qp == 0) Message::SendInfo() << arg1 << " is same as " << arg2;
  else Message::SendInfo() << arg1 << " is super-entity of " << arg2 << " , max level found=" << qp;
  //  Message::SendInfo()<<" Trouve "<<qp<<std::endl;
  return 0;
}

//=======================================================================
//function : fun12
//purpose  :
//=======================================================================
static Standard_Integer fun12(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  //        ****    DumpShare         ****
  WS->DumpShare(); return 0;
}

//=======================================================================
//function : fun13
//purpose  :
//=======================================================================
static Standard_Integer fun13(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  //        ****    ListItems         ****
  WS->ListItems(theArgVec[1]); return 0;
}

//=======================================================================
//function : fun14
//purpose  :
//=======================================================================
static Standard_Integer fun14(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  Standard_Integer theNbArgs = theNbArgs;
  const Standard_CString arg1 = theArgVec[1];
  //        ****    NewInt            ****
  if (theNbArgs < 1)
  {
    Message::SendInfo() << "Donner la valeur entiere pour IntParam";
    return 1;
  }
  Handle(IFSelect_IntParam) intpar = new IFSelect_IntParam;
  if (theNbArgs >= 1)       intpar->SetValue(atoi(arg1));
  return pilot->RecordItem(intpar);
}

//=======================================================================
//function : fun15
//purpose  :
//=======================================================================
static Standard_Integer fun15(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  Standard_Integer theNbArgs = theNbArgs;
  const Standard_CString arg1 = theArgVec[1];
  const Standard_CString arg2 = theArgVec[2];
  //        ****    SetInt            ****
  if (theNbArgs < 3)
  {
    Message::SendInfo() << "Donner 2 arguments : nom Parametre et Valeur";
    return 1;
  }
  Standard_Integer val = atoi(arg2);
  DeclareAndCast(IFSelect_IntParam, par, WS->NamedItem(arg1));
  if (!WS->SetIntValue(par, val))
    return 1;
  return 0;
}

//=======================================================================
//function : fun16
//purpose  :
//=======================================================================
static Standard_Integer fun16(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  Standard_Integer theNbArgs = theNbArgs;
  const Standard_CString arg1 = theArgVec[1];
  //        ****    NewText           ****
  if (theNbArgs < 1)
  {
    Message::SendInfo() << "Donner la valeur texte pour TextParam";
    return 1;
  }
  Handle(TCollection_HAsciiString) textpar = new TCollection_HAsciiString();
  if (theNbArgs >= 1) textpar->AssignCat(arg1);
  return pilot->RecordItem(textpar);
}

//=======================================================================
//function : fun17
//purpose  :
//=======================================================================
static Standard_Integer fun17(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  Standard_Integer theNbArgs = theNbArgs;
  const Standard_CString arg1 = theArgVec[1];
  const Standard_CString arg2 = theArgVec[2];
  //        ****    SetText           ****
  if (theNbArgs < 3)
  {
    Message::SendInfo() << "Donner 2 arguments : nom Parametre et Valeur";
    return 1;
  }
  DeclareAndCast(TCollection_HAsciiString, par, WS->NamedItem(arg1));
  if (!WS->SetTextValue(par, arg2))
    return 1;
  return 0;
}

//=======================================================================
//function : fun19
//purpose  :
//=======================================================================
static Standard_Integer fun19(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  Standard_Integer theNbArgs = theNbArgs;
  const Standard_CString arg1 = theArgVec[1];
  //        ****    DumpSel           ****
  if (theNbArgs < 2)
  {
    Message::SendInfo() << "Give 1 argument : Selection Name";
    return 1;
  }
  WS->DumpSelection(GetCasted(IFSelect_Selection, WS->NamedItem(arg1)));
  return 0;
}

//=======================================================================
//function : fun20
//purpose  :
//=======================================================================
static Standard_Integer fun20(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  Standard_Integer theNbArgs = theNbArgs;
  //        ****    EvalSel           ****
  //        ****    GiveList          ****
  //        ****    GiveShort GivePointed  ****
  //        ****    MakeList          ****
  char mode = theArgVec[0][0];  // givelist/makelist
  if (mode == 'g') mode = theArgVec[0][4];  // l list  s short  p pointed
  if (theNbArgs < 2)
  {
    Message::SendInfo() << "Give Entity ID, or Selection Name [+ optional other selection or entity]";
    return 1;
  }

  //    MakeList : sur Pointed existante ou a creer
  Handle(IFSelect_SelectPointed) pnt;
  if (mode == 'm')
  {
    const Standard_CString arg1 = theArgVec[1];
    Handle(Standard_Transient) item = WS->NamedItem(arg1);
    pnt = GetCasted(IFSelect_SelectPointed, item);
    if (!pnt.IsNull())
    {
      Message::SendInfo() << arg1 << ":Already existing Selection for List, cleared then filled";
      pnt->Clear();
    }
    else if (!item.IsNull())
    {
      Message::SendInfo() << arg1 << ":Already existing Item not for a List, command ignored";
      return 1;
    }
    else
    {
      pnt = new IFSelect_SelectPointed;
      WS->AddNamedItem(arg1, pnt);
    }
  }

  Handle(TColStd_HSequenceOfTransient) result =
    XSDRAW_FunctionsSession::GiveList(WS, pilot->CommandPart((mode == 'm' ? 2 : 1)));
  if (result.IsNull())
    return 1;
  Interface_EntityIterator iter(result);
  Message::SendInfo() << pilot->CommandPart((mode == 'm' ? 2 : 1)) << " : ";
  if (mode == 'l')   WS->ListEntities(iter, 0, Message::SendInfo());
  else if (mode == 's' || mode == 'm') WS->ListEntities(iter, 2, Message::SendInfo());
  else if (mode == 'p')
  {
    Message::SendInfo() << iter.NbEntities() << " Entities : ";
    for (iter.Start(); iter.More(); iter.Next())
      Message::SendInfo() << " +" << WS->StartingNumber(iter.Value());
    Message::SendInfo();
  }

  if (!pnt.IsNull())
  {
    pnt->SetList(result);
    Message::SendInfo() << "List set to a SelectPointed : " << theArgVec[1];
    Message::SendInfo() << "Later editable by command setlist";
  }

  return 0;
}

//=======================================================================
//function : fun20c
//purpose  :
//=======================================================================
static Standard_Integer fun20c(Draw_Interpretor& theDI,
                               Standard_Integer theNbArgs,
                               const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  Standard_Integer theNbArgs = theNbArgs;
  //        ****    GiveCount         ****
  if (theNbArgs < 2)
  {
    Message::SendInfo() << "Give Entity ID, or Selection Name [+ optional other selection or entity]";
    return 1;
  }
  //  WS->EvaluateSelection(GetCasted(IFSelect_Selection,WS->NamedItem(arg1)));
  Handle(TColStd_HSequenceOfTransient) result =
    XSDRAW_FunctionsSession::GiveList(WS, pilot->CommandPart(1));
  if (result.IsNull())
    return 1;
  Message::SendInfo() << pilot->CommandPart(1) << " : List of " << result->Length() << " Entities";
  return 0;
}

//=======================================================================
//function : funselsuite
//purpose  :
//=======================================================================
static Standard_Integer funselsuite(Draw_Interpretor& theDI,
                                    Standard_Integer theNbArgs,
                                    const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  Standard_Integer theNbArgs = theNbArgs;
  //        ****    SelSuite         ****
  if (theNbArgs < 2)
  {
    Message::SendInfo() << "Give Entity ID, or Selection Name [+ optional other selection or entity]";
    return 1;
  }
  //  WS->EvaluateSelection(GetCasted(IFSelect_Selection,WS->NamedItem(arg1)));
  Handle(IFSelect_SelectSuite) selsuite = new IFSelect_SelectSuite;

  for (Standard_Integer i = 1; i < theNbArgs; i++)
  {
    Handle(IFSelect_Selection) sel = WS->GiveSelection(theArgVec[i]);
    if (!selsuite->AddInput(sel))
    {
      Message::SendInfo() << pilot->Arg(i - 1) << " : not a SelectDeduct, no more can be added. Abandon";
      return 1;
    }
  }
  selsuite->SetLabel(pilot->CommandPart(1));
  return pilot->RecordItem(selsuite);
}

//=======================================================================
//function : fun21
//purpose  :
//=======================================================================
static Standard_Integer fun21(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  //        ****    ClearItems           ****
  WS->ClearItems();  WS->ClearFinalModifiers();  WS->ClearShareOut(Standard_False);
  return 0;
}

static Standard_Integer fun22(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  Standard_Integer theNbArgs = theNbArgs;
  const Standard_CString arg1 = theArgVec[1];
  //        ****    ClearData           ****
  Standard_Integer mode = -1;
  if (theNbArgs >= 2)
  {
    if (arg1[0] == 'a') mode = 1;
    if (arg1[0] == 'g') mode = 2;
    if (arg1[0] == 'c') mode = 3;
    if (arg1[0] == 'p') mode = 4;
    if (arg1[0] == '?') mode = -1;
  }
  else mode = 0;
  if (mode <= 0)
  {
    if (mode < 0) Message::SendInfo() << "Give a suitable mode";
    Message::SendInfo() << "  Available Modes :\n"
      << " a : all data    g : graph+check  c : check  p : selectpointed";
    return (mode < 0 ? IFSelect_RetError : IFSelect_RetVoid);
  }
  WS->ClearData(mode);
  return 0;
}

//=======================================================================
//function : fun24
//purpose  :
//=======================================================================
static Standard_Integer fun24(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  Standard_Integer theNbArgs = theNbArgs;
  //        ****    Item Label         ****
  TCollection_AsciiString label;
  if (theNbArgs < 2)
  {
    Message::SendInfo() << " Give  label to search";
    return 1;
  }
  for (int i = 1; i < theNbArgs; i++)
  {
    label.AssignCat(theArgVec[i]);
    if (i < theNbArgs - 1) label.AssignCat(" ");
  }
  for (int mode = 0; mode <= 2; mode++)
  {
    int nbitems = 0;  int id;
    Message::SendInfo() << "Searching label : " << label << ". in mode ";
    if (mode == 0) Message::SendInfo() << " exact";
    if (mode == 1) Message::SendInfo() << " same head";
    if (mode == 2) Message::SendInfo() << " search if present";
    for (id = WS->NextIdentForLabel(label.ToCString(), 0, mode); id != 0;
         id = WS->NextIdentForLabel(label.ToCString(), id, mode))
    {
      Message::SendInfo() << " " << id;  nbitems++;
    }
    Message::SendInfo() << " -- giving " << nbitems << " found";
  }
  return 0;
}

//=======================================================================
//function : fun25
//purpose  :
//=======================================================================
static Standard_Integer fun25(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  Standard_Integer theNbArgs = theNbArgs;
  const Standard_CString arg1 = theArgVec[1];
  //        ****    Save (Dump)       ****
  if (theNbArgs < 2)
  {
    Message::SendInfo() << "Donner nom du Fichier";
    return 1;
  }
  IFSelect_SessionFile dumper(WS, arg1);
  if (!dumper.IsDone())
    return 1;
  return 0;
}

//=======================================================================
//function : fun26
//purpose  :
//=======================================================================
static Standard_Integer fun26(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  Standard_Integer theNbArgs = theNbArgs;
  const Standard_CString arg1 = theArgVec[1];
  //        ****    Restore (Dump)    ****
  if (theNbArgs < 2)
  {
    Message::SendInfo() << "Donner nom du Fichier";
    return 1;
  }
  IFSelect_SessionFile dumper(WS);
  Standard_Integer readstat = dumper.Read(arg1);
  if (readstat == 0) return 0;
  else if (readstat > 0) Message::SendInfo() << "-- Erreur Lecture Fichier " << arg1;
  else                    Message::SendInfo() << "-- Pas pu ouvrir Fichier " << arg1;
  return 0;
}

//=======================================================================
//function : fun27
//purpose  :
//=======================================================================
static Standard_Integer fun27(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  Standard_Integer theNbArgs = theNbArgs;
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  const Standard_CString arg1 = theArgVec[1];
  Standard_CString arg2 = theArgVec[2];
  const Standard_CString anEmptyStr = "";
  if (arg2 && strlen(arg2) == 2 && arg2[0] == '"' && arg2[1] == '"')
  {
    arg2 = anEmptyStr;
  }
  //        ****    Param(Value)         ****
  if (theNbArgs < 2 || (theNbArgs == 3 && strcmp(arg1, "-p") == 0))
  {
    Handle(TColStd_HSequenceOfHAsciiString) li = Interface_Static::Items();
    Standard_Integer i, nb = li->Length(), aPatternNb = 0;
    size_t aPatternLen = strlen(arg2);
    if (theNbArgs == 3)
    {
      for (i = 1; i <= nb; i++)
      {
        if (strncmp(li->Value(i)->String().ToCString(), arg2, aPatternLen) == 0)
        {
          aPatternNb++;
        }
      }
    }
    else
    {
      aPatternNb = nb;
    }
    Message::SendInfo() << " List of parameters : " << aPatternNb << " items : ";
    for (i = 1; i <= nb; i++)
    {
      if (theNbArgs == 3 && strncmp(li->Value(i)->String().ToCString(), arg2, aPatternLen) != 0)
      {
        continue;
      }
      Message::SendInfo() << li->Value(i)->String();
      Message::SendInfo() << " : " << Interface_Static::CVal(li->Value(i)->ToCString());
    }
    return 0;
  }
  else if (atoi(arg1) > 0)
  {
    Standard_Integer use = atoi(arg1);
    WS->TraceStatics(use);
  }
  else
  {
    if (theNbArgs > 2) Message::SendInfo() << "     FORMER STATUS of Static Parameter " << arg1;
    else          Message::SendInfo() << "     ACTUAL STATUS of Static Parameter " << arg1;
    if (!Interface_Static::IsPresent(arg1))
    {
      Message::SendInfo() << " Parameter " << arg1 << " undefined";
      return 1;
    }
    if (!Interface_Static::IsSet(arg1)) Message::SendInfo() << " Parameter " << arg1 << " not valued";
    else if (theNbArgs == 2) Interface_Static::Static(arg1)->Print(Message::SendInfo());
    else Message::SendInfo() << " Value : " << Interface_Static::CVal(arg1);

    if (theNbArgs == 2) Message::SendInfo() << "To modify, param name_param new_val";
    else
    {
      if (strlen(arg2) != 0)
      {
        Message::SendInfo() << " New demanded value : " << arg2;
      }
      else
      {
        Message::SendInfo() << " New demanded value : not valued";
      }
      if (Interface_Static::SetCVal(arg1, arg2))
      {
        Message::SendInfo() << "   OK";
        return 0;
      }
      else
      {
        Message::SendInfo() << " , refused";
        return 1;
      }
    }
  }
  return 0;
}

//=======================================================================
//function : fun29
//purpose  :
//=======================================================================
static Standard_Integer fun29(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  //        ****    SentFiles         ****
  Handle(TColStd_HSequenceOfHAsciiString) list = WS->SentFiles();
  if (list.IsNull())
  {
    Message::SendInfo() << "List of Sent Files not enabled"; return 0;
  }
  Standard_Integer i, nb = list->Length();
  Message::SendInfo() << "  Sent Files : " << nb << " : ";
  for (i = 1; i <= nb; i++)
    Message::SendInfo() << list->Value(i)->ToCString();
  return 0;
}

//=======================================================================
//function : fun30
//purpose  :
//=======================================================================
static Standard_Integer fun30(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  Standard_Integer theNbArgs = theNbArgs;
  const Standard_CString arg1 = theArgVec[1];
  //        ****    FilePrefix        ****
  if (theNbArgs < 2)
  {
    if (WS->FilePrefix().IsNull()) Message::SendInfo() << "Pas de prefixe defini";
    else Message::SendInfo() << "Prefixe : " << WS->FilePrefix()->ToCString();
    Message::SendInfo() << "Pour changer :  filepref newprefix";
    return 0;
  }
  WS->SetFilePrefix(arg1);
  return 0;
}

//=======================================================================
//function : fun31
//purpose  :
//=======================================================================
static Standard_Integer fun31(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  Standard_Integer theNbArgs = theNbArgs;
  const Standard_CString arg1 = theArgVec[1];
  //        ****    FileExtension     ****
  if (theNbArgs < 2)
  {
    if (WS->FileExtension().IsNull()) Message::SendInfo() << "Pas d extension definie";
    else Message::SendInfo() << "Extension : " << WS->FileExtension()->ToCString();
    Message::SendInfo() << "Pour changer :  fileext newext";
    return 0;
  }
  WS->SetFileExtension(arg1);
  return 0;
}

//=======================================================================
//function : fun32
//purpose  :
//=======================================================================
static Standard_Integer fun32(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  Standard_Integer theNbArgs = theNbArgs;
  const Standard_CString arg1 = theArgVec[1];
  const Standard_CString arg2 = theArgVec[2];
  //        ****    FileRoot          ****
  if (theNbArgs < 2)
  {
    Message::SendInfo() << "Donner Dispatch et nom de Root";
    return 1;
  }
  DeclareAndCast(IFSelect_Dispatch, disp, WS->NamedItem(arg1));
  if (theNbArgs < 3)
  {
    if (WS->FileRoot(disp).IsNull()) Message::SendInfo() << "Pas de racine definie pour " << arg1;
    else Message::SendInfo() << "Racine pour " << arg1 << " : " << WS->FileRoot(disp)->ToCString();
    Message::SendInfo() << "Pour changer :  fileroot nomdisp newroot";
    return 0;
  }
  if (!WS->SetFileRoot(disp, arg2))
    return 1;
  return 0;
}

//=======================================================================
//function : fun33
//purpose  :
//=======================================================================
static Standard_Integer fun33(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  Standard_Integer theNbArgs = theNbArgs;
  const Standard_CString arg1 = theArgVec[1];
  //        ****    Default File Root     ****
  if (theNbArgs < 2)
  {
    if (WS->DefaultFileRoot().IsNull()) Message::SendInfo() << "Pas de racine par defaut definie";
    else Message::SendInfo() << "Racine par defaut : " << WS->DefaultFileRoot()->ToCString();
    Message::SendInfo() << "Pour changer :  filedef newdef";
    return 0;
  }
  WS->SetDefaultFileRoot(arg1);
  return 0;
}

//=======================================================================
//function : fun34
//purpose  :
//=======================================================================
static Standard_Integer fun34(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  //        ****    EvalFile          ****
  if (!WS->HasModel())
  {
    Message::SendInfo() << "Pas de Modele charge, abandon";
    return 1;
  }

  Message::SendInfo() << "Evaluation avec Memorisation des resultats";
  WS->EvaluateFile();
  Standard_Integer nbf = WS->NbFiles();
  for (Standard_Integer i = 1; i <= nbf; i++)
  {
    Handle(Interface_InterfaceModel) mod = WS->FileModel(i);
    if (mod.IsNull())
    {
      Message::SendInfo() << "Modele " << i << " Model non genere ..."; continue;
    }
    TCollection_AsciiString name = WS->FileName(i);
    Message::SendInfo() << "Fichier n0 " << i << " Nb Entites : " << mod->NbEntities() << "  Nom: ";
    Message::SendInfo() << name;
  }
  return 0;
}

//=======================================================================
//function : fun35
//purpose  :
//=======================================================================
static Standard_Integer fun35(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  //        ****    ClearFile          ****
  WS->ClearFile();
  return 0;
}

//=======================================================================
//function : fun36
//purpose  :
//=======================================================================
static Standard_Integer fun36(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  Standard_Integer theNbArgs = theNbArgs;
  //        ****    Split              ****
  IFSelect_ReturnStatus stat = IFSelect_RetVoid;
  if (theNbArgs < 2) Message::SendInfo() << "Split : derniere liste de dispatches definie";
  else
  {
    WS->ClearShareOut(Standard_True);
    for (Standard_Integer i = 1; i < theNbArgs; i++)
    {
      DeclareAndCast(IFSelect_Dispatch, disp, WS->NamedItem(theArgVec[i]));
      if (disp.IsNull())
      {
        Message::SendInfo() << "Pas un dispatch:" << theArgVec[i] << ", Splitt abandonne";
        stat = IFSelect_RetError;
      }
      else WS->SetActive(disp, Standard_True);
    }
  }
  if (stat == IFSelect_RetError) return stat;
  WS->BeginSentFiles(Standard_True);
  if (!WS->SendSplit())
    return 1;
  return 0;
}

//=======================================================================
//function : fun37
//purpose  :
//=======================================================================
static Standard_Integer fun37(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  Standard_Integer theNbArgs = theNbArgs;
  const Standard_CString arg1 = theArgVec[1];
  //        ****    Remaining Data     ****
  char mode = '?';  IFSelect_RemainMode numod = IFSelect_RemainDisplay;
  if (theNbArgs >= 2) mode = arg1[0];
  if (mode == 'u') numod = IFSelect_RemainUndo;
  else if (mode == 'l') numod = IFSelect_RemainDisplay;
  else if (mode == 'c') numod = IFSelect_RemainCompute;
  else if (mode == 'f') numod = IFSelect_RemainForget;
  else
  {
    if (theNbArgs < 2) Message::SendInfo() << "Donner un Mode - ";
    Message::SendInfo() << "Modes possibles : l  list, c compute, u undo, f forget";
    if (mode == '?') return 0;   else return 1;
  }
  if (!WS->SetRemaining(numod)) return 0;
  return 0;
}

//=======================================================================
//function : fun38
//purpose  :
//=======================================================================
static Standard_Integer fun38(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  Standard_Integer theNbArgs = theNbArgs;
  const Standard_CString arg1 = theArgVec[1];
  const Standard_CString arg2 = theArgVec[2];
  //        ****    SetModelContent    ****
  if (theNbArgs < 3)
  {
    Message::SendInfo() << "Donner nom selection et mode (k=keep,r=remove)";
    return 1;
  }
  Standard_Boolean keepmode;
  DeclareAndCast(IFSelect_Selection, sel, WS->NamedItem(arg1));
  if (sel.IsNull())
  {
    Message::SendInfo() << "Pas de Selection de Nom : " << arg1;
    return 1;
  }
  if (arg2[0] == 'k')
  {
    Message::SendInfo() << " -- SetContent keep ..."; keepmode = Standard_True;
  }
  else if (arg2[0] == 'r')
  {
    Message::SendInfo() << " -- SetContent remove ..."; keepmode = Standard_False;
  }
  else
  {
    Message::SendInfo() << "Donner nom selection et mode (k=keep,r=remove)";
    return 1;
  }

  if (WS->SetModelContent(sel, keepmode)) Message::SendInfo() << " Done";
  else Message::SendInfo() << " Result empty, ignored";
  return 0;
}

//=======================================================================
//function : fun40
//purpose  :
//=======================================================================
static Standard_Integer fun40(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  //        ****    ListModif          ****
  WS->ListFinalModifiers(Standard_True);
  WS->ListFinalModifiers(Standard_False); return 0;
}

//=======================================================================
//function : fun41
//purpose  :
//=======================================================================
static Standard_Integer fun41(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  Standard_Integer theNbArgs = theNbArgs;
  const Standard_CString arg1 = theArgVec[1];
  //        ****    Modifier           ****
  if (theNbArgs < 2)
  {
    Message::SendInfo() << "Donner Nom du Modifier";
    return 1;
  }
  DeclareAndCast(IFSelect_GeneralModifier, modif, WS->NamedItem(arg1));
  if (modif.IsNull())
  {
    Message::SendInfo() << "Pas de Modifier de Nom : " << arg1; return 0;
  }
  Handle(IFSelect_IntParam) low, up;

  Handle(IFSelect_Dispatch) disp = modif->Dispatch();
  Message::SendInfo() << "Modifier : " << arg1 << " Label : " << modif->Label();
  Standard_Integer rank = WS->ModifierRank(modif);
  if (modif->IsKind(STANDARD_TYPE(IFSelect_Modifier)))
    Message::SendInfo() << "Model Modifier n0." << rank;
  else Message::SendInfo() << "File Modifier n0." << rank;
  if (disp.IsNull()) Message::SendInfo() << "  Applique a tous les Dispatchs";
  else
  {
    Message::SendInfo() << "  Dispatch : " << disp->Label();
    if (WS->HasName(disp)) Message::SendInfo() << " - Nom:" << WS->Name(disp)->ToCString();
    Message::SendInfo();
  }

  Handle(IFSelect_Selection) sel = modif->Selection();
  if (!sel.IsNull()) Message::SendInfo() << "  Selection : " << sel->Label();
  if (WS->HasName(sel)) Message::SendInfo() << " - Nom:" << WS->Name(sel)->ToCString();
  Message::SendInfo();
  return 0;
}

//=======================================================================
//function : fun42
//purpose  :
//=======================================================================
static Standard_Integer fun42(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  Standard_Integer theNbArgs = theNbArgs;
  const Standard_CString arg1 = theArgVec[1];
  const Standard_CString arg2 = theArgVec[2];
  //        ****    ModifSel           ****
  if (theNbArgs < 2)
  {
    Message::SendInfo() << "Donner Nom Modifier; + Nom Selection optionnel\n"
      << "Selection pour Mettre une Selection, sinon Annule";
    return 1;
  }
  DeclareAndCast(IFSelect_GeneralModifier, modif, WS->NamedItem(arg1));
  if (modif.IsNull())
  {
    Message::SendInfo() << "Pas un nom de Modifier : " << arg1;
    return 1;
  }
  Handle(IFSelect_Selection) sel;
  if (arg2[0] != '\0')
  {
    sel = GetCasted(IFSelect_Selection, WS->NamedItem(arg2));
    if (sel.IsNull())
    {
      Message::SendInfo() << "Pas un nom de Selection : " << arg2;
      return 1;
    }
  }
  if (!WS->SetItemSelection(modif, sel))
    return 1;
  return 0;
}

//=======================================================================
//function : fun43
//purpose  :
//=======================================================================
static Standard_Integer fun43(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  Standard_Integer theNbArgs = theNbArgs;
  const Standard_CString arg1 = theArgVec[1];
  const Standard_CString arg2 = theArgVec[2];
  //        ****    SetAppliedModifier           ****
  if (theNbArgs < 2)
  {
    Message::SendInfo() << "Donner Nom Modifier; + Nom Dispatch ou Transformer optionnel :\n"
      << " - rien : tous Dispatches\n - Dispatch : ce Dispatch seul\n"
      << " - Transformer : pas un Dispatch mais un Transformer";
    return 1;
  }
  DeclareAndCast(IFSelect_GeneralModifier, modif, WS->NamedItem(arg1));
  if (modif.IsNull())
  {
    Message::SendInfo() << "Pas un nom de Modifier : " << arg1;
    return 1;
  }
  Handle(Standard_Transient) item;
  if (arg2[0] != '\0')
  {
    item = WS->NamedItem(arg2);
    if (item.IsNull())
    {
      Message::SendInfo() << "Pas un nom connu : " << arg2;
      return 1;
    }
  }
  else item = WS->ShareOut();
  if (!WS->SetAppliedModifier(modif, item))
    return 1;
  return 0;
}

//=======================================================================
//function : fun44
//purpose  :
//=======================================================================
static Standard_Integer fun44(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  Standard_Integer theNbArgs = theNbArgs;
  const Standard_CString arg1 = theArgVec[1];
  //        ****    ResetApplied (modifier)    ****
  if (theNbArgs < 2)
  {
    Message::SendInfo() << "Designer un modifier";
    return 1;
  }
  DeclareAndCast(IFSelect_GeneralModifier, modif, WS->NamedItem(arg1));
  if (modif.IsNull())
  {
    Message::SendInfo() << "Pas un nom de Modifier : " << arg1;
    return 1;
  }
  if (!WS->ResetAppliedModifier(modif))
    return 1;
  return 0;
}

//=======================================================================
//function : fun45
//purpose  :
//=======================================================================
static Standard_Integer fun45(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  Standard_Integer theNbArgs = theNbArgs;
  const Standard_CString arg1 = theArgVec[1];
  const Standard_CString arg2 = theArgVec[2];
  const Standard_CString arg3 = theArgVec[3];
  //        ****    ModifMove         ****
  if (theNbArgs < 4)
  {
    Message::SendInfo() << "modifmove MF rang1 rang2, M pour Model F pour File";
    return 1;
  }
  Standard_Boolean formodel;
  if (arg1[0] == 'm' || arg1[0] == 'M') formodel = Standard_True;
  else if (arg1[0] == 'f' || arg1[0] == 'F') formodel = Standard_False;
  else
  {
    Message::SendInfo() << "preciser M pour Model, F pour File";
    return 1;
  }
  Standard_Integer before = atoi(arg2);
  Standard_Integer after = atoi(arg3);
  if (before == 0 || after == 0)
  {
    Message::SendInfo() << "Donner 2 Entiers Positifs";
    return 1;
  }
  if (!WS->ChangeModifierRank(formodel, before, after))
    return 1;
  return 0;
}

//=======================================================================
//function : fun51
//purpose  :
//=======================================================================
static Standard_Integer fun51(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  const Standard_CString arg1 = theArgVec[1];
  const Standard_CString arg2 = theArgVec[2];
  //        ****    DispSel           ****
  if (theNbArgs < 3)
  {
    Message::SendInfo() << "Donner Noms Dispatch et Selection Finale";
    return 1;
  }
  DeclareAndCast(IFSelect_Dispatch, disp, WS->NamedItem(arg1));
  if (disp.IsNull())
  {
    Message::SendInfo() << "Pas un nom de Dispatch : " << arg1;
    return 1;
  }
  DeclareAndCast(IFSelect_Selection, sel, WS->NamedItem(arg2));
  if (sel.IsNull())
  {
    Message::SendInfo() << "Pas un nom de Selection : " << arg2;
    return 1;
  }
  if (!WS->SetItemSelection(disp, sel))
    return 1;
  return 0;
}

////=======================================================================
////function : fun_dispone
////purpose  :
////=======================================================================
//static Standard_Integer fun_dispone(Draw_Interpretor& theDI,
//                                    Standard_Integer theNbArgs,
//                                    const char** theArgVec)
//{
//  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
//  //        ****    DispOne           ****
//  Handle(IFSelect_DispPerOne) disp = new IFSelect_DispPerOne;
//  return pilot->RecordItem(disp);
//}

////=======================================================================
////function : fun_dispglob
////purpose  :
////=======================================================================
//static Standard_Integer fun_dispglob(Draw_Interpretor& theDI,
//                                     Standard_Integer theNbArgs,
//                                     const char** theArgVec)
//{
//  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
//  //        ****    DispGlob          ****
//  Handle(IFSelect_DispGlobal) disp = new IFSelect_DispGlobal;
//  return pilot->RecordItem(disp);
//}

//=======================================================================
//function : fun_dispcount
//purpose  :
//=======================================================================
static Standard_Integer fun_dispcount(Draw_Interpretor& theDI,
                                      Standard_Integer theNbArgs,
                                      const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  Standard_Integer theNbArgs = theNbArgs;
  const Standard_CString arg1 = theArgVec[1];
  //        ****    DispCount         ****
  if (theNbArgs < 2)
  {
    Message::SendInfo() << "Donner Nom IntParam pour Count";
    return 1;
  }
  DeclareAndCast(IFSelect_IntParam, par, WS->NamedItem(arg1));
  if (par.IsNull())
  {
    Message::SendInfo() << "Pas un nom de IntParam : " << arg1;
    return 1;
  }
  Handle(IFSelect_DispPerCount) disp = new IFSelect_DispPerCount;
  disp->SetCount(par);
  return pilot->RecordItem(disp);
}

//=======================================================================
//function : fun_dispfiles
//purpose  :
//=======================================================================
static Standard_Integer fun_dispfiles(Draw_Interpretor& theDI,
                                      Standard_Integer theNbArgs,
                                      const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  Standard_Integer theNbArgs = theNbArgs;
  const Standard_CString arg1 = theArgVec[1];
  //        ****    DispFiles         ****
  if (theNbArgs < 2)
  {
    Message::SendInfo() << "Donner Nom IntParam pour NbFiles";
    return 1;
  }
  DeclareAndCast(IFSelect_IntParam, par, WS->NamedItem(arg1));
  if (par.IsNull())
  {
    Message::SendInfo() << "Pas un nom de IntParam : " << arg1;
    return 1;
  }
  Handle(IFSelect_DispPerFiles) disp = new IFSelect_DispPerFiles;
  disp->SetCount(par);
  return pilot->RecordItem(disp);
}

//=======================================================================
//function : fun_dispsign
//purpose  :
//=======================================================================
static Standard_Integer fun_dispsign(Draw_Interpretor& theDI,
                                     Standard_Integer theNbArgs,
                                     const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  Standard_Integer theNbArgs = theNbArgs;
  const Standard_CString arg1 = theArgVec[1];
  //        ****    DispFiles         ****
  if (theNbArgs < 2)
  {
    Message::SendInfo() << "Donner Nom Signature";
    return 1;
  }
  DeclareAndCast(IFSelect_Signature, sig, WS->NamedItem(arg1));
  if (sig.IsNull())
  {
    Message::SendInfo() << "Pas un nom de Signature : " << arg1;
    return 1;
  }
  Handle(IFSelect_DispPerSignature) disp = new IFSelect_DispPerSignature;
  disp->SetSignCounter(new IFSelect_SignCounter(sig));
  return pilot->RecordItem(disp);
}

//=======================================================================
//function : fun56
//purpose  :
//=======================================================================
static Standard_Integer fun56(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  Standard_Integer theNbArgs = theNbArgs;
  const Standard_CString arg1 = theArgVec[1];
  //        ****    Dispatch           ****
  if (theNbArgs < 2)
  {
    Message::SendInfo() << "Donner Nom du Dispatch";
    return 1;
  }
  DeclareAndCast(IFSelect_Dispatch, disp, WS->NamedItem(arg1));
  if (disp.IsNull())
  {
    Message::SendInfo() << "Pas un dispatch : " << arg1;
    return 1;
  }
  Standard_Integer num = WS->DispatchRank(disp);
  Message::SendInfo() << "Dispatch de Nom : " << arg1 << " , en ShareOut, Numero " << num << " : ";
  Handle(IFSelect_Selection) sel = WS->ItemSelection(disp);
  Handle(TCollection_HAsciiString) selname = WS->Name(sel);
  if (sel.IsNull())  Message::SendInfo() << "Pas de Selection Finale";
  else if (selname.IsNull()) Message::SendInfo() << "Selection Finale : #" << WS->ItemIdent(sel);
  else Message::SendInfo() << "Selection Finale : " << selname->ToCString();
  if (disp->HasRootName()) Message::SendInfo() << "-- Racine nom de fichier : "
    << disp->RootName()->ToCString();
  return 0;
}

//=======================================================================
//function : fun57
//purpose  :
//=======================================================================
static Standard_Integer fun57(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  Standard_Integer theNbArgs = theNbArgs;
  const Standard_CString arg1 = theArgVec[1];
  //        ****    Remove           ****
  if (theNbArgs < 2)
  {
    Message::SendInfo() << "Give Name to Remove !";
    return 1;
  }
  if (!WS->RemoveNamedItem(arg1))
    return 1;
  return 0;
}

//=======================================================================
//function : fun58
//purpose  :
//=======================================================================
static Standard_Integer fun58(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  Standard_Integer theNbArgs = theNbArgs;
  const Standard_CString arg1 = theArgVec[1];
  //        ****    EvalDisp          ****
  if (theNbArgs < 3)
  {
    Message::SendInfo() << "evaldisp mode disp [disp ...] :  Mode + Name(s) of Dispatch(es). Mode:\n"
      << "  0 brief  1 +forgotten ents  2 +duplicata  3 1+2"
      << "See also : evaladisp  writedisp  xsplit";
    return 0;
  }
  Standard_Boolean OK = Standard_True;
  Standard_Integer i, mode = atoi(arg1);  Message::SendInfo() << " Mode " << mode << "\n";
  for (i = 2; i < theNbArgs; i++)
  {
    DeclareAndCast(IFSelect_Dispatch, disp, WS->NamedItem(theArgVec[i]));
    if (disp.IsNull())
    {
      Message::SendInfo() << "Not a dispatch:" << theArgVec[i]; OK = Standard_False;
    }
  }
  if (!OK)
  {
    Message::SendInfo() << "Some of the parameters are not correct";
    return 1;
  }

  WS->ClearShareOut(Standard_True);
  for (i = 2; i < theNbArgs; i++)
  {
    DeclareAndCast(IFSelect_Dispatch, disp, WS->NamedItem(theArgVec[i]));
    WS->SetActive(disp, Standard_True);
  }
  //      WS->EvaluateDispatch(disp,mode);
  WS->EvaluateComplete(mode);
  return 0;
}

//=======================================================================
//function : fun_evaladisp
//purpose  :
//=======================================================================
static Standard_Integer fun_evaladisp(Draw_Interpretor& theDI,
                                      Standard_Integer theNbArgs,
                                      const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  Standard_Integer theNbArgs = theNbArgs;
  const Standard_CString arg1 = theArgVec[1];
  //        ****    EvalADisp [GiveList]         ****
  if (theNbArgs < 3)
  {
    Message::SendInfo() << "evaladisp mode(=0-1-2-3) disp [givelist] :  Mode + Dispatch [+ GiveList]\n  If GiveList not given, computed from Selection of the Dispatch. Mode:\n"
      << "  0 brief  1 +forgotten ents  2 +duplicata  3 1+2"
      << "See also : writedisp";
    return 0;
  }
  if (arg1[1] != '\0')
  {
    Message::SendInfo() << "first parameter : mode, must be a number between 0 and 3";
    return 1;
  }
  Standard_Integer mode = atoi(arg1);  Message::SendInfo() << " Mode " << mode << "\n";
  //  DeclareAndCast(IFSelect_Dispatch,disp,WS->NamedItem(theArgVec[2]));
  Handle(IFSelect_Dispatch) disp = XSDRAW_FunctionsSession::GiveDispatch(WS, theArgVec[2], Standard_True);
  if (disp.IsNull())
  {
    Message::SendInfo() << "Not a dispatch:" << theArgVec[2];
    return 1;
  }
  Handle(IFSelect_Selection) selsav = disp->FinalSelection();
  Handle(IFSelect_Selection) sel;
  if (theNbArgs > 3)
  {
    Handle(IFSelect_SelectPointed) sp = new IFSelect_SelectPointed;
    Handle(TColStd_HSequenceOfTransient) list = XSDRAW_FunctionsSession::GiveList
    (XSDRAWBase::Session(), pilot->CommandPart(3));
    Standard_Integer nb = (list.IsNull() ? 0 : list->Length());
    if (nb > 0)
    {
      sp->AddList(list);  sel = sp;
    }
  }

  if (sel.IsNull() && selsav.IsNull())
  {
    Message::SendInfo() << "No Selection nor GiveList defined";
    return 1;
  }
  if (sel.IsNull() && !selsav.IsNull())
  {
    if (theNbArgs > 3) Message::SendInfo() << "GiveList is empty, hence computed from the Selection of the Dispatch";
    sel = selsav;
  }
  disp->SetFinalSelection(sel);
  //  WS->ClearShareOut(Standard_True);
  //  WS->SetActive(disp,Standard_True);
  WS->EvaluateDispatch(disp, mode);
  disp->SetFinalSelection(selsav);

  return 0;
}

//=======================================================================
//function : fun_writedisp
//purpose  :
//=======================================================================
static Standard_Integer fun_writedisp(Draw_Interpretor& theDI,
                                      Standard_Integer theNbArgs,
                                      const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  Standard_Integer theNbArgs = theNbArgs;
  const Standard_CString arg1 = theArgVec[1];
  //        ****    EvalADisp [GiveList]         ****
  if (theNbArgs < 3)
  {
    Message::SendInfo() << "writedisp filename disp [givelist] :  FileName + Dispatch [+ GiveList]\n  If GiveList not given, computed from Selection of the Dispatch.\n"
      << "FileName : rootname.ext will gives rootname_1.ext etc...\n"
      << "  path/rootname.ext gives  path/rootname_1.ext etc...\n"
      << "See also : evaladisp";
    return 0;
  }
  TCollection_AsciiString prefix, rootname, suffix;
  SplitFileName(arg1, prefix, rootname, suffix);
  if (rootname.Length() == 0 || suffix.Length() == 0)
  {
    Message::SendInfo() << "Empty Root Name or Extension";
    return 1;
  }

  //  DeclareAndCast(IFSelect_Dispatch,disp,WS->NamedItem(theArgVec[2]));
  Handle(IFSelect_Dispatch) disp = XSDRAW_FunctionsSession::GiveDispatch(WS, theArgVec[2], Standard_True);
  if (disp.IsNull())
  {
    Message::SendInfo() << "Not a dispatch:" << theArgVec[2];
    return 1;
  }
  Handle(IFSelect_Selection) selsav = disp->FinalSelection();
  Handle(IFSelect_Selection) sel;
  if (theNbArgs > 3)
  {
    Handle(IFSelect_SelectPointed) sp = new IFSelect_SelectPointed;
    Handle(TColStd_HSequenceOfTransient) list = XSDRAW_FunctionsSession::GiveList
    (XSDRAWBase::Session(), pilot->CommandPart(3));
    Standard_Integer nb = (list.IsNull() ? 0 : list->Length());
    if (nb > 0)
    {
      sp->AddList(list);  sel = sp;
    }
  }

  if (sel.IsNull() && selsav.IsNull())
  {
    Message::SendInfo() << "No Selection nor GiveList defined";
    return 1;
  }
  if (sel.IsNull() && !selsav.IsNull())
  {
    if (theNbArgs > 3) Message::SendInfo() << "GiveList is empty, hence computed from the Selection of the Dispatch";
    sel = selsav;
  }

  WS->ClearShareOut(Standard_True);
  disp->SetFinalSelection(sel);
  WS->SetActive(disp, Standard_True);
  WS->BeginSentFiles(Standard_True);

  WS->SetFilePrefix(prefix.ToCString());
  WS->SetFileExtension(suffix.ToCString());
  WS->SetFileRoot(disp, rootname.ToCString());

  Standard_Boolean OK = WS->SendSplit();
  disp->SetFinalSelection(selsav);
  return (OK ? IFSelect_RetDone : IFSelect_RetFail);
}

//=======================================================================
//function : fun59
//purpose  :
//=======================================================================
static Standard_Integer fun59(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  Standard_Integer theNbArgs = theNbArgs;
  const Standard_CString arg1 = theArgVec[1];
  //        ****    EvalComplete      ****
  Standard_Integer mode = 0;
  if (theNbArgs < 2) Message::SendInfo() << " -- mode par defaut 0\n";
  else
  {
    mode = atoi(arg1); Message::SendInfo() << " -- mode : " << mode;
  }
  WS->EvaluateComplete(mode); return 0;
}

//=======================================================================
//function : fun60
//purpose  :
//=======================================================================
static Standard_Integer fun60(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  //        ****    LastRunCheckList    ****
  Interface_CheckIterator chlist = WS->LastRunCheckList();
  Handle(IFSelect_CheckCounter) counter = new IFSelect_CheckCounter(0);
  counter->Analyse(chlist, WS->Model(), Standard_False);
  counter->PrintCount(Message::SendInfo());
  return 0;
}

//=======================================================================
//function : fun61
//purpose  :
//=======================================================================
static Standard_Integer fun61(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  Standard_Integer theNbArgs = theNbArgs;
  const Standard_CString arg1 = theArgVec[1];
  //        ****    RunTransformer    ****
  if (theNbArgs < 2)
  {
    Message::SendInfo() << "Donner Nom de Transformer";
    return 1;
  }
  DeclareAndCast(IFSelect_Transformer, tsf, WS->NamedItem(arg1));
  Standard_Integer effect = WS->RunTransformer(tsf);
  switch (effect)
  {
    case -4: Message::SendInfo() << "Edition sur place, nouveau Protocole, erreur recalcul graphe"; break;
    case -3: Message::SendInfo() << "Erreur, Transformation ignoree"; break;
    case -2: Message::SendInfo() << "Erreur sur edition sur place, risque de corruption (verifier)"; break;
    case -1: Message::SendInfo() << "Erreur sur edition locale, risque de corruption (verifier)"; break;
    case  0:
      if (tsf.IsNull()) Message::SendInfo() << "Erreur, pas un Transformer: " << arg1;
      else Message::SendInfo() << "Execution non faite";
      break;
    case  1: Message::SendInfo() << "Transformation locale (graphe non touche)"; break;
    case  2: Message::SendInfo() << "Edition sur place (graphe recalcule)";  break;
    case  3: Message::SendInfo() << "Modele reconstruit"; break;
    case  4: Message::SendInfo() << "Edition sur place, nouveau Protocole";  break;
    case  5: Message::SendInfo() << "Nouveau Modele avec nouveau Protocole"; break;
    default: break;
  }
  return ((effect > 0) ? IFSelect_RetDone : IFSelect_RetFail);
}

//=======================================================================
//function : fun62
//purpose  :
//=======================================================================
static Standard_Integer fun62(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  //        ****    TransformStandard Copy         ****
  return pilot->RecordItem(WS->NewTransformStandard(Standard_True));
}

//=======================================================================
//function : fun63
//purpose  :
//=======================================================================
static Standard_Integer fun63(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  //        ****    TransformStandard OntheSpot         ****
  return pilot->RecordItem(WS->NewTransformStandard(Standard_False));
}

//=======================================================================
//function : fun6465
//purpose  :
//=======================================================================
static Standard_Integer fun6465(Draw_Interpretor& theDI,
                                Standard_Integer theNbArgs,
                                const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  Standard_Integer theNbArgs = theNbArgs;
  const Standard_CString arg1 = theArgVec[1];
  //        ****    Run Modifier avec Standard Copy     ****
  //        ****    Run Modifier avec OnTheSpot         ****
  Standard_Boolean runcopy = (theArgVec[0][3] == 'c');
  //  soit c est un nom, sinon c est une commande
  Handle(IFSelect_Modifier) modif;
  if (WS->NameIdent(arg1) > 0)
    modif = GetCasted(IFSelect_Modifier, WS->NamedItem(arg1));
  else
  {
    pilot->RemoveWord(0);    // c etait la commande run
    pilot->Perform();
    modif = GetCasted(IFSelect_Modifier, pilot->RecordedItem());
  }
  Message_Messenger::StreamBuffer Message::SendInfo() = Message::SendInfo();
  if (modif.IsNull())
  {
    Message::SendInfo() << "Pas un nom de Modifier : " << arg1;
    return 1;
  }

  Handle(TColStd_HSequenceOfTransient) list;
  Handle(IFSelect_SelectPointed) sp;
  if (theNbArgs > 2)
  {
    list = XSDRAW_FunctionsSession::GiveList(WS, pilot->CommandPart(2));
    sp = new IFSelect_SelectPointed;
    sp->SetList(list);
  }

  Standard_Integer effect = 0;
  effect = WS->RunModifierSelected(modif, sp, runcopy);
  //      Message::SendInfo()<<"Modifier applique sur TransformStandard #"<<WS->ItemIdent(tsf)<<std::endl;
  switch (effect)
  {
    case -4: Message::SendInfo() << "Edition sur place, nouveau Protocole, erreur recalcul graphe"; break;
    case -3: Message::SendInfo() << "Erreur, Transformation ignoree"; break;
    case -2: Message::SendInfo() << "Erreur sur edition sur place, risque de corruption (verifier)"; break;
    case -1: Message::SendInfo() << "Erreur sur edition locale, risque de corruption (verifier)"; break;
    case  0:
      if (modif.IsNull()) Message::SendInfo() << "Erreur, pas un Modifier: " << arg1;
      else Message::SendInfo() << "Execution non faite";
      break;
    case  1: Message::SendInfo() << "Transformation locale (graphe non touche)"; break;
    case  2: Message::SendInfo() << "Edition sur place (graphe recalcule)";  break;
    case  3: Message::SendInfo() << "Modele reconstruit"; break;
    case  4: Message::SendInfo() << "Edition sur place, nouveau Protocole";  break;
    case  5: Message::SendInfo() << "Nouveau Modele avec nouveau Protocole"; break;
    default: break;
  }
  return ((effect > 0) ? IFSelect_RetDone : IFSelect_RetFail);
}

//=======================================================================
//function : fun66
//purpose  :
//=======================================================================
static Standard_Integer fun66(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  //        ****    (xset) ModifReorder         ****
  char opt = ' ';
  Standard_Integer theNbArgs = theNbArgs;
  if (theNbArgs >= 2) opt = pilot->Word(1).Value(1);
  if (opt != 'f' && opt != 'l')
  {
    Message::SendInfo() << "Donner option : f -> root-first  l -> root-last";
    return 1;
  }
  return pilot->RecordItem(new IFSelect_ModifReorder(opt == 'l'));
}

//=======================================================================
//function : fun70
//purpose  :
//=======================================================================
static Standard_Integer fun70(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  Standard_Integer theNbArgs = theNbArgs;
  const Standard_CString arg1 = theArgVec[1];
  //        ****    SelToggle         ****
  if (theNbArgs < 2)
  {
    Message::SendInfo() << "Donner Nom de Selection";
    return 1;
  }
  DeclareAndCast(IFSelect_Selection, sel, WS->NamedItem(arg1));
  if (!WS->ToggleSelectExtract(sel))
  {
    Message::SendInfo() << "Pas une SelectExtract : " << arg1;
    return 1;
  }
  if (WS->IsReversedSelectExtract(sel)) Message::SendInfo() << arg1 << " a present Reversed";
  else Message::SendInfo() << arg1 << " a present Directe";
  return 0;
}

//=======================================================================
//function : fun71
//purpose  :
//=======================================================================
static Standard_Integer fun71(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  Standard_Integer theNbArgs = theNbArgs;
  const Standard_CString arg1 = theArgVec[1];
  const Standard_CString arg2 = theArgVec[2];
  //        ****    SelInput          ****
  if (theNbArgs < 3)
  {
    Message::SendInfo() << "Donner Noms Selections cible et input";
    return 1;
  }
  DeclareAndCast(IFSelect_Selection, sel, WS->NamedItem(arg1));
  DeclareAndCast(IFSelect_Selection, sou, WS->NamedItem(arg2));
  if (sel.IsNull() || sou.IsNull())
  {
    Message::SendInfo() << "Incorrect : " << arg1 << "," << arg2;
    return 1;
  }
  if (!WS->SetInputSelection(sel, sou))
  {
    Message::SendInfo() << "Nom incorrect ou Selection " << arg1 << " ni Extract ni Deduct";
    return 1;
  }
  return 0;
}

//=======================================================================
//function : fun72
//purpose  :
//=======================================================================
static Standard_Integer fun72(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  //        ****    SelModelRoots     ****
  return pilot->RecordItem(new IFSelect_SelectModelRoots);
}

//=======================================================================
//function : fun73
//purpose  :
//=======================================================================
static Standard_Integer fun73(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  Standard_Integer theNbArgs = theNbArgs;
  const Standard_CString arg1 = theArgVec[1];
  const Standard_CString arg2 = theArgVec[2];
  //        ****    SelRange          ****
  if (theNbArgs >= 2 && arg1[0] == '?') theNbArgs = 1;
  if (theNbArgs < 2)
  {
    Message::SendInfo() << "Donner la description du SelectRange"
      << "    Formes admises :\n <n1> <n2>  : Range de <n1> a <n2>\n"
      << " <n1> tout seul : Range n0 <n1>\n  from <n1>  : Range From <n1>\n"
      << "  until <n2> : Range Until <n2>";
    return 0;
  }

  Handle(IFSelect_IntParam) low, up;
  Handle(IFSelect_SelectRange) sel;
  //                                         Range From
  if (pilot->Word(1).IsEqual("from"))
  {
    if (theNbArgs < 3)
    {
      Message::SendInfo() << "Forme admise : from <i>";
      return 1;
    }
    low = GetCasted(IFSelect_IntParam, WS->NamedItem(arg2));
    sel = new IFSelect_SelectRange;
    sel->SetFrom(low);
    //                                         Range Until
  }
  else if (pilot->Word(1).IsEqual("until"))
  {
    if (theNbArgs < 3)
    {
      Message::SendInfo() << "Forme admise : until <i>";
      return 1;
    }
    up = GetCasted(IFSelect_IntParam, WS->NamedItem(arg2));
    sel = new IFSelect_SelectRange;
    sel->SetUntil(up);
    //                                         Range One (n-th)
  }
  else if (theNbArgs < 3)
  {
    low = GetCasted(IFSelect_IntParam, WS->NamedItem(arg1));
    sel = new IFSelect_SelectRange;
    sel->SetOne(low);
    //                                         Range (from-to)
  }
  else
  {
    low = GetCasted(IFSelect_IntParam, WS->NamedItem(arg1));
    up = GetCasted(IFSelect_IntParam, WS->NamedItem(arg2));
    sel = new IFSelect_SelectRange;
    sel->SetRange(low, up);
  }
  return pilot->RecordItem(sel);
}

//=======================================================================
//function : fun74
//purpose  :
//=======================================================================
static Standard_Integer fun74(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  //        ****    SelRoots          ****
  return pilot->RecordItem(new IFSelect_SelectRoots);
}

//=======================================================================
//function : fun75
//purpose  :
//=======================================================================
static Standard_Integer fun75(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  //        ****    SelShared         ****
  return pilot->RecordItem(new IFSelect_SelectShared);
}

//=======================================================================
//function : fun76
//purpose  :
//=======================================================================
static Standard_Integer fun76(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  Standard_Integer theNbArgs = theNbArgs;
  const Standard_CString arg1 = theArgVec[1];
  const Standard_CString arg2 = theArgVec[2];
  //        ****    SelDiff           ****
  Handle(IFSelect_Selection) sel = new IFSelect_SelectDiff;
  if (sel.IsNull())
    return 1;
  if (theNbArgs < 3) Message::SendInfo() << "Diff sans input : ne pas oublier de les definir (ctlmain, ctlsec)!";
  DeclareAndCast(IFSelect_Selection, selmain, WS->NamedItem(arg1));
  DeclareAndCast(IFSelect_Selection, selsec, WS->NamedItem(arg2));
  if (theNbArgs >= 2)
    if (!WS->SetControl(sel, selmain, Standard_True))
      Message::SendInfo() << "Echec ControlMain:" << arg1 << " , a refaire (ctlmain)";
  if (theNbArgs >= 3)
    if (!WS->SetControl(sel, selsec, Standard_False))
      Message::SendInfo() << "Echec ControlSecond:" << arg2 << " , a refaire (ctlsec)";
  return pilot->RecordItem(sel);
}

//=======================================================================
//function : fun77
//purpose  :
//=======================================================================
static Standard_Integer fun77(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  Standard_Integer theNbArgs = theNbArgs;
  const Standard_CString arg1 = theArgVec[1];
  const Standard_CString arg2 = theArgVec[2];
  //        ****    SelControlMain       ****
  if (theNbArgs < 3)
  {
    Message::SendInfo() << "Donner Noms de Control et MainInput";
    return 1;
  }
  DeclareAndCast(IFSelect_Selection, sel, WS->NamedItem(arg1));
  DeclareAndCast(IFSelect_Selection, selmain, WS->NamedItem(arg2));
  if (WS->SetControl(sel, selmain, Standard_True)) return 0;
  Message::SendInfo() << "Nom incorrect ou Selection " << arg1 << " pas de type Control";
  return 1;
}

//=======================================================================
//function : fun78
//purpose  :
//=======================================================================
static Standard_Integer fun78(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  Standard_Integer theNbArgs = theNbArgs;
  const Standard_CString arg1 = theArgVec[1];
  const Standard_CString arg2 = theArgVec[2];
  //        ****    SelControlSecond       ****
  if (theNbArgs < 3)
  {
    Message::SendInfo() << "Donner Noms de Control et SecondInput";
    return 1;
  }
  DeclareAndCast(IFSelect_Selection, sel, WS->NamedItem(arg1));
  DeclareAndCast(IFSelect_Selection, seldif, WS->NamedItem(arg2));
  if (WS->SetControl(sel, seldif, Standard_False))  return 0;
  Message::SendInfo() << "Nom incorrect ou Selection " << arg1 << " pas de type Control";
  return 1;
}

//=======================================================================
//function : fun79
//purpose  :
//=======================================================================
static Standard_Integer fun79(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  //        ****    SelModelAll       ****
  return pilot->RecordItem(new IFSelect_SelectModelEntities);
}

//=======================================================================
//function : fun80
//purpose  :
//=======================================================================
static Standard_Integer fun80(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  Standard_Integer theNbArgs = theNbArgs;
  const Standard_CString arg1 = theArgVec[1];
  const Standard_CString arg2 = theArgVec[2];
  //        ****    SelCombAdd        ****
  if (theNbArgs < 3)
  {
    Message::SendInfo() << "Donner n0 Combine et une Input";
    return 1;
  }
  DeclareAndCast(IFSelect_Selection, sel, WS->NamedItem(arg1));
  DeclareAndCast(IFSelect_Selection, seladd, WS->NamedItem(arg2));
  if (WS->CombineAdd(sel, seladd)) return 0;
  Message::SendInfo() << "Nom incorrect ou Selection " << arg1 << " pas Combine";
  return 1;
}

//=======================================================================
//function : fun81
//purpose  :
//=======================================================================
static Standard_Integer fun81(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  Standard_Integer theNbArgs = theNbArgs;
  const Standard_CString arg1 = theArgVec[1];
  const Standard_CString arg2 = theArgVec[2];
  //        ****    SelCombRem        ****
  if (theNbArgs < 3)
  {
    Message::SendInfo() << "Donner n0 Combine et RANG a supprimer";
    return 1;
  }
  DeclareAndCast(IFSelect_Selection, sel, WS->NamedItem(arg1));
  DeclareAndCast(IFSelect_Selection, inp, WS->NamedItem(arg2));
  if (WS->CombineRemove(sel, inp)) return 0;
  Message::SendInfo() << "Nom incorrect ou Selection " << arg1 << " ni Union ni Intersection";
  return 1;
}

//=======================================================================
//function : fun82
//purpose  :
//=======================================================================
static Standard_Integer fun82(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  Standard_Integer theNbArgs = theNbArgs;
  const Standard_CString arg1 = theArgVec[1];
  //        ****    SelEntNumber      ****
  if (theNbArgs < 2)
  {
    Message::SendInfo() << "Donner Nom IntParam pour n0 Entite";
    return 1;
  }
  DeclareAndCast(IFSelect_IntParam, par, WS->NamedItem(arg1));
  Handle(IFSelect_SelectEntityNumber) sel = new IFSelect_SelectEntityNumber;
  sel->SetNumber(par);
  return pilot->RecordItem(sel);
}

//=======================================================================
//function : fun83
//purpose  :
//=======================================================================
static Standard_Integer fun83(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  //        ****    SelUnion          ****
  return pilot->RecordItem(new IFSelect_SelectUnion);
}

//=======================================================================
//function : fun84
//purpose  :
//=======================================================================
static Standard_Integer fun84(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  //        ****    SelIntersection   ****
  return pilot->RecordItem(new IFSelect_SelectIntersection);
}

//=======================================================================
//function : fun85
//purpose  :
//=======================================================================
static Standard_Integer fun85(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  Standard_Integer theNbArgs = theNbArgs;
  const Standard_CString arg1 = theArgVec[1];
  //        ****    SelTextType Exact ****
  if (theNbArgs < 2)
  {
    Message::SendInfo() << "Donner le TYPE a selectionner";
    return 1;
  }
  return pilot->RecordItem(new IFSelect_SelectSignature
  (new IFSelect_SignType, arg1, Standard_True));
}

//=======================================================================
//function : fun86
//purpose  :
//=======================================================================
static Standard_Integer fun86(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  //        ****    SelErrorEntities  ****
  return pilot->RecordItem(new IFSelect_SelectErrorEntities);
}

//=======================================================================
//function : fun87
//purpose  :
//=======================================================================
static Standard_Integer fun87(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  //        ****    SelUnknownEntities  **
  return pilot->RecordItem(new IFSelect_SelectUnknownEntities);
}

//=======================================================================
//function : fun88
//purpose  :
//=======================================================================
static Standard_Integer fun88(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  //        ****    SelSharing        ****
  return pilot->RecordItem(new IFSelect_SelectSharing);
}

//=======================================================================
//function : fun89
//purpose  :
//=======================================================================
static Standard_Integer fun89(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  Standard_Integer theNbArgs = theNbArgs;
  const Standard_CString arg1 = theArgVec[1];
  //        ****    SelTextType Contain **
  if (theNbArgs < 2)
  {
    Message::SendInfo() << "Donner le TYPE a selectionner";
    return 1;
  }
  return pilot->RecordItem(new IFSelect_SelectSignature
  (new IFSelect_SignType, arg1, Standard_False));
}

//=======================================================================
//function : fun90
//purpose  :
//=======================================================================
static Standard_Integer fun90(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  //        ****    SelPointed        ****
  Handle(IFSelect_SelectPointed) sp = new IFSelect_SelectPointed;
  if (theNbArgs > 1)
  {
    Handle(TColStd_HSequenceOfTransient) list = GiveList
    (XSDRAWBase::Session(), pilot->CommandPart(1));
    if (list.IsNull())
      return 1;
    Message::SendInfo() << "SelectPointed : " << list->Length() << " entities";
    sp->AddList(list);
  }
  return pilot->RecordItem(sp);
}

//=======================================================================
//function : fun91
//purpose  :
//=======================================================================
static Standard_Integer fun91(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  Standard_Integer theNbArgs = theNbArgs;
  const Standard_CString arg1 = theArgVec[1];
  //        ****    SetPointed (edit) / SetList (edit)    ****
  if (theNbArgs < 2)
  {
    Message::SendInfo() << "Donner NOM SelectPointed + Option(s) :\n"
      << " aucune : liste des entites pointees\n"
      << " 0: Clear  +nn ajout entite nn  -nn enleve nn  /nn toggle nn";
    return 1;
  }
  DeclareAndCast(IFSelect_SelectPointed, sp, WS->NamedItem(arg1));
  if (sp.IsNull())
  {
    Message::SendInfo() << "Pas une SelectPointed:" << arg1;
    return 1;
  }
  const Handle(Interface_InterfaceModel)& model = WS->Model();  // pour Print
  if (theNbArgs == 2)
  {    // listage simple
    Standard_Integer nb = sp->NbItems();
    Message::SendInfo() << " SelectPointed : " << arg1 << " : " << nb << " Items :";
    for (Standard_Integer i = 1; i <= nb; i++)
    {
      Handle(Standard_Transient) pointed = sp->Item(i);
      Standard_Integer id = WS->StartingNumber(pointed);
      if (id == 0) Message::SendInfo() << " (inconnu)";
      else
      {
        Message::SendInfo() << "  "; model->Print(pointed, Message::SendInfo());
      }
    }
    if (nb > 0) Message::SendInfo();
    return 0;
  }

  for (Standard_Integer ia = 2; ia < theNbArgs; ia++)
  {
    const TCollection_AsciiString argi = pilot->Word(ia);
    Standard_Integer id = pilot->Number(&(argi.ToCString())[1]);
    if (id == 0)
    {
      if (!argi.IsEqual("0")) Message::SendInfo() << "Incorrect,ignore:" << argi;
      else
      {
        Message::SendInfo() << "Clear SelectPointed"; sp->Clear();
      }
    }
    else if (argi.Value(1) == '-')
    {
      Handle(Standard_Transient) item = WS->StartingEntity(id);
      if (sp->Remove(item)) Message::SendInfo() << "Removed:no." << id;
      else Message::SendInfo() << " Echec Remove " << id;
      Message::SendInfo() << ": ";
      model->Print(item, Message::SendInfo());
    }
    else if (argi.Value(1) == '/')
    {
      Handle(Standard_Transient) item = WS->StartingEntity(id);
      if (sp->Remove(item)) Message::SendInfo() << "Toggled:n0." << id;
      else Message::SendInfo() << " Echec Toggle " << id;
      Message::SendInfo() << ": ";
      model->Print(item, Message::SendInfo());
    }
    else if (argi.Value(1) == '+')
    {
      Handle(Standard_Transient) item = WS->StartingEntity(id);
      if (sp->Add(item)) Message::SendInfo() << "Added:no." << id;
      else Message::SendInfo() << " Echec Add " << id;
      Message::SendInfo() << ": ";
      model->Print(item, Message::SendInfo());
    }
    else
    {
      Message::SendInfo() << "Ignore:" << argi << " , donner n0 PRECEDE de + ou - ou /";
    }
  }
  return 0;
}

//=======================================================================
//function : fun92
//purpose  :
//=======================================================================
static Standard_Integer fun92(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  //        ****    SelIncorrectEntities  ****
  WS->ComputeCheck();
  return pilot->RecordItem(new IFSelect_SelectIncorrectEntities);
}

//=======================================================================
//function : fun93
//purpose  :
//=======================================================================
static Standard_Integer fun93(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  Standard_Integer theNbArgs = theNbArgs;
  const Standard_CString arg1 = theArgVec[1];
  const Standard_CString arg2 = theArgVec[2];
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  //        ****    SelSignature        ****
  if (theNbArgs < 3)
  {
    Message::SendInfo() << "Give name of Signature or Counter, text + option exact(D) else contains";
    return 1;
  }
  Standard_Boolean exact = Standard_True;
  if (theNbArgs > 3)
  {
    if (theArgVec[3][0] == 'c') exact = Standard_False;
  }

  DeclareAndCast(IFSelect_Signature, sign, WS->NamedItem(arg1));
  DeclareAndCast(IFSelect_SignCounter, cnt, WS->NamedItem(arg1));
  Handle(IFSelect_SelectSignature) sel;

  if (!sign.IsNull())     sel = new IFSelect_SelectSignature(sign, arg2, exact);
  else if (!cnt.IsNull()) sel = new IFSelect_SelectSignature(cnt, arg2, exact);
  else
  {
    Message::SendInfo() << arg1 << ":neither Signature nor Counter";
    return 1;
  }

  return pilot->RecordItem(sel);
}

//=======================================================================
//function : fun94
//purpose  :
//=======================================================================
static Standard_Integer fun94(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  Standard_Integer theNbArgs = theNbArgs;
  const Standard_CString arg1 = theArgVec[1];
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  //        ****    SignCounter        ****
  if (theNbArgs < 2)
  {
    Message::SendInfo() << "Donner nom signature";
    return 1;
  }
  DeclareAndCast(IFSelect_Signature, sign, WS->NamedItem(arg1));
  if (sign.IsNull())
  {
    Message::SendInfo() << arg1 << ":pas une signature";
    return 1;
  }
  Handle(IFSelect_SignCounter) cnt = new IFSelect_SignCounter(sign, Standard_True, Standard_True);
  return pilot->RecordItem(cnt);
}

//=======================================================================
//function : funbselected
//purpose  :
//=======================================================================
static Standard_Integer funbselected(Draw_Interpretor& theDI,
                                     Standard_Integer theNbArgs,
                                     const char** theArgVec)
{
  Standard_Integer theNbArgs = theNbArgs;
  const Standard_CString arg1 = theArgVec[1];
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  //        ****    NbSelected = GraphCounter        ****
  if (theNbArgs < 2)
  {
    Message::SendInfo() << "Donner nom selection (deduction) a appliquer";
    return 1;
  }
  DeclareAndCast(IFSelect_SelectDeduct, applied, WS->GiveSelection(arg1));
  if (applied.IsNull())
  {
    Message::SendInfo() << arg1 << ":pas une SelectDeduct";
    return 1;
  }
  Handle(IFSelect_GraphCounter) cnt = new IFSelect_GraphCounter(Standard_True, Standard_True);
  cnt->SetApplied(applied);
  return pilot->RecordItem(cnt);
}

//=======================================================================
//function : fun_editlist
//purpose  :
//=======================================================================
static Standard_Integer fun_editlist(Draw_Interpretor& theDI,
                                     Standard_Integer theNbArgs,
                                     const char** theArgVec)
{
  Standard_Integer theNbArgs = theNbArgs;
  if (theNbArgs < 2)
  {
    Message::SendInfo() << "Give the name of an EditForm or an Editor";
    return 1;
  }
  const Standard_CString arg1 = theArgVec[1];
  const Standard_CString arg2 = theArgVec[2];
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();

  //  EditForm

  DeclareAndCast(IFSelect_EditForm, edf, WS->NamedItem(arg1));
  Handle(IFSelect_Editor) edt;
  if (!edf.IsNull())
  {
    Message::SendInfo() << "Print EditForm " << arg1;
    edt = edf->Editor();
    if (theNbArgs < 3)
    {

      //       DEFINITIONS : Editor (direct ou via EditForm)

      if (edt.IsNull()) edt = GetCasted(IFSelect_Editor, WS->NamedItem(arg1));
      if (edt.IsNull()) return 0;

      Message::SendInfo() << "Editor, Label : " << edt->Label();
      Message::SendInfo() << " --  Names (short - complete) + Labels of Values";
      edt->PrintNames(Message::SendInfo());
      Message::SendInfo() << " --  Definitions  --";
      edt->PrintDefs(Message::SendInfo());
      if (!edf.IsNull())
      {
        edf->PrintDefs(Message::SendInfo());
        Message::SendInfo() << "To display values, add an option : o original  f final  m modified";
      }

      return 0;

    }
    else
    {
      char opt = arg2[0];
      Standard_Integer what = 0;
      if (opt == 'o') what = -1;
      else if (opt == 'f') what = 1;

      edf->PrintValues(Message::SendInfo(), what, Standard_False);
    }
  }

  return 0;
}

//=======================================================================
//function : fun_editvalue
//purpose  :
//=======================================================================
static Standard_Integer fun_editvalue(Draw_Interpretor& theDI,
                                      Standard_Integer theNbArgs,
                                      const char** theArgVec)
{
  Standard_Integer theNbArgs = theNbArgs;
  if (theNbArgs < 3)
  {
    Message::SendInfo() << "Give the name of an EditForm + name of Value [+ newvalue or . to nullify]";
    return 1;
  }
  const Standard_CString arg1 = theArgVec[1];
  const Standard_CString arg2 = theArgVec[2];
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  DeclareAndCast(IFSelect_EditForm, edf, WS->NamedItem(arg1));
  if (edf.IsNull())
  {
    Message::SendInfo() << "Not an EditForm : " << arg1;
    return 1;
  }
  Standard_Integer num = edf->NameNumber(arg2);
  if (num == 0) Message::SendInfo() << "Unknown Value Name : " << arg2;
  if (num < 0) Message::SendInfo() << "Not Extracted Value Name : " << arg2;
  if (num <= 0)
    return 1;

  Standard_Boolean islist = edf->Editor()->IsList(num);
  Standard_CString name = edf->Editor()->Name(num, Standard_True); // vrai nom
  Handle(TColStd_HSequenceOfHAsciiString) listr;
  Handle(TCollection_HAsciiString) str;
  Message::SendInfo() << "Value Name : " << name << (edf->IsModified(num) ? "(already edited) : " : " : ");

  if (islist)
  {
    listr = edf->EditedList(num);
    if (listr.IsNull()) Message::SendInfo() << "(NULL LIST)";
    else
    {
      Standard_Integer ilist, nblist = listr->Length();
      Message::SendInfo() << "(List : " << nblist << " Items)";
      for (ilist = 1; ilist <= nblist; ilist++)
      {
        str = listr->Value(ilist);
        Message::SendInfo() << "  [" << ilist << "]	" << (str.IsNull() ? "(NULL)" : str->ToCString());
      }
    }
    if (theNbArgs < 4) Message::SendInfo() << "To Edit, options by editval edit-form value-name ?";
  }
  else
  {
    str = edf->EditedValue(num);
    Message::SendInfo() << (str.IsNull() ? "(NULL)" : str->ToCString());
  }
  if (theNbArgs < 4) return 0;

  //  Valeur simple ou liste ?
  Standard_Integer numarg = 3;
  str.Nullify();

  const Standard_CString argval = pilot->Arg(numarg);
  if (islist)
  {
    if (argval[0] == '?')
    {
      Message::SendInfo() << "To Edit, options" << " + val : add value at end (blanks allowed)"
        << " +nn text : insert val before item nn"
        << " nn text : replace item nn with a new value"
        << " -nn : remove item nn" << " . : clear the list";
      return 0;
    }
    Standard_Boolean stated = Standard_False;
    Handle(IFSelect_ListEditor) listed = edf->ListEditor(num);
    if (listed.IsNull())
      return 1;
    if (argval[0] == '.')
    {
      listr.Nullify();  stated = listed->LoadEdited(listr);
    }
    else if (argval[0] == '+')
    {
      Standard_Integer numadd = 0;
      if (argval[1] != '\0') numadd = atoi(argval);
      stated = listed->AddValue(new TCollection_HAsciiString(pilot->CommandPart(numarg + 1)), numadd);
    }
    else if (argval[0] == '-')
    {
      Standard_Integer numrem = atoi(argval);
      stated = listed->Remove(numrem);
    }
    else
    {
      Standard_Integer numset = atoi(argval);
      if (numset > 0) stated = listed->AddValue
      (new TCollection_HAsciiString(pilot->CommandPart(numarg + 1)), numset);
    }
    if (stated) stated = edf->ModifyList(num, listed, Standard_True);
    if (stated) Message::SendInfo() << "List Edition done";
    else Message::SendInfo() << "List Edition not done, option" << argval;
  }
  else
  {
    if (argval[0] == '.' && argval[1] == '\0') str.Nullify();
    else str = new TCollection_HAsciiString(pilot->CommandPart(numarg));
    if (edf->Modify(num, str, Standard_True))
    {
      Message::SendInfo() << "Now set to " << (str.IsNull() ? "(NULL)" : str->ToCString());
    }
    else
    {
      Message::SendInfo() << "Modify not done";
      return 1;
    }
  }
  return 0;
}

//=======================================================================
//function : fun_editclear
//purpose  :
//=======================================================================
static Standard_Integer fun_editclear(Draw_Interpretor& theDI,
                                      Standard_Integer theNbArgs,
                                      const char** theArgVec)
{
  if (theNbArgs < 2)
  {
    Message::SendInfo() << "Give the name of an EditForm [+ name of Value  else all]";
    return 1;
  }
  const Standard_CString arg1 = theArgVec[1];
  const Standard_CString arg2 = theArgVec[2];
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  DeclareAndCast(IFSelect_EditForm, edf, WS->NamedItem(arg1));
  if (edf.IsNull())
  {
    Message::SendInfo() << "Not an EditForm : " << arg1;
    return 1;
  }
  if (theNbArgs < 3)
  {
    edf->ClearEdit(); Message::SendInfo() << "All Modifications Cleared";
  }
  else
  {
    Standard_Integer num = edf->NameNumber(arg2);
    if (num == 0) Message::SendInfo() << "Unknown Value Name : " << arg2;
    if (num < 0) Message::SendInfo() << "Not Extracted Value Name : " << arg2;
    if (num <= 0)
      return 1;
    if (!edf->IsModified(num))
    {
      Message::SendInfo() << "Value " << arg2 << " was not modified"; return 0;
    }
    edf->ClearEdit(num);
    Message::SendInfo() << "Modification on Value " << arg2 << " Cleared";
  }
  return 0;
}

//=======================================================================
//function : fun_editapply
//purpose  :
//=======================================================================
static Standard_Integer fun_editapply(Draw_Interpretor& theDI,
                                      Standard_Integer theNbArgs,
                                      const char** theArgVec)
{
  if (theNbArgs < 2)
  {
    Message::SendInfo() << "Give the name of an EditForm [+ option keep to re-apply edited values]";
    return 1;
  }
  const Standard_CString arg1 = theArgVec[1];
  const Standard_CString arg2 = theArgVec[2];
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  DeclareAndCast(IFSelect_EditForm, edf, WS->NamedItem(arg1));
  if (edf.IsNull())
  {
    Message::SendInfo() << "Not an EditForm : " << arg1;
    return 1;
  }

  Handle(Standard_Transient) ent = edf->Entity();
  Handle(Interface_InterfaceModel) model = edf->Model();
  if (!model.IsNull())
  {
    if (ent.IsNull()) Message::SendInfo() << "Applying modifications on loaded model";
    else
    {
      Message::SendInfo() << "Applying modifications on loaded entity : ";
      model->PrintLabel(ent, Message::SendInfo());
    }
  }
  else Message::SendInfo() << "Applying modifications";

  if (!edf->ApplyData(edf->Entity(), edf->Model()))
  {
    Message::SendInfo() << "Modifications could not be applied";
    return 1;
  }
  Message::SendInfo() << "Modifications have been applied";

  Standard_Boolean stat = Standard_True;
  if (theNbArgs > 2 && arg2[0] == 'k') stat = Standard_False;
  if (stat)
  {
    edf->ClearEdit();
    Message::SendInfo() << "Edited values are cleared";
  }
  else Message::SendInfo() << "Edited values are kept for another loading/applying";

  return 0;
}

//=======================================================================
//function : fun_editload
//purpose  :
//=======================================================================
static Standard_Integer fun_editload(Draw_Interpretor& theDI,
                                     Standard_Integer theNbArgs,
                                     const char** theArgVec)
{
  if (theNbArgs < 2)
  {
    Message::SendInfo() << "Give the name of an EditForm [+ Entity-Ident]";
    return 1;
  }
  const Standard_CString arg1 = theArgVec[1];
  const Standard_CString arg2 = theArgVec[2];
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  DeclareAndCast(IFSelect_EditForm, edf, WS->NamedItem(arg1));
  if (edf.IsNull())
  {
    Message::SendInfo() << "Not an EditForm : " << arg1;
    return 1;
  }

  Standard_Integer num = (theNbArgs < 3 ? 0 : pilot->Number(arg2));
  Standard_Boolean stat = Standard_False;
  if (theNbArgs < 3)
  {
    Message::SendInfo() << "EditForm " << arg1 << " : Loading Model";
    stat = edf->LoadModel(WS->Model());
  }
  else if (num <= 0)
  {
    Message::SendInfo() << "Not an entity ident : " << arg2;
    return 1;
  }
  else
  {
    Message::SendInfo() << "EditForm " << arg1 << " : Loading Entity " << arg2;
    stat = edf->LoadData(WS->StartingEntity(num), WS->Model());
  }

  if (!stat)
  {
    Message::SendInfo() << "Loading not done";
    return 1;
  }
  Message::SendInfo() << "Loading done";
  return 0;
}

//=======================================================================
//function : GiveEntity
//purpose  :
//=======================================================================
Handle(Standard_Transient) GiveEntity(const Handle(XSControl_WorkSession)& WS,
                                      const Standard_CString name)
{
  Handle(Standard_Transient) ent;  // demarre a Null
  Standard_Integer num = GiveEntityNumber(WS, name);
  if (num > 0) ent = WS->StartingEntity(num);
  return ent;
}

//=======================================================================
//function : GiveEntityNumber
//purpose  :
//=======================================================================
Standard_Integer GiveEntityNumber(const Handle(XSControl_WorkSession)& WS,
                                  const Standard_CString name)
{
  Standard_Integer num = 0;
  if (!name || name[0] == '\0')
  {
    char ligne[80];  ligne[0] = '\0';
    std::cin >> ligne;
    //    std::cin.clear();  std::cin.getline (ligne,79);
    if (ligne[0] == '\0') return 0;
    num = WS->NumberFromLabel(ligne);
  }
  else num = WS->NumberFromLabel(name);
  return num;
}

//=======================================================================
//function : Init
//purpose  :
//=======================================================================
void XSDRAW_FunctionsSession::Init(Draw_Interpretor& theDI)
{
  static int THE_XSDRAW_FunctionsSession_initactor = 0;
  if (THE_XSDRAW_FunctionsSession_initactor)
  {
    return;
  }

  THE_XSDRAW_FunctionsSession_initactor = 1;
  Standard_CString aGroup = "DE: General";
  theDI.Add("xstatus", "Lists XSTEP Status : Version, System Name ...", __FILE__, funstatus, aGroup);
  theDI.Add("handler", "Toggle status catch Handler Error of the session", __FILE__, fun1, aGroup);
  theDI.Add("xload", "file:string  : Read File -> Load Model", __FILE__, fun3, aGroup);
  theDI.Add("xread", "file:string  : Read File -> Load Model", __FILE__, fun3, aGroup);
  theDI.Add("writeall", "file:string  : Write all model (no split)", __FILE__, fun4, aGroup);
  theDI.Add("writesel", "file:string sel:Selection : Write Selected (no split)", __FILE__, fun5, aGroup);
  theDI.Add("writeent", "file:string  n1ent n2ent...:integer : Write Entite(s) (no split)", __FILE__, fun6, aGroup);
  theDI.Add("writent", "file:string  n1ent n2ent...:integer : Write Entite(s) (no split)", __FILE__, fun6, aGroup);
  theDI.Add("elabel", "nument:integer   : Displays Label Model of an entity", __FILE__, fun7, aGroup);
  theDI.Add("enum", "label:string  : Displays entities n0.s of which Label Model ends by..", __FILE__, fun8, aGroup);

  theDI.Add("listtypes", "List nb entities per type. Optional selection name  else all model", __FILE__, fun9, aGroup);
  theDI.Add("count", "Count : counter [selection]", __FILE__, funcount, aGroup);
  theDI.Add("listcount", "List Counted : counter [selection [nument]]", __FILE__, funcount, aGroup);
  theDI.Add("sumcount", "Summary Counted : counter [selection [nument]]", __FILE__, funcount, aGroup);
  theDI.Add("signtype", "Sign Type [newone]", __FILE__, funsigntype, aGroup);
  theDI.Add("signcase", "signature : displays possible cases", __FILE__, funsigncase, aGroup);

  theDI.Add("estatus", "ent/nument : displays status of an entity", __FILE__, fun10, aGroup);
  theDI.Add("data", "Data (DumpModel); whole help : data tout court", __FILE__, fun11, aGroup);
  theDI.Add("entity", "give n0 ou id of entity [+ level]", __FILE__, fundumpent, aGroup);
  theDI.Add("signature", "signature name + n0/ident entity", __FILE__, funsign, aGroup);
  theDI.Add("queryparent", " give 2 n0s/labels of entities : dad son", __FILE__, funqp, aGroup);

  theDI.Add("dumpshare", "Dump Share (dispatches, IntParams)", __FILE__, fun12, aGroup);
  theDI.Add("listitems", "List Items [label else all]  ->Type,Label[,Name]", __FILE__, fun13, aGroup);
  theDI.Add("integer", "value:integer : cree un IntParam", __FILE__, fun14, aGroup);
  theDI.Add("setint", "name:IntParam   newValue:integer  : Change valeur IntParam", __FILE__, fun15, aGroup);
  theDI.Add("text", "value:string  : cree un TextParam", __FILE__, fun16, aGroup);
  theDI.Add("settext", "Name:TextParam  newValue:string   : Change valeur TextParam", __FILE__, fun17, aGroup);
  theDI.Add("dumpsel", "Dump Selection suivi du Nom de la Selection a dumper", __FILE__, fun19, aGroup);
  theDI.Add("evalsel", "name:Selection [num/sel]  : Evalue une Selection", __FILE__, fun20, aGroup);
  theDI.Add("givelist", "num/sel [num/sel ...]  : Evaluates GiveList", __FILE__, fun20, aGroup);
  theDI.Add("giveshort", "num/sel [num/sel ...]  : GiveList in short form", __FILE__, fun20, aGroup);
  theDI.Add("givepointed", "num/sel [num/sel ...]  : GiveList to fill a SelectPointed", __FILE__, fun20, aGroup);
  theDI.Add("makelist", "listname [givelist] : Makes a List(SelectPointed) from GiveList", __FILE__, fun20, aGroup);
  theDI.Add("givecount", "num/sel [num/sel ...]  : Counts GiveList", __FILE__, fun20c, aGroup);
  theDI.Add("selsuite", "sel sel ...  : Creates a SelectSuite", __FILE__, funselsuite, aGroup);
  theDI.Add("clearitems", "Clears all items (selections, dispatches, etc)", __FILE__, fun21, aGroup);
  theDI.Add("cleardata", "mode:a-g-c-p  : Clears all or some data (model, check...)", __FILE__, fun22, aGroup);

  theDI.Add("itemlabel", "xxx xxx : liste items having this label", __FILE__, fun24, aGroup);
  theDI.Add("xsave", "filename:string  : sauve items-session", __FILE__, fun25, aGroup);
  theDI.Add("xrestore", "filename:string  : restaure items-session", __FILE__, fun26, aGroup);
  theDI.Add("param", "[-p Pattern] - displays all parameters or filtered by pattern;\n"
            "par_name - displays parameter;\n"
            "par_name par_value - changes parameter's value", __FILE__, fun27, aGroup);

  theDI.Add("sentfiles", "Lists files sent from last Load", __FILE__, fun29, aGroup);
  theDI.Add("fileprefix", "prefix:string    : definit File Prefix", __FILE__, fun30, aGroup);
  theDI.Add("fileext", "extent:string    : definit File Extension", __FILE__, fun31, aGroup);
  theDI.Add("fileroot", "disp:Dispatch  root:string  : definit File Root sur un Dispatch", __FILE__, fun32, aGroup);
  theDI.Add("filedef", "defroot:string   : definit File DefaultRoot", __FILE__, fun33, aGroup);
  theDI.Add("evalfile", "Evaluation du FileNaming et memorisation", __FILE__, fun34, aGroup);
  theDI.Add("clearfile", "Efface la liste d'EvalFile", __FILE__, fun35, aGroup);
  theDI.Add("xsplit", "[disp:Dispatch  sinon tout]  : Split, la grande affaire !", __FILE__, fun36, aGroup);
  theDI.Add("remaining", "options... : Remaining Entities, help complet par  remaining ?", __FILE__, fun37, aGroup);
  theDI.Add("setcontent", "sel:Selection mode:k ou r  : Restreint contenu du modele", __FILE__, fun38, aGroup);

  theDI.Add("listmodif", "List Final Modifiers", __FILE__, fun40, aGroup);
  theDI.Add("dumpmodif", "modif:Modifier  : Affiche le Statut d'un Modifier", __FILE__, fun41, aGroup);
  theDI.Add("modifsel", "modif:Modifier [sel:Selection]  : Change/Annule Selection de Modifier", __FILE__, fun42, aGroup);
  theDI.Add("setapplied", "modif:Modifier [name:un item sinon sortie fichier]  : Applique un Modifier", __FILE__, fun43, aGroup);
  theDI.Add("resetapplied", "modif:Modifier  : Enleve un Modifier de la sortie fichier", __FILE__, fun44, aGroup);
  theDI.Add("modifmove", "modif:Modifier M(model)/F(file) avant,apres:integer  : Deplace un Modifier (sortie fichier)", __FILE__, fun45, aGroup);

  theDI.Add("dispsel", "disp:Dispatch sel:Selection  -> Selection Finale de Dispatch", __FILE__, fun51, aGroup);
  theDI.Add("dispone", "cree DispPerOne", __FILE__, fun_dispone, aGroup);
  theDI.Add("dispglob", "cree DispGlobal", __FILE__, fun_dispglob, aGroup);
  theDI.Add("dispcount", "count:IntParam  : cree DispPerCount", __FILE__, fun_dispcount, aGroup);
  theDI.Add("dispfile", "files:IntParam  : cree DispPerFiles", __FILE__, fun_dispfiles, aGroup);
  theDI.Add("dispsign", "sign:Signature  : cree DispPerSignature", __FILE__, fun_dispsign, aGroup);
  theDI.Add("dumpdisp", "disp:Dispatch   : Affiche le Statut d'un Dispatch", __FILE__, fun56, aGroup);

  theDI.Add("xremove", "nom  : Remove a Control Item de la Session", __FILE__, fun57, aGroup);
  theDI.Add("evaldisp", "mode=[0-3]  disp:Dispatch  : Evaluates one or more Dispatch(es)", __FILE__, fun58, aGroup);
  theDI.Add("evaladisp", "mode=[0-3]  disp:Dispatch [givelist]  : Evaluates a Dispatch (on a GiveList)", __FILE__, fun_evaladisp, aGroup);
  theDI.Add("writedisp", "filepattern  disp:Dispatch [givelist]  : Writes Entities by Splitting by a Dispatch", __FILE__, fun_writedisp, aGroup);
  theDI.Add("evalcomplete", "Evaluation Complete de la Repartition", __FILE__, fun59, aGroup);

  theDI.Add("runcheck", "affiche LastRunCheckList (write,modif)", __FILE__, fun60, aGroup);
  theDI.Add("runtranformer", "transf:Transformer  : Applique un Transformer", __FILE__, fun61, aGroup);
  theDI.Add("copy", "cree TransformStandard, option Copy, vide", __FILE__, fun62, aGroup);
  theDI.Add("onthespot", "cree TransformStandard, option OntheSpot, vide", __FILE__, fun63, aGroup);
  theDI.Add("runcopy", "modif:ModelModifier [givelist] : Run <modif> via TransformStandard option Copy", __FILE__, fun6465, aGroup);
  theDI.Add("runonthespot", "modif:ModelModifier [givelist] : Run <modif> via TransformStandard option OnTheSpot", __FILE__, fun6465, aGroup);
  theDI.Add("reorder", "[f ou t] reordonne le modele", __FILE__, fun66, aGroup);

  theDI.Add("toggle", "sel:Selection genre Extract  : Toggle Direct/Reverse", __FILE__, fun70, aGroup);
  theDI.Add("input", "sel:Selection genre Deduct ou Extract  input:Selection  : Set Input", __FILE__, fun71, aGroup);
  theDI.Add("modelroots", "cree SelectModelRoots", __FILE__, fun72, aGroup);
  theDI.Add("range", "options... : cree SelectRange ...; tout court pour help", __FILE__, fun73, aGroup);
  theDI.Add("roots", "cree SelectRoots (local roots)", __FILE__, fun74, aGroup);
  theDI.Add("shared", "cree SelectShared", __FILE__, fun75, aGroup);
  theDI.Add("diff", "[main:Selection diff:Selection]  : cree SelectDiff", __FILE__, fun76, aGroup);
  theDI.Add("selmain", "sel:Selection genre Control  main:Selection  : Set Main Input", __FILE__, fun77, aGroup);
  theDI.Add("selsecond", "sel:Selection genre Control  sec:Selection   : Set Second Input", __FILE__, fun78, aGroup);
  theDI.Add("modelall", "cree SelectModelAll", __FILE__, fun79, aGroup);
  theDI.Add("seladd", "sel:Selection genre Combine  input:Selection  : Add Selection", __FILE__, fun80, aGroup);
  theDI.Add("selrem", "sel:Selection genre Combine  input:Selection  : Remove Selection", __FILE__, fun81, aGroup);
  theDI.Add("number", "num:IntParam  : Cree SelectEntityNumber", __FILE__, fun82, aGroup);

  theDI.Add("union", "cree SelectUnion (vide), cf aussi combadd, combrem", __FILE__, fun83, aGroup);
  theDI.Add("intersect", "cree SelectIntersection (vide), cf aussi combadd, combrem", __FILE__, fun84, aGroup);
  theDI.Add("typexact", "type:string  : cree SelectTextType Exact", __FILE__, fun85, aGroup);
  theDI.Add("errors", "cree SelectErrorEntities (from file)", __FILE__, fun86, aGroup);
  theDI.Add("unknown", "cree SelectUnknownEntities", __FILE__, fun87, aGroup);
  theDI.Add("sharing", "cree SelectSharing", __FILE__, fun88, aGroup);
  theDI.Add("typecontain", "type:string  : cree SelectTextType Contains", __FILE__, fun89, aGroup);
  theDI.Add("pointed", "cree SelectPointed [num/sel num/sel]", __FILE__, fun90, aGroup);
  theDI.Add("setpointed", "sel:SelectPointed  : edition SelectPointed. tout court pour help", __FILE__, fun91, aGroup);
  theDI.Add("setlist", "sel:SelectPointed  : edition SelectPointed. tout court pour help", __FILE__, fun91, aGroup);
  theDI.Add("incorrect", "cree SelectIncorrectEntities (computed)", __FILE__, fun92, aGroup);

  theDI.Add("signsel", "sign:Signature|cnt:Counter text:string [e(D)|c] : cree SelectSignature", __FILE__, fun93, aGroup);
  theDI.Add("signcounter", "sign:Signature : cree SignCounter", __FILE__, fun94, aGroup);
  theDI.Add("nbselected", "applied:Selection : cree GraphCounter(=NbSelected)", __FILE__, funbselected, aGroup);

  theDI.Add("editlist", "editor or editform : lists defs + values", __FILE__, fun_editlist, aGroup);
  theDI.Add("editvalue", "editform paramname [newval or .] : lists-changes a value", __FILE__, fun_editvalue, aGroup);
  theDI.Add("editclear", "editform [paramname] : clears edition on all or one param", __FILE__, fun_editclear, aGroup);
  theDI.Add("editload", "editform [entity-id] : loads from model or an entity", __FILE__, fun_editload, aGroup);
  theDI.Add("editapply", "editform [keep] : applies on loaded data", __FILE__, fun_editapply, aGroup);
}
