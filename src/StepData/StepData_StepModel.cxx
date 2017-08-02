// Copyright (c) 1999-2014 OPEN CASCADE SAS
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


#include <Interface_Check.hxx>
#include <Interface_CopyTool.hxx>
#include <Interface_EntityIterator.hxx>
#include <Interface_GeneralLib.hxx>
#include <Interface_GeneralModule.hxx>
#include <Interface_InterfaceModel.hxx>
#include <Interface_Macros.hxx>
#include <Interface_ShareTool.hxx>
#include <Message_Messenger.hxx>
#include <Standard_NoSuchObject.hxx>
#include <Standard_Transient.hxx>
#include <Standard_Type.hxx>
#include <StepData.hxx>
#include <StepData_Protocol.hxx>
#include <StepData_StepModel.hxx>
#include <StepData_StepWriter.hxx>
#include <TCollection_HAsciiString.hxx>

#include <stdio.h>
IMPLEMENT_STANDARD_RTTIEXT(StepData_StepModel,Interface_InterfaceModel)

// Entete de fichier : liste d entites
StepData_StepModel::StepData_StepModel ()
{
  initParameters();
}

//=======================================================================
//function : initParameters
//purpose  : 
//=======================================================================
void StepData_StepModel::initParameters()
{
  //for each created item, value from static map should be
  //checked, if static map is bound with this parameter than get value from static
  Handle(Interface_Static) aStaticItem;
  Handle(Interface_Static) anItem = new Interface_Static("step", "write.step.product.name", Interface_ParamText, "");
  myParamMap.Bind("write.step.product.name", anItem);

  anItem = new Interface_Static("step", "write.step.assembly", Interface_ParamEnum, "");
  Interface_Static::InitValues(anItem, "enum 0");
  Interface_Static::InitValues(anItem, "eval Off");
  Interface_Static::InitValues(anItem, "eval On");
  Interface_Static::InitValues(anItem, "eval Auto");
  aStaticItem = Interface_Static::Static("write.step.assembly");
  anItem->SetCStringValue(aStaticItem.IsNull() ? "Auto" : aStaticItem->CStringValue());
  myParamMap.Bind("write.step.assembly", anItem);

  anItem = new Interface_Static("step", "step.angleunit.mode", Interface_ParamEnum, "");
  Interface_Static::InitValues(anItem, "enum 0");
  Interface_Static::InitValues(anItem, "eval File");
  Interface_Static::InitValues(anItem, "eval Rad");
  Interface_Static::InitValues(anItem, "eval Deg");
  aStaticItem = Interface_Static::Static("step.angleunit.mode");
  anItem->SetCStringValue(aStaticItem.IsNull() ? "File" : aStaticItem->CStringValue());
  myParamMap.Bind("step.angleunit.mode", anItem);

  anItem = new Interface_Static("step", "write.step.schema", Interface_ParamEnum, "");
  Interface_Static::InitValues(anItem, "enum 1");
  Interface_Static::InitValues(anItem, "eval AP214CD");
  Interface_Static::InitValues(anItem, "eval AP214DIS");
  Interface_Static::InitValues(anItem, "eval AP203");
  Interface_Static::InitValues(anItem, "eval AP214IS");
  Interface_Static::InitValues(anItem, "eval AP242DIS");
  aStaticItem = Interface_Static::Static("write.step.schema");
  anItem->SetCStringValue(aStaticItem.IsNull() ? "AP214IS" : aStaticItem->CStringValue());
  myParamMap.Bind("write.step.schema", anItem);

  // Type of Product Definition for reading
  // Note: the numbers should be consistent with function FindShapeReprType()
  // in STEPControl_ActorRead.cxx
  anItem = new Interface_Static("step", "read.step.shape.repr", Interface_ParamEnum, "");
  Interface_Static::InitValues(anItem, "enum 1");
  Interface_Static::InitValues(anItem, "eval All");   // 1
  Interface_Static::InitValues(anItem, "eval ABSR");  // 2
  Interface_Static::InitValues(anItem, "eval MSSR");  // 3
  Interface_Static::InitValues(anItem, "eval GBSSR"); // 4
  Interface_Static::InitValues(anItem, "eval FBSR");  // 5
  Interface_Static::InitValues(anItem, "eval EBWSR"); // 6
  Interface_Static::InitValues(anItem, "eval GBWSR"); // 7
  aStaticItem = Interface_Static::Static("read.step.shape.repr");
  anItem->SetCStringValue(aStaticItem.IsNull() ? "All" : aStaticItem->CStringValue());
  myParamMap.Bind("read.step.shape.repr", anItem);

  // Mode for reading shapes attached to main SDR by SRR
  // (hybrid model representation in AP203 since 1998)
  anItem = new Interface_Static("step", "read.step.shape.relationship", Interface_ParamEnum, "");
  Interface_Static::InitValues(anItem, "enum 0");
  Interface_Static::InitValues(anItem, "eval OFF");
  Interface_Static::InitValues(anItem, "eval ON");
  aStaticItem = Interface_Static::Static("read.step.shape.relationship");
  anItem->SetCStringValue(aStaticItem.IsNull() ? "ON" : aStaticItem->CStringValue());
  myParamMap.Bind("read.step.shape.relationship", anItem);

  // Mode for reading shapes attached to Product by ShapeAspect 
  // (hybrid model representation in AP203 before 1998)
  anItem = new Interface_Static("step", "read.step.shape.aspect", Interface_ParamEnum, "");
  Interface_Static::InitValues(anItem, "enum 0");
  Interface_Static::InitValues(anItem, "eval OFF");
  Interface_Static::InitValues(anItem, "eval ON");
  aStaticItem = Interface_Static::Static("read.step.shape.aspect");
  anItem->SetCStringValue(aStaticItem.IsNull() ? "ON" : aStaticItem->CStringValue());
  myParamMap.Bind("read.step.shape.aspect", anItem);

  // Mode for reading SDR and ShapeRepr if it is necessary
  anItem = new Interface_Static("step", "read.step.product.mode", Interface_ParamEnum, "");
  Interface_Static::InitValues(anItem, "enum 0");
  Interface_Static::InitValues(anItem, "eval OFF");
  Interface_Static::InitValues(anItem, "eval ON");
  aStaticItem = Interface_Static::Static("read.step.product.mode");
  anItem->SetCStringValue(aStaticItem.IsNull() ? "ON" : aStaticItem->CStringValue());
  myParamMap.Bind("read.step.product.mode", anItem);

  // Order of reading ShapeDefinitionRepresentation in ProductDefinition
  anItem = new Interface_Static("step", "read.step.product.context", Interface_ParamEnum, "");
  Interface_Static::InitValues(anItem, "enum 1");
  Interface_Static::InitValues(anItem, "eval all");     // 1
  Interface_Static::InitValues(anItem, "eval design");  // 2
  Interface_Static::InitValues(anItem, "eval analysis");// 3
  aStaticItem = Interface_Static::Static("read.step.product.context");
  anItem->SetCStringValue(aStaticItem.IsNull() ? "all" : aStaticItem->CStringValue());
  myParamMap.Bind("read.step.product.context", anItem);

  // What we try to read in ProductDefinition
  anItem = new Interface_Static("step", "read.step.assembly.level", Interface_ParamEnum, "");
  Interface_Static::InitValues(anItem, "enum 1");
  Interface_Static::InitValues(anItem, "eval all");      // 1
  Interface_Static::InitValues(anItem, "eval assembly"); // 2
  Interface_Static::InitValues(anItem, "eval structure");// 3
  Interface_Static::InitValues(anItem, "eval shape");    // 4
  aStaticItem = Interface_Static::Static("read.step.assembly.level");
  anItem->SetCStringValue(aStaticItem.IsNull() ? "all" : aStaticItem->CStringValue());
  myParamMap.Bind("read.step.assembly.level", anItem);

  // unit: supposed to be cascade unit (target unit for reading)
  anItem = new Interface_Static("step", "write.step.unit", Interface_ParamEnum, "");
  Interface_Static::InitValues(anItem, "enum 1");
  Interface_Static::InitValues(anItem, "eval INCH");  // 1
  Interface_Static::InitValues(anItem, "eval MM");    // 2
  Interface_Static::InitValues(anItem, "eval ??");    // 3
  Interface_Static::InitValues(anItem, "eval FT");    // 4
  Interface_Static::InitValues(anItem, "eval MI");    // 5
  Interface_Static::InitValues(anItem, "eval M");     // 6
  Interface_Static::InitValues(anItem, "eval KM");    // 7
  Interface_Static::InitValues(anItem, "eval MIL");   // 8
  Interface_Static::InitValues(anItem, "eval UM");    // 9
  Interface_Static::InitValues(anItem, "eval CM");    //10
  Interface_Static::InitValues(anItem, "eval UIN");   //11
  aStaticItem = Interface_Static::Static("write.step.unit");
  anItem->SetCStringValue(aStaticItem.IsNull() ? "MM" : aStaticItem->CStringValue());
  myParamMap.Bind("write.step.unit", anItem);

  // Non-manifold topology reading: OFF by default (ssv; 26.11.2010)
  anItem = new Interface_Static("step", "read.step.nonmanifold", Interface_ParamEnum, "");
  Interface_Static::InitValues(anItem, "enum 0");
  Interface_Static::InitValues(anItem, "eval Off");
  Interface_Static::InitValues(anItem, "eval On");
  aStaticItem = Interface_Static::Static("read.step.nonmanifold");
  anItem->SetIntegerValue(aStaticItem.IsNull() ? 0 : aStaticItem->IntegerValue());
  myParamMap.Bind("read.step.nonmanifold", anItem);

  // Non-manifold topology writing: OFF by default (ssv; 26.11.2010)
  anItem = new Interface_Static("step", "write.step.nonmanifold", Interface_ParamEnum, "");
  Interface_Static::InitValues(anItem, "enum 0");
  Interface_Static::InitValues(anItem, "eval Off");
  Interface_Static::InitValues(anItem, "eval On");
  aStaticItem = Interface_Static::Static("write.step.nonmanifold");
  anItem->SetIntegerValue(aStaticItem.IsNull() ? 0 : aStaticItem->IntegerValue());
  myParamMap.Bind("write.step.nonmanifold", anItem);

  // I-Deas-like STEP processing: OFF by default (ssv; 22.11.2010)
  anItem = new Interface_Static("step", "read.step.ideas", Interface_ParamEnum, "");
  Interface_Static::InitValues(anItem, "enum 0");
  Interface_Static::InitValues(anItem, "eval Off");
  Interface_Static::InitValues(anItem, "eval On");
  aStaticItem = Interface_Static::Static("read.step.ideas");
  anItem->SetIntegerValue(aStaticItem.IsNull() ? 0 : aStaticItem->IntegerValue());
  myParamMap.Bind("read.step.ideas", anItem);

  //Parameter to write all free vertices in one SDR (name and style of vertex are lost) (default) 
  //or each vertex in its own SDR (name and style of vertex are exported). (ika; 21.07.2014)
  anItem = new Interface_Static("step", "write.step.vertex.mode", Interface_ParamEnum, "");
  Interface_Static::InitValues(anItem, "enum 0");
  Interface_Static::InitValues(anItem, "eval One Compound");
  Interface_Static::InitValues(anItem, "eval Single Vertex");
  aStaticItem = Interface_Static::Static("write.step.vertex.mode");
  anItem->SetIntegerValue(aStaticItem.IsNull() ? 0 : aStaticItem->IntegerValue());
  myParamMap.Bind("write.step.vertex.mode", anItem);

  // abv 15.11.00: ShapeProcessing
  anItem = new Interface_Static("XSTEP", "write.step.resource.name", Interface_ParamText, "STEP");
  myParamMap.Bind("write.step.resource.name", anItem);
  anItem = new Interface_Static("XSTEP", "read.step.resource.name", Interface_ParamText, "STEP");
  myParamMap.Bind("read.step.resource.name", anItem);
  anItem = new Interface_Static("XSTEP", "write.step.sequence", Interface_ParamText, "ToSTEP");
  myParamMap.Bind("write.step.sequence", anItem);
  anItem = new Interface_Static("XSTEP", "read.step.sequence", Interface_ParamText, "FromSTEP");
  myParamMap.Bind("read.step.sequence", anItem);

  // ika 28.07.16: Paremeter to read all top level solids and shells,
  // should be used only in case of invalid shape_representation without links to shapes.
  anItem = new Interface_Static("step", "read.step.all.shapes", Interface_ParamEnum, "");
  Interface_Static::InitValues(anItem, "enum 0");
  Interface_Static::InitValues(anItem, "eval Off");
  Interface_Static::InitValues(anItem, "eval On");
  aStaticItem = Interface_Static::Static("read.step.all.shapes");
  anItem->SetIntegerValue(aStaticItem.IsNull() ? 0 : aStaticItem->IntegerValue());
  myParamMap.Bind("read.step.all.shapes", anItem);

  //--------------------------------------------------------------------------------
  //From Base Class
  anItem = new Interface_Static("XSTEP", "read.precision.mode", Interface_ParamEnum, "");
  Interface_Static::InitValues(anItem, "ematch 0");
  Interface_Static::InitValues(anItem, "eval File");
  Interface_Static::InitValues(anItem, "eval User");
  anItem->SetIntegerValue(0);
  aStaticItem = Interface_Static::Static("read.precision.mode");
  anItem->SetIntegerValue(aStaticItem.IsNull() ? 0 : aStaticItem->IntegerValue());
  myParamMap.Bind("read.precision.mode", anItem);

  anItem = new Interface_Static("XSTEP", "read.precision.val", Interface_ParamReal, "1.e-03");
  myParamMap.Bind("read.precision.val", anItem);

  anItem = new Interface_Static("XSTEP", "read.maxprecision.mode", Interface_ParamEnum, "");
  Interface_Static::InitValues(anItem, "ematch 0");
  Interface_Static::InitValues(anItem, "eval Preferred");
  Interface_Static::InitValues(anItem, "eval Forced");
  aStaticItem = Interface_Static::Static("read.maxprecision.mode");
  anItem->SetIntegerValue(aStaticItem.IsNull() ? 0 : aStaticItem->IntegerValue());
  myParamMap.Bind("read.maxprecision.mode", anItem);

  anItem = new Interface_Static("XSTEP", "read.maxprecision.val", Interface_ParamReal, "1.");
  myParamMap.Bind("read.maxprecision.val", anItem);

  //   encode regularity
  //  negatif ou nul : ne rien faire. positif : on y va
  anItem = new Interface_Static("XSTEP", "read.encoderegularity.angle", Interface_ParamReal, "0.01");
  myParamMap.Bind("read.encoderegularity.angle", anItem);

  //   compute surface curves
  //  0 : par defaut. 2 : ne garder que le 2D. 3 : ne garder que le 3D
  //gka S4054
  anItem = new Interface_Static("XSTEP", "read.surfacecurve.mode", Interface_ParamEnum, "");
  Interface_Static::InitValues(anItem, "ematch -3");
  Interface_Static::InitValues(anItem, "eval 3DUse_Forced");
  Interface_Static::InitValues(anItem, "eval 2DUse_Forced");
  Interface_Static::InitValues(anItem, "eval ?");
  Interface_Static::InitValues(anItem, "eval Default");
  Interface_Static::InitValues(anItem, "eval ?");
  Interface_Static::InitValues(anItem, "eval 2DUse_Preferred");
  Interface_Static::InitValues(anItem, "eval 3DUse_Preferred");
  aStaticItem = Interface_Static::Static("read.surfacecurve.mode");
  anItem->SetIntegerValue(aStaticItem.IsNull() ? 0 : aStaticItem->IntegerValue());
  myParamMap.Bind("read.surfacecurve.mode", anItem);

  //   write precision
  anItem = new Interface_Static("XSTEP", "write.precision.mode", Interface_ParamEnum, "");
  Interface_Static::InitValues(anItem, "ematch -1");
  Interface_Static::InitValues(anItem, "eval Min");
  Interface_Static::InitValues(anItem, "eval Average");
  Interface_Static::InitValues(anItem, "eval Max");
  Interface_Static::InitValues(anItem, "eval User");
  aStaticItem = Interface_Static::Static("write.precision.mode");
  anItem->SetIntegerValue(aStaticItem.IsNull() ? 0 : aStaticItem->IntegerValue());
  myParamMap.Bind("write.precision.mode", anItem);

  anItem = new Interface_Static("XSTEP", "write.precision.val", Interface_ParamReal, "1.e-03");
  myParamMap.Bind("write.precision.val", anItem);

  // Write surface curves
  // 0: write (defaut), 1: do not write, 2: write except for analytical surfaces
  anItem = new Interface_Static("XSTEP", "write.surfacecurve.mode", Interface_ParamEnum, "");
  Interface_Static::InitValues(anItem, "ematch 0");
  Interface_Static::InitValues(anItem, "eval Off");
  Interface_Static::InitValues(anItem, "eval On");
  //Interface_Static::InitValues(anItem,"eval NoAnalytic");
  aStaticItem = Interface_Static::Static("write.surfacecurve.mode");
  anItem->SetIntegerValue(aStaticItem.IsNull() ? 1 : aStaticItem->IntegerValue());
  myParamMap.Bind("write.surfacecurve.mode", anItem);
  //--------------------------------------------------------------------------------

  //#74 rln 10.03.99 S4135: adding new parameter for handling use of BRepLib::SameParameter
  anItem = new Interface_Static("XSTEP", "read.stdsameparameter.mode", Interface_ParamEnum, "");
  Interface_Static::InitValues(anItem, "ematch 0");
  Interface_Static::InitValues(anItem, "eval Off");
  Interface_Static::InitValues(anItem, "eval On");
  aStaticItem = Interface_Static::Static("read.stdsameparameter.mode");
  anItem->SetIntegerValue(aStaticItem.IsNull() ? 0 : aStaticItem->IntegerValue());
  myParamMap.Bind("read.stdsameparameter.mode", anItem);

  // unit: supposed to be cascade unit (target unit for reading)
  anItem = new Interface_Static("XSTEP", "xstep.cascade.unit", Interface_ParamEnum, "");
  Interface_Static::InitValues(anItem, "enum 1");
  Interface_Static::InitValues(anItem, "eval INCH");  // 1
  Interface_Static::InitValues(anItem, "eval MM");    // 2
  Interface_Static::InitValues(anItem, "eval ??");    // 3
  Interface_Static::InitValues(anItem, "eval FT");    // 4
  Interface_Static::InitValues(anItem, "eval MI");    // 5
  Interface_Static::InitValues(anItem, "eval M");     // 6
  Interface_Static::InitValues(anItem, "eval KM");    // 7
  Interface_Static::InitValues(anItem, "eval MIL");   // 8
  Interface_Static::InitValues(anItem, "eval UM");    // 9
  Interface_Static::InitValues(anItem, "eval CM");    //10
  Interface_Static::InitValues(anItem, "eval UIN");   //11
  aStaticItem = Interface_Static::Static("xstep.cascade.unit");
  anItem->SetCStringValue(aStaticItem.IsNull() ? "MM" : aStaticItem->CStringValue());
  myParamMap.Bind("xstep.cascade.unit", anItem);

  //-----------------------------------------------------------
  // Few variables for advanced control of translation process
  //-----------------------------------------------------------

  // Indicates whether to write sub-shape names to 'Name' attributes of
  // STEP Representation Items
  anItem = new Interface_Static("stepcaf", "write.stepcaf.subshapes.name", Interface_ParamEnum, "");
  Interface_Static::InitValues(anItem, "enum 0");
  Interface_Static::InitValues(anItem, "eval Off"); // 0
  Interface_Static::InitValues(anItem, "eval On");  // 1
  aStaticItem = Interface_Static::Static("write.stepcaf.subshapes.name");
  anItem->SetIntegerValue(aStaticItem.IsNull() ? 0 : aStaticItem->IntegerValue());
  myParamMap.Bind("write.stepcaf.subshapes.name", anItem);

  // Indicates whether to read sub-shape names from 'Name' attributes of
  // STEP Representation Items
  anItem = new Interface_Static("stepcaf", "read.stepcaf.subshapes.name", Interface_ParamEnum, "");
  Interface_Static::InitValues(anItem, "enum 0");
  Interface_Static::InitValues(anItem, "eval Off"); // 0
  Interface_Static::InitValues(anItem, "eval On");  // 1
  aStaticItem = Interface_Static::Static("read.stepcaf.subshapes.name");
  anItem->SetIntegerValue(aStaticItem.IsNull() ? 0 : aStaticItem->IntegerValue());
  myParamMap.Bind("read.stepcaf.subshapes.name", anItem);

  // STEP file encoding for names translation
  // Note: the numbers should be consistent with Resource_FormatType enumeration
  anItem = new Interface_Static ("stepcaf", "read.stepcaf.codepage", Interface_ParamEnum, "");
  Interface_Static::InitValues(anItem, "enum 0");
  Interface_Static::InitValues(anItem, "eval SJIS");         // Resource_FormatType_SJIS
  Interface_Static::InitValues(anItem, "eval EUC");          // Resource_FormatType_EUC
  Interface_Static::InitValues(anItem, "eval ANSI");         // Resource_FormatType_ANSI
  Interface_Static::InitValues(anItem, "eval GB");           // Resource_FormatType_GB
  Interface_Static::InitValues(anItem, "eval UTF8");         // Resource_FormatType_UTF8
  Interface_Static::InitValues(anItem, "eval SystemLocale"); // Resource_FormatType_SystemLocale
  aStaticItem = Interface_Static::Static("read.stepcaf.codepage");
  anItem->SetIntegerValue(aStaticItem.IsNull() ? 0 : aStaticItem->IntegerValue());
  myParamMap.Bind("read.stepcaf.codepage", anItem);
}

