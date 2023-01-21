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
//function : createmesh
//purpose  :
//=======================================================================
static Standard_Integer createmesh(Draw_Interpretor& theDI,
                                   Standard_Integer theNbArgs,
                                   const char** theArgVec)
{
  if (theNbArgs < 3)
  {
    theDI << "Wrong number of parameters\n";
    theDI << "Use: " << theArgVec[0] << " <mesh name> <stl file>\n";
    return 0;
  }

  Handle(AIS_InteractiveContext) aContext = ViewerTest::GetAISContext();
  if (aContext.IsNull())
  {
    theDI << "No active view. Please call 'vinit' first\n";
    return 0;
  }

  // Progress indicator
  OSD_Path aFile(theArgVec[2]);
  Handle(Draw_ProgressIndicator) aProgress = new Draw_ProgressIndicator(theDI, 1);
  Handle(Poly_Triangulation) aSTLMesh = RWStl::ReadFile(aFile, aProgress->Start());

  theDI << "Reading OK...\n";
  Handle(XSDRAWSTLVRML_DataSource) aDS = new XSDRAWSTLVRML_DataSource(aSTLMesh);
  theDI << "Data source is created successful\n";
  Handle(MeshVS_Mesh) aMesh = new MeshVS_Mesh();
  theDI << "MeshVS_Mesh is created successful\n";

  aMesh->SetDataSource(aDS);
  aMesh->AddBuilder(new MeshVS_MeshPrsBuilder(aMesh.operator->()), Standard_True);

  aMesh->GetDrawer()->SetColor(MeshVS_DA_EdgeColor, Quantity_NOC_YELLOW);

  // Hide all nodes by default
  Handle(TColStd_HPackedMapOfInteger) aNodes = new TColStd_HPackedMapOfInteger();
  const Standard_Integer aLen = aSTLMesh->NbNodes();
  for (Standard_Integer anIndex = 1; anIndex <= aLen; anIndex++)
    aNodes->ChangeMap().Add(anIndex);
  aMesh->SetHiddenNodes(aNodes);
  aMesh->SetSelectableNodes(aNodes);

  VDisplayAISObject(theArgVec[1], aMesh);
  aContext->Deactivate(aMesh);

  Draw::Set(theArgVec[1], new XSDRAWSTLVRML_DrawableMesh(aMesh));
  Handle(V3d_View) aView = ViewerTest::CurrentView();
  if (!aView.IsNull())
    aView->FitAll();

  return 0;
}

//=======================================================================
//function : create3d
//purpose  :
//=======================================================================
static Standard_Integer create3d(Draw_Interpretor& theDI,
                                 Standard_Integer theNbArgs,
                                 const char** theArgVec)
{
  if (theNbArgs < 2)
  {
    theDI << "Wrong number of parameters\n";
    theDI << "Use: " << theArgVec[0] << " <mesh name>\n";
    return 0;
  }

  Handle(AIS_InteractiveContext) aContext = ViewerTest::GetAISContext();
  if (aContext.IsNull())
  {
    theDI << "No active view. Please call 'vinit' first\n";
    return 0;
  }

  Handle(XSDRAWSTLVRML_DataSource3D) aDS = new XSDRAWSTLVRML_DataSource3D();
  theDI << "Data source is created successful\n";
  Handle(MeshVS_Mesh) aMesh = new MeshVS_Mesh();
  theDI << "MeshVS_Mesh is created successful\n";

  aMesh->SetDataSource(aDS);
  aMesh->AddBuilder(new MeshVS_MeshPrsBuilder(aMesh.operator->()), Standard_True);

  aMesh->GetDrawer()->SetColor(MeshVS_DA_EdgeColor, Quantity_NOC_YELLOW);

  // Hide all nodes by default
  Handle(TColStd_HPackedMapOfInteger) aNodes = new TColStd_HPackedMapOfInteger();
  Standard_Integer aLen = aDS->GetAllNodes().Extent();
  for (Standard_Integer anIndex = 1; anIndex <= aLen; anIndex++)
    aNodes->ChangeMap().Add(anIndex);
  aMesh->SetHiddenNodes(aNodes);
  aMesh->SetSelectableNodes(aNodes);

  VDisplayAISObject(theArgVec[1], aMesh);
  aContext->Deactivate(aMesh);

  Draw::Set(theArgVec[1], new XSDRAWSTLVRML_DrawableMesh(aMesh));
  Handle(V3d_View) aView = ViewerTest::CurrentView();
  if (!aView.IsNull())
    aView->FitAll();

  return 0;
}

