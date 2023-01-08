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

#include <BRep_Builder.hxx>
#include <BRepTools.hxx>
#include <Geom_Geometry.hxx>
#include <IFSelect_Act.hxx>
#include <IFSelect_Functions.hxx>
#include <IFSelect_SessionPilot.hxx>
#include <Interface_Macros.hxx>
#include <Interface_InterfaceModel.hxx>
#include <Message.hxx>
#include <Message_Messenger.hxx>
#include <ShapeExtend_Explorer.hxx>
#include <TCollection_AsciiString.hxx>
#include <TColStd_HSequenceOfTransient.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Iterator.hxx>
#include <TopTools_HSequenceOfShape.hxx>
#include <Transfer_SimpleBinderOfTransient.hxx>
#include <Transfer_FinderProcess.hxx>
#include <Transfer_TransientListBinder.hxx>
#include <Transfer_TransientProcess.hxx>
#include <TransferBRep.hxx>
#include <TransferBRep_ShapeListBinder.hxx>
#include <TransferBRep_ShapeMapper.hxx>
#include <XSControl.hxx>
#include <XSControl_ConnectedShapes.hxx>
#include <XSControl_Controller.hxx>
#include <XSDRAW_FunctionsShape.hxx>
#include <XSControl_TransferReader.hxx>
#include <XSControl_TransferWriter.hxx>
#include <XSControl_Vars.hxx>
#include <XSControl_WorkSession.hxx>

#include <stdio.h>

