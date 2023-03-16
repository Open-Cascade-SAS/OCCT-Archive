// Created on: 2003-08-15
// Created by: Sergey ZARITCHNY
// Copyright (c) 2003-2014 OPEN CASCADE SAS
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

#include <XDEDRAW_Common.hxx>

#include <DBRep.hxx>
#include <DDF.hxx>
#include <DDocStd.hxx>
#include <DDocStd_DrawDocument.hxx>
#include <DE_ConfigurationContext.hxx>
#include <DE_Wrapper.hxx>
#include <Draw.hxx>
#include <Draw_Interpretor.hxx>
#include <Draw_ProgressIndicator.hxx>
#include <IFSelect_SessionPilot.hxx>
#include <IGESCAFControl_Provider.hxx>
#include <Interface_Macros.hxx>
#include <Interface_Static.hxx>
#include <Message.hxx>
#include <OSD_OpenFile.hxx>
#include <OSD_Path.hxx>
#include <STEPCAFControl_ConfigurationNode.hxx>
#include <STEPCAFControl_ExternFile.hxx>
#include <STEPCAFControl_Provider.hxx>
#include <TDF_Data.hxx>
#include <TDF_Tool.hxx>
#include <TDocStd_Application.hxx>
#include <TDocStd_Document.hxx>
#include <TopoDS_Shape.hxx>
#include <UnitsAPI.hxx>
#include <UnitsMethods.hxx>
#include <Vrml_ConfigurationNode.hxx>
#include <Vrml_Provider.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_Editor.hxx>
#include <XCAFDoc_ShapeTool.hxx>
#include <XSAlgo.hxx>
#include <XSAlgo_AlgoContainer.hxx>
#include <XSControl_WorkSession.hxx>
#include <XSDRAW.hxx>
#include <XSDRAW_Vars.hxx>

namespace
{
  //============================================================
  // Support for several models in DRAW
  //============================================================
  static NCollection_DataMap<TCollection_AsciiString, Handle(Standard_Transient)> thedictws;

  //=======================================================================
  //function : GetLengthUnit
  //purpose  :
  //=======================================================================
  Standard_Real getLengthUnit(const Handle(TDocStd_Document)& theDoc = nullptr)
  {
    if (!theDoc.IsNull())
    {
      Standard_Real aUnit = 1.;
      if (XCAFDoc_DocumentTool::GetLengthUnit(theDoc, aUnit,
          UnitsMethods_LengthUnit_Millimeter))
      {
        return aUnit;
      }
    }
    XSAlgo::AlgoContainer()->PrepareForTransfer();
    return UnitsMethods::GetCasCadeLengthUnit();
  }

  //=======================================================================
  //function : parseCoordinateSystem
  //purpose  : Parse RWMesh_CoordinateSystem enumeration.
  //=======================================================================
  static bool parseCoordinateSystem(const char* theArg,
                                    RWMesh_CoordinateSystem& theSystem)
  {
    TCollection_AsciiString aCSStr(theArg);
    aCSStr.LowerCase();
    if (aCSStr == "zup")
    {
      theSystem = RWMesh_CoordinateSystem_Zup;
    }
    else if (aCSStr == "yup")
    {
      theSystem = RWMesh_CoordinateSystem_Yup;
    }
    else
    {
      return Standard_False;
    }
    return Standard_True;
  }

  //=======================================================================
  //function : SetCurrentWS
  //purpose  :
  //=======================================================================
  static Standard_Boolean ClearDicWS()
  {
    thedictws.Clear();
    return Standard_True;
  }

  //=======================================================================
  //function : SetCurrentWS
  //purpose  :
  //=======================================================================
  static void AddWS(TCollection_AsciiString theFileName,
                    const Handle(XSControl_WorkSession)& theWS)
  {
    theWS->SetVars(new XSDRAW_Vars); // support of DRAW variables
    thedictws.Bind(theFileName, theWS);
  }

  //=======================================================================
  //function : SetCurrentWS
  //purpose  :
  //=======================================================================
  static Standard_Boolean FillDicWS(NCollection_DataMap<TCollection_AsciiString, Handle(STEPCAFControl_ExternFile)>& theDicFile)
  {
    ClearDicWS();
    if (theDicFile.IsEmpty())
    {
      return Standard_False;
    }
    Handle(STEPCAFControl_ExternFile) anEF;
    NCollection_DataMap<TCollection_AsciiString, Handle(STEPCAFControl_ExternFile)>::Iterator aDicEFIt(theDicFile);
    for (; aDicEFIt.More(); aDicEFIt.Next())
    {
      TCollection_AsciiString aFileName = aDicEFIt.Key();
      anEF = aDicEFIt.Value();
      AddWS(aFileName, anEF->GetWS());
    }
    return Standard_True;
  }

