// Created on: 1995-03-06
// Created by: Laurent PAINNOT
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

#ifndef _Poly_Triangulation_HeaderFile
#define _Poly_Triangulation_HeaderFile

#include <Bnd_Box.hxx>
#include <Standard.hxx>
#include <Standard_DefineHandle.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <TColgp_HArray1OfPnt2d.hxx>
#include <Poly_Array1OfTriangle.hxx>
#include <TShort_HArray1OfShortReal.hxx>

typedef NCollection_Vec3 <Standard_ShortReal> Vec3f;

//! Provides a triangulation for a surface, a set of surfaces, or more generally a shape.
//! A triangulation consists of an approximate representation of the actual shape, using a collection of points and triangles.
//! The points are located on the surface. The edges of the triangles connect adjacent points with a
//! straight line that approximates the true curve on the surface.
//! A triangulation comprises:
//! -   A table of 3D nodes (3D points on the surface).
//! -   A table of triangles. Each triangle (Poly_Triangle object) comprises a triplet of indices in the table of 3D
//!     nodes specific to the triangulation.
//! -   A table of 2D nodes (2D points), parallel to the table of 3D nodes. This table is optional.
//!     If it exists, the coordinates of a 2D point are the (u, v) parameters of the corresponding 3D point on the surface approximated by the triangulation.
//! -   A deflection (optional), which maximizes the distance from a point on the surface to the corresponding point on its approximate triangulation.
//! In many cases, algorithms do not need to work with the exact representation of a surface.
//! A triangular representation induces simpler and more robust adjusting, faster performances, and the results are as good.
class Poly_Triangulation : public Standard_Transient
{

public:

  DEFINE_STANDARD_RTTIEXT(Poly_Triangulation, Standard_Transient)

  //! Constructs an empty triangulation.
  Standard_EXPORT Poly_Triangulation();

  //! Constructs a triangulation from a set of triangles. The
  //! triangulation is initialized without a triangle or a node, but capable of
  //! containing nbNodes nodes, and nbTriangles
  //! triangles. Here the UVNodes flag indicates whether
  //! 2D nodes will be associated with 3D ones, (i.e. to
  //! enable a 2D representation).
  Standard_EXPORT Poly_Triangulation(const Standard_Integer nbNodes, const Standard_Integer nbTriangles, const Standard_Boolean hasUVNodes);

  //! Constructs a triangulation from a set of triangles.
  //! The triangulation is initialized without a triangle or a node,
  //! but capable of containing nbNodes nodes, and nbTriangles triangles.
  //! Here the UVNodes flag indicates whether 2D nodes will be associated with 3D ones,
  //! (i.e. to enable a 2D representation).
  //! Here the hasNormals flag indicates whether normals will be given and associated with nodes.
  Standard_EXPORT Poly_Triangulation(const Standard_Integer nbNodes,
                                     const Standard_Integer nbTriangles,
                                     const Standard_Boolean hasUVNodes,
                                     const Standard_Boolean hasNormals);

  //! Constructs a triangulation from a set of triangles. The
  //! triangulation is initialized with 3D points from Nodes and triangles
  //! from Triangles.
  Standard_EXPORT Poly_Triangulation(const TColgp_Array1OfPnt& Nodes, const Poly_Array1OfTriangle& Triangles);

  //! Constructs a triangulation from a set of triangles. The
  //! triangulation is initialized with 3D points from Nodes, 2D points from
  //! UVNodes and triangles from Triangles, where
  //! coordinates of a 2D point from UVNodes are the
  //! (u, v) parameters of the corresponding 3D point
  //! from Nodes on the surface approximated by the
  //! constructed triangulation.
  Standard_EXPORT Poly_Triangulation(const TColgp_Array1OfPnt& Nodes, const TColgp_Array1OfPnt2d& UVNodes, const Poly_Array1OfTriangle& Triangles);

  //! Destructor
  Standard_EXPORT virtual ~Poly_Triangulation();

  //! Creates full copy of current triangulation
  Standard_EXPORT virtual Handle(Poly_Triangulation) Copy() const;

