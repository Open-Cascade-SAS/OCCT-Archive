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

#include <XSDRAWIGES.hxx>

#include <BRepTools.hxx>
#include <DBRep.hxx>
#include <DDocStd.hxx>
#include <DDocStd_DrawDocument.hxx>
#include <DE_ConfigurationNode.hxx>
#include <DE_Wrapper.hxx>
#include <Draw.hxx>
#include <DrawTrSurf.hxx>
#include <Draw_Interpretor.hxx>
#include <Draw_PluginMacro.hxx>
#include <Draw_ProgressIndicator.hxx>
#include <OSD_OpenFile.hxx>
#include <OSD_Path.hxx>
#include <IGESCAFControl_ConfigurationNode.hxx>
#include <IGESCAFControl_Provider.hxx>
#include <IGESControl_Controller.hxx>
#include <IGESControl_Reader.hxx>
#include <IGESControl_Writer.hxx>
#include <IGESData_IGESEntity.hxx>
#include <IGESData_IGESModel.hxx>
#include <Interface_Static.hxx>
#include <Message.hxx>
#include <XSControl_WorkSession.hxx>
#include <XSDRAWBase.hxx>
#include <XSAlgo.hxx>
#include <XSAlgo_AlgoContainer.hxx>
#include <XSControl_TransferReader.hxx>
#include <XSControl_WorkSession.hxx>
#include <TColStd_Array1OfAsciiString.hxx>
#include <TColStd_MapIteratorOfMapOfTransient.hxx>
#include <TDataStd_Name.hxx>
#include <TDocStd_Application.hxx>
#include <TopoDS_Shape.hxx>
#include <Transfer_IteratorOfProcessForTransient.hxx>
#include <Transfer_TransientProcess.hxx>

//=======================================================================
//function : WriteShape
//purpose  : Creates a file Shape_'number'
//=======================================================================
void WriteShape(const TopoDS_Shape& shape,
                const Standard_Integer number)
{
  char fname[110];
  sprintf(fname, "Shape_%d", number);
  std::ofstream f(fname, std::ios::out | std::ios::binary);
  std::cout << "Output file name : " << fname << std::endl;
  f << "DBRep_DrawableShape\n";

  BRepTools::Write(shape, f);
  f.close();
}

//=======================================================================
//function : XSDRAW_CommandPart
//purpose  :
//=======================================================================
TCollection_AsciiString XSDRAW_CommandPart(Standard_Integer argc,
                                           const char** argv,
                                           const Standard_Integer argf)
{
  TCollection_AsciiString res;
  for (Standard_Integer i = argf; i < argc; i++)
  {
    if (i > argf) res.AssignCat(" ");
    res.AssignCat(argv[i]);
  }
  return res;
}

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
//function : FileAndVar
//purpose  : 
//=======================================================================
Standard_Boolean FileAndVar(const Handle(XSControl_WorkSession)& session,
                            const Standard_CString file,
                            const Standard_CString var,
                            const Standard_CString def,
                            TCollection_AsciiString& resfile,
                            TCollection_AsciiString& resvar)
{
  Standard_Boolean iafic = Standard_True;
  resfile.Clear();  resvar.Clear();
  if (file)
    if (file[0] == '\0' ||
        (file[0] == '.' && file[1] == '\0')) iafic = Standard_False;
  if (!iafic) resfile.AssignCat(session->LoadedFile());
  else        resfile.AssignCat(file);

  if (var && var[0] != '\0' && (var[0] != '.' || var[1] != '\0'))
    resvar.AssignCat(var);
  else if (resfile.Length() == 0) resvar.AssignCat(def);
  else
  {
    Standard_Integer nomdeb, nomfin;
    nomdeb = resfile.SearchFromEnd("/");
    if (nomdeb <= 0) nomdeb = resfile.SearchFromEnd("\\");  // pour NT
    if (nomdeb < 0) nomdeb = 0;
    nomfin = resfile.SearchFromEnd(".");
    if (nomfin < nomdeb) nomfin = resfile.Length() + 1;
    resvar = resfile.SubString(nomdeb + 1, nomfin - 1);
  }
  return iafic;
}

