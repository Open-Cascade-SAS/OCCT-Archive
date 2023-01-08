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

#include <XSDRAWOBJ.hxx>

#include <DBRep.hxx>
#include <DDocStd.hxx>
#include <DDocStd_DrawDocument.hxx>
#include <Draw.hxx>
#include <Draw_Interpretor.hxx>
#include <Draw_ProgressIndicator.hxx>
#include <RWObj_ConfigurationNode.hxx>
#include <RWObj_Provider.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_ShapeTool.hxx>
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

//=============================================================================
//function : ReadObj
//purpose  : Reads OBJ file
//=============================================================================
static Standard_Integer ReadObj(Draw_Interpretor& theDI,
                                Standard_Integer theNbArgs,
                                const char** theArgVec)
{
  TCollection_AsciiString aDestName, aFilePath;
  Handle(RWObj_ConfigurationNode) aNode = new RWObj_ConfigurationNode();
  aNode->GlobalParameters.LengthUnit = XSDRAWBase::GetLengthUnit();
  Standard_Boolean toUseExistingDoc = Standard_False;
  Standard_Boolean isNoDoc = (TCollection_AsciiString(theArgVec[0]) == "readobj");
  for (Standard_Integer anArgIter = 1; anArgIter < theNbArgs; ++anArgIter)
  {
    TCollection_AsciiString anArgCase(theArgVec[anArgIter]);
    anArgCase.LowerCase();
    if (anArgIter + 1 < theNbArgs
        && (anArgCase == "-unit"
        || anArgCase == "-units"
        || anArgCase == "-fileunit"
        || anArgCase == "-fileunits"))
    {
      const TCollection_AsciiString aUnitStr(theArgVec[++anArgIter]);
      aNode->InternalParameters.FileLengthUnit = UnitsAPI::AnyToSI(1.0, aUnitStr.ToCString());
      if (aNode->InternalParameters.FileLengthUnit <= 0.0)
      {
        theDI << "Syntax error: wrong length unit '" << aUnitStr << "'\n";
        return 1;
      }
    }
    else if (anArgIter + 1 < theNbArgs
             && (anArgCase == "-filecoordinatesystem"
             || anArgCase == "-filecoordsystem"
             || anArgCase == "-filecoordsys"))
    {
      if (!parseCoordinateSystem(theArgVec[++anArgIter], aNode->InternalParameters.FileCS))
      {
        theDI << "Syntax error: unknown coordinate system '" << theArgVec[anArgIter] << "'\n";
        return 1;
      }
    }
    else if (anArgIter + 1 < theNbArgs
             && (anArgCase == "-resultcoordinatesystem"
             || anArgCase == "-resultcoordsystem"
             || anArgCase == "-resultcoordsys"
             || anArgCase == "-rescoordsys"))
    {
      if (!parseCoordinateSystem(theArgVec[++anArgIter], aNode->InternalParameters.SystemCS))
      {
        theDI << "Syntax error: unknown coordinate system '" << theArgVec[anArgIter] << "'\n";
        return 1;
      }
    }
    else if (anArgCase == "-singleprecision"
             || anArgCase == "-singleprec")
    {
      aNode->InternalParameters.ReadSinglePrecision = Standard_True;
      if (anArgIter + 1 < theNbArgs
          && Draw::ParseOnOff(theArgVec[anArgIter + 1],
          aNode->InternalParameters.ReadSinglePrecision))
      {
        ++anArgIter;
      }
    }
    else if (isNoDoc
             && (anArgCase == "-singleface"
             || anArgCase == "-singletriangulation"))
    {
      aNode->InternalParameters.ReadCreateShapes = Standard_True;
    }
    else if (!isNoDoc
             && (anArgCase == "-nocreate"
             || anArgCase == "-nocreatedoc"))
    {
      toUseExistingDoc = Standard_True;
      if (anArgIter + 1 < theNbArgs
          && Draw::ParseOnOff(theArgVec[anArgIter + 1], toUseExistingDoc))
      {
        ++anArgIter;
      }
    }
    else if (aDestName.IsEmpty())
    {
      aDestName = theArgVec[anArgIter];
    }
    else if (aFilePath.IsEmpty())
    {
      aFilePath = theArgVec[anArgIter];
    }
    else
    {
      theDI << "Syntax error at '" << theArgVec[anArgIter] << "'\n";
      return 1;
    }
  }
  if (aFilePath.IsEmpty())
  {
    theDI << "Syntax error: wrong number of arguments\n";
    return 1;
  }
  Handle(TDocStd_Document) aDoc;
  if (!isNoDoc)
  {
    Handle(TDocStd_Application) anApp = DDocStd::GetApplication();
    Standard_CString aNameVar = aDestName.ToCString();
    DDocStd::GetDocument(aNameVar, aDoc, Standard_False);
    if (aDoc.IsNull())
    {
      if (toUseExistingDoc)
      {
        theDI << "Error: document with name " << aDestName << " does not exist\n";
        return 1;
      }
      anApp->NewDocument(TCollection_ExtendedString("BinXCAF"), aDoc);
    }
    else if (!toUseExistingDoc)
    {
      theDI << "Error: document with name " << aDestName << " already exists\n";
      return 1;
    }
  }
  Handle(Draw_ProgressIndicator) aProgress = new Draw_ProgressIndicator(theDI, 1);
  Handle(RWObj_Provider) aProvider = new RWObj_Provider(aNode);
  Handle(XSControl_WorkSession) aWS = XSDRAWBase::Session();
  Standard_Boolean aReadStat = Standard_False;
  if (isNoDoc)
  {
    TopoDS_Shape aShape;
    aReadStat = aProvider->Read(aFilePath, aShape, aWS, aProgress->Start());
    if (aReadStat)
    {
      DBRep::Set(aDestName.ToCString(), aShape);
    }
  }
  else
  {
    aReadStat = aProvider->Read(aFilePath, aDoc, aWS, aProgress->Start());
    if (aReadStat)
    {
      Handle(DDocStd_DrawDocument) aDrawDoc = new DDocStd_DrawDocument(aDoc);
      TDataStd_Name::Set(aDoc->GetData()->Root(), aDestName);
      Draw::Set(aDestName.ToCString(), aDrawDoc);
    }
  }
  if (!aReadStat)
  {
    theDI << "Cannot read any relevant data from the Obj file\n";
    return 1;
  }
  return 0;
}