  //=======================================================================
  //function : SetCurrentWS
  //purpose  :
  //=======================================================================
  static Standard_Boolean SetCurrentWS(TCollection_AsciiString theFileName)
  {
    if (!thedictws.IsBound(theFileName))
      return Standard_False;
    Handle(XSControl_WorkSession) CurrentWS =
      Handle(XSControl_WorkSession)::DownCast(thedictws.ChangeFind(theFileName));
    XSDRAW::Pilot()->SetSession(CurrentWS);
    return Standard_True;
  }
}

//=======================================================================
//function : SetCurWS
//purpose  : Set current file if many files are read
//=======================================================================

static Standard_Integer SetCurWS(Draw_Interpretor& di, Standard_Integer argc, const char** argv)
{
  if (argc < 2)
  {
    di << "Use: " << argv[0] << " filename \n";
    return 1;
  }
  TCollection_AsciiString filename(argv[1]);
  SetCurrentWS(filename);
  return 0;
}


//=======================================================================
//function : GetDicWSList
//purpose  : List all files recorded after translation
//=======================================================================

static Standard_Integer GetDicWSList(Draw_Interpretor& di, Standard_Integer /*argc*/, const char** /*argv*/)
{
  NCollection_DataMap<TCollection_AsciiString, Handle(Standard_Transient)> DictWS = thedictws;
  if (DictWS.IsEmpty()) return 1;
  NCollection_DataMap<TCollection_AsciiString, Handle(Standard_Transient)>::Iterator DicIt(DictWS);
  di << " The list of last translated files:\n";
  Standard_Integer num = 0;
  for (; DicIt.More(); DicIt.Next(), num++)
  {
    TCollection_AsciiString strng(DicIt.Key());
    if (num) di << "\n";
    di << "\"" << strng.ToCString() << "\"";
  }
  return 0;
}

//=======================================================================
//function : GetCurWS
//purpose  : Return name of file which is current
//=======================================================================

static Standard_Integer GetCurWS(Draw_Interpretor& di, Standard_Integer /*argc*/, const char** /*argv*/)
{
  Handle(XSControl_WorkSession) WS = XSDRAW::Session();
  di << "\"" << WS->LoadedFile() << "\"";
  return 0;
}

//=======================================================================
//function : FromShape
//purpose  : Apply fromshape command to all the loaded WSs
//=======================================================================

static Standard_Integer FromShape(Draw_Interpretor& di, Standard_Integer argc, const char** argv)
{
  if (argc < 2)
  {
    di << argv[0] << " shape: search for shape origin among all last tranalated files\n";
    return 0;
  }

  char command[256];
  Sprintf(command, "fromshape %.200s -1", argv[1]);
  NCollection_DataMap<TCollection_AsciiString, Handle(Standard_Transient)> DictWS = thedictws;
  if (DictWS.IsEmpty()) return di.Eval(command);

  Handle(XSControl_WorkSession) WS = XSDRAW::Session();

  NCollection_DataMap<TCollection_AsciiString, Handle(Standard_Transient)>::Iterator DicIt(DictWS);
  //  di << "Searching for shape among all the loaded files:\n";
  Standard_Integer num = 0;
  for (; DicIt.More(); DicIt.Next(), num++)
  {
    Handle(XSControl_WorkSession) CurrentWS =
      Handle(XSControl_WorkSession)::DownCast(DicIt.Value());
    XSDRAW::Pilot()->SetSession(CurrentWS);
    di.Eval(command);
  }

  XSDRAW::Pilot()->SetSession(WS);
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
  aNode->GlobalParameters.LengthUnit = getLengthUnit(aDoc);
  Handle(Draw_ProgressIndicator) aProgress = new Draw_ProgressIndicator(theDI);
  Handle(IGESCAFControl_Provider) aProvider =
    new IGESCAFControl_Provider(aNode);
  aProvider->SetToUpdateStaticParameters(false);
  Handle(XSControl_WorkSession) aWS = XSDRAW::Session();
  if (!aProvider->Read(theArgVec[2], aDoc, aWS, aProgress->Start()))
  {
    theDI << "Error: Can't read IGES file\n";
    return 1;
  }
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
  aNode->GlobalParameters.LengthUnit = getLengthUnit(aDoc);
  Handle(Draw_ProgressIndicator) aProgress = new Draw_ProgressIndicator(theDI);
  Handle(IGESCAFControl_Provider) aProvider =
    new IGESCAFControl_Provider(aNode);
  aProvider->SetToUpdateStaticParameters(false);
  Handle(XSControl_WorkSession) aWS = XSDRAW::Session();
  const TCollection_AsciiString aPath = theArgVec[2];
  if (!aProvider->Write(aPath, aDoc, aWS, aProgress->Start()))
  {
    theDI << "Error: Can't write IGES file\n";
    return 1;
  }
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
  aNode->GlobalParameters.LengthUnit = getLengthUnit(aDoc);
  Handle(Draw_ProgressIndicator) aProgress = new Draw_ProgressIndicator(theDI);
  Handle(STEPCAFControl_Provider) aProvider =
    new STEPCAFControl_Provider(aNode);
  aProvider->SetToUpdateStaticParameters(false);
  Standard_Boolean aReadStat = Standard_False;
  Handle(XSControl_WorkSession) aWS = XSDRAW::Session();
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
  Message::SendInfo() << "Document saved with name " << aDocName;

  NCollection_DataMap<TCollection_AsciiString, Handle(STEPCAFControl_ExternFile)> aDicFile = aProvider->GetExternalFiles();
  FillDicWS(aDicFile);
  AddWS(aFilePath, XSDRAW::Session());
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
  aNode->GlobalParameters.LengthUnit = getLengthUnit(aDoc);
  Handle(Draw_ProgressIndicator) aProgress = new Draw_ProgressIndicator(theDI);
  Handle(STEPCAFControl_Provider) aProvider =
    new STEPCAFControl_Provider(aNode);
  aProvider->SetToUpdateStaticParameters(false);
  Standard_Boolean aReadStat = Standard_False;
  Handle(XSControl_WorkSession) aWS = XSDRAW::Session();
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
  NCollection_DataMap<TCollection_AsciiString, Handle(STEPCAFControl_ExternFile)> aDicFile = aProvider->GetExternalFiles();
  FillDicWS(aDicFile);
  AddWS(aFilePath, XSDRAW::Session());
  return 0;
}

