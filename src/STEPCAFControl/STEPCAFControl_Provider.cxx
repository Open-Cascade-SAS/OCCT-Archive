// Copyright (c) 2022 OPEN CASCADE SAS
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

#include <STEPCAFControl_Provider.hxx>

#include <BinXCAFDrivers.hxx>
#include <Interface_Static.hxx>
#include <Message.hxx>
#include <STEPControl_Controller.hxx>
#include <StepData_StepModel.hxx>
#include <STEPControl_ActorWrite.hxx>
#include <STEPCAFControl_ConfigurationNode.hxx>
#include <STEPCAFControl_Controller.hxx>
#include <STEPCAFControl_Reader.hxx>
#include <STEPCAFControl_Writer.hxx>
#include <TDocStd_Document.hxx>
#include <TDataStd_Name.hxx>
#include <TDF_Tool.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XSControl_WorkSession.hxx>
#include <UnitsMethods.hxx>

IMPLEMENT_STANDARD_RTTIEXT(STEPCAFControl_Provider, DE_Provider)

//=======================================================================
// function : STEPCAFControl_Provider
// purpose  :
//=======================================================================
STEPCAFControl_Provider::STEPCAFControl_Provider()
{}

//=======================================================================
// function : STEPCAFControl_Provider
// purpose  :
//=======================================================================
STEPCAFControl_Provider::STEPCAFControl_Provider(const Handle(DE_ConfigurationNode)& theNode)
  :DE_Provider(theNode)
{}

//=======================================================================
// function : STEPCAFControl_Provider
// purpose  :
//=======================================================================
void STEPCAFControl_Provider::personizeWS(Handle(XSControl_WorkSession)& theWS)
{
  if (theWS.IsNull())
  {
    Message::SendWarning() << "Warning: STEPCAFControl_Provider :"
      << " Null work session, use internal temporary session";
    theWS = new XSControl_WorkSession();
  }
  Handle(STEPControl_Controller) aCntrl =
    Handle(STEPControl_Controller)::DownCast(theWS->NormAdaptor());
  if (aCntrl.IsNull())
  {
    theWS->SelectNorm("STEP");
  }
}