//=======================================================================
//function : getMesh
//purpose  :
//=======================================================================
Handle(MeshVS_Mesh) getMesh(const char* theName,
                            Draw_Interpretor& theDI)
{
  Handle(XSDRAWSTLVRML_DrawableMesh) aDrawMesh =
    Handle(XSDRAWSTLVRML_DrawableMesh)::DownCast(Draw::Get(theName));

  if (aDrawMesh.IsNull())
  {
    theDI << "There is no such object\n";
    return NULL;
  }
  else
  {
    Handle(MeshVS_Mesh) aMesh = aDrawMesh->GetMesh();
    if (aMesh.IsNull())
    {
      theDI << "There is invalid mesh\n";
      return NULL;
    }
    else
      return aMesh;
  }
}

//=======================================================================
//function : setcolor
//purpose  :
//=======================================================================
static Standard_Integer setcolor(Draw_Interpretor& theDI,
                                 Standard_Integer theNbArgs,
                                 const char** theArgVec,
                                 Standard_Integer theParam)
{
  if (theNbArgs < 5)
    theDI << "Wrong number of parameters\n";
  else
  {
    Handle(MeshVS_Mesh) aMesh = getMesh(theArgVec[1], theDI);
    if (!aMesh.IsNull())
    {
      Standard_Real aRed = Draw::Atof(theArgVec[2]);
      Standard_Real aGreen = Draw::Atof(theArgVec[3]);
      Standard_Real aBlue = Draw::Atof(theArgVec[4]);
      aMesh->GetDrawer()->SetColor((MeshVS_DrawerAttribute)theParam,
                                   Quantity_Color(aRed, aGreen, aBlue, Quantity_TOC_RGB));

      Handle(AIS_InteractiveContext) aContext = ViewerTest::GetAISContext();

      if (aContext.IsNull())
        theDI << "The context is null\n";
      else
        aContext->Redisplay(aMesh, Standard_True);
    }
  }
  return 0;
}

//=======================================================================
//function : meshcolor
//purpose  :
//=======================================================================
static Standard_Integer meshcolor(Draw_Interpretor& theInterp,
                                  Standard_Integer theNbArgs,
                                  const char** theArgVec)
{
  return setcolor(theInterp, theNbArgs, theArgVec, MeshVS_DA_InteriorColor);
}

//=======================================================================
//function : linecolor
//purpose  :
//=======================================================================
static Standard_Integer linecolor(Draw_Interpretor& theInterp,
                                  Standard_Integer theNbArgs,
                                  const char** theArgVec)
{
  return setcolor(theInterp, theNbArgs, theArgVec, MeshVS_DA_EdgeColor);
}

//=======================================================================
//function : meshmat
//purpose  :
//=======================================================================
static Standard_Integer meshmat(Draw_Interpretor& theDI,
                                Standard_Integer theNbArgs,
                                const char** theArgVec)
{
  if (theNbArgs < 3)
    theDI << "Wrong number of parameters\n";
  else
  {
    Handle(MeshVS_Mesh) aMesh = getMesh(theArgVec[1], theDI);
    if (!aMesh.IsNull())
    {
      Standard_Integer aMaterial = Draw::Atoi(theArgVec[2]);

      Graphic3d_MaterialAspect aMatAsp =
        (Graphic3d_MaterialAspect)(Graphic3d_NameOfMaterial)aMaterial;

      if (theNbArgs == 4)
      {
        Standard_Real aTransparency = Draw::Atof(theArgVec[3]);
        aMatAsp.SetTransparency(Standard_ShortReal(aTransparency));
      }
      aMesh->GetDrawer()->SetMaterial(MeshVS_DA_FrontMaterial, aMatAsp);
      aMesh->GetDrawer()->SetMaterial(MeshVS_DA_BackMaterial, aMatAsp);

      Handle(AIS_InteractiveContext) aContext = ViewerTest::GetAISContext();

      if (aContext.IsNull())
        theDI << "The context is null\n";
      else
        aContext->Redisplay(aMesh, Standard_True);
    }
  }
  return 0;
}

//=======================================================================
//function : shrink
//purpose  :
//=======================================================================
static Standard_Integer shrink(Draw_Interpretor& theDI,
                               Standard_Integer theNbArgs,
                               const char** theArgVec)
{
  if (theNbArgs < 3)
    theDI << "Wrong number of parameters\n";
  else
  {
    Handle(MeshVS_Mesh) aMesh = getMesh(theArgVec[1], theDI);
    if (!aMesh.IsNull())
    {
      Standard_Real aShrinkCoeff = Draw::Atof(theArgVec[2]);
      aMesh->GetDrawer()->SetDouble(MeshVS_DA_ShrinkCoeff, aShrinkCoeff);

      Handle(AIS_InteractiveContext) aContext = ViewerTest::GetAISContext();

      if (aContext.IsNull())
        theDI << "The context is null\n";
      else
        aContext->Redisplay(aMesh, Standard_True);
    }
  }
  return 0;
}