  //! Copy constructor for triangulation.
  Standard_EXPORT Poly_Triangulation (const Handle(Poly_Triangulation)& theTriangulation);

  //! Returns the deflection of this triangulation.
  Standard_Real Deflection() const { 
    return myDeflection;
  }

  //! Sets the deflection of this triangulation to theDeflection.
  //! See more on deflection in Polygon2D
  void Deflection (const Standard_Real theDeflection) {
    myDeflection = theDeflection;
  }

  //! Deallocates the UV nodes.
  Standard_EXPORT void RemoveUVNodes();

  //! Returns TRUE if triangulation has some geometry.
  virtual Standard_Boolean HasGeometry() const {
    return !myNodes.IsEmpty() && !myTriangles.IsEmpty();
  }

  //! Returns the number of nodes for this triangulation.
  Standard_Integer NbNodes() const {
    return myNodes.Size();
  }

  //! Returns the number of triangles for this triangulation.
  Standard_Integer NbTriangles() const {
    return myTriangles.Size();
  }

  //! Sets a node coordinates.
  void SetNode (const Standard_Integer theIndex,
                const gp_Pnt&          thePnt)
  {
    myNodes.SetValue (theIndex, thePnt);
  }

  //! Returns a node at the given index.
  const gp_Pnt& Node (const Standard_Integer theIndex) const {
    return myNodes.Value (theIndex);
  }

  //! Give access to the node at the given index.
  gp_Pnt& ChangeNode (const Standard_Integer theIndex) {
    return myNodes.ChangeValue (theIndex);
  }

  //! Returns Standard_True if 2D nodes are associated with 3D nodes for this triangulation.
  Standard_Boolean HasUVNodes() const {
    return myHasUVNodes;
  }

  //! Sets an UV-node coordinates.
  void SetUVNode (const Standard_Integer theIndex,
                  const gp_Pnt2d&        thePnt)
  {
    myUVNodes.SetValue (theIndex, thePnt);
  }

  //! Returns UV-node at the given index.
  const gp_Pnt2d& UVNode (const Standard_Integer theIndex) const {
    return myUVNodes.Value (theIndex);
  }

  //! Give access to the UVNode at the given index.
  gp_Pnt2d& ChangeUVNode (const Standard_Integer theIndex) {
    return myUVNodes.ChangeValue (theIndex);
  }

  //! Sets a triangle.
  void SetTriangle (const Standard_Integer theIndex,
                    const Poly_Triangle&   theTriangle)
  {
    myTriangles.SetValue (theIndex, theTriangle);
  }

  //! Returns triangle at the given index.
  const Poly_Triangle& Triangle (const Standard_Integer theIndex) const {
    return myTriangles.Value (theIndex);
  }

  //! Give access to the triangle at the given index.
  //! Raises an exception if theIndex is less than 1 or greater than number of triangles.
  Poly_Triangle& ChangeTriangle (const Standard_Integer theIndex) {
    return myTriangles.ChangeValue (theIndex);
  }

  //! Returns Standard_True if nodal normals are defined.
  Standard_Boolean HasNormals() const {
    return !myNormals.IsEmpty() && myNormals.Length() == NbNodes();
  }

  //! Changes normal at the given index.
  void SetNormal (const Standard_Integer theIndex,
                  const Vec3f&           theNormal)
  {
    // If an array for normals is not allocated yet, do it now.
    if (myNormals.IsEmpty() || myNormals.Size() != myNodes.Size())
      myNormals.Resize (1, myNodes.Size(), Standard_False);

    // Set a normal.
    myNormals.ChangeValue (theIndex) = theNormal;
  }

  //! Changes normal at the given index.
  void SetNormal (const Standard_Integer theIndex,
                  const gp_XYZ&          theNormal)
  {
    SetNormal (theIndex, static_cast<Standard_ShortReal>(theNormal.X()),
                         static_cast<Standard_ShortReal>(theNormal.Y()),
                         static_cast<Standard_ShortReal>(theNormal.Z()));
  }

