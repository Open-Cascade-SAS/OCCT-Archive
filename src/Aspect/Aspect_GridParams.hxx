// Created on: 1995-03-02
// Created by: Jean-Louis Frenkel
// Copyright (c) 1995-1999 Matra Datavision
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

#ifndef _Aspect_GridParams_HeaderFile
#define _Aspect_GridParams_HeaderFile

#include <Quantity_Color.hxx>
#include <gp_Pnt.hxx>

class Aspect_GridParams
{
public:

  //! Constructor.
  Aspect_GridParams()
  : myColor (0.8, 0.8, 0.8, Quantity_TOC_RGB),
    myPosition (0.0, 0.0, 0.0),
    myScale (0.01),
    myLineThickness (0.01),
    myIsBackground (false),
    myIsDrawAxis (true),
    myIsInfinity (false)
  {}

  //! Getters
  Standard_Boolean IsBackground() const { return myIsBackground; }
  Standard_Boolean IsDrawAxis() const { return myIsDrawAxis; }
  const Quantity_Color& Color() const { return myColor; }
  Standard_Real Scale() const { return myScale; }
  Standard_Boolean IsInfinity() const { return myIsInfinity; }
  const gp_Pnt& Position() const { return myPosition; }
  Standard_Real LineThickness() const { return myLineThickness; }

  //! Setters
  void SetIsBackground (Standard_Boolean theValue) { myIsBackground = theValue; }
  void SetIsDrawAxis (Standard_Boolean theValue) { myIsDrawAxis = theValue; }
  void SetColor (const Quantity_Color& theColor) { myColor = theColor; }
  void SetScale (Standard_Real theScale) { myScale = theScale; }
  void SetIsInfinity (Standard_Boolean theValue) { myIsInfinity = theValue; }
  void SetPosition (const gp_Pnt& thePosition) { myPosition = thePosition; }
  void SetLineThickness (Standard_Real theThickness) { myLineThickness = theThickness; }

private:

  Quantity_Color   myColor;
  gp_Pnt           myPosition;
  Standard_Real    myScale;
  Standard_Real    myLineThickness;
  Standard_Boolean myIsBackground;
  Standard_Boolean myIsDrawAxis;
  Standard_Boolean myIsInfinity;
};

#endif // _Aspect_GridParams_HeaderFile
