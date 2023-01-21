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

#include <XSDRAW_FunctionsShape.hxx>

#include <BRep_Builder.hxx>
#include <BRepTools.hxx>
#include <DBRep.hxx>
#include <DrawTrSurf.hxx>
#include <Geom_Geometry.hxx>
#include <IFSelect_Act.hxx>
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
#include <XSDRAW.hxx>
#include <XSDRAWBase.hxx>
#include <XSControl_TransferReader.hxx>
#include <XSControl_TransferWriter.hxx>
#include <XSControl_Vars.hxx>
#include <XSControl_WorkSession.hxx>

//=======================================================================
//function : GiveEntityNumber
//purpose  :
//=======================================================================
static Standard_Integer GiveEntityNumber(const Handle(XSControl_WorkSession)& WS,
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
//function : XSControl_tpdraw
//purpose  : 
//=======================================================================
static Standard_Integer XSControl_tpdraw(Draw_Interpretor& theDI,
                                         Standard_Integer theNbArgs,
                                         const char** theArgVec)
{
  XSDRAW::StreamContainer aSSC(theDI);
  const Standard_CString arg1 = theArgVec[1];
  const Standard_CString arg2 = theArgVec[2];
  const Standard_CString arg3 = theArgVec[3];
  const Handle(Transfer_TransientProcess)& TP = XSDRAWBase::Session()->TransferReader()->TransientProcess();
  if (TP.IsNull())
  {
    aSSC.SStream() << "No Transfer Read" << std::endl;
    return 1;
  }
  //        ****    tpdraw        ****
  if (theNbArgs < 2)
  {
    aSSC.SStream() << "Donner [mode facultatif : item ou root] , NUMERO , nom DRAW facultatif" << std::endl;
    aSSC.SStream() << "  mode si present : item ou root, sinon n0 d entite modele" << std::endl;
    aSSC.SStream() << "  NUMERO entier : d entite, d item transfert ou de root transfert\n"
      << "    ou * pour dire tous" << std::endl;
    return 1;
  }
  Standard_Integer mode = 0, num = 0;
  if (arg1[0] == 'i') mode = 1;
  else if (arg1[0] == 'r') mode = 2;
  Standard_Boolean tout = Standard_False;
  if (mode == 0)
  {
    if (theNbArgs < 2)
    {
      aSSC.SStream() << "Donner au moins un NUMERO ou *" << std::endl;
      return 1;
    }
    if (arg1[0] == '*') tout = Standard_True;
    else num = GiveEntityNumber(XSDRAWBase::Session(), arg1);
  }
  else
  {
    if (arg2[0] == '*') tout = Standard_True;
    else num = GiveEntityNumber(XSDRAWBase::Session(), arg2);
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
      aSSC.SStream() << "Pas de modele, preciser n0 d item de transfert" << std::endl;
      return 1;
    }
  }
  if (mode == 0)
  {
    aSSC.SStream() << "Entite de modele";    max = model->NbEntities();
  }
  if (mode == 1)
  {
    aSSC.SStream() << "Item de transfert";   max = TP->NbMapped();
  }
  if (mode == 2)
  {
    aSSC.SStream() << "Racine de transfert"; max = TP->NbRoots();
  }
  if (tout)
  {
    n1 = 1;  n2 = max;
    aSSC.SStream() << ", listage de 1 a " << max << std::endl;
  }
  else if (num <= 0 || num > max)
  {
    aSSC.SStream() << " - Num=" << num << " hors limite (de 1 a " << max << ")" << std::endl;
    return 1;
  }
  else
  {
    n1 = n2 = num;  nbvar = -1;  // nbvar : 1ere shape simple = pas de n0
    aSSC.SStream() << ", n0 " << num << std::endl;
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
      if (!tout) aSSC.SStream() << "Entite n0 " << num << " : non repertoriee" << std::endl;
      continue;
    }
    if (!binder->HasResult())
    {
      if (!tout) aSSC.SStream() << "Entite n0 " << num << " : pas de resultat" << std::endl;
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
        aSSC.SStream() << " (no Shape recorded)" << std::endl; continue;
      }
      if (tout) aSSC.SStream() << "[ " << i << " ]:";
      if (num == 0) aSSC.SStream() << " pas dans le modele";
      else aSSC.SStream() << " ent.n0 " << num;
      aSSC.SStream() << ", item transfert n0 " << index;
      if (nbvar == 0)
      {
        if (theNbArgs > 3 && mode > 0) sprintf(nomvar, "%s", arg3);
        else if (theNbArgs > 2 && mode == 0) sprintf(nomvar, "%s", arg2);
        else                            sprintf(nomvar, "tp_%d", i);
      }
      else
      {
        if (theNbArgs > 3 && mode > 0) sprintf(nomvar, "%s_%d", arg3, nbvar);
        else if (theNbArgs > 2 && mode == 0) sprintf(nomvar, "%s_%d", arg2, nbvar);
        else                            sprintf(nomvar, "tp_%d", i);
      }
      aSSC.SStream() << " -> 1 DRAW Shape: " << nomvar << std::endl;
      DBRep::Set(nomvar, sh);
      continue;
    }
    DeclareAndCast(TransferBRep_ShapeListBinder, slb, binder);
    if (!slb.IsNull())
    {
      Standard_Integer nbs = slb->NbShapes();
      if (tout) aSSC.SStream() << "[ " << i << " ]:";
      if (num == 0) aSSC.SStream() << " pas dans le modele";
      else aSSC.SStream() << " ent.n0 " << num;
      aSSC.SStream() << ", item transfert n0 " << index;
      aSSC.SStream() << " -> " << nbs << " DRAW Shapes :";
      for (Standard_Integer j = 1; j <= nbs; j++)
      {
        sh = slb->Shape(j);  if (nbvar < 0) nbvar = 0;  nbvar++;
        if (sh.IsNull())
        {
          aSSC.SStream() << " (no Shape recorded)" << std::endl; continue;
        }
        if (theNbArgs > 3 && mode > 0) sprintf(nomvar, "%s_%d", arg3, nbvar);
        else if (theNbArgs > 2 && mode == 0) sprintf(nomvar, "%s_%d", arg2, nbvar);
        else                        sprintf(nomvar, "tp_%d_%d", i, nbvar);
        aSSC.SStream() << " " << nomvar;
        DBRep::Set(nomvar, sh);
      }
      aSSC.SStream() << std::endl;
      continue;
    }
    DeclareAndCast(Transfer_SimpleBinderOfTransient, trb, binder);
    if (!trb.IsNull())
    {
      Handle(Standard_Transient) resu = trb->Result();
      if (resu.IsNull())
      {
        aSSC.SStream() << "Entite n0 " << num << " : pas de resultat" << std::endl;
        continue;
      }
      DeclareAndCast(Geom_Geometry, geom, resu);
      aSSC.SStream() << "Entite n0 " << num << " : resultat " << resu->DynamicType()->Name();
      if (geom.IsNull())
      {
        aSSC.SStream() << std::endl; continue;
      }
      nbvar++;
      if (nbvar == 0)
      {
        if (theNbArgs > 3 && mode > 0) sprintf(nomvar, "%s", arg3);
        else if (theNbArgs > 2 && mode == 0) sprintf(nomvar, "%s", arg2);
        else                            sprintf(nomvar, "tp_%d", i);
      }
      else
      {
        if (theNbArgs > 3 && mode > 0) sprintf(nomvar, "%s_%d", arg3, nbvar);
        else if (theNbArgs > 2 && mode == 0) sprintf(nomvar, "%s_%d", arg2, nbvar);
        else                            sprintf(nomvar, "tp_%d", i);
      }
      char* nomv = nomvar;
      DrawTrSurf::Set(nomv, geom);
      aSSC.SStream() << " -> DRAW Geom : " << nomvar << std::endl;
      continue;
    }

    if (sh.IsNull() && trb.IsNull())
      if (!tout) aSSC.SStream() << "Entite n0 " << num << " : resultat pas une Shape mais " << binder->ResultTypeName() << std::endl;
  }

  if (sh.IsNull()) aSSC.SStream() << " (No Shape)" << std::endl;
  return 0;
}

