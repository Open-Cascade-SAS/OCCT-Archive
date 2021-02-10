// Created on: 2015-12-10
// Created by: Vlad Romashko
// Copyright (c) 2015 OPEN CASCADE SAS
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

#ifndef _TDataXtd_SurfacicMesh_HeaderFile
#define _TDataXtd_SurfacicMesh_HeaderFile

#include <Standard.hxx>
#include <Standard_Type.hxx>

#include <Poly_Mesh.hxx>
#include <TDF_Attribute.hxx>
#include <Standard_Boolean.hxx>
#include <Standard_Integer.hxx>
#include <Standard_OStream.hxx>
#include <Standard_GUID.hxx>

class TDF_Label;
class TDF_RelocationTable;

class TDataXtd_SurfacicMesh;
DEFINE_STANDARD_HANDLE(TDataXtd_SurfacicMesh, TDF_Attribute)

//! An Ocaf attribute containing a mesh (Poly_Mesh).
//! It includes all methods of Poly_Mesh (and Poly_Triangulation).
class TDataXtd_SurfacicMesh : public TDF_Attribute
{
public:
  
  //! Static methods
  //  ==============

  //! Returns the ID of the mesh attribute.
  Standard_EXPORT static const Standard_GUID& GetID();
  
  //! Finds or creates a mesh attribute.
  Standard_EXPORT static Handle(TDataXtd_SurfacicMesh) Set (const TDF_Label& theLabel);

  //! Finds or creates a mesh attribute with specified ID.
  //! It allows setting several mesh-attributes at the same label.
  Standard_EXPORT static Handle(TDataXtd_SurfacicMesh) Set (const TDF_Label& theLabel,
                                                            const Standard_GUID& theID);

  //! Finds or creates a mesh attribute.
  //! Initializes the attribute by a mesh (Poly_Mesh) object.
  //! If the mesh consists of only triangles,
  //! you may put Poly_Triangulation object as a 2nd parameter of this method.
  Standard_EXPORT static Handle(TDataXtd_SurfacicMesh) Set (const TDF_Label& theLabel,
                                                            const Handle(Poly_Mesh)& theMesh);

  //! Finds or creates a mesh attribute (the same method as above).
  //! Additionally, it allows setting several mesh-attributes at the same label.
  Standard_EXPORT static Handle(TDataXtd_SurfacicMesh) Set (const TDF_Label& theLabel,
                                                            const Standard_GUID& theID,
                                                            const Handle(Poly_Mesh)& theMesh);

  //! Object methods
  //  ==============

  //! A constructor.
  //! Don't use it directly, 
  //! use please the static method Set(),
  //! which returns the attribute attached to a label.
  Standard_EXPORT TDataXtd_SurfacicMesh();

  //! Sets the explicit ID (user defined) for the attribute.
  Standard_EXPORT void SetID (const Standard_GUID& theID) Standard_OVERRIDE;

  //! Sets default ID for the attribute.
  Standard_EXPORT void SetID() Standard_OVERRIDE;

  //! Sets the mesh.
  //! If the mesh consists of only triangles,
  //! you may put Poly_Triangulation object.
  Standard_EXPORT void Set (const Handle(Poly_Mesh)& theMesh);

  //! Returns the underlying mesh.
  Standard_EXPORT const Handle(Poly_Mesh)& Get() const;


  //! Poly_Mesh methods
  //  =================

  //! The methods are "covered" by this attribute to prevent direct modification of the mesh.

  //! Returns the deflection of this triangulation.
  Standard_EXPORT Standard_Real Deflection() const;

  //! Sets the deflection of this mesh to theDeflection.
  Standard_EXPORT void Deflection (const Standard_Real theDeflection);

  //! Deallocates the UV nodes.
  Standard_EXPORT void RemoveUVNodes();

  //! @return the number of nodes for this mesh.
  Standard_EXPORT Standard_Integer NbNodes() const;

  //! @return the number of triangles for this mesh.
  Standard_EXPORT Standard_Integer NbTriangles() const;

  //! @return the number of quadrangles for this mesh.
  Standard_EXPORT Standard_Integer NbQuads() const;

  //! @return Standard_True if 2D nodes are associated with 3D nodes for this mesh.
  Standard_EXPORT Standard_Boolean HasUVNodes() const;

  //! Sets a node by index.
  Standard_EXPORT void SetNode (const Standard_Integer& theIndex, const gp_Pnt& theNode);

  //! Returns a node by index.
  Standard_EXPORT const gp_Pnt& Node (const Standard_Integer theIndex) const;

  //! Sets a UV-node by index.
  Standard_EXPORT void SetUVNode (const Standard_Integer theIndex, const gp_Pnt2d& theUVNode);

  //! Returns an UV-node by index.
  Standard_EXPORT const gp_Pnt2d& UVNode (const Standard_Integer theIndex) const;

  //! Sets a triangle by index.
  Standard_EXPORT void SetTriangle (const Standard_Integer theIndex, const Poly_Triangle& theTriangle);

  //! Returns a triangle by index.
  Standard_EXPORT const Poly_Triangle& Triangle (const Standard_Integer theIndex) const;

  //! Sets a quadrangle by index.
  Standard_EXPORT void SetQuad (const Standard_Integer theIndex, const Poly_Quad& theQuad);

  //! Returns a quadrangle by index.
  Standard_EXPORT const Poly_Quad& Quad (const Standard_Integer theIndex) const;

  //! Returns Standard_True if nodal normals are defined.
  Standard_EXPORT Standard_Boolean HasNormals() const;

  //! Sets normal by index.
  Standard_EXPORT void SetNormal (const Standard_Integer theIndex,
                                  const gp_XYZ&          theNormal);

  //! Sets normal by index.
  Standard_EXPORT void SetNormal (const Standard_Integer theIndex,
                                  const Standard_ShortReal theNormalX,
                                  const Standard_ShortReal theNormalY, 
                                  const Standard_ShortReal theNormalZ);

  //! Returns normal by index.
  Standard_EXPORT const Vec3f& Normal(const Standard_Integer theIndex) const;

  //! Returns normal by index.
  Standard_EXPORT void Normal(const Standard_Integer theIndex, gp_XYZ& theNormal) const;

  //! Dumps the content of me into the stream
  Standard_EXPORT virtual void DumpJson(Standard_OStream& theOStream, Standard_Integer theDepth = -1) const Standard_OVERRIDE;


  //! Inherited attribute methods
  //  ===========================

  Standard_EXPORT const Standard_GUID& ID() const Standard_OVERRIDE;
  
  Standard_EXPORT void Restore (const Handle(TDF_Attribute)& theWithMesh) Standard_OVERRIDE;
  
  Standard_EXPORT Handle(TDF_Attribute) NewEmpty() const Standard_OVERRIDE;
  
  Standard_EXPORT void Paste (const Handle(TDF_Attribute)& theIntoMesh, const Handle(TDF_RelocationTable)& theRT) const Standard_OVERRIDE;
  
  Standard_EXPORT virtual Standard_OStream& Dump (Standard_OStream& theOS) const Standard_OVERRIDE;

  DEFINE_STANDARD_RTTI_INLINE(TDataXtd_SurfacicMesh,TDF_Attribute)

private:

  Handle(Poly_Mesh) myMesh;
  Standard_GUID     myID;
};

#endif // _TDataXtd_SurfacicMesh_HeaderFile
