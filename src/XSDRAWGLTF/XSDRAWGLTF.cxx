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

#include <XSDRAWGLTF.hxx>

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
//function : ReadGltf
//purpose  : Reads glTF file
//=============================================================================
static Standard_Integer ReadGltf(Draw_Interpretor& theDI,
                                 Standard_Integer theNbArgs,
                                 const char** theArgVec)
{
  TCollection_AsciiString aDestName, aFilePath;
  Handle(RWGltf_ConfigurationNode) aNode =
    new RWGltf_ConfigurationNode();
  Standard_Boolean toUseExistingDoc = Standard_False;
  //Standard_Boolean toListExternalFiles = Standard_False;
  //Standard_Boolean isParallel = Standard_False;
  //Standard_Boolean isDoublePrec = Standard_False;
  //Standard_Boolean toSkipLateDataLoading = Standard_False;
  //Standard_Boolean toKeepLateData = Standard_True;
  //Standard_Boolean toPrintDebugInfo = Standard_False;
  //Standard_Boolean toLoadAllScenes = Standard_False;
  //Standard_Boolean toPrintAssetInfo = Standard_False;
  Standard_Boolean isNoDoc = (TCollection_AsciiString(theArgVec[0]) == "readgltf");
  for (Standard_Integer anArgIter = 1; anArgIter < theNbArgs; ++anArgIter)
  {
    TCollection_AsciiString anArgCase(theArgVec[anArgIter]);
    anArgCase.LowerCase();
    if (!isNoDoc
        && (anArgCase == "-nocreate"
        || anArgCase == "-nocreatedoc"))
    {
      toUseExistingDoc = Draw::ParseOnOffIterator(theNbArgs, theArgVec, anArgIter);
    }
    else if (anArgCase == "-parallel")
    {
      aNode->InternalParameters.ReadParallel =
        Draw::ParseOnOffIterator(theNbArgs, theArgVec, anArgIter);
    }
    else if (anArgCase == "-doubleprec"
             || anArgCase == "-doubleprecision"
             || anArgCase == "-singleprec"
             || anArgCase == "-singleprecision")
    {
      aNode->InternalParameters.ReadSinglePrecision =
        !Draw::ParseOnOffIterator(theNbArgs, theArgVec, anArgIter);
      if (anArgCase.StartsWith("-single"))
      {
        aNode->InternalParameters.ReadSinglePrecision =
          Draw::ParseOnOffIterator(theNbArgs, theArgVec, anArgIter);
      }
    }
    else if (anArgCase == "-skiplateloading")
    {
      aNode->InternalParameters.ReadSkipLateDataLoading =
        Draw::ParseOnOffIterator(theNbArgs, theArgVec, anArgIter);
    }
    else if (anArgCase == "-keeplate")
    {
      aNode->InternalParameters.ReadKeepLateData =
        Draw::ParseOnOffIterator(theNbArgs, theArgVec, anArgIter);
    }
    else if (anArgCase == "-allscenes")
    {
      aNode->InternalParameters.ReadLoadAllScenes =
        Draw::ParseOnOffIterator(theNbArgs, theArgVec, anArgIter);
    }
    else if (anArgCase == "-toprintinfo"
             || anArgCase == "-toprintdebuginfo")
    {
      aNode->InternalParameters.ReadPrintDebugMessages =
        Draw::ParseOnOffIterator(theNbArgs, theArgVec, anArgIter);
    }
    //else if (anArgCase == "-listexternalfiles"
    //      || anArgCase == "-listexternals"
    //      || anArgCase == "-listexternal"
    //      || anArgCase == "-external"
    //      || anArgCase == "-externalfiles")
    //{
    //  toListExternalFiles = Draw::ParseOnOffIterator (theNbArgs, theArgVec, anArgIter);
    //}
    //else if (anArgCase == "-assetinfo"
    //      || anArgCase == "-metadata")
    //{
    //  toPrintAssetInfo = Draw::ParseOnOffIterator (theNbArgs, theArgVec, anArgIter);
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
  //if (aFilePath.IsEmpty() && !aDestName.IsEmpty())
  //{
  //  if (toListExternalFiles || toPrintAssetInfo)
  //  {
  //    std::swap (aFilePath, aDestName);
  //  }
  //}
  if (aFilePath.IsEmpty() || aDestName.IsEmpty())
  {
    theDI << "Syntax error: wrong number of arguments\n";
    return 1;
  }

  Handle(Draw_ProgressIndicator) aProgress = new Draw_ProgressIndicator(theDI, 1);
  Handle(TDocStd_Document) aDoc;
  if (!aDestName.IsEmpty()
      && !isNoDoc)
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

  aNode->GlobalParameters.LengthUnit = GetLengthUnit(aDoc);
  Handle(RWGltf_Provider) aProvider =
    new RWGltf_Provider(aNode);
  Standard_Boolean aReadStat = Standard_False;
  Handle(XSControl_WorkSession) aWS = XSDRAW::Session();
  if (isNoDoc)
  {
    TopoDS_Shape aResShape;
    aReadStat = aProvider->Read(aFilePath, aResShape, aWS, aProgress->Start());
    if (aReadStat)
    {
      DBRep::Set(aDestName.ToCString(), aResShape);
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
    theDI << "Cannot read any relevant data from the GLTF file\n";
    return 1;
  }
  //bool isFirstLine = true;
  //if (toPrintAssetInfo)
  //{
  //  for (TColStd_IndexedDataMapOfStringString::Iterator aKeyIter (aReader.Metadata()); aKeyIter.More(); aKeyIter.Next())
  //  {
  //    if (!isFirstLine)
  //    {
  //      theDI << "\n";
  //    }
  //    isFirstLine = false;
  //    theDI << aKeyIter.Key() << ": " << aKeyIter.Value();
  //  }
  //}
  //if (toListExternalFiles)
  //{
  //  if (!isFirstLine)
  //  {
  //    theDI << "\n";
  //  }
  //  for (NCollection_IndexedMap<TCollection_AsciiString>::Iterator aFileIter (aReader.ExternalFiles()); aFileIter.More(); aFileIter.Next())
  //  {
  //    theDI << "\"" << aFileIter.Value() << "\" ";
  //  }
  //}

  return 0;
}

//=============================================================================
//function : WriteGltf
//purpose  : Writes glTF file
//=============================================================================
static Standard_Integer WriteGltf(Draw_Interpretor& theDI,
                                  Standard_Integer theNbArgs,
                                  const char** theArgVec)
{
  TCollection_AsciiString aGltfFilePath;
  Handle(TDocStd_Document) aDoc;
  TopoDS_Shape aShape;
  Handle(TDocStd_Application) anApp = DDocStd::GetApplication();
  Handle(RWGltf_ConfigurationNode) aNode =
    new RWGltf_ConfigurationNode();
  //TColStd_IndexedDataMapOfStringString aFileInfo;
  //RWGltf_WriterTrsfFormat aTrsfFormat = RWGltf_WriterTrsfFormat_Compact;
  //RWMesh_CoordinateSystem aSystemCoordSys = RWMesh_CoordinateSystem_Zup;
  //bool toForceUVExport = false, toEmbedTexturesInGlb = true;
  //bool toMergeFaces = false, toSplitIndices16 = false;
  //bool isParallel = false;
  //RWMesh_NameFormat aNodeNameFormat = RWMesh_NameFormat_InstanceOrProduct;
  //RWMesh_NameFormat aMeshNameFormat = RWMesh_NameFormat_Product;
  //RWGltf_DracoParameters aDracoParameters;
  for (Standard_Integer anArgIter = 1; anArgIter < theNbArgs; ++anArgIter)
  {
    TCollection_AsciiString anArgCase(theArgVec[anArgIter]);
    anArgCase.LowerCase();
    if (anArgCase == "-comments"
        && anArgIter + 1 < theNbArgs)
    {
      aNode->InternalParameters.WriteComment = theArgVec[++anArgIter];
    }
    else if (anArgCase == "-author"
             && anArgIter + 1 < theNbArgs)
    {
      aNode->InternalParameters.WriteAuthor = theArgVec[++anArgIter];
    }
    else if (anArgCase == "-forceuvexport"
             || anArgCase == "-forceuv")
    {
      aNode->InternalParameters.WriteForcedUVExport = true;
      if (anArgIter + 1 < theNbArgs
          && Draw::ParseOnOff(theArgVec[anArgIter + 1], aNode->InternalParameters.WriteForcedUVExport))
      {
        ++anArgIter;
      }
    }
    else if (anArgCase == "-mergefaces")
    {
      aNode->InternalParameters.WriteMergeFaces = true;
      if (anArgIter + 1 < theNbArgs
          && Draw::ParseOnOff(theArgVec[anArgIter + 1], aNode->InternalParameters.WriteMergeFaces))
      {
        ++anArgIter;
      }
    }
    else if (anArgCase == "-splitindices16"
             || anArgCase == "-splitindexes16"
             || anArgCase == "-splitindices"
             || anArgCase == "-splitindexes"
             || anArgCase == "-splitind")
    {
      aNode->InternalParameters.WriteSplitIndices16 = true;
      if (anArgIter + 1 < theNbArgs
          && Draw::ParseOnOff(theArgVec[anArgIter + 1], aNode->InternalParameters.WriteSplitIndices16))
      {
        ++anArgIter;
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
    else if (anArgCase == "-trsfformat"
             && anArgIter + 1 < theNbArgs)
    {
      TCollection_AsciiString aTrsfStr(theArgVec[++anArgIter]);
      aTrsfStr.LowerCase();
      if (aTrsfStr == "compact")
      {
        aNode->InternalParameters.WriteTrsfFormat = RWGltf_WriterTrsfFormat_Compact;
      }
      else if (aTrsfStr == "mat4")
      {
        aNode->InternalParameters.WriteTrsfFormat = RWGltf_WriterTrsfFormat_Mat4;
      }
      else if (aTrsfStr == "trs")
      {
        aNode->InternalParameters.WriteTrsfFormat = RWGltf_WriterTrsfFormat_TRS;
      }
      else
      {
        theDI << "Syntax error at '" << anArgCase << "'\n";
        return 1;
      }
    }
    else if (anArgCase == "-nodenameformat"
             || anArgCase == "-nodename")
    {
      ++anArgIter;
      if (anArgIter >= theNbArgs
          || !parseNameFormat(theArgVec[anArgIter], aNode->InternalParameters.WriteNodeNameFormat))
      {
        theDI << "Syntax error at '" << anArgCase << "'\n";
        return 1;
      }
    }
    else if (anArgCase == "-meshnameformat"
             || anArgCase == "-meshname")
    {
      ++anArgIter;
      if (anArgIter >= theNbArgs
          || !parseNameFormat(theArgVec[anArgIter], aNode->InternalParameters.WriteMeshNameFormat))
      {
        theDI << "Syntax error at '" << anArgCase << "'\n";
        return 1;
      }
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
        aNode->InternalParameters.WriteNodeNameFormat = RWMesh_NameFormat_Product;
      }
    }
    else if (aGltfFilePath.IsEmpty())
    {
      aGltfFilePath = theArgVec[anArgIter];
    }
    else if (anArgCase == "-texturesSeparate")
    {
      aNode->InternalParameters.WriteEmbedTexturesInGlb = false;
    }
    else if (anArgCase == "-draco")
    {
      aNode->InternalParameters.WriteDracoParameters.DracoCompression =
        Draw::ParseOnOffIterator(theNbArgs, theArgVec, anArgIter);
    }
    else if (anArgCase == "-compressionlevel" && (anArgIter + 1) < theNbArgs
             && Draw::ParseInteger(theArgVec[anArgIter + 1],
             aNode->InternalParameters.WriteDracoParameters.CompressionLevel))
    {
      ++anArgIter;
    }
    else if (anArgCase == "-quantizepositionbits" && (anArgIter + 1) < theNbArgs
             && Draw::ParseInteger(theArgVec[anArgIter + 1],
             aNode->InternalParameters.WriteDracoParameters.QuantizePositionBits))
    {
      ++anArgIter;
    }
    else if (anArgCase == "-quantizenormalbits" && (anArgIter + 1) < theNbArgs
             && Draw::ParseInteger(theArgVec[anArgIter + 1],
             aNode->InternalParameters.WriteDracoParameters.QuantizeNormalBits))
    {
      ++anArgIter;
    }
    else if (anArgCase == "-quantizetexcoordbits" && (anArgIter + 1) < theNbArgs
             && Draw::ParseInteger(theArgVec[anArgIter + 1],
             aNode->InternalParameters.WriteDracoParameters.QuantizeTexcoordBits))
    {
      ++anArgIter;
    }
    else if (anArgCase == "-quantizecolorbits" && (anArgIter + 1) < theNbArgs
             && Draw::ParseInteger(theArgVec[anArgIter + 1],
             aNode->InternalParameters.WriteDracoParameters.QuantizeColorBits))
    {
      ++anArgIter;
    }
    else if (anArgCase == "-quantizegenericbits" && (anArgIter + 1) < theNbArgs
             && Draw::ParseInteger(theArgVec[anArgIter + 1],
             aNode->InternalParameters.WriteDracoParameters.QuantizeGenericBits))
    {
      ++anArgIter;
    }
    else if (anArgCase == "-unifiedquantization")
    {
      aNode->InternalParameters.WriteDracoParameters.UnifiedQuantization =
        Draw::ParseOnOffIterator(theNbArgs, theArgVec, anArgIter);
    }
    else if (anArgCase == "-parallel")
    {
      aNode->InternalParameters.WriteParallel =
        Draw::ParseOnOffIterator(theNbArgs, theArgVec, anArgIter);
    }
    else
    {
      theDI << "Syntax error at '" << theArgVec[anArgIter] << "'\n";
      return 1;
    }
  }
  if (aGltfFilePath.IsEmpty())
  {
    theDI << "Syntax error: wrong number of arguments\n";
    return 1;
  }

  Handle(Draw_ProgressIndicator) aProgress = new Draw_ProgressIndicator(theDI, 1);

  TCollection_AsciiString anExt = aGltfFilePath;
  anExt.LowerCase();
  aNode->GlobalParameters.LengthUnit = GetLengthUnit(aDoc);
  Handle(RWGltf_Provider) aProvider =
    new RWGltf_Provider(aNode);
  Standard_Boolean aWriteStat = Standard_False;
  Handle(XSControl_WorkSession) aWS = XSDRAW::Session();
  if (!aDoc.IsNull())
  {
    aWriteStat = aProvider->Write(aGltfFilePath, aDoc, aWS, aProgress->Start());
  }
  else if (!aShape.IsNull())
  {
    aWriteStat = aProvider->Write(aGltfFilePath, aShape, aWS, aProgress->Start());
  }
  if (!aWriteStat)
  {
    theDI << "Cannot write any relevant data to the GLTF file\n";
    return 1;
  }
  return 0;
}

//=======================================================================
//function : InitCommands
//purpose  :
//=======================================================================
void XSDRAWGLTF::InitCommands(Draw_Interpretor& theCommands)
{
  const char* g = "XSTEP-STL/VRML";  // Step transfer file commands
  //XSDRAW::LoadDraw(theCommands);

  theCommands.Add("ReadGltf",
                  "ReadGltf Doc file [-parallel {on|off}] [-listExternalFiles] [-noCreateDoc] [-doublePrecision {on|off}] [-assetInfo]"
                  "\n\t\t: Read glTF file into XDE document."
                  "\n\t\t:   -listExternalFiles do not read mesh and only list external files"
                  "\n\t\t:   -noCreateDoc read into existing XDE document"
                  "\n\t\t:   -doublePrecision store triangulation with double or single floating point"
                  "\n\t\t:                    precision (single by default)"
                  "\n\t\t:   -skipLateLoading data loading is skipped and can be performed later"
                  "\n\t\t:                    (false by default)"
                  "\n\t\t:   -keepLate data is loaded into itself with preservation of information"
                  "\n\t\t:             about deferred storage to load/unload this data later."
                  "\n\t\t:   -allScenes load all scenes defined in the document instead of default one (false by default)"
                  "\n\t\t:   -toPrintDebugInfo print additional debug information during data reading"
                  "\n\t\t:   -assetInfo print asset information",
                  __FILE__, ReadGltf, g);
  theCommands.Add("readgltf",
                  "readgltf shape file"
                  "\n\t\t: Same as ReadGltf but reads glTF file into a shape instead of a document.",
                  __FILE__, ReadGltf, g);
  theCommands.Add("WriteGltf",
                  "WriteGltf Doc file [-trsfFormat {compact|TRS|mat4}]=compact"
                  "\n\t\t:            [-systemCoordSys {Zup|Yup}]=Zup"
                  "\n\t\t:            [-comments Text] [-author Name]"
                  "\n\t\t:            [-forceUVExport]=0 [-texturesSeparate]=0 [-mergeFaces]=0 [-splitIndices16]=0"
                  "\n\t\t:            [-nodeNameFormat {empty|product|instance|instOrProd|prodOrInst|prodAndInst|verbose}]=instOrProd"
                  "\n\t\t:            [-meshNameFormat {empty|product|instance|instOrProd|prodOrInst|prodAndInst|verbose}]=product"
                  "\n\t\t:            [-draco]=0 [-compressionLevel {0-10}]=7 [-quantizePositionBits Value]=14 [-quantizeNormalBits Value]=10"
                  "\n\t\t:            [-quantizeTexcoordBits Value]=12 [-quantizeColorBits Value]=8 [-quantizeGenericBits Value]=12"
                  "\n\t\t:            [-unifiedQuantization]=0 [-parallel]=0"
                  "\n\t\t: Write XDE document into glTF file."
                  "\n\t\t:   -trsfFormat       preferred transformation format"
                  "\n\t\t:   -systemCoordSys   system coordinate system; Zup when not specified"
                  "\n\t\t:   -mergeFaces       merge Faces within the same Mesh"
                  "\n\t\t:   -splitIndices16   split Faces to keep 16-bit indices when -mergeFaces is enabled"
                  "\n\t\t:   -forceUVExport    always export UV coordinates"
                  "\n\t\t:   -texturesSeparate write textures to separate files"
                  "\n\t\t:   -nodeNameFormat   name format for Nodes"
                  "\n\t\t:   -meshNameFormat   name format for Meshes"
                  "\n\t\t:   -draco            use Draco compression 3D geometric meshes"
                  "\n\t\t:   -compressionLevel draco compression level [0-10] (by default 7), a value of 0 will apply sequential encoding and preserve face order"
                  "\n\t\t:   -quantizePositionBits quantization bits for position attribute when using Draco compression (by default 14)"
                  "\n\t\t:   -quantizeNormalBits   quantization bits for normal attribute when using Draco compression (by default 10)"
                  "\n\t\t:   -quantizeTexcoordBits quantization bits for texture coordinate attribute when using Draco compression (by default 12)"
                  "\n\t\t:   -quantizeColorBits    quantization bits for color attribute when using Draco compression (by default 8)"
                  "\n\t\t:   -quantizeGenericBits  quantization bits for skinning attribute (joint indices and joint weights)"
                  "\n                        and custom attributes when using Draco compression (by default 12)"
                  "\n\t\t:   -unifiedQuantization  quantization is applied on each primitive separately if this option is false"
                  "\n\t\t:   -parallel             use multithreading for Draco compression",
                  __FILE__, WriteGltf, g);
  theCommands.Add("writegltf",
                  "writegltf shape file",
                  __FILE__, WriteGltf, g);
}

//=======================================================================
//function : Factory
//purpose  :
//=======================================================================
void XSDRAWGLTF::Factory(Draw_Interpretor& theDI)
{
  XSDRAWIGES::InitSelect();
  XSDRAWIGES::InitToBRep(theDI);
  XSDRAWIGES::InitFromBRep(theDI);
  XSDRAWSTEP::InitCommands(theDI);
  XSDRAWGLTF::InitCommands(theDI);
  XSDRAW::LoadDraw(theDI);
#ifdef OCCT_DEBUG
  theDI << "Draw Plugin : All TKXSDRAW commands are loaded\n";
#endif
}