//=======================================================================
// function : initStatic
// purpose  :
//=======================================================================
void STEPCAFControl_Provider::initStatic(const Handle(DE_ConfigurationNode)& theNode)
{
  Handle(STEPCAFControl_ConfigurationNode) aNode =
    Handle(STEPCAFControl_ConfigurationNode)::DownCast(theNode);
  STEPCAFControl_Controller::Init();
  if (!myToUpdateStaticParameters)
  {
    return;
  }
  // Get previous values
  myOldValues.ReadBSplineContinuity =
    (STEPCAFControl_ConfigurationNode::ReadMode_BSplineContinuity)
    Interface_Static::IVal("read.iges.bspline.continuity");
  myOldValues.ReadPrecisionMode =
    (STEPCAFControl_ConfigurationNode::ReadMode_Precision)
    Interface_Static::IVal("read.precision.mode");
  myOldValues.ReadPrecisionVal =
    Interface_Static::RVal("read.precision.val");
  myOldValues.ReadMaxPrecisionMode =
    (STEPCAFControl_ConfigurationNode::ReadMode_MaxPrecision)
    Interface_Static::IVal("read.maxprecision.mode");
  myOldValues.ReadMaxPrecisionVal =
    Interface_Static::RVal("read.maxprecision.val");
  myOldValues.ReadSameParamMode =
    Interface_Static::IVal("read.stdsameparameter.mode") == 1;
  myOldValues.ReadSurfaceCurveMode =
    (STEPCAFControl_ConfigurationNode::ReadMode_SurfaceCurve)
    Interface_Static::IVal("read.surfacecurve.mode");
  myOldValues.EncodeRegAngle =
    Interface_Static::RVal("read.encoderegularity.angle") * 180.0 / M_PI;
  myOldValues.AngleUnit =
    (STEPCAFControl_ConfigurationNode::AngleUnitMode)
    Interface_Static::IVal("step.angleunit.mode");

  myOldValues.ReadResourceName =
    Interface_Static::CVal("read.step.resource.name");
  myOldValues.ReadSequence =
    Interface_Static::CVal("read.step.sequence");
  myOldValues.ReadProductMode =
    Interface_Static::IVal("read.step.product.mode") == 1;
  myOldValues.ReadProductContext =
    (STEPCAFControl_ConfigurationNode::ReadMode_ProductContext)
    Interface_Static::IVal("read.step.product.context");
  myOldValues.ReadShapeRepr =
    (STEPCAFControl_ConfigurationNode::ReadMode_ShapeRepr)
    Interface_Static::IVal("read.step.shape.repr");
  myOldValues.ReadTessellated =
    (STEPCAFControl_ConfigurationNode::RWMode_Tessellated)
    Interface_Static::IVal("read.step.tessellated");
  myOldValues.ReadAssemblyLevel =
    (STEPCAFControl_ConfigurationNode::ReadMode_AssemblyLevel)
    Interface_Static::IVal("read.step.assembly.level");
  myOldValues.ReadRelationship =
    Interface_Static::IVal("read.step.shape.relationship") == 1;
  myOldValues.ReadShapeAspect =
    Interface_Static::IVal("read.step.shape.aspect") == 1;
  myOldValues.ReadConstrRelation =
    Interface_Static::IVal("read.step.constructivegeom.relationship") == 1;
  myOldValues.ReadSubshapeNames =
    Interface_Static::IVal("read.stepcaf.subshapes.name") == 1;
  myOldValues.ReadCodePage =
    (Resource_FormatType)Interface_Static::IVal("read.step.codepage");
  myOldValues.ReadNonmanifold =
    Interface_Static::IVal("read.step.nonmanifold") == 1;
  myOldValues.ReadIdeas =
    Interface_Static::IVal("read.step.ideas") == 1;
  myOldValues.ReadAllShapes =
    Interface_Static::IVal("read.step.all.shapes") == 1;
  myOldValues.ReadRootTransformation =
    Interface_Static::IVal("read.step.root.transformation") == 1;

  myOldValues.WritePrecisionMode =
    (STEPCAFControl_ConfigurationNode::WriteMode_PrecisionMode)
    Interface_Static::IVal("write.precision.mode");
  myOldValues.WritePrecisionVal =
    Interface_Static::RVal("write.precision.val");
  myOldValues.WriteAssembly =
    (STEPCAFControl_ConfigurationNode::WriteMode_Assembly)
    Interface_Static::IVal("write.step.assembly");
  myOldValues.WriteSchema =
    (STEPCAFControl_ConfigurationNode::WriteMode_StepSchema)
    Interface_Static::IVal("write.step.schema");
  myOldValues.WriteTessellated =
    (STEPCAFControl_ConfigurationNode::RWMode_Tessellated)
    Interface_Static::IVal("write.step.tessellated");
  myOldValues.WriteProductName =
    Interface_Static::CVal("write.step.product.name");
  myOldValues.WriteSurfaceCurMode =
    Interface_Static::IVal("write.surfacecurve.mode") == 1;
  myOldValues.WriteUnit =
    (UnitsMethods_LengthUnit)Interface_Static::IVal("write.step.unit");
  myOldValues.WriteResourceName =
    Interface_Static::CVal("write.resource.name");
  myOldValues.WriteSequence =
    Interface_Static::CVal("write.step.sequence");
  myOldValues.WriteVertexMode =
    (STEPCAFControl_ConfigurationNode::WriteMode_VertexMode)
    Interface_Static::IVal("write.step.vertex.mode");
  myOldValues.WriteSubshapeNames =
    Interface_Static::IVal("write.stepcaf.subshapes.name") == 1;

  // Set new values
  setStatic(aNode->InternalParameters);
}