//=======================================================================
//function : XSControl_tpcompound
//purpose  :
//=======================================================================
static Standard_Integer XSControl_tpcompound(Draw_Interpretor& theDI,
                                             Standard_Integer theNbArgs,
                                             const char** theArgVec)
{
  XSDRAW::StreamContainer aSSC(theDI);
  const Standard_CString arg1 = theArgVec[1];
  const Handle(Transfer_TransientProcess)& TP = XSDRAWBase::Session()->TransferReader()->TransientProcess();
  if (TP.IsNull())
  {
    aSSC.SStream() << "No Transfer Read" << std::endl;
    return 1;
  }
  //        ****    tpcompound        ****
  if (theNbArgs < 2)
  {
    aSSC.SStream() << "Give a NAME for the Compound  + optional givelist, else roots are taken" << std::endl;
    return 1;
  }
  Handle(TopTools_HSequenceOfShape) list;
  if (theNbArgs == 2) list = TransferBRep::Shapes(TP);
  else
  {
    Handle(TColStd_HSequenceOfTransient) lise = XSDRAWBase::Session()->GiveList(theArgVec[2]);
    if (lise.IsNull())
    {
      aSSC.SStream() << "Not a valid entity list : " << theArgVec[2] << std::endl;
      return 1;
    }
    list = TransferBRep::Shapes(TP, lise);
    aSSC.SStream() << lise->Length() << " Entities, ";
  }
  if (list.IsNull())
  {
    aSSC.SStream() << "No Shape listed" << std::endl;
    return 1;
  }
  Standard_Integer nb = list->Length();
  aSSC.SStream() << nb << " Shape(s) listed" << std::endl;
  TopoDS_Compound C;
  BRep_Builder B;
  B.MakeCompound(C);
  for (Standard_Integer i = 1; i <= nb; i++)  B.Add(C, list->Value(i));
  DBRep::Set(arg1, C);
  return 0;
}

