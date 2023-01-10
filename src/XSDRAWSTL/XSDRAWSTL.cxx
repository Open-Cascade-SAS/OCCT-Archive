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

#include <DBRep.hxx>
#include <DDocStd.hxx>
#include <DDocStd_DrawDocument.hxx>
#include <Draw.hxx>
#include <Draw_Interpretor.hxx>
#include <Draw_ProgressIndicator.hxx>
#include <RWStl_ConfigurationNode.hxx>
#include <RWStl_Provider.hxx>
#include <XSControl_WorkSession.hxx>
#include <XSDRAWBase.hxx>
#include <TDataStd_Name.hxx>
#include <TDocStd_Application.hxx>
#include <TopoDS_Shape.hxx>

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
  aNode->GlobalParameters.LengthUnit = XSDRAWBase::GetLengthUnit();
  Handle(RWStl_Provider) aProvider = new RWStl_Provider(aNode);
  aNode->InternalParameters.WriteAscii = isASCIIMode;
  Handle(XSControl_WorkSession) aWS = XSDRAWBase::Session();
  Handle(Draw_ProgressIndicator) aProgress = new Draw_ProgressIndicator(theDI);
  if (!aProvider->Write(theArgVec[2], aShape, aWS, aProgress->Start()))
  {
    theDI << "Error: Mesh writing has been failed.\n";
  }
  XSDRAWBase::CollectActiveWorkSessions(aWS, theArgVec[2], XSDRAWBase::WorkSessionList());
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
  aNode->GlobalParameters.LengthUnit = XSDRAWBase::GetLengthUnit();
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
  Handle(XSControl_WorkSession) aWS = XSDRAWBase::Session();
  if (!aProvider->Read(aFilePath, aShape, aWS, aProgress->Start()))
  {
    theDI << "Cannot read any relevant data from the STL file\n";
    return 1;
  }
  DBRep::Set(aShapeName.ToCString(), aShape);
  XSDRAWBase::CollectActiveWorkSessions(aWS, aFilePath, XSDRAWBase::WorkSessionList());
  return 0;
}

//=============================================================================
//function : Factory
//purpose  :
//=============================================================================
void XSDRAWSTL::Factory(Draw_Interpretor& theDI)
{
  const char* g = "XSTEP-STL/VRML";  // Step transfer file commands

  theDI.Add("writestl", "shape file [ascii/binary (0/1) : 1 by default] [InParallel (0/1) : 0 by default]", __FILE__, writestl, g);
  theDI.Add("readstl",
            "readstl shape file [-brep] [-mergeAngle Angle] [-multi]"
            "\n\t\t: Reads STL file and creates a new shape with specified name."
            "\n\t\t: When -brep is specified, creates a Compound of per-triangle Faces."
            "\n\t\t: Single triangulation-only Face is created otherwise (default)."
            "\n\t\t: -mergeAngle specifies maximum angle in degrees between triangles to merge equal nodes; disabled by default."
            "\n\t\t: -multi creates a face per solid in multi-domain files; ignored when -brep is set.",
            __FILE__, readstl, g);
}
