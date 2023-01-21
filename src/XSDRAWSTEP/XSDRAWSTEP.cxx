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

#include <XSDRAWSTEP.hxx>

#include <DBRep.hxx>
#include <DDocStd.hxx>
#include <DDocStd_DrawDocument.hxx>
#include <Draw.hxx>
#include <Draw_Interpretor.hxx>
#include <Draw_PluginMacro.hxx>
#include <Draw_ProgressIndicator.hxx>
#include <OSD_OpenFile.hxx>
#include <OSD_Path.hxx>
#include <Interface_Static.hxx>
#include <Message.hxx>
#include <STEPCAFControl_ConfigurationNode.hxx>
#include <STEPCAFControl_Provider.hxx>
#include <STEPControl_ActorWrite.hxx>
#include <STEPControl_Controller.hxx>
#include <STEPControl_Reader.hxx>
#include <STEPControl_StepModelType.hxx>
#include <STEPControl_Writer.hxx>
#include <StepData_StepModel.hxx>
#include <StepGeom_Axis2Placement3d.hxx>
#include <StepSelect_Activator.hxx>
#include <STEPSelections_AssemblyExplorer.hxx>
#include <STEPSelections_Counter.hxx>
#include <StepToTopoDS_MakeTransformed.hxx>
#include <XSControl_WorkSession.hxx>
#include <XSDRAWBase.hxx>
#include <XSAlgo.hxx>
#include <XSAlgo_AlgoContainer.hxx>
#include <XSControl_WorkSession.hxx>
#include <TDataStd_Name.hxx>
#include <TDocStd_Application.hxx>
#include <TopoDS_Shape.hxx>
#include <UnitsMethods.hxx>

namespace
{
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
}

