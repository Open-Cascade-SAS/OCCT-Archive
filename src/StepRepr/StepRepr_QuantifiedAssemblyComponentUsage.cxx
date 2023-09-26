// Created on: 2000-07-03
// Created by: Andrey BETENEV
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

// Generator:	ExpToCas (EXPRESS -> CASCADE/XSTEP Translator) V1.1

#include <StepBasic_ProductDefinition.hxx>
#include <StepRepr_QuantifiedAssemblyComponentUsage.hxx>
#include <TCollection_HAsciiString.hxx>

IMPLEMENT_STANDARD_RTTIEXT(StepRepr_QuantifiedAssemblyComponentUsage,StepRepr_AssemblyComponentUsage)

//=======================================================================
//function : StepRepr_QuantifiedAssemblyComponentUsage
//purpose  : 
//=======================================================================
StepRepr_QuantifiedAssemblyComponentUsage::StepRepr_QuantifiedAssemblyComponentUsage ()
{
}

//=======================================================================
//function : Init
//purpose  : 
//=======================================================================

void StepRepr_QuantifiedAssemblyComponentUsage::Init (const Handle(TCollection_HAsciiString)& theProductDefinitionRelationship_Id,
                                                      const Handle(TCollection_HAsciiString)& theProductDefinitionRelationship_Name,
                                                      const Standard_Boolean hasProductDefinitionRelationship_Description,
                                                      const Handle(TCollection_HAsciiString)& theProductDefinitionRelationship_Description,
                                                      const Handle(StepBasic_ProductDefinition)& theProductDefinitionRelationship_RelatingProductDefinition,
                                                      const Handle(StepBasic_ProductDefinition)& theProductDefinitionRelationship_RelatedProductDefinition,
                                                      const Standard_Boolean hasAssemblyComponentUsage_ReferenceDesignator,
                                                      const Handle(TCollection_HAsciiString)& theAssemblyComponentUsage_ReferenceDesignator,
                                                      const Handle(Standard_Transient)& theQuantity)
{
  StepRepr_AssemblyComponentUsage::Init(theProductDefinitionRelationship_Id,
                                        theProductDefinitionRelationship_Name,
                                        hasProductDefinitionRelationship_Description,
                                        theProductDefinitionRelationship_Description,
                                        theProductDefinitionRelationship_RelatingProductDefinition,
                                        theProductDefinitionRelationship_RelatedProductDefinition,
                                        hasAssemblyComponentUsage_ReferenceDesignator,
                                        theAssemblyComponentUsage_ReferenceDesignator);

  myQuantity = theQuantity;
}

//=======================================================================
//function : Init
//purpose  : 
//=======================================================================

void StepRepr_QuantifiedAssemblyComponentUsage::Init (const Handle(TCollection_HAsciiString)& theProductDefinitionRelationship_Id,
                                                      const Handle(TCollection_HAsciiString)& theProductDefinitionRelationship_Name,
                                                      const Standard_Boolean hasProductDefinitionRelationship_Description,
                                                      const Handle(TCollection_HAsciiString)& theProductDefinitionRelationship_Description,
                                                      const StepBasic_ProductDefinitionOrReference& theProductDefinitionRelationship_RelatingProductDefinition,
                                                      const StepBasic_ProductDefinitionOrReference& theProductDefinitionRelationship_RelatedProductDefinition,
                                                      const Standard_Boolean hasAssemblyComponentUsage_ReferenceDesignator,
                                                      const Handle(TCollection_HAsciiString)& theAssemblyComponentUsage_ReferenceDesignator,
                                                      const Handle(Standard_Transient)& theQuantity)
{
  StepRepr_AssemblyComponentUsage::Init(theProductDefinitionRelationship_Id,
                                        theProductDefinitionRelationship_Name,
                                        hasProductDefinitionRelationship_Description,
                                        theProductDefinitionRelationship_Description,
                                        theProductDefinitionRelationship_RelatingProductDefinition,
                                        theProductDefinitionRelationship_RelatedProductDefinition,
                                        hasAssemblyComponentUsage_ReferenceDesignator,
                                        theAssemblyComponentUsage_ReferenceDesignator);

  myQuantity = theQuantity;
}

//=======================================================================
//function : Quantity
//purpose  : 
//=======================================================================

Handle(Standard_Transient) StepRepr_QuantifiedAssemblyComponentUsage::Quantity () const
{
  return myQuantity;
}

//=======================================================================
//function : SetQuantity
//purpose  : 
//=======================================================================

void StepRepr_QuantifiedAssemblyComponentUsage::SetQuantity (const Handle(Standard_Transient)& theQuantity)
{
  myQuantity = theQuantity;
}