//=======================================================================
//function : XSControl_traccess
//purpose  : 
//=======================================================================
static Standard_Integer XSControl_traccess(Draw_Interpretor& theDI,
                                           Standard_Integer theNbArgs,
                                           const char** theArgVec)
{
  XSDRAW::StreamContainer aSSC(theDI);
  const Standard_CString arg1 = theArgVec[1];
  const Standard_CString arg2 = theArgVec[2];
  TCollection_AsciiString aCommand(theArgVec[0]);
  //        ****    trdraw : TransferReader        **** 26
  //        ****    trsave : TransferReader        **** 27
  //        ****    trcomp  (comp -> DRAW)         **** 28
  //        ****    trscomp (comp -> save)         **** 29
  Standard_Boolean cascomp = (aCommand.Location(1, 'o', 1, 5) > 0);
  Standard_Boolean cassave = (aCommand.Location(1, 's', 1, 5) > 0);
  TCollection_AsciiString nomsh, noms;
  const Handle(XSControl_TransferReader)& TR = XSDRAWBase::Session()->TransferReader();
  if (TR.IsNull())
  {
    aSSC.SStream() << " manque init" << std::endl;
    return 1;
  }
  const Handle(Interface_InterfaceModel)& mdl = TR->Model();
  if (mdl.IsNull())
  {
    aSSC.SStream() << " modele absent" << std::endl;
    return 1;
  }
  Standard_Integer num = (theNbArgs > 1 ? GiveEntityNumber(XSDRAWBase::Session(), arg1) : 0);

  if (theNbArgs > 1) nomsh = arg1;
  else nomsh = cascomp ? "TREAD_COMP" : "TREAD_LIST";
  if (cassave) aSSC.SStream() << " save shapes -> current directory" << std::endl;

  if (num == 0 || cascomp)
  {
    TopoDS_Compound C;        // pour cas compound
    BRep_Builder B;
    B.MakeCompound(C);

    const Handle(TopTools_HSequenceOfShape)& list = TR->ShapeResultList(Standard_True);
    aSSC.SStream() << " TOUS RESULTATS par ShapeResultList, soit " << list->Length() << std::endl;
    for (Standard_Integer i = 1, nb = list->Length(); i <= nb; ++i)
    {
      noms = nomsh + "_" + i;
      if ((i % 1000) == 0) aSSC.SStream() << "(" << i << ")" << std::endl;
      else if ((i % 100) == 0) aSSC.SStream() << "*";
      else if ((i % 10) == 0) aSSC.SStream() << "0";
      else                     aSSC.SStream() << ".";
      if (list->Value(i).IsNull()) continue;
      if (!cascomp && !cassave) DBRep::Set(noms.ToCString(), list->Value(i));
      else if (!cascomp && cassave) BRepTools::Write(list->Value(i), noms.ToCString());
      else if (cascomp) B.Add(C, list->Value(i));
    }
    aSSC.SStream() << std::endl;
    if (cascomp && !cassave) DBRep::Set(nomsh.ToCString(), C);
    else if (cascomp && cassave) BRepTools::Write(C, nomsh.ToCString());
  }
  else
  {
    if (num < 1 || num > mdl->NbEntities())
    {
      aSSC.SStream() << " incorrect:" << arg1 << std::endl;
      return 1;
    }
    TopoDS_Shape sh = TR->ShapeResult(mdl->Value(num));
    if (sh.IsNull())
    {
      aSSC.SStream() << " Pas de resultat pour " << arg1 << std::endl;
      return 1;
    }
    if (theNbArgs > 2) nomsh = arg2;
    else nomsh = TCollection_AsciiString("TREAD_") + num;
    if (!cascomp && !cassave) DBRep::Set(nomsh.ToCString(), sh);
    else if (!cascomp && cassave) BRepTools::Write(sh, nomsh.ToCString());
    else aSSC.SStream() << "Option non comprise" << std::endl;
  }
  return 0;
}