//=======================================================================
//function : stepread
//purpose  :
//=======================================================================
static Standard_Integer stepread(Draw_Interpretor& theDI,
                                 Standard_Integer theNbArgs,
                                 const char** theArgVec)
{
  if (theNbArgs < 3)
  {
    theDI << "Use: stepread  [file] [f or r (type of model full or reduced)]\n";
    return 1;
  }

  // Progress indicator
  Handle(Draw_ProgressIndicator) progress = new Draw_ProgressIndicator(theDI, 1);
  Message_ProgressScope aPSRoot(progress->Start(), "Reading", 100);

  STEPControl_Reader sr(XSDRAWBase::Session(), Standard_False);
  TCollection_AsciiString fnom, rnom;
  Standard_Boolean modfic = FileAndVar
  (XSDRAWBase::Session(), theArgVec[1], theArgVec[2], "STEP", fnom, rnom);
  if (modfic) theDI << " File STEP to read : " << fnom.ToCString() << "\n";
  else        theDI << " Model taken from the session : " << fnom.ToCString() << "\n";
  theDI << " -- Names of variables BREP-DRAW prefixed by : " << rnom.ToCString() << "\n";
  IFSelect_ReturnStatus readstat = IFSelect_RetVoid;

  aPSRoot.SetName("Loading");
  progress->Show(aPSRoot);

  Standard_Boolean fromtcl = Standard_False;
  Standard_Boolean aFullMode = Standard_False;
  Standard_Integer k = 3;
  if (theNbArgs > k)
  {
    if (theArgVec[k][0] == 'f' || theArgVec[3][0] == 'F')
    {
      aFullMode = Standard_True;
      k++;
    }
    else if (theArgVec[k][0] == 'r' || theArgVec[3][0] == 'R')
    {
      aFullMode = Standard_False;
      k++;
    }
    else
      fromtcl = Standard_True;
  }
  if (!fromtcl)
    fromtcl = theNbArgs > k;
  if (aFullMode)
    std::cout << "Full model for translation with additional info will be used \n" << std::flush;
  else
    std::cout << "Reduced model for translation without additional info will be used \n" << std::flush;

  sr.WS()->SetModeStat(aFullMode);

  if (modfic) readstat = sr.ReadFile(fnom.ToCString());
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

  XSAlgo::AlgoContainer()->PrepareForTransfer(); // update unit info
  sr.SetSystemLengthUnit(UnitsMethods::GetCasCadeLengthUnit());

  //   nom = "." -> fichier deja lu
  Standard_Integer i, num, nbs, modepri = 1;
  if (fromtcl) modepri = 4;
  while (modepri)
  {
    num = sr.NbRootsForTransfer();
    if (!fromtcl)
    {
      theDI << "NbRootsForTransfer=" << num << " :\n";
      for (i = 1; i <= num; i++)
      {
        theDI << "Root." << i << ", Ent. ";
        Standard_SStream aTmpStream;
        sr.Model()->Print(sr.RootForTransfer(i), aTmpStream);
        theDI << aTmpStream.str().c_str();
        theDI << " Type:" << sr.RootForTransfer(i)->DynamicType()->Name() << "\n";
      }

      std::cout << "Mode (0 End, 1 root n0 1, 2 one root/n0, 3 one entity/n0, 4 Selection) : " << std::flush;
      std::cin >> modepri;
    }

    if (modepri == 0) { theDI << "End Reading STEP\n"; return 0; }
    if (modepri <= 2)
    {
      num = 1;
      if (modepri == 2)
      {
        std::cout << "Root N0 : " << std::flush;  std::cin >> num;
      }
      aPSRoot.SetName("Translation");
      progress->Show(aPSRoot);

      if (!sr.TransferRoot(num, aPSRoot.Next(80)))
        theDI << "Transfer root n0 " << num << " : no result\n";
      else
      {
        nbs = sr.NbShapes();
        char shname[30];  Sprintf(shname, "%s_%d", rnom.ToCString(), nbs);
        theDI << "Transfer root n0 " << num << " OK  -> DRAW Shape: " << shname << "\n";
        theDI << "Now, " << nbs << " Shapes produced\n";
        TopoDS_Shape sh = sr.Shape(nbs);
        DBRep::Set(shname, sh);
      }
      if (aPSRoot.UserBreak())
        return 1;
    }
    else if (modepri == 3)
    {
      std::cout << "Entity : " << std::flush;  num = GiveEntityNumber(XSDRAWBase::Session(), "");
      if (!sr.TransferOne(num))
        theDI << "Transfer entity n0 " << num << " : no result\n";
      else
      {
        nbs = sr.NbShapes();
        char shname[30];  Sprintf(shname, "%s_%d", rnom.ToCString(), num);
        theDI << "Transfer entity n0 " << num << " OK  -> DRAW Shape: " << shname << "\n";
        theDI << "Now, " << nbs << " Shapes produced\n";
        TopoDS_Shape sh = sr.Shape(nbs);
        DBRep::Set(shname, sh);
      }
    }
    else if (modepri == 4)
    {
      //      char snm[100];  Standard_Integer answer = 1;
      Handle(TColStd_HSequenceOfTransient)  list;

      //  Selection, nommee ou via tcl. tcl : raccourcis admis
      //   * donne xst-transferrable-roots
      if (fromtcl)
      {
        modepri = 0;    // d ioffice une seule passe
        if (theArgVec[k][0] == '*' && theArgVec[k][1] == '\0')
        {
          theDI << "Transferrable Roots : ";
          list = XSDRAWBase::Session()->GiveList("xst-transferrable-roots");
          //list = new TColStd_HSequenceOfTransient;
          //for(Standard_Integer j=1; j<=num; j++)
          //  list->Append(sr.RootForTransfer(j));
        }
        else
        {
          theDI << "List given by " << theArgVec[k];
          if (theNbArgs > k + 1) theDI << " " << theArgVec[k + 1];
          theDI << " : ";
          list = XSDRAWBase::Session()->GiveList(theArgVec[k], (theNbArgs > (k + 1) ? theArgVec[k + 1] : 0));
        }
        if (list.IsNull()) { theDI << "No list defined. Give a selection name or * for all transferrable roots\n"; continue; }
      }
      else
      {
        std::cout << "Name of Selection :" << std::flush;
        list = XSDRAWBase::Session()->GiveList("");
        if (list.IsNull()) { theDI << "No list defined\n"; continue; }
      }

      Standard_Integer ill, nbl = list->Length();
      theDI << "Nb entities selected : " << nbl << "\n";
      if (nbl == 0) continue;

      aPSRoot.SetName("Translation");
      progress->Show(aPSRoot);

      Message_ProgressScope aPS(aPSRoot.Next(80), "Root", nbl);
      for (ill = 1; ill <= nbl && aPS.More(); ill++)
      {
        num = sr.Model()->Number(list->Value(ill));
        if (num == 0) continue;
        if (!sr.TransferOne(num, aPS.Next()))
          theDI << "Transfer entity n0 " << num << " : no result\n";
        else
        {
          nbs = sr.NbShapes();
          char shname[30];  Sprintf(shname, "%s_%d", rnom.ToCString(), nbs);
          theDI << "Transfer entity n0 " << num << " OK  -> DRAW Shape: " << shname << "\n";
          theDI << "Now, " << nbs << " Shapes produced\n";
          TopoDS_Shape sh = sr.Shape(nbs);
          DBRep::Set(shname, sh);
        }
      }
      if (aPSRoot.UserBreak())
        return 1;
    }
    else theDI << "Unknown mode n0 " << modepri << "\n";
  }
  return 0;
}