//=======================================================================
//function : XSControl_tpdraw
//purpose  : 
//=======================================================================
static IFSelect_ReturnStatus XSControl_tpdraw
(const Handle(IFSelect_SessionPilot)& pilot)
{
  Standard_Integer argc = pilot->NbWords();
  const Standard_CString arg1 = pilot->Arg(1);
  const Standard_CString arg2 = pilot->Arg(2);
  const Standard_CString arg3 = pilot->Arg(3);
  const Handle(Transfer_TransientProcess)& TP = XSControl::Session(pilot)->TransferReader()->TransientProcess();
  Message_Messenger::StreamBuffer sout = Message::SendInfo();
  if (TP.IsNull())
  {
    sout << "No Transfer Read" << std::endl; return IFSelect_RetError;
  }
  //        ****    tpdraw        ****
  if (argc < 2)
  {
    sout << "Donner [mode facultatif : item ou root] , NUMERO , nom DRAW facultatif" << std::endl;
    sout << "  mode si present : item ou root, sinon n0 d entite modele" << std::endl;
    sout << "  NUMERO entier : d entite, d item transfert ou de root transfert\n"
      << "    ou * pour dire tous" << std::endl;
    return IFSelect_RetError;
  }
  Standard_Integer mode = 0, num = 0;
  if (arg1[0] == 'i') mode = 1;
  else if (arg1[0] == 'r') mode = 2;
  Standard_Boolean tout = Standard_False;
  if (mode == 0)
  {
    if (argc < 2)
    {
      sout << "Donner au moins un NUMERO ou *" << std::endl; return IFSelect_RetError;
    }
    if (arg1[0] == '*') tout = Standard_True;
    else num = IFSelect_Functions::GiveEntityNumber(XSControl::Session(pilot), arg1);
  }
  else
  {
    if (arg2[0] == '*') tout = Standard_True;
    else num = IFSelect_Functions::GiveEntityNumber(XSControl::Session(pilot), arg2);
  }

  Standard_Integer nbvar = 0;
  Handle(Transfer_Binder) binder;
  Handle(Standard_Transient) ent;
  TopoDS_Shape sh;    char nomvar[40];
  //  Standard_Boolean moderoot =  (pilot->Word(0).Value(3) == 'r');

  Standard_Integer n1, n2, i, max = 0, index = 0;
  Handle(Interface_InterfaceModel) model = TP->Model();
  if (model.IsNull())
  {
    if (mode == 0)
    {
      sout << "Pas de modele, preciser n0 d item de transfert" << std::endl;
      return IFSelect_RetError;
    }
  }
  if (mode == 0)
  {
    sout << "Entite de modele";    max = model->NbEntities();
  }
  if (mode == 1)
  {
    sout << "Item de transfert";   max = TP->NbMapped();
  }
  if (mode == 2)
  {
    sout << "Racine de transfert"; max = TP->NbRoots();
  }
  if (tout)
  {
    n1 = 1;  n2 = max;
    sout << ", listage de 1 a " << max << std::endl;
  }
  else if (num <= 0 || num > max)
  {
    sout << " - Num=" << num << " hors limite (de 1 a " << max << ")" << std::endl;
    return IFSelect_RetError;
  }
  else
  {
    n1 = n2 = num;  nbvar = -1;  // nbvar : 1ere shape simple = pas de n0
    sout << ", n0 " << num << std::endl;
  }

  for (i = n1; i <= n2; i++)
  {
    if (mode == 0)
    {
      ent = model->Value(i);
      num = i;
      index = TP->MapIndex(ent);
    }
    else if (mode == 1)
    {
      ent = TP->Mapped(i);
      if (model.IsNull()) num = 0;
      else num = model->Number(ent);
      index = i;
    }
    else if (mode == 2)
    {
      ent = TP->Root(i);
      if (model.IsNull()) num = 0;
      else num = model->Number(ent);
      index = TP->MapIndex(ent);
    }

    if (index > 0) binder = TP->MapItem(index);
    if (binder.IsNull()) index = 0;
    if (index == 0)
    {
      if (!tout) sout << "Entite n0 " << num << " : non repertoriee" << std::endl;
      continue;
    }
    if (!binder->HasResult())
    {
      if (!tout) sout << "Entite n0 " << num << " : pas de resultat" << std::endl;
      continue;
    }
    sh = TransferBRep::ShapeResult(binder);
    //DeclareAndCast(TransferBRep_ShapeBinder,shb,binder);
    if (!sh.IsNull())
    {
      //sh = shb->Result();
      nbvar++;
      if (sh.IsNull())
      {
        sout << " (no Shape recorded)" << std::endl; continue;
      }
      if (tout) sout << "[ " << i << " ]:";
      if (num == 0) sout << " pas dans le modele";
      else sout << " ent.n0 " << num;
      sout << ", item transfert n0 " << index;
      if (nbvar == 0)
      {
        if (argc > 3 && mode > 0) sprintf(nomvar, "%s", arg3);
        else if (argc > 2 && mode == 0) sprintf(nomvar, "%s", arg2);
        else                            sprintf(nomvar, "tp_%d", i);
      }
      else
      {
        if (argc > 3 && mode > 0) sprintf(nomvar, "%s_%d", arg3, nbvar);
        else if (argc > 2 && mode == 0) sprintf(nomvar, "%s_%d", arg2, nbvar);
        else                            sprintf(nomvar, "tp_%d", i);
      }
      sout << " -> 1 DRAW Shape: " << nomvar << std::endl;
      XSControl::Vars(pilot)->SetShape(nomvar, sh);
      continue;
    }
    DeclareAndCast(TransferBRep_ShapeListBinder, slb, binder);
    if (!slb.IsNull())
    {
      Standard_Integer nbs = slb->NbShapes();
      if (tout) sout << "[ " << i << " ]:";
      if (num == 0) sout << " pas dans le modele";
      else sout << " ent.n0 " << num;
      sout << ", item transfert n0 " << index;
      sout << " -> " << nbs << " DRAW Shapes :";
      for (Standard_Integer j = 1; j <= nbs; j++)
      {
        sh = slb->Shape(j);  if (nbvar < 0) nbvar = 0;  nbvar++;
        if (sh.IsNull())
        {
          sout << " (no Shape recorded)" << std::endl; continue;
        }
        if (argc > 3 && mode > 0) sprintf(nomvar, "%s_%d", arg3, nbvar);
        else if (argc > 2 && mode == 0) sprintf(nomvar, "%s_%d", arg2, nbvar);
        else                        sprintf(nomvar, "tp_%d_%d", i, nbvar);
        sout << " " << nomvar;
        XSControl::Vars(pilot)->SetShape(nomvar, sh);
      }
      sout << std::endl;
      continue;
    }
    DeclareAndCast(Transfer_SimpleBinderOfTransient, trb, binder);
    if (!trb.IsNull())
    {
      Handle(Standard_Transient) resu = trb->Result();
      if (resu.IsNull())
      {
        sout << "Entite n0 " << num << " : pas de resultat" << std::endl;
        continue;
      }
      DeclareAndCast(Geom_Geometry, geom, resu);
      sout << "Entite n0 " << num << " : resultat " << resu->DynamicType()->Name();
      if (geom.IsNull())
      {
        sout << std::endl; continue;
      }
      nbvar++;
      if (nbvar == 0)
      {
        if (argc > 3 && mode > 0) sprintf(nomvar, "%s", arg3);
        else if (argc > 2 && mode == 0) sprintf(nomvar, "%s", arg2);
        else                            sprintf(nomvar, "tp_%d", i);
      }
      else
      {
        if (argc > 3 && mode > 0) sprintf(nomvar, "%s_%d", arg3, nbvar);
        else if (argc > 2 && mode == 0) sprintf(nomvar, "%s_%d", arg2, nbvar);
        else                            sprintf(nomvar, "tp_%d", i);
      }
      char* nomv = nomvar;
      XSControl::Vars(pilot)->Set(nomv, geom);
      sout << " -> DRAW Geom : " << nomvar << std::endl;
      continue;
    }

    if (sh.IsNull() && trb.IsNull())
      if (!tout) sout << "Entite n0 " << num << " : resultat pas une Shape mais " << binder->ResultTypeName() << std::endl;
  }

  if (sh.IsNull()) sout << " (No Shape)" << std::endl;
  return IFSelect_RetDone;
}