//=======================================================================
//function : closed
//purpose  :
//=======================================================================
static Standard_Integer closed(Draw_Interpretor& theDI,
                               Standard_Integer theArgc,
                               const char** theArgv)
{
  if (theArgc < 3)
  {
    theDI << "Wrong number of parameters.\n";
  }
  else
  {
    Handle(MeshVS_Mesh) aMesh = getMesh(theArgv[1], theDI);
    if (!aMesh.IsNull())
    {
      Standard_Boolean aFlag = Draw::Atoi(theArgv[2]) != 0;
      aMesh->GetDrawer()->SetBoolean(MeshVS_DA_SupressBackFaces, aFlag);

      Handle(AIS_InteractiveContext) aContext = ViewerTest::GetAISContext();
      if (aContext.IsNull())
      {
        theDI << "The context is null\n";
      }
      else
      {
        aContext->Redisplay(aMesh, Standard_True);
      }
    }
  }
  return 0;
}

//=======================================================================
//function : mdisplay
//purpose  :
//=======================================================================
static Standard_Integer mdisplay(Draw_Interpretor& theDI,
                                 Standard_Integer theNbArgs,
                                 const char** theArgVec)
{
  if (theNbArgs < 2)
    theDI << "Wrong number of parameters\n";
  else
  {
    Handle(MeshVS_Mesh) aMesh = getMesh(theArgVec[1], theDI);
    if (!aMesh.IsNull())
    {
      Handle(AIS_InteractiveContext) aContext = ViewerTest::GetAISContext();

      if (aContext.IsNull())
        theDI << "The context is null\n";
      else
      {
        aContext->Display(aMesh, Standard_True);
      }
    }
  }
  return 0;
}

//=======================================================================
//function : merase
//purpose  :
//=======================================================================
static Standard_Integer merase(Draw_Interpretor& theDI,
                               Standard_Integer theNbArgs,
                               const char** theArgVec)
{
  if (theNbArgs < 2)
    theDI << "Wrong number of parameters\n";
  else
  {
    Handle(MeshVS_Mesh) aMesh = getMesh(theArgVec[1], theDI);
    if (!aMesh.IsNull())
    {
      Handle(AIS_InteractiveContext) aContext = ViewerTest::GetAISContext();

      if (aContext.IsNull())
        theDI << "The context is null\n";
      else
      {
        aContext->Erase(aMesh, Standard_True);
      }
    }
    else
      theDI << "Mesh is null\n";
  }
  return 0;
}

//=======================================================================
//function : hidesel
//purpose  :
//=======================================================================
static Standard_Integer hidesel(Draw_Interpretor& theDI,
                                Standard_Integer theNbArgs,
                                const char** theArgVec)
{
  if (theNbArgs < 2)
  {
    theDI << "Wrong number of parameters\n";
    theDI << "Use: " << theArgVec[0] << " <mesh name>\n";
    return 0;
  }

  Handle(AIS_InteractiveContext) aContext = ViewerTest::GetAISContext();
  Handle(MeshVS_Mesh) aMesh = getMesh(theArgVec[1], theDI);
  if (aMesh.IsNull())
  {
    theDI << "The mesh is invalid\n";
    return 0;
  }

  if (aContext.IsNull())
    theDI << "The context is null\n";
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
    for (aContext->InitSelected(); aContext->MoreSelected(); aContext->NextSelected())
    {
      Handle(MeshVS_MeshEntityOwner) anOwner =
        Handle(MeshVS_MeshEntityOwner)::DownCast(aContext->SelectedOwner());
      if (!anOwner.IsNull())
      {
        if (anOwner->Type() == MeshVS_ET_Node)
        {
          aHiddenNodes->ChangeMap().Add(anOwner->ID());
        }
        else
        {
          aHiddenElements->ChangeMap().Add(anOwner->ID());
        }
      }
    }
    aContext->ClearSelected(Standard_False);
    aMesh->SetHiddenNodes(aHiddenNodes);
    aMesh->SetHiddenElems(aHiddenElements);
    aContext->Redisplay(aMesh, Standard_True);
  }

  return 0;
}