//=======================================================================
//function : testreadstep
//purpose  :
//=======================================================================
static Standard_Integer testreadstep(Draw_Interpretor& theDI,
                                     Standard_Integer theNbArgs,
                                     const char** theArgVec)
{
  if (theNbArgs < 3 || theNbArgs > 4)
  {
    theDI << "ERROR in " << theArgVec[0] << "Wrong Number of Arguments.\n";
    theDI << " Usage : " << theArgVec[0] << " file_name shape_name [-stream]\n";
    theDI << " Option -stream forces usage of API accepting stream\n";
    return 1;
  }

  Standard_Boolean useStream = (theNbArgs > 3 && !strcasecmp(theArgVec[3], "-stream"));

  STEPControl_Reader Reader;
  Standard_CString filename = theArgVec[1];
  IFSelect_ReturnStatus readstat;
  if (useStream)
  {
    std::ifstream aStream;
    OSD_OpenStream(aStream, filename, std::ios::in | std::ios::binary);
    TCollection_AsciiString aFolder, aFileNameShort;
    OSD_Path::FolderAndFileFromPath(filename, aFolder, aFileNameShort);
    readstat = Reader.ReadStream(aFileNameShort.ToCString(), aStream);
  }
  else
  {
    readstat = Reader.ReadFile(filename);
  }
  theDI << "Status from reading STEP file " << filename << " : ";
  switch (readstat)
  {
    case IFSelect_RetVoid: { theDI << "empty file\n"; return 1; }
    case IFSelect_RetDone: { theDI << "file read\n";    break; }
    case IFSelect_RetError: { theDI << "file not found\n";   return 1; }
    case IFSelect_RetFail: { theDI << "error during read\n";  return 1; }
    default: { theDI << "failure\n";   return 1; }
  }
  XSAlgo::AlgoContainer()->PrepareForTransfer(); // update unit info
  Reader.SetSystemLengthUnit(UnitsMethods::GetCasCadeLengthUnit());
  Reader.TransferRoots();
  TopoDS_Shape shape = Reader.OneShape();
  DBRep::Set(theArgVec[2], shape);
  theDI << "Count of shapes produced : " << Reader.NbShapes() << "\n";
  return 0;
}

//=======================================================================
//function : steptrans
//purpose  :
//=======================================================================
static Standard_Integer steptrans(Draw_Interpretor& theDI,
                                  Standard_Integer theNbArgs,
                                  const char** theArgVec)
{
  if (theNbArgs < 5)
  {
    theDI << "give shape-name new-shape + entity-n0 entity-n0: AXIS2\n";
    return 1;
  }
  Handle(XSControl_WorkSession) aWS = XSDRAWBase::Session();
  TopoDS_Shape shape = DBRep::Get(theArgVec[1]);
  if (shape.IsNull())
  {
    theDI << "Not a shape : " << theArgVec[1] << "\n";
    return 1;
  }
  Handle(StepGeom_Axis2Placement3d) ax1, ax2;
  Standard_Integer n1 = 0, n2 = 0;
  n1 = GiveEntityNumber(aWS, theArgVec[3]);
  if (theNbArgs > 4) n2 = GiveEntityNumber(aWS, theArgVec[4]);
  if (n1 > 0) ax1 = Handle(StepGeom_Axis2Placement3d)::DownCast
  (aWS->StartingEntity(n1));
  if (n2 > 0) ax2 = Handle(StepGeom_Axis2Placement3d)::DownCast
  (aWS->StartingEntity(n2));
  StepToTopoDS_MakeTransformed mktrans;
  if (mktrans.Compute(ax1, ax2))
  {
    TopLoc_Location loc(mktrans.Transformation());
    shape.Move(loc);
    //    mktrans.Transform (shape);
    DBRep::Set(theArgVec[2], shape);
    theDI << "Transformed Shape as " << theArgVec[2] << "\n";
  }
  else
    theDI << "No transformation computed\n";
  return 0;
}