//=======================================================================
//function : Expand
//purpose  :
//=======================================================================
static Standard_Integer Expand(Draw_Interpretor& di, Standard_Integer argc, const char** argv)
{
  if (argc < 3)
  {
    di << "Use: " << argv[0] << " Doc recurs(0/1) or Doc recurs(0/1) label1 label2 ... or Doc recurs(0/1 shape1 shape2 ...\n";
    return 1;
  }
  Handle(TDocStd_Document) Doc;
  DDocStd::GetDocument(argv[1], Doc);
  if (Doc.IsNull())
  {
    di << argv[1] << " is not a document\n"; return 1;
  }

  Handle(XCAFDoc_ShapeTool) aShapeTool = XCAFDoc_DocumentTool::ShapeTool(Doc->Main());
  Standard_Boolean recurs = Standard_False;
  if (atoi(argv[2]) != 0)
    recurs = Standard_True;

  if (argc == 3)
  {
    if (!XCAFDoc_Editor::Expand(Doc->Main(), recurs))
    {
      di << "No suitable labels to expand\n";
      return 1;
    }
  }
  else
  {
    for (Standard_Integer i = 3; i < argc; i++)
    {
      TDF_Label aLabel;
      TDF_Tool::Label(Doc->GetData(), argv[i], aLabel);
      if (aLabel.IsNull())
      {
        TopoDS_Shape aShape;
        aShape = DBRep::Get(argv[i]);
        aLabel = aShapeTool->FindShape(aShape);
      }

      if (!aLabel.IsNull())
      {
        if (!XCAFDoc_Editor::Expand(Doc->Main(), aLabel, recurs))
        {
          di << "The shape is assembly or not compound\n";
          return 1;
        }
      }
      else
      {
        di << argv[i] << " is not a shape\n"; return 1;
      }
    }
  }
  return 0;
}

//=======================================================================
//function : Extract
//purpose  :
//=======================================================================
static Standard_Integer Extract(Draw_Interpretor& di,
                                Standard_Integer argc,
                                const char** argv)
{
  if (argc < 4)
  {
    di << "Use: " << argv[0] << "dstDoc [dstAssmblSh] srcDoc srcLabel1 srcLabel2 ...\n";
    return 1;
  }

  Handle(TDocStd_Document) aSrcDoc, aDstDoc;
  DDocStd::GetDocument(argv[1], aDstDoc);
  if (aDstDoc.IsNull())
  {
    di << "Error " << argv[1] << " is not a document\n";
    return 1;
  }
  TDF_Label aDstLabel;
  Standard_Integer anArgInd = 3;
  TDF_Tool::Label(aDstDoc->GetData(), argv[2], aDstLabel);
  Handle(XCAFDoc_ShapeTool) aDstShapeTool = XCAFDoc_DocumentTool::ShapeTool(aDstDoc->Main());
  if (aDstLabel.IsNull())
  {
    aDstLabel = aDstShapeTool->Label();
    anArgInd = 2; // to get Src Doc
  }
  DDocStd::GetDocument(argv[anArgInd++], aSrcDoc);
  if (aSrcDoc.IsNull())
  {
    di << "Error " << argv[anArgInd] << " is not a document\n";
    return 1;
  }

  TDF_LabelSequence aSrcShapes;
  for (; anArgInd < argc; anArgInd++)
  {
    TDF_Label aSrcLabel;
    TDF_Tool::Label(aSrcDoc->GetData(), argv[anArgInd], aSrcLabel);
    if (aSrcLabel.IsNull())
    {
      di << "[" << argv[anArgInd] << "] is not valid Src label\n";
      return 1;
    }
    aSrcShapes.Append(aSrcLabel);
  }
  if (aSrcShapes.IsEmpty())
  {
    di << "Error: No Shapes to extract\n";
    return 1;
  }

  if (!XCAFDoc_Editor::Extract(aSrcShapes, aDstLabel))
  {
    di << "Error: Cannot extract labels\n";
    return 1;
  }
  return 0;
}

