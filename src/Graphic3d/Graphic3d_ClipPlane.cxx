// Created on: 2013-07-12
// Created by: Anton POLETAEV
// Copyright (c) 2013-2014 OPEN CASCADE SAS
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

#include <Graphic3d_ClipPlane.hxx>

#include <Graphic3d_AspectFillArea3d.hxx>
#include <gp_Pln.hxx>
#include <Standard_Atomic.hxx>

IMPLEMENT_STANDARD_RTTIEXT(Graphic3d_ClipPlane,Standard_Transient)

namespace
{
  static volatile Standard_Integer THE_CLIP_PLANE_COUNTER = 0;
}

// =======================================================================
// function : Graphic3d_ClipPlane
// purpose  :
// =======================================================================
Graphic3d_ClipPlane::Graphic3d_ClipPlane()
{
  init();
}

// =======================================================================
// function : Graphic3d_ClipPlane
// purpose  :
// =======================================================================
Graphic3d_ClipPlane::Graphic3d_ClipPlane (const Graphic3d_Vec4d& theEquation)
{
  init (gp_Pln (theEquation.x(), theEquation.y(), theEquation.z(), theEquation.a()));
  updateInversedPlane();
}

// =======================================================================
// function : Graphic3d_ClipPlane
// purpose  :
// =======================================================================
Graphic3d_ClipPlane::Graphic3d_ClipPlane (const Graphic3d_ClipPlane& theOther)
: Standard_Transient (theOther)
{
  *mySectionStyle = *theOther.CappingSectionStyle();
  init (theOther.myPlane,
        theOther.myEquationRev,
        theOther.myIsOn,
        theOther.myIsCapping,
        theOther.ToOverrideCappingAspect(),
        theOther.CappingSectionStyle());
  updateInversedPlane();
}

// =======================================================================
// function : Graphic3d_ClipPlane
// purpose  :
// =======================================================================
Graphic3d_ClipPlane::Graphic3d_ClipPlane (const gp_Pln& thePlane)
{
  init (thePlane);
  updateInversedPlane();
}

// =======================================================================
// function : SetEquation
// purpose  :
// =======================================================================
void Graphic3d_ClipPlane::SetEquation (const Graphic3d_Vec4d& theEquation)
{
  myPlane = gp_Pln (theEquation.x(), theEquation.y(), theEquation.z(), theEquation.w());
  myEquation = theEquation;
  updateInversedPlane();
  myOrientationDirty = Standard_True;
}

// =======================================================================
// function : SetPlane
// purpose  :
// =======================================================================
void Graphic3d_ClipPlane::SetEquation (const gp_Pln& thePlane)
{
  myPlane = thePlane;
  thePlane.Coefficients (myEquation[0], myEquation[1], myEquation[2], myEquation[3]);
  updateInversedPlane();
  myOrientationDirty = Standard_True;
}

// =======================================================================
// function : SetOn
// purpose  :
// =======================================================================
void Graphic3d_ClipPlane::SetOn (const Standard_Boolean theIsOn)
{
  if (myPrevInChain != NULL)
  {
    throw Standard_ProgramError ("Graphic3d_ClipPlane::SetOn() - undefined operation for a plane in Union");
  }
  myIsOn = theIsOn;
}

// =======================================================================
// function : SetCapping
// purpose  :
// =======================================================================
void Graphic3d_ClipPlane::SetCapping (const Standard_Boolean theIsOn)
{
  myIsCapping = theIsOn;
}

// =======================================================================
// function : Clone
// purpose  :
// =======================================================================
Handle(Graphic3d_ClipPlane) Graphic3d_ClipPlane::Clone() const
{
  return new Graphic3d_ClipPlane(*this);
}

// =======================================================================
// function : SetCappingColor
// purpose  :
// =======================================================================
void Graphic3d_ClipPlane::SetCappingColor (const Quantity_Color& theColor)
{
  mySectionStyle->SetInteriorColor (theColor);
  mySectionStyle->ChangeFrontMaterial().SetColor (theColor);
  ++myAspectMod;
}

// =======================================================================
// purpose  :
// =======================================================================
void Graphic3d_ClipPlane::SetCappingSectionStyle (const Handle(Graphic3d_AspectFillCapping)& theStyle)
{
  mySectionStyle = theStyle;
  if (mySectionStyle->FrontMaterial().MaterialType() != Graphic3d_MATERIAL_ASPECT)
  {
  }
}