//=======================================================================
//function : igesbrep
//purpose  :
//=======================================================================
static Standard_Integer igesbrep(Draw_Interpretor& theDI,
                                 Standard_Integer theNbArgs,
                                 const char** theArgVec)
{
  Handle(XSControl_WorkSession) aWS = XSDRAWBase::Session();
  Handle(IGESControl_Controller) aCtl =
    Handle(IGESControl_Controller)::DownCast(aWS->NormAdaptor());
  if (aCtl.IsNull())
  {
    aWS->SelectNorm("IGES");
  }

  // Progress indicator
  Handle(Draw_ProgressIndicator) progress = new Draw_ProgressIndicator(theDI, 1);
  Message_ProgressScope aPSRoot(progress->Start(), "Reading", 100);

  IGESControl_Reader Reader(XSDRAWBase::Session(), Standard_False);
  Standard_Boolean aFullMode = Standard_True;
  Reader.WS()->SetModeStat(aFullMode);

  TCollection_AsciiString fnom, rnom;

  Standard_Boolean modfic = FileAndVar
  (aWS, theArgVec[1], theArgVec[2], "IGESBREP", fnom, rnom);
  if (modfic) theDI << " File IGES to read : " << fnom.ToCString() << "\n";
  else        theDI << " Model taken from the session : " << fnom.ToCString() << "\n";
  theDI << " -- Names of variables BREP-DRAW prefixed by : " << rnom.ToCString() << "\n";
  IFSelect_ReturnStatus readstat = IFSelect_RetVoid;

#ifdef CHRONOMESURE
  OSD_Timer Chr; Chr.Reset();
  IDT_SetLevel(3);
#endif

  // Reading the file
  aPSRoot.SetName("Loading");
  progress->Show(aPSRoot);

  if (modfic) readstat = Reader.ReadFile(fnom.ToCString());
  else  if (XSDRAWBase::Session()->NbStartingEntities() > 0) readstat = IFSelect_RetDone;

  aPSRoot.Next(20); // On average loading takes 20% 
  if (aPSRoot.UserBreak())
    return 1;

  if (readstat != IFSelect_RetDone)
  {
    if (modfic) theDI << "Could not read file " << fnom.ToCString() << " , abandon\n";
    else theDI << "No model loaded\n";
    return 1;
  }
  // Choice of treatment
  Standard_Boolean fromtcl = (theNbArgs > 3);
  Standard_Integer modepri = 1, nent, nbs;
  if (fromtcl) modepri = 4;

  while (modepri)
  {
    //Roots for transfer are defined before setting mode ALL or OnlyVisible - gka 
    //mode OnlyVisible does not work.
    // nent = Reader.NbRootsForTransfer();
    if (!fromtcl)
    {
      std::cout << "Mode (0 End, 1 Visible Roots, 2 All Roots, 3 Only One Entity, 4 Selection) :" << std::flush;
      modepri = -1;

      // amv 26.09.2003 : this is used to avoid error of enter's simbol        
      char str[80];
      std::cin >> str;
      modepri = Draw::Atoi(str);
    }

    if (modepri == 0)
    {  //fin
      theDI << "Bye and good luck! \n";
      break;
    }

    else if (modepri <= 2)
    {  // 1 : Visible Roots, 2 : All Roots
      theDI << "All Geometry Transfer\n";
      theDI << "spline_continuity (read) : " << Interface_Static::IVal("read.iges.bspline.continuity") << " (0 : no modif, 1 : C1, 2 : C2)\n";
      theDI << "  To modify : command  param read.iges.bspline.continuity\n";
      const Handle(XSControl_WorkSession)& thesession = Reader.WS();
      thesession->TransferReader()->Context().Clear();

      aPSRoot.SetName("Translation");
      progress->Show(aPSRoot);

      if (modepri == 1) Reader.SetReadVisible(Standard_True);
      Reader.TransferRoots(aPSRoot.Next(80));

      if (aPSRoot.UserBreak())
        return 1;

      // result in only one shape for all the roots
      //        or in one shape for one root.
      theDI << "Count of shapes produced : " << Reader.NbShapes() << "\n";
      Standard_Integer answer = 1;
      if (Reader.NbShapes() > 1)
      {
        std::cout << " pass(0)  one shape for all (1)\n or one shape per root (2)\n + WriteBRep (one for all : 3) (one per root : 4) : " << std::flush;
        answer = -1;
        //amv 26.09.2003                                                        
        char str_a[80];
        std::cin >> str_a;
        answer = Draw::Atoi(str_a);
      }
      if (answer == 0) continue;
      if (answer == 1 || answer == 3)
      {
        TopoDS_Shape shape = Reader.OneShape();
        // save the shape
        if (shape.IsNull()) { theDI << "No Shape produced\n"; continue; }
        char fname[110];
        Sprintf(fname, "%s", rnom.ToCString());
        theDI << "Saving shape in variable Draw : " << fname << "\n";
        if (answer == 3) WriteShape(shape, 1);
        try
        {
          OCC_CATCH_SIGNALS
            DBRep::Set(fname, shape);
        }
        catch (Standard_Failure const& anException)
        {
          theDI << "** Exception : ";
          theDI << anException.GetMessageString();
          theDI << " ** Skip\n";
          theDI << "Saving shape in variable Draw : " << fname << "\n";
          WriteShape(shape, 1);
        }
      }

      else if (answer == 2 || answer == 4)
      {
        Standard_Integer numshape = Reader.NbShapes();
        for (Standard_Integer inum = 1; inum <= numshape; inum++)
        {
          // save all the shapes
          TopoDS_Shape shape = Reader.Shape(inum);
          if (shape.IsNull()) { theDI << "No Shape produced\n"; continue; }
          char fname[110];
          Sprintf(fname, "%s_%d", rnom.ToCString(), inum);
          theDI << "Saving shape in variable Draw : " << fname << "\n";
          if (answer == 4) WriteShape(shape, inum);
          try
          {
            OCC_CATCH_SIGNALS
              DBRep::Set(fname, shape);
          }
          catch (Standard_Failure const& anException)
          {
            theDI << "** Exception : ";
            theDI << anException.GetMessageString();
            theDI << " ** Skip\n";
          }
        }
      }
      else return 0;
    }

    else if (modepri == 3)
    {  // One Entity
      std::cout << "Only One Entity" << std::endl;
      std::cout << "spline_continuity (read) : " << Interface_Static::IVal("read.iges.bspline.continuity") << " (0 : no modif, 1 : C1, 2 : C2)" << std::endl;
      std::cout << "  To modify : command  param read.iges.bspline.continuity" << std::endl;
      std::cout << " give the number of the Entity : " << std::flush;
      nent = GiveEntityNumber(aWS, "");

      if (!Reader.TransferOne(nent))
        theDI << "Transfer entity n0 " << nent << " : no result\n";
      else
      {
        nbs = Reader.NbShapes();
        char shname[30];  Sprintf(shname, "%s_%d", rnom.ToCString(), nent);
        theDI << "Transfer entity n0 " << nent << " OK  -> DRAW Shape: " << shname << "\n";
        theDI << "Now, " << nbs << " Shapes produced\n";
        TopoDS_Shape sh = Reader.Shape(nbs);
        DBRep::Set(shname, sh);
      }
    }

    else if (modepri == 4)
    {   // Selection
      Standard_Integer answer = 1;
      Handle(TColStd_HSequenceOfTransient)  list;

      //  Selection, nommee ou via tcl. tcl : raccourcis admis
      //   * donne iges-visible + xst-transferrable-roots
      //   *r donne xst-model-roots (TOUTES racines)

      if (fromtcl && theArgVec[3][0] == '*' && theArgVec[3][1] == '\0')
      {
        theDI << "All Geometry Transfer\n";
        theDI << "spline_continuity (read) : " << Interface_Static::IVal("read.iges.bspline.continuity") << " (0 : no modif, 1 : C1, 2 : C2)\n";
        theDI << "  To modify : command  param read.iges.bspline.continuity\n";
        const Handle(XSControl_WorkSession)& thesession = Reader.WS();
        thesession->TransferReader()->Context().Clear();

        aPSRoot.SetName("Translation");
        progress->Show(aPSRoot);

        Reader.SetReadVisible(Standard_True);
        Reader.TransferRoots(aPSRoot.Next(80));

        if (aPSRoot.UserBreak())
          return 1;

        // result in only one shape for all the roots
        TopoDS_Shape shape = Reader.OneShape();
        // save the shape
        char fname[110];
        Sprintf(fname, "%s", rnom.ToCString());
        theDI << "Saving shape in variable Draw : " << fname << "\n";
        try
        {
          OCC_CATCH_SIGNALS
            DBRep::Set(fname, shape);
        }
        catch (Standard_Failure const& anException)
        {
          theDI << "** Exception : ";
          theDI << anException.GetMessageString();
          theDI << " ** Skip\n";
          theDI << "Saving shape in variable Draw : " << fname << "\n";
          WriteShape(shape, 1);
        }
        return 0;
      }

      if (fromtcl)
      {
        modepri = 0;    // d office, une seule passe
        if (theArgVec[3][0] == '*' && theArgVec[3][1] == 'r' && theArgVec[3][2] == '\0')
        {
          theDI << "All Roots : ";
          list = XSDRAWBase::Session()->GiveList("xst-model-roots");
        }
        else
        {
          TCollection_AsciiString compart = XSDRAW_CommandPart(theNbArgs, theArgVec, 3);
          theDI << "List given by " << compart.ToCString() << " : ";
          list = XSDRAWBase::Session()->GiveList(compart.ToCString());
        }
        if (list.IsNull())
        {
          theDI << "No list defined. Give a selection name or * for all visible transferrable roots\n";
          continue;
        }
      }
      else
      {
        std::cout << "Name of Selection :" << std::flush;
        list = XSDRAWBase::Session()->GiveList("");
        if (list.IsNull()) { std::cout << "No list defined" << std::endl; continue; }
      }

      Standard_Integer nbl = list->Length();
      theDI << "Nb entities selected : " << nbl << "\n";
      if (nbl == 0) continue;
      while (answer)
      {
        if (!fromtcl)
        {
          std::cout << "Choice: 0 abandon  1 transfer all  2 with confirmation  3 list n0s ents :" << std::flush;
          answer = -1;
          // anv 26.09.2003                                                     
          char str_answer[80];
          std::cin >> str_answer;
          answer = Draw::Atoi(str_answer);
        }
        if (answer <= 0 || answer > 3) continue;
        if (answer == 3)
        {
          for (Standard_Integer ill = 1; ill <= nbl; ill++)
          {
            Handle(Standard_Transient) ent = list->Value(ill);
            theDI << "  ";// model->Print(ent,theDI);
          }
          theDI << "\n";
        }
        if (answer == 1 || answer == 2)
        {
          Standard_Integer nbt = 0;
          Handle(XSControl_WorkSession) thesession = Reader.WS();

          aPSRoot.SetName("Translation");
          progress->Show(aPSRoot);

          Message_ProgressScope aPS(aPSRoot.Next(80), "Root", nbl);
          for (Standard_Integer ill = 1; ill <= nbl && aPS.More(); ill++)
          {
            nent = Reader.Model()->Number(list->Value(ill));
            if (nent == 0) continue;
            if (!Reader.TransferOne(nent, aPS.Next()))
              theDI << "Transfer entity n0 " << nent << " : no result\n";
            else
            {
              nbs = Reader.NbShapes();
              char shname[30];  Sprintf(shname, "%s_%d", rnom.ToCString(), nbs);
              theDI << "Transfer entity n0 " << nent << " OK  -> DRAW Shape: " << shname << "\n";
              theDI << "Now, " << nbs << " Shapes produced\n";
              TopoDS_Shape sh = Reader.Shape(nbs);
              DBRep::Set(shname, sh);
              nbt++;
            }
          }
          if (aPSRoot.UserBreak())
            return 1;
          theDI << "Nb Shapes successfully produced : " << nbt << "\n";
          answer = 0;  // on ne reboucle pas
        }
      }
    }
    else theDI << "Unknown mode n0 " << modepri << "\n";
  }
  return 0;
}