//=======================================================================
//function : XSControl_IsEqualSubShape
//purpose  : 
//=======================================================================
// PTV 23.08.2000 Added for checking where are an entity from.
static Standard_Boolean XSControl_IsEqualSubShape(const TopoDS_Shape& Shape,
                                                  TopoDS_Shape& sh,
                                                  Standard_Integer aLevel)
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
static Standard_Integer XSControl_fromshape(Draw_Interpretor& theDI,
                                            Standard_Integer theNbArgs,
                                            const char** theArgVec)
{
  XSDRAW::StreamContainer aSSC(theDI);
  const Standard_CString arg1 = theArgVec[1];
  //        ****    fromshape (tread)         ****
  if (theNbArgs < 2)
  {
    aSSC.SStream() << "Give name of a DRAW Shape" << std::endl;
    return 1;
  }
  const char* a1 = (char*)arg1;
  TopoDS_Shape Shape = DBRep::Get(a1);
  if (Shape.IsNull())
  {
    aSSC.SStream() << "Not a DRAW Shape:" << arg1 << std::endl;
    return 1;
  }
  Standard_Boolean yena = Standard_False;
  Standard_Integer aLevel = 1;
  if (theNbArgs >= 3)
    aLevel = atoi(theArgVec[2]);
  Standard_Boolean silent = Standard_False;
  if (aLevel < 0)
  {
    silent = Standard_True;
    aLevel = -aLevel;
  }

  //    IMPORT
  const Handle(XSControl_TransferReader)& TR = XSDRAWBase::Session()->TransferReader();
  if (TR.IsNull())
  {
  }  // aSSC.SStream()<<"No read transfer (import) recorded"<<std::endl;
  else
  {
    yena = Standard_True;
    if (!silent) aSSC.SStream() << "Shape " << arg1 << " : ";
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
          aSSC.SStream() << "Shape " << arg1 << " : ";
        aSSC.SStream() << "no map" << std::endl;
      }
      else
      {
        TopoDS_Shape S0 = Shape;
        TopLoc_Location L;
        S0.Location(L);
        Standard_Integer i, nb = TP->NbMapped();
        if (!silent) aSSC.SStream() << "searching in map among " << nb << " ...";
        for (i = 1; i <= nb; i++)
        {
          ent = TP->Mapped(i);
          TopoDS_Shape sh = TransferBRep::ShapeResult(TP, ent);
          if (sh.IsNull())
          {
            continue;
          }
          if (XSControl_IsEqualSubShape(Shape, sh, aLevel))
            break;
          modrec = -2;
          sh.Location(L);
          if (XSControl_IsEqualSubShape(S0, sh, aLevel))
            break;
          modrec = 2;
        }
      }
    }
    if (!ent.IsNull())
    {
      if (silent) aSSC.SStream() << "Shape " << arg1 << ": ";
      if (modrec < 0) aSSC.SStream() << "(moved from origin) ";
      //else aSSC.SStream()<<"(origin) ";
    }
    //  on affiche
    if (ent.IsNull())
    {
      if (!silent) aSSC.SStream() << " unknown as imported";
      // skl 11.05.2004
      // if Shape is a compound try to make "fromshape" for its subshapes
      if (Shape.ShapeType() == TopAbs_COMPOUND)
      {
        aSSC.SStream() << std::endl << "Subshapes imported from entities:";
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
            aSSC.SStream() << "  " << XSDRAWBase::Session()->Model()->Number(subent);
          }
        }
      }
    }
    else
    {
      aSSC.SStream() << "imported from entity ";
      XSDRAWBase::Session()->Model()->Print(ent, aSSC.SStream());
      if (silent) aSSC.SStream() << " in file " << XSDRAWBase::Session()->LoadedFile() << std::endl;
    }
    if (!silent) aSSC.SStream() << std::endl;
  }

  //   ET EN EXPORT ?
  const Handle(Transfer_FinderProcess)& FP = XSDRAWBase::Session()->TransferWriter()->FinderProcess();
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
      aSSC.SStream() << "Shape " << arg1 << ": exported to entity ";
      XSDRAWBase::Session()->Model()->Print(ent, aSSC.SStream());
      if (silent) aSSC.SStream() << " in file " << XSDRAWBase::Session()->LoadedFile();
      aSSC.SStream() << std::endl;
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
          if (nb > 0) aSSC.SStream() << "Shape " << arg1 << ": exported to entities ";
          for (; i <= nb; i++)
          {
            XSDRAWBase::Session()->Model()->Print(TransientListBinder->Transient(i), aSSC.SStream());
            if (i < nb) aSSC.SStream() << ", ";
          }
          if (nb > 0)
          {
            if (silent) aSSC.SStream() << " in file " << XSDRAWBase::Session()->LoadedFile();
            aSSC.SStream() << std::endl;
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
            aSSC.SStream()<<"Shape "<<arg1<<" : exported to entities ";
                else aSSC.SStream() << ", ";
                start = Standard_False;
                XSControl::Session(pilot)->Model()->Print(cent,aSSC.SStream());
              }
              if ( ! start ) aSSC.SStream()<<std::endl;
            }
          }  */
      }
    }
  }
  if (!yena)
  {
    aSSC.SStream() << "No transfer (either import or export) recorded" << std::endl;
    return 1;
  }

  return 0;
}

