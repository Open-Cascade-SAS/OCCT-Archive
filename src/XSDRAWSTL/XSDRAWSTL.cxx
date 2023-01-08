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

#include <XSDRAWSTL.hxx>

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
  }
  else
  {
    TopoDS_Shape aShape = DBRep::Get(theArgVec[1]);
    Standard_Boolean isASCIIMode = Standard_False;
    if (theNbArgs == 4)
    {
      isASCIIMode = (Draw::Atoi(theArgVec[3]) == 0);
    }
    Handle(RWStl_ConfigurationNode) aNode = new RWStl_ConfigurationNode();
    aNode->GlobalParameters.LengthUnit = GetLengthUnit();
    Handle(RWStl_Provider) aProvider = new RWStl_Provider(aNode);
    aNode->InternalParameters.WriteAscii = isASCIIMode;
    Handle(XSControl_WorkSession) aWS = XSDRAW::Session();
    Handle(Draw_ProgressIndicator) aProgress = new Draw_ProgressIndicator(theDI);
    if (!aProvider->Write(theArgVec[2], aShape, aWS, aProgress->Start()))
    {
      theDI << "Error: Mesh writing has been failed.\n";
    }
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
  aNode->GlobalParameters.LengthUnit = GetLengthUnit();
  //bool toCreateCompOfTris = false;
  //bool anIsMulti = false;
  //double aMergeAngle = M_PI / 2.0;
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
      if (!toCreateCompOfTris)
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

//=======================================================================
//function : InitCommands
//purpose  :
//=======================================================================
void XSDRAWSTL::InitCommands(Draw_Interpretor& theCommands)
{
  const char* g = "XSTEP-STL/VRML";  // Step transfer file commands
  //XSDRAW::LoadDraw(theCommands);

  theCommands.Add("writestl", "shape file [ascii/binary (0/1) : 1 by default] [InParallel (0/1) : 0 by default]", __FILE__, writestl, g);
  theCommands.Add("readstl",
                  "readstl shape file [-brep] [-mergeAngle Angle] [-multi]"
                  "\n\t\t: Reads STL file and creates a new shape with specified name."
                  "\n\t\t: When -brep is specified, creates a Compound of per-triangle Faces."
                  "\n\t\t: Single triangulation-only Face is created otherwise (default)."
                  "\n\t\t: -mergeAngle specifies maximum angle in degrees between triangles to merge equal nodes; disabled by default."
                  "\n\t\t: -multi creates a face per solid in multi-domain files; ignored when -brep is set.",
                  __FILE__, readstl, g);

}