//=======================================================================
//function : testread
//purpose  :
//=======================================================================
static Standard_Integer testread(Draw_Interpretor& theDI,
                                 Standard_Integer theNbArgs,
                                 const char** theArgVec)
{
  if (theNbArgs != 3)
  {
    theDI << "ERROR in " << theArgVec[0] << "Wrong Number of Arguments.\n";
    theDI << " Usage : " << theArgVec[0] << " file_name shape_name\n";
    return 1;
  }
  IGESControl_Reader Reader;
  Standard_CString filename = theArgVec[1];
  IFSelect_ReturnStatus readstat = Reader.ReadFile(filename);
  theDI << "Status from reading IGES file " << filename << " : ";
  switch (readstat)
  {
    case IFSelect_RetVoid: { theDI << "empty file\n"; return 1; }
    case IFSelect_RetDone: { theDI << "file read\n";    break; }
    case IFSelect_RetError: { theDI << "file not found\n";   return 1; }
    case IFSelect_RetFail: { theDI << "error during read\n";  return 1; }
    default: { theDI << "failure\n";   return 1; }
  }
  Reader.TransferRoots();
  TopoDS_Shape shape = Reader.OneShape();
  DBRep::Set(theArgVec[2], shape);
  theDI << "Count of shapes produced : " << Reader.NbShapes() << "\n";
  return 0;
}