//=======================================================================
//function : stepwrite
//purpose  : 
//=======================================================================
static Standard_Integer stepwrite(Draw_Interpretor& theDI,
                                  Standard_Integer theNbArgs,
                                  const char** theArgVec)
{
  Handle(XSControl_WorkSession) aWS = XSDRAWBase::Session();
  Handle(STEPControl_Controller) aCtl =
    Handle(STEPControl_Controller)::DownCast(aWS->NormAdaptor());
  if (aCtl.IsNull())
  {
    aWS->SelectNorm("STEP");
  }
  if (theNbArgs < 3)
  {
    theDI << "Error: Give mode[1-4] and Shape name + optional file. Mode possible\n";
    theDI << "f ou 1 : FacettedBRep        s ou 2 : ShellBasedSurfaceModel\n"
      << "m ou 3 : ManifoldSolidBrep   w ou 4 : GeometricCurveSet/WireFrame\n";
    return 1;
  }
  STEPControl_StepModelType mode;
  switch (theArgVec[1][0])
  {
    case 'a':
    case '0': mode = STEPControl_AsIs;                    break;
    case 'f':
    case '1': mode = STEPControl_FacetedBrep;             break;
    case 's':
    case '2': mode = STEPControl_ShellBasedSurfaceModel;  break;
    case 'm':
    case '3': mode = STEPControl_ManifoldSolidBrep;       break;
    case 'w':
    case '4': mode = STEPControl_GeometricCurveSet;       break;
    default:  theDI << "Error: 1st arg = mode, incorrect [give fsmw]\n"; return 1;
  }
  Handle(STEPControl_ActorWrite) ActWrite =
    Handle(STEPControl_ActorWrite)::DownCast(aWS->NormAdaptor()->ActorWrite());
  if (!ActWrite.IsNull())
    ActWrite->SetGroupMode(Interface_Static::IVal("write.step.assembly"));

  TopoDS_Shape shape = DBRep::Get(theArgVec[2]);
  STEPControl_Writer sw(aWS, Standard_False);
  Handle(Interface_InterfaceModel) stepmodel = sw.Model();
  Standard_Integer nbavant = (stepmodel.IsNull() ? 0 : stepmodel->NbEntities());

  Handle(Draw_ProgressIndicator) progress = new Draw_ProgressIndicator(theDI, 1);
  Message_ProgressScope aPSRoot(progress->Start(), "Translating", 100);
  progress->Show(aPSRoot);

  Standard_Integer stat = sw.Transfer(shape, mode, Standard_True, aPSRoot.Next(90));
  if (stat == IFSelect_RetDone)
  {
    theDI << "Translation: OK\n";
  }
  else
  {
    theDI << "Error: translation failed, status = " << stat << "\n";
  }
  if (aPSRoot.UserBreak())
    return 1;
  aPSRoot.SetName("Writing");
  progress->Show(aPSRoot);

  //   Que s est-il passe
  stepmodel = sw.Model();
  Standard_Integer nbapres = (stepmodel.IsNull() ? 0 : stepmodel->NbEntities());
  if (nbavant > 0) theDI << "Beware : Model not empty before transferring\n";
  if (nbapres <= nbavant) theDI << "Beware : No data produced by this transfer\n";
  if (nbapres == 0) { theDI << "No data to write\n"; return 0; }

  if (theNbArgs <= 3)
  {
    theDI << " Now, to write a file, command : writeall filename\n";
    return 0;
  }
  const char* nomfic = theArgVec[3];
  stat = sw.Write(nomfic);
  switch (stat)
  {
    case IFSelect_RetVoid: theDI << "Error: No file written\n"; return 1;
    case IFSelect_RetDone: theDI << "File " << nomfic << " written\n"; break;
    case IFSelect_RetStop: theDI << "Error on writing file: no space on disk or destination is write protected\n"; return 1;
    default: theDI << "Error: File " << nomfic << " written with fail messages\n"; return 1;
  }
  XSDRAWBase::CollectActiveWorkSessions(aWS, nomfic, XSDRAWBase::WorkSessionList());
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
  TCollection_AsciiString aFilePath;
  TopoDS_Shape aShape;
  bool toTestStream = false;
  for (Standard_Integer anArgIter = 1; anArgIter < theNbArgs; ++anArgIter)
  {
    TCollection_AsciiString anArgCase(theArgVec[anArgIter]);
    anArgCase.LowerCase();
    if (anArgCase == "-stream")
    {
      toTestStream = true;
    }
    else if (aFilePath.IsEmpty())
    {
      aFilePath = theArgVec[anArgIter];
    }
    else if (aShape.IsNull())
    {
      aShape = DBRep::Get(theArgVec[anArgIter]);
      if (aShape.IsNull())
      {
        theDI << "Syntax error: '" << theArgVec[anArgIter] << "' is not a shape\n";
        return 1;
      }
    }
    else
    {
      theDI << "Syntax error: unknown argument '" << theArgVec[anArgIter] << "'\n";
      return 1;
    }
  }
  if (aShape.IsNull())
  {
    theDI << "Syntax error: wrong number of arguments\n";
    return 1;
  }

  STEPControl_Writer aWriter;
  IFSelect_ReturnStatus aStat = aWriter.Transfer(aShape, STEPControl_AsIs);
  if (aStat != IFSelect_RetDone)
  {
    theDI << "Error on transferring shape\n";
    return 1;
  }

  if (toTestStream)
  {
    std::ofstream aStream;
    OSD_OpenStream(aStream, aFilePath, std::ios::out | std::ios::binary);
    aStat = aWriter.WriteStream(aStream);
    aStream.close();
    if (!aStream.good()
        && aStat == IFSelect_RetDone)
    {
      aStat = IFSelect_RetFail;
    }
  }
  else
  {
    aStat = aWriter.Write(aFilePath.ToCString());
  }
  if (aStat != IFSelect_RetDone)
  {
    theDI << "Error on writing file\n";
    return 1;
  }
  theDI << "File Is Written\n";
  return 0;
}

