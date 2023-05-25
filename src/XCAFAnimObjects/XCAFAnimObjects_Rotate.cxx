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

#include "XCAFAnimObjects_Rotate.hxx"

#include <Message.hxx>

//=======================================================================
//function : XCAFAnimObjects_Rotate
//purpose  :
//=======================================================================
XCAFAnimObjects_Rotate::XCAFAnimObjects_Rotate(const gp_Quaternion& theRotate) :
  XCAFAnimObjects_Operation(false),
  myRotatePresentation(1, 1)
{
  myRotatePresentation.SetValue(1, theRotate);
}

//=======================================================================
//function : XCAFAnimObjects_Rotate
//purpose  :
//=======================================================================
XCAFAnimObjects_Rotate::XCAFAnimObjects_Rotate(const gp_XYZ& theRotate) :
  XCAFAnimObjects_Operation(false),
  myRotatePresentation(1, 1)
{
  // Convert angles from degrees to radians and create quaternions for each rotation.
  gp_Quaternion aQuaternionX(gp_Dir(1, 0, 0), theRotate.X() * M_PI / 180.0);
  gp_Quaternion aQuaternionY(gp_Dir(0, 1, 0), theRotate.Y() * M_PI / 180.0);
  gp_Quaternion aQuaternionZ(gp_Dir(0, 0, 1), theRotate.Z() * M_PI / 180.0);

  // Combine the rotations. The order depends on the rotation order in the original Euler angles.
  gp_Quaternion aCombinedQuaternion = aQuaternionX * aQuaternionY * aQuaternionZ;

  myRotatePresentation.SetValue(1, aCombinedQuaternion);
}

//=======================================================================
//function : XCAFAnimObjects_Rotate
//purpose  :
//=======================================================================
XCAFAnimObjects_Rotate::XCAFAnimObjects_Rotate(const NCollection_Array1<gp_Quaternion>& theRotate,
                                               const NCollection_Array1<double>& theTimeStamps) :
  XCAFAnimObjects_Operation(theTimeStamps),
  myRotatePresentation(theRotate)
{}

//=======================================================================
//function : XCAFAnimObjects_Rotate
//purpose  :
//=======================================================================
XCAFAnimObjects_Rotate::XCAFAnimObjects_Rotate(const NCollection_Array2<double>& theGeneralPresentation,
                                               const NCollection_Array1<double>& theTimeStamps) :
  XCAFAnimObjects_Operation(false),
  myRotatePresentation(1, theGeneralPresentation.RowLength())
{
  if (theGeneralPresentation.ColLength() != 4)
  {
    Message::SendWarning() << "Warning: XCAFAnimObjects_Rotate: Incorrect Quaternion general presentation";
    return;
  }
  for (int aRowInd = 1; aRowInd <= theGeneralPresentation.RowLength(); aRowInd++)
  {
    gp_Quaternion aQuat(theGeneralPresentation.Value(aRowInd, 1),
                        theGeneralPresentation.Value(aRowInd, 2),
                        theGeneralPresentation.Value(aRowInd, 3),
                        theGeneralPresentation.Value(aRowInd, 4));
    myRotatePresentation.SetValue(aRowInd, aQuat);
  }
}

//=======================================================================
//function : GeneralPresentation
//purpose  :
//=======================================================================
NCollection_Array2<double> XCAFAnimObjects_Rotate::GeneralPresentation() const
{
  NCollection_Array2<double> aRes(1, myRotatePresentation.Length(), 1, 4);
  for (int aRowInd = 1; aRowInd <= myRotatePresentation.Length(); aRowInd++)
  {
    const gp_Quaternion& aQuat = myRotatePresentation.Value(aRowInd);

    aRes.SetValue(aRowInd, 1, aQuat.X());
    aRes.SetValue(aRowInd, 2, aQuat.Y());
    aRes.SetValue(aRowInd, 3, aQuat.Z());
    aRes.SetValue(aRowInd, 4, aQuat.W());
  }
}