//=======================================================================
// function : setStatic
// purpose  :
//=======================================================================
void STEPCAFControl_Provider::setStatic(const STEPCAFControl_ConfigurationNode::STEPCAFControl_InternalSection theParameter)
{
  Interface_Static::SetIVal("read.iges.bspline.continuity",
                            theParameter.ReadBSplineContinuity);
  Interface_Static::SetIVal("read.precision.mode",
                            theParameter.ReadPrecisionMode);
  Interface_Static::SetRVal("read.precision.val",
                            theParameter.ReadPrecisionVal);
  Interface_Static::SetIVal("read.maxprecision.mode",
                            theParameter.ReadMaxPrecisionMode);
  Interface_Static::SetRVal("read.maxprecision.val",
                            theParameter.ReadMaxPrecisionVal);
  Interface_Static::SetIVal("read.stdsameparameter.mode",
                            theParameter.ReadSameParamMode);
  Interface_Static::SetIVal("read.surfacecurve.mode",
                            theParameter.ReadSurfaceCurveMode);
  Interface_Static::SetRVal("read.encoderegularity.angle",
                            theParameter.EncodeRegAngle * M_PI / 180.0);
  Interface_Static::SetIVal("step.angleunit.mode",
                            theParameter.AngleUnit);

  Interface_Static::SetCVal("read.step.resource.name",
                            theParameter.ReadResourceName.ToCString());
  Interface_Static::SetCVal("read.step.sequence",
                            theParameter.ReadSequence.ToCString());
  Interface_Static::SetIVal("read.step.product.mode",
                            theParameter.ReadProductMode);
  Interface_Static::SetIVal("read.step.product.context",
                            theParameter.ReadProductContext);
  Interface_Static::SetIVal("read.step.shape.repr",
                            theParameter.ReadShapeRepr);
  Interface_Static::SetIVal("read.step.tessellated",
                            theParameter.ReadTessellated);
  Interface_Static::SetIVal("read.step.assembly.level",
                            theParameter.ReadAssemblyLevel);
  Interface_Static::SetIVal("read.step.shape.relationship",
                            theParameter.ReadRelationship);
  Interface_Static::SetIVal("read.step.shape.aspect",
                            theParameter.ReadShapeAspect);
  Interface_Static::SetIVal("read.step.constructivegeom.relationship",
                            theParameter.ReadConstrRelation);
  Interface_Static::SetIVal("read.stepcaf.subshapes.name",
                            theParameter.ReadSubshapeNames);
  Interface_Static::SetIVal("read.step.codepage",
                            theParameter.ReadCodePage);
  Interface_Static::SetIVal("read.step.nonmanifold",
                            theParameter.ReadNonmanifold);
  Interface_Static::SetIVal("read.step.ideas",
                            theParameter.ReadIdeas);
  Interface_Static::SetIVal("read.step.all.shapes",
                            theParameter.ReadAllShapes);
  Interface_Static::SetIVal("read.step.root.transformation",
                            theParameter.ReadRootTransformation);

  Interface_Static::SetIVal("write.precision.mode",
                            theParameter.WritePrecisionMode);
  Interface_Static::SetRVal("write.precision.val",
                            theParameter.WritePrecisionVal);
  Interface_Static::SetIVal("write.step.assembly",
                            theParameter.WriteAssembly);
  Interface_Static::SetIVal("write.step.schema",
                            theParameter.WriteSchema);
  Interface_Static::SetIVal("write.step.tessellated",
                            theParameter.WriteTessellated);
  Interface_Static::SetCVal("write.step.product.name",
                            theParameter.WriteProductName.ToCString());
  Interface_Static::SetIVal("write.surfacecurve.mode",
                            theParameter.WriteSurfaceCurMode);
  Interface_Static::SetIVal("write.step.unit",
                            theParameter.WriteUnit);
  Interface_Static::SetCVal("write.resource.name",
                            theParameter.WriteResourceName.ToCString());
  Interface_Static::SetCVal("write.step.sequence",
                            theParameter.WriteSequence.ToCString());
  Interface_Static::SetIVal("write.step.vertex.mode",
                            theParameter.WriteVertexMode);
  Interface_Static::SetIVal("write.stepcaf.subshapes.name",
                            theParameter.WriteSubshapeNames);
}

//=======================================================================
// function : resetStatic
// purpose  :
//=======================================================================
void STEPCAFControl_Provider::resetStatic()
{
  if (!myToUpdateStaticParameters)
  {
    return;
  }
  setStatic(myOldValues);
}