//=======================================================================
//function : showonly
//purpose  :
//=======================================================================
static Standard_Integer showonly(Draw_Interpretor& theDI,
                                 Standard_Integer theNbArgs,
                                 const char** theArgVec)
{
  if (theNbArgs < 2)
  {
    theDI << "Wrong number of parameters\n";
    theDI << "Use: " << theArgVec[0] << " <mesh name>\n";
    return 0;
  }

  Handle(AIS_InteractiveContext) aContext = ViewerTest::GetAISContext();
  Handle(MeshVS_Mesh) aMesh = getMesh(theArgVec[1], theDI);
  if (aMesh.IsNull())
  {
    theDI << "The mesh is invalid\n";
    return 0;
  }

  if (aContext.IsNull())
    theDI << "The context is null\n";
  else
  {
    Handle(TColStd_HPackedMapOfInteger) aHiddenNodes =
      new TColStd_HPackedMapOfInteger(aMesh->GetDataSource()->GetAllNodes());
    Handle(TColStd_HPackedMapOfInteger) aHiddenElements =
      new TColStd_HPackedMapOfInteger(aMesh->GetDataSource()->GetAllElements());
    for (aContext->InitSelected(); aContext->MoreSelected(); aContext->NextSelected())
    {
      Handle(MeshVS_MeshEntityOwner) anOwner =
        Handle(MeshVS_MeshEntityOwner)::DownCast(aContext->SelectedOwner());
      if (!anOwner.IsNull())
      {
        if (anOwner->Type() == MeshVS_ET_Node)
        {
          aHiddenNodes->ChangeMap().Remove(anOwner->ID());
        }
        else
        {
          aHiddenElements->ChangeMap().Remove(anOwner->ID());
        }
      }
    }
    aMesh->SetHiddenNodes(aHiddenNodes);
    aMesh->SetHiddenElems(aHiddenElements);
    aContext->Redisplay(aMesh, Standard_True);
  }

  return 0;
}

//=======================================================================
//function : showonly
//purpose  :
//=======================================================================
static Standard_Integer showall(Draw_Interpretor& theDI,
                                Standard_Integer theNbArgs,
                                const char** theArgVec)
{
  if (theNbArgs < 2)
  {
    theDI << "Wrong number of parameters\n";
    theDI << "Use: " << theArgVec[0] << " <mesh name>\n";
    return 0;
  }

  Handle(AIS_InteractiveContext) aContext = ViewerTest::GetAISContext();
  Handle(MeshVS_Mesh) aMesh = getMesh(theArgVec[1], theDI);
  if (aMesh.IsNull())
  {
    theDI << "The mesh is invalid\n";
    return 0;
  }

  if (aContext.IsNull())
    theDI << "The context is null\n";
  else
  {
    aMesh->SetHiddenNodes(new TColStd_HPackedMapOfInteger());
    aMesh->SetHiddenElems(new TColStd_HPackedMapOfInteger());
    aContext->Redisplay(aMesh, Standard_True);
  }

  return 0;
}