//=======================================================================
//function : countexpected
//purpose  :
//=======================================================================
static Standard_Integer countexpected(Draw_Interpretor& theDI,
                                      Standard_Integer /*theNbArgs*/,
                                      const char** /*theArgVec*/)
{
  Handle(IFSelect_WorkSession) WS = XSDRAWBase::Session();
  const Interface_Graph& graph = WS->Graph();

  Handle(TColStd_HSequenceOfTransient) roots = WS->GiveList("xst-transferrable-roots", "");
  STEPSelections_Counter cnt;

  for (Standard_Integer i = 1; i <= roots->Length(); i++)
  {
    cnt.Count(graph, roots->Value(i));
  }

  theDI << "Instances of Faces \t: " << cnt.NbInstancesOfFaces() << "\n";
  theDI << "Instances of Shells\t: " << cnt.NbInstancesOfShells() << "\n";
  theDI << "Instances of Solids\t: " << cnt.NbInstancesOfSolids() << "\n";
  theDI << "Instances of Wires in GS\t: " << cnt.NbInstancesOfWires() << "\n";
  theDI << "Instances of Edges in GS\t: " << cnt.NbInstancesOfEdges() << "\n";

  theDI << "Source Faces \t: " << cnt.NbSourceFaces() << "\n";
  theDI << "Source Shells\t: " << cnt.NbSourceShells() << "\n";
  theDI << "Source Solids\t: " << cnt.NbSourceSolids() << "\n";
  theDI << "Source Wires in GS\t: " << cnt.NbSourceWires() << "\n";
  theDI << "Source Edges in GS\t: " << cnt.NbSourceEdges() << "\n";

  return 1;
}

//=======================================================================
//function : dumpassembly
//purpose  :
//=======================================================================
static Standard_Integer dumpassembly(Draw_Interpretor& /*theDI*/,
                                     Standard_Integer /*theNbArgs*/,
                                     const char** /*theArgVec*/)
{
  Handle(IFSelect_WorkSession) WS = XSDRAWBase::Session();
  const Interface_Graph& graph = WS->Graph();

  STEPSelections_AssemblyExplorer exp(graph);
  exp.Dump(std::cout);
  return 0;
}

