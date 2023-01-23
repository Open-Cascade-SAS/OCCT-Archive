// Created on: 1992-02-11
// Created by: Christian CAILLET
// Copyright (c) 1992-1999 Matra Datavision
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

#include <StepData_WriterLib.hxx>

#include <StepData_NodeOfWriterLib.hxx>
#include <Standard_NoSuchObject.hxx>
#include <Standard_Transient.hxx>
#include <StepData_ReadWriteModule.hxx>
#include <StepData_Protocol.hxx>
#include <StepData_GlobalNodeOfWriterLib.hxx>

static Handle(StepData_GlobalNodeOfWriterLib) THE_GLOBAL_NODE_OF_LIBS;

static Handle(StepData_Protocol) THE_GLOBAL_PROTOCOL;
static Handle(StepData_NodeOfWriterLib) THE_LAST_NODE_OF_LIBS;

//=======================================================================
// function : SetGlobal
// purpose  :
//=======================================================================
void StepData_WriterLib::SetGlobal(const Handle(StepData_ReadWriteModule)& amodule,
                                   const Handle(StepData_Protocol)& aprotocol)
{
  if (THE_GLOBAL_NODE_OF_LIBS.IsNull()) THE_GLOBAL_NODE_OF_LIBS = new StepData_GlobalNodeOfWriterLib;
  THE_GLOBAL_NODE_OF_LIBS->Add(amodule, aprotocol);
}

//=======================================================================
// function : StepData_WriterLib
// purpose  :
//=======================================================================
StepData_WriterLib::StepData_WriterLib(const Handle(StepData_Protocol)& aprotocol)
{
  Standard_Boolean last = Standard_False;
  if (aprotocol.IsNull()) return;    // PAS de protocole = Lib VIDE
  if (!THE_GLOBAL_PROTOCOL.IsNull()) last =
    (THE_GLOBAL_PROTOCOL == aprotocol);

  if (last) thelist = THE_LAST_NODE_OF_LIBS;
  //  Si Pas d optimisation disponible : construire la liste
  else
  {
    AddProtocol(aprotocol);
    //  Ceci definit l optimisation (pour la fois suivante)
    THE_LAST_NODE_OF_LIBS = thelist;
    THE_GLOBAL_PROTOCOL = aprotocol;
  }
}

//=======================================================================
// function : StepData_WriterLib
// purpose  :
//=======================================================================
StepData_WriterLib::StepData_WriterLib() {}

//=======================================================================
// function : AddProtocol
// purpose  :
//=======================================================================
void StepData_WriterLib::AddProtocol(const Handle(Standard_Transient)& aprotocol)
{
  //  DownCast car Protocol->Resources, meme redefini et utilise dans d autres
  //  librairies, doit toujours renvoyer le type le plus haut
  Handle(StepData_Protocol) aproto = Handle(StepData_Protocol)::DownCast(aprotocol);
  if (aproto.IsNull()) return;

  //  D abord, ajouter celui-ci a la liste : chercher le Node
  Handle(StepData_GlobalNodeOfWriterLib) curr;
  for (curr = THE_GLOBAL_NODE_OF_LIBS; !curr.IsNull(); )
  {        // curr->Next : plus loin
    const Handle(StepData_Protocol)& protocol = curr->Protocol();
    if (!protocol.IsNull())
    {
      //  Match Protocol ?
      if (protocol->DynamicType() == aprotocol->DynamicType())
      {
        if (thelist.IsNull()) thelist = new StepData_NodeOfWriterLib;
        thelist->AddNode(curr);
        break;  // UN SEUL MODULE PAR PROTOCOLE
      }
    }
    curr = curr->Next();  // cette formule est refusee dans "for"
  }
  //  Ensuite, Traiter les ressources
  Standard_Integer nb = aproto->NbResources();
  for (Standard_Integer i = 1; i <= nb; i++)
  {
    AddProtocol(aproto->Resource(i));
  }
  //  Ne pas oublier de desoptimiser
  THE_GLOBAL_PROTOCOL.Nullify();
  THE_LAST_NODE_OF_LIBS.Nullify();
}

//=======================================================================
// function : Clear
// purpose  :
//=======================================================================
void StepData_WriterLib::Clear()
{
  thelist = new StepData_NodeOfWriterLib;
}

//=======================================================================
// function : SetComplete
// purpose  :
//=======================================================================
void StepData_WriterLib::SetComplete()
{
  thelist = new StepData_NodeOfWriterLib;
  //    On prend chacun des Protocoles de la Liste Globale et on l ajoute
  Handle(StepData_GlobalNodeOfWriterLib) curr;
  for (curr = THE_GLOBAL_NODE_OF_LIBS; !curr.IsNull(); )
  {        // curr->Next : plus loin
    const Handle(StepData_Protocol)& protocol = curr->Protocol();
    //    Comme on prend tout tout tout, on ne se preoccupe pas des Ressources !
    if (!protocol.IsNull()) thelist->AddNode(curr);
    curr = curr->Next();  // cette formule est refusee dans "for"
  }
}

//=======================================================================
// function : Select
// purpose  :
//=======================================================================
Standard_Boolean StepData_WriterLib::Select(const Handle(Standard_Transient)& obj,
                                            Handle(StepData_ReadWriteModule)& module,
                                            Standard_Integer& CN) const
{
  module.Nullify();  CN = 0;    // Reponse "pas trouve"
  if (thelist.IsNull()) return Standard_False;
  Handle(StepData_NodeOfWriterLib) curr = thelist;
  for (curr = thelist; !curr.IsNull(); )
  {        // curr->Next : plus loin
    const Handle(StepData_Protocol)& protocol = curr->Protocol();
    if (!protocol.IsNull())
    {
      CN = protocol->CaseNumber(obj);
      if (CN > 0)
      {
        module = curr->Module();
        return Standard_True;
      }
    }
    curr = curr->Next();        // cette formule est refusee dans "for"
  }
  return Standard_False;        // ici, pas trouce
}

//=======================================================================
// function : Start
// purpose  :
//=======================================================================
void StepData_WriterLib::Start()
{
  thecurr = thelist;
}

//=======================================================================
// function : More
// purpose  :
//=======================================================================
Standard_Boolean StepData_WriterLib::More() const
{
  return (!thecurr.IsNull());
}

//=======================================================================
// function : Next
// purpose  :
//=======================================================================
void StepData_WriterLib::Next()
{
  if (!thecurr.IsNull()) thecurr = thecurr->Next();
}

//=======================================================================
// function : Module
// purpose  :
//=======================================================================
const Handle(StepData_ReadWriteModule)& StepData_WriterLib::Module() const
{
  if (thecurr.IsNull()) throw Standard_NoSuchObject("Library from LibCtl");
  return thecurr->Module();
}

//=======================================================================
// function : Protocol
// purpose  :
//=======================================================================
const Handle(StepData_Protocol)& StepData_WriterLib::Protocol() const
{
  if (thecurr.IsNull()) throw Standard_NoSuchObject("Library from LibCtl");
  return thecurr->Protocol();
}