//=======================================================================
//function : meshcolors
//purpose  :
//=======================================================================
static Standard_Integer meshcolors(Draw_Interpretor& theDI,
                                   Standard_Integer theNbArgs,
                                   const char** theArgVec)
{
  try
  {
    OCC_CATCH_SIGNALS
      if (theNbArgs < 4)
      {
        theDI << "Wrong number of parameters\n";
        theDI << "Use : meshcolors <mesh name> <mode> <isreflect>\n";
        theDI << "mode : {elem1|elem2|nodal|nodaltex|none}\n";
        theDI << "       elem1 - different color for each element\n";
        theDI << "       elem2 - one color for one side\n";
        theDI << "       nodal - different color for each node\n";
        theDI << "       nodaltex - different color for each node with texture interpolation\n";
        theDI << "       none  - clear\n";
        theDI << "isreflect : {0|1} \n";

        return 0;
      }

    Handle(MeshVS_Mesh) aMesh = getMesh(theArgVec[1], theDI);

    if (aMesh.IsNull())
    {
      theDI << "Mesh not found\n";
      return 0;
    }
    Handle(AIS_InteractiveContext) anIC = ViewerTest::GetAISContext();
    if (anIC.IsNull())
    {
      theDI << "The context is null\n";
      return 0;
    }
    if (!aMesh.IsNull())
    {
      TCollection_AsciiString aMode = TCollection_AsciiString(theArgVec[2]);
      Quantity_Color aColor1(Quantity_NOC_BLUE1);
      Quantity_Color aColor2(Quantity_NOC_RED1);
      if (aMode.IsEqual("elem1") || aMode.IsEqual("elem2") || aMode.IsEqual("nodal") || aMode.IsEqual("nodaltex") || aMode.IsEqual("none"))
      {
        Handle(MeshVS_PrsBuilder) aTempBuilder;
        Standard_Integer aReflection = Draw::Atoi(theArgVec[3]);

        for (Standard_Integer aCount = 0; aCount < aMesh->GetBuildersCount(); aCount++)
        {
          aTempBuilder = aMesh->FindBuilder("MeshVS_ElementalColorPrsBuilder");
          if (!aTempBuilder.IsNull())
            aMesh->RemoveBuilderById(aTempBuilder->GetId());

          aTempBuilder = aMesh->FindBuilder("MeshVS_NodalColorPrsBuilder");
          if (!aTempBuilder.IsNull())
            aMesh->RemoveBuilderById(aTempBuilder->GetId());
        }

        if (aMode.IsEqual("elem1") || aMode.IsEqual("elem2"))
        {
          Handle(MeshVS_ElementalColorPrsBuilder) aBuilder = new MeshVS_ElementalColorPrsBuilder(
            aMesh, MeshVS_DMF_ElementalColorDataPrs | MeshVS_DMF_OCCMask);
          // Color
          const TColStd_PackedMapOfInteger& anAllElements = aMesh->GetDataSource()->GetAllElements();
          TColStd_MapIteratorOfPackedMapOfInteger anIter(anAllElements);

          if (aMode.IsEqual("elem1"))
            for (; anIter.More(); anIter.Next())
            {
              Quantity_Color aColor((Quantity_NameOfColor)(anIter.Key() % Quantity_NOC_WHITE));
              aBuilder->SetColor1(anIter.Key(), aColor);
            }
          else
            for (; anIter.More(); anIter.Next())
              aBuilder->SetColor2(anIter.Key(), aColor1, aColor2);

          aMesh->AddBuilder(aBuilder, Standard_True);
        }

        if (aMode.IsEqual("nodal"))
        {
          Handle(MeshVS_NodalColorPrsBuilder) aBuilder = new MeshVS_NodalColorPrsBuilder(
            aMesh, MeshVS_DMF_NodalColorDataPrs | MeshVS_DMF_OCCMask);
          aMesh->AddBuilder(aBuilder, Standard_True);

          // Color
          const TColStd_PackedMapOfInteger& anAllNodes =
            aMesh->GetDataSource()->GetAllNodes();
          TColStd_MapIteratorOfPackedMapOfInteger anIter(anAllNodes);
          for (; anIter.More(); anIter.Next())
          {
            Quantity_Color aColor((Quantity_NameOfColor)(
              anIter.Key() % Quantity_NOC_WHITE));
            aBuilder->SetColor(anIter.Key(), aColor);
          }
          aMesh->AddBuilder(aBuilder, Standard_True);
        }

        if (aMode.IsEqual("nodaltex"))
        {
          // assign nodal builder to the mesh
          Handle(MeshVS_NodalColorPrsBuilder) aBuilder = new MeshVS_NodalColorPrsBuilder(
            aMesh, MeshVS_DMF_NodalColorDataPrs | MeshVS_DMF_OCCMask);
          aMesh->AddBuilder(aBuilder, Standard_True);
          aBuilder->UseTexture(Standard_True);

          // prepare color map for texture
          Aspect_SequenceOfColor aColorMap;
          aColorMap.Append((Quantity_NameOfColor)Quantity_NOC_RED);
          aColorMap.Append((Quantity_NameOfColor)Quantity_NOC_YELLOW);
          aColorMap.Append((Quantity_NameOfColor)Quantity_NOC_BLUE1);

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
            try
            {
              OCC_CATCH_SIGNALS
                aScaleValue = (aCoords.Value(1) - (Standard_Real)aMinX) / aDelta;
            }
            catch (Standard_Failure const&)
            {
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

        aMesh->GetDrawer()->SetBoolean(MeshVS_DA_ColorReflection, aReflection != 0);

        anIC->Redisplay(aMesh, Standard_True);
      }
      else
      {
        theDI << "Wrong mode name\n";
        return 0;
      }
    }
  }
  catch (Standard_Failure const&)
  {
    theDI << "Error\n";
  }

  return 0;
}

//=======================================================================
//function : meshvectors
//purpose  :
//=======================================================================
static Standard_Integer meshvectors(Draw_Interpretor& theDI,
                                    Standard_Integer theNbArgs,
                                    const char** theArgVec)
{
  if (theNbArgs < 3)
  {
    theDI << "Wrong number of parameters\n";
    theDI << "Use : meshvectors <mesh name> < -mode {elem|nodal|none} > [-maxlen len] [-color name] [-arrowpart ratio] [-issimple {1|0}]\n";
    theDI << "Supported mode values:\n";
    theDI << "       elem  - vector per element\n";
    theDI << "       nodal - vector per node\n";
    theDI << "       none  - clear\n";

    return 0;
  }

  Handle(MeshVS_Mesh) aMesh = getMesh(theArgVec[1], theDI);

  if (aMesh.IsNull())
  {
    theDI << "Mesh not found\n";
    return 0;
  }
  Handle(AIS_InteractiveContext) anIC = ViewerTest::GetAISContext();
  if (anIC.IsNull())
  {
    theDI << "The context is null\n";
    return 0;
  }

  TCollection_AsciiString aParam;
  TCollection_AsciiString aMode("none");
  Standard_Real           aMaxlen(1.0);
  Quantity_Color          aColor(Quantity_NOC_ORANGE);
  Standard_Real           anArrowPart(0.1);
  Standard_Boolean        isSimplePrs(Standard_False);

  for (Standard_Integer anIdx = 2; anIdx < theNbArgs; anIdx++)
  {
    if (!aParam.IsEmpty())
    {
      if (aParam == "-mode")
      {
        aMode = theArgVec[anIdx];
      }
      else if (aParam == "-maxlen")
      {
        aMaxlen = Draw::Atof(theArgVec[anIdx]);
      }
      else if (aParam == "-color")
      {
        if (!Quantity_Color::ColorFromName(theArgVec[anIdx], aColor))
        {
          theDI << "Syntax error at " << aParam << "\n";
          return 1;
        }
      }
      else if (aParam == "-arrowpart")
      {
        anArrowPart = Draw::Atof(theArgVec[anIdx]);
      }
      else if (aParam == "-issimple")
      {
        isSimplePrs = Draw::Atoi(theArgVec[anIdx]) != 0;
      }
      aParam.Clear();
    }
    else if (theArgVec[anIdx][0] == '-')
    {
      aParam = theArgVec[anIdx];
    }
  }

  if (!aMode.IsEqual("elem") && !aMode.IsEqual("nodal") && !aMode.IsEqual("none"))
  {
    theDI << "Wrong mode name\n";
    return 0;
  }

  Handle(MeshVS_PrsBuilder) aTempBuilder;

  aTempBuilder = aMesh->FindBuilder("MeshVS_VectorPrsBuilder");
  if (!aTempBuilder.IsNull())
    aMesh->RemoveBuilderById(aTempBuilder->GetId());

  if (!aMode.IsEqual("none"))
  {
    Handle(MeshVS_VectorPrsBuilder) aBuilder = new MeshVS_VectorPrsBuilder(aMesh.operator->(),
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
    aCoords.Init(0.);
    TColStd_MapIteratorOfPackedMapOfInteger anIter(anAllIDs);
    for (; anIter.More(); anIter.Next())
    {
      Standard_Boolean IsValidData = Standard_False;
      if (anIsElement)
      {
        aMesh->GetDataSource()->GetGeomType(anIter.Key(), anIsElement, aEntType);
        if (aEntType == MeshVS_ET_Face)
          IsValidData = aMesh->GetDataSource()->GetNormal(anIter.Key(), 3, aCoords.ChangeValue(1), aCoords.ChangeValue(2), aCoords.ChangeValue(3));
      }
      else
        IsValidData = aMesh->GetDataSource()->GetGeom(anIter.Key(), Standard_False, aCoords, aNbNodes, aEntType);

      gp_Vec aNorm;
      if (IsValidData)
      {
        aNorm = gp_Vec(aCoords.Value(1), aCoords.Value(2), aCoords.Value(3));
        if (aNorm.Magnitude() < gp::Resolution())
        {
          aNorm = gp_Vec(0, 0, 1); //method GetGeom(...) returns coordinates of nodes
        }
      }
      else
      {
        aNorm = gp_Vec(0, 0, 1);
      }
      aBuilder->SetVector(anIsElement, anIter.Key(), aNorm.Normalized());
    }

    aMesh->AddBuilder(aBuilder, Standard_False);
    aMesh->GetDrawer()->SetDouble(MeshVS_DA_VectorArrowPart, anArrowPart);
  }

  anIC->Redisplay(aMesh, Standard_True);

  return 0;
}

//=======================================================================
//function : meshtext
//purpose  :
//=======================================================================
static Standard_Integer meshtext(Draw_Interpretor& theDI,
                                 Standard_Integer theNbArgs,
                                 const char** theArgVec)
{
  if (theNbArgs < 2)
  {
    theDI << "Wrong number of parameters\n";
    theDI << "Use : meshtext <mesh name>\n";
    return 0;
  }

  Handle(MeshVS_Mesh) aMesh = getMesh(theArgVec[1], theDI);

  if (aMesh.IsNull())
  {
    theDI << "Mesh not found\n";
    return 0;
  }

  Handle(AIS_InteractiveContext) anIC = ViewerTest::GetAISContext();
  if (anIC.IsNull())
  {
    theDI << "The context is null\n";
    return 0;
  }

  // Prepare triangle labels
  MeshVS_DataMapOfIntegerAsciiString aLabels;
  Standard_Integer aLen = aMesh->GetDataSource()->GetAllElements().Extent();
  for (Standard_Integer anIndex = 1; anIndex <= aLen; anIndex++)
  {
    aLabels.Bind(anIndex, TCollection_AsciiString(anIndex));
  }

  Handle(MeshVS_TextPrsBuilder) aTextBuilder = new MeshVS_TextPrsBuilder(aMesh.operator->(), 20., Quantity_NOC_YELLOW);
  aTextBuilder->SetTexts(Standard_True, aLabels);
  aMesh->AddBuilder(aTextBuilder);

  return 0;
}

//=======================================================================
//function : meshdeform
//purpose  :
//=======================================================================
static Standard_Integer meshdeform(Draw_Interpretor& theDI,
                                   Standard_Integer theNbArgs,
                                   const char** theArgVec)
{
  if (theNbArgs < 3)
  {
    theDI << "Wrong number of parameters\n";
    theDI << "Use : meshdeform <mesh name> < -mode {on|off} > [-scale scalefactor]\n";
    return 0;
  }

  Handle(MeshVS_Mesh) aMesh = getMesh(theArgVec[1], theDI);

  if (aMesh.IsNull())
  {
    theDI << "Mesh not found\n";
    return 0;
  }
  Handle(AIS_InteractiveContext) anIC = ViewerTest::GetAISContext();
  if (anIC.IsNull())
  {
    theDI << "The context is null\n";
    return 0;
  }

  TCollection_AsciiString aParam;
  TCollection_AsciiString aMode("off");
  Standard_Real           aScale(1.0);

  for (Standard_Integer anIdx = 2; anIdx < theNbArgs; anIdx++)
  {
    if (!aParam.IsEmpty())
    {
      if (aParam == "-mode")
      {
        aMode = theArgVec[anIdx];
      }
      else if (aParam == "-scale")
      {
        aScale = Draw::Atof(theArgVec[anIdx]);
      }
      aParam.Clear();
    }
    else if (theArgVec[anIdx][0] == '-')
    {
      aParam = theArgVec[anIdx];
    }
  }

  if (!aMode.IsEqual("on") && !aMode.IsEqual("off"))
  {
    theDI << "Wrong mode name\n";
    return 0;
  }

  Handle(MeshVS_DeformedDataSource) aDefDS =
    new MeshVS_DeformedDataSource(aMesh->GetDataSource(), aScale);

  const TColStd_PackedMapOfInteger& anAllIDs = aMesh->GetDataSource()->GetAllNodes();

  Standard_Integer aNbNodes;
  MeshVS_EntityType aEntType;

  TColStd_MapIteratorOfPackedMapOfInteger anIter(anAllIDs);
  for (; anIter.More(); anIter.Next())
  {
    TColStd_Array1OfReal aCoords(1, 3);
    aMesh->GetDataSource()->GetGeom(anIter.Key(), Standard_False, aCoords, aNbNodes, aEntType);

    gp_Vec aNorm = gp_Vec(aCoords.Value(1), aCoords.Value(2), aCoords.Value(3));
    if (!aNorm.Magnitude())
      aNorm = gp_Vec(0, 0, 1);
    aDefDS->SetVector(anIter.Key(), aNorm.Normalized());
  }

  aMesh->SetDataSource(aDefDS);

  anIC->Redisplay(aMesh, Standard_False);

  Handle(V3d_View) aView = ViewerTest::CurrentView();
  if (!aView.IsNull())
    aView->FitAll();

  return 0;
}

//=======================================================================
//function : mesh_edge_width
//purpose  :
//=======================================================================
static Standard_Integer mesh_edge_width(Draw_Interpretor& theDI,
                                        Standard_Integer theNbArgs,
                                        const char** theArgVec)
{
  try
  {
    OCC_CATCH_SIGNALS
      if (theNbArgs < 3)
      {
        theDI << "Wrong number of parameters\n";
        theDI << "Use : mesh_edge_width <mesh name> <width>\n";
        return 0;
      }

    Handle(MeshVS_Mesh) aMesh = getMesh(theArgVec[1], theDI);
    if (aMesh.IsNull())
    {
      theDI << "Mesh not found\n";
      return 0;
    }

    const char* aWidthStr = theArgVec[2];
    if (aWidthStr == 0 || Draw::Atof(aWidthStr) <= 0)
    {
      theDI << "Width must be real value more than zero\n";
      return 0;
    }

    double aWidth = Draw::Atof(aWidthStr);

    Handle(AIS_InteractiveContext) anIC = ViewerTest::GetAISContext();
    if (anIC.IsNull())
    {
      theDI << "The context is null\n";
      return 0;
    }

    Handle(MeshVS_Drawer) aDrawer = aMesh->GetDrawer();
    if (aDrawer.IsNull())
    {
      theDI << "The drawer is null\n";
      return 0;
    }

    aDrawer->SetDouble(MeshVS_DA_EdgeWidth, aWidth);
    anIC->Redisplay(aMesh, Standard_True);
  }
  catch (Standard_Failure const&)
  {
    theDI << "Error\n";
  }

  return 0;
}

//=======================================================================
//function : meshinfo
//purpose  :
//=======================================================================
static Standard_Integer meshinfo(Draw_Interpretor& theDI,
                                 Standard_Integer theNbArgs,
                                 const char** theArgVec)
{
  if (theNbArgs != 2)
  {
    theDI << "Wrong number of parameters. Use : meshinfo mesh\n";
    return 0;
  }

  Handle(MeshVS_Mesh) aMesh = getMesh(theArgVec[1], theDI);
  if (aMesh.IsNull())
  {
    theDI << "Mesh not found\n";
    return 0;
  }

  Handle(XSDRAWSTLVRML_DataSource) stlMeshSource = Handle(XSDRAWSTLVRML_DataSource)::DownCast(aMesh->GetDataSource());
  if (!stlMeshSource.IsNull())
  {
    const TColStd_PackedMapOfInteger& nodes = stlMeshSource->GetAllNodes();
    const TColStd_PackedMapOfInteger& tris = stlMeshSource->GetAllElements();

    theDI << "Nb nodes = " << nodes.Extent() << "\n";
    theDI << "Nb triangles = " << tris.Extent() << "\n";
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

  theCommands.Add("meshfromstl", "creates MeshVS_Mesh from STL file", __FILE__, createmesh, g);
  theCommands.Add("mesh3delem", "creates 3d element mesh to test", __FILE__, create3d, g);
  theCommands.Add("meshshadcolor", "change MeshVS_Mesh shading color", __FILE__, meshcolor, g);
  theCommands.Add("meshlinkcolor", "change MeshVS_Mesh line color", __FILE__, linecolor, g);
  theCommands.Add("meshmat", "change MeshVS_Mesh material and transparency", __FILE__, meshmat, g);
  theCommands.Add("meshshrcoef", "change MeshVS_Mesh shrink coeff", __FILE__, shrink, g);
  theCommands.Add("meshclosed", "meshclosed meshname (0/1) \nChange MeshVS_Mesh drawing mode. 0 - not closed object, 1 - closed object", __FILE__, closed, g);
  theCommands.Add("meshshow", "display MeshVS_Mesh object", __FILE__, mdisplay, g);
  theCommands.Add("meshhide", "erase MeshVS_Mesh object", __FILE__, merase, g);
  theCommands.Add("meshhidesel", "hide selected entities", __FILE__, hidesel, g);
  theCommands.Add("meshshowsel", "show only selected entities", __FILE__, showonly, g);
  theCommands.Add("meshshowall", "show all entities", __FILE__, showall, g);
  theCommands.Add("meshcolors", "display color presentation", __FILE__, meshcolors, g);
  theCommands.Add("meshvectors", "display sample vectors", __FILE__, meshvectors, g);
  theCommands.Add("meshtext", "display text labels", __FILE__, meshtext, g);
  theCommands.Add("meshdeform", "display deformed mesh", __FILE__, meshdeform, g);
  theCommands.Add("mesh_edge_width", "set width of edges", __FILE__, mesh_edge_width, g);
  theCommands.Add("meshinfo", "displays the number of nodes and triangles", __FILE__, meshinfo, g);
}

//=======================================================================
//function : Factory
//purpose  :
//=======================================================================
void XSDRAWSTLVRML::Factory(Draw_Interpretor& theDI)
{
  XSDRAWSTLVRML::InitCommands(theDI);
#ifdef OCCT_DEBUG
  theDI << "Draw Plugin : All TKXSDRAW commands are loaded\n";
#endif
}

// Declare entry point PLUGINFACTORY
DPLUGIN(XSDRAWSTLVRML)
