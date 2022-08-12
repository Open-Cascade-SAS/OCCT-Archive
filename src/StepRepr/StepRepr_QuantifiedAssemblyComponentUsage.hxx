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

#ifndef _StepRepr_QuantifiedAssemblyComponentUsage_HeaderFile
#define _StepRepr_QuantifiedAssemblyComponentUsage_HeaderFile

#include <Standard.hxx>
#include <Standard_Type.hxx>

#include <StepRepr_AssemblyComponentUsage.hxx>

class TCollection_HAsciiString;
class StepBasic_ProductDefinition;


class StepRepr_QuantifiedAssemblyComponentUsage;
DEFINE_STANDARD_HANDLE(StepRepr_QuantifiedAssemblyComponentUsage, StepRepr_AssemblyComponentUsage)

//! Representation of STEP entity QuantifiedAssemblyComponentUsage
class StepRepr_QuantifiedAssemblyComponentUsage : public StepRepr_AssemblyComponentUsage
{

public:

  
  //! Empty constructor
  Standard_EXPORT StepRepr_QuantifiedAssemblyComponentUsage();
  
  //! Initialize all fields (own and inherited)
  Standard_EXPORT void Init (const Handle(TCollection_HAsciiString)& theProductDefinitionRelationship_Id,
                             const Handle(TCollection_HAsciiString)& theProductDefinitionRelationship_Name,
                             const Standard_Boolean hasProductDefinitionRelationship_Description,
                             const Handle(TCollection_HAsciiString)& theProductDefinitionRelationship_Description,
                             const Handle(StepBasic_ProductDefinition)& theProductDefinitionRelationship_RelatingProductDefinition,
                             const Handle(StepBasic_ProductDefinition)& theProductDefinitionRelationship_RelatedProductDefinition,
                             const Standard_Boolean hasAssemblyComponentUsage_ReferenceDesignator,
                             const Handle(TCollection_HAsciiString)& theAssemblyComponentUsage_ReferenceDesignator,
                             const Handle(Standard_Transient)& theQuantity);

  //! Initialize all fields (own and inherited)
  Standard_EXPORT void Init (const Handle(TCollection_HAsciiString)& theProductDefinitionRelationship_Id,
                             const Handle(TCollection_HAsciiString)& theProductDefinitionRelationship_Name,
                             const Standard_Boolean hasProductDefinitionRelationship_Description,
                             const Handle(TCollection_HAsciiString)& theProductDefinitionRelationship_Description,
                             const StepBasic_ProductDefinitionOrReference& theProductDefinitionRelationship_RelatingProductDefinition,
                             const StepBasic_ProductDefinitionOrReference& theProductDefinitionRelationship_RelatedProductDefinition,
                             const Standard_Boolean hasAssemblyComponentUsage_ReferenceDesignator,
                             const Handle(TCollection_HAsciiString)& theAssemblyComponentUsage_ReferenceDesignator,
                             const Handle(Standard_Transient)& theQuantity);
  
  //! Returns field Quantity
  Standard_EXPORT Handle(Standard_Transient) Quantity() const;
  
  //! Set field Quantity
  Standard_EXPORT void SetQuantity (const Handle(Standard_Transient)& theQuantity);




  DEFINE_STANDARD_RTTIEXT(StepRepr_QuantifiedAssemblyComponentUsage,StepRepr_AssemblyComponentUsage)

protected:




private:


  Handle(Standard_Transient) myQuantity;


};







#endif // _StepRepr_QuantifiedAssemblyComponentUsage_HeaderFile