Handle(Standard_Transient) StepData_StepModel::Entity
(const Standard_Integer num) const
{  return Value(num);  }      // nom plus joli

void StepData_StepModel::GetFromAnother
(const Handle(Interface_InterfaceModel)& other)
{
  theheader.Clear();
  DeclareAndCast(StepData_StepModel,another,other);
  if (another.IsNull()) return;
  Interface_EntityIterator iter = another->Header();
  //  recopier le header. Attention, header distinct du contenu ...
  Interface_CopyTool TC (this,StepData::HeaderProtocol());
  for (; iter.More(); iter.Next()) {
    Handle(Standard_Transient) newhead;
    if (!TC.Copy(iter.Value(),newhead,Standard_False,Standard_False)) continue;
    if (!newhead.IsNull()) theheader.Append(newhead);
  }
}

Handle(Interface_InterfaceModel) StepData_StepModel::NewEmptyModel () const
{  return new StepData_StepModel;  }


Interface_EntityIterator StepData_StepModel::Header () const
{
  Interface_EntityIterator iter;
  theheader.FillIterator(iter);
  return iter;
}

Standard_Boolean StepData_StepModel::HasHeaderEntity
(const Handle(Standard_Type)& atype) const
{  return (theheader.NbTypedEntities(atype) == 1);  }

