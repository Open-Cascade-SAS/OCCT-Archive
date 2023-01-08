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

#include <AIS_InteractiveContext.hxx>
//#include <Aspect_TypeOfMarker.hxx>
//#include <Bnd_Box.hxx>
#include <BRep_Builder.hxx>
#include <BRepLib_PointCloudShape.hxx>
#include <DBRep.hxx>
#include <DDocStd.hxx>
#include <DDocStd_DrawDocument.hxx>
#include <Draw.hxx>
#include <Draw_Interpretor.hxx>
#include <Draw_PluginMacro.hxx>
#include <Draw_ProgressIndicator.hxx>
#include <Graphic3d_MaterialAspect.hxx>
#include <MeshVS_DataMapOfIntegerAsciiString.hxx>
#include <MeshVS_DeformedDataSource.hxx>
#include <MeshVS_Drawer.hxx>
#include <MeshVS_DrawerAttribute.hxx>
#include <MeshVS_ElementalColorPrsBuilder.hxx>
#include <MeshVS_Mesh.hxx>
#include <MeshVS_MeshEntityOwner.hxx>
#include <MeshVS_MeshPrsBuilder.hxx>
#include <MeshVS_NodalColorPrsBuilder.hxx>
#include <MeshVS_PrsBuilder.hxx>
#include <MeshVS_TextPrsBuilder.hxx>
#include <MeshVS_VectorPrsBuilder.hxx>
#include <OSD_Path.hxx>
#include <Quantity_Color.hxx>
//#include <Quantity_HArray1OfColor.hxx>
#include <Quantity_NameOfColor.hxx>
#include <RWGltf_ConfigurationNode.hxx>
#include <RWGltf_Provider.hxx>
//#include <RWGltf_DracoParameters.hxx>
//#include <RWGltf_CafReader.hxx>
//#include <RWGltf_CafWriter.hxx>
#include <RWMesh_FaceIterator.hxx>
#include <RWStl.hxx>
#include <RWStl_ConfigurationNode.hxx>
#include <RWStl_Provider.hxx>
//#include <RWObj.hxx>
#include <RWObj_ConfigurationNode.hxx>
#include <RWObj_Provider.hxx>
//#include <RWObj_CafReader.hxx>
//#include <RWObj_CafWriter.hxx>
#include <RWPly_ConfigurationNode.hxx>
#include <RWPly_Provider.hxx>
//#include <RWPly_CafWriter.hxx>
#include <RWPly_PlyWriterContext.hxx>
//#include <SelectMgr_SelectionManager.hxx>
//#include <Standard_ErrorHandler.hxx>
//#include <StdSelect_ViewerSelector3d.hxx>
//#include <StlAPI.hxx>
//#include <StlAPI_Writer.hxx>
//#include <TColgp_SequenceOfXYZ.hxx>
#include <TCollection_AsciiString.hxx>
#include <TColStd_Array1OfReal.hxx>
#include <TColStd_HPackedMapOfInteger.hxx>
#include <TColStd_MapIteratorOfPackedMapOfInteger.hxx>
#include <TDataStd_Name.hxx>
#include <TDocStd_Application.hxx>
#include <TDocStd_Document.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <UnitsAPI.hxx>
#include <UnitsMethods.hxx>
#include <V3d_View.hxx>
#include <ViewerTest.hxx>
#include <Vrml_ConfigurationNode.hxx>
#include <Vrml_Provider.hxx>
//#include <VrmlAPI.hxx>
//#include <VrmlAPI_Writer.hxx>
//#include <VrmlData_DataMapOfShapeAppearance.hxx>
//#include <VrmlData_Scene.hxx>
//#include <VrmlData_ShapeConvert.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_ShapeTool.hxx>
#include <XCAFPrs_DocumentExplorer.hxx>
#include <XSAlgo.hxx>
#include <XSAlgo_AlgoContainer.hxx>
#include <XSDRAW.hxx>
#include <XSDRAWIGES.hxx>
#include <XSDRAWSTEP.hxx>

#ifndef _STDIO_H
#include <stdio.h>
#endif

extern Standard_Boolean VDisplayAISObject(const TCollection_AsciiString& theName,
                                          const Handle(AIS_InteractiveObject)& theAISObj,
                                          Standard_Boolean theReplaceIfExists = Standard_True);