//=======================================================================
// function : Read
// purpose  :
//=======================================================================
bool STEPCAFControl_Provider::Read(const TCollection_AsciiString& thePath,
                                   const Handle(TDocStd_Document)& theDocument,
                                   Handle(XSControl_WorkSession)& theWS,
                                   const Message_ProgressRange& theProgress)
{
  if (theDocument.IsNull())
  {
    Message::SendFail() << "Error: STEPCAFControl_Provider : "
      << "Null document";
    return false;
  }
  if (GetNode().IsNull() ||
      !GetNode()->IsKind(STANDARD_TYPE(STEPCAFControl_ConfigurationNode)))
  {
    Message::SendFail() << "Error: STEPCAFControl_Provider : "
      << "Incorrect or empty Configuration Node";
    return false;
  }
  Handle(STEPCAFControl_ConfigurationNode) aNode =
    Handle(STEPCAFControl_ConfigurationNode)::DownCast(GetNode());
  initStatic(aNode);

  personizeWS(theWS);
  XCAFDoc_DocumentTool::SetLengthUnit(theDocument,
                                      aNode->GlobalParameters.LengthUnit,
                                      UnitsMethods_LengthUnit_Millimeter);
  const Standard_Boolean toUseLoaded = thePath == ".";
  TCollection_AsciiString aFile;
  if (toUseLoaded)
  {
    aFile = theWS->LoadedFile();
    Message::SendInfo() << "Model taken from the STEP session : "
      << aFile;
  }
  else
  {
    aFile = thePath;
    Message::SendInfo() << "File STEP to read : "
      << aFile;
  }
  STEPCAFControl_Reader aReader(theWS, !toUseLoaded);
  aReader.SetColorMode(aNode->InternalParameters.ReadColor);
  aReader.SetNameMode(aNode->InternalParameters.ReadName);
  aReader.SetLayerMode(aNode->InternalParameters.ReadLayer);
  aReader.SetPropsMode(aNode->InternalParameters.ReadProps);
  XSControl_ReturnStatus aReadStat = XSControl_RetVoid;
  if (!toUseLoaded)
  {
    aReadStat = aReader.ReadFile(thePath.ToCString());
  }
  else if (theWS->NbStartingEntities() > 0)
  {
    aReadStat = XSControl_RetDone;
  }
  if (aReadStat != XSControl_RetDone)
  {
    Message::SendFail() << "Error: STEPCAFControl_Provider : ["
      << aFile << "] : abandon, no model loaded";
    resetStatic();
    return false;
  }
  if (!aReader.Transfer(theDocument, theProgress))
  {
    Message::SendFail() << "Error: STEPCAFControl_Provider : [" <<
      aFile << "] : Cannot read any relevant data from the STEP file";
    resetStatic();
    return false;
  }
  resetStatic();
  return true;
}


//=======================================================================
// function : Read
// purpose  :
//=======================================================================
bool STEPCAFControl_Provider::Read(std::istream& theIStream,
                                   const Handle(TDocStd_Document)& theDocument,
                                   const TCollection_AsciiString theName,
                                   Handle(XSControl_WorkSession)& theWS,
                                   const Message_ProgressRange& theProgress)
{
  if (theDocument.IsNull())
  {
    Message::SendFail() << "Error: STEPCAFControl_Provider : "
      << "Null document";
    return false;
  }
  if (GetNode().IsNull() ||
      !GetNode()->IsKind(STANDARD_TYPE(STEPCAFControl_ConfigurationNode)))
  {
    Message::SendFail() << "Error: STEPCAFControl_Provider : "
      << "Incorrect or empty Configuration Node";
    return false;
  }
  Handle(STEPCAFControl_ConfigurationNode) aNode =
    Handle(STEPCAFControl_ConfigurationNode)::DownCast(GetNode());
  initStatic(aNode);

  personizeWS(theWS);
  XCAFDoc_DocumentTool::SetLengthUnit(theDocument,
                                      aNode->GlobalParameters.LengthUnit,
                                      UnitsMethods_LengthUnit_Millimeter);
  Message::SendInfo() << "Model taken from the STEP stream";
  STEPCAFControl_Reader aReader(theWS);
  aReader.SetColorMode(aNode->InternalParameters.ReadColor);
  aReader.SetNameMode(aNode->InternalParameters.ReadName);
  aReader.SetLayerMode(aNode->InternalParameters.ReadLayer);
  aReader.SetPropsMode(aNode->InternalParameters.ReadProps);
  XSControl_ReturnStatus aReadStat = XSControl_RetVoid;
  aReadStat = aReader.ReadStream(theName.ToCString(), theIStream);
  if (aReadStat != XSControl_RetDone)
  {
    Message::SendFail() << "Error: STEPCAFControl_Provider : "
      << "Abandon, no model loaded via stream";
    resetStatic();
    return false;
  }
  if (!aReader.Transfer(theDocument, theProgress))
  {
    Message::SendFail() << "Error: STEPCAFControl_Provider : "
      << "Cannot read any relevant data from the STEP file";
    resetStatic();
    return false;
  }
  resetStatic();
  return true;
}