//=======================================================================
//function : stepfileunits
//purpose  :
//=======================================================================
static Standard_Integer stepfileunits(Draw_Interpretor& theDI,
                                      Standard_Integer theNbArgs,
                                      const char** theArgVec)
{
  if (theNbArgs < 2)
  {
    theDI << "Error: Invalid number of parameters. Should be: getfileunits name_file\n";
    return 1;
  }
  STEPControl_Reader aStepReader;
  IFSelect_ReturnStatus readstat = IFSelect_RetVoid;
  readstat = aStepReader.ReadFile(theArgVec[1]);
  if (readstat != IFSelect_RetDone)
  {
    theDI << "No model loaded\n";
    return 1;
  }

  TColStd_SequenceOfAsciiString anUnitLengthNames;
  TColStd_SequenceOfAsciiString anUnitAngleNames;
  TColStd_SequenceOfAsciiString anUnitSolidAngleNames;
  aStepReader.FileUnits(anUnitLengthNames, anUnitAngleNames, anUnitSolidAngleNames);

  Standard_Integer i = 1;
  theDI << "=====================================================\n";
  theDI << "LENGTH Unit\n";
  for (; i <= anUnitLengthNames.Length(); i++)
    theDI << anUnitLengthNames(i).ToCString() << "\n";

  theDI << "=====================================================\n";
  theDI << "Angle Unit\n";
  for (i = 1; i <= anUnitAngleNames.Length(); i++)
    theDI << anUnitAngleNames(i).ToCString() << "\n";

  theDI << "=====================================================\n";
  theDI << "Solid Angle Unit\n";
  for (i = 1; i <= anUnitSolidAngleNames.Length(); i++)
    theDI << anUnitSolidAngleNames(i).ToCString() << "\n";

  return 0;
}

//=======================================================================
//function : ReadStep
//purpose  : Read STEP file to DECAF document 
//=======================================================================
static Standard_Integer ReadStep(Draw_Interpretor& theDI,
                                 Standard_Integer theNbArgs,
                                 const char** theArgVec)
{
  Standard_CString aDocName = NULL;
  TCollection_AsciiString aFilePath, aModeStr;
  bool aToTestStream = false;
  for (Standard_Integer anArgIter = 1; anArgIter < theNbArgs; ++anArgIter)
  {
    TCollection_AsciiString anArgCase(theArgVec[anArgIter]);
    anArgCase.LowerCase();
    if (anArgCase == "-stream")
    {
      aToTestStream = true;
    }
    else if (aDocName == NULL)
    {
      aDocName = theArgVec[anArgIter];
    }
    else if (aFilePath.IsEmpty())
    {
      aFilePath = theArgVec[anArgIter];
    }
    else if (aModeStr.IsEmpty())
    {
      aModeStr = theArgVec[anArgIter];
    }
    else
    {
      theDI << "Syntax error at '" << theArgVec[anArgIter] << "'\n";
      return 1;
    }
  }
  Handle(STEPCAFControl_ConfigurationNode) aNode =
    new STEPCAFControl_ConfigurationNode();
  if (!aModeStr.IsEmpty())
  {
    Standard_Boolean aMode = Standard_True;
    for (Standard_Integer i = 1; aModeStr.Value(i); ++i)
    {
      switch (aModeStr.Value(i))
      {
        case '-': aMode = Standard_False; break;
        case '+': aMode = Standard_True; break;
        case 'c': aNode->InternalParameters.WriteColor = aMode; break;
        case 'n': aNode->InternalParameters.WriteName = aMode; break;
        case 'l': aNode->InternalParameters.WriteLayer = aMode; break;
        case 'v': aNode->InternalParameters.WriteProps = aMode; break;
        default:
        {
          theDI << "Syntax error at '" << aModeStr << "'\n";
          return 1;
        }
      }
    }
  }
  Handle(TDocStd_Document) aDoc;
  if (!DDocStd::GetDocument(aDocName, aDoc, Standard_False))
  {
    Handle(TDocStd_Application) anApp = DDocStd::GetApplication();
    anApp->NewDocument("BinXCAF", aDoc);
    TDataStd_Name::Set(aDoc->GetData()->Root(), aDocName);
    Handle(DDocStd_DrawDocument) aDrawDoc = new DDocStd_DrawDocument(aDoc);
    Draw::Set(aDocName, aDrawDoc);
  }
  aNode->GlobalParameters.LengthUnit = XSDRAWBase::GetLengthUnit(aDoc);
  Handle(Draw_ProgressIndicator) aProgress = new Draw_ProgressIndicator(theDI);
  Handle(STEPCAFControl_Provider) aProvider =
    new STEPCAFControl_Provider(aNode);
  aProvider->SetToUpdateStaticParameters(false);
  Standard_Boolean aReadStat = Standard_False;
  Handle(XSControl_WorkSession) aWS = XSDRAWBase::Session();
  if (aToTestStream)
  {
    std::ifstream aStream;
    OSD_OpenStream(aStream, aFilePath.ToCString(), std::ios::in | std::ios::binary);
    aReadStat =
      aProvider->Read(aStream, aDoc, aFilePath, aWS, aProgress->Start());
  }
  else
  {
    aReadStat =
      aProvider->Read(aFilePath, aDoc, aWS, aProgress->Start());
  }
  if (!aReadStat)
  {
    theDI << "Cannot read any relevant data from the STEP file\n";
    return 1;
  }
  Handle(DDocStd_DrawDocument) aDrawDoc = new DDocStd_DrawDocument(aDoc);
  Draw::Set(aDocName, aDrawDoc);
  XSDRAWBase::CollectActiveWorkSessions(aWS, aFilePath, XSDRAWBase::WorkSessionList());
  Message::SendInfo() << "Document saved with name " << aDocName;
  return 0;
}