//=============================================================================
//function : WriteObj
//purpose  : Writes OBJ file
//=============================================================================
static Standard_Integer WriteObj(Draw_Interpretor& theDI,
                                 Standard_Integer theNbArgs,
                                 const char** theArgVec)
{
  TCollection_AsciiString anObjFilePath;
  Handle(TDocStd_Document) aDoc;
  TopoDS_Shape aShape;
  Handle(RWObj_ConfigurationNode) aNode = new RWObj_ConfigurationNode();
  aNode->GlobalParameters.LengthUnit = XSDRAWBase::GetLengthUnit();
  Handle(TDocStd_Application) anApp = DDocStd::GetApplication();
  for (Standard_Integer anArgIter = 1; anArgIter < theNbArgs; ++anArgIter)
  {
    TCollection_AsciiString anArgCase(theArgVec[anArgIter]);
    anArgCase.LowerCase();
    if (anArgIter + 1 < theNbArgs
        && (anArgCase == "-unit"
        || anArgCase == "-units"
        || anArgCase == "-fileunit"
        || anArgCase == "-fileunits"))
    {
      const TCollection_AsciiString aUnitStr(theArgVec[++anArgIter]);
      aNode->InternalParameters.FileLengthUnit = UnitsAPI::AnyToSI(1.0, aUnitStr.ToCString());
      if (aNode->InternalParameters.FileLengthUnit <= 0.0)
      {
        theDI << "Syntax error: wrong length unit '" << aUnitStr << "'\n";
        return 1;
      }
    }
    else if (anArgIter + 1 < theNbArgs
             && (anArgCase == "-filecoordinatesystem"
             || anArgCase == "-filecoordsystem"
             || anArgCase == "-filecoordsys"))
    {
      if (!parseCoordinateSystem(theArgVec[++anArgIter], aNode->InternalParameters.FileCS))
      {
        theDI << "Syntax error: unknown coordinate system '" << theArgVec[anArgIter] << "'\n";
        return 1;
      }
    }
    else if (anArgIter + 1 < theNbArgs
             && (anArgCase == "-systemcoordinatesystem"
             || anArgCase == "-systemcoordsystem"
             || anArgCase == "-systemcoordsys"
             || anArgCase == "-syscoordsys"))
    {
      if (!parseCoordinateSystem(theArgVec[++anArgIter], aNode->InternalParameters.SystemCS))
      {
        theDI << "Syntax error: unknown coordinate system '" << theArgVec[anArgIter] << "'\n";
        return 1;
      }
    }
    else if (anArgCase == "-comments"
             && anArgIter + 1 < theNbArgs)
    {
      aNode->InternalParameters.WriteComment = theArgVec[++anArgIter];
    }
    else if (anArgCase == "-author"
             && anArgIter + 1 < theNbArgs)
    {
      aNode->InternalParameters.WriteAuthor = theArgVec[++anArgIter];
    }
    else if (aDoc.IsNull())
    {
      Standard_CString aNameVar = theArgVec[anArgIter];
      DDocStd::GetDocument(aNameVar, aDoc, false);
      if (aDoc.IsNull())
      {
        aShape = DBRep::Get(aNameVar);
        if (aShape.IsNull())
        {
          theDI << "Syntax error: '" << aNameVar << "' is not a shape nor document\n";
          return 1;
        }
      }
    }
    else if (anObjFilePath.IsEmpty())
    {
      anObjFilePath = theArgVec[anArgIter];
    }
    else
    {
      theDI << "Syntax error at '" << theArgVec[anArgIter] << "'\n";
      return 1;
    }
  }
  if (anObjFilePath.IsEmpty())
  {
    theDI << "Syntax error: wrong number of arguments\n";
    return 1;
  }
  Handle(Draw_ProgressIndicator) aProgress = new Draw_ProgressIndicator(theDI, 1);
  Handle(RWObj_Provider) aProvider = new RWObj_Provider(aNode);
  Handle(XSControl_WorkSession) aWS = XSDRAWBase::Session();
  Standard_Boolean aWriteStatus = Standard_False;
  if (aDoc.IsNull())
  {
    aWriteStatus = aProvider->Write(theArgVec[2], aShape, aWS, aProgress->Start());
  }
  else
  {
    aWriteStatus = aProvider->Write(theArgVec[2], aDoc, aWS, aProgress->Start());
  }
  if (!aWriteStatus)
  {
    theDI << "Error: Mesh writing has been failed\n";
    return 1;
  }
  return 0;
}