//=======================================================================
//function : ReadVrml
//purpose  :
//=======================================================================
static Standard_Integer ReadVrml(Draw_Interpretor& theDI,
                                 Standard_Integer  theNbArgs,
                                 const char** theArgVec)
{
  if (theNbArgs < 3)
  {
    theDI.PrintHelp(theArgVec[0]);
    return 1;
  }
  Handle(Vrml_ConfigurationNode) aNode =
    new Vrml_ConfigurationNode();
  Handle(TDocStd_Document) aDoc;
  Standard_Boolean toUseExistingDoc = Standard_False;
  Standard_CString aDocName = NULL;
  TCollection_AsciiString aFilePath;
  aNode->GlobalParameters.LengthUnit = getLengthUnit(aDoc);
  for (Standard_Integer anArgIt = 1; anArgIt < theNbArgs; anArgIt++)
  {
    TCollection_AsciiString anArg(theArgVec[anArgIt]);
    anArg.LowerCase();
    if (anArgIt + 1 < theNbArgs && anArg == "-fileunit")
    {
      const TCollection_AsciiString aUnitStr(theArgVec[++anArgIt]);
      aNode->InternalParameters.ReadFileUnit = UnitsAPI::AnyToSI(1.0, aUnitStr.ToCString());
      if (aNode->InternalParameters.ReadFileUnit <= 0.0)
      {
        theDI << "Error: wrong length unit '" << aUnitStr << "'\n";
        return 1;
      }
    }
    else if (anArgIt + 1 < theNbArgs && anArg == "-filecoordsys")
    {
      if (!parseCoordinateSystem(theArgVec[++anArgIt], aNode->InternalParameters.ReadFileCoordinateSys))
      {
        theDI << "Error: unknown coordinate system '" << theArgVec[anArgIt] << "'\n";
        return 1;
      }
    }
    else if (anArgIt + 1 < theNbArgs && anArg == "-systemcoordsys")
    {
      if (!parseCoordinateSystem(theArgVec[++anArgIt], aNode->InternalParameters.ReadSystemCoordinateSys))
      {
        theDI << "Error: unknown coordinate system '" << theArgVec[anArgIt] << "'\n";
        return 1;
      }
    }
    else if (anArg == "-fillincomplete")
    {
      aNode->InternalParameters.ReadFillIncomplete = true;
      if (anArgIt + 1 < theNbArgs &&
          Draw::ParseOnOff(theArgVec[anArgIt + 1], aNode->InternalParameters.ReadFillIncomplete))
      {
        ++anArgIt;
      }
    }
    else if (anArg == "-nocreatedoc")
    {
      toUseExistingDoc = true;
    }
    else if (aDocName == nullptr)
    {
      aDocName = theArgVec[anArgIt];
      DDocStd::GetDocument(aDocName, aDoc, Standard_False);
    }
    else if (aFilePath.IsEmpty())
    {
      aFilePath = theArgVec[anArgIt];
    }
    else
    {
      theDI << "Syntax error at '" << theArgVec[anArgIt] << "'\n";
      return 1;
    }
  }

  if (aFilePath.IsEmpty() || aDocName == nullptr)
  {
    theDI << "Syntax error: wrong number of arguments\n";
    return 1;
  }

  if (aDoc.IsNull())
  {
    if (toUseExistingDoc)
    {
      theDI << "Error: document with name " << aDocName << " does not exist\n";
      return 1;
    }
    Handle(TDocStd_Application) anApp = DDocStd::GetApplication();
    anApp->NewDocument("BinXCAF", aDoc);
  }
  else if (!toUseExistingDoc)
  {
    theDI << "Error: document with name " << aDocName << " already exists\n";
    return 1;
  }
  Handle(Vrml_Provider) aProvider =
    new Vrml_Provider(aNode);
  Handle(Draw_ProgressIndicator) aProgress = new Draw_ProgressIndicator(theDI, 1);
  Handle(XSControl_WorkSession) aWS = XSDRAW::Session();
  if (!aProvider->Read(aFilePath, aDoc, aWS, aProgress->Start()))
  {
    theDI << "Error: file reading failed '" << aFilePath << "'\n";
    return 1;
  }
  TDataStd_Name::Set(aDoc->GetData()->Root(), aDocName);
  Handle(DDocStd_DrawDocument) aDD = new DDocStd_DrawDocument(aDoc);
  Draw::Set(aDocName, aDD);
  return 0;
}

