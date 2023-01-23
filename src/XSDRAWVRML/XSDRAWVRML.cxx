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

#include <XSDRAWVRML.hxx>

#include <DBRep.hxx>
#include <DDocStd.hxx>
#include <DDocStd_DrawDocument.hxx>
#include <DE_ConfigurationNode.hxx>
#include <DE_Wrapper.hxx>
#include <Draw.hxx>
#include <Draw_Interpretor.hxx>
#include <Draw_PluginMacro.hxx>
#include <Draw_ProgressIndicator.hxx>
#include <Vrml_ConfigurationNode.hxx>
#include <Vrml_Provider.hxx>
#include <XSControl_WorkSession.hxx>
#include <XSDRAWBase.hxx>
#include <TDataStd_Name.hxx>
#include <TDocStd_Application.hxx>
#include <TopoDS_Shape.hxx>
#include <UnitsAPI.hxx>

//=============================================================================
//function : parseCoordinateSystem
//purpose  : Parse RWMesh_CoordinateSystem enumeration
//=============================================================================
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
  aNode->GlobalParameters.LengthUnit = XSDRAWBase::GetLengthUnit(aDoc);
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
  Handle(XSControl_WorkSession) aWS = XSDRAWBase::Session();
  if (!aProvider->Read(aFilePath, aDoc, aWS, aProgress->Start()))
  {
    theDI << "Error: file reading failed '" << aFilePath << "'\n";
    return 1;
  }
  TDataStd_Name::Set(aDoc->GetData()->Root(), aDocName);
  Handle(DDocStd_DrawDocument) aDD = new DDocStd_DrawDocument(aDoc);
  Draw::Set(aDocName, aDD);
  XSDRAWBase::CollectActiveWorkSessions(aWS, aFilePath, XSDRAWBase::WorkSessionList());
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
  aNode->GlobalParameters.LengthUnit = XSDRAWBase::GetLengthUnit(aDoc);
  Handle(Vrml_Provider) aProvider =
    new Vrml_Provider(aNode);

  Handle(Draw_ProgressIndicator) aProgress = new Draw_ProgressIndicator(theDI, 1);
  Handle(XSControl_WorkSession) aWS = XSDRAWBase::Session();
  if (!aProvider->Write(theArgVec[2], aDoc, aWS, aProgress->Start()))
  {
    theDI << "Error: file writing failed '" << theArgVec[2] << "'\n";
    return 1;
  }
  XSDRAWBase::CollectActiveWorkSessions(aWS, theArgVec[2], XSDRAWBase::WorkSessionList());
  return 0;
}

//=======================================================================
//function : loadvrml
//purpose  :
//=======================================================================
static Standard_Integer loadvrml(Draw_Interpretor& theDI,
                                 Standard_Integer theNbArgs,
                                 const char** theArgVec)
{
  if (theNbArgs < 3)
  {
    theDI << "Error: wrong number of parameters" << "\n";
    return 1;
  }
  Handle(Vrml_ConfigurationNode) aNode =
    new Vrml_ConfigurationNode();
  aNode->GlobalParameters.LengthUnit = XSDRAWBase::GetLengthUnit();
  Handle(Vrml_Provider) aProvider =
    new Vrml_Provider(aNode);
  TopoDS_Shape aShape;
  Handle(Draw_ProgressIndicator) aProgress = new Draw_ProgressIndicator(theDI, 1);
  Handle(XSControl_WorkSession) aWS = XSDRAWBase::Session();
  if (!aProvider->Read(theArgVec[2], aShape, aWS, aProgress->Start()))
  {
    theDI << "Error: file reading failed '" << theArgVec[2] << "'\n";
    return 1;
  }
  DBRep::Set(theArgVec[1], aShape);
  return 0;
}

//=============================================================================
//function : writevrml
//purpose  :
//=============================================================================
static Standard_Integer writevrml(Draw_Interpretor& theDI,
                                  Standard_Integer theNbArgs,
                                  const char** theArgVec)
{
  if (theNbArgs < 3 || theNbArgs > 5)
  {
    theDI << "wrong number of parameters\n";
    return 0;
  }
  TopoDS_Shape aShape = DBRep::Get(theArgVec[1]);
  // Get the optional parameters
  Standard_Integer aVersion = 2;
  Standard_Integer aType = 1;
  if (theNbArgs >= 4)
  {
    aVersion = Draw::Atoi(theArgVec[3]);
    if (theNbArgs == 5)
      aType = Draw::Atoi(theArgVec[4]);
  }
  // Bound parameters
  aVersion = Max(1, aVersion);
  aVersion = Min(2, aVersion);
  aType = Max(0, aType);
  aType = Min(2, aType);
  Handle(Vrml_ConfigurationNode) aNode =
    new Vrml_ConfigurationNode();
  aNode->GlobalParameters.LengthUnit = XSDRAWBase::GetLengthUnit();
  Handle(Vrml_Provider) aProvider =
    new Vrml_Provider(aNode);
  aNode->InternalParameters.WriterVersion =
    (Vrml_ConfigurationNode::WriteMode_WriterVersion)aVersion;
  aNode->InternalParameters.WriteRepresentationType =
    (Vrml_ConfigurationNode::WriteMode_RepresentationType)aType;

  Handle(XSControl_WorkSession) aWS = XSDRAWBase::Session();
  Handle(Draw_ProgressIndicator) aProgress = new Draw_ProgressIndicator(theDI, 1);
  if (!aProvider->Write(theArgVec[2], aShape, aWS, aProgress->Start()))
  {
    theDI << "Error: file writing failed '" << theArgVec[2] << "'\n";
    return 1;
  }
  return 0;
}

//=============================================================================
//function : Factory
//purpose  :
//=============================================================================
void XSDRAWVRML::Factory(Draw_Interpretor& theDI)
{
  static Standard_Boolean anInitActor = Standard_False;
  if (anInitActor)
  {
    return;
  }
  anInitActor = Standard_True;

  DE_Wrapper::GlobalWrapper()->Bind(new Vrml_ConfigurationNode());

  Standard_CString aGroup = "XDE translation commands";
  theDI.Add("ReadVrml",
            "ReadVrml docName filePath [-fileCoordSys {Zup|Yup}] [-fileUnit Unit]"
            "\n\t\t:                   [-systemCoordSys {Zup|Yup}] [-noCreateDoc] [-fillIncomplete {ON|OFF}]"
            "\n\t\t: Read Vrml file into XDE document."
            "\n\t\t:   -fileCoordSys   coordinate system defined by Vrml file; Yup when not specified."
            "\n\t\t:   -fileUnit       length unit of Vrml file content."
            "\n\t\t:   -systemCoordSys result coordinate system; Zup when not specified."
            "\n\t\t:   -noCreateDoc    read into existing XDE document."
            "\n\t\t:   -fillIncomplete fill the document with partially retrieved data even if reader has failed with "
            "error; true when not specified",
            __FILE__, ReadVrml, aGroup);
  theDI.Add("WriteVrml",
            "WriteVrml Doc filename [version VRML#1.0/VRML#2.0 (1/2): 2 by default] [representation shaded/wireframe/both (0/1/2): 0 by default]",
            __FILE__, WriteVrml, aGroup);
  theDI.Add("loadvrml", "shape file", __FILE__, loadvrml, aGroup);
  theDI.Add("writevrml", "shape file [version VRML#1.0/VRML#2.0 (1/2): 2 by default] [representation shaded/wireframe/both (0/1/2): 1 by default]", __FILE__, writevrml, aGroup);
}

// Declare entry point PLUGINFACTORY
DPLUGIN(XSDRAWVRML)
