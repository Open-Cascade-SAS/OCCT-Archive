// Created on: 2023-29-11
// Created by: Irina KOCHETKOVA
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

#ifndef _BRepAlgoAPI_HullTransform_HeaderFile
#define _BRepAlgoAPI_HullTransform_HeaderFile

#include <Standard.hxx>
#include <BRepBuilderAPI_HullTransform.hxx>

#include <list>

//! The class implements an algorithm of hull transformtation according to the custom transformation function
//! (see BRepTools_HullTransformation for more information).
//! Steps of preparation before transformation
//! - Convert to nurbs
//! - Add sections parallel to YZ plane to decrease the tolerance.
//! The class also implements an autosectioning method, which calculates the set of sections
//! to decrease the tolerance as much as possible.
//! It iterationally tries to do a transformation, find the most problematic (with the biggest tolerance) place,
//! and add the section there.
class BRepAlgoAPI_HullTransform
{
public:

  DEFINE_STANDARD_ALLOC

public: 

  //! Empty constructor.
  Standard_EXPORT BRepAlgoAPI_HullTransform();

  //! Sets flag for performing auto generation of sections.
  //! @param theAutoFlag [in] true if the autosection is necessary, false otherwise.
  //! @param theTolerance [in] minimal distance between two sections.
  void SetAutoSections(const bool theAutoFlag, double theTolerance = 0)
  {
    myIsAutoSection = theAutoFlag;
    myTolerance = theTolerance;
  }

  //! Sets predefined sections
  void SetSections(const std::list<double>& theSections)
  {
    mySections = theSections;
  }

  //! Sets the Hull.
  void SetHull(const TopoDS_Shape& theHull)
  {
    myHull = theHull;
  }

  //! Initialize parameters for linear transformation function.
  Standard_EXPORT void InitLinear(const double theCM,
                                  const double theCBNew,
                                  const double theCBOld,
                                  const double theLPP);

  //! Initialize parameters for quad transformation function.
  Standard_EXPORT void InitQuad(const double theAftlim,
                                const double theCCA,
                                const double theCCF,
                                const double theForelim,
                                const double theAftCoef,
                                const double theForeCoef,
                                const bool theModifyAftZone,
                                const bool theModifyForeZone);

  //! Performs the main calculations.
  //! @return the result shape, empty in case of fail.
  Standard_EXPORT TopoDS_Shape Perform();

  //! Returns sections, used for the transformatoin.
  std::list<double> Sections()
  {
    return mySections;
  }

private:

  //! Returns the result of splitting myHyll by mySections
  TopoDS_Shape split();

  //! Finds the first vertex in theHull with the biggest tolerance, with valid X position using already added
  //! sections and myTolerance and add its X position to the mySections.
  void addSection(const TopoDS_Shape& theHull);

private:

  bool myIsAutoSection;         //!< auto section mode
  double myTolerance;           //!< tolerance for autosectioning algorithm
  std::list<double> mySections; //!< sections
  TopoDS_Shape myHull;          //!< the initial Hull, converted to nurbs in Perform()

  bool myLinear;                //!< Flag of linear.quad transformation function

  //! Linear parameters
  double _cm;
  double _cb_new;
  double _cb_old;
  double _lpp;

  //! Quad parameters
  double _aftlim;
  double _cca;
  double _ccf;
  double _forelim;
  double _aft_coef;
  double _fore_coef;
  bool _modify_aft_zone;
  bool _modify_fore_zone;
};

#endif // _BRepAlgoAPI_HullTransform_HeaderFile
