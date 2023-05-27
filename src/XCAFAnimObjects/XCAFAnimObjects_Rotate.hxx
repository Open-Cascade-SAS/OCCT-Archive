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

#ifndef _XCAFAnimObjects_Rotate_HeaderFile
#define _XCAFAnimObjects_Rotate_HeaderFile

#include <XCAFAnimObjects_Operation.hxx>
#include <gp_Quaternion.hxx>

//! 
class XCAFAnimObjects_Rotate : public XCAFAnimObjects_Operation
{
public:

  //! 
  enum XCAFAnimObjects_Rotate_Type
  {
    XCAFAnimObjects_Rotate_Type_XYZ = 0,
    XCAFAnimObjects_Rotate_Type_XZY,
    XCAFAnimObjects_Rotate_Type_YZX,
    XCAFAnimObjects_Rotate_Type_YXZ,
    XCAFAnimObjects_Rotate_Type_ZXY,
    XCAFAnimObjects_Rotate_Type_ZYX,
    XCAFAnimObjects_Rotate_Type_XY,
    XCAFAnimObjects_Rotate_Type_XZ,
    XCAFAnimObjects_Rotate_Type_YX,
    XCAFAnimObjects_Rotate_Type_YZ,
    XCAFAnimObjects_Rotate_Type_ZX,
    XCAFAnimObjects_Rotate_Type_ZY,
    XCAFAnimObjects_Rotate_Type_X,
    XCAFAnimObjects_Rotate_Type_Y,
    XCAFAnimObjects_Rotate_Type_Z
  };

public:

  //! 
  Standard_EXPORT XCAFAnimObjects_Rotate(const NCollection_Array1<double>& theRotate,
                                         const NCollection_Array1<double>& theTimeStamps,
                                         const XCAFAnimObjects_Rotate_Type theRotateType = XCAFAnimObjects_Rotate_Type_XYZ);

  //! 
  Standard_EXPORT XCAFAnimObjects_Rotate(const NCollection_Array2<double>& theGeneralPresentation,
                                         const NCollection_Array1<double>& theTimeStamps,
                                         const XCAFAnimObjects_Rotate_Type theRotateType = XCAFAnimObjects_Rotate_Type_XYZ);

  //!
  Standard_EXPORT XCAFAnimObjects_Rotate(const Handle(XCAFAnimObjects_Rotate)& theOperation);

  //! 
  XCAFAnimObjects_OperationType GetType() const Standard_OVERRIDE { return XCAFAnimObjects_OperationType_Rotate; }

  //! 
  XCAFAnimObjects_Rotate_Type GetRotateType() const { return myRotateType; }

  //! 
  void SetRotateType(const XCAFAnimObjects_Rotate_Type theRotateType) { myRotateType = theRotateType; }

  //! 
  TCollection_AsciiString GetTypeName() const Standard_OVERRIDE { return "Rotate"; }

  //! 
  NCollection_Array2<double> GeneralPresentation() const { return myRotatePresentation; }

  //! 
  const NCollection_Array2<double>& RotatePresentation() const { return myRotatePresentation; }

private:

  XCAFAnimObjects_Rotate_Type myRotateType; //!< 
  NCollection_Array2<double> myRotatePresentation; //!< 
};

#endif // _XCAFAnimObjects_Rotate_HeaderFile