//=======================================================================
//function : WriteStep
//purpose  : Write DECAF document to STEP
//=======================================================================
static Standard_Integer WriteStep(Draw_Interpretor& theDI,
                                  Standard_Integer theNbArgs,
                                  const char** theArgVec)
{
  Handle(TDocStd_Document) aDoc;
  TCollection_AsciiString aDocName, aFilePath;
  Handle(STEPCAFControl_ConfigurationNode) aNode =
    new STEPCAFControl_ConfigurationNode();
  bool aHasModeArg = false, aToTestStream = false;
  TDF_Label aLabel;
  for (Standard_Integer anArgIter = 1; anArgIter < theNbArgs; ++anArgIter)
  {
    TCollection_AsciiString anArgCase(theArgVec[anArgIter]);
    anArgCase.LowerCase();
    if (anArgCase == "-stream")
    {
      aToTestStream = true;
    }
    else if (aDocName.IsEmpty())
    {
      Standard_CString aDocNameStr = theArgVec[anArgIter];
      DDocStd::GetDocument(aDocNameStr, aDoc);
      if (aDoc.IsNull())
      {
        theDI << "Syntax error: '" << theArgVec[anArgIter] << "' is not a document\n";
        return 1;
      }
      aDocName = aDocNameStr;
    }
    else if (aFilePath.IsEmpty())
    {
      aFilePath = theArgVec[anArgIter];
    }
    else if (!aHasModeArg)
    {
      aHasModeArg = true;
      Standard_Boolean aIsWriteType = Standard_True;
      switch (anArgCase.Value(1))
      {
        case 'a':
        case '0': aNode->InternalParameters.WriteModelType = STEPControl_AsIs; break;
        case 'f':
        case '1': aNode->InternalParameters.WriteModelType = STEPControl_FacetedBrep; break;
        case 's':
        case '2': aNode->InternalParameters.WriteModelType = STEPControl_ShellBasedSurfaceModel; break;
        case 'm':
        case '3': aNode->InternalParameters.WriteModelType = STEPControl_ManifoldSolidBrep; break;
        case 'w':
        case '4': aNode->InternalParameters.WriteModelType = STEPControl_GeometricCurveSet; break;
        default:
        {
          aIsWriteType = Standard_False;
        }
      }
      Standard_Boolean aWrMode = Standard_True;
      Standard_Boolean aIsAttrType = Standard_True;
      for (Standard_Integer i = 1; i <= anArgCase.Length(); ++i)
      {
        switch (anArgCase.Value(i))
        {
          case '-': aWrMode = Standard_False; break;
          case '+': aWrMode = Standard_True; break;
          case 'c': aNode->InternalParameters.WriteColor = aWrMode; break;
          case 'n': aNode->InternalParameters.WriteName = aWrMode; break;
          case 'l': aNode->InternalParameters.WriteLayer = aWrMode; break;
          case 'v': aNode->InternalParameters.WriteProps = aWrMode; break;
          default:
          {
            aIsAttrType = Standard_False;
          }
        }
      }
      if (!aIsAttrType && !aIsWriteType)
      {
        theDI << "Syntax error: mode '" << anArgCase << "' is incorrect mode\n";
        return 1;
      }
    }
    else if (aNode->InternalParameters.WriteMultiPrefix.IsEmpty()
             && anArgCase.Search(":") == -1)
    {
      aNode->InternalParameters.WriteMultiPrefix = theArgVec[anArgIter];
    }
    else if (aLabel.IsNull())
    {
      aNode->InternalParameters.WriteLabels.Append(theArgVec[anArgIter]);
    }
    else
    {
      theDI << "Syntax error: unknown argument '" << theArgVec[anArgIter] << "'\n";
      return 1;
    }
  }
  if (aFilePath.IsEmpty())
  {
    theDI << "Syntax error: wrong number of arguments\n";
    return 1;
  }
  aNode->GlobalParameters.LengthUnit = XSDRAWBase::GetLengthUnit(aDoc);
  Handle(Draw_ProgressIndicator) aProgress = new Draw_ProgressIndicator(theDI);
  Handle(STEPCAFControl_Provider) aProvider =
    new STEPCAFControl_Provider(aNode);
  aProvider->SetToUpdateStaticParameters(false);
  Standard_Boolean aReadStat = Standard_False;
  Handle(XSControl_WorkSession) aWS = XSDRAWBase::Session();
  if (aToTestStream)
  {
    std::ofstream aStream;
    OSD_OpenStream(aStream, aFilePath, std::ios::out | std::ios::binary);
    aReadStat =
      aProvider->Write(aStream, aDoc, aWS, aProgress->Start());
  }
  else
  {
    aReadStat =
      aProvider->Write(aFilePath, aDoc, aWS, aProgress->Start());
  }
  if (!aReadStat)
  {
    theDI << "Cannot write any relevant data to the STEP file\n";
    return 1;
  }
  XSDRAWBase::CollectActiveWorkSessions(aWS, aFilePath, XSDRAWBase::WorkSessionList());
  return 0;
}

