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

#ifndef _XCAFAnimObjects_Skew_HeaderFile
#define _XCAFAnimObjects_Skew_HeaderFile

#include <XCAFAnimObjects_Operation.hxx>
#include <gp_Quaternion.hxx>
#include <gp_Ax3.hxx>

//! 
class XCAFAnimObjects_Skew : public XCAFAnimObjects_Operation
{
public:

  //! 
  struct Skew
  {
    double Angle = 0.; //!< 
    gp_Dir Axis1; //!< 
    gp_Dir Axis2; //!< 
  };

  //! 
  Standard_EXPORT XCAFAnimObjects_Skew(const Skew& theSkew);

  //! 
  Standard_EXPORT XCAFAnimObjects_Skew(const NCollection_Array1<Skew>& theSkew,
                                       const NCollection_Array1<double>& theTimeStamps);

  //! 
  Standard_EXPORT XCAFAnimObjects_Skew(const NCollection_Array2<double>& theGeneralPresentation,
                                       const NCollection_Array1<double>& theTimeStamps);

  //! 
  XCAFAnimObjects_OperationType GetType() const Standard_OVERRIDE { return XCAFAnimObjects_OperationType_Skew; }

  //! 
  TCollection_AsciiString GetTypeName() const Standard_OVERRIDE { return "Skew"; }

  //! 
  Standard_EXPORT NCollection_Array2<double> GeneralPresentation() const Standard_OVERRIDE;

  //! 
  const NCollection_Array1<Skew>& SkewPresentation() const { return mySkewPresentation; }

private:

  NCollection_Array1<Skew> mySkewPresentation; //!< 
};

#endif // _XCAFAnimObjects_Skew_HeaderFile