//=======================================================================
//function : XSControl_tpcompound
//purpose  :
//=======================================================================
static IFSelect_ReturnStatus XSControl_tpcompound
(const Handle(IFSelect_SessionPilot)& pilot)
{
  Standard_Integer argc = pilot->NbWords();
  const Standard_CString arg1 = pilot->Arg(1);
  const Handle(Transfer_TransientProcess)& TP = XSControl::Session(pilot)->TransferReader()->TransientProcess();
  Message_Messenger::StreamBuffer sout = Message::SendInfo();
  if (TP.IsNull())
  {
    sout << "No Transfer Read" << std::endl; return IFSelect_RetError;
  }
  //        ****    tpcompound        ****
  if (argc < 2)
  {
    sout << "Give a NAME for the Compound  + optional givelist, else roots are taken" << std::endl; return IFSelect_RetError;
  }
  Handle(TopTools_HSequenceOfShape) list;
  if (argc == 2) list = TransferBRep::Shapes(TP);
  else
  {
    Handle(TColStd_HSequenceOfTransient) lise = IFSelect_Functions::GiveList(pilot->Session(), pilot->CommandPart(2));
    if (lise.IsNull())
    {
      sout << "Not a valid entity list : " << pilot->CommandPart(2) << std::endl; return IFSelect_RetError;
    }
    list = TransferBRep::Shapes(TP, lise);
    sout << lise->Length() << " Entities, ";
  }
  if (list.IsNull())
  {
    sout << "No Shape listed" << std::endl; return IFSelect_RetError;
  }
  Standard_Integer nb = list->Length();
  sout << nb << " Shape(s) listed" << std::endl;
  TopoDS_Compound C;
  BRep_Builder B;
  B.MakeCompound(C);
  for (Standard_Integer i = 1; i <= nb; i++)  B.Add(C, list->Value(i));
  XSControl::Vars(pilot)->SetShape(arg1, C);
  return IFSelect_RetDone;
}