//=======================================================================
//function : Factory
//purpose  :
//=======================================================================
void XSDRAWSTEP::Factory(Draw_Interpretor& theDI)
{
  static Standard_Boolean initactor = Standard_False;
  if (initactor)
  {
    return;
  }
  initactor = Standard_True;
  const char* g = "DE: STEP";  // Step transfer file commands
  theDI.Add("stepwrite", "stepwrite mode[0-4 afsmw] shape", __FILE__, stepwrite, g);
  theDI.Add("testwritestep", "testwritestep filename.stp shape [-stream]",
            __FILE__, testwrite, g);
  theDI.Add("stepread", "stepread  [file] [f or r (type of model full or reduced)]", __FILE__, stepread, g);
  theDI.Add("testreadstep", "testreadstep file shape [-stream]", __FILE__, testreadstep, g);
  theDI.Add("steptrans", "steptrans shape stepax1 stepax2", __FILE__, steptrans, g);
  theDI.Add("countexpected", "TEST", __FILE__, countexpected, g);
  theDI.Add("dumpassembly", "TEST", __FILE__, dumpassembly, g);
  theDI.Add("stepfileunits", "stepfileunits name_file", __FILE__, stepfileunits, g);
  theDI.Add("ReadStep",
            "Doc filename [mode] [-stream]"
            "\n\t\t: Read STEP file to a document."
            "\n\t\t:  -stream read using istream reading interface (testing)",
            __FILE__, ReadStep, g);
  theDI.Add("WriteStep",
            "Doc filename [mode=a [multifile_prefix] [label]] [-stream]"
            "\n\t\t: Write DECAF document to STEP file"
            "\n\t\t:   mode can be: a or 0 : AsIs (default)"
            "\n\t\t:                f or 1 : FacettedBRep        s or 2 : ShellBasedSurfaceModel"
            "\n\t\t:                m or 3 : ManifoldSolidBrep   w or 4 : GeometricCurveSet/WireFrame"
            "\n\t\t:   multifile_prefix: triggers writing assembly components as separate files,"
            "\n\t\t:                     and defines common prefix for their names"
            "\n\t\t:   label  tag of the sub-assembly label to save only that sub-assembly"
            "\n\t\t:  -stream read using ostream writing interface (testing)",
            __FILE__, WriteStep, g);
}

// Declare entry point PLUGINFACTORY
DPLUGIN(XSDRAWSTEP)