//=======================================================================
//function : WriteVrml
//purpose  : Write DECAF document to Vrml
//=======================================================================
static Standard_Integer WriteVrml(Draw_Interpretor& theDI,
                                  Standard_Integer theNbArgs,
                                  const char** theArgVec)
{
  if (theNbArgs < 3)
  {
    theDI << "Use: " << theArgVec[0] << " Doc filename: write document to Vrml file\n";
    return 0;
  }

  Handle(TDocStd_Document) aDoc;
  DDocStd::GetDocument(theArgVec[1], aDoc);
  if (aDoc.IsNull())
  {
    theDI << theArgVec[1] << " is not a document\n";
    return 1;
  }

  if (theNbArgs < 3 || theNbArgs > 5)
  {
    theDI << "wrong number of parameters\n";
    return 0;
  }
  Handle(Vrml_ConfigurationNode) aNode =
    new Vrml_ConfigurationNode();
  aNode->GlobalParameters.LengthUnit = getLengthUnit(aDoc) * 0.001;
  Handle(Vrml_Provider) aProvider =
    new Vrml_Provider(aNode);

  Handle(Draw_ProgressIndicator) aProgress = new Draw_ProgressIndicator(theDI, 1);
  Handle(XSControl_WorkSession) aWS = XSDRAW::Session();
  if (!aProvider->Write(theArgVec[2], aDoc, aWS, aProgress->Start()))
  {
    theDI << "Error: file writing failed '" << theArgVec[2] << "'\n";
    return 1;
  }
  return 0;
}

//=======================================================================
//function : DumpConfiguration
//purpose  :
//=======================================================================
static Standard_Integer DumpConfiguration(Draw_Interpretor& theDI,
                                          Standard_Integer theNbArgs,
                                          const char** theArgVec)
{
  Handle(DE_Wrapper) aConf = DE_Wrapper::GlobalWrapper();
  TCollection_AsciiString aPath;
  Standard_Boolean aIsRecursive = Standard_True;
  Standard_Boolean isHandleFormat = Standard_False;
  Standard_Boolean isHandleVendors = Standard_False;
  TColStd_ListOfAsciiString aFormats;
  TColStd_ListOfAsciiString aVendors;
  for (Standard_Integer anArgIter = 1; anArgIter < theNbArgs; ++anArgIter)
  {
    TCollection_AsciiString anArg(theArgVec[anArgIter]);
    anArg.LowerCase();
    if ((anArg == "-path") &&
        (anArgIter + 1 < theNbArgs))
    {
      ++anArgIter;
      aPath = theArgVec[anArgIter];
    }
    else if ((anArg == "-recursive") &&
             (anArgIter + 1 < theNbArgs) &&
             Draw::ParseOnOff(theArgVec[anArgIter + 1], aIsRecursive))
    {
      ++anArgIter;
    }
    else if (anArg == "-format")
    {
      isHandleFormat = Standard_True;
      isHandleVendors = Standard_False;
    }
    else if (anArg == "-vendor")
    {
      isHandleFormat = Standard_False;
      isHandleVendors = Standard_True;
    }
    else if (isHandleFormat)
    {
      aFormats.Append(theArgVec[anArgIter]);
    }
    else if (isHandleVendors)
    {
      aVendors.Append(theArgVec[anArgIter]);
    }
    else if (!isHandleFormat && !isHandleVendors)
    {
      theDI << "Syntax error at argument '" << theArgVec[anArgIter] << "'\n";
      return 1;
    }
  }
  Standard_Boolean aStat = Standard_True;
  if (!aPath.IsEmpty())
  {
    aStat = aConf->Save(aPath, aIsRecursive, aFormats, aVendors);
  }
  else
  {
    theDI << aConf->Save(aIsRecursive, aFormats, aVendors) << "\n";
  }
  if (!aStat)
  {
    return 1;
  }
  return 0;
}