//=======================================================================
//function : brepiges
//purpose  :
//=======================================================================
static Standard_Integer brepiges(Draw_Interpretor& theDI,
                                 Standard_Integer theNbArgs,
                                 const char** theArgVec)
{
  Handle(XSControl_WorkSession) aWS = XSDRAWBase::Session();
  aWS->SelectNorm("IGES");
  // ecriture dans le model d'une entite :
  //    -  model_AddEntity(ent)             : ecriture de l`entite seule
  //    -  model->AddWithRefs(ent, protocol): ecriture de l`entite et eventuellement 
  //                                          . de sa matrice de transformation 
  //                                          . de ses sous-elements

  IGESControl_Writer ICW(Interface_Static::CVal("write.iges.unit"),
                         Interface_Static::IVal("write.iges.brep.mode"));
  theDI << "unit (write) : " << Interface_Static::CVal("write.iges.unit") << "\n";
  theDI << "mode  write  : " << Interface_Static::CVal("write.iges.brep.mode") << "\n";
  theDI << "  To modify : command  param\n";

  //  Mode d emploi (K4B ->) : brepiges shape [+shape][ +shape] nomfic
  //   c a d tant qu il y a des + on ajoute ce qui suit
  const char* nomfic = NULL;
  Standard_Integer npris = 0;

  Handle(Draw_ProgressIndicator) progress = new Draw_ProgressIndicator(theDI, 1);
  Message_ProgressScope aPSRoot(progress->Start(), "Translating", 100);
  progress->Show(aPSRoot);

  Message_ProgressScope aPS(aPSRoot.Next(90), NULL, theNbArgs);
  for (Standard_Integer i = 1; i < theNbArgs && aPS.More(); i++)
  {
    const char* nomvar = theArgVec[i];
    if (theArgVec[i][0] == '+') nomvar = &(theArgVec[i])[1];
    else if (i > 1) { nomfic = theArgVec[i];  break; }
    TopoDS_Shape Shape = DBRep::Get(nomvar);
    if (ICW.AddShape(Shape, aPS.Next())) npris++;
    else if (ICW.AddGeom(DrawTrSurf::GetCurve(nomvar))) npris++;
    else if (ICW.AddGeom(DrawTrSurf::GetSurface(nomvar))) npris++;
  }
  ICW.ComputeModel();

  if (aPSRoot.UserBreak())
    return 1;
  aPSRoot.SetName("Writing");
  progress->Show(aPSRoot);

  theDI << npris << " Shapes written, giving " << ICW.Model()->NbEntities() << " Entities\n";

  if (!nomfic) // delayed write
  {
    theDI << " Now, to write a file, command : writeall filename\n";
    return 0;
  }

  // write file
  if (!ICW.Write(nomfic))
  {
    theDI << " Error: could not write file " << nomfic << "\n";
    return 1;
  }
  theDI << " File " << nomfic << " written\n";
  aWS->SetModel(ICW.Model());

  return 0;
}