//=======================================================================
// function : Write
// purpose  :
//=======================================================================
bool STEPCAFControl_Provider::Write(const TCollection_AsciiString& thePath,
                                    const Handle(TDocStd_Document)& theDocument,
                                    Handle(XSControl_WorkSession)& theWS,
                                    const Message_ProgressRange& theProgress)
{
  if (GetNode().IsNull() ||
      !GetNode()->IsKind(STANDARD_TYPE(STEPCAFControl_ConfigurationNode)))
  {
    Message::SendFail() << "Error: STEPCAFControl_Provider : "
      << "Incorrect or empty Configuration Node";
    return false;
  }
  Handle(STEPCAFControl_ConfigurationNode) aNode =
    Handle(STEPCAFControl_ConfigurationNode)::DownCast(GetNode());
  initStatic(aNode);
  XCAFDoc_DocumentTool::SetLengthUnit(theDocument,
                                      UnitsMethods::GetLengthUnitScale(
                                      aNode->InternalParameters.WriteUnit,
                                      UnitsMethods_LengthUnit_Millimeter),
                                      UnitsMethods_LengthUnit_Millimeter);
  personizeWS(theWS);
  STEPCAFControl_Writer aWriter(theWS, Standard_True);
  STEPControl_StepModelType aMode =
    static_cast<STEPControl_StepModelType>(aNode->InternalParameters.WriteModelType);
  aWriter.SetColorMode(aNode->InternalParameters.WriteColor);
  aWriter.SetNameMode(aNode->InternalParameters.WriteName);
  aWriter.SetLayerMode(aNode->InternalParameters.WriteLayer);
  aWriter.SetPropsMode(aNode->InternalParameters.WriteProps);

  TDF_LabelSequence aLabels;
  TCollection_AsciiString aLabelsString;
  for (TColStd_SequenceOfAsciiString::Iterator anIter(aNode->InternalParameters.WriteLabels);
       anIter.More(); anIter.Next())
  {
    const TCollection_AsciiString& aValue = anIter.Value();
    TDF_Label aLabel;
    TDF_Tool::Label(theDocument->Main().Data(), aValue, aLabel, Standard_False);
    if (aLabel.IsNull())
    {
      Message::SendFail() << "Error: No label for entry '" << aValue << "'";
      return false;
    }
    if (!aLabelsString.IsEmpty())
    {
      aLabelsString += " ";
    }
    aLabelsString += aValue;
    aLabels.Append(aLabel);
  }
  TCollection_ExtendedString aDocName;
  Handle(TDataStd_Name) aNameAttr;
  if (theDocument->GetData()->Root().FindAttribute(TDataStd_Name::GetID(), aNameAttr))
  {
    aDocName = aNameAttr->Get();
  }
  Standard_Boolean aTransferStatus = Standard_True;
  Standard_CString aMultiFilePrefix = !aNode->InternalParameters.WriteMultiPrefix.IsEmpty() ?
    aNode->InternalParameters.WriteMultiPrefix.ToCString() : nullptr;
  Message::SendInfo() << "Writing STEP file "
    << thePath;
  if (aLabels.IsEmpty())
  {
    Message::SendInfo() << "Translating labels "
      << aLabelsString << " of document " << aDocName << " to STEP";
    aTransferStatus = aWriter.Transfer(theDocument, aMode, aMultiFilePrefix, theProgress);
  }
  else
  {
    Message::SendInfo() << "Translating document "
      << aDocName << " to STEP";
    aTransferStatus = aWriter.Transfer(aLabels, aMode, aMultiFilePrefix, theProgress);
  }
  if (!aTransferStatus)
  {
    Message::SendFail() << "Error: STEPCAFControl_Provider : "
      << "The document cannot be translated or gives no result";
    resetStatic();
    return false;
  }
  if (thePath == ".")
  {
    resetStatic();
    Message::SendInfo() << "Document has been translated into the session";
    return true;
  }
  if (aWriter.Write(thePath.ToCString()) != XSControl_RetDone)
  {
    Message::SendFail() << "Error: STEPCAFControl_Provider : [" <<
      thePath << "] : Write failed";
    resetStatic();
    return false;
  }
  Message::SendInfo() << "STEP file [" << thePath << "] Successfully written";
  resetStatic();
  return true;
}