//=======================================================================
//function : XSControl_traccess
//purpose  : 
//=======================================================================
static IFSelect_ReturnStatus XSControl_traccess
(const Handle(IFSelect_SessionPilot)& pilot)
{
  Standard_Integer argc = pilot->NbWords();
  const Standard_CString arg1 = pilot->Arg(1);
  const Standard_CString arg2 = pilot->Arg(2);
  //        ****    trdraw : TransferReader        **** 26
  //        ****    trsave : TransferReader        **** 27
  //        ****    trcomp  (comp -> DRAW)         **** 28
  //        ****    trscomp (comp -> save)         **** 29
  Standard_Boolean cascomp = (pilot->Word(0).Location(1, 'o', 1, 5) > 0);
  Standard_Boolean cassave = (pilot->Word(0).Location(1, 's', 1, 5) > 0);
  TCollection_AsciiString nomsh, noms;
  const Handle(XSControl_TransferReader)& TR = XSControl::Session(pilot)->TransferReader();
  Message_Messenger::StreamBuffer sout = Message::SendInfo();
  if (TR.IsNull())
  {
    sout << " manque init" << std::endl; return IFSelect_RetError;
  }
  const Handle(Interface_InterfaceModel)& mdl = TR->Model();
  if (mdl.IsNull())
  {
    sout << " modele absent" << std::endl; return IFSelect_RetError;
  }
  Standard_Integer num = (argc > 1 ? IFSelect_Functions::GiveEntityNumber(XSControl::Session(pilot), arg1) : 0);

  if (argc > 1) nomsh = arg1;
  else nomsh = cascomp ? "TREAD_COMP" : "TREAD_LIST";
  if (cassave) sout << " save shapes -> current directory" << std::endl;

  if (num == 0 || cascomp)
  {
    TopoDS_Compound C;        // pour cas compound
    BRep_Builder B;
    B.MakeCompound(C);

    const Handle(TopTools_HSequenceOfShape)& list = TR->ShapeResultList(Standard_True);
    sout << " TOUS RESULTATS par ShapeResultList, soit " << list->Length() << std::endl;
    for (Standard_Integer i = 1, nb = list->Length(); i <= nb; ++i)
    {
      noms = nomsh + "_" + i;
      if ((i % 1000) == 0) sout << "(" << i << ")" << std::endl;
      else if ((i % 100) == 0) sout << "*";
      else if ((i % 10) == 0) sout << "0";
      else                     sout << ".";
      if (list->Value(i).IsNull()) continue;
      if (!cascomp && !cassave) XSControl::Vars(pilot)->SetShape(noms.ToCString(), list->Value(i));
      else if (!cascomp && cassave) BRepTools::Write(list->Value(i), noms.ToCString());
      else if (cascomp) B.Add(C, list->Value(i));
    }
    sout << std::endl;
    if (cascomp && !cassave) XSControl::Vars(pilot)->SetShape(nomsh.ToCString(), C);
    else if (cascomp && cassave) BRepTools::Write(C, nomsh.ToCString());
  }
  else
  {
    if (num < 1 || num > mdl->NbEntities())
    {
      sout << " incorrect:" << arg1 << std::endl; return IFSelect_RetError;
    }
    TopoDS_Shape sh = TR->ShapeResult(mdl->Value(num));
    if (sh.IsNull())
    {
      sout << " Pas de resultat pour " << arg1 << std::endl; return IFSelect_RetError;
    }
    if (argc > 2) nomsh = arg2;
    else nomsh = TCollection_AsciiString("TREAD_") + num;
    if (!cascomp && !cassave) XSControl::Vars(pilot)->SetShape(nomsh.ToCString(), sh);
    else if (!cascomp && cassave) BRepTools::Write(sh, nomsh.ToCString());
    else sout << "Option non comprise" << std::endl;
  }
  return IFSelect_RetDone;
}

//=======================================================================
//function : XSControl_IsEqualSubShape
//purpose  : 
//=======================================================================
// PTV 23.08.2000 Added for checking where are an entity from.
static Standard_Boolean XSControl_IsEqualSubShape(const TopoDS_Shape& Shape,
                                                  TopoDS_Shape& sh, Standard_Integer aLevel)
{
  if (sh.IsSame(Shape)) return Standard_True;
  if (aLevel > 0)
  {
    TopoDS_Shape varShape;
    aLevel--;
    TopoDS_Iterator it(sh);
    for (; it.More(); it.Next())
    {
      varShape = it.Value();
      if (XSControl_IsEqualSubShape(Shape, varShape, aLevel)) return Standard_True;
    }
  }
  return Standard_False;
}

