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

#include <Dynamics_CollisionBRepShape.hxx>

#include <Dynamics_CollisionBRepShapeConvex.hxx>
#include <Dynamics_CollisionBody.hxx>

#ifdef HAVE_BULLET
  #include <Dynamics_Graphic3dTriangulation.hxx>
  #include <Dynamics_PolyTriangulation.hxx>
  #include <Dynamics_MotionState.hxx>
#endif

#include <BRep_Tool.hxx>
#include <Message.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopExp_Explorer.hxx>
#include <TopLoc_Location.hxx>

IMPLEMENT_STANDARD_RTTIEXT(Dynamics_CollisionBRepShape,       Dynamics_CollisionShape)
IMPLEMENT_STANDARD_RTTIEXT(Dynamics_CollisionBRepShapeConvex, Dynamics_CollisionBRepShape)

namespace
{
#ifdef HAVE_BULLET
  //! Wrapper over btCompoundShape with destructor deleting child shapes.
  ATTRIBUTE_ALIGNED16(class) Dynamics_CompoundShape : public btCompoundShape
  {
  public:
    //! Constructor.
    Dynamics_CompoundShape (bool theToEnableDynamicAabbTree = true, const int theInitialChildCapacity = 0)
    : btCompoundShape (theToEnableDynamicAabbTree, theInitialChildCapacity) {}

    //! Destructor.
    virtual ~Dynamics_CompoundShape()
    {
      for (int aChildIter = 0; aChildIter < getNumChildShapes(); ++aChildIter)
      {
        btCollisionShape* aShape = getChildShape (aChildIter);
        delete aShape;
      }
    }
  };

  //! Wrapper over btBvhTriangleMeshShape with destructor deleting Mesh Interface.
  ATTRIBUTE_ALIGNED16(class) Dynamics_BvhTriangleMeshShape : public btBvhTriangleMeshShape
  {
  public:
    //! Constructor.
    Dynamics_BvhTriangleMeshShape (btStridingMeshInterface* theMeshInterface,
                                   bool theToUseQuantizedAabbCompression,
                                   bool theToBuildBvh = true)
    : btBvhTriangleMeshShape (theMeshInterface, theToUseQuantizedAabbCompression, theToBuildBvh) {}

    //! Destructor.
    virtual ~Dynamics_BvhTriangleMeshShape()
    {
      delete m_meshInterface;
      m_meshInterface = NULL;
    }
  };

  //! Wrapper over btConvexTriangleMeshShape with destructor deleting Mesh Interface.
  ATTRIBUTE_ALIGNED16(class) Dynamics_ConvexTriangleMeshShape : public btConvexTriangleMeshShape
  {
  public:
    //! Constructor.
    Dynamics_ConvexTriangleMeshShape (btStridingMeshInterface* theMeshInterface,
                                      bool theToBuildBvh = true)
    : btConvexTriangleMeshShape (theMeshInterface, theToBuildBvh) {}

    //! Destructor.
    virtual ~Dynamics_ConvexTriangleMeshShape()
    {
      btStridingMeshInterface* aMeshInterface = getMeshInterface();
      delete aMeshInterface;
    }
  };

  //! Wrap triangulation into mesh interface.
  btStridingMeshInterface* wrapTriangulation (const Handle(Poly_Triangulation)& thePolyTri)
  {
    if (!thePolyTri.IsNull())
    {
      return new Dynamics_PolyTriangulation (thePolyTri);
    }
    return NULL;
  }
#endif
}