// =======================================================================
// function : OrientationMatrix
// purpose  :
// =======================================================================
const Graphic3d_Mat4& Graphic3d_ClipPlane::OrientationMatrix() const
{
  if (myOrientationDirty)
  {
    const Standard_ShortReal aDirection[] = {
      static_cast<Standard_ShortReal> (myEquation[0]),
      static_cast<Standard_ShortReal> (myEquation[1]),
      static_cast<Standard_ShortReal> (myEquation[2])
     };

    const Standard_ShortReal aTranslate[] = {
      static_cast<Standard_ShortReal> (myEquation[0] * -myEquation[3]),
      static_cast<Standard_ShortReal> (myEquation[1] * -myEquation[3]),
      static_cast<Standard_ShortReal> (myEquation[2] * -myEquation[3])
    };

    Standard_ShortReal aSide1[] = { 0.0f, 0.0f, 0.0f };
    Standard_ShortReal aSide2[] = { 0.0f, 0.0f, 0.0f };

    const Standard_ShortReal aMagintude = static_cast<Standard_ShortReal> (Sqrt (myEquation[0] * myEquation[0] + myEquation[2] * myEquation[2]));

    if (aMagintude < ShortRealSmall())
    {
      aSide1[0] = 1.0f;
    }
    else
    {
      aSide1[0] =  aDirection[2] / aMagintude;
      aSide1[2] = -aDirection[0] / aMagintude;
    }

    aSide2[0] = (-aSide1[1] * aDirection[2]) - (-aSide1[2] * aDirection[1]);
    aSide2[1] = (-aSide1[2] * aDirection[0]) - (-aSide1[0] * aDirection[2]);
    aSide2[2] = (-aSide1[0] * aDirection[1]) - (-aSide1[1] * aDirection[0]);

    myOrientationMat.SetValue (0, 0, aSide1[0]);
    myOrientationMat.SetValue (1, 0, aSide1[1]);
    myOrientationMat.SetValue (2, 0, aSide1[2]);
    myOrientationMat.SetValue (3, 0, 0.0F);

    myOrientationMat.SetValue (0, 1, aDirection[0]);
    myOrientationMat.SetValue (1, 1, aDirection[1]);
    myOrientationMat.SetValue (2, 1, aDirection[2]);
    myOrientationMat.SetValue (3, 1, 0.0F);

    myOrientationMat.SetValue (0, 2, aSide2[0]);
    myOrientationMat.SetValue (1, 2, aSide2[1]);
    myOrientationMat.SetValue (2, 2, aSide2[2]);
    myOrientationMat.SetValue (3, 2, 0.0F);

    myOrientationMat.SetValue (0, 3, aTranslate[0]);
    myOrientationMat.SetValue (1, 3, aTranslate[1]);
    myOrientationMat.SetValue (2, 3, aTranslate[2]);
    myOrientationMat.SetValue (3, 3, 1.0F);

    myOrientationDirty = Standard_False;
  }

  return myOrientationMat;
}

// =======================================================================
// function : init
// purpose  :
// =======================================================================
void Graphic3d_ClipPlane::init (const gp_Pln& thePlane,
                                const Graphic3d_Vec4d& theEquationRev,
                                const Standard_Boolean theIsOn,
                                const Standard_Boolean theIsCapping,
                                const Standard_Boolean theOverrideStyle,
                                const Handle(Graphic3d_AspectFillCapping)& theStyle)
{
  if (myEntityUID.IsEmpty())
  {
    myEntityUID = TCollection_AsciiString ("Graphic3d_ClipPlane_") //DynamicType()->Name()
                + TCollection_AsciiString (Standard_Atomic_Increment (&THE_CLIP_PLANE_COUNTER));
  }

  myPrevInChain         = NULL;
  myEquationRev         = theEquationRev;
  myChainLenFwd         = 1;
  myPlane               = thePlane;
  myPlane.Coefficients (myEquation[0], myEquation[1], myEquation[2], myEquation[3]);
  myIsOn                = theIsOn;
  myIsCapping           = theIsCapping;
  myOverrideObjectStyle = theOverrideStyle;
  mySectionStyle        = theStyle.IsNull() ? new Graphic3d_AspectFillCapping() : theStyle;
  myOrientationDirty    = Standard_True;
}

// =======================================================================
// function : updateChainLen
// purpose  :
// =======================================================================
void Graphic3d_ClipPlane::updateChainLen()
{
  myChainLenFwd = !myNextInChain.IsNull() ? (myNextInChain->myChainLenFwd + 1) : 1;
  if (myPrevInChain != NULL)
  {
    myPrevInChain->updateChainLen();
  }
}

// =======================================================================
// function : SetChainNextPlane
// purpose  :
// =======================================================================
void Graphic3d_ClipPlane::SetChainNextPlane (const Handle(Graphic3d_ClipPlane)& thePlane)
{
  myOrientationDirty = Standard_True;
  if (!myNextInChain.IsNull())
  {
    myNextInChain->myPrevInChain = NULL;
  }
  myNextInChain = thePlane;
  if (!myNextInChain.IsNull())
  {
    myNextInChain->myPrevInChain = this;
  }
  updateChainLen();
}

// =======================================================================
// function : DumpJson
// purpose  :
// =======================================================================
void Graphic3d_ClipPlane::DumpJson (Standard_OStream& theOStream, Standard_Integer theDepth) const
{
  OCCT_DUMP_TRANSIENT_CLASS_BEGIN (theOStream)

  OCCT_DUMP_FIELD_VALUE_POINTER (theOStream, this)

  OCCT_DUMP_FIELD_VALUES_DUMPED (theOStream, theDepth, mySectionStyle.get())
  OCCT_DUMP_FIELD_VALUE_POINTER (theOStream, myNextInChain.get())
  OCCT_DUMP_FIELD_VALUE_POINTER (theOStream, myPrevInChain)

  OCCT_DUMP_FIELD_VALUE_STRING (theOStream, myEntityUID)

  OCCT_DUMP_FIELD_VALUES_DUMPED (theOStream, theDepth, &myPlane)

  OCCT_DUMP_FIELD_VALUES_NUMERICAL (theOStream, "Equation", 4, myEquation.x(), myEquation.y(), myEquation.z(), myEquation.w())
  OCCT_DUMP_FIELD_VALUES_NUMERICAL (theOStream, "EquationRev", 4, myEquationRev.x(), myEquationRev.y(), myEquationRev.z(), myEquationRev.w())

  OCCT_DUMP_FIELD_VALUE_NUMERICAL (theOStream, myChainLenFwd)
  OCCT_DUMP_FIELD_VALUE_NUMERICAL (theOStream, myFlags)
  OCCT_DUMP_FIELD_VALUE_NUMERICAL (theOStream, myEquationMod)
  OCCT_DUMP_FIELD_VALUE_NUMERICAL (theOStream, myAspectMod)
  OCCT_DUMP_FIELD_VALUE_NUMERICAL (theOStream, myIsOn)
  OCCT_DUMP_FIELD_VALUE_NUMERICAL (theOStream, myIsCapping)
}