//=======================================================================
//function : XSControl_fromshape
//purpose  : 
//=======================================================================
static IFSelect_ReturnStatus XSControl_fromshape
(const Handle(IFSelect_SessionPilot)& pilot)
{
  Standard_Integer argc = pilot->NbWords();
  const Standard_CString arg1 = pilot->Arg(1);
  //        ****    fromshape (tread)         ****
  Message_Messenger::StreamBuffer sout = Message::SendInfo();
  if (argc < 2)
  {
    sout << "Give name of a DRAW Shape" << std::endl;
    return IFSelect_RetError;
  }
  const char* a1 = (char*)arg1;
  TopoDS_Shape Shape = XSControl::Vars(pilot)->GetShape(a1);
  if (Shape.IsNull())
  {
    sout << "Not a DRAW Shape:" << arg1 << std::endl;
    return IFSelect_RetError;
  }
  Standard_Boolean yena = Standard_False;
  Standard_Integer aLevel = 1;
  if (argc >= 3)
    aLevel = atoi(pilot->Arg(2));
  Standard_Boolean silent = Standard_False;
  if (aLevel < 0)
  {
    silent = Standard_True;
    aLevel = -aLevel;
  }

  //    IMPORT
  const Handle(XSControl_TransferReader)& TR = XSControl::Session(pilot)->TransferReader();
  if (TR.IsNull())
  {
  }  // sout<<"No read transfer (import) recorded"<<std::endl;
  else
  {
    yena = Standard_True;
    if (!silent) sout << "Shape " << arg1 << " : ";
    Standard_Integer modrec = 1;
    Handle(Standard_Transient) ent = TR->EntityFromShapeResult(Shape, modrec);
    if (ent.IsNull())
    {
      modrec = -1;
      ent = TR->EntityFromShapeResult(Shape, modrec);
    }
    if (ent.IsNull())
    {
      modrec = 2;
      Handle(Transfer_TransientProcess) TP = TR->TransientProcess();
      if (TP.IsNull())
      {
        if (silent)
          sout << "Shape " << arg1 << " : ";
        sout << "no map" << std::endl;
      }
      else
      {
        TopoDS_Shape S0 = Shape;
        TopLoc_Location L;
        S0.Location(L);
        Standard_Integer i, nb = TP->NbMapped();
        if (!silent) sout << "searching in map among " << nb << " ...";
        for (i = 1; i <= nb; i++)
        {
          ent = TP->Mapped(i);
          TopoDS_Shape sh = TransferBRep::ShapeResult(TP, ent);
          if (sh.IsNull())
          {
            ent.Nullify();
            continue;
          }
          if (XSControl_IsEqualSubShape(Shape, sh, aLevel)) break;
          modrec = -2;
          sh.Location(L);
          if (XSControl_IsEqualSubShape(S0, sh, aLevel)) break;
          ent.Nullify();
          modrec = 2;
        }
      }
    }
    if (!ent.IsNull())
    {
      if (silent) sout << "Shape " << arg1 << ": ";
      if (modrec < 0) sout << "(moved from origin) ";
      //else sout<<"(origin) ";
    }
    //  on affiche
    if (ent.IsNull())
    {
      if (!silent) sout << " unknown as imported";
      // skl 11.05.2004
      // if Shape is a compound try to make "fromshape" for its subshapes
      if (Shape.ShapeType() == TopAbs_COMPOUND)
      {
        sout << std::endl << "Subshapes imported from entities:";
        TopoDS_Iterator Iter(Shape);
        for (; Iter.More(); Iter.Next())
        {
          TopoDS_Shape subsh = Iter.Value();
          Standard_Integer submodrec = 1;
          Handle(Standard_Transient) subent = TR->EntityFromShapeResult(subsh, submodrec);
          if (subent.IsNull())
          {
            submodrec = -1;
            subent = TR->EntityFromShapeResult(subsh, submodrec);
          }
          if (!subent.IsNull())
          {
            sout << "  " << XSControl::Session(pilot)->Model()->Number(subent);
          }
        }
      }
    }
    else
    {
      sout << "imported from entity ";
      XSControl::Session(pilot)->Model()->Print(ent, sout);
      if (silent) sout << " in file " << XSControl::Session(pilot)->LoadedFile() << std::endl;
    }
    if (!silent) sout << std::endl;
  }

  //   ET EN EXPORT ?
  const Handle(Transfer_FinderProcess)& FP = XSControl::Session(pilot)->TransferWriter()->FinderProcess();
  if (FP.IsNull())
  {
  }
  else
  {
    yena = Standard_True;
    Handle(Transfer_Finder) fnd = TransferBRep::ShapeMapper(FP, Shape);
    Handle(Standard_Transient) ent;
    if (!fnd.IsNull()) ent = FP->FindTransient(fnd);
    if (!ent.IsNull())
    {
      sout << "Shape " << arg1 << ": exported to entity ";
      XSControl::Session(pilot)->Model()->Print(ent, sout);
      if (silent) sout << " in file " << XSControl::Session(pilot)->LoadedFile();
      sout << std::endl;
    }
    // abv 31.08.00: treat case of split shape (several results)
    // it is supposed that results are of the same type and lie in one-level comp
    else
    {
      Handle(Transfer_Binder) bnd = FP->Find(fnd);
      if (!bnd.IsNull())
      {
        Handle(Transfer_TransientListBinder) TransientListBinder =
          //Handle(Transfer_TransientListBinder)::DownCast( bnd->Next(Standard_True) ); //skl
          Handle(Transfer_TransientListBinder)::DownCast(bnd); //skl
        if (!TransientListBinder.IsNull())
        {
          Standard_Integer i = 1, nb = TransientListBinder->NbTransients();
          if (nb > 0) sout << "Shape " << arg1 << ": exported to entities ";
          for (; i <= nb; i++)
          {
            XSControl::Session(pilot)->Model()->Print(TransientListBinder->Transient(i), sout);
            if (i < nb) sout << ", ";
          }
          if (nb > 0)
          {
            if (silent) sout << " in file " << XSControl::Session(pilot)->LoadedFile();
            sout << std::endl;
          }
        }
        /*	else {
            TopoDS_Shape comp = TransferBRep::ShapeResult(bnd);
            if ( ! comp.IsNull() && comp.ShapeType() < Shape.ShapeType() ) {
              Standard_Boolean start = Standard_True;
              for ( TopoDS_Iterator it(comp); it.More(); it.Next() ) {
                Handle(Transfer_Finder) cfnd = TransferBRep::ShapeMapper (FP,it.Value());
                if ( cfnd.IsNull() ) continue;
                Handle(Standard_Transient) cent = FP->FindTransient (cfnd);
                if ( cent.IsNull() ) continue;
                if ( start )
            sout<<"Shape "<<arg1<<" : exported to entities ";
                else sout << ", ";
                start = Standard_False;
                XSControl::Session(pilot)->Model()->Print(cent,sout);
              }
              if ( ! start ) sout<<std::endl;
            }
          }  */
      }
    }
  }
  if (!yena) sout << "No transfer (either import or export) recorded" << std::endl;

  return IFSelect_RetVoid;
}