// =======================================================================
// function : Init
// purpose  :
// =======================================================================
bool Dynamics_CollisionBRepShape::Init (const TopoDS_Shape& theShape,
                                        Dynamics_CollisionBRepFlags theFlags)
{
  releaseShape();
  if (theShape.IsNull())
  {
    return false;
  }

#ifdef HAVE_BULLET
  const bool toUseCompression = (theFlags & Dynamics_CollisionBRepFlags_Compressed) != 0;
  const bool toBuildBvh       = (theFlags & Dynamics_CollisionBRepFlags_BuildBVH) != 0;
  const bool isConvex         = (theFlags & Dynamics_CollisionBRepFlags_Convex) != 0;

  TopLoc_Location aLoc;
  btTransform aTrsfBtLocal;
  aTrsfBtLocal.setIdentity();
  if ((theFlags & Dynamics_CollisionBRepFlags_Single) != 0)
  {
    int aNbNodes = 0, aNbTris = 0;
    for (TopExp_Explorer aFaceIter (theShape, TopAbs_FACE); aFaceIter.More(); aFaceIter.Next())
    {
      const TopoDS_Face& aFace = TopoDS::Face (aFaceIter.Current());
      const Handle(Poly_Triangulation)& aPolyTri = BRep_Tool::Triangulation (aFace, aLoc);
      if (!aPolyTri.IsNull())
      {
        aNbNodes += aPolyTri->NbNodes();
        aNbTris  += aPolyTri->NbTriangles();
      }
    }
    if (aNbNodes < 3
     || aNbTris < 1)
    {
      return false;
    }

    btConvexHullShape aConvexHullShape;
    aConvexHullShape.setMargin (0); // this is to compensate for a bug in bullet

    Handle(Graphic3d_ArrayOfTriangles) aTriangles;
    if (!isConvex)
    {
      aTriangles = new Graphic3d_ArrayOfTriangles (aNbNodes, aNbTris * 3, false, false, false);
    }
    for (TopExp_Explorer aFaceIter (theShape, TopAbs_FACE); aFaceIter.More(); aFaceIter.Next())
    {
      const TopoDS_Face& aFace = TopoDS::Face (aFaceIter.Current());
      const Handle(Poly_Triangulation)& aPolyTri = BRep_Tool::Triangulation (aFace, aLoc);
      const gp_Trsf aTrsf = aLoc;
      if (!aPolyTri.IsNull())
      {
        const int aFirstNode = !aTriangles.IsNull() ? aTriangles->VertexNumber() : 0;
        for (int aNodeIter = 1; aNodeIter <= aPolyTri->NbNodes(); ++aNodeIter)
        {
          gp_Pnt aNode = aPolyTri->Node (aNodeIter);
          aNode.Transform (aTrsf);
          if (!aTriangles.IsNull())
          {
            aTriangles->AddVertex (aNode);
          }
          else
          {
            aConvexHullShape.addPoint (btVector3 (btScalar (aNode.X()), btScalar (aNode.Y()), btScalar (aNode.Z())), false);
          }
        }
        if (!aTriangles.IsNull())
        {
          Graphic3d_Vec4i anElem (-1, -1, -1, -1);
          for (int aTriIter = 1; aTriIter <= aPolyTri->NbTriangles(); ++aTriIter)
          {
            aPolyTri->Triangle (aTriIter).Get (anElem[0], anElem[1], anElem[2]);
            aTriangles->AddEdges (aFirstNode + anElem[0], aFirstNode + anElem[1], aFirstNode + anElem[2]);
          }
        }
      }
    }

    if (isConvex)
    {
      // create a hull approximation
      //btConvexHullShape aConvexHullShape ((const btScalar* )aTriangles->Attributes()->Data(), aNbNodes, aTriangles->Attributes()->Stride);
      aConvexHullShape.recalcLocalAabb();

      btShapeHull aHull (&aConvexHullShape);
      aHull.buildHull (0);
      if (aHull.numVertices() == 0)
      {
        Message::SendTrace ("Dynamics_CollisionBRepShape, btShapeHull returned an empty result");
        return false;
      }
      myShape = new btConvexHullShape ((const btScalar* )aHull.getVertexPointer(), aHull.numVertices(), sizeof(btVector3));
    }
    else
    {
      btStridingMeshInterface* aMeshInterface = new Dynamics_Graphic3dTriangulation (aTriangles);
      myShape = new Dynamics_BvhTriangleMeshShape (aMeshInterface, toUseCompression, toBuildBvh);
    }
    return true;
  }

  Dynamics_CompoundShape* aCompShape = new Dynamics_CompoundShape();
  myShape = aCompShape;
  for (TopExp_Explorer aFaceIter (theShape, TopAbs_FACE); aFaceIter.More(); aFaceIter.Next())
  {
    const TopoDS_Face& aFace = TopoDS::Face (aFaceIter.Current());
    const Handle(Poly_Triangulation)& aPolyTri = BRep_Tool::Triangulation (aFace, aLoc);
    if (btStridingMeshInterface* aChild = wrapTriangulation (aPolyTri))
    {
      Dynamics_MotionState::TrsfBtFromGp (aTrsfBtLocal, aLoc.Transformation());
      if (isConvex)
      {
        aCompShape->addChildShape (aTrsfBtLocal, new Dynamics_ConvexTriangleMeshShape (aChild, toBuildBvh));
      }
      else
      {
        aCompShape->addChildShape (aTrsfBtLocal, new Dynamics_BvhTriangleMeshShape (aChild, toUseCompression, toBuildBvh));
      }
    }
  }
  if (aCompShape->getNumChildShapes() == 0)
  {
    releaseShape();
    return false;
  }
  return true;
#else
  (void )theFlags;
  return false;
#endif
}