//=======================================================================
//function : XSControl_trconnexentities
//purpose  : 
//=======================================================================
static Standard_Integer XSControl_trconnexentities(Draw_Interpretor& theDI,
                                                   Standard_Integer theNbArgs,
                                                   const char** theArgVec)
{
  XSDRAW::StreamContainer aSSC(theDI);
  const Standard_CString arg1 = theArgVec[1];
  //        ****    connected entities (last transfer)         ****
  const Handle(XSControl_TransferReader)& TR = XSDRAWBase::Session()->TransferReader();
  Handle(Transfer_TransientProcess) TP;
  if (!TR.IsNull()) TP = TR->TransientProcess();
  if (TP.IsNull())
  {
    aSSC.SStream() << "no transfer map" << std::endl;
    return 1;
  }
  if (theNbArgs < 2)
  {
    aSSC.SStream() << "Give name of a DRAW Shape + optional shape type v-e-w-f(D)-s" << std::endl;
    return 1;
  }
  const char* a1 = arg1;
  TopoDS_Shape Shape = DBRep::Get(a1);
  if (Shape.IsNull())
  {
    aSSC.SStream() << "Not a DRAW Shape:" << arg1 << std::endl;
    return 1;
  }
  aSSC.SStream() << "Shape " << arg1 << " : ";

  Handle(TColStd_HSequenceOfTransient) list =
    XSControl_ConnectedShapes::AdjacentEntities(Shape, TP, TopAbs_FACE);
  Standard_Integer i, nb = list->Length();
  aSSC.SStream() << nb << " Entities produced Connected Shapes :" << std::endl;
  const Handle(Interface_InterfaceModel)& model = XSDRAWBase::Session()->Model();
  aSSC.SStream() << "(";
  for (i = 1; i <= nb; i++)
  {
    if (i > 1) aSSC.SStream() << ",";
    aSSC.SStream() << model->Number(list->Value(i));
  }
  aSSC.SStream() << ")" << std::endl;
  return 0;
}