//=======================================================================
//function : CompareConfiguration
//purpose  :
//=======================================================================
static Standard_Integer CompareConfiguration(Draw_Interpretor& theDI,
                                             Standard_Integer theNbArgs,
                                             const char** theArgVec)
{
  if (theNbArgs > 5)
  {
    theDI.PrintHelp(theArgVec[0]);
    return 1;
  }
  Handle(DE_ConfigurationContext) aResourceFirst = new DE_ConfigurationContext();
  if (!aResourceFirst->Load(theArgVec[1]))
  {
    theDI << "Error: Can't load first configuration\n";
    return 1;
  }
  Handle(DE_ConfigurationContext) aResourceSecond = new DE_ConfigurationContext();
  if (!aResourceSecond->Load(theArgVec[2]))
  {
    theDI << "Error: Can't load second configuration\n";
    return 1;
  }
  const DE_ResourceMap& aResourceMapFirst = aResourceFirst->GetInternalMap();
  const DE_ResourceMap& aResourceMapSecond = aResourceSecond->GetInternalMap();
  Standard_Integer anDiffers = 0;
  for (DE_ResourceMap::Iterator anOrigIt(aResourceMapFirst);
       anOrigIt.More(); anOrigIt.Next())
  {
    const TCollection_AsciiString& anOrigValue = anOrigIt.Value();
    const TCollection_AsciiString& anOrigKey = anOrigIt.Key();
    TCollection_AsciiString aCompValue;
    if (!aResourceMapSecond.Find(anOrigKey, aCompValue))
    {
      Message::SendWarning() << "Second configuration don't have the next scope : " << anOrigKey;
      anDiffers++;
    }
    if (!aCompValue.IsEqual(anOrigValue))
    {
      Message::SendWarning() << "Configurations have differs value with the next scope :" << anOrigKey
        << " First value : " << anOrigValue << " Second value : " << aCompValue;
      anDiffers++;
    }
  }
  TCollection_AsciiString aMessage;
  if (aResourceMapFirst.Extent() != aResourceMapSecond.Extent() || anDiffers > 0)
  {
    theDI << "Error: Configurations are not same : " << " Differs count : "
      << anDiffers << " Count of first's scopes : " << aResourceMapFirst.Extent()
      << " Count of second's scopes : " << aResourceMapSecond.Extent() << "\n";
    return 1;
  }
  return 0;
}

//=======================================================================
//function : LoadConfiguration
//purpose  :
//=======================================================================
static Standard_Integer LoadConfiguration(Draw_Interpretor& theDI,
                                          Standard_Integer theNbArgs,
                                          const char** theArgVec)
{
  if (theNbArgs > 4)
  {
    theDI.PrintHelp(theArgVec[0]);
    return 1;
  }
  Handle(DE_Wrapper) aConf = DE_Wrapper::GlobalWrapper();
  TCollection_AsciiString aString = theArgVec[1];
  Standard_Boolean aIsRecursive = Standard_True;
  if (theNbArgs == 4)
  {
    TCollection_AsciiString anArg = theArgVec[2];
    anArg.LowerCase();
    if (!(anArg == "-recursive") ||
        !Draw::ParseOnOff(theArgVec[3], aIsRecursive))
    {
      theDI << "Syntax error at argument '" << theArgVec[3] << "'\n";
      return 1;
    }
  }
  if (!aConf->Load(aString, aIsRecursive))
  {
    theDI << "Error: configuration is incorrect\n";
    return 1;
  }
  return 0;
}

//=======================================================================
//function : ReadFile
//purpose  :
//=======================================================================
static Standard_Integer ReadFile(Draw_Interpretor& theDI,
                                 Standard_Integer theNbArgs,
                                 const char** theArgVec)
{
  if (theNbArgs > 6)
  {
    theDI.PrintHelp(theArgVec[0]);
    return 1;
  }
  TCollection_AsciiString aDocShapeName;
  TCollection_AsciiString aFilePath;
  Handle(TDocStd_Document) aDoc;
  Handle(TDocStd_Application) anApp = DDocStd::GetApplication();
  TCollection_AsciiString aConfString;
  Standard_Boolean isNoDoc = (TCollection_AsciiString(theArgVec[0]) == "readfile");
  for (Standard_Integer anArgIter = 1; anArgIter < theNbArgs; ++anArgIter)
  {
    TCollection_AsciiString anArg(theArgVec[anArgIter]);
    anArg.LowerCase();
    if ((anArg == "-conf") &&
        (anArgIter + 1 < theNbArgs))
    {
      ++anArgIter;
      aConfString = theArgVec[anArgIter];
    }
    else if (aDocShapeName.IsEmpty())
    {
      aDocShapeName = theArgVec[anArgIter];
      Standard_CString aNameVar = aDocShapeName.ToCString();
      if (!isNoDoc)
      {
        DDocStd::GetDocument(aNameVar, aDoc, Standard_False);
      }
    }
    else if (aFilePath.IsEmpty())
    {
      aFilePath = theArgVec[anArgIter];
    }
    else
    {
      theDI << "Syntax error at argument '" << theArgVec[anArgIter] << "'\n";
      return 1;
    }
  }
  if (aDocShapeName.IsEmpty() || aFilePath.IsEmpty())
  {
    theDI << "Syntax error: wrong number of arguments\n";
    return 1;
  }
  if (aDoc.IsNull() && !isNoDoc)
  {
    anApp->NewDocument(TCollection_ExtendedString("BinXCAF"), aDoc);
    Handle(DDocStd_DrawDocument) aDrawDoc = new DDocStd_DrawDocument(aDoc);
    TDataStd_Name::Set(aDoc->GetData()->Root(), theArgVec[1]);
    Draw::Set(theArgVec[1], aDrawDoc);
  }

  Handle(DE_Wrapper) aConf = DE_Wrapper::GlobalWrapper()->Copy();
  Standard_Boolean aStat = Standard_True;
  if (!aConfString.IsEmpty())
  {
    aStat = aConf->Load(aConfString);
  }
  if (aStat)
  {
    TopoDS_Shape aShape;
    Handle(XSControl_WorkSession) aWS = XSDRAW::Session();
    aStat = isNoDoc ? aConf->Read(aFilePath, aShape, aWS) : aConf->Read(aFilePath, aDoc, aWS);
    if (isNoDoc && aStat)
    {
      DBRep::Set(aDocShapeName.ToCString(), aShape);
    }
  }
  if (!aStat)
  {
    return 1;
  }
  return 0;
}