//=======================================================================
//function : XSControl_trconnexentities
//purpose  : 
//=======================================================================
static IFSelect_ReturnStatus XSControl_trconnexentities
(const Handle(IFSelect_SessionPilot)& pilot)
{
  Standard_Integer argc = pilot->NbWords();
  const Standard_CString arg1 = pilot->Arg(1);
  //        ****    connected entities (last transfer)         ****
  const Handle(XSControl_TransferReader)& TR = XSControl::Session(pilot)->TransferReader();
  Handle(Transfer_TransientProcess) TP;
  if (!TR.IsNull()) TP = TR->TransientProcess();
  Message_Messenger::StreamBuffer sout = Message::SendInfo();
  if (TP.IsNull())
  {
    sout << "no transfer map" << std::endl; return IFSelect_RetVoid;
  }
  if (argc < 2)
  {
    sout << "Give name of a DRAW Shape + optional shape type v-e-w-f(D)-s" << std::endl;
    return IFSelect_RetError;
  }
  const char* a1 = (const char*)arg1;
  TopoDS_Shape Shape = XSControl::Vars(pilot)->GetShape(a1);
  if (Shape.IsNull())
  {
    sout << "Not a DRAW Shape:" << arg1 << std::endl; return IFSelect_RetError;
  }
  sout << "Shape " << arg1 << " : ";

  Handle(TColStd_HSequenceOfTransient) list =
    XSControl_ConnectedShapes::AdjacentEntities(Shape, TP, TopAbs_FACE);
  Standard_Integer i, nb = list->Length();
  sout << nb << " Entities produced Connected Shapes :" << std::endl;
  const Handle(Interface_InterfaceModel)& model = XSControl::Session(pilot)->Model();
  sout << "(";
  for (i = 1; i <= nb; i++)
  {
    if (i > 1) sout << ",";
    sout << model->Number(list->Value(i));
  }
  sout << ")" << std::endl;
  return IFSelect_RetDone;
}