//=======================================================================
//function : InitCommands
//purpose  :
//=======================================================================
void XSDRAWOBJ::Factory(Draw_Interpretor& theDI)
{
  const char* g = "XSTEP-STL/VRML";  // Step transfer file commands
  theDI.Add("ReadObj",
            "ReadObj Doc file [-fileCoordSys {Zup|Yup}] [-fileUnit Unit]"
            "\n\t\t:                  [-resultCoordSys {Zup|Yup}] [-singlePrecision]"
            "\n\t\t:                  [-listExternalFiles] [-noCreateDoc]"
            "\n\t\t: Read OBJ file into XDE document."
            "\n\t\t:   -fileUnit       length unit of OBJ file content;"
            "\n\t\t:   -fileCoordSys   coordinate system defined by OBJ file; Yup when not specified."
            "\n\t\t:   -resultCoordSys result coordinate system; Zup when not specified."
            "\n\t\t:   -singlePrecision truncate vertex data to single precision during read; FALSE by default."
            "\n\t\t:   -listExternalFiles do not read mesh and only list external files."
            "\n\t\t:   -noCreateDoc    read into existing XDE document.",
            __FILE__, ReadObj, g);
  theDI.Add("readobj",
            "readobj shape file [-fileCoordSys {Zup|Yup}] [-fileUnit Unit]"
            "\n\t\t:                    [-resultCoordSys {Zup|Yup}] [-singlePrecision]"
            "\n\t\t:                    [-singleFace]"
            "\n\t\t: Same as ReadObj but reads OBJ file into a shape instead of a document."
            "\n\t\t:   -singleFace merge OBJ content into a single triangulation Face.",
            __FILE__, ReadObj, g);
  theDI.Add("WriteObj",
            "WriteObj Doc file [-fileCoordSys {Zup|Yup}] [-fileUnit Unit]"
            "\n\t\t:                   [-systemCoordSys {Zup|Yup}]"
            "\n\t\t:                   [-comments Text] [-author Name]"
            "\n\t\t: Write XDE document into OBJ file."
            "\n\t\t:   -fileUnit       length unit of OBJ file content;"
            "\n\t\t:   -fileCoordSys   coordinate system defined by OBJ file; Yup when not specified."
            "\n\t\t:   -systemCoordSys system coordinate system; Zup when not specified.",
            __FILE__, WriteObj, g);
  theDI.Add("writeobj",
            "writeobj shape file",
            __FILE__, WriteObj, g);
}