//=======================================================================
// function : Write
// purpose  :
//=======================================================================
bool STEPCAFControl_Provider::Write(std::ostream& theOStream,
                                    const Handle(TDocStd_Document)& theDocument,
                                    Handle(XSControl_WorkSession)& theWS,
                                    const Message_ProgressRange& theProgress)
{
  if (GetNode().IsNull() ||
      !GetNode()->IsKind(STANDARD_TYPE(STEPCAFControl_ConfigurationNode)))
  {
    Message::SendFail() << "Error: STEPCAFControl_Provider : "
      << "Incorrect or empty Configuration Node";
    return false;
  }
  Handle(STEPCAFControl_ConfigurationNode) aNode =
    Handle(STEPCAFControl_ConfigurationNode)::DownCast(GetNode());
  initStatic(aNode);
  personizeWS(theWS);
  STEPCAFControl_Writer aWriter(theWS, Standard_True);
  STEPControl_StepModelType aMode =
    static_cast<STEPControl_StepModelType>(aNode->InternalParameters.WriteModelType);
  aWriter.SetColorMode(aNode->InternalParameters.WriteColor);
  aWriter.SetNameMode(aNode->InternalParameters.WriteName);
  aWriter.SetLayerMode(aNode->InternalParameters.WriteLayer);
  aWriter.SetPropsMode(aNode->InternalParameters.WriteProps);
  Handle(StepData_StepModel) aModel = aWriter.ChangeWriter().Model();
  aModel->SetWriteLengthUnit(UnitsMethods::GetLengthUnitScale(
    aNode->InternalParameters.WriteUnit,
    UnitsMethods_LengthUnit_Millimeter));
  TDF_LabelSequence aLabels;
  TCollection_AsciiString aLabelsString;
  for (TColStd_SequenceOfAsciiString::Iterator anIter(aNode->InternalParameters.WriteLabels);
       anIter.More(); anIter.Next())
  {
    const TCollection_AsciiString& aValue = anIter.Value();
    TDF_Label aLabel;
    TDF_Tool::Label(theDocument->Main().Data(), aValue, aLabel, Standard_False);
    if (aLabel.IsNull())
    {
      Message::SendFail() << "Error: No label for entry '" << aValue << "'";
      return false;
    }
    if (!aLabelsString.IsEmpty())
    {
      aLabelsString += " ";
    }
    aLabelsString += aValue;
    aLabels.Append(aLabel);
  }
  TCollection_ExtendedString aDocName;
  Handle(TDataStd_Name) aNameAttr;
  if (theDocument->GetData()->Root().FindAttribute(TDataStd_Name::GetID(), aNameAttr))
  {
    aDocName = aNameAttr->Get();
  }
  Standard_Boolean aTransferStatus = Standard_True;
  Standard_CString aMultiFilePrefix = !aNode->InternalParameters.WriteMultiPrefix.IsEmpty() ?
    aNode->InternalParameters.WriteMultiPrefix.ToCString() : nullptr;
  Message::SendInfo() << "Writing STEP file to stream";
  if (aLabels.IsEmpty())
  {
    Message::SendInfo() << "Translating labels "
      << aLabelsString << " of document " << aDocName << " to STEP";
    aTransferStatus = aWriter.Transfer(theDocument, aMode, aMultiFilePrefix, theProgress);
  }
  else
  {
    Message::SendInfo() << "Translating document "
      << aDocName << " to STEP";
    aTransferStatus = aWriter.Transfer(aLabels, aMode, aMultiFilePrefix, theProgress);
  }
  if (!aTransferStatus)
  {
    Message::SendFail() << "Error: STEPCAFControl_Provider : "
      << "The document cannot be translated or gives no result";
    resetStatic();
    return false;
  }
  if (aWriter.WriteStream(theOStream) != XSControl_RetDone)
  {
    Message::SendFail() << "Error: STEPCAFControl_Provider : Write to stream failed";
    resetStatic();
    return false;
  }
  Message::SendInfo() << "STEP file to stream successfully written";
  resetStatic();
  return true;
}