//=======================================================================
//function : XSControl_trimport
//purpose  : 
//=======================================================================
static IFSelect_ReturnStatus XSControl_trimport
(const Handle(IFSelect_SessionPilot)& pilot)
{
  //  FileName ou . (pour courant)  VarName  GiveList (obligatoire)
  //    GiveList : * pour xst-transferrable-roots
  Handle(XSControl_WorkSession) WS = XSControl::Session(pilot);

  Standard_Integer argc = pilot->NbWords();
  Message_Messenger::StreamBuffer sout = Message::SendInfo();
  if (argc < 4)
  {
    sout << "Give : filename or . for current model;  varname or . to take fileroot\n  GiveList, * for all transferrable roots" << std::endl;
    return IFSelect_RetError;
  }
  const Standard_CString arg1 = pilot->Arg(1);
  const Standard_CString arg2 = pilot->Arg(2);
  const Standard_CString arg3 = pilot->Arg(3);

  //  File Name and Variable (root) Name

  TCollection_AsciiString fnom, rnom;
  Standard_Boolean modfic = XSDRAW_FunctionsShape::FileAndVar
  (WS, arg1, arg2, "IMPORT", fnom, rnom);
  if (modfic) sout << " File to read : " << fnom << std::endl;
  else        sout << " Model taken from the session : " << fnom << std::endl;
  sout << " -- Names of variables BREP-DRAW prefixed by : " << rnom << std::endl;

  //  keep the current command, because sub-commands will be called
  TCollection_AsciiString compart = pilot->CommandPart(3);

  //  Reading file if required

  if (modfic)
  {
    TCollection_AsciiString comload("xload ");
    comload.AssignCat(arg1);
    IFSelect_ReturnStatus status = pilot->Execute(comload);
    if (status != IFSelect_RetDone)
    {
      sout << "Abandon import" << std::endl; return status;
    }
  }
  else
  {
    sout << "Currently Loaded Model" << std::endl;
  }

  //  Selecting Entities

  Handle(TColStd_HSequenceOfTransient)  list;
  if (arg3[0] == '*' && arg3[1] == '\0')
  {
    list = WS->GiveList("xst-transferrable-roots");
    sout << "All Transferrable Roots : ";
  }
  else
  {
    sout << "List given by " << compart.ToCString() << " : ";
    list = WS->GiveList(compart.ToCString());
  }
  if (list.IsNull())
  {
    sout << "No list defined. Abandon" << std::endl; return IFSelect_RetError;
  }
  Standard_Integer nbl = list->Length();
  sout << "Nb entities selected : " << nbl << std::endl;

  //  Starting Transfer

  WS->InitTransferReader(0);
  const Handle(XSControl_TransferReader)& TR = WS->TransferReader();
  if (TR.IsNull())
  {
    sout << " init not done or failed" << std::endl; return IFSelect_RetError;
  }

  TR->BeginTransfer();

  //  Transferring
  Standard_Integer nbt = TR->TransferList(list);
  sout << "Nb Entities Selected : " << nbl << " have given " << nbt << " results" << std::endl;

  //  Filling VARS. one compound (trimpcomp) or one shape per ent (trimport)
  Standard_Boolean iscomp = (pilot->Arg(0)[5] == 'c');
  Standard_Integer nbs = 0;
  TopoDS_Shape sh;
  TopoDS_Compound C;
  BRep_Builder B;
  B.MakeCompound(C);
  Handle(Interface_InterfaceModel)  mdl = TR->Model();
  if (mdl.IsNull())
  {
    sout << " modele absent" << std::endl; return IFSelect_RetError;
  }
  for (Standard_Integer il = 1; il <= nbl; il++)
  {
    Handle(Standard_Transient) ent = list->Value(il);
    sh = TR->ShapeResult(ent);
    if (sh.IsNull()) continue;
    nbs++;
    if (iscomp) B.Add(C, sh);
    else
    {
      char nomsh[50];
      sprintf(nomsh, "%s_%d", rnom.ToCString(), nbs);
      XSControl::Vars(pilot)->SetShape(nomsh, sh);
    }
  }
  if (nbs == 0) sout << "No Shape produced" << std::endl;
  else if (nbs == 1)
  {
    sout << "One Shape produced, named " << rnom.ToCString() << std::endl;
    XSControl::Vars(pilot)->SetShape(rnom.ToCString(), sh);
  }
  else if (iscomp)
  {
    sout << "One compound made of " << nbs << " Shapes, named " << rnom.ToCString() << std::endl;
    XSControl::Vars(pilot)->SetShape(rnom.ToCString(), C);
  }
  else
  {  // several individual shapes
    sout << nbs << " Shapes, named " << rnom.ToCString() << "_1 to " << rnom.ToCString() << "_" << nbs << std::endl;
  }

  return IFSelect_RetDone;
}