//=======================================================================
//function : testwrite
//purpose  :
//=======================================================================
static Standard_Integer testwrite(Draw_Interpretor& theDI,
                                  Standard_Integer theNbArgs,
                                  const char** theArgVec)
{
  if (theNbArgs != 3)
  {
    theDI << "ERROR in " << theArgVec[0] << "Wrong Number of Arguments.\n";
    theDI << " Usage : " << theArgVec[0] << " file_name shape_name\n";
    return 1;
  }
  IGESControl_Writer Writer;
  Standard_CString filename = theArgVec[1];
  TopoDS_Shape shape = DBRep::Get(theArgVec[2]);
  Standard_Boolean ok = Writer.AddShape(shape);
  if (!ok)
  {
    theDI << "Shape not add\n";
    return 1;
  }

  if (!(Writer.Write(filename)))
  {
    theDI << "Error on writing file\n";
    return 1;
  }
  theDI << "File Is Written\n";
  return 0;
}

//=======================================================================
//function : igesparam
//purpose  :
//=======================================================================
static Standard_Integer igesparam(Draw_Interpretor& theDI,
                                  Standard_Integer,
                                  const char**)
{
  //  liste des parametres
  theDI << "List of parameters which control IGES :\n";
  theDI << "  unit : write.iges.unit\n  mode write : write.iges.brep.mode\n  spline_continuity (read) : read.iges.bspline.continuity\nSee definition by  defparam, read/edit value by  param\n";
  theDI << "unit (write) : " << Interface_Static::CVal("write.iges.unit") << "\n";
  theDI << "mode  write  : " << Interface_Static::CVal("write.iges.brep.mode") << "\n";
  theDI << "spline_continuity (read) : " << Interface_Static::IVal("read.iges.bspline.continuity") << " (0 : no modif, 1 : C1, 2 : C2)\n";
  theDI << "\n To modifier, param nom_param new_val\n";
  return 0;
}