//=======================================================================
// function : Read
// purpose  :
//=======================================================================
bool STEPCAFControl_Provider::Read(const TCollection_AsciiString& thePath,
                                   TopoDS_Shape& theShape,
                                   Handle(XSControl_WorkSession)& theWS,
                                   const Message_ProgressRange& theProgress)
{
  (void)theProgress;
  if (GetNode().IsNull() ||
      !GetNode()->IsKind(STANDARD_TYPE(STEPCAFControl_ConfigurationNode)))
  {
    Message::SendFail() << "Error: STEPCAFControl_Provider : "
      << "Incorrect or empty Configuration Node";
    return false;
  }
  Handle(STEPCAFControl_ConfigurationNode) aNode =
    Handle(STEPCAFControl_ConfigurationNode)::DownCast(GetNode());
  initStatic(aNode);
  personizeWS(theWS);
  STEPControl_Reader aReader(theWS);
  if (aReader.ReadFile(thePath.ToCString()) != XSControl_RetDone)
  {
    Message::SendFail() << "Error: STEPCAFControl_Provider : ["
      << thePath << "] : abandon, no model loaded";
    resetStatic();
    return false;
  }
  Handle(StepData_StepModel) aModel = Handle(StepData_StepModel)::DownCast(aReader.Model());
  aModel->SetLocalLengthUnit(aNode->GlobalParameters.LengthUnit);
  if (aReader.TransferRoots() <= 0)
  {
    Message::SendFail() << "Error: STEPCAFControl_Provider : [" <<
      thePath << "] : Cannot read any relevant data from the STEP file";
    resetStatic();
    return false;
  }
  theShape = aReader.OneShape();
  resetStatic();
  return true;
}

//=======================================================================
// function : Read
// purpose  :
//=======================================================================
bool STEPCAFControl_Provider::Read(std::istream& theIStream,
                                   TopoDS_Shape& theShape,
                                   const TCollection_AsciiString theName,
                                   Handle(XSControl_WorkSession)& theWS,
                                   const Message_ProgressRange& theProgress)
{
  (void)theProgress;
  if (GetNode().IsNull() ||
      !GetNode()->IsKind(STANDARD_TYPE(STEPCAFControl_ConfigurationNode)))
  {
    Message::SendFail() << "Error: STEPCAFControl_Provider : "
      << "Incorrect or empty Configuration Node";
    return false;
  }
  Handle(STEPCAFControl_ConfigurationNode) aNode =
    Handle(STEPCAFControl_ConfigurationNode)::DownCast(GetNode());
  initStatic(aNode);
  personizeWS(theWS);
  STEPControl_Reader aReader(theWS);
  if (aReader.ReadStream(theName.ToCString(), theIStream) != XSControl_RetDone)
  {
    Message::SendFail() << "Error: STEPCAFControl_Provider : "
      << "Abandon, no model loaded from STEP stream";
    resetStatic();
    return false;
  }
  Handle(StepData_StepModel) aModel = Handle(StepData_StepModel)::DownCast(aReader.Model());
  aModel->SetLocalLengthUnit(aNode->GlobalParameters.LengthUnit);
  if (aReader.TransferRoots() <= 0)
  {
    Message::SendFail() << "Error: STEPCAFControl_Provider : "
      << "Cannot read any relevant data from the STEP stream";
    resetStatic();
    return false;
  }
  theShape = aReader.OneShape();
  resetStatic();
  return true;
}