//=======================================================================
//function : XSControl_twrite
//purpose  : 
//=======================================================================
static IFSelect_ReturnStatus XSControl_twrite
(const Handle(IFSelect_SessionPilot)& pilot)
{
  Standard_Integer argc = pilot->NbWords();
  const Standard_CString arg1 = pilot->Arg(1);
  //        ****    twrite         ****
  Message_Messenger::StreamBuffer sout = Message::SendInfo();
  Handle(XSControl_TransferWriter) TW = XSControl::Session(pilot)->TransferWriter();
  if (argc < 2)
  {
    sout << " donner nom de shape draw" << std::endl; return IFSelect_RetError;
  }
  sout << "Attention, on alimente le modele courant ..." << std::endl;

  // Shape
  for (Standard_Integer i = 1; i < argc; i++)
  {
    const char* ai = (const char*)pilot->Arg(i);
    TopoDS_Shape Shape = XSControl::Vars(pilot)->GetShape(ai);
    if (Shape.IsNull())
    {
      sout << "pas un nom de shape draw:" << arg1 << std::endl; continue;
    }
    sout << "Pour Shape : " << ai;
    Standard_Integer stat = TW->TransferWriteShape(XSControl::Session(pilot)->Model(), Shape);
    sout << " Transfer Write Status = " << stat << std::endl;
  }
  pilot->Session()->ComputeGraph();
  // Transient ? (Geom) : ignore
  return IFSelect_RetDone;
}


//=======================================================================
//function : Init
//purpose  :
//=======================================================================
void  XSDRAW_FunctionsShape::Init()
{
  static int THE_XSDRAW_FunctionsShape_initactor = 0;
  if (THE_XSDRAW_FunctionsShape_initactor)
  {
    return;
  }

  THE_XSDRAW_FunctionsShape_initactor = 1;

  IFSelect_Act::SetGroup("DE: General");
  IFSelect_Act::AddFunc("tpdraw", "[mode:item or root]  num|*  [nomvar] Passes an ITEM to Shape Draw (Start or Result)", XSControl_tpdraw);
  IFSelect_Act::AddFunc("tpcompound", "name:cstring [givelist] : -> compound with Shapes Root or from givelist", XSControl_tpcompound);
  IFSelect_Act::AddFunc("trdraw", "results ->DRAW : all;  or num [name] : from ent.num -> DRAW [name/tread_num]", XSControl_traccess);
  IFSelect_Act::AddFunc("trsave", "results ->files : all;  or num [name] : from ent.num -> DRAW [name/tread_num]", XSControl_traccess);
  IFSelect_Act::AddFunc("trcomp", "results -> 1 compound -> DRAW + name optional", XSControl_traccess);
  IFSelect_Act::AddFunc("trscomp", "results -> 1 compound -> file + name optional", XSControl_traccess);
  IFSelect_Act::AddFunc("fromshape", "shape [level=1]: imported/exported entity (when known)", XSControl_fromshape);
  IFSelect_Act::AddFunc("trconnexent", "name of draw shape : entities -> connected shapes (when known)", XSControl_trconnexentities);
  IFSelect_Act::AddFunc("trimport", "filename or .  varname  givelist  -> 1 shape per entity", XSControl_trimport);
  IFSelect_Act::AddFunc("trimpcomp", "filename or .  varname  givelist -> one xcompound", XSControl_trimport);
  IFSelect_Act::AddFunc("twrite", "shape : transfer write for this shape, AFTER newmodel !", XSControl_twrite);
}