//=======================================================================
//function : XSControl_twrite
//purpose  : 
//=======================================================================
static Standard_Integer XSControl_twrite(Draw_Interpretor& theDI,
                                         Standard_Integer theNbArgs,
                                         const char** theArgVec)
{
  XSDRAW::StreamContainer aSSC(theDI);
  const Standard_CString arg1 = theArgVec[1];
  //        ****    twrite         ****
  Handle(XSControl_TransferWriter) TW = XSDRAWBase::Session()->TransferWriter();
  if (theNbArgs < 2)
  {
    aSSC.SStream() << " donner nom de shape draw" << std::endl;
    return 1;
  }
  aSSC.SStream() << "Attention, on alimente le modele courant ..." << std::endl;

  // Shape
  for (Standard_Integer i = 1; i < theNbArgs; i++)
  {
    const char* ai = theArgVec[i];
    TopoDS_Shape Shape = DBRep::Get(ai);
    if (Shape.IsNull())
    {
      aSSC.SStream() << "pas un nom de shape draw:" << arg1 << std::endl; continue;
    }
    aSSC.SStream() << "Pour Shape : " << ai;
    Standard_Integer stat = TW->TransferWriteShape(XSDRAWBase::Session()->Model(), Shape);
    aSSC.SStream() << " Transfer Write Status = " << stat << std::endl;
  }
  XSDRAWBase::Session()->ComputeGraph();
  // Transient ? (Geom) : ignore
  return 0;
}

//=======================================================================
//function : Init
//purpose  :
//=======================================================================
void  XSDRAW_FunctionsShape::Init(Draw_Interpretor& theDI)
{
  static int THE_XSDRAW_FunctionsShape_initactor = 0;
  if (THE_XSDRAW_FunctionsShape_initactor)
  {
    return;
  }
  THE_XSDRAW_FunctionsShape_initactor = 1;

  Standard_CString aGroup = "DE: General";
  theDI.Add("tpdraw", "[mode:item or root]  num|*  [nomvar] Passes an ITEM to Shape Draw (Start or Result)", XSControl_tpdraw, aGroup);
  theDI.Add("tpcompound", "name:cstring [givelist] : -> compound with Shapes Root or from givelist", __FILE__, XSControl_tpcompound, aGroup);
  theDI.Add("trdraw", "results ->DRAW : all;  or num [name] : from ent.num -> DRAW [name/tread_num]", __FILE__, XSControl_traccess, aGroup);
  theDI.Add("trsave", "results ->files : all;  or num [name] : from ent.num -> DRAW [name/tread_num]", __FILE__, XSControl_traccess, aGroup);
  theDI.Add("trcomp", "results -> 1 compound -> DRAW + name optional", __FILE__, XSControl_traccess, aGroup);
  theDI.Add("trscomp", "results -> 1 compound -> file + name optional", __FILE__, XSControl_traccess, aGroup);
  theDI.Add("fromshape", "shape [level=1]: imported/exported entity (when known)", __FILE__, XSControl_fromshape, aGroup);
  theDI.Add("trconnexent", "name of draw shape : entities -> connected shapes (when known)", __FILE__, XSControl_trconnexentities, aGroup);
  theDI.Add("twrite", "shape : transfer write for this shape, AFTER newmodel !", __FILE__, XSControl_twrite, aGroup);
}
