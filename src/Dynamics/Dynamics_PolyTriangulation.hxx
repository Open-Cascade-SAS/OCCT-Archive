// Copyright (c) 2017-2021 OPEN CASCADE SAS
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

#ifndef _Dynamics_PolyTriangulation_HeaderFile
#define _Dynamics_PolyTriangulation_HeaderFile

#include <Poly_Triangulation.hxx>

#ifdef HAVE_BULLET

#include <Dynamics_Internal.hxx>

//! Interface for Poly_Triangulation defining continuous triangulation data.
ATTRIBUTE_ALIGNED16(class) Dynamics_PolyTriangulation : public btStridingMeshInterface
{
public:
  BT_DECLARE_ALIGNED_ALLOCATOR()

  //! Constructor.
  Standard_EXPORT Dynamics_PolyTriangulation (const Handle(Poly_Triangulation)& thePolyTri);

  //! Destructor.
  Standard_EXPORT virtual ~Dynamics_PolyTriangulation();

  //! Get read/write access to a subpart of a triangle mesh.
  Standard_EXPORT virtual void getLockedVertexIndexBase (unsigned char** theVertexBase,
                                                         int& theNbVerts,
                                                         PHY_ScalarType& theType,
                                                         int& theStride,
                                                         unsigned char** theIndexBase,
                                                         int& theIndexStride,
                                                         int& theNbFaces,
                                                         PHY_ScalarType& theIndicesType,
                                                         int theSubPart) Standard_OVERRIDE;

  //! Get read access to a subpart of a triangle mesh.
  Standard_EXPORT virtual void getLockedReadOnlyVertexIndexBase (const unsigned char** theVertexBase,
                                                                 int& theNbVerts,
                                                                 PHY_ScalarType& theType,
                                                                 int& theStride,
                                                                 const unsigned char** theIndexBase,
                                                                 int& theIndexStride,
                                                                 int& theNbFaces,
                                                                 PHY_ScalarType& theIndicesType,
                                                                 int theSubPart) const Standard_OVERRIDE;

  //! Finishes the access to a subpart of the triangle mesh.
  virtual void unLockVertexBase         (int theSubPart)       Standard_OVERRIDE { (void )theSubPart; }
  virtual void unLockReadOnlyVertexBase (int theSubPart) const Standard_OVERRIDE { (void )theSubPart; }

  //! Returns the number of separate subparts; each subpart has a continuous array of vertices and indices.
  virtual int getNumSubParts() const Standard_OVERRIDE { return 1; }

  virtual void preallocateVertices (int theNbVerts)   Standard_OVERRIDE { (void )theNbVerts; }
  virtual void preallocateIndices  (int theNbIndices) Standard_OVERRIDE { (void )theNbIndices; }

  //virtual void InternalProcessAllTriangles(btInternalTriangleIndexCallback* callback,const btVector3& aabbMin,const btVector3& aabbMax) const;
  //virtual bool hasPremadeAabb() const Standard_OVERRIDE { return false; }
  //virtual void setPremadeAabb(const btVector3& aabbMin, const btVector3& aabbMax ) const Standard_OVERRIDE;
  //virtual void getPremadeAabb(btVector3* aabbMin, btVector3* aabbMax ) const Standard_OVERRIDE

protected:

  Handle(Poly_Triangulation) myPolyTri;

};

#endif // HAVE_BULLET

#endif // _Dynamics_PolyTriangulation_HeaderFile