//=======================================================================
//function : XSDRAWIGES_tplosttrim
//purpose  :
//=======================================================================
static Standard_Integer XSDRAWIGES_tplosttrim(Draw_Interpretor& theDI,
                                              Standard_Integer theNbArgs,
                                              const char** theArgVec)
{
  Handle(XSControl_WorkSession) aWS = XSDRAWBase::Session();
  const Handle(Transfer_TransientProcess)& TP = aWS->TransferReader()->TransientProcess();
  TColStd_Array1OfAsciiString strarg(1, 3);
  TColStd_Array1OfAsciiString typarg(1, 3);
  strarg.SetValue(1, "xst-type(CurveOnSurface)");
  strarg.SetValue(2, "xst-type(Boundary)");
  strarg.SetValue(3, "xst-type(Loop)");
  typarg.SetValue(1, "IGESGeom_TrimmedSurface");
  typarg.SetValue(2, "IGESGeom_BoundedSurface");
  typarg.SetValue(3, "IGESSolid_Face");
  if (TP.IsNull()) { theDI << "No Transfer Read\n"; return 1; }
  Standard_Integer nbFaces = 0, totFaces = 0;
  Transfer_IteratorOfProcessForTransient itrp = TP->AbnormalResult();
  Standard_Integer k = 0;
  if (theNbArgs > 1)
  {
    //    TCollection_AsciiString Arg = pilot->Word(1);
    TCollection_AsciiString Arg(theArgVec[1]);
    for (k = 1; k <= 3; k++)
    {
      if (typarg.Value(k).Location(Arg, 1, typarg.Value(k).Length()) != 0) break;
    }
  }
  if (k == 4) { theDI << "Invalid argument\n"; return 0; }
  for (Standard_Integer j = 1; j <= 3; j++)
  {
    TColStd_MapOfTransient aMap;
    if (theNbArgs == 1) k = j;
    Handle(TColStd_HSequenceOfTransient) list = aWS->GiveList(strarg.Value(k).ToCString());
    if (!list.IsNull()) itrp.Filter(list);
    else
    {
      theDI << "No untrimmed faces\n";
      return 0;
    }
    for (itrp.Start(); itrp.More(); itrp.Next())
    {
      Handle(Standard_Transient) ent = itrp.Starting();
      Handle(TColStd_HSequenceOfTransient) super = aWS->Sharings(ent);
      if (!super.IsNull())
      {
        Standard_Integer nb = super->Length();
        if (nb > 0)
        {
          for (Standard_Integer i = 1; i <= nb; i++)
            if (super->Value(i)->IsKind(typarg.Value(k).ToCString()))
            {
              if (aMap.Add(super->Value(i))) nbFaces++;
            }
        }
      }
    }
    if (nbFaces != 0)
    {
      if (j == 1) theDI << "Number of untrimmed faces: \n";
      switch (k)
      {
        case 1:
          theDI << "Trimmed Surface: \n"; break;
        case 2:
          theDI << "Bounded Surface: \n"; break;
        case 3:
          theDI << "Face: \n"; break;
      }

      TColStd_MapIteratorOfMapOfTransient itmap;
      Standard_SStream aTmpStream;
      for (itmap.Initialize(aMap); itmap.More(); itmap.Next())
      {
        aWS->Model()->Print(itmap.Key(), aTmpStream);
        aTmpStream << "  ";
      }
      theDI << aTmpStream.str().c_str();
      theDI << "\n";
      theDI << "\nNumber:" << nbFaces << "\n";
      totFaces += nbFaces;
    }
    if (theNbArgs > 1) break;
    nbFaces = 0;
  }

  if (totFaces == 0) theDI << "No untrimmed faces\n";
  else              theDI << "Total number :" << totFaces << "\n";
  return 0;
}

