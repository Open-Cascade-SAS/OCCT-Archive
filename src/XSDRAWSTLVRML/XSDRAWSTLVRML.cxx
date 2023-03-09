// Created on: 2000-05-30
// Created by: Sergey MOZOKHIN
// Copyright (c) 2000-2014 OPEN CASCADE SAS
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

#include <XSDRAWSTLVRML.hxx>

#include <AIS_InteractiveContext.hxx>
#include <Aspect_TypeOfMarker.hxx>
#include <Bnd_Box.hxx>
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
#include <Quantity_HArray1OfColor.hxx>
#include <Quantity_NameOfColor.hxx>
#include <RWGltf_DracoParameters.hxx>
#include <RWGltf_Provider.hxx>
#include <RWMesh_FaceIterator.hxx>
#include <RWObj_ConfigurationNode.hxx>
#include <RWObj_Provider.hxx>
#include <RWPly_ConfigurationNode.hxx>
#include <RWPly_PlyWriterContext.hxx>
#include <RWPly_Provider.hxx>
#include <RWStl.hxx>
#include <RWStl_ConfigurationNode.hxx>
#include <RWStl_Provider.hxx>
#include <TColgp_SequenceOfXYZ.hxx>
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
#include <VrmlAPI_Writer.hxx>
#include <VrmlData_DataMapOfShapeAppearance.hxx>
#include <VrmlData_Scene.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_ShapeTool.hxx>
#include <XCAFPrs_DocumentExplorer.hxx>
#include <XSAlgo.hxx>
#include <XSAlgo_AlgoContainer.hxx>
#include <XSDRAW.hxx>
#include <XSDRAWIGES.hxx>
#include <XSDRAWSTEP.hxx>
#include <XSDRAWSTLVRML_DataSource.hxx>
#include <XSDRAWSTLVRML_DataSource3D.hxx>
#include <XSDRAWSTLVRML_DrawableMesh.hxx>

extern Standard_Boolean VDisplayAISObject(const TCollection_AsciiString& theName,
                                          const Handle(AIS_InteractiveObject)& theAISObj,
                                          Standard_Boolean theReplaceIfExists = Standard_True);

namespace
{
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