//=============================================================================
//function : parseNameFormat
//purpose  : Parse RWMesh_NameFormat enumeration
//=============================================================================
static bool parseNameFormat(const char* theArg,
                            RWMesh_NameFormat& theFormat)
{
  TCollection_AsciiString aName(theArg);
  aName.LowerCase();
  if (aName == "empty")
  {
    theFormat = RWMesh_NameFormat_Empty;
  }
  else if (aName == "product"
           || aName == "prod")
  {
    theFormat = RWMesh_NameFormat_Product;
  }
  else if (aName == "instance"
           || aName == "inst")
  {
    theFormat = RWMesh_NameFormat_Instance;
  }
  else if (aName == "instanceorproduct"
           || aName == "instance||product"
           || aName == "instance|product"
           || aName == "instorprod"
           || aName == "inst||prod"
           || aName == "inst|prod")
  {
    theFormat = RWMesh_NameFormat_InstanceOrProduct;
  }
  else if (aName == "productorinstance"
           || aName == "product||instance"
           || aName == "product|instance"
           || aName == "prodorinst"
           || aName == "prod||inst"
           || aName == "prod|inst")
  {
    theFormat = RWMesh_NameFormat_ProductOrInstance;
  }
  else if (aName == "productandinstance"
           || aName == "prodandinst"
           || aName == "product&instance"
           || aName == "prod&inst")
  {
    theFormat = RWMesh_NameFormat_ProductAndInstance;
  }
  else if (aName == "productandinstanceandocaf"
           || aName == "verbose"
           || aName == "debug")
  {
    theFormat = RWMesh_NameFormat_ProductAndInstanceAndOcaf;
  }
  else
  {
    return false;
  }
  return true;
}

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
//function : GetLengthUnit
//purpose  : Gets length unit value from static interface and document in M
//=======================================================================
static Standard_Real GetLengthUnit(const Handle(TDocStd_Document)& theDoc = nullptr)
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
  aNode->GlobalParameters.LengthUnit = GetLengthUnit();
  Standard_Boolean toUseExistingDoc = Standard_False;
  //Standard_Real aFileUnitFactor = -1.0;
  //RWMesh_CoordinateSystem aResultCoordSys = RWMesh_CoordinateSystem_Zup, aFileCoordSys = RWMesh_CoordinateSystem_Yup;
  //Standard_Boolean toListExternalFiles = Standard_False, isSingleFace = Standard_False, isSinglePrecision = Standard_False;
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
    //else if (anArgCase == "-listexternalfiles"
    //         || anArgCase == "-listexternals"
    //         || anArgCase == "-listexternal"
    //         || anArgCase == "-external"
    //         || anArgCase == "-externalfiles")
    //{
    //  toListExternalFiles = Standard_True;
    //}
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
  Handle(RWStl_Provider) aProvider = new RWStl_Provider(aNode);
  Handle(XSControl_WorkSession) aWS = XSDRAW::Session();
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
  Handle(RWObj_ConfigurationNode) aNode = new RWObj_ConfigurationNode();
  aNode->GlobalParameters.LengthUnit = GetLengthUnit();
  Handle(TDocStd_Application) anApp = DDocStd::GetApplication();
  //TColStd_IndexedDataMapOfStringString aFileInfo;
  //Standard_Real aFileUnitFactor = -1.0;
  //RWMesh_CoordinateSystem aSystemCoordSys = RWMesh_CoordinateSystem_Zup, aFileCoordSys = RWMesh_CoordinateSystem_Yup;
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
        TopoDS_Shape aShape = DBRep::Get(aNameVar);
        if (aShape.IsNull())
        {
          theDI << "Syntax error: '" << aNameVar << "' is not a shape nor document\n";
          return 1;
        }
        anApp->NewDocument(TCollection_ExtendedString("BinXCAF"), aDoc);
        Handle(XCAFDoc_ShapeTool) aShapeTool = XCAFDoc_DocumentTool::ShapeTool(aDoc->Main());
        aShapeTool->AddShape(aShape);
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
  Handle(RWStl_Provider) aProvider = new RWStl_Provider(aNode);
  Handle(XSControl_WorkSession) aWS = XSDRAW::Session();
  if (!aProvider->Write(theArgVec[2], aDoc, aWS, aProgress->Start()))
  {
    theDI << "Error: Mesh writing has been failed.\n";
  }
  return 0;
}

//=======================================================================
//function : InitCommands
//purpose  :
//=======================================================================
void XSDRAWOBJ::InitCommands(Draw_Interpretor& theCommands)
{
  const char* g = "XSTEP-STL/VRML";  // Step transfer file commands
  theCommands.Add("ReadObj",
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
  theCommands.Add("readobj",
                  "readobj shape file [-fileCoordSys {Zup|Yup}] [-fileUnit Unit]"
                  "\n\t\t:                    [-resultCoordSys {Zup|Yup}] [-singlePrecision]"
                  "\n\t\t:                    [-singleFace]"
                  "\n\t\t: Same as ReadObj but reads OBJ file into a shape instead of a document."
                  "\n\t\t:   -singleFace merge OBJ content into a single triangulation Face.",
                  __FILE__, ReadObj, g);
  theCommands.Add("WriteObj",
                  "WriteObj Doc file [-fileCoordSys {Zup|Yup}] [-fileUnit Unit]"
                  "\n\t\t:                   [-systemCoordSys {Zup|Yup}]"
                  "\n\t\t:                   [-comments Text] [-author Name]"
                  "\n\t\t: Write XDE document into OBJ file."
                  "\n\t\t:   -fileUnit       length unit of OBJ file content;"
                  "\n\t\t:   -fileCoordSys   coordinate system defined by OBJ file; Yup when not specified."
                  "\n\t\t:   -systemCoordSys system coordinate system; Zup when not specified.",
                  __FILE__, WriteObj, g);
  theCommands.Add("writeobj",
                  "writeobj shape file",
                  __FILE__, WriteObj, g);
}
