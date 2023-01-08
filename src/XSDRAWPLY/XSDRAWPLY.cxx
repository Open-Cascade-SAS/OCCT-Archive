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

#include <XSDRAWSTLVRML.hxx>

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
#include <XSDRAWPLY.hxx>
#include <XSDRAWSTLVRML_DataSource.hxx>
#include <XSDRAWSTLVRML_DataSource3D.hxx>
#include <XSDRAWSTLVRML_DrawableMesh.hxx>

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

//=======================================================================
//function : writeply
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
  //bool hasColors = true, hasNormals = true, hasTexCoords = false, hasPartId = true, hasFaceId = false;
  //TColStd_IndexedDataMapOfStringString aFileInfo;
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
  aNode->GlobalParameters.LengthUnit = GetLengthUnit(aDoc);

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

//=======================================================================
//function : InitCommands
//purpose  :
//=======================================================================
void XSDRAWSTLVRML::InitCommands(Draw_Interpretor& theCommands)
{
  const char* g = "XSTEP-STL/VRML";  // Step transfer file commands
  //XSDRAW::LoadDraw(theCommands);
  theCommands.Add("WritePly", R"(
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
  theCommands.Add("writeply",
                  "writeply shape file",
                  __FILE__, WritePly, g);
}

//=======================================================================
//function : Factory
//purpose  :
//=======================================================================
void XSDRAWPLY::Factory(Draw_Interpretor& theDI)
{
  XSDRAWIGES::InitSelect();
  XSDRAWIGES::InitToBRep(theDI);
  XSDRAWIGES::InitFromBRep(theDI);
  XSDRAWSTLVRML::InitCommands(theDI);
  XSDRAW::LoadDraw(theDI);
#ifdef OCCT_DEBUG
  theDI << "Draw Plugin : All TKXSDRAW commands are loaded\n";
#endif
}
