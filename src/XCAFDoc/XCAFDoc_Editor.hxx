// Created on: 2015-05-14
// Created by: Ilya Novikov
// Copyright (c) 2000-2015 OPEN CASCADE SAS
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

#ifndef _XCAFDoc_Editor_HeaderFile
#define _XCAFDoc_Editor_HeaderFile

#include <Standard_DefineAlloc.hxx>
#include <TDataStd_Name.hxx>
#include <TDF_AttributeMap.hxx>
#include <TDF_Label.hxx>
#include <TDF_LabelDataMap.hxx>
#include <TDF_LabelSequence.hxx>

class XCAFDoc_VisMaterial;
class XCAFDoc_ShapeTool;

//! Tool for edit structure of document.
class XCAFDoc_Editor
{
public:

  DEFINE_STANDARD_ALLOC

  //! Convert Shape (compound/compsolid/shell/wire) to assembly.
  //! Only compounds expanded recursively
  Standard_EXPORT static Standard_Boolean Expand(const TDF_Label& theDoc,
                                                 const TDF_Label& theShape,
                                                 const Standard_Boolean theRecursively = Standard_True);

  //! Convert all compounds in Doc to assembly
  Standard_EXPORT static Standard_Boolean Expand(const TDF_Label& theDoc,
                                                 const Standard_Boolean theRecursively = Standard_True);

  //! Extracts passed labels and put them as a component of theDstLabel
  //! or creates new shape labels if theDstLabel is a main label of document
  Standard_EXPORT static Standard_Boolean Extract(const TDF_LabelSequence& theSrcLabels,
                                                  const TDF_Label& theDstLabel,
                                                  const Standard_Boolean theIsNoVisMat = Standard_False);

  //! Extracts passed label and put it as a component of theDstLabel
  //! or creates new shape label if theDstLabel is a main label of document
  Standard_EXPORT static Standard_Boolean Extract(const TDF_Label& theSrcLabel,
                                                  const TDF_Label& theDstLabel,
                                                  const Standard_Boolean theIsNoVisMat = Standard_False);

  //! Copies shapes label with saving of shape structure (recursively)
  //! theMap is a relating map of the original shapes label and labels created from them
  Standard_EXPORT static TDF_Label CloneShapeLabel(const TDF_Label& theSrcLabel,
                                                   const Handle(XCAFDoc_ShapeTool)& theSrcShapeTool,
                                                   const Handle(XCAFDoc_ShapeTool)& theDstShapeTool,
                                                   TDF_LabelDataMap& theMap);

  //! Copies meta data conteins from the source label to the destination label
  //! theVisMatMap can be NULL, if theSrcLabel and theDstLabel located in the same document
  //! Protected against creating a new label for non-existent tools
  Standard_EXPORT static void CloneMetaData(const TDF_Label& theSrcLabel,
                                            const TDF_Label& theDstLabel,
                                            NCollection_DataMap<Handle(XCAFDoc_VisMaterial), Handle(XCAFDoc_VisMaterial)>* theVisMatMap,
                                            const Standard_Boolean theToCopyColor = Standard_True,
                                            const Standard_Boolean theToCopyLayer = Standard_True,
                                            const Standard_Boolean theToCopyMaterial = Standard_True,
                                            const Standard_Boolean theToCopyVisMaterial = Standard_True,
                                            const Standard_Boolean theToCopyAttributes = Standard_True);

};

#endif // _XCAFDoc_Editor_HeaderFile