  //=============================================================================
  //function : parseNameFormat
  //purpose  : Parse RWMesh_NameFormat enumeration.
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
  //purpose  : Parse RWMesh_CoordinateSystem enumeration.
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
  Standard_Boolean toPrintAssetInfo = Standard_False;
  Standard_Boolean toListExternalFiles = Standard_False;
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
    else if (anArgCase == "-listexternalfiles"
             || anArgCase == "-listexternals"
             || anArgCase == "-listexternal"
             || anArgCase == "-external"
             || anArgCase == "-externalfiles")
    {
      toListExternalFiles = Draw::ParseOnOffIterator(theNbArgs, theArgVec, anArgIter);
    }
    else if (anArgCase == "-assetinfo"
             || anArgCase == "-metadata")
    {
      toPrintAssetInfo = Draw::ParseOnOffIterator(theNbArgs, theArgVec, anArgIter);
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

  aNode->GlobalParameters.LengthUnit = getLengthUnit(aDoc);
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
  bool isFirstLine = true;
  if (toPrintAssetInfo)
  {
    for (TColStd_IndexedDataMapOfStringString::Iterator aKeyIter(aProvider->GetMetadata()); aKeyIter.More(); aKeyIter.Next())
    {
      if (!isFirstLine)
      {
        theDI << "\n";
      }
      isFirstLine = false;
      theDI << aKeyIter.Key() << ": " << aKeyIter.Value();
    }
  }
  if (toListExternalFiles)
  {
    if (!isFirstLine)
    {
      theDI << "\n";
    }
    for (NCollection_IndexedMap<TCollection_AsciiString>::Iterator aFileIter(aProvider->GetExternalFiles()); aFileIter.More(); aFileIter.Next())
    {
      theDI << "\"" << aFileIter.Value() << "\" ";
    }
  }
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
    else if (aDoc.IsNull() && aShape.IsNull())
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
  aNode->GlobalParameters.LengthUnit = getLengthUnit(aDoc);
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

//=============================================================================
//function : writestl
//purpose  :
//=============================================================================
static Standard_Integer writestl(Draw_Interpretor& theDI,
                                 Standard_Integer theNbArgs,
                                 const char** theArgVec)
{
  if (theNbArgs < 3 || theNbArgs > 4)
  {
    theDI << "Use: " << theArgVec[0]
      << " shape file [ascii/binary (0/1) : 1 by default]\n";
    return 1;
  }
  TopoDS_Shape aShape = DBRep::Get(theArgVec[1]);
  Standard_Boolean isASCIIMode = Standard_False;
  if (theNbArgs == 4)
  {
    isASCIIMode = (Draw::Atoi(theArgVec[3]) == 0);
  }
  Handle(RWStl_ConfigurationNode) aNode = new RWStl_ConfigurationNode();
  aNode->GlobalParameters.LengthUnit = getLengthUnit();
  Handle(RWStl_Provider) aProvider = new RWStl_Provider(aNode);
  aNode->InternalParameters.WriteAscii = isASCIIMode;
  Handle(XSControl_WorkSession) aWS = XSDRAW::Session();
  Handle(Draw_ProgressIndicator) aProgress = new Draw_ProgressIndicator(theDI);
  if (!aProvider->Write(theArgVec[2], aShape, aWS, aProgress->Start()))
  {
    theDI << "Error: Mesh writing has been failed.\n";
  }
  return 0;
}

//=============================================================================
//function : readstl
//purpose  : Reads stl file
//=============================================================================
static Standard_Integer readstl(Draw_Interpretor& theDI,
                                Standard_Integer theArgc,
                                const char** theArgv)
{
  TCollection_AsciiString aShapeName, aFilePath;
  Handle(RWStl_ConfigurationNode) aNode = new RWStl_ConfigurationNode();
  aNode->GlobalParameters.LengthUnit = getLengthUnit();
  for (Standard_Integer anArgIter = 1; anArgIter < theArgc; ++anArgIter)
  {
    TCollection_AsciiString anArg(theArgv[anArgIter]);
    anArg.LowerCase();
    if (aShapeName.IsEmpty())
    {
      aShapeName = theArgv[anArgIter];
    }
    else if (aFilePath.IsEmpty())
    {
      aFilePath = theArgv[anArgIter];
    }
    else if (anArg == "-brep")
    {
      Standard_Boolean toCreateCompOfTris = Standard_True;
      if (anArgIter + 1 < theArgc
          && Draw::ParseOnOff(theArgv[anArgIter + 1], toCreateCompOfTris))
      {
        ++anArgIter;
      }
      if (toCreateCompOfTris)
      {
        aNode->InternalParameters.ReadShapeType =
          RWStl_ConfigurationNode::ReadMode_ShapeType_CompShape;
      }
    }
    else if (anArg == "-multi")
    {
      Standard_Boolean anIsMulti = Standard_True;
      if (anArgIter + 1 < theArgc
          && Draw::ParseOnOff(theArgv[anArgIter + 1], anIsMulti))
      {
        ++anArgIter;
      }
      if (anIsMulti)
      {
        aNode->InternalParameters.ReadShapeType =
          RWStl_ConfigurationNode::ReadMode_ShapeType_MultiMesh;
      }
    }
    else if (anArg == "-mergeangle"
             || anArg == "-smoothangle"
             || anArg == "-nomergeangle"
             || anArg == "-nosmoothangle")
    {
      if (anArg.StartsWith("-no"))
      {
        aNode->InternalParameters.ReadMergeAngle = M_PI / 2.0;
      }
      else
      {
        aNode->InternalParameters.ReadMergeAngle = M_PI / 4.0;
        if (anArgIter + 1 < theArgc
            && Draw::ParseReal(theArgv[anArgIter + 1], aNode->InternalParameters.ReadMergeAngle))
        {
          if (aNode->InternalParameters.ReadMergeAngle < 0.0 ||
              aNode->InternalParameters.ReadMergeAngle > 90.0)
          {
            theDI << "Syntax error: angle should be within [0,90] range";
            return 1;
          }
          ++anArgIter;
        }
      }
    }
    else
    {
      theDI << "Syntax error: unknown argument '" << theArgv[anArgIter] << "'\n";
      return 1;
    }
  }
  if (aFilePath.IsEmpty())
  {
    theDI << "Syntax error: not enough arguments\n";
    return 1;
  }
  Handle(Draw_ProgressIndicator) aProgress = new Draw_ProgressIndicator(theDI, 1);
  Handle(RWStl_Provider) aProvider = new RWStl_Provider(aNode);
  TopoDS_Shape aShape;
  Handle(XSControl_WorkSession) aWS = XSDRAW::Session();
  if (!aProvider->Read(aFilePath, aShape, aWS, aProgress->Start()))
  {
    theDI << "Cannot read any relevant data from the STL file\n";
    return 1;
  }
  DBRep::Set(aShapeName.ToCString(), aShape);
  return 0;
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
  aNode->GlobalParameters.LengthUnit = getLengthUnit();
  Standard_Boolean toUseExistingDoc = Standard_False;
  Standard_Boolean toListExternalFiles = Standard_False;
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
      aNode->InternalParameters.ReadCreateSingle = Standard_True;
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
    else if (anArgCase == "-listexternalfiles"
          || anArgCase == "-listexternals"
          || anArgCase == "-listexternal"
          || anArgCase == "-external"
          || anArgCase == "-externalfiles")
    {
      toListExternalFiles = Standard_True;
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

  if (toListExternalFiles)
  {
    for (NCollection_IndexedMap<TCollection_AsciiString>::Iterator aFileIter(aProvider->GetExternalFiles()); aFileIter.More(); aFileIter.Next())
    {
      theDI << "\"" << aFileIter.Value() << "\" ";
    }
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
  aNode->GlobalParameters.LengthUnit = getLengthUnit();
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
    else if (aDoc.IsNull() && aShape.IsNull())
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
  Handle(XSControl_WorkSession) aWS = XSDRAW::Session();
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

static Standard_Integer writevrml
(Draw_Interpretor& di, Standard_Integer argc, const char** argv)
{
  if (argc < 3 || argc > 5)
  {
    di << "wrong number of parameters\n";
    return 0;
  }

  TopoDS_Shape aShape = DBRep::Get(argv[1]);

  // Get the optional parameters
  Standard_Integer aVersion = 2;
  Standard_Integer aType = 1;
  if (argc >= 4)
  {
    aVersion = Draw::Atoi(argv[3]);
    if (argc == 5)
      aType = Draw::Atoi(argv[4]);
  }

  // Bound parameters
  aVersion = Max(1, aVersion);
  aVersion = Min(2, aVersion);
  aType = Max(0, aType);
  aType = Min(2, aType);

  VrmlAPI_Writer writer;

  switch (aType)
  {
    case 0: writer.SetRepresentation(VrmlAPI_ShadedRepresentation); break;
    case 1: writer.SetRepresentation(VrmlAPI_WireFrameRepresentation); break;
    case 2: writer.SetRepresentation(VrmlAPI_BothRepresentation); break;
  }

  if (!writer.Write(aShape, argv[2], aVersion))
  {
    di << "Error: File " << argv[2] << " was not written\n";
  }

  return 0;
}

//=======================================================================
//function : loadvrml
//purpose  :
//=======================================================================

static Standard_Integer loadvrml
(Draw_Interpretor& di, Standard_Integer argc, const char** argv)
{
  if (argc<3) di << "wrong number of parameters"    << "\n";
  else {
    TopoDS_Shape aShape ;
    VrmlData_DataMapOfShapeAppearance aShapeAppMap;

    //-----------------------------------------------------------
    std::filebuf aFic;
    std::istream aStream (&aFic);

    if (aFic.open(argv[2], std::ios::in)) {

      // Get path of the VRML file.
      OSD_Path aPath(argv[2]);
      TCollection_AsciiString aVrmlDir(".");
      TCollection_AsciiString aDisk = aPath.Disk();
      TCollection_AsciiString aTrek = aPath.Trek();
      if (!aTrek.IsEmpty())
      {
        if (!aDisk.IsEmpty())
          aVrmlDir = aDisk;
        else
          aVrmlDir.Clear();
        aTrek.ChangeAll('|', '/');
        aVrmlDir += aTrek;
      }

      VrmlData_Scene aScene;
      XSAlgo::AlgoContainer()->PrepareForTransfer(); // update unit info
      Standard_Real anOCCUnitMM = UnitsMethods::GetCasCadeLengthUnit();
      aScene.SetLinearScale(1000. / anOCCUnitMM);

      aScene.SetVrmlDir (aVrmlDir);
      aScene << aStream;
      const char * aStr = 0L;
      switch (aScene.Status()) {

      case VrmlData_StatusOK:
        {
          aShape = aScene.GetShape(aShapeAppMap);
          break;
        }
      case VrmlData_EmptyData:            aStr = "EmptyData"; break;
      case VrmlData_UnrecoverableError:   aStr = "UnrecoverableError"; break;
      case VrmlData_GeneralError:         aStr = "GeneralError"; break;
      case VrmlData_EndOfFile:            aStr = "EndOfFile"; break;
      case VrmlData_NotVrmlFile:          aStr = "NotVrmlFile"; break;
      case VrmlData_CannotOpenFile:       aStr = "CannotOpenFile"; break;
      case VrmlData_VrmlFormatError:      aStr = "VrmlFormatError"; break;
      case VrmlData_NumericInputError:    aStr = "NumericInputError"; break;
      case VrmlData_IrrelevantNumber:     aStr = "IrrelevantNumber"; break;
      case VrmlData_BooleanInputError:    aStr = "BooleanInputError"; break;
      case VrmlData_StringInputError:     aStr = "StringInputError"; break;
      case VrmlData_NodeNameUnknown:      aStr = "NodeNameUnknown"; break;
      case VrmlData_NonPositiveSize:      aStr = "NonPositiveSize"; break;
      case VrmlData_ReadUnknownNode:      aStr = "ReadUnknownNode"; break;
      case VrmlData_NonSupportedFeature:  aStr = "NonSupportedFeature"; break;
      case VrmlData_OutputStreamUndefined:aStr = "OutputStreamUndefined"; break;
      case VrmlData_NotImplemented:       aStr = "NotImplemented"; break;
      default:
        break;
      }
      if (aStr) {
        di << " ++ VRML Error: " << aStr << " in line "
          << aScene.GetLineError() << "\n";
      }
      else {
        DBRep::Set(argv[1],aShape);
      }
    }
    else {
      di << "cannot open file\n";
    }


    //-----------------------------------------------------------
  }
  return 0;
}

//-----------------------------------------------------------------------------
static Standard_Integer createmesh
(Draw_Interpretor& di, Standard_Integer argc, const char** argv )
{
  if (argc<3)
  {
    di << "Wrong number of parameters\n";
    di << "Use: " << argv[0] << " <mesh name> <stl file>\n";
    return 0;
  }

  Handle(AIS_InteractiveContext) aContext = ViewerTest::GetAISContext();
  if (aContext.IsNull())
  {
    di << "No active view. Please call 'vinit' first\n";
    return 0;
  }

  // Progress indicator
  OSD_Path aFile( argv[2] );
  Handle(Draw_ProgressIndicator) aProgress = new Draw_ProgressIndicator (di, 1);
  Handle(Poly_Triangulation) aSTLMesh = RWStl::ReadFile (aFile, aProgress->Start());

  di << "Reading OK...\n";
  Handle( XSDRAWSTLVRML_DataSource ) aDS = new XSDRAWSTLVRML_DataSource( aSTLMesh );
  di << "Data source is created successful\n";
  Handle( MeshVS_Mesh ) aMesh = new MeshVS_Mesh();
  di << "MeshVS_Mesh is created successful\n";

  aMesh->SetDataSource( aDS );
  aMesh->AddBuilder( new MeshVS_MeshPrsBuilder( aMesh.operator->() ), Standard_True );

  aMesh->GetDrawer()->SetColor( MeshVS_DA_EdgeColor, Quantity_NOC_YELLOW );

  // Hide all nodes by default
  Handle(TColStd_HPackedMapOfInteger) aNodes = new TColStd_HPackedMapOfInteger();
  const Standard_Integer aLen = aSTLMesh->NbNodes();
  for ( Standard_Integer anIndex = 1; anIndex <= aLen; anIndex++ )
    aNodes->ChangeMap().Add( anIndex );
  aMesh->SetHiddenNodes( aNodes );
  aMesh->SetSelectableNodes ( aNodes );

  VDisplayAISObject(argv[1], aMesh);
  aContext->Deactivate( aMesh );

  Draw::Set( argv[1], new XSDRAWSTLVRML_DrawableMesh( aMesh ) );
  Handle( V3d_View ) aView = ViewerTest::CurrentView();
  if ( !aView.IsNull() )
    aView->FitAll();

  return 0;
}
//-----------------------------------------------------------------------------

static Standard_Integer create3d
(Draw_Interpretor& di, Standard_Integer argc, const char** argv )
{
  if (argc<2)
  {
    di << "Wrong number of parameters\n";
    di << "Use: " << argv[0] << " <mesh name>\n";
    return 0;
  }

  Handle(AIS_InteractiveContext) aContext = ViewerTest::GetAISContext();
  if (aContext.IsNull())
  {
    di << "No active view. Please call 'vinit' first\n";
    return 0;
  }

  Handle( XSDRAWSTLVRML_DataSource3D ) aDS = new XSDRAWSTLVRML_DataSource3D();
  di << "Data source is created successful\n";
  Handle( MeshVS_Mesh ) aMesh = new MeshVS_Mesh();
  di << "MeshVS_Mesh is created successful\n";

  aMesh->SetDataSource( aDS );
  aMesh->AddBuilder( new MeshVS_MeshPrsBuilder( aMesh.operator->() ), Standard_True );

  aMesh->GetDrawer()->SetColor( MeshVS_DA_EdgeColor, Quantity_NOC_YELLOW );

  // Hide all nodes by default
  Handle(TColStd_HPackedMapOfInteger) aNodes = new TColStd_HPackedMapOfInteger();
  Standard_Integer aLen = aDS->GetAllNodes().Extent();
  for ( Standard_Integer anIndex = 1; anIndex <= aLen; anIndex++ )
    aNodes->ChangeMap().Add( anIndex );
  aMesh->SetHiddenNodes( aNodes );
  aMesh->SetSelectableNodes ( aNodes );

  VDisplayAISObject(argv[1], aMesh);
  aContext->Deactivate( aMesh );

  Draw::Set( argv[1], new XSDRAWSTLVRML_DrawableMesh( aMesh ) );
  Handle( V3d_View ) aView = ViewerTest::CurrentView();
  if ( !aView.IsNull() )
    aView->FitAll();

  return 0;
}

Handle( MeshVS_Mesh ) getMesh( const char* theName, Draw_Interpretor& di)
{
  Handle( XSDRAWSTLVRML_DrawableMesh ) aDrawMesh =
    Handle( XSDRAWSTLVRML_DrawableMesh )::DownCast( Draw::Get( theName ) );

  if( aDrawMesh.IsNull() )
  {
    di << "There is no such object\n";
    return NULL;
  }
  else
  {
    Handle( MeshVS_Mesh ) aMesh = aDrawMesh->GetMesh();
    if( aMesh.IsNull() )
    {
      di << "There is invalid mesh\n";
      return NULL;
    }
    else
      return aMesh;
  }
}

//-----------------------------------------------------------------------------
static Standard_Integer setcolor
(Draw_Interpretor& di, Standard_Integer argc, const char** argv, Standard_Integer theParam )
{
  if (argc<5)
    di << "Wrong number of parameters\n";
  else
  {
    Handle( MeshVS_Mesh ) aMesh = getMesh( argv[1], di );
    if( !aMesh.IsNull() )
    {
      Standard_Real aRed = Draw::Atof (argv[2]);
      Standard_Real aGreen = Draw::Atof (argv[3]);
      Standard_Real aBlue = Draw::Atof (argv[4]);
      aMesh->GetDrawer()->SetColor( (MeshVS_DrawerAttribute)theParam,
                                    Quantity_Color( aRed, aGreen, aBlue, Quantity_TOC_RGB ) );

      Handle( AIS_InteractiveContext ) aContext = ViewerTest::GetAISContext();

      if( aContext.IsNull() )
        di << "The context is null\n";
      else
        aContext->Redisplay (aMesh, Standard_True);
    }
  }
  return 0;
}
//-----------------------------------------------------------------------------
static Standard_Integer meshcolor
(Draw_Interpretor& theInterp, Standard_Integer argc, const char** argv )
{
  return setcolor( theInterp, argc, argv, MeshVS_DA_InteriorColor );
}
//-----------------------------------------------------------------------------
static Standard_Integer linecolor
(Draw_Interpretor& theInterp, Standard_Integer argc, const char** argv )
{
  return setcolor( theInterp, argc, argv, MeshVS_DA_EdgeColor );
}
//-----------------------------------------------------------------------------
static Standard_Integer meshmat
(Draw_Interpretor& di, Standard_Integer argc, const char** argv )
{
  if (argc<3)
    di << "Wrong number of parameters\n";
  else
  {
    Handle( MeshVS_Mesh ) aMesh = getMesh( argv[1], di );
    if( !aMesh.IsNull() )
    {
      Standard_Integer aMaterial = Draw::Atoi (argv[2]);

      Graphic3d_MaterialAspect aMatAsp =
        (Graphic3d_MaterialAspect)(Graphic3d_NameOfMaterial)aMaterial;

      if (argc == 4)
      {
        Standard_Real aTransparency = Draw::Atof(argv[3]);
        aMatAsp.SetTransparency (Standard_ShortReal (aTransparency));
      }
      aMesh->GetDrawer()->SetMaterial( MeshVS_DA_FrontMaterial, aMatAsp );
      aMesh->GetDrawer()->SetMaterial( MeshVS_DA_BackMaterial, aMatAsp );

      Handle( AIS_InteractiveContext ) aContext = ViewerTest::GetAISContext();

      if( aContext.IsNull() )
        di << "The context is null\n";
      else
        aContext->Redisplay (aMesh, Standard_True);
    }
  }
  return 0;
}
//-----------------------------------------------------------------------------
static Standard_Integer shrink
(Draw_Interpretor& di, Standard_Integer argc, const char** argv )
{
  if (argc<3)
    di << "Wrong number of parameters\n";
  else
  {
    Handle( MeshVS_Mesh ) aMesh = getMesh( argv[1], di );
    if( !aMesh.IsNull() )
    {
      Standard_Real aShrinkCoeff = Draw::Atof (argv[2]);
      aMesh->GetDrawer()->SetDouble( MeshVS_DA_ShrinkCoeff, aShrinkCoeff );

      Handle( AIS_InteractiveContext ) aContext = ViewerTest::GetAISContext();

      if( aContext.IsNull() )
        di << "The context is null\n";
      else
        aContext->Redisplay (aMesh, Standard_True);
    }
  }
  return 0;
}

//-----------------------------------------------------------------------------
static Standard_Integer closed (Draw_Interpretor& theDI, Standard_Integer theArgc, const char** theArgv)
{
  if (theArgc < 3)
  {
    theDI << "Wrong number of parameters.\n";
  }
  else
  {
    Handle(MeshVS_Mesh) aMesh = getMesh (theArgv[1], theDI);
    if (!aMesh.IsNull())
    {
      Standard_Boolean aFlag = Draw::Atoi (theArgv[2]) != 0;
      aMesh->GetDrawer()->SetBoolean (MeshVS_DA_SupressBackFaces, aFlag);

      Handle( AIS_InteractiveContext ) aContext = ViewerTest::GetAISContext();
      if (aContext.IsNull())
      {
        theDI << "The context is null\n";
      }
      else
      {
        aContext->Redisplay (aMesh, Standard_True);
      }
    }
  }
  return 0;
}

//-----------------------------------------------------------------------------

static Standard_Integer mdisplay
(Draw_Interpretor& di, Standard_Integer argc, const char** argv )
{
  if (argc<2)
    di << "Wrong number of parameters\n";
  else
  {
    Handle( MeshVS_Mesh ) aMesh = getMesh( argv[1], di );
    if( !aMesh.IsNull() )
    {
      Handle( AIS_InteractiveContext ) aContext = ViewerTest::GetAISContext();

      if( aContext.IsNull() )
        di << "The context is null\n";
      else
      {
        aContext->Display (aMesh, Standard_True);
      }
    }
  }
  return 0;
}
//-----------------------------------------------------------------------------
static Standard_Integer merase
(Draw_Interpretor& di, Standard_Integer argc, const char** argv )
{
  if (argc<2)
    di << "Wrong number of parameters\n";
  else
  {
    Handle( MeshVS_Mesh ) aMesh = getMesh( argv[1], di );
    if( !aMesh.IsNull() )
    {
      Handle( AIS_InteractiveContext ) aContext = ViewerTest::GetAISContext();

      if( aContext.IsNull() )
        di << "The context is null\n";
      else
      {
        aContext->Erase (aMesh, Standard_True);
      }
    }
    else
      di << "Mesh is null\n";
  }
  return 0;
}
//-----------------------------------------------------------------------------
static Standard_Integer hidesel
(Draw_Interpretor& di, Standard_Integer argc, const char** argv )
{
  if (argc<2)
  {
    di << "Wrong number of parameters\n";
    di << "Use: " << argv[0] << " <mesh name>\n";
    return 0;
  }

  Handle( AIS_InteractiveContext ) aContext = ViewerTest::GetAISContext();
  Handle( MeshVS_Mesh ) aMesh = getMesh( argv[1], di );
  if( aMesh.IsNull() )
  {
    di << "The mesh is invalid\n";
    return 0;
  }

  if( aContext.IsNull() )
    di << "The context is null\n";
  else
  {
    Handle(TColStd_HPackedMapOfInteger) aHiddenNodes = aMesh->GetHiddenNodes();
    if (aHiddenNodes.IsNull())
    {
      aHiddenNodes = new TColStd_HPackedMapOfInteger();
    }
    Handle(TColStd_HPackedMapOfInteger) aHiddenElements = aMesh->GetHiddenElems();
    if (aHiddenElements.IsNull())
    {
      aHiddenElements = new TColStd_HPackedMapOfInteger();
    }
    for( aContext->InitSelected(); aContext->MoreSelected(); aContext->NextSelected() )
    {
      Handle( MeshVS_MeshEntityOwner ) anOwner =
        Handle( MeshVS_MeshEntityOwner )::DownCast( aContext->SelectedOwner() );
      if( !anOwner.IsNull() )
      {
        if( anOwner->Type()==MeshVS_ET_Node )
        {
          aHiddenNodes->ChangeMap().Add( anOwner->ID() );
        }
        else
        {
          aHiddenElements->ChangeMap().Add( anOwner->ID() );
        }
      }
    }
    aContext->ClearSelected (Standard_False);
    aMesh->SetHiddenNodes( aHiddenNodes );
    aMesh->SetHiddenElems( aHiddenElements );
    aContext->Redisplay (aMesh, Standard_True);
  }

  return 0;
}
//-----------------------------------------------------------------------------
static Standard_Integer showonly
(Draw_Interpretor& di, Standard_Integer argc, const char** argv )
{
  if (argc<2)
  {
    di << "Wrong number of parameters\n";
    di << "Use: " << argv[0] << " <mesh name>\n";
    return 0;
  }


  Handle( AIS_InteractiveContext ) aContext = ViewerTest::GetAISContext();
  Handle( MeshVS_Mesh ) aMesh = getMesh( argv[1], di );
  if( aMesh.IsNull() )
  {
    di << "The mesh is invalid\n";
    return 0;
  }

  if( aContext.IsNull() )
    di << "The context is null\n";
  else
  {
    Handle(TColStd_HPackedMapOfInteger) aHiddenNodes =
      new TColStd_HPackedMapOfInteger(aMesh->GetDataSource()->GetAllNodes());
    Handle(TColStd_HPackedMapOfInteger) aHiddenElements =
      new TColStd_HPackedMapOfInteger(aMesh->GetDataSource()->GetAllElements());
    for( aContext->InitSelected(); aContext->MoreSelected(); aContext->NextSelected() )
    {
      Handle( MeshVS_MeshEntityOwner ) anOwner =
        Handle( MeshVS_MeshEntityOwner )::DownCast( aContext->SelectedOwner() );
      if( !anOwner.IsNull() )
      {
        if( anOwner->Type() == MeshVS_ET_Node )
        {
          aHiddenNodes->ChangeMap().Remove( anOwner->ID() );
        }
        else
        {
          aHiddenElements->ChangeMap().Remove( anOwner->ID() );
        }
      }
    }
    aMesh->SetHiddenNodes( aHiddenNodes );
    aMesh->SetHiddenElems( aHiddenElements );
    aContext->Redisplay (aMesh, Standard_True);
  }

  return 0;
}
//-----------------------------------------------------------------------------
static Standard_Integer showall
(Draw_Interpretor& di, Standard_Integer argc, const char** argv )
{
  if (argc<2)
  {
    di << "Wrong number of parameters\n";
    di << "Use: " << argv[0] << " <mesh name>\n";
    return 0;
  }

  Handle( AIS_InteractiveContext ) aContext = ViewerTest::GetAISContext();
  Handle( MeshVS_Mesh ) aMesh = getMesh( argv[1], di );
  if( aMesh.IsNull() )
  {
    di << "The mesh is invalid\n";
    return 0;
  }

  if( aContext.IsNull() )
    di << "The context is null\n";
  else
  {
    aMesh->SetHiddenNodes( new TColStd_HPackedMapOfInteger() );
    aMesh->SetHiddenElems( new TColStd_HPackedMapOfInteger() );
    aContext->Redisplay (aMesh, Standard_True);
  }

  return 0;
}

//-----------------------------------------------------------------------------
static Standard_Integer meshcolors( Draw_Interpretor& di,
                                    Standard_Integer argc,
                                    const char** argv )
{
  try
  {
    OCC_CATCH_SIGNALS
      if ( argc < 4 )
      {
        di << "Wrong number of parameters\n";
        di << "Use : meshcolors <mesh name> <mode> <isreflect>\n";
        di << "mode : {elem1|elem2|nodal|nodaltex|none}\n";
        di << "       elem1 - different color for each element\n";
        di << "       elem2 - one color for one side\n";
        di << "       nodal - different color for each node\n";
        di << "       nodaltex - different color for each node with texture interpolation\n";
        di << "       none  - clear\n";
        di << "isreflect : {0|1} \n";

        return 0;
      }

      Handle( MeshVS_Mesh ) aMesh = getMesh( argv[ 1 ], di );

      if ( aMesh.IsNull() )
      {
        di << "Mesh not found\n";
        return 0;
      }
      Handle(AIS_InteractiveContext) anIC = ViewerTest::GetAISContext();
      if ( anIC.IsNull() )
      {
        di << "The context is null\n";
        return 0;
      }
      if( !aMesh.IsNull() )
      {
        TCollection_AsciiString aMode = TCollection_AsciiString (argv[2]);
        Quantity_Color aColor1(Quantity_NOC_BLUE1);
        Quantity_Color aColor2(Quantity_NOC_RED1);
        if( aMode.IsEqual("elem1") || aMode.IsEqual("elem2") || aMode.IsEqual("nodal") || aMode.IsEqual("nodaltex") || aMode.IsEqual("none") )
        {
          Handle(MeshVS_PrsBuilder) aTempBuilder;
          Standard_Integer aReflection = Draw::Atoi(argv[3]);

          for (Standard_Integer aCount = 0 ; aCount < aMesh->GetBuildersCount(); aCount++ ){
            aTempBuilder = aMesh->FindBuilder("MeshVS_ElementalColorPrsBuilder");
            if( !aTempBuilder.IsNull())
              aMesh->RemoveBuilderById(aTempBuilder->GetId());

            aTempBuilder = aMesh->FindBuilder("MeshVS_NodalColorPrsBuilder");
            if( !aTempBuilder.IsNull())
              aMesh->RemoveBuilderById(aTempBuilder->GetId());
          }

          if( aMode.IsEqual("elem1") || aMode.IsEqual("elem2") )
          {
            Handle(MeshVS_ElementalColorPrsBuilder) aBuilder = new MeshVS_ElementalColorPrsBuilder(
                aMesh, MeshVS_DMF_ElementalColorDataPrs | MeshVS_DMF_OCCMask );
              // Color
            const TColStd_PackedMapOfInteger& anAllElements = aMesh->GetDataSource()->GetAllElements();
            TColStd_MapIteratorOfPackedMapOfInteger anIter( anAllElements );

            if( aMode.IsEqual("elem1") )
              for ( ; anIter.More(); anIter.Next() )
              {
                Quantity_Color aColor( (Quantity_NameOfColor)( anIter.Key() % Quantity_NOC_WHITE ) );
                aBuilder->SetColor1( anIter.Key(), aColor );
              }
            else
              for ( ; anIter.More(); anIter.Next() )
                aBuilder->SetColor2( anIter.Key(), aColor1, aColor2 );

            aMesh->AddBuilder( aBuilder, Standard_True );
          }


          if( aMode.IsEqual("nodal") )
          {
            Handle(MeshVS_NodalColorPrsBuilder) aBuilder = new MeshVS_NodalColorPrsBuilder(
                aMesh, MeshVS_DMF_NodalColorDataPrs | MeshVS_DMF_OCCMask );
            aMesh->AddBuilder( aBuilder, Standard_True );

            // Color
            const TColStd_PackedMapOfInteger& anAllNodes =
              aMesh->GetDataSource()->GetAllNodes();
            TColStd_MapIteratorOfPackedMapOfInteger anIter( anAllNodes );
            for ( ; anIter.More(); anIter.Next() )
            {
              Quantity_Color aColor( (Quantity_NameOfColor)(
                anIter.Key() % Quantity_NOC_WHITE ) );
              aBuilder->SetColor( anIter.Key(), aColor );
            }
            aMesh->AddBuilder( aBuilder, Standard_True );
          }

          if(aMode.IsEqual("nodaltex"))
          {
            // assign nodal builder to the mesh
            Handle(MeshVS_NodalColorPrsBuilder) aBuilder = new MeshVS_NodalColorPrsBuilder(
                   aMesh, MeshVS_DMF_NodalColorDataPrs | MeshVS_DMF_OCCMask);
            aMesh->AddBuilder(aBuilder, Standard_True);
            aBuilder->UseTexture(Standard_True);

            // prepare color map for texture
            Aspect_SequenceOfColor aColorMap;
            aColorMap.Append((Quantity_NameOfColor) Quantity_NOC_RED);
            aColorMap.Append((Quantity_NameOfColor) Quantity_NOC_YELLOW);
            aColorMap.Append((Quantity_NameOfColor) Quantity_NOC_BLUE1);

            // prepare scale map for mesh - it will be assigned to mesh as texture coordinates
            // make mesh color interpolated from minimum X coord to maximum X coord
            Handle(MeshVS_DataSource) aDataSource = aMesh->GetDataSource();
            Standard_Real aMinX, aMinY, aMinZ, aMaxX, aMaxY, aMaxZ;

            // get bounding box for calculations
            aDataSource->GetBoundingBox().Get(aMinX, aMinY, aMinZ, aMaxX, aMaxY, aMaxZ);
            Standard_Real aDelta = aMaxX - aMinX;

            // assign color scale map values (0..1) to nodes
            TColStd_DataMapOfIntegerReal aScaleMap;
            TColStd_Array1OfReal aCoords(1, 3);
            Standard_Integer     aNbNodes;
            MeshVS_EntityType    aType;

            // iterate nodes
            const TColStd_PackedMapOfInteger& anAllNodes =
                  aMesh->GetDataSource()->GetAllNodes();
            TColStd_MapIteratorOfPackedMapOfInteger anIter(anAllNodes);
            for (; anIter.More(); anIter.Next())
            {
              //get node coordinates to aCoord variable
              aDataSource->GetGeom(anIter.Key(), Standard_False, aCoords, aNbNodes, aType);

              Standard_Real aScaleValue;
              try {
                OCC_CATCH_SIGNALS
                aScaleValue = (aCoords.Value(1) - (Standard_Real) aMinX) / aDelta;
              } catch(Standard_Failure const&) {
                aScaleValue = 0;
              }

              aScaleMap.Bind(anIter.Key(), aScaleValue);
            }

            //set color map for builder and a color for invalid scale value
            aBuilder->SetColorMap(aColorMap);
            aBuilder->SetInvalidColor(Quantity_NOC_BLACK);
            aBuilder->SetTextureCoords(aScaleMap);
            aMesh->AddBuilder(aBuilder, Standard_True);
          }

          aMesh->GetDrawer()->SetBoolean (MeshVS_DA_ColorReflection, aReflection != 0);

          anIC->Redisplay (aMesh, Standard_True);
        }
        else
        {
          di << "Wrong mode name\n";
          return 0;
        }
      }
  }
  catch ( Standard_Failure const& )
  {
    di << "Error\n";
  }

  return 0;
}
//-----------------------------------------------------------------------------
static Standard_Integer meshvectors( Draw_Interpretor& di,
                                     Standard_Integer argc,
                                     const char** argv )
{
  if ( argc < 3 )
  {
    di << "Wrong number of parameters\n";
    di << "Use : meshvectors <mesh name> < -mode {elem|nodal|none} > [-maxlen len] [-color name] [-arrowpart ratio] [-issimple {1|0}]\n";
    di << "Supported mode values:\n";
    di << "       elem  - vector per element\n";
    di << "       nodal - vector per node\n";
    di << "       none  - clear\n";

    return 0;
  }

  Handle( MeshVS_Mesh ) aMesh = getMesh( argv[ 1 ], di );

  if ( aMesh.IsNull() )
  {
    di << "Mesh not found\n";
    return 0;
  }
  Handle(AIS_InteractiveContext) anIC = ViewerTest::GetAISContext();
  if ( anIC.IsNull() )
  {
    di << "The context is null\n";
    return 0;
  }

  TCollection_AsciiString aParam;
  TCollection_AsciiString aMode("none");
  Standard_Real           aMaxlen(1.0);
  Quantity_Color          aColor(Quantity_NOC_ORANGE);
  Standard_Real           anArrowPart(0.1);
  Standard_Boolean        isSimplePrs(Standard_False);

  for (Standard_Integer anIdx = 2; anIdx < argc; anIdx++)
  {
    if (!aParam.IsEmpty())
    {
      if (aParam == "-mode")
      {
        aMode       = argv[anIdx];
      }
      else if (aParam == "-maxlen")
      {
        aMaxlen     = Draw::Atof(argv[anIdx]);
      }
      else if (aParam == "-color")
      {
        if (!Quantity_Color::ColorFromName (argv[anIdx], aColor))
        {
          di << "Syntax error at " << aParam;
          return 1;
        }
      }
      else if (aParam == "-arrowpart")
      {
        anArrowPart = Draw::Atof(argv[anIdx]);
      }
      else if (aParam == "-issimple")
      {
        isSimplePrs = Draw::Atoi(argv[anIdx]) != 0;
      }
      aParam.Clear();
    }
    else if (argv[anIdx][0] == '-')
    {
      aParam = argv[anIdx];
    }
  }

  if( !aMode.IsEqual("elem") && !aMode.IsEqual("nodal") && !aMode.IsEqual("none") )
  {
    di << "Wrong mode name\n";
    return 0;
  }

  Handle(MeshVS_PrsBuilder) aTempBuilder;

  aTempBuilder = aMesh->FindBuilder("MeshVS_VectorPrsBuilder");
  if( !aTempBuilder.IsNull())
    aMesh->RemoveBuilderById(aTempBuilder->GetId());

  if( !aMode.IsEqual("none") )
  {
    Handle(MeshVS_VectorPrsBuilder) aBuilder = new MeshVS_VectorPrsBuilder( aMesh.operator->(), 
                                                                            aMaxlen,
                                                                            aColor,
                                                                            MeshVS_DMF_VectorDataPrs,
                                                                            0,
                                                                            -1,
                                                                            MeshVS_BP_Vector,
                                                                            isSimplePrs);

    Standard_Boolean anIsElement = aMode.IsEqual("elem");
    const TColStd_PackedMapOfInteger& anAllIDs = anIsElement ? aMesh->GetDataSource()->GetAllElements() :
                                                               aMesh->GetDataSource()->GetAllNodes();

    Standard_Integer aNbNodes;
    MeshVS_EntityType aEntType;

    TColStd_Array1OfReal aCoords(1, 3);
    aCoords.Init (0.);
    TColStd_MapIteratorOfPackedMapOfInteger anIter( anAllIDs );
    for ( ; anIter.More(); anIter.Next() )
    {
      Standard_Boolean IsValidData = Standard_False; 
      if (anIsElement) {
        aMesh->GetDataSource()->GetGeomType(anIter.Key(), anIsElement, aEntType);
        if (aEntType == MeshVS_ET_Face)
          IsValidData = aMesh->GetDataSource()->GetNormal(anIter.Key(), 3, aCoords.ChangeValue(1), aCoords.ChangeValue(2), aCoords.ChangeValue(3));
      } else
        IsValidData = aMesh->GetDataSource()->GetGeom(anIter.Key(), Standard_False, aCoords, aNbNodes, aEntType);

      gp_Vec aNorm;
      if(IsValidData)
      { 
        aNorm = gp_Vec(aCoords.Value(1), aCoords.Value(2), aCoords.Value(3));
        if(aNorm.Magnitude() < gp::Resolution())
        {
          aNorm = gp_Vec(0,0,1); //method GetGeom(...) returns coordinates of nodes
        }
      }
      else
      {
        aNorm = gp_Vec(0,0,1);
      }
      aBuilder->SetVector(anIsElement, anIter.Key(), aNorm.Normalized());
    }

    aMesh->AddBuilder( aBuilder, Standard_False );
    aMesh->GetDrawer()->SetDouble ( MeshVS_DA_VectorArrowPart, anArrowPart );
  }

  anIC->Redisplay (aMesh, Standard_True);

  return 0;
}
//-----------------------------------------------------------------------------

static Standard_Integer meshtext( Draw_Interpretor& di,
                                  Standard_Integer argc,
                                  const char** argv )
{
  if ( argc < 2 )
  {
    di << "Wrong number of parameters\n";
    di << "Use : meshtext <mesh name>\n";
    return 0;
  }

  Handle( MeshVS_Mesh ) aMesh = getMesh( argv[ 1 ], di );

  if ( aMesh.IsNull() )
  {
    di << "Mesh not found\n";
    return 0;
  }

  Handle(AIS_InteractiveContext) anIC = ViewerTest::GetAISContext();
  if ( anIC.IsNull() )
  {
    di << "The context is null\n";
    return 0;
  }

  // Prepare triangle labels
  MeshVS_DataMapOfIntegerAsciiString aLabels;
  Standard_Integer aLen = aMesh->GetDataSource()->GetAllElements().Extent();
  for ( Standard_Integer anIndex = 1; anIndex <= aLen; anIndex++ ){
    aLabels.Bind( anIndex, TCollection_AsciiString( anIndex ) );
  }

  Handle(MeshVS_TextPrsBuilder) aTextBuilder = new MeshVS_TextPrsBuilder( aMesh.operator->(), 20., Quantity_NOC_YELLOW );
  aTextBuilder->SetTexts( Standard_True, aLabels );
  aMesh->AddBuilder( aTextBuilder );

  return 0;
}

static Standard_Integer meshdeform( Draw_Interpretor& di,
                                    Standard_Integer argc,
                                    const char** argv )
{
  if ( argc < 3 )
  {
    di << "Wrong number of parameters\n";
    di << "Use : meshdeform <mesh name> < -mode {on|off} > [-scale scalefactor]\n";
    return 0;
  }

  Handle( MeshVS_Mesh ) aMesh = getMesh( argv[ 1 ], di );

  if ( aMesh.IsNull() )
  {
    di << "Mesh not found\n";
    return 0;
  }
  Handle(AIS_InteractiveContext) anIC = ViewerTest::GetAISContext();
  if ( anIC.IsNull() )
  {
    di << "The context is null\n";
    return 0;
  }

  TCollection_AsciiString aParam;
  TCollection_AsciiString aMode("off");
  Standard_Real           aScale(1.0);

  for (Standard_Integer anIdx = 2; anIdx < argc; anIdx++)
  {
    if (!aParam.IsEmpty())
    {
      if (aParam == "-mode")
      {
        aMode = argv[anIdx];
      }
      else if (aParam == "-scale")
      {
        aScale = Draw::Atof(argv[anIdx]);
      }
      aParam.Clear();
    }
    else if (argv[anIdx][0] == '-')
    {
      aParam = argv[anIdx];
    }
  }

  if(!aMode.IsEqual("on") && !aMode.IsEqual("off"))
  {
    di << "Wrong mode name\n";
    return 0;
  }

  Handle ( MeshVS_DeformedDataSource ) aDefDS =
    new MeshVS_DeformedDataSource( aMesh->GetDataSource() , aScale );

  const TColStd_PackedMapOfInteger& anAllIDs = aMesh->GetDataSource()->GetAllNodes();

  Standard_Integer aNbNodes;
  MeshVS_EntityType aEntType;

  TColStd_MapIteratorOfPackedMapOfInteger anIter( anAllIDs );
  for ( ; anIter.More(); anIter.Next() )
  {
    TColStd_Array1OfReal aCoords(1, 3);
    aMesh->GetDataSource()->GetGeom(anIter.Key(), Standard_False, aCoords, aNbNodes, aEntType);

    gp_Vec aNorm = gp_Vec(aCoords.Value(1), aCoords.Value(2), aCoords.Value(3));
    if( !aNorm.Magnitude() )
      aNorm = gp_Vec(0,0,1);
    aDefDS->SetVector(anIter.Key(), aNorm.Normalized());
  }

  aMesh->SetDataSource(aDefDS);

  anIC->Redisplay (aMesh, Standard_False);

  Handle( V3d_View ) aView = ViewerTest::CurrentView();
  if ( !aView.IsNull() )
    aView->FitAll();

  return 0;
}

static Standard_Integer mesh_edge_width( Draw_Interpretor& di,
                                        Standard_Integer argc,
                                        const char** argv )
{
  try
  {
    OCC_CATCH_SIGNALS
      if ( argc < 3 )
      {
        di << "Wrong number of parameters\n";
        di << "Use : mesh_edge_width <mesh name> <width>\n";
        return 0;
      }

      Handle(MeshVS_Mesh) aMesh = getMesh( argv[ 1 ], di );
      if ( aMesh.IsNull() )
      {
        di << "Mesh not found\n";
        return 0;
      }

      const char* aWidthStr = argv[ 2 ];
      if ( aWidthStr == 0 || Draw::Atof( aWidthStr ) <= 0 )
      {
        di << "Width must be real value more than zero\n";
        return 0;
      }

      double aWidth = Draw::Atof( aWidthStr );

      Handle(AIS_InteractiveContext) anIC = ViewerTest::GetAISContext();
      if ( anIC.IsNull() )
      {
        di << "The context is null\n";
        return 0;
      }

      Handle(MeshVS_Drawer) aDrawer = aMesh->GetDrawer();
      if ( aDrawer.IsNull() )
      {
        di << "The drawer is null\n";
        return 0;
      }

      aDrawer->SetDouble( MeshVS_DA_EdgeWidth, aWidth );
      anIC->Redisplay (aMesh, Standard_True);
  }
  catch ( Standard_Failure const& )
  {
    di << "Error\n";
  }

  return 0;
}

//-----------------------------------------------------------------------------

static Standard_Integer meshinfo(Draw_Interpretor& di,
                                 Standard_Integer argc,
                                 const char** argv)
{
  if ( argc != 2 )
  {
    di << "Wrong number of parameters. Use : meshinfo mesh\n";
    return 0;
  }

  Handle(MeshVS_Mesh) aMesh = getMesh(argv[ 1 ], di);
  if ( aMesh.IsNull() )
  {
    di << "Mesh not found\n";
    return 0;
  }

  Handle(XSDRAWSTLVRML_DataSource) stlMeshSource = Handle(XSDRAWSTLVRML_DataSource)::DownCast(aMesh->GetDataSource());
  if (!stlMeshSource.IsNull())
  {
    const TColStd_PackedMapOfInteger& nodes = stlMeshSource->GetAllNodes();
    const TColStd_PackedMapOfInteger& tris  = stlMeshSource->GetAllElements();

    di << "Nb nodes = " << nodes.Extent() << "\n";
    di << "Nb triangles = " << tris.Extent() << "\n";
  }

  return 0;
}

//=======================================================================
//function : WritePly
//purpose  : write PLY file
//=======================================================================
static Standard_Integer WritePly(Draw_Interpretor& theDI,
                                 Standard_Integer theNbArgs,
                                 const char** theArgVec)
{
  Handle(TDocStd_Document) aDoc;
  Handle(TDocStd_Application) anApp = DDocStd::GetApplication();
  TCollection_AsciiString aShapeName, aFileName;
  Handle(RWPly_ConfigurationNode) aNode = new RWPly_ConfigurationNode();
  Standard_Real aDist = 0.0;
  Standard_Real aDens = Precision::Infinite();
  Standard_Real aTol = Precision::Confusion();
  bool isPntSet = false, isDensityPoints = false;
  for (Standard_Integer anArgIter = 1; anArgIter < theNbArgs; ++anArgIter)
  {
    TCollection_AsciiString anArg(theArgVec[anArgIter]);
    anArg.LowerCase();
    if (anArg == "-normal")
    {
      aNode->InternalParameters.WriteNormals =
        Draw::ParseOnOffIterator(theNbArgs, theArgVec, anArgIter);
    }
    else if (anArg == "-nonormal")
    {
      aNode->InternalParameters.WriteNormals =
        !Draw::ParseOnOffIterator(theNbArgs, theArgVec, anArgIter);
    }
    else if (anArg == "-color"
             || anArg == "-nocolor"
             || anArg == "-colors"
             || anArg == "-nocolors")
    {
      aNode->InternalParameters.WriteColors =
        Draw::ParseOnOffNoIterator(theNbArgs, theArgVec, anArgIter);
    }
    else if (anArg == "-uv"
             || anArg == "-nouv")
    {
      aNode->InternalParameters.WriteTexCoords =
        Draw::ParseOnOffNoIterator(theNbArgs, theArgVec, anArgIter);
    }
    else if (anArg == "-partid")
    {
      aNode->InternalParameters.WritePartId =
        Draw::ParseOnOffNoIterator(theNbArgs, theArgVec, anArgIter);
      aNode->InternalParameters.WriteFaceId =
        aNode->InternalParameters.WriteFaceId &&
        !aNode->InternalParameters.WritePartId;
    }
    else if (anArg == "-surfid"
             || anArg == "-surfaceid"
             || anArg == "-faceid")
    {
      aNode->InternalParameters.WriteFaceId =
        Draw::ParseOnOffNoIterator(theNbArgs, theArgVec, anArgIter);
      aNode->InternalParameters.WritePartId =
        aNode->InternalParameters.WritePartId &&
        !aNode->InternalParameters.WriteFaceId;
    }
    else if (anArg == "-pntset"
             || anArg == "-pntcloud"
             || anArg == "-pointset"
             || anArg == "-pointcloud"
             || anArg == "-cloud"
             || anArg == "-points")
    {
      isPntSet = Draw::ParseOnOffIterator(theNbArgs, theArgVec, anArgIter);
    }
    else if ((anArg == "-dist"
             || anArg == "-distance")
             && anArgIter + 1 < theNbArgs
             && Draw::ParseReal(theArgVec[anArgIter + 1], aDist))
    {
      ++anArgIter;
      isPntSet = true;
      if (aDist < 0.0)
      {
        theDI << "Syntax error: -distance value should be >= 0.0\n";
        return 1;
      }
      aDist = Max(aDist, Precision::Confusion());
    }
    else if ((anArg == "-dens"
             || anArg == "-density")
             && anArgIter + 1 < theNbArgs
             && Draw::ParseReal(theArgVec[anArgIter + 1], aDens))
    {
      ++anArgIter;
      isDensityPoints = Standard_True;
      isPntSet = true;
      if (aDens <= 0.0)
      {
        theDI << "Syntax error: -density value should be > 0.0\n";
        return 1;
      }
    }
    else if ((anArg == "-tol"
             || anArg == "-tolerance")
             && anArgIter + 1 < theNbArgs
             && Draw::ParseReal(theArgVec[anArgIter + 1], aTol))
    {
      ++anArgIter;
      isPntSet = true;
      if (aTol < Precision::Confusion())
      {
        theDI << "Syntax error: -tol value should be >= "
          << Precision::Confusion() << "\n";
        return 1;
      }
    }
    else if (anArg == "-comments"
             && anArgIter + 1 < theNbArgs)
    {
      aNode->InternalParameters.WriteComment = theArgVec[++anArgIter];
    }
    else if (anArg == "-author"
             && anArgIter + 1 < theNbArgs)
    {
      aNode->InternalParameters.WriteAuthor = theArgVec[++anArgIter];
    }
    else if (aDoc.IsNull())
    {
      if (aShapeName.IsEmpty())
      {
        aShapeName = theArgVec[anArgIter];
      }

      Standard_CString aNameVar = theArgVec[anArgIter];
      DDocStd::GetDocument(aNameVar, aDoc, false);
      if (aDoc.IsNull())
      {
        TopoDS_Shape aShape = DBRep::Get(aNameVar);
        if (!aShape.IsNull())
        {
          anApp->NewDocument(TCollection_ExtendedString("BinXCAF"), aDoc);
          Handle(XCAFDoc_ShapeTool) aShapeTool = XCAFDoc_DocumentTool::ShapeTool(aDoc->Main());
          aShapeTool->AddShape(aShape);
        }
      }
    }
    else if (aFileName.IsEmpty())
    {
      aFileName = theArgVec[anArgIter];
    }
    else
    {
      theDI << "Syntax error at '" << theArgVec[anArgIter] << "'\n";
      return 1;
    }
  }
  if (aDoc.IsNull()
      && !aShapeName.IsEmpty())
  {
    theDI << "Syntax error: '" << aShapeName << "' is not a shape nor document\n";
    return 1;
  }
  else if (aDoc.IsNull()
           || aFileName.IsEmpty())
  {
    theDI << "Syntax error: wrong number of arguments\n";
    return 1;
  }
  aNode->GlobalParameters.LengthUnit = getLengthUnit(aDoc);

  if (isPntSet)
  {
    TDF_LabelSequence aRootLabels;
    Handle(XCAFDoc_ShapeTool) aShapeTool = XCAFDoc_DocumentTool::ShapeTool(aDoc->Main());
    aShapeTool->GetFreeShapes(aRootLabels);
    if (aRootLabels.IsEmpty())
    {
      theDI << "Error: empty document\n";
      return 1;
    }
    class PointCloudPlyWriter : public BRepLib_PointCloudShape, public RWPly_PlyWriterContext
    {
    public:
      PointCloudPlyWriter(Standard_Real theTol)
        : BRepLib_PointCloudShape(TopoDS_Shape(), theTol)
      {}

      void AddFaceColor(const TopoDS_Shape& theFace, const Graphic3d_Vec4ub& theColor)
      {
        myFaceColor.Bind(theFace, theColor);
      }

    protected:
      virtual void addPoint(const gp_Pnt& thePoint,
                            const gp_Vec& theNorm,
                            const gp_Pnt2d& theUV,
                            const TopoDS_Shape& theFace)
      {
        Graphic3d_Vec4ub aColor;
        myFaceColor.Find(theFace, aColor);
        RWPly_PlyWriterContext::WriteVertex(thePoint,
                                            Graphic3d_Vec3((float)theNorm.X(), (float)theNorm.Y(), (float)theNorm.Z()),
                                            Graphic3d_Vec2((float)theUV.X(), (float)theUV.Y()),
                                            aColor);
      }

    private:
      NCollection_DataMap<TopoDS_Shape, Graphic3d_Vec4ub> myFaceColor;
    };

    PointCloudPlyWriter aPlyCtx(aTol);
    aPlyCtx.SetNormals(aNode->InternalParameters.WriteNormals);
    aPlyCtx.SetColors(aNode->InternalParameters.WriteColors);
    aPlyCtx.SetTexCoords(aNode->InternalParameters.WriteTexCoords);

    TopoDS_Compound aComp;
    BRep_Builder().MakeCompound(aComp);
    for (XCAFPrs_DocumentExplorer aDocExplorer(aDoc, aRootLabels, XCAFPrs_DocumentExplorerFlags_OnlyLeafNodes);
         aDocExplorer.More(); aDocExplorer.Next())
    {
      const XCAFPrs_DocumentNode& aDocNode = aDocExplorer.Current();
      for (RWMesh_FaceIterator aFaceIter(aDocNode.RefLabel, aDocNode.Location, true, aDocNode.Style); aFaceIter.More(); aFaceIter.Next())
      {
        BRep_Builder().Add(aComp, aFaceIter.Face());
        Graphic3d_Vec4ub aColorVec(255);
        if (aFaceIter.HasFaceColor())
        {
          Graphic3d_Vec4 aColorF = aFaceIter.FaceColor();
          aColorVec.SetValues((unsigned char)int(aColorF.r() * 255.0f),
                              (unsigned char)int(aColorF.g() * 255.0f),
                              (unsigned char)int(aColorF.b() * 255.0f),
                              (unsigned char)int(aColorF.a() * 255.0f));
        }
        aPlyCtx.AddFaceColor(aFaceIter.Face(), aColorVec);
      }
    }
    aPlyCtx.SetShape(aComp);

    Standard_Integer aNbPoints = isDensityPoints
      ? aPlyCtx.NbPointsByDensity(aDens)
      : aPlyCtx.NbPointsByTriangulation();
    if (aNbPoints <= 0)
    {
      theDI << "Error: unable to generate points\n";
      return 1;
    }

    if (!aPlyCtx.Open(aFileName)
        || !aPlyCtx.WriteHeader(aNbPoints, 0, TColStd_IndexedDataMapOfStringString()))
    {
      theDI << "Error: unable to create file '" << aFileName << "'\n";
      return 1;
    }

    Standard_Boolean isDone = isDensityPoints
      ? aPlyCtx.GeneratePointsByDensity(aDens)
      : aPlyCtx.GeneratePointsByTriangulation();
    if (!isDone)
    {
      theDI << "Error: Point cloud was not generated in file '" << aFileName << "'\n";
      return 1;
    }
    else if (!aPlyCtx.Close())
    {
      theDI << "Error: Point cloud file '" << aFileName << "' was not written\n";
      return 1;
    }
    else
    {
      theDI << aNbPoints << "\n";
    }
  }
  else
  {
    Handle(Draw_ProgressIndicator) aProgress = new Draw_ProgressIndicator(theDI, 1);
    Handle(RWPly_Provider) aProvider = new RWPly_Provider(aNode);
    Handle(XSControl_WorkSession) aWS = XSDRAW::Session();
    if (!aProvider->Write(theArgVec[2], aDoc, aWS, aProgress->Start()))
    {
      theDI << "Error: file writing failed '" << theArgVec[2] << "'\n";
      return 1;
    }
  }
  return 0;
}

//-----------------------------------------------------------------------------

void  XSDRAWSTLVRML::InitCommands (Draw_Interpretor& theCommands)
{
  const char* g = "XSTEP-STL/VRML";  // Step transfer file commands
  //XSDRAW::LoadDraw(theCommands);

  theCommands.Add ("ReadGltf",
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
  theCommands.Add ("readgltf",
                   "readgltf shape file"
                   "\n\t\t: Same as ReadGltf but reads glTF file into a shape instead of a document.",
                   __FILE__, ReadGltf, g);
  theCommands.Add ("WriteGltf",
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
  theCommands.Add ("writegltf",
                   "writegltf shape file",
                   __FILE__, WriteGltf, g);
  theCommands.Add ("writevrml", "shape file [version VRML#1.0/VRML#2.0 (1/2): 2 by default] [representation shaded/wireframe/both (0/1/2): 1 by default]",__FILE__,writevrml,g);
  theCommands.Add ("writestl",  "shape file [ascii/binary (0/1) : 1 by default] [InParallel (0/1) : 0 by default]",__FILE__,writestl,g);
  theCommands.Add ("readstl",
                   "readstl shape file [-brep] [-mergeAngle Angle] [-multi]"
                   "\n\t\t: Reads STL file and creates a new shape with specified name."
                   "\n\t\t: When -brep is specified, creates a Compound of per-triangle Faces."
                   "\n\t\t: Single triangulation-only Face is created otherwise (default)."
                   "\n\t\t: -mergeAngle specifies maximum angle in degrees between triangles to merge equal nodes; disabled by default."
                   "\n\t\t: -multi creates a face per solid in multi-domain files; ignored when -brep is set.",
                   __FILE__, readstl, g);
  theCommands.Add ("loadvrml" , "shape file",__FILE__,loadvrml,g);
  theCommands.Add ("ReadObj",
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
  theCommands.Add ("readobj",
                   "readobj shape file [-fileCoordSys {Zup|Yup}] [-fileUnit Unit]"
           "\n\t\t:                    [-resultCoordSys {Zup|Yup}] [-singlePrecision]"
           "\n\t\t:                    [-singleFace]"
           "\n\t\t: Same as ReadObj but reads OBJ file into a shape instead of a document."
           "\n\t\t:   -singleFace merge OBJ content into a single triangulation Face.",
           __FILE__, ReadObj, g);
  theCommands.Add ("WriteObj",
                   "WriteObj Doc file [-fileCoordSys {Zup|Yup}] [-fileUnit Unit]"
           "\n\t\t:                   [-systemCoordSys {Zup|Yup}]"
           "\n\t\t:                   [-comments Text] [-author Name]"
           "\n\t\t: Write XDE document into OBJ file."
           "\n\t\t:   -fileUnit       length unit of OBJ file content;"
           "\n\t\t:   -fileCoordSys   coordinate system defined by OBJ file; Yup when not specified."
           "\n\t\t:   -systemCoordSys system coordinate system; Zup when not specified.",
                   __FILE__, WriteObj, g);
  theCommands.Add ("writeobj",
                   "writeobj shape file",
                   __FILE__, WriteObj, g);

  theCommands.Add ("meshfromstl",     "creates MeshVS_Mesh from STL file",            __FILE__, createmesh,      g );
  theCommands.Add ("mesh3delem",      "creates 3d element mesh to test",              __FILE__, create3d,        g );
  theCommands.Add ("meshshadcolor",   "change MeshVS_Mesh shading color",             __FILE__, meshcolor,       g );
  theCommands.Add ("meshlinkcolor",   "change MeshVS_Mesh line color",                __FILE__, linecolor,       g );
  theCommands.Add ("meshmat",         "change MeshVS_Mesh material and transparency", __FILE__, meshmat,         g );
  theCommands.Add ("meshshrcoef",     "change MeshVS_Mesh shrink coeff",              __FILE__, shrink,          g );
  theCommands.Add ("meshclosed",      "meshclosed meshname (0/1) \nChange MeshVS_Mesh drawing mode. 0 - not closed object, 1 - closed object", __FILE__, closed, g);
  theCommands.Add ("meshshow",        "display MeshVS_Mesh object",                   __FILE__, mdisplay,        g );
  theCommands.Add ("meshhide",        "erase MeshVS_Mesh object",                     __FILE__, merase,          g );
  theCommands.Add ("meshhidesel",     "hide selected entities",                       __FILE__, hidesel,         g );
  theCommands.Add ("meshshowsel",     "show only selected entities",                  __FILE__, showonly,        g );
  theCommands.Add ("meshshowall",     "show all entities",                            __FILE__, showall,         g );
  theCommands.Add ("meshcolors",      "display color presentation",                   __FILE__, meshcolors,      g );
  theCommands.Add ("meshvectors",     "display sample vectors",                       __FILE__, meshvectors,     g );
  theCommands.Add ("meshtext",        "display text labels",                          __FILE__, meshtext,        g );
  theCommands.Add ("meshdeform",      "display deformed mesh",                        __FILE__, meshdeform,      g );
  theCommands.Add ("mesh_edge_width", "set width of edges",                           __FILE__, mesh_edge_width, g );
  theCommands.Add ("meshinfo",        "displays the number of nodes and triangles",   __FILE__, meshinfo,        g );
  theCommands.Add ("WritePly", R"(
WritePly Doc file [-normals {0|1}]=1 [-colors {0|1}]=1 [-uv {0|1}]=0 [-partId {0|1}]=1 [-faceId {0|1}]=0
                  [-pointCloud {0|1}]=0 [-distance Value]=0.0 [-density Value] [-tolerance Value]
Write document or triangulated shape into PLY file.
 -normals write per-vertex normals
 -colors  write per-vertex colors
 -uv      write per-vertex UV coordinates
 -partId  write per-element part index (alternative to -faceId)
 -faceId  write per-element face index (alternative to -partId)

Generate point cloud out of the shape and write it into PLY file.
 -pointCloud write point cloud instead without triangulation indices
 -distance   sets distance from shape into the range [0, Value];
 -density    sets density of points to generate randomly on surface;
 -tolerance  sets tolerance; default value is Precision::Confusion();
)", __FILE__, WritePly, g);
  theCommands.Add ("writeply",
                   "writeply shape file",
                   __FILE__, WritePly, g);
}

//==============================================================================
// XSDRAWSTLVRML::Factory
//==============================================================================
void XSDRAWSTLVRML::Factory(Draw_Interpretor& theDI)
{
  XSDRAWIGES::InitSelect();
  XSDRAWIGES::InitToBRep(theDI);
  XSDRAWIGES::InitFromBRep(theDI);
  XSDRAWSTEP::InitCommands(theDI);
  XSDRAWSTLVRML::InitCommands(theDI);
  XSDRAW::LoadDraw(theDI);
#ifdef OCCT_DEBUG
  theDI << "Draw Plugin : All TKXSDRAW commands are loaded\n";
#endif
}

// Declare entry point PLUGINFACTORY
DPLUGIN(XSDRAWSTLVRML)