Handle(Standard_Transient) StepData_StepModel::HeaderEntity
(const Handle(Standard_Type)& atype) const
{  return theheader.TypedEntity(atype);  }


//   Remplissage du Header

void StepData_StepModel::ClearHeader ()
{  theheader.Clear();  }


void StepData_StepModel::AddHeaderEntity
(const Handle(Standard_Transient)& ent)
{  theheader.Append(ent);  }


//void StepData_StepModel::VerifyCheck(Handle(Interface_Check)& ach) const
//{
//  Interface_GeneralLib lib(StepData::HeaderProtocol());
//  Handle(StepData_StepModel) me (this);
//  Handle(Interface_Protocol) aHP = StepData::HeaderProtocol();
//  Interface_ShareTool sh(me,aHP);
//  Handle(Interface_GeneralModule) module;  Standard_Integer CN;
//  for (Interface_EntityIterator iter = Header(); iter.More(); iter.Next()) {
//    Handle(Standard_Transient) head = iter.Value();
//    if (!lib.Select(head,module,CN)) continue;
//    module->CheckCase(CN,head,sh,ach);
//  }
//}


void StepData_StepModel::DumpHeader
(const Handle(Message_Messenger)& S, const Standard_Integer /*level*/) const
{
  //  NB : level n est pas utilise

  Handle(StepData_Protocol) stepro = StepData::HeaderProtocol();
  Standard_Boolean iapro = !stepro.IsNull();
  if (!iapro) S<<" -- WARNING : StepModel DumpHeader, Protocol not defined\n";

  Interface_EntityIterator iter = Header();
  Standard_Integer nb = iter.NbEntities();
  S << " --  Step Model Header : " <<iter.NbEntities() << " Entities :\n";
  for (iter.Start(); iter.More(); iter.Next()) {
    S << "  "  << iter.Value()->DynamicType()->Name() << "\n";
  }
  if (!iapro || nb == 0) return;
  S << " --  --        STEP MODEL    HEADER  CONTENT      --  --" << Message_EndLine;
  S << " --   Dumped with Protocol : " << stepro->DynamicType()->Name()
    << "   --"<<Message_EndLine;

  Standard_SStream aSStream;
  Handle(StepData_StepModel) me (this);
  StepData_StepWriter SW(me);
  SW.SendModel(stepro,Standard_True);    // envoi HEADER seul
  SW.Print(aSStream);
  S << aSStream.str().c_str();
}