//=======================================================================
//function : WriteFile
//purpose  :
//=======================================================================
static Standard_Integer WriteFile(Draw_Interpretor& theDI,
                                  Standard_Integer theNbArgs,
                                  const char** theArgVec)
{
  if (theNbArgs > 6)
  {
    theDI.PrintHelp(theArgVec[0]);
    return 1;
  }
  TCollection_AsciiString aDocShapeName;
  TCollection_AsciiString aFilePath;
  Handle(TDocStd_Document) aDoc;
  Handle(TDocStd_Application) anApp = DDocStd::GetApplication();
  TCollection_AsciiString aConfString;
  Standard_Boolean isNoDoc = (TCollection_AsciiString(theArgVec[0]) == "writefile");
  for (Standard_Integer anArgIter = 1; anArgIter < theNbArgs; ++anArgIter)
  {
    TCollection_AsciiString anArg(theArgVec[anArgIter]);
    anArg.LowerCase();
    if ((anArg == "-conf") &&
        (anArgIter + 1 < theNbArgs))
    {
      ++anArgIter;
      aConfString = theArgVec[anArgIter];
    }
    else if (aDocShapeName.IsEmpty())
    {
      aDocShapeName = theArgVec[anArgIter];
      Standard_CString aNameVar = aDocShapeName.ToCString();
      if (!isNoDoc)
      {
        DDocStd::GetDocument(aNameVar, aDoc, Standard_False);
      }
    }
    else if (aFilePath.IsEmpty())
    {
      aFilePath = theArgVec[anArgIter];
    }
    else
    {
      theDI << "Syntax error at argument '" << theArgVec[anArgIter] << "'\n";
      return 1;
    }
  }
  if (aDocShapeName.IsEmpty() || aFilePath.IsEmpty())
  {
    theDI << "Syntax error: wrong number of arguments\n";
    return 1;
  }
  if (aDoc.IsNull() && !isNoDoc)
  {
    theDI << "Error: incorrect document\n";
    return 1;
  }
  Handle(DE_Wrapper) aConf = DE_Wrapper::GlobalWrapper()->Copy();
  Standard_Boolean aStat = Standard_True;
  if (!aConfString.IsEmpty())
  {
    aStat = aConf->Load(aConfString);
  }
  Handle(XSControl_WorkSession) aWS = XSDRAW::Session();
  if (aStat)
  {
    if (isNoDoc)
    {
      TopoDS_Shape aShape = DBRep::Get(aDocShapeName);
      if (aShape.IsNull())
      {
        theDI << "Error: incorrect shape " << aDocShapeName << "\n";
        return 1;
      }
      aStat = aConf->Write(aFilePath, aShape, aWS);
    }
    else
    {
      aStat = aConf->Write(aFilePath, aDoc, aWS);
    }
  }
  if (!aStat)
  {
    return 1;
  }
  return 0;
}