//=======================================================================
//function : XSDRAWIGES_TPSTAT
//purpose  :
//=======================================================================
static Standard_Integer XSDRAWIGES_TPSTAT(Draw_Interpretor& theDI,
                                          Standard_Integer theNbArgs,
                                          const char** theArgVec)
{
  Handle(XSControl_WorkSession) aWS = XSDRAWBase::Session();
  const Standard_CString arg1 = theArgVec[1];
  const Handle(Transfer_TransientProcess)& TP = aWS->TransferReader()->TransientProcess();
  IGESControl_Reader read;
  Handle(Interface_InterfaceModel) model = TP->Model();
  if (model.IsNull()) { theDI << "No Transfer Read\n"; return -1; }
  Handle(XSControl_WorkSession) thesession = read.WS();
  thesession->SetMapReader(TP);
  Standard_Integer mod1 = 0;
  if (theNbArgs > 1)
  {
    char a2 = arg1[1]; if (a2 == '\0') a2 = '!';
    switch (arg1[0])
    {
      case 'g': read.PrintTransferInfo(IFSelect_FailAndWarn, IFSelect_GeneralInfo); break;
      case 'c': read.PrintTransferInfo(IFSelect_FailAndWarn, IFSelect_CountByItem); break;
      case 'C': read.PrintTransferInfo(IFSelect_FailAndWarn, IFSelect_ListByItem); break;
      case 'r': read.PrintTransferInfo(IFSelect_FailAndWarn, IFSelect_ResultCount); break;
      case 's': read.PrintTransferInfo(IFSelect_FailAndWarn, IFSelect_Mapping); break;
      case '?': mod1 = -1; break;
      default: mod1 = -2; break;
    }
  }
  if (mod1 < -1) theDI << "Unknown Mode\n";
  if (mod1 < 0)
  {
    theDI << "Modes available :\n"
      << "g : general    c : checks (count)  C (list)\n"
      << "r : number of CasCade resulting shapes\n"
      << "s : mapping between IGES entities and CasCade shapes\n";
    if (mod1 < -1) return -1;
    return 0;
  }
  return 0;
}

//=======================================================================
//function : etest
//purpose  :
//=======================================================================
static Standard_Integer etest(Draw_Interpretor& theDI,
                              Standard_Integer theNbArgs,
                              const char** theArgVec)
{
  if (theNbArgs < 3)
  {
    theDI << "etest igesfile shape\n";
    return 0;
  }
  IGESControl_Reader aReader;
  aReader.ReadFile(theArgVec[1]);
  aReader.SetReadVisible(Standard_True);
  aReader.TransferRoots();
  TopoDS_Shape shape = aReader.OneShape();
  DBRep::Set(theArgVec[2], shape);
  return 0;
}

//=======================================================================
//function : ReadIges
//purpose  : Read IGES to DECAF document
//=======================================================================
static Standard_Integer ReadIges(Draw_Interpretor& theDI,
                                 Standard_Integer theNbArgs,
                                 const char** theArgVec)
{
  if (theNbArgs < 3)
  {
    theDI << "Use: " << theArgVec[0] << " Doc filename [mode]: read IGES file to a document\n";
    return 0;
  }
  Handle(IGESCAFControl_ConfigurationNode) aNode =
    new IGESCAFControl_ConfigurationNode();
  Standard_Integer onlyvisible = Interface_Static::IVal("read.iges.onlyvisible");
  aNode->InternalParameters.ReadOnlyVisible = onlyvisible == 1;
  if (theNbArgs == 4)
  {
    Standard_Boolean aMode = Standard_True;
    for (Standard_Integer i = 0; theArgVec[3][i]; i++)
      switch (theArgVec[3][i])
      {
        case '-': aMode = Standard_False; break;
        case '+': aMode = Standard_True; break;
        case 'c': aNode->InternalParameters.ReadColor = aMode; break;
        case 'n': aNode->InternalParameters.ReadName = aMode; break;
        case 'l': aNode->InternalParameters.ReadLayer = aMode; break;
      }
  }
  Handle(TDocStd_Document) aDoc;
  if (!DDocStd::GetDocument(theArgVec[1], aDoc, Standard_False))
  {
    Handle(TDocStd_Application) anApp = DDocStd::GetApplication();
    anApp->NewDocument("BinXCAF", aDoc);
    TDataStd_Name::Set(aDoc->GetData()->Root(), theArgVec[1]);
    Handle(DDocStd_DrawDocument) aDrawD = new DDocStd_DrawDocument(aDoc);
    Draw::Set(theArgVec[1], aDrawD);
  }
  aNode->GlobalParameters.LengthUnit = XSDRAWBase::GetLengthUnit(aDoc);
  Handle(Draw_ProgressIndicator) aProgress = new Draw_ProgressIndicator(theDI);
  Handle(IGESCAFControl_Provider) aProvider =
    new IGESCAFControl_Provider(aNode);
  aProvider->SetToUpdateStaticParameters(false);
  Handle(XSControl_WorkSession) aWS = XSDRAWBase::Session();
  if (!aProvider->Read(theArgVec[2], aDoc, aWS, aProgress->Start()))
  {
    theDI << "Error: Can't read IGES file\n";
    return 1;
  }
  XSDRAWBase::CollectActiveWorkSessions(aWS, theArgVec[2], XSDRAWBase::WorkSessionList());
  Message::SendInfo() << "Document saved with name " << theArgVec[1];
  return 0;
}