//=======================================================================
// function : Write
// purpose  :
//=======================================================================
bool STEPCAFControl_Provider::Write(const TCollection_AsciiString& thePath,
                                    const TopoDS_Shape& theShape,
                                    Handle(XSControl_WorkSession)& theWS,
                                    const Message_ProgressRange& theProgress)
{
  if (GetNode().IsNull() ||
      !GetNode()->IsKind(STANDARD_TYPE(STEPCAFControl_ConfigurationNode)))
  {
    Message::SendFail() << "Error: STEPCAFControl_Provider : "
      << "Incorrect or empty Configuration Node";
    return false;
  }
  Handle(STEPCAFControl_ConfigurationNode) aNode =
    Handle(STEPCAFControl_ConfigurationNode)::DownCast(GetNode());
  initStatic(aNode);

  personizeWS(theWS);
  STEPControl_Writer aWriter(theWS, Standard_True);
  Handle(StepData_StepModel) aModel = aWriter.Model();
  Standard_Integer aNbEntities = (aModel.IsNull() ? 0 : aModel->NbEntities());
  aModel->SetWriteLengthUnit(UnitsMethods::GetLengthUnitScale(
    aNode->InternalParameters.WriteUnit,
    UnitsMethods_LengthUnit_Millimeter));
  XSControl_ReturnStatus aWritestat =
    aWriter.Transfer(theShape, aNode->InternalParameters.WriteModelType, true, theProgress);
  if (aNbEntities > 0)
  {
    Message::SendTrace() << "STEPCAFControl_Provider : Model not empty before transferring";
  }
  if (aWritestat != XSControl_RetDone)
  {
    Message::SendFail() << "Error: STEPCAFControl_Provider : "
      << "Can't translate shape to STEP model";
    resetStatic();
    return false;
  }
  if (thePath == ".")
  {
    resetStatic();
    Message::SendInfo() << "Step model has been translated into the session";
    return true;
  }
  if (aWriter.Write(thePath.ToCString()) != XSControl_RetDone)
  {
    Message::SendFail() << "Error: STEPCAFControl_Provider : "
      << "Can't write STEP file " << thePath;
    resetStatic();
    return false;
  }
  resetStatic();
  return true;
}

//=======================================================================
// function : Write
// purpose  :
//=======================================================================
bool STEPCAFControl_Provider::Write(std::ostream& theOStream,
                                    const TopoDS_Shape& theShape,
                                    Handle(XSControl_WorkSession)& theWS,
                                    const Message_ProgressRange& theProgress)
{
  if (GetNode().IsNull() ||
      !GetNode()->IsKind(STANDARD_TYPE(STEPCAFControl_ConfigurationNode)))
  {
    Message::SendFail() << "Error: STEPCAFControl_Provider : "
      << "Incorrect or empty Configuration Node";
    return false;
  }
  Handle(STEPCAFControl_ConfigurationNode) aNode =
    Handle(STEPCAFControl_ConfigurationNode)::DownCast(GetNode());
  initStatic(aNode);

  personizeWS(theWS);
  STEPControl_Writer aWriter(theWS, Standard_True);
  Handle(StepData_StepModel) aModel = aWriter.Model();
  Standard_Integer aNbEntities = (aModel.IsNull() ? 0 : aModel->NbEntities());
  aModel->SetWriteLengthUnit(UnitsMethods::GetLengthUnitScale(
    aNode->InternalParameters.WriteUnit,
    UnitsMethods_LengthUnit_Millimeter));
  XSControl_ReturnStatus aWritestat =
    aWriter.Transfer(theShape, aNode->InternalParameters.WriteModelType, true, theProgress);
  if (aNbEntities > 0)
  {
    Message::SendTrace() << "STEPCAFControl_Provider : Model not empty before transferring";
  }
  if (aWritestat != XSControl_RetDone)
  {
    Message::SendFail() << "Error: STEPCAFControl_Provider : "
      << "Can't translate shape to STEP model";
    resetStatic();
    return false;
  }
  if (aWriter.WriteStream(theOStream) != XSControl_RetDone)
  {
    Message::SendFail() << "Error: STEPCAFControl_Provider : "
      << "Can't write STEP to stream";
    resetStatic();
    return false;
  }
  resetStatic();
  return true;
}

//=======================================================================
// function : GetFormat
// purpose  :
//=======================================================================
TCollection_AsciiString STEPCAFControl_Provider::GetFormat() const
{
  return TCollection_AsciiString("STEP");
}

//=======================================================================
// function : GetVendor
// purpose  :
//=======================================================================
TCollection_AsciiString STEPCAFControl_Provider::GetVendor() const
{
  return TCollection_AsciiString("OCC");
}