void  StepData_StepModel::ClearLabels ()
{  theidnums.Nullify();  }

void  StepData_StepModel::SetIdentLabel
(const Handle(Standard_Transient)& ent, const Standard_Integer ident)
{
  Standard_Integer num = Number(ent);
  if (!num) 
    return;
  Standard_Integer nbEnt = NbEntities();
  if(theidnums.IsNull())
  {
    theidnums = new TColStd_HArray1OfInteger(1,nbEnt);
    theidnums->Init(0);
  }
  else if(nbEnt > theidnums->Length())
  {
    Standard_Integer prevLength = theidnums->Length();
    Handle(TColStd_HArray1OfInteger) idnums1 = new TColStd_HArray1OfInteger(1,nbEnt);
    idnums1->Init(0);
    Standard_Integer k =1;
    for( ; k <= prevLength; k++)
      idnums1->SetValue(k , theidnums->Value(k));
    theidnums = idnums1;
  }
  theidnums->SetValue(num,ident);

}

Standard_Integer  StepData_StepModel::IdentLabel
(const Handle(Standard_Transient)& ent) const
{
  if(theidnums.IsNull())
    return 0;
  Standard_Integer num = Number(ent);
  return (!num ? 0 : theidnums->Value(num));
 }

void  StepData_StepModel::PrintLabel
(const Handle(Standard_Transient)& ent, const Handle(Message_Messenger)& S) const
{
  Standard_Integer num = (theidnums.IsNull() ? 0 : Number(ent));
  Standard_Integer  nid = (!num ? 0 : theidnums->Value(num));
  if      (nid > 0) S<<"#"<<nid;
  else if (num > 0) S<<"(#"<<num<<")";
  else              S<<"(#0..)";
}

Handle(TCollection_HAsciiString) StepData_StepModel::StringLabel
(const Handle(Standard_Transient)& ent) const
{
  Handle(TCollection_HAsciiString) label;
  char text[20];
  Standard_Integer num = (theidnums.IsNull() ? 0 : Number(ent));
  Standard_Integer  nid = (!num ? 0 : theidnums->Value(num));

  if      (nid > 0) sprintf (text, "#%d",nid);
  else if (num > 0) sprintf (text, "(#%d)",num);
  else              sprintf (text, "(#0..)");

  label = new TCollection_HAsciiString(text);
  return label;
}