//=======================================================================
//function : WriteIges
//purpose  : Write DECAF document to IGES
//=======================================================================
//=======================================================================
static Standard_Integer WriteIges(Draw_Interpretor& theDI,
                                  Standard_Integer theNbArgs,
                                  const char** theArgVec)
{
  if (theNbArgs < 3)
  {
    theDI << "Use: " << theArgVec[0] << " Doc filename [mode]: write document to IGES file\n";
    return 0;
  }
  Handle(IGESCAFControl_ConfigurationNode) aNode =
    new IGESCAFControl_ConfigurationNode();
  Handle(TDocStd_Document) aDoc;
  DDocStd::GetDocument(theArgVec[1], aDoc);
  if (aDoc.IsNull())
  {
    theDI << theArgVec[1] << " is not a document\n";
    return 1;
  }
  if (theNbArgs == 4)
  {
    Standard_Boolean aMode = Standard_True;
    for (Standard_Integer i = 0; theArgVec[3][i]; i++)
      switch (theArgVec[3][i])
      {
        case '-': aMode = Standard_False; break;
        case '+': aMode = Standard_True; break;
        case 'c': aNode->InternalParameters.WriteColor = aMode; break;
        case 'n': aNode->InternalParameters.WriteName = aMode; break;
        case 'l': aNode->InternalParameters.WriteLayer = aMode; break;
      }
  }
  aNode->GlobalParameters.LengthUnit = XSDRAWBase::GetLengthUnit(aDoc);
  Handle(Draw_ProgressIndicator) aProgress = new Draw_ProgressIndicator(theDI);
  Handle(IGESCAFControl_Provider) aProvider =
    new IGESCAFControl_Provider(aNode);
  aProvider->SetToUpdateStaticParameters(false);
  Handle(XSControl_WorkSession) aWS = XSDRAWBase::Session();
  const TCollection_AsciiString aPath = theArgVec[2];
  if (!aProvider->Write(aPath, aDoc, aWS, aProgress->Start()))
  {
    theDI << "Error: Can't write IGES file\n";
    return 1;
  }
  XSDRAWBase::CollectActiveWorkSessions(aWS, aPath, XSDRAWBase::WorkSessionList());
  return 0;
}

//=======================================================================
//function : InitToBRep
//purpose  :
//=======================================================================
void XSDRAWIGES::Factory(Draw_Interpretor& theDI)
{
  static Standard_Boolean aIsActivated = Standard_False;
  if (aIsActivated)
  {
    return;
  }
  aIsActivated = Standard_True;

  IGESControl_Controller::Init();
  DE_Wrapper::GlobalWrapper()->Bind(new IGESCAFControl_ConfigurationNode());

  const char* aGroup = "DE: IGES";

  theDI.Add("tplosttrim", "number of untrimmed faces during last transfer", __FILE__, XSDRAWIGES_tplosttrim, aGroup);
  theDI.Add("igesbrep", "igesbrep [file else already loaded model] [name DRAW]", __FILE__, igesbrep, aGroup);
  theDI.Add("testreadiges", "testreadiges [file else already loaded model] [name DRAW]", __FILE__, testread, aGroup);
  theDI.Add("igesparam", "igesparam ->list, + name ->one param, + name val->change", __FILE__, igesparam, aGroup);
  theDI.Add("TPSTAT", " ", __FILE__, XSDRAWIGES_TPSTAT, aGroup);
  theDI.Add("etest", "test of eviewer", __FILE__, etest, aGroup);

  theDI.Add("ReadIges", "Doc filename: Read IGES file to DECAF document", __FILE__, ReadIges, aGroup);
  theDI.Add("WriteIges", "Doc filename: Write DECAF document to IGES file", __FILE__, WriteIges, aGroup);
  theDI.Add("igesread", "igesread [file else already loaded model] [name DRAW]", __FILE__, igesbrep, aGroup);
  theDI.Add("igeswrite", "igesread [file else already loaded model] [name DRAW]", __FILE__, brepiges, aGroup);
  theDI.Add("brepiges", "brepiges sh1 [+sh2 [+sh3 ..]] filename.igs", __FILE__, brepiges, aGroup);
  theDI.Add("testwriteiges", "testwriteiges filename.igs shape", __FILE__, testwrite, aGroup);
}

// Declare entry point PLUGINFACTORY
DPLUGIN(XSDRAWIGES)