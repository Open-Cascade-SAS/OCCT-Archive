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
#include <XSDRAW.hxx>
#include <XSDRAWBase.hxx>

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
  XSDRAW::StreamContainer aSSC(theDI);
  (void)theNbArgs;
  (void)theArgVec;
  //        ****    Version & cie     ****
    //#58 rln
  aSSC.SStream() << "Processor Version : " << XSTEP_PROCESSOR_VERSION;
  aSSC.SStream() << "OL Version        : " << XSTEP_SYSTEM_VERSION;
  aSSC.SStream() << "Configuration     : " << XSTEP_Config;
  aSSC.SStream() << "UL Names          : " << XSTEP_ULNames;
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
  XSDRAW::StreamContainer aSSC(theDI);
  (void)theNbArgs;
  (void)theArgVec;
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  //        ****    ToggleHandler     ****
  Standard_Boolean hand = !WS->ErrorHandle();
  if (hand) aSSC.SStream() << " --  Mode Catch Error now Active";
  else      aSSC.SStream() << " --  Mode Catch Error now Inactive";
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
  XSDRAW::StreamContainer aSSC(theDI);
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();

  const Standard_CString arg1 = theArgVec[1];
  //        ****    XRead / Load         ****
  if (theNbArgs < 2)
  {
    aSSC.SStream() << "Read/Load : give file name !";
    return 1;
  }
  if (WS->Protocol().IsNull())
  {
    aSSC.SStream() << "Protocol not defined";
    return 1;
  }
  if (WS->WorkLibrary().IsNull())
  {
    aSSC.SStream() << "WorkLibrary not defined";
    return 1;
  }
  IFSelect_ReturnStatus status = WS->ReadFile(arg1);
  // status : 0 OK, 1 erreur lecture, 2 Fail(try/catch),
  //          -1 fichier non trouve, -2 lecture faite mais resultat vide
  switch (status)
  {
    case IFSelect_RetVoid: aSSC.SStream() << "file:" << arg1 << " gives empty result"; break;
    case IFSelect_RetError: aSSC.SStream() << "file:" << arg1 << " could not be opened"; break;
    case IFSelect_RetDone: aSSC.SStream() << "file:" << arg1 << " read"; break;
    case IFSelect_RetFail: aSSC.SStream() << "file:" << arg1 << " : error while reading"; break;
    case IFSelect_RetStop: aSSC.SStream() << "file:" << arg1 << " : EXCEPTION while reading"; break;
    default: aSSC.SStream() << "file:" << arg1 << " could not be read"; break;
  }
  if (status != IFSelect_RetDone)
    return 1;
  //      aSSC.SStream()<<" - clearing list of already written files"<<std::endl;
  WS->BeginSentFiles(Standard_True);
  return 0;
}

//=======================================================================
//function : fun4
//purpose  :
//=======================================================================
static Standard_Integer fun4(Draw_Interpretor& theDI,
                             Standard_Integer theNbArgs,
                             const char** theArgVec)
{
  XSDRAW::StreamContainer aSSC(theDI);
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();

  const Standard_CString arg1 = theArgVec[1];
  //        ****    Write All         ****
  if (theNbArgs < 2)
  {
    aSSC.SStream() << "Write All : give file name !";
    return 1;
  }
  return (WS->SendAll(arg1) == IFSelect_RetDone ? 0 : 1);
}

//=======================================================================
//function : fun6
//purpose  :
//=======================================================================
static Standard_Integer fun6(Draw_Interpretor& theDI,
                             Standard_Integer theNbArgs,
                             const char** theArgVec)
{
  XSDRAW::StreamContainer aSSC(theDI);
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();

  const Standard_CString arg1 = theArgVec[1];
  //        ****    Write Entite(s)         ****
  if (theNbArgs < 3)
  {
    aSSC.SStream() << "Write Entitie(s) : give file name + n0s entitie(s)!";
    return 1;
  }
  int ko = 0;
  Handle(IFSelect_SelectPointed) sp = new IFSelect_SelectPointed;
  for (Standard_Integer ia = 2; ia < theNbArgs; ia++)
  {
    Standard_Integer id = WS->NumberFromLabel(theArgVec[ia]);
    if (id > 0)
    {
      Handle(Standard_Transient) item = WS->StartingEntity(id);
      if (sp->Add(item)) aSSC.SStream() << "Added:no." << id;
      else
      {
        aSSC.SStream() << " Fail Add n0." << id; ko++;
      }
    }
    else
    {
      aSSC.SStream() << "Not an entity number:" << theArgVec[ia]; ko++;
    }
  }
  if (ko > 0)
  {
    aSSC.SStream() << ko << " bad arguments, abandon";
    return 1;
  }
  return (WS->SendSelected(arg1, sp) == IFSelect_RetDone ? 0 : 1);
}

//=======================================================================
//function : fun7
//purpose  :
//=======================================================================
static Standard_Integer fun7(Draw_Interpretor& theDI,
                             Standard_Integer theNbArgs,
                             const char** theArgVec)
{
  XSDRAW::StreamContainer aSSC(theDI);
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();

  const Standard_CString arg1 = theArgVec[1];
  //        ****    Entity Label       ****
  if (theNbArgs < 2)
  {
    aSSC.SStream() << "Give entity number";
    return 1;
  }
  if (!WS->HasModel())
  {
    aSSC.SStream() << "No loaded model, abandon";
    return 1;
  }
  Standard_Integer nument = WS->NumberFromLabel(arg1);
  if (nument <= 0 || nument > WS->NbStartingEntities())
  {
    aSSC.SStream() << "Not a suitable number: " << arg1;
    return 1;
  }
  aSSC.SStream() << "N0." << nument << " ->Label in Model : ";
  WS->Model()->PrintLabel(WS->StartingEntity(nument), aSSC.SStream());

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
  XSDRAW::StreamContainer aSSC(theDI);
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();

  const Standard_CString arg1 = theArgVec[1];
  //        ****    Entity Number      ****
  if (theNbArgs < 2)
  {
    aSSC.SStream() << "Give label to search";
    return 1;
  }
  if (!WS->HasModel())
  {
    aSSC.SStream() << "No loaded model, abandon";
    return 1;
  }
  const Handle(Interface_InterfaceModel)& model = WS->Model();
  Standard_Integer i, cnt = 0;
  Standard_Boolean exact = Standard_False;
  aSSC.SStream() << " **  Search Entity Number for Label : " << arg1;
  for (i = model->NextNumberForLabel(arg1, 0, exact); i != 0;
       i = model->NextNumberForLabel(arg1, i, exact))
  {
    cnt++;
    aSSC.SStream() << " **  Found n0/id:";
    model->Print(model->Value(i), aSSC.SStream());

  }

  if (cnt == 0) aSSC.SStream() << " **  No Match";
  else if (cnt == 1) aSSC.SStream() << " **  1 Match";
  else aSSC.SStream() << cnt << " Matches";
  return 0;
}

//=======================================================================
//function : funsigntype
//purpose  :
//=======================================================================
static Standard_Integer fun9(Draw_Interpretor& theDI,
                             Standard_Integer theNbArgs,
                             const char** theArgVec)
{
  XSDRAW::StreamContainer aSSC(theDI);
  Handle(IFSelect_WorkSession) WS = XSDRAWBase::Session();
  Handle(IFSelect_Signature) signtype = WS->SignType();
  if (signtype.IsNull()) signtype = new IFSelect_SignType;
  Handle(IFSelect_SignCounter) aCounter =
    new IFSelect_SignCounter(signtype, Standard_False);
  if (theNbArgs == 1)
  {
    aCounter->AddModel(WS->Model());
  }
  else
  {
    //   on demande un givelist
    Handle(TColStd_HSequenceOfTransient) list = WS->GiveList(theArgVec[1]);
    if (list.IsNull())
    {
      theDI << "Error: Nothing selected from : " << theArgVec[1] << "\n";
      return 1;
    }
    aCounter->AddWithGraph(list, WS->Graph());
  }
  aCounter->PrintList(aSSC.SStream(), WS->Model(), IFSelect_CountByItem);
  return 0;
}

//=======================================================================
//function : funsigntype
//purpose  :
//=======================================================================
static Standard_Integer funsigntype(Draw_Interpretor& theDI,
                                    Standard_Integer theNbArgs,
                                    const char** theArgVec)
{
  XSDRAW::StreamContainer aSSC(theDI);
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();

  const Standard_CString arg1 = theArgVec[1];
  //        ****    Sign Type              ****
  Handle(IFSelect_Signature) signtype = WS->SignType();
  if (signtype.IsNull()) aSSC.SStream() << "signtype actually undefined";
  else
  {
    Handle(TCollection_HAsciiString) str = WS->Name(signtype);
    Standard_Integer id = WS->ItemIdent(signtype);
    aSSC.SStream() << signtype->Label();
    if (str.IsNull())
    {
      if (id > 0) aSSC.SStream() << "signtype : item n0 " << id;
    }
    else
    {
      aSSC.SStream() << "signtype : also named as " << str->ToCString();
    }
  }
  if (theNbArgs < 2) aSSC.SStream() << "signtype newitem  to change, signtype . to clear";
  else
  {
    if (arg1[0] == '.' && arg1[1] == '\0')
    {
      signtype.Nullify();
      aSSC.SStream() << "signtype now cleared";
    }
    else
    {
      signtype = GetCasted(IFSelect_Signature, WS->NamedItem(arg1));
      if (signtype.IsNull())
      {
        aSSC.SStream() << "Not a Signature : " << arg1;
        return 1;
      }
      else aSSC.SStream() << "signtype now set to " << arg1;
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
  XSDRAW::StreamContainer aSSC(theDI);
  (void)theNbArgs;
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  const Standard_CString arg1 = theArgVec[1];
  //        ****    Sign Case              ****
  Handle(IFSelect_Signature) signcase = GetCasted(IFSelect_Signature, WS->NamedItem(arg1));
  if (signcase.IsNull()) aSSC.SStream() << "Not a Signature : " << arg1;
  else
  {
    Standard_Boolean hasmin, hasmax;  Standard_Integer valmin, valmax;
    if (signcase->IsIntCase(hasmin, valmin, hasmax, valmax))
    {
      aSSC.SStream() << "Signature " << arg1 << " : Integer Case";
      if (hasmin) aSSC.SStream() << " - Mini:" << valmin;
      if (hasmax) aSSC.SStream() << " - Maxi:" << valmax;

    }
    Handle(TColStd_HSequenceOfAsciiString) caselist = signcase->CaseList();
    if (caselist.IsNull()) aSSC.SStream() << "Signature " << arg1 << " : no predefined case, see command  count " << arg1;
    else
    {
      Standard_Integer i, nb = caselist->Length();
      aSSC.SStream() << "Signature " << arg1 << " : " << nb << " basic cases :";
      for (i = 1; i <= nb; i++) aSSC.SStream() << "  " << caselist->Value(i);

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
  XSDRAW::StreamContainer aSSC(theDI);
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();

  const Standard_CString arg1 = theArgVec[1];
  //        ****    Entity Status          ****
  Standard_Integer i, nb;
  if (theNbArgs < 2)
  {
    nb = Interface_Category::NbCategories();
    aSSC.SStream() << " Categories defined :" << nb << " i.e. :\n";
    for (i = 0; i <= nb; i++)
      aSSC.SStream() << "Cat." << i << "  : " << Interface_Category::Name(i) << "\n";
    aSSC.SStream() << " On a given entity : give its number";
    return 0;
  }
  Standard_Integer num = WS->NumberFromLabel(arg1);
  if (num <= 0 || num > WS->NbStartingEntities())
  {
    aSSC.SStream() << "Not a suitable entity number : " << arg1;
    return 1;
  }
  Handle(Standard_Transient) ent = WS->StartingEntity(num);
  WS->PrintEntityStatus(ent, aSSC.SStream());
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
  XSDRAW::StreamContainer aSSC(theDI);
  if (theNbArgs < 2)
  {
    aSSC.SStream() << "Error: Empty Mode\n";
    return 1;
  }
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  //  
  const Standard_CString arg1 = theArgVec[1];
  //        ****    DumpModel (Data)  ****
  Standard_Integer niv = 0;
  //  char arg10 = arg1[0];
  //  if (theNbArgs < 2) arg10 = '?';
  switch (arg1[0])
  {
    case '?':
      aSSC.SStream() << "? for this help, else give a listing mode (first letter suffices) :\n"
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
    default: aSSC.SStream() << "Unknown Mode .   tout court pour help";
      return 1;
  }
  WS->DumpModel(niv, aSSC.SStream());
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
  XSDRAW::StreamContainer aSSC(theDI);
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  Handle(IFSelect_WorkLibrary) WL = WS->WorkLibrary();
  Standard_Integer levdef = 0, levmax = 10, level;
  WL->DumpLevels(levdef, levmax);
  if (theNbArgs < 2 || (theNbArgs == 2 && levmax < 0))
  {
    aSSC.SStream() << "Give n0 or id of entity";
    if (levmax < 0)
      aSSC.SStream() << "  and dump level";
    else
      aSSC.SStream() << "  + optional, dump level in [0 - " << levmax << "] , default = " << levdef;
    for (level = 0; level <= levmax; level++)
    {
      Standard_CString help = WL->DumpHelp(level);
      if (help[0] != '\0')
        aSSC.SStream() << level << " : " << help;
    }
    return 1;
  }

  const Standard_CString arg1 = theArgVec[1];
  const Standard_CString arg2 = theArgVec[2];
  Standard_Integer num = WS->NumberFromLabel(arg1);
  if (num == 0)
    return 1;
  level = levdef;
  if (theNbArgs > 2)
    level = atoi(arg2);
  Handle(Standard_Transient) ent = WS->StartingEntity(num);
  if (ent.IsNull())
  {
    aSSC.SStream() << "No entity with given id " << arg1 << " (" << num << ") is found in the current model";
  }
  else
  {
    aSSC.SStream() << "  --   DUMP  Entity n0 " << num << "  level " << level;
    WL->DumpEntity(WS->Model(), WS->Protocol(), ent, aSSC.SStream(), level);

    Interface_CheckIterator chl = WS->CheckOne(ent);
    if (!chl.IsEmpty(Standard_False))
      chl.Print(aSSC.SStream(), WS->Model(), Standard_False);
  }
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
  XSDRAW::StreamContainer aSSC(theDI);
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();

  const Standard_CString arg1 = theArgVec[1];
  const Standard_CString arg2 = theArgVec[2];
  if (theNbArgs < 3)
  {
    aSSC.SStream() << " Give signature name + n0 or id of entity";
    return 1;
  }
  DeclareAndCast(IFSelect_Signature, sign, WS->NamedItem(arg1));
  if (sign.IsNull())
  {
    aSSC.SStream() << "Not a signature : " << arg1;
    return 1;
  }
  Standard_Integer num = WS->NumberFromLabel(arg2);
  Handle(Standard_Transient) ent = WS->StartingEntity(num);
  if (num == 0)
    return 1;
  aSSC.SStream() << "Entity n0 " << num << " : " << WS->SignValue(sign, ent);
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
  XSDRAW::StreamContainer aSSC(theDI);
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();

  const Standard_CString arg1 = theArgVec[1];
  const Standard_CString arg2 = theArgVec[2];
  if (theNbArgs < 3)
  {
    aSSC.SStream() << " Give 2 numeros or labels : dad son";
    return 1;
  }
  Standard_Integer n1 = WS->NumberFromLabel(arg1);
  Standard_Integer n2 = WS->NumberFromLabel(arg2);
  aSSC.SStream() << "QueryParent for dad:" << arg1 << ":" << n1 << " and son:" << arg2 << ":" << n2;
  Standard_Integer qp = WS->QueryParent(WS->StartingEntity(n1), WS->StartingEntity(n2));
  if (qp < 0) aSSC.SStream() << arg1 << " is not super-entity of " << arg2;
  else if (qp == 0) aSSC.SStream() << arg1 << " is same as " << arg2;
  else aSSC.SStream() << arg1 << " is super-entity of " << arg2 << " , max level found=" << qp;
  //  aSSC.SStream()<<" Trouve "<<qp<<std::endl;
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
  XSDRAW::StreamContainer aSSC(theDI);
  (void)theNbArgs;
  (void)theArgVec;
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  //        ****    DumpShare         ****
  WS->DumpShare();
  return 0;
}

//=======================================================================
//function : fun13
//purpose  :
//=======================================================================
static Standard_Integer fun13(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  XSDRAW::StreamContainer aSSC(theDI);
  (void)theNbArgs;
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  //        ****    ListItems         ****
  WS->ListItems(theArgVec[1]);
  return 0;
}

//=======================================================================
//function : fun15
//purpose  :
//=======================================================================
static Standard_Integer fun15(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  XSDRAW::StreamContainer aSSC(theDI);
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();

  const Standard_CString arg1 = theArgVec[1];
  const Standard_CString arg2 = theArgVec[2];
  //        ****    SetInt            ****
  if (theNbArgs < 3)
  {
    aSSC.SStream() << "Donner 2 arguments : nom Parametre et Valeur";
    return 1;
  }
  Standard_Integer val = atoi(arg2);
  DeclareAndCast(IFSelect_IntParam, par, WS->NamedItem(arg1));
  if (!WS->SetIntValue(par, val))
    return 1;
  return 0;
}

//=======================================================================
//function : fun17
//purpose  :
//=======================================================================
static Standard_Integer fun17(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  XSDRAW::StreamContainer aSSC(theDI);
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();

  const Standard_CString arg1 = theArgVec[1];
  const Standard_CString arg2 = theArgVec[2];
  //        ****    SetText           ****
  if (theNbArgs < 3)
  {
    aSSC.SStream() << "Donner 2 arguments : nom Parametre et Valeur";
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
  XSDRAW::StreamContainer aSSC(theDI);
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();

  const Standard_CString arg1 = theArgVec[1];
  //        ****    DumpSel           ****
  if (theNbArgs < 2)
  {
    aSSC.SStream() << "Give 1 argument : Selection Name";
    return 1;
  }
  WS->DumpSelection(GetCasted(IFSelect_Selection, WS->NamedItem(arg1)));
  return 0;
}

//=======================================================================
//function : fun21
//purpose  :
//=======================================================================
static Standard_Integer fun21(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  XSDRAW::StreamContainer aSSC(theDI);
  (void)theNbArgs;
  (void)theArgVec;
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  //        ****    ClearItems           ****
  WS->ClearItems();  WS->ClearFinalModifiers();  WS->ClearShareOut(Standard_False);
  return 0;
}

static Standard_Integer fun22(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  XSDRAW::StreamContainer aSSC(theDI);
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();

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
    if (mode < 0) aSSC.SStream() << "Give a suitable mode";
    aSSC.SStream() << "  Available Modes :\n"
      << " a : all     g : graph+check  c : check  p : selectpointed";
    return 1;
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
  XSDRAW::StreamContainer aSSC(theDI);
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();

  //        ****    Item Label         ****
  TCollection_AsciiString label;
  if (theNbArgs < 2)
  {
    aSSC.SStream() << " Give  label to search";
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
    aSSC.SStream() << "Searching label : " << label << ". in mode ";
    if (mode == 0) aSSC.SStream() << " exact";
    if (mode == 1) aSSC.SStream() << " same head";
    if (mode == 2) aSSC.SStream() << " search if present";
    for (id = WS->NextIdentForLabel(label.ToCString(), 0, mode); id != 0;
         id = WS->NextIdentForLabel(label.ToCString(), id, mode))
    {
      aSSC.SStream() << " " << id;  nbitems++;
    }
    aSSC.SStream() << " -- giving " << nbitems << " found";
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
  XSDRAW::StreamContainer aSSC(theDI);
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();

  const Standard_CString arg1 = theArgVec[1];
  //        ****    Save (Dump)       ****
  if (theNbArgs < 2)
  {
    aSSC.SStream() << "Donner nom du Fichier";
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
  XSDRAW::StreamContainer aSSC(theDI);
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();

  const Standard_CString arg1 = theArgVec[1];
  //        ****    Restore (Dump)    ****
  if (theNbArgs < 2)
  {
    aSSC.SStream() << "Donner nom du Fichier";
    return 1;
  }
  IFSelect_SessionFile dumper(WS);
  Standard_Integer readstat = dumper.Read(arg1);
  if (readstat == 0)
    return 0;
  else if (readstat > 0) aSSC.SStream() << "-- Erreur Lecture Fichier " << arg1;
  else                    aSSC.SStream() << "-- Pas pu ouvrir Fichier " << arg1;
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
  XSDRAW::StreamContainer aSSC(theDI);
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
    aSSC.SStream() << " List of parameters : " << aPatternNb << " items : ";
    for (i = 1; i <= nb; i++)
    {
      if (theNbArgs == 3 && strncmp(li->Value(i)->String().ToCString(), arg2, aPatternLen) != 0)
      {
        continue;
      }
      aSSC.SStream() << li->Value(i)->String();
      aSSC.SStream() << " : " << Interface_Static::CVal(li->Value(i)->ToCString());
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
    if (theNbArgs > 2) aSSC.SStream() << "     FORMER STATUS of Static Parameter " << arg1;
    else          aSSC.SStream() << "     ACTUAL STATUS of Static Parameter " << arg1;
    if (!Interface_Static::IsPresent(arg1))
    {
      aSSC.SStream() << " Parameter " << arg1 << " undefined";
      return 1;
    }
    if (!Interface_Static::IsSet(arg1)) aSSC.SStream() << " Parameter " << arg1 << " not valued";
    else if (theNbArgs == 2) Interface_Static::Static(arg1)->Print(aSSC.SStream());
    else aSSC.SStream() << " Value : " << Interface_Static::CVal(arg1);

    if (theNbArgs == 2) aSSC.SStream() << "To modify, param name_param new_val";
    else
    {
      if (strlen(arg2) != 0)
      {
        aSSC.SStream() << " New demanded value : " << arg2;
      }
      else
      {
        aSSC.SStream() << " New demanded value : not valued";
      }
      if (Interface_Static::SetCVal(arg1, arg2))
      {
        aSSC.SStream() << "   OK";
        return 0;
      }
      else
      {
        aSSC.SStream() << " , refused";
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
  XSDRAW::StreamContainer aSSC(theDI);
  (void)theNbArgs;
  (void)theArgVec;
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  //        ****    SentFiles         ****
  Handle(TColStd_HSequenceOfHAsciiString) list = WS->SentFiles();
  if (list.IsNull())
  {
    aSSC.SStream() << "List of Sent Files not enabled";
    return 0;
  }
  Standard_Integer i, nb = list->Length();
  aSSC.SStream() << "  Sent Files : " << nb << " : ";
  for (i = 1; i <= nb; i++)
    aSSC.SStream() << list->Value(i)->ToCString();
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
  XSDRAW::StreamContainer aSSC(theDI);
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();

  const Standard_CString arg1 = theArgVec[1];
  //        ****    FilePrefix        ****
  if (theNbArgs < 2)
  {
    if (WS->FilePrefix().IsNull()) aSSC.SStream() << "Pas de prefixe defini";
    else aSSC.SStream() << "Prefixe : " << WS->FilePrefix()->ToCString();
    aSSC.SStream() << "Pour changer :  filepref newprefix";
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
  XSDRAW::StreamContainer aSSC(theDI);
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();

  const Standard_CString arg1 = theArgVec[1];
  //        ****    FileExtension     ****
  if (theNbArgs < 2)
  {
    if (WS->FileExtension().IsNull()) aSSC.SStream() << "Pas d extension definie";
    else aSSC.SStream() << "Extension : " << WS->FileExtension()->ToCString();
    aSSC.SStream() << "Pour changer :  fileext newext";
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
  XSDRAW::StreamContainer aSSC(theDI);
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();

  const Standard_CString arg1 = theArgVec[1];
  const Standard_CString arg2 = theArgVec[2];
  //        ****    FileRoot          ****
  if (theNbArgs < 2)
  {
    aSSC.SStream() << "Donner Dispatch et nom de Root";
    return 1;
  }
  DeclareAndCast(IFSelect_Dispatch, disp, WS->NamedItem(arg1));
  if (theNbArgs < 3)
  {
    if (WS->FileRoot(disp).IsNull()) aSSC.SStream() << "Pas de racine definie pour " << arg1;
    else aSSC.SStream() << "Racine pour " << arg1 << " : " << WS->FileRoot(disp)->ToCString();
    aSSC.SStream() << "Pour changer :  fileroot nomdisp newroot";
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
  XSDRAW::StreamContainer aSSC(theDI);
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();

  const Standard_CString arg1 = theArgVec[1];
  //        ****    Default File Root     ****
  if (theNbArgs < 2)
  {
    if (WS->DefaultFileRoot().IsNull()) aSSC.SStream() << "Pas de racine par defaut definie";
    else aSSC.SStream() << "Racine par defaut : " << WS->DefaultFileRoot()->ToCString();
    aSSC.SStream() << "Pour changer :  filedef newdef";
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
  XSDRAW::StreamContainer aSSC(theDI);
  (void)theNbArgs;
  (void)theArgVec;
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  //        ****    EvalFile          ****
  if (!WS->HasModel())
  {
    aSSC.SStream() << "Pas de Modele charge, abandon";
    return 1;
  }

  aSSC.SStream() << "Evaluation avec Memorisation des resultats";
  WS->EvaluateFile();
  Standard_Integer nbf = WS->NbFiles();
  for (Standard_Integer i = 1; i <= nbf; i++)
  {
    Handle(Interface_InterfaceModel) mod = WS->FileModel(i);
    if (mod.IsNull())
    {
      aSSC.SStream() << "Modele " << i << " Model non genere ..."; continue;
    }
    TCollection_AsciiString name = WS->FileName(i);
    aSSC.SStream() << "Fichier n0 " << i << " Nb Entites : " << mod->NbEntities() << "  Nom: ";
    aSSC.SStream() << name;
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
  XSDRAW::StreamContainer aSSC(theDI);
  (void)theNbArgs;
  (void)theArgVec;
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
  XSDRAW::StreamContainer aSSC(theDI);
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();

  //        ****    Split              ****
  IFSelect_ReturnStatus stat = IFSelect_RetVoid;
  if (theNbArgs < 2) aSSC.SStream() << "Split : derniere liste de dispatches definie";
  else
  {
    WS->ClearShareOut(Standard_True);
    for (Standard_Integer i = 1; i < theNbArgs; i++)
    {
      DeclareAndCast(IFSelect_Dispatch, disp, WS->NamedItem(theArgVec[i]));
      if (disp.IsNull())
      {
        aSSC.SStream() << "Pas un dispatch:" << theArgVec[i] << ", Splitt abandonne";
        stat = IFSelect_RetError;
      }
      else WS->SetActive(disp, Standard_True);
    }
  }
  if (stat == IFSelect_RetError)
    return stat;
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
  XSDRAW::StreamContainer aSSC(theDI);
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();

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
    if (theNbArgs < 2) aSSC.SStream() << "Donner un Mode - ";
    aSSC.SStream() << "Modes possibles : l  list, c compute, u undo, f forget";
    if (mode == '?')
      return 0;
    else
      return 1;
  }
  if (!WS->SetRemaining(numod))
    return 0;
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
  XSDRAW::StreamContainer aSSC(theDI);
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();

  const Standard_CString arg1 = theArgVec[1];
  const Standard_CString arg2 = theArgVec[2];
  //        ****    SetModelContent    ****
  if (theNbArgs < 3)
  {
    aSSC.SStream() << "Donner nom selection et mode (k=keep,r=remove)";
    return 1;
  }
  Standard_Boolean keepmode;
  DeclareAndCast(IFSelect_Selection, sel, WS->NamedItem(arg1));
  if (sel.IsNull())
  {
    aSSC.SStream() << "Pas de Selection de Nom : " << arg1;
    return 1;
  }
  if (arg2[0] == 'k')
  {
    aSSC.SStream() << " -- SetContent keep ..."; keepmode = Standard_True;
  }
  else if (arg2[0] == 'r')
  {
    aSSC.SStream() << " -- SetContent remove ..."; keepmode = Standard_False;
  }
  else
  {
    aSSC.SStream() << "Donner nom selection et mode (k=keep,r=remove)";
    return 1;
  }

  if (WS->SetModelContent(sel, keepmode)) aSSC.SStream() << " Done";
  else aSSC.SStream() << " Result empty, ignored";
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
  XSDRAW::StreamContainer aSSC(theDI);
  (void)theNbArgs;
  (void)theArgVec;
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  //        ****    ListModif          ****
  WS->ListFinalModifiers(Standard_True);
  WS->ListFinalModifiers(Standard_False);
  return 0;
}

//=======================================================================
//function : fun41
//purpose  :
//=======================================================================
static Standard_Integer fun41(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  XSDRAW::StreamContainer aSSC(theDI);
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();

  const Standard_CString arg1 = theArgVec[1];
  //        ****    Modifier           ****
  if (theNbArgs < 2)
  {
    aSSC.SStream() << "Donner Nom du Modifier";
    return 1;
  }
  DeclareAndCast(IFSelect_GeneralModifier, modif, WS->NamedItem(arg1));
  if (modif.IsNull())
  {
    aSSC.SStream() << "Pas de Modifier de Nom : " << arg1;
    return 0;
  }
  Handle(IFSelect_IntParam) low, up;

  Handle(IFSelect_Dispatch) disp = modif->Dispatch();
  aSSC.SStream() << "Modifier : " << arg1 << " Label : " << modif->Label();
  Standard_Integer rank = WS->ModifierRank(modif);
  if (modif->IsKind(STANDARD_TYPE(IFSelect_Modifier)))
    aSSC.SStream() << "Model Modifier n0." << rank;
  else aSSC.SStream() << "File Modifier n0." << rank;
  if (disp.IsNull()) aSSC.SStream() << "  Applique a tous les Dispatchs";
  else
  {
    aSSC.SStream() << "  Dispatch : " << disp->Label();
    if (WS->HasName(disp)) aSSC.SStream() << " - Nom:" << WS->Name(disp)->ToCString();

  }

  Handle(IFSelect_Selection) sel = modif->Selection();
  if (!sel.IsNull()) aSSC.SStream() << "  Selection : " << sel->Label();
  if (WS->HasName(sel)) aSSC.SStream() << " - Nom:" << WS->Name(sel)->ToCString();

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
  XSDRAW::StreamContainer aSSC(theDI);
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();

  const Standard_CString arg1 = theArgVec[1];
  const Standard_CString arg2 = theArgVec[2];
  //        ****    ModifSel           ****
  if (theNbArgs < 2)
  {
    aSSC.SStream() << "Donner Nom Modifier; + Nom Selection optionnel\n"
      << "Selection pour Mettre une Selection, sinon Annule";
    return 1;
  }
  DeclareAndCast(IFSelect_GeneralModifier, modif, WS->NamedItem(arg1));
  if (modif.IsNull())
  {
    aSSC.SStream() << "Pas un nom de Modifier : " << arg1;
    return 1;
  }
  Handle(IFSelect_Selection) sel;
  if (arg2[0] != '\0')
  {
    sel = GetCasted(IFSelect_Selection, WS->NamedItem(arg2));
    if (sel.IsNull())
    {
      aSSC.SStream() << "Pas un nom de Selection : " << arg2;
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
  XSDRAW::StreamContainer aSSC(theDI);
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();

  const Standard_CString arg1 = theArgVec[1];
  const Standard_CString arg2 = theArgVec[2];
  //        ****    SetAppliedModifier           ****
  if (theNbArgs < 2)
  {
    aSSC.SStream() << "Donner Nom Modifier; + Nom Dispatch ou Transformer optionnel :\n"
      << " - rien : tous Dispatches\n - Dispatch : ce Dispatch seul\n"
      << " - Transformer : pas un Dispatch mais un Transformer";
    return 1;
  }
  DeclareAndCast(IFSelect_GeneralModifier, modif, WS->NamedItem(arg1));
  if (modif.IsNull())
  {
    aSSC.SStream() << "Pas un nom de Modifier : " << arg1;
    return 1;
  }
  Handle(Standard_Transient) item;
  if (arg2[0] != '\0')
  {
    item = WS->NamedItem(arg2);
    if (item.IsNull())
    {
      aSSC.SStream() << "Pas un nom connu : " << arg2;
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
  XSDRAW::StreamContainer aSSC(theDI);
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();

  const Standard_CString arg1 = theArgVec[1];
  //        ****    ResetApplied (modifier)    ****
  if (theNbArgs < 2)
  {
    aSSC.SStream() << "Designer un modifier";
    return 1;
  }
  DeclareAndCast(IFSelect_GeneralModifier, modif, WS->NamedItem(arg1));
  if (modif.IsNull())
  {
    aSSC.SStream() << "Pas un nom de Modifier : " << arg1;
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
  XSDRAW::StreamContainer aSSC(theDI);
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();

  const Standard_CString arg1 = theArgVec[1];
  const Standard_CString arg2 = theArgVec[2];
  const Standard_CString arg3 = theArgVec[3];
  //        ****    ModifMove         ****
  if (theNbArgs < 4)
  {
    aSSC.SStream() << "modifmove MF rang1 rang2, M pour Model F pour File";
    return 1;
  }
  Standard_Boolean formodel;
  if (arg1[0] == 'm' || arg1[0] == 'M') formodel = Standard_True;
  else if (arg1[0] == 'f' || arg1[0] == 'F') formodel = Standard_False;
  else
  {
    aSSC.SStream() << "preciser M pour Model, F pour File";
    return 1;
  }
  Standard_Integer before = atoi(arg2);
  Standard_Integer after = atoi(arg3);
  if (before == 0 || after == 0)
  {
    aSSC.SStream() << "Donner 2 Entiers Positifs";
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
  XSDRAW::StreamContainer aSSC(theDI);
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  const Standard_CString arg1 = theArgVec[1];
  const Standard_CString arg2 = theArgVec[2];
  //        ****    DispSel           ****
  if (theNbArgs < 3)
  {
    aSSC.SStream() << "Donner Noms Dispatch et Selection Finale";
    return 1;
  }
  DeclareAndCast(IFSelect_Dispatch, disp, WS->NamedItem(arg1));
  if (disp.IsNull())
  {
    aSSC.SStream() << "Pas un nom de Dispatch : " << arg1;
    return 1;
  }
  DeclareAndCast(IFSelect_Selection, sel, WS->NamedItem(arg2));
  if (sel.IsNull())
  {
    aSSC.SStream() << "Pas un nom de Selection : " << arg2;
    return 1;
  }
  if (!WS->SetItemSelection(disp, sel))
    return 1;
  return 0;
}

//=======================================================================
//function : fun56
//purpose  :
//=======================================================================
static Standard_Integer fun56(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  XSDRAW::StreamContainer aSSC(theDI);
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();

  const Standard_CString arg1 = theArgVec[1];
  //        ****    Dispatch           ****
  if (theNbArgs < 2)
  {
    aSSC.SStream() << "Donner Nom du Dispatch";
    return 1;
  }
  DeclareAndCast(IFSelect_Dispatch, disp, WS->NamedItem(arg1));
  if (disp.IsNull())
  {
    aSSC.SStream() << "Pas un dispatch : " << arg1;
    return 1;
  }
  Standard_Integer num = WS->DispatchRank(disp);
  aSSC.SStream() << "Dispatch de Nom : " << arg1 << " , en ShareOut, Numero " << num << " : ";
  Handle(IFSelect_Selection) sel = WS->ItemSelection(disp);
  Handle(TCollection_HAsciiString) selname = WS->Name(sel);
  if (sel.IsNull())  aSSC.SStream() << "Pas de Selection Finale";
  else if (selname.IsNull()) aSSC.SStream() << "Selection Finale : #" << WS->ItemIdent(sel);
  else aSSC.SStream() << "Selection Finale : " << selname->ToCString();
  if (disp->HasRootName()) aSSC.SStream() << "-- Racine nom de fichier : "
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
  XSDRAW::StreamContainer aSSC(theDI);
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();

  const Standard_CString arg1 = theArgVec[1];
  //        ****    Remove           ****
  if (theNbArgs < 2)
  {
    aSSC.SStream() << "Give Name to Remove !";
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
  XSDRAW::StreamContainer aSSC(theDI);
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();

  const Standard_CString arg1 = theArgVec[1];
  //        ****    EvalDisp          ****
  if (theNbArgs < 3)
  {
    aSSC.SStream() << "evaldisp mode disp [disp ...] :  Mode + Name(s) of Dispatch(es). Mode:\n"
      << "  0 brief  1 +forgotten ents  2 +duplicata  3 1+2"
      << "See also : evaladisp  writedisp  xsplit";
    return 0;
  }
  Standard_Boolean OK = Standard_True;
  Standard_Integer i, mode = atoi(arg1);  aSSC.SStream() << " Mode " << mode << "\n";
  for (i = 2; i < theNbArgs; i++)
  {
    DeclareAndCast(IFSelect_Dispatch, disp, WS->NamedItem(theArgVec[i]));
    if (disp.IsNull())
    {
      aSSC.SStream() << "Not a dispatch:" << theArgVec[i]; OK = Standard_False;
    }
  }
  if (!OK)
  {
    aSSC.SStream() << "Some of the parameters are not correct";
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
//function : fun59
//purpose  :
//=======================================================================
static Standard_Integer fun59(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  XSDRAW::StreamContainer aSSC(theDI);
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();

  const Standard_CString arg1 = theArgVec[1];
  //        ****    EvalComplete      ****
  Standard_Integer mode = 0;
  if (theNbArgs < 2) aSSC.SStream() << " -- mode par defaut 0\n";
  else
  {
    mode = atoi(arg1); aSSC.SStream() << " -- mode : " << mode;
  }
  WS->EvaluateComplete(mode);
  return 0;
}

//=======================================================================
//function : fun60
//purpose  :
//=======================================================================
static Standard_Integer fun60(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  XSDRAW::StreamContainer aSSC(theDI);
  (void)theNbArgs;
  (void)theArgVec;
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  //        ****    LastRunCheckList    ****
  Interface_CheckIterator chlist = WS->LastRunCheckList();
  Handle(IFSelect_CheckCounter) counter = new IFSelect_CheckCounter(0);
  counter->Analyse(chlist, WS->Model(), Standard_False);
  counter->PrintCount(aSSC.SStream());
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
  XSDRAW::StreamContainer aSSC(theDI);
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();

  const Standard_CString arg1 = theArgVec[1];
  //        ****    RunTransformer    ****
  if (theNbArgs < 2)
  {
    aSSC.SStream() << "Donner Nom de Transformer";
    return 1;
  }
  DeclareAndCast(IFSelect_Transformer, tsf, WS->NamedItem(arg1));
  Standard_Integer effect = WS->RunTransformer(tsf);
  switch (effect)
  {
    case -4: aSSC.SStream() << "Edition sur place, nouveau Protocole, erreur recalcul graphe"; break;
    case -3: aSSC.SStream() << "Erreur, Transformation ignoree"; break;
    case -2: aSSC.SStream() << "Erreur sur edition sur place, risque de corruption (verifier)"; break;
    case -1: aSSC.SStream() << "Erreur sur edition locale, risque de corruption (verifier)"; break;
    case  0:
      if (tsf.IsNull()) aSSC.SStream() << "Erreur, pas un Transformer: " << arg1;
      else aSSC.SStream() << "Execution non faite";
      break;
    case  1: aSSC.SStream() << "Transformation locale (graphe non touche)"; break;
    case  2: aSSC.SStream() << "Edition sur place (graphe recalcule)";  break;
    case  3: aSSC.SStream() << "Modele reconstruit"; break;
    case  4: aSSC.SStream() << "Edition sur place, nouveau Protocole";  break;
    case  5: aSSC.SStream() << "Nouveau Modele avec nouveau Protocole"; break;
    default: break;
  }
  return ((effect > 0) ? IFSelect_RetDone : IFSelect_RetFail);
}

//=======================================================================
//function : fun70
//purpose  :
//=======================================================================
static Standard_Integer fun70(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  XSDRAW::StreamContainer aSSC(theDI);
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();

  const Standard_CString arg1 = theArgVec[1];
  //        ****    SelToggle         ****
  if (theNbArgs < 2)
  {
    aSSC.SStream() << "Donner Nom de Selection";
    return 1;
  }
  DeclareAndCast(IFSelect_Selection, sel, WS->NamedItem(arg1));
  if (!WS->ToggleSelectExtract(sel))
  {
    aSSC.SStream() << "Pas une SelectExtract : " << arg1;
    return 1;
  }
  if (WS->IsReversedSelectExtract(sel)) aSSC.SStream() << arg1 << " a present Reversed";
  else aSSC.SStream() << arg1 << " a present Directe";
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
  XSDRAW::StreamContainer aSSC(theDI);
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();

  const Standard_CString arg1 = theArgVec[1];
  const Standard_CString arg2 = theArgVec[2];
  //        ****    SelInput          ****
  if (theNbArgs < 3)
  {
    aSSC.SStream() << "Donner Noms Selections cible et input";
    return 1;
  }
  DeclareAndCast(IFSelect_Selection, sel, WS->NamedItem(arg1));
  DeclareAndCast(IFSelect_Selection, sou, WS->NamedItem(arg2));
  if (sel.IsNull() || sou.IsNull())
  {
    aSSC.SStream() << "Incorrect : " << arg1 << "," << arg2;
    return 1;
  }
  if (!WS->SetInputSelection(sel, sou))
  {
    aSSC.SStream() << "Nom incorrect ou Selection " << arg1 << " ni Extract ni Deduct";
    return 1;
  }
  return 0;
}

//=======================================================================
//function : fun77
//purpose  :
//=======================================================================
static Standard_Integer fun77(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  XSDRAW::StreamContainer aSSC(theDI);
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();

  const Standard_CString arg1 = theArgVec[1];
  const Standard_CString arg2 = theArgVec[2];
  //        ****    SelControlMain       ****
  if (theNbArgs < 3)
  {
    aSSC.SStream() << "Donner Noms de Control et MainInput";
    return 1;
  }
  DeclareAndCast(IFSelect_Selection, sel, WS->NamedItem(arg1));
  DeclareAndCast(IFSelect_Selection, selmain, WS->NamedItem(arg2));
  if (WS->SetControl(sel, selmain, Standard_True))
    return 0;
  aSSC.SStream() << "Nom incorrect ou Selection " << arg1 << " pas de type Control";
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
  XSDRAW::StreamContainer aSSC(theDI);
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();

  const Standard_CString arg1 = theArgVec[1];
  const Standard_CString arg2 = theArgVec[2];
  //        ****    SelControlSecond       ****
  if (theNbArgs < 3)
  {
    aSSC.SStream() << "Donner Noms de Control et SecondInput";
    return 1;
  }
  DeclareAndCast(IFSelect_Selection, sel, WS->NamedItem(arg1));
  DeclareAndCast(IFSelect_Selection, seldif, WS->NamedItem(arg2));
  if (WS->SetControl(sel, seldif, Standard_False))  return 0;
  aSSC.SStream() << "Nom incorrect ou Selection " << arg1 << " pas de type Control";
  return 1;
}

//=======================================================================
//function : fun80
//purpose  :
//=======================================================================
static Standard_Integer fun80(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  XSDRAW::StreamContainer aSSC(theDI);
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();

  const Standard_CString arg1 = theArgVec[1];
  const Standard_CString arg2 = theArgVec[2];
  //        ****    SelCombAdd        ****
  if (theNbArgs < 3)
  {
    aSSC.SStream() << "Donner n0 Combine et une Input";
    return 1;
  }
  DeclareAndCast(IFSelect_Selection, sel, WS->NamedItem(arg1));
  DeclareAndCast(IFSelect_Selection, seladd, WS->NamedItem(arg2));
  if (WS->CombineAdd(sel, seladd))
    return 0;
  aSSC.SStream() << "Nom incorrect ou Selection " << arg1 << " pas Combine";
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
  XSDRAW::StreamContainer aSSC(theDI);
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();

  const Standard_CString arg1 = theArgVec[1];
  const Standard_CString arg2 = theArgVec[2];
  //        ****    SelCombRem        ****
  if (theNbArgs < 3)
  {
    aSSC.SStream() << "Donner n0 Combine et RANG a supprimer";
    return 1;
  }
  DeclareAndCast(IFSelect_Selection, sel, WS->NamedItem(arg1));
  DeclareAndCast(IFSelect_Selection, inp, WS->NamedItem(arg2));
  if (WS->CombineRemove(sel, inp))
    return 0;
  aSSC.SStream() << "Nom incorrect ou Selection " << arg1 << " ni Union ni Intersection";
  return 1;
}

//=======================================================================
//function : fun91
//purpose  :
//=======================================================================
static Standard_Integer fun91(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  XSDRAW::StreamContainer aSSC(theDI);
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();

  const Standard_CString arg1 = theArgVec[1];
  //        ****    SetPointed (edit) / SetList (edit)    ****
  if (theNbArgs < 2)
  {
    aSSC.SStream() << "Donner NOM SelectPointed + Option(s) :\n"
      << " aucune : liste des entites pointees\n"
      << " 0: Clear  +nn ajout entite nn  -nn enleve nn  /nn toggle nn";
    return 1;
  }
  DeclareAndCast(IFSelect_SelectPointed, sp, WS->NamedItem(arg1));
  if (sp.IsNull())
  {
    aSSC.SStream() << "Pas une SelectPointed:" << arg1;
    return 1;
  }
  const Handle(Interface_InterfaceModel)& model = WS->Model();  // pour Print
  if (theNbArgs == 2)
  {    // listage simple
    Standard_Integer nb = sp->NbItems();
    aSSC.SStream() << " SelectPointed : " << arg1 << " : " << nb << " Items :";
    for (Standard_Integer i = 1; i <= nb; i++)
    {
      Handle(Standard_Transient) pointed = sp->Item(i);
      Standard_Integer id = WS->StartingNumber(pointed);
      if (id == 0) aSSC.SStream() << " (inconnu)";
      else
      {
        aSSC.SStream() << "  "; model->Print(pointed, aSSC.SStream());
      }
    }
    return 0;
  }

  for (Standard_Integer ia = 2; ia < theNbArgs; ia++)
  {
    const TCollection_AsciiString argi = theArgVec[ia];
    Standard_Integer id = WS->NumberFromLabel(&(argi.ToCString())[1]);
    if (id == 0)
    {
      if (!argi.IsEqual("0")) aSSC.SStream() << "Incorrect,ignore:" << argi;
      else
      {
        aSSC.SStream() << "Clear SelectPointed"; sp->Clear();
      }
    }
    else if (argi.Value(1) == '-')
    {
      Handle(Standard_Transient) item = WS->StartingEntity(id);
      if (sp->Remove(item)) aSSC.SStream() << "Removed:no." << id;
      else aSSC.SStream() << " Echec Remove " << id;
      aSSC.SStream() << ": ";
      model->Print(item, aSSC.SStream());
    }
    else if (argi.Value(1) == '/')
    {
      Handle(Standard_Transient) item = WS->StartingEntity(id);
      if (sp->Remove(item)) aSSC.SStream() << "Toggled:n0." << id;
      else aSSC.SStream() << " Echec Toggle " << id;
      aSSC.SStream() << ": ";
      model->Print(item, aSSC.SStream());
    }
    else if (argi.Value(1) == '+')
    {
      Handle(Standard_Transient) item = WS->StartingEntity(id);
      if (sp->Add(item)) aSSC.SStream() << "Added:no." << id;
      else aSSC.SStream() << " Echec Add " << id;
      aSSC.SStream() << ": ";
      model->Print(item, aSSC.SStream());
    }
    else
    {
      aSSC.SStream() << "Ignore:" << argi << " , donner n0 PRECEDE de + ou - ou /";
    }
  }
  return 0;
}

//=======================================================================
//function : fun_editlist
//purpose  :
//=======================================================================
static Standard_Integer fun_editlist(Draw_Interpretor& theDI,
                                     Standard_Integer theNbArgs,
                                     const char** theArgVec)
{
  XSDRAW::StreamContainer aSSC(theDI);
  if (theNbArgs < 2)
  {
    aSSC.SStream() << "Give the name of an EditForm or an Editor";
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
    aSSC.SStream() << "Print EditForm " << arg1;
    edt = edf->Editor();
    if (theNbArgs < 3)
    {
      //       DEFINITIONS : Editor (direct ou via EditForm)

      if (edt.IsNull()) edt = GetCasted(IFSelect_Editor, WS->NamedItem(arg1));
      if (edt.IsNull())
        return 0;

      aSSC.SStream() << "Editor, Label : " << edt->Label();
      aSSC.SStream() << " --  Names (short - complete) + Labels of Values";
      edt->PrintNames(aSSC.SStream());
      aSSC.SStream() << " --  Definitions  --";
      edt->PrintDefs(aSSC.SStream());
      if (!edf.IsNull())
      {
        edf->PrintDefs(aSSC.SStream());
        aSSC.SStream() << "To display values, add an option : o original  f final  m modified";
      }

      return 0;

    }
    else
    {
      char opt = arg2[0];
      Standard_Integer what = 0;
      if (opt == 'o') what = -1;
      else if (opt == 'f') what = 1;

      edf->PrintValues(aSSC.SStream(), what, Standard_False);
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
  XSDRAW::StreamContainer aSSC(theDI);
  if (theNbArgs < 2)
  {
    aSSC.SStream() << "Give the name of an EditForm [+ name of Value  else all]";
    return 1;
  }
  const Standard_CString arg1 = theArgVec[1];
  const Standard_CString arg2 = theArgVec[2];
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  DeclareAndCast(IFSelect_EditForm, edf, WS->NamedItem(arg1));
  if (edf.IsNull())
  {
    aSSC.SStream() << "Not an EditForm : " << arg1;
    return 1;
  }
  if (theNbArgs < 3)
  {
    edf->ClearEdit(); aSSC.SStream() << "All Modifications Cleared";
  }
  else
  {
    Standard_Integer num = edf->NameNumber(arg2);
    if (num == 0) aSSC.SStream() << "Unknown Value Name : " << arg2;
    if (num < 0) aSSC.SStream() << "Not Extracted Value Name : " << arg2;
    if (num <= 0)
      return 1;
    if (!edf->IsModified(num))
    {
      aSSC.SStream() << "Value " << arg2 << " was not modified";
      return 0;
    }
    edf->ClearEdit(num);
    aSSC.SStream() << "Modification on Value " << arg2 << " Cleared";
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
  XSDRAW::StreamContainer aSSC(theDI);
  if (theNbArgs < 2)
  {
    aSSC.SStream() << "Give the name of an EditForm [+ option keep to re-apply edited values]";
    return 1;
  }
  const Standard_CString arg1 = theArgVec[1];
  const Standard_CString arg2 = theArgVec[2];
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  DeclareAndCast(IFSelect_EditForm, edf, WS->NamedItem(arg1));
  if (edf.IsNull())
  {
    aSSC.SStream() << "Not an EditForm : " << arg1;
    return 1;
  }

  Handle(Standard_Transient) ent = edf->Entity();
  Handle(Interface_InterfaceModel) model = edf->Model();
  if (!model.IsNull())
  {
    if (ent.IsNull()) aSSC.SStream() << "Applying modifications on loaded model";
    else
    {
      aSSC.SStream() << "Applying modifications on loaded entity : ";
      model->PrintLabel(ent, aSSC.SStream());
    }
  }
  else aSSC.SStream() << "Applying modifications";

  if (!edf->ApplyData(edf->Entity(), edf->Model()))
  {
    aSSC.SStream() << "Modifications could not be applied";
    return 1;
  }
  aSSC.SStream() << "Modifications have been applied";

  Standard_Boolean stat = Standard_True;
  if (theNbArgs > 2 && arg2[0] == 'k') stat = Standard_False;
  if (stat)
  {
    edf->ClearEdit();
    aSSC.SStream() << "Edited values are cleared";
  }
  else aSSC.SStream() << "Edited values are kept for another loading/applying";

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
  XSDRAW::StreamContainer aSSC(theDI);
  if (theNbArgs < 2)
  {
    aSSC.SStream() << "Give the name of an EditForm [+ Entity-Ident]";
    return 1;
  }
  const Standard_CString arg1 = theArgVec[1];
  const Standard_CString arg2 = theArgVec[2];
  Handle(XSControl_WorkSession) WS = XSDRAWBase::Session();
  DeclareAndCast(IFSelect_EditForm, edf, WS->NamedItem(arg1));
  if (edf.IsNull())
  {
    aSSC.SStream() << "Not an EditForm : " << arg1;
    return 1;
  }

  Standard_Integer num = (theNbArgs < 3 ? 0 : WS->NumberFromLabel(arg2));
  Standard_Boolean stat = Standard_False;
  if (theNbArgs < 3)
  {
    aSSC.SStream() << "EditForm " << arg1 << " : Loading Model";
    stat = edf->LoadModel(WS->Model());
  }
  else if (num <= 0)
  {
    aSSC.SStream() << "Not an entity ident : " << arg2;
    return 1;
  }
  else
  {
    aSSC.SStream() << "EditForm " << arg1 << " : Loading Entity " << arg2;
    stat = edf->LoadData(WS->StartingEntity(num), WS->Model());
  }

  if (!stat)
  {
    aSSC.SStream() << "Loading not done";
    return 1;
  }
  aSSC.SStream() << "Loading done";
  return 0;
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
  //theDI.Add("writesel", "file:string sel:Selection : Write Selected (no split)", __FILE__, fun5, aGroup);
  theDI.Add("writeent", "file:string  n1ent n2ent...:integer : Write Entite(s) (no split)", __FILE__, fun6, aGroup);
  theDI.Add("writent", "file:string  n1ent n2ent...:integer : Write Entite(s) (no split)", __FILE__, fun6, aGroup);
  theDI.Add("elabel", "nument:integer   : Displays Label Model of an entity", __FILE__, fun7, aGroup);
  theDI.Add("enum", "label:string  : Displays entities n0.s of which Label Model ends by..", __FILE__, fun8, aGroup);

  theDI.Add("listtypes", "List nb entities per type. Optional selection name  else all model", __FILE__, fun9, aGroup);
  //theDI.Add("count", "Count : counter [selection]", __FILE__, funcount, aGroup);
  //theDI.Add("listcount", "List Counted : counter [selection [nument]]", __FILE__, funcount, aGroup);
  //theDI.Add("sumcount", "Summary Counted : counter [selection [nument]]", __FILE__, funcount, aGroup);
  theDI.Add("signtype", "Sign Type [newone]", __FILE__, funsigntype, aGroup);
  theDI.Add("signcase", "signature : displays possible cases", __FILE__, funsigncase, aGroup);

  theDI.Add("estatus", "ent/nument : displays status of an entity", __FILE__, fun10, aGroup);
  theDI.Add("data", "Data (DumpModel); whole help : data tout court", __FILE__, fun11, aGroup);
  theDI.Add("entity", "give n0 ou id of entity [+ level]", __FILE__, fundumpent, aGroup);
  theDI.Add("signature", "signature name + n0/ident entity", __FILE__, funsign, aGroup);
  theDI.Add("queryparent", " give 2 n0s/labels of entities : dad son", __FILE__, funqp, aGroup);

  theDI.Add("dumpshare", "Dump Share (dispatches, IntParams)", __FILE__, fun12, aGroup);
  theDI.Add("listitems", "List Items [label else all]  ->Type,Label[,Name]", __FILE__, fun13, aGroup);
  //theDI.Add("integer", "value:integer : cree un IntParam", __FILE__, fun14, aGroup);
  theDI.Add("setint", "name:IntParam   newValue:integer  : Change valeur IntParam", __FILE__, fun15, aGroup);
  //theDI.Add("text", "value:string  : cree un TextParam", __FILE__, fun16, aGroup);
  theDI.Add("settext", "Name:TextParam  newValue:string   : Change valeur TextParam", __FILE__, fun17, aGroup);
  theDI.Add("dumpsel", "Dump Selection suivi du Nom de la Selection a dumper", __FILE__, fun19, aGroup);
  //theDI.Add("evalsel", "name:Selection [num/sel]  : Evalue une Selection", __FILE__, fun20, aGroup);
  //theDI.Add("givelist", "num/sel [num/sel ...]  : Evaluates GiveList", __FILE__, fun20, aGroup);
  //theDI.Add("giveshort", "num/sel [num/sel ...]  : GiveList in short form", __FILE__, fun20, aGroup);
  //theDI.Add("givepointed", "num/sel [num/sel ...]  : GiveList to fill a SelectPointed", __FILE__, fun20, aGroup);
  //theDI.Add("makelist", "listname [givelist] : Makes a List(SelectPointed) from GiveList", __FILE__, fun20, aGroup);
  //theDI.Add("givecount", "num/sel [num/sel ...]  : Counts GiveList", __FILE__, fun20c, aGroup);
  //theDI.Add("selsuite", "sel sel ...  : Creates a SelectSuite", __FILE__, funselsuite, aGroup);
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
  //theDI.Add("dispone", "cree DispPerOne", __FILE__, fun_dispone, aGroup);
  //theDI.Add("dispglob", "cree DispGlobal", __FILE__, fun_dispglob, aGroup);
  //theDI.Add("dispcount", "count:IntParam  : cree DispPerCount", __FILE__, fun_dispcount, aGroup);
  //theDI.Add("dispfile", "files:IntParam  : cree DispPerFiles", __FILE__, fun_dispfiles, aGroup);
  //theDI.Add("dispsign", "sign:Signature  : cree DispPerSignature", __FILE__, fun_dispsign, aGroup);
  theDI.Add("dumpdisp", "disp:Dispatch   : Affiche le Statut d'un Dispatch", __FILE__, fun56, aGroup);

  theDI.Add("xremove", "nom  : Remove a Control Item de la Session", __FILE__, fun57, aGroup);
  theDI.Add("evaldisp", "mode=[0-3]  disp:Dispatch  : Evaluates one or more Dispatch(es)", __FILE__, fun58, aGroup);
  //theDI.Add("evaladisp", "mode=[0-3]  disp:Dispatch [givelist]  : Evaluates a Dispatch (on a GiveList)", __FILE__, fun_evaladisp, aGroup);
  //theDI.Add("writedisp", "filepattern  disp:Dispatch [givelist]  : Writes Entities by Splitting by a Dispatch", __FILE__, fun_writedisp, aGroup);
  theDI.Add("evalcomplete", "Evaluation Complete de la Repartition", __FILE__, fun59, aGroup);

  theDI.Add("runcheck", "affiche LastRunCheckList (write,modif)", __FILE__, fun60, aGroup);
  theDI.Add("runtranformer", "transf:Transformer  : Applique un Transformer", __FILE__, fun61, aGroup);
  //theDI.Add("copy", "cree TransformStandard, option Copy, vide", __FILE__, fun62, aGroup);
  //theDI.Add("onthespot", "cree TransformStandard, option OntheSpot, vide", __FILE__, fun63, aGroup);
  //theDI.Add("runcopy", "modif:ModelModifier [givelist] : Run <modif> via TransformStandard option Copy", __FILE__, fun6465, aGroup);
  //theDI.Add("runonthespot", "modif:ModelModifier [givelist] : Run <modif> via TransformStandard option OnTheSpot", __FILE__, fun6465, aGroup);
  //theDI.Add("reorder", "[f ou t] reordonne le modele", __FILE__, fun66, aGroup);

  theDI.Add("toggle", "sel:Selection genre Extract  : Toggle Direct/Reverse", __FILE__, fun70, aGroup);
  theDI.Add("input", "sel:Selection genre Deduct ou Extract  input:Selection  : Set Input", __FILE__, fun71, aGroup);
  //theDI.Add("modelroots", "cree SelectModelRoots", __FILE__, fun72, aGroup);
  //theDI.Add("range", "options... : cree SelectRange ...; tout court pour help", __FILE__, fun73, aGroup);
  //theDI.Add("roots", "cree SelectRoots (local roots)", __FILE__, fun74, aGroup);
  //theDI.Add("shared", "cree SelectShared", __FILE__, fun75, aGroup);
  //theDI.Add("diff", "[main:Selection diff:Selection]  : cree SelectDiff", __FILE__, fun76, aGroup);
  theDI.Add("selmain", "sel:Selection genre Control  main:Selection  : Set Main Input", __FILE__, fun77, aGroup);
  theDI.Add("selsecond", "sel:Selection genre Control  sec:Selection   : Set Second Input", __FILE__, fun78, aGroup);
  //theDI.Add("modelall", "cree SelectModelAll", __FILE__, fun79, aGroup);
  theDI.Add("seladd", "sel:Selection genre Combine  input:Selection  : Add Selection", __FILE__, fun80, aGroup);
  theDI.Add("selrem", "sel:Selection genre Combine  input:Selection  : Remove Selection", __FILE__, fun81, aGroup);
  //theDI.Add("number", "num:IntParam  : Cree SelectEntityNumber", __FILE__, fun82, aGroup);

  //theDI.Add("union", "cree SelectUnion (vide), cf aussi combadd, combrem", __FILE__, fun83, aGroup);
  //theDI.Add("intersect", "cree SelectIntersection (vide), cf aussi combadd, combrem", __FILE__, fun84, aGroup);
  //theDI.Add("typexact", "type:string  : cree SelectTextType Exact", __FILE__, fun85, aGroup);
  //theDI.Add("errors", "cree SelectErrorEntities (from file)", __FILE__, fun86, aGroup);
  //theDI.Add("unknown", "cree SelectUnknownEntities", __FILE__, fun87, aGroup);
  //theDI.Add("sharing", "cree SelectSharing", __FILE__, fun88, aGroup);
  //theDI.Add("typecontain", "type:string  : cree SelectTextType Contains", __FILE__, fun89, aGroup);
  //theDI.Add("pointed", "cree SelectPointed [num/sel num/sel]", __FILE__, fun90, aGroup);
  theDI.Add("setpointed", "sel:SelectPointed  : edition SelectPointed. tout court pour help", __FILE__, fun91, aGroup);
  theDI.Add("setlist", "sel:SelectPointed  : edition SelectPointed. tout court pour help", __FILE__, fun91, aGroup);
  //theDI.Add("incorrect", "cree SelectIncorrectEntities (computed)", __FILE__, fun92, aGroup);

  //theDI.Add("signsel", "sign:Signature|cnt:Counter text:string [e(D)|c] : cree SelectSignature", __FILE__, fun93, aGroup);
  //theDI.Add("signcounter", "sign:Signature : cree SignCounter", __FILE__, fun94, aGroup);
  //theDI.Add("nbselected", "applied:Selection : cree GraphCounter(=NbSelected)", __FILE__, funbselected, aGroup);

  theDI.Add("editlist", "editor or editform : lists defs + values", __FILE__, fun_editlist, aGroup);
  //theDI.Add("editvalue", "editform paramname [newval or .] : lists-changes a value", __FILE__, fun_editvalue, aGroup);
  theDI.Add("editclear", "editform [paramname] : clears edition on all or one param", __FILE__, fun_editclear, aGroup);
  theDI.Add("editload", "editform [entity-id] : loads from model or an entity", __FILE__, fun_editload, aGroup);
  theDI.Add("editapply", "editform [keep] : applies on loaded data", __FILE__, fun_editapply, aGroup);
}