void XDEDRAW_Common::InitCommands(Draw_Interpretor& di)
{
  static Standard_Boolean initactor = Standard_False;
  if (initactor)
  {
    return;
  }
  initactor = Standard_True;

  Standard_CString g = "XDE translation commands";

  di.Add("ReadIges", "Doc filename: Read IGES file to DECAF document", __FILE__, ReadIges, g);
  di.Add("WriteIges", "Doc filename: Write DECAF document to IGES file", __FILE__, WriteIges, g);
  di.Add("ReadStep",
         "Doc filename [mode] [-stream]"
         "\n\t\t: Read STEP file to a document."
         "\n\t\t:  -stream read using istream reading interface (testing)",
         __FILE__, ReadStep, g);
  di.Add("WriteStep" ,
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

  di.Add("XFileList", "Print list of files that was transferred by the last transfer", __FILE__, GetDicWSList, g);
  di.Add("XFileCur", ": returns name of file which is set as current", __FILE__, GetCurWS, g);
  di.Add("XFileSet", "filename: Set the specified file to be the current one", __FILE__, SetCurWS, g);
  di.Add("XFromShape", "shape: do fromshape command for all the files", __FILE__, FromShape, g);

  di.Add("XExpand", "XExpand Doc recursively(0/1) or XExpand Doc recursively(0/1) label1 label2 ..."
         "or XExpand Doc recursively(0/1) shape1 shape2 ...", __FILE__, Expand, g);
  di.Add("XExtract", "XExtract dstDoc [dstAssmblSh] srcDoc srcLabel1 srcLabel2 ...\t"
         "Extracts given srcLabel1 srcLabel2 ... from srcDoc into given Doc or assembly shape",
         __FILE__, Extract, g);

  di.Add("ReadVrml",
         "ReadVrml docName filePath [-fileCoordSys {Zup|Yup}] [-fileUnit Unit]"
         "\n\t\t:                   [-systemCoordSys {Zup|Yup}] [-noCreateDoc] [-fillIncomplete {ON|OFF}]"
         "\n\t\t: Read Vrml file into XDE document."
         "\n\t\t:   -fileCoordSys   coordinate system defined by Vrml file; Yup when not specified."
         "\n\t\t:   -fileUnit       length unit of Vrml file content."
         "\n\t\t:   -systemCoordSys result coordinate system; Zup when not specified."
         "\n\t\t:   -noCreateDoc    read into existing XDE document."
         "\n\t\t:   -fillIncomplete fill the document with partially retrieved data even if reader has failed with "
         "error; true when not specified", 
         __FILE__, ReadVrml, g);
  di.Add("WriteVrml",
         "WriteVrml Doc filename [version VRML#1.0/VRML#2.0 (1/2): 2 by default] [representation shaded/wireframe/both (0/1/2): 0 by default]",
         __FILE__, WriteVrml, g);

  di.Add("DumpConfiguration",
         "DumpConfiguration [-path <path>] [-recursive {on|off}] [-format fmt1 fmt2 ...] [-vendor vend1 vend2 ...]\n"
         "\n\t\t: Dump special resource generated from global configuration."
         "\n\t\t:   '-path' - save resource configuration to the file"
         "\n\t\t:   '-recursive' - flag to generate a resource from providers. Default is On. Off disables other options"
         "\n\t\t:   '-format' - flag to generate a resource for choosen formats. If list is empty, generate it for all"
         "\n\t\t:   '-vendor' - flag to generate a resource for choosen vendors. If list is empty, generate it for all",
         __FILE__, DumpConfiguration, g);
  di.Add("LoadConfiguration",
         "LoadConfiguration conf [-recursive {on|off}]\n"
         "\n\t\t:   'conf' - path to the resouce file or string value in the special format"
         "\n\t\t:   '-recursive' - flag to generate a resource for all providers. Default is true"
         "\n\t\t: Configure global configuration according special resource",
         __FILE__, LoadConfiguration, g);
  di.Add("CompareConfiguration",
         "CompareConfiguration conf1 conf2\n"
         "\n\t\t:   'conf1' - path to the first resouce file or string value in the special format"
         "\n\t\t:   'conf2' - path to the second resouce file or string value in the special format"
         "\n\t\t: Compare two configurations",
         __FILE__, CompareConfiguration, g);
  di.Add("ReadFile",
         "ReadFile docName filePath [-conf <value|path>]\n"
         "\n\t\t: Read CAD file to document with registered format's providers. Use global configuration by default.",
         __FILE__, ReadFile, g);
  di.Add("readfile",
         "readfile shapeName filePath [-conf <value|path>]\n"
         "\n\t\t: Read CAD file to shape with registered format's providers. Use global configuration by default.",
         __FILE__, ReadFile, g);
  di.Add("WriteFile",
         "WriteFile docName filePath [-conf <value|path>]\n"
         "\n\t\t: Write CAD file to document with registered format's providers. Use global configuration by default.",
         __FILE__, WriteFile, g);
  di.Add("writefile",
         "writefile shapeName filePath [-conf <value|path>]\n"
         "\n\t\t: Write CAD file to shape with registered format's providers. Use global configuration by default.",
         __FILE__, WriteFile, g);
}
