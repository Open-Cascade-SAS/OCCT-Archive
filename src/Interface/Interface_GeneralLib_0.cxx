// Created on: 1992-02-03
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

#include <Interface_GeneralLib.hxx>

#include <Interface_NodeOfGeneralLib.hxx>
#include <Standard_NoSuchObject.hxx>
#include <Standard_Transient.hxx>
#include <Interface_GeneralModule.hxx>
#include <Interface_Protocol.hxx>
#include <Interface_GlobalNodeOfGeneralLib.hxx>

#include <Standard_NoSuchObject.hxx>

static Handle(Interface_GlobalNodeOfGeneralLib) THE_GLOBAL_NODE_OF_GEN_LIBS;

static Handle(Interface_Protocol) THE_GLOBAL_PROTOCOL;
static Handle(Interface_NodeOfGeneralLib) THE_LAST_NODE_OF_GEN_LIBS;

//=======================================================================
// function : SetGlobal
// purpose  :
//=======================================================================
void Interface_GeneralLib::SetGlobal(const Handle(Interface_GeneralModule)& amodule,
                                     const Handle(Interface_Protocol)& aprotocol)
{
  if (THE_GLOBAL_NODE_OF_GEN_LIBS.IsNull()) THE_GLOBAL_NODE_OF_GEN_LIBS = new Interface_GlobalNodeOfGeneralLib;
  THE_GLOBAL_NODE_OF_GEN_LIBS->Add(amodule, aprotocol);
}

//=======================================================================
// function : Interface_GeneralLib
// purpose  :
//=======================================================================
Interface_GeneralLib::Interface_GeneralLib(const Handle(Interface_Protocol)& aprotocol)
{
  Standard_Boolean last = Standard_False;
  if (aprotocol.IsNull()) return;    // PAS de protocole = Lib VIDE
  if (!THE_GLOBAL_PROTOCOL.IsNull()) last =
    (THE_GLOBAL_PROTOCOL == aprotocol);

  if (last) thelist = THE_LAST_NODE_OF_GEN_LIBS;
  //  Si Pas d optimisation disponible : construire la liste
  else
  {
    AddProtocol(aprotocol);
    //  Ceci definit l optimisation (pour la fois suivante)
    THE_LAST_NODE_OF_GEN_LIBS = thelist;
    THE_GLOBAL_PROTOCOL = aprotocol;
  }
}

//=======================================================================
// function : Interface_GeneralLib
// purpose  :
//=======================================================================
Interface_GeneralLib::Interface_GeneralLib() {}

//=======================================================================
// function : AddProtocol
// purpose  :
//=======================================================================
void Interface_GeneralLib::AddProtocol(const Handle(Standard_Transient)& aprotocol)
{
  //  DownCast car Protocol->Resources, meme redefini et utilise dans d autres
  //  librairies, doit toujours renvoyer le type le plus haut
  Handle(Interface_Protocol) aproto = Handle(Interface_Protocol)::DownCast(aprotocol);
  if (aproto.IsNull()) return;

  //  D abord, ajouter celui-ci a la liste : chercher le Node
  Handle(Interface_GlobalNodeOfGeneralLib) curr;
  for (curr = THE_GLOBAL_NODE_OF_GEN_LIBS; !curr.IsNull(); )
  {        // curr->Next : plus loin
    const Handle(Interface_Protocol)& protocol = curr->Protocol();
    if (!protocol.IsNull())
    {
      //  Match Protocol ?
      if (protocol->DynamicType() == aprotocol->DynamicType())
      {
        if (thelist.IsNull()) thelist = new Interface_NodeOfGeneralLib;
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
  THE_LAST_NODE_OF_GEN_LIBS.Nullify();
}

//=======================================================================
// function : Clear
// purpose  :
//=======================================================================
void Interface_GeneralLib::Clear()
{
  thelist = new Interface_NodeOfGeneralLib;
}

//=======================================================================
// function : SetComplete
// purpose  :
//=======================================================================
void Interface_GeneralLib::SetComplete()
{
  thelist = new Interface_NodeOfGeneralLib;
  //    On prend chacun des Protocoles de la Liste Globale et on l ajoute
  Handle(Interface_GlobalNodeOfGeneralLib) curr;
  for (curr = THE_GLOBAL_NODE_OF_GEN_LIBS; !curr.IsNull(); )
  {        // curr->Next : plus loin
    const Handle(Interface_Protocol)& protocol = curr->Protocol();
    //    Comme on prend tout tout tout, on ne se preoccupe pas des Ressources !
    if (!protocol.IsNull()) thelist->AddNode(curr);
    curr = curr->Next();  // cette formule est refusee dans "for"
  }
}


//=======================================================================
// function : Select
// purpose  :
//=======================================================================
Standard_Boolean Interface_GeneralLib::Select(const Handle(Standard_Transient)& obj,
                                              Handle(Interface_GeneralModule)& module,
                                              Standard_Integer& CN) const
{
  module.Nullify();  CN = 0;    // Reponse "pas trouve"
  if (thelist.IsNull()) return Standard_False;
  Handle(Interface_NodeOfGeneralLib) curr = thelist;
  for (curr = thelist; !curr.IsNull(); )
  {        // curr->Next : plus loin
    const Handle(Interface_Protocol)& protocol = curr->Protocol();
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
void Interface_GeneralLib::Start()
{
  thecurr = thelist;
}

//=======================================================================
// function : More
// purpose  :
//=======================================================================
Standard_Boolean Interface_GeneralLib::More() const
{
  return (!thecurr.IsNull());
}

//=======================================================================
// function : Next
// purpose  :
//=======================================================================
void Interface_GeneralLib::Next()
{
  if (!thecurr.IsNull()) thecurr = thecurr->Next();
}

//=======================================================================
// function : Module
// purpose  :
//=======================================================================
const Handle(Interface_GeneralModule)& Interface_GeneralLib::Module() const
{
  if (thecurr.IsNull()) throw Standard_NoSuchObject("Library from LibCtl");
  return thecurr->Module();
}

//=======================================================================
// function : Protocol
// purpose  :
//=======================================================================
const Handle(Interface_Protocol)& Interface_GeneralLib::Protocol() const
{
  if (thecurr.IsNull()) throw Standard_NoSuchObject("Library from LibCtl");
  return thecurr->Protocol();
}
