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

#ifndef _XCAFAnimObjects_Transform_HeaderFile
#define _XCAFAnimObjects_Transform_HeaderFile

#include <XCAFAnimObjects_Operation.hxx>
#include <NCollection_Mat4.hxx>

//! 
class XCAFAnimObjects_Transform : public XCAFAnimObjects_Operation
{
public:

  //! 
  Standard_EXPORT XCAFAnimObjects_Transform(const NCollection_Mat4<double>& theTransform);

  //! 
  Standard_EXPORT XCAFAnimObjects_Transform(const NCollection_Array1<NCollection_Mat4<double>>& theTransform,
                                            const NCollection_Array1<double>& theTimeStamps);

  //! 
  Standard_EXPORT XCAFAnimObjects_Transform(const NCollection_Array2<double>& theGeneralPresentation,
                                            const NCollection_Array1<double>& theTimeStamps);

  //!
  Standard_EXPORT XCAFAnimObjects_Transform(const Handle(XCAFAnimObjects_Transform)& theOperation);

  //! 
  XCAFAnimObjects_OperationType GetType() const Standard_OVERRIDE { return XCAFAnimObjects_OperationType_Transform; }

  //! 
  TCollection_AsciiString GetTypeName() const Standard_OVERRIDE { return "Transform"; }

  //! 
  Standard_EXPORT NCollection_Array2<double> GeneralPresentation() const Standard_OVERRIDE;

  //! 
  const NCollection_Array1<NCollection_Mat4<double>>& TransformPresentation() const { return myTransformPresentation; }

private:

  NCollection_Array1<NCollection_Mat4<double>> myTransformPresentation; //!< 
};

#endif // _XCAFAnimObjects_Transform_HeaderFile
