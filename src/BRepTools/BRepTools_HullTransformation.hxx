// Created on: 2023-07-07
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

#ifndef _BRepTools_HullTransformation_HeaderFile
#define _BRepTools_HullTransformation_HeaderFile

#include <BRepTools_Modification.hxx>
//class gp_GTrsf;
//class TopoDS_Face;
//class Geom_Surface;
//class TopLoc_Location;
//class TopoDS_Edge;
//class Geom_Curve;
//class TopoDS_Vertex;
//class gp_Pnt;
//class Geom2d_Curve;


class BRepTools_HullTransformation;
DEFINE_STANDARD_HANDLE(BRepTools_HullTransformation, BRepTools_Modification)

//! Defines a modification of the geometry by a custom function.
//! All methods return True and transform the geometry.
class BRepTools_HullTransformation : public BRepTools_Modification
{

public:

  
  Standard_EXPORT BRepTools_HullTransformation();
  
  //! Calculate a new point's position.
  Standard_EXPORT gp_Pnt NewPointFrom(const gp_Pnt& theOldP);
  
  //! Returns Standard_True  if  the face  <F> has  been
  //! modified.  In this  case, <S> is the new geometric
  //! support of  the  face, <L> the  new location,<Tol>
  //! the new  tolerance.<RevWires> has  to  be set   to
  //! Standard_True   when the modification reverses the
  //! normal of  the   surface.(the wires   have  to  be
  //! reversed).   <RevFace>   has   to   be   set    to
  //! Standard_True if  the orientation  of the modified
  //! face changes in the  shells which contain  it.  --
  //! Here, <RevFace>  will  return Standard_True if the
  //! -- gp_Trsf is negative.
  Standard_EXPORT Standard_Boolean NewSurface (const TopoDS_Face& F, Handle(Geom_Surface)& S, TopLoc_Location& L, Standard_Real& Tol, Standard_Boolean& RevWires, Standard_Boolean& RevFace) Standard_OVERRIDE;
  
  //! Returns Standard_True  if  the edge  <E> has  been
  //! modified.  In this case,  <C> is the new geometric
  //! support of the  edge, <L> the  new location, <Tol>
  //! the         new    tolerance.   Otherwise, returns
  //! Standard_False,    and  <C>,  <L>,   <Tol> are not
  //! significant.
  Standard_EXPORT Standard_Boolean NewCurve (const TopoDS_Edge& E, Handle(Geom_Curve)& C, TopLoc_Location& L, Standard_Real& Tol) Standard_OVERRIDE;
  
  //! Returns  Standard_True if the  vertex <V> has been
  //! modified.  In this  case, <P> is the new geometric
  //! support of the vertex,   <Tol> the new  tolerance.
  //! Otherwise, returns Standard_False, and <P>,  <Tol>
  //! are not significant.
  Standard_EXPORT Standard_Boolean NewPoint (const TopoDS_Vertex& V, gp_Pnt& P, Standard_Real& Tol) Standard_OVERRIDE;
  
  //! Returns Standard_True if  the edge  <E> has a  new
  //! curve on surface on the face <F>.In this case, <C>
  //! is the new geometric support of  the edge, <L> the
  //! new location, <Tol> the new tolerance.
  //! Otherwise, returns  Standard_False, and <C>,  <L>,
  //! <Tol> are not significant.
  Standard_EXPORT Standard_Boolean NewCurve2d (const TopoDS_Edge& E, const TopoDS_Face& F, const TopoDS_Edge& NewE, const TopoDS_Face& NewF, Handle(Geom2d_Curve)& C, Standard_Real& Tol) Standard_OVERRIDE;
  
  //! Returns Standard_True if the Vertex  <V> has a new
  //! parameter on the  edge <E>. In  this case,  <P> is
  //! the parameter,    <Tol>  the     new    tolerance.
  //! Otherwise, returns Standard_False, and <P>,  <Tol>
  //! are not significant.
  Standard_EXPORT Standard_Boolean NewParameter (const TopoDS_Vertex& V, const TopoDS_Edge& E, Standard_Real& P, Standard_Real& Tol) Standard_OVERRIDE;
  
  //! Returns the  continuity of  <NewE> between <NewF1>
  //! and <NewF2>.
  //!
  //! <NewE> is the new  edge created from <E>.  <NewF1>
  //! (resp. <NewF2>) is the new  face created from <F1>
  //! (resp. <F2>).
  Standard_EXPORT GeomAbs_Shape Continuity (const TopoDS_Edge& E, const TopoDS_Face& F1, const TopoDS_Face& F2, const TopoDS_Edge& NewE, const TopoDS_Face& NewF1, const TopoDS_Face& NewF2) Standard_OVERRIDE;

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
  
  DEFINE_STANDARD_RTTIEXT(BRepTools_HullTransformation,BRepTools_Modification)

private:

  Standard_Real myScale;
  bool myLinear;

  double _cm;
  double _cb_new;
  double _cb_old;
  double _lpp;

  double _aftlim;
  double _cca;
  double _ccf;
  double _forelim;
  double _aft_coef;
  double _fore_coef;
  bool _modify_aft_zone;
  bool _modify_fore_zone;

};

#endif // _BRepTools_HullTransformation_HeaderFile