  //! Changes normal at the given index.
  void SetNormal (const Standard_Integer theIndex,
                  const Standard_ShortReal theNormalX,
                  const Standard_ShortReal theNormalY,
                  const Standard_ShortReal theNormalZ)
  {
    SetNormal (theIndex, Vec3f (theNormalX, theNormalY, theNormalZ));
  }

  //! Returns normal at the given index.
  const Vec3f& Normal (const Standard_Integer theIndex) const {
    return myNormals (theIndex);
  }

  //! Returns normal at the given index.
  void Normal (const Standard_Integer theIndex,
               gp_XYZ&                theNormal) const
  {
    const Vec3f& aCoords = Normal (theIndex);
    theNormal.SetCoord (aCoords.x(), aCoords.y(), aCoords.z());
  }

  //! Sets the table of node normals.
  //! Raises exception if length of theNormals != 3 * NbNodes
  Standard_DEPRECATED("Deprecated method SetNormals() should be replaced \
 by method with array as object instead of handle. \
 Array of floats should be replaced by vector of normals")
  Standard_EXPORT void SetNormals (const Handle(TShort_HArray1OfShortReal)& theNormals);

  //! An advanced method resizing an internal array of nodes.
  //! The old nodes are copied into the new array.
  //! UV-nodes as well as normals, if exist are extended and copied too.
  Standard_EXPORT void ResizeNodes (const Standard_Integer theNbNodes);

  //! An advanced method resizing an internal array of triangles.
  //! The old triangles are copied into the new array.
  Standard_EXPORT void ResizeTriangles (const Standard_Integer theNbTriangles);

  //! Returns cached min - max range of triangulation data,
  //! which is VOID by default (e.g, no cached information).
  Standard_EXPORT const Bnd_Box& CachedMinMax() const;

  //! Sets a cached min - max range of this triangulation.
  //! The bounding box should exactly match actual range of triangulation data
  //! without a gap or transformation, or otherwise undefined behavior will be observed.
  //! Passing a VOID range invalidates the cache.
  Standard_EXPORT void SetCachedMinMax (const Bnd_Box& theBox);

  //! Returns TRUE if there is some cached min - max range of this triangulation.
  Standard_EXPORT Standard_Boolean HasCachedMinMax() const { return myCachedMinMax != NULL; }

  //! Updates cached min - max range of this triangulation with bounding box of nodal data.
  void UpdateCachedMinMax()
  {
    Bnd_Box aBox;
    MinMax (aBox, gp_Trsf(), true);
    SetCachedMinMax (aBox);
  }

  //! Extends the passed box with bounding box of this triangulation.
  //! Uses cached min - max range when available and:
  //! - input transformation theTrsf has no rotation part;
  //! - theIsAccurate is set to FALSE;
  //! - no triangulation data available (e.g. it is deferred and not loaded).
  //! @param theBox [in] [out] bounding box to extend by this triangulation
  //! @param theTrsf [in] optional transformation
  //! @param theIsAccurate [in] when FALSE, allows using a cached min - max range of this triangulation
  //!                           even for non-identity transformation.
  //! @return FALSE if there is no any data to extend the passed box (no both triangulation and cached min - max range).
  Standard_EXPORT Standard_Boolean MinMax (Bnd_Box& theBox, const gp_Trsf& theTrsf, const bool theIsAccurate = false) const;

  //! Dumps the content of me into the stream
  Standard_EXPORT virtual void DumpJson (Standard_OStream& theOStream, Standard_Integer theDepth = -1) const;

protected:

  //! Clears cached min - max range saved previously.
  Standard_EXPORT void unsetCachedMinMax();

  //! Calculates bounding box of nodal data.
  //! @param theTrsf [in] optional transformation.
  Standard_EXPORT virtual Bnd_Box computeBoundingBox (const gp_Trsf& theTrsf) const;

protected:

  Bnd_Box*                  myCachedMinMax;
  Standard_Real             myDeflection;
  Standard_Boolean          myHasUVNodes;
  TColgp_Array1OfPnt        myNodes;
  TColgp_Array1OfPnt2d      myUVNodes;
  Poly_Array1OfTriangle     myTriangles;
  NCollection_Array1<Vec3f> myNormals;
};

#endif // _Poly_Triangulation_HeaderFile
