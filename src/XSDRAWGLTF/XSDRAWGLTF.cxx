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

#include <DBRep.hxx>
#include <DDocStd.hxx>
#include <DDocStd_DrawDocument.hxx>
#include <Draw.hxx>
#include <Draw_Interpretor.hxx>
#include <Draw_ProgressIndicator.hxx>
#include <RWGltf_ConfigurationNode.hxx>
#include <RWGltf_Provider.hxx>
#include <XSControl_WorkSession.hxx>
#include <XSDRAWBase.hxx>
#include <TDataStd_Name.hxx>
#include <TDocStd_Application.hxx>
#include <TopoDS_Shape.hxx>

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

  aNode->GlobalParameters.LengthUnit = XSDRAWBase::GetLengthUnit(aDoc);
  Handle(RWGltf_Provider) aProvider =
    new RWGltf_Provider(aNode);
  Standard_Boolean aReadStat = Standard_False;
  Handle(XSControl_WorkSession) aWS = XSDRAWBase::Session();
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
  aNode->GlobalParameters.LengthUnit = XSDRAWBase::GetLengthUnit(aDoc);
  Handle(RWGltf_Provider) aProvider =
    new RWGltf_Provider(aNode);
  Standard_Boolean aWriteStat = Standard_False;
  Handle(XSControl_WorkSession) aWS = XSDRAWBase::Session();
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
//function : Factory
//purpose  :
//=======================================================================
void XSDRAWGLTF::Factory(Draw_Interpretor& theDI)
{
  const char* g = "XSTEP-STL/VRML";  // Step transfer file commands

  theDI.Add("ReadGltf",
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
  theDI.Add("readgltf",
            "readgltf shape file"
            "\n\t\t: Same as ReadGltf but reads glTF file into a shape instead of a document.",
            __FILE__, ReadGltf, g);
  theDI.Add("WriteGltf",
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
  theDI.Add("writegltf",
            "writegltf shape file",
            __FILE__, WriteGltf, g);
  XSDRAWBase::LoadDraw(theDI);
#ifdef OCCT_DEBUG
  theDI << "Draw Plugin : All XSDRAWGLTF commands are loaded\n";
#endif
}
