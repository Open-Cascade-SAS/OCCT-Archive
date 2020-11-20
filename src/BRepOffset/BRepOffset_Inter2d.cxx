// Created on: 1996-09-03
// Created by: Yves FRICAUD
// Copyright (c) 1996-1999 Matra Datavision
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

//  Modified by skv - Wed Dec 24 18:08:39 2003 OCC4455

#include <BRepOffset_Inter2d.hxx>

#include <Adaptor2d_Curve2d.hxx>
#include <Adaptor3d_CurveOnSurface.hxx>
#include <Adaptor3d_Surface.hxx>
#include <Bnd_Box.hxx>
#include <BndLib_Add3dCurve.hxx>
#include <BOPTools_AlgoTools.hxx>
#include <BRep_Builder.hxx>
#include <BRep_CurveRepresentation.hxx>
#include <BRep_GCurve.hxx>
#include <BRep_ListIteratorOfListOfCurveRepresentation.hxx>
#include <BRep_TEdge.hxx>
#include <BRep_Tool.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepAdaptor_Curve2d.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <BRepAlgo_AsDes.hxx>
#include <BRepAlgo_Image.hxx>
#include <BRepLib.hxx>
#include <BRepLib_MakeVertex.hxx>
#include <BRepOffset_Analyse.hxx>
#include <BRepOffset_Offset.hxx>
#include <BRepOffset_Tool.hxx>
#include <BRepOffset_MakeOffset.hxx>
#include <BRepTools.hxx>
#include <BRepTools_WireExplorer.hxx>
#include <Geom2d_BezierCurve.hxx>
#include <Geom2d_BSplineCurve.hxx>
#include <Geom2d_Line.hxx>
#include <Geom2d_TrimmedCurve.hxx>
#include <Geom2dAdaptor_Curve.hxx>
#include <Geom2dConvert_CompCurveToBSplineCurve.hxx>
#include <Geom2dInt_GInter.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Geom_BSplineSurface.hxx>
#include <Geom_ConicalSurface.hxx>
#include <Geom_CylindricalSurface.hxx>
#include <Geom_Line.hxx>
#include <Geom_Plane.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <GeomAdaptor_Surface.hxx>
#include <GeomAPI_ProjectPointOnCurve.hxx>
#include <GeomAPI_Interpolate.hxx>
#include <ShapeConstruct_ProjectCurveOnSurface.hxx>
#include <GeomConvert_CompCurveToBSplineCurve.hxx>
#include <GeomLib.hxx>
#include <GeomProjLib.hxx>
#include <gp_Pnt.hxx>
#include <IntRes2d_IntersectionPoint.hxx>
#include <IntRes2d_IntersectionSegment.hxx>
#include <Precision.hxx>
#include <TColGeom2d_SequenceOfCurve.hxx>
#include <TColgp_Array1OfPnt2d.hxx>
#include <TColgp_SequenceOfPnt.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Iterator.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Wire.hxx>
#include <TopTools_DataMapIteratorOfDataMapOfShapeListOfShape.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopTools_DataMapOfOrientedShapeShape.hxx>
#include <TopTools_DataMapIteratorOfDataMapOfOrientedShapeShape.hxx>
#include <ElCLib.hxx>

#include <stdio.h>
#ifdef DRAW 
#include <DBRep.hxx>
#include <Geom2d_BoundedCurve.hxx>
#include <Geom_BoundedSurface.hxx>
#include <Geom_BoundedCurve.hxx>
#include <BRep_CurveOnSurface.hxx>
#include <Geom_Surface.hxx>
Standard_Boolean Inter2dAffichInt2d;
static Standard_Integer NbF2d = 0;
static Standard_Integer NbE2d = 0;
static Standard_Integer NbNewVertices  = 0;
#endif

static Standard_Boolean ExtendPCurve(Handle(Geom2d_Curve)& thePCurve,
                                     Standard_Real         theFirstOnEdge,
                                     Standard_Real         theLastOnEdge,
                                     Standard_Real         theFirstPar,
                                     Standard_Real         theLastPar,
                                     Standard_Real         theDelta);

static Standard_Boolean ComputeEndOfExtension(const Handle(Geom_Surface)&  theSurf,
                                              const gp_Pnt2d&              theStartPoint,
                                              const gp_Dir2d&              theTangent,
                                              const Standard_Real          the2Offset,
                                              //gp_Pnt2d&                   theEndPoint,
                                              Standard_Real&               theParamOnEnd,
                                              Standard_Real&               theLenToEnd,
                                              Handle(Geom2d_TrimmedCurve)& theSegment);

static void AddEdgeWithFaceToMap(const TopoDS_Edge& theEdge,
                                 const TopoDS_Face& theFace,
                                 TopTools_IndexedDataMapOfShapeListOfShape& theEFmap);

static Standard_Boolean OrientationInEdge(const TopoDS_Vertex& theVertex,
                                          const TopoDS_Edge&   theEdge,
                                          TopAbs_Orientation&  theOr);

static void GetEdgesOrientedInFace(const TopoDS_Shape& theShape,
                                   const TopoDS_Face&  theFace,
                                   const Handle(BRepAlgo_AsDes)& theAsDes,
                                   TopTools_SequenceOfShape&     theSeqEdges);

static Standard_Integer DefineClosedness(const TopoDS_Face& theFace);

//=======================================================================
//function : CommonVertex
//purpose  : 
//=======================================================================

static TopoDS_Vertex CommonVertex(TopoDS_Edge& E1,
				  TopoDS_Edge& E2)
{
  TopoDS_Vertex V1[2],V2[2],V;
  //
  TopExp::Vertices(E1,V1[0],V1[1], Standard_True);
  TopExp::Vertices(E2,V2[0],V2[1], Standard_True);
  // The first edge is the current one, the second edge is the next one.
  // We check last vertex of the first edge first.
  if (V1[1].IsSame(V2[0]) || V1[1].IsSame(V2[1])) return V1[1];
  if (V1[0].IsSame(V2[0]) || V1[0].IsSame(V2[1])) return V1[0];
  //
  return V;
}

static TopAbs_Orientation DefineOrientationOfVertexOnEdge(const TopoDS_Vertex&      theVertex,
                                                          const TopoDS_Edge&        theEdge,
                                                          const BRepOffset_Analyse& theAnalyse)
{
  TopAbs_Orientation anOr = TopAbs_EXTERNAL;
  
  const BRepOffset_ListOfInterval& aListInterv = theAnalyse.Type(theEdge);
  if (!aListInterv.IsEmpty() &&
      ((aListInterv.First().Type() != ChFiDS_Concave &&
        aListInterv.First().Type() != ChFiDS_Convex) ||
       theAnalyse.IsEdgeSmooth(theEdge)))
  {
    TopoDS_Vertex aV1, aV2;
    TopExp::Vertices(theEdge, aV1, aV2);
    if (!aV1.IsSame(aV2))
      anOr = (aV1.IsSame(theVertex))? aV1.Orientation() : aV2.Orientation();
  }

  return anOr;
}

static Standard_Boolean IsOrientationChanged(TopTools_IndexedMapOfShape& theMap,
                                             const TopoDS_Edge& theEdge)
{
  Standard_Boolean IsOrChanged = Standard_False;
  
  if (!theMap.Contains(theEdge))
    theMap.Add(theEdge);
  else
  {
    Standard_Integer anInd = theMap.FindIndex(theEdge);
    const TopoDS_Shape& anEdge = theMap(anInd);
    if (theEdge.Orientation() != anEdge.Orientation())
    {
      theMap.Substitute( anInd, theEdge );
      IsOrChanged = Standard_True;
    }
  }

  return IsOrChanged;
}


//=======================================================================
//function : Store
//purpose  : Store the vertices <theLV> into AsDes for the edge <theEdge>.
//           The vertices are added despite of the coincidence with
//           already added vertices. When all vertices for all edges
//           are added the coinciding chains of vertices should be fused
//           using FuseVertices() method.
//=======================================================================
static void Store(const TopoDS_Edge& theEdge,
                  const TopTools_ListOfShape& theLV,
                  const Standard_Real theTol,
                  const Standard_Boolean IsToUpdate,
                  Handle(BRepAlgo_AsDes) theAsDes2d,
                  TopTools_IndexedDataMapOfShapeListOfShape& theDMVV)
{
  const TopTools_ListOfShape& aLVEx = theAsDes2d->Descendant(theEdge);
  if (!IsToUpdate && aLVEx.IsEmpty()) {
    if (theLV.Extent()) theAsDes2d->Add(theEdge, theLV);
    return;
  }
  //
  GeomAPI_ProjectPointOnCurve aProjPC;
  if (IsToUpdate) {
    Standard_Real aT1, aT2;
    const Handle(Geom_Curve)& aC = BRep_Tool::Curve(theEdge, aT1, aT2);
    aProjPC.Init(aC, aT1, aT2);
  }
  //
  TopTools_MapOfShape aMV;
  TopTools_ListIteratorOfListOfShape aIt(theLV);
  for (; aIt.More(); aIt.Next()) {
    const TopoDS_Vertex& aV = TopoDS::Vertex(aIt.Value());
    if (!aMV.Add(aV)) {
      continue;
    }
    //
    const gp_Pnt& aP = BRep_Tool::Pnt(aV);
    //
    TopTools_ListOfShape aLVC;
    TopTools_ListIteratorOfListOfShape aItEx(aLVEx);
    for (; aItEx.More(); aItEx.Next()) {
      const TopoDS_Vertex& aVEx = TopoDS::Vertex(aItEx.Value());
      if (aV.IsSame(aVEx)) {
        break;
      }
      const gp_Pnt& aPEx = BRep_Tool::Pnt(aVEx);
      if (aP.IsEqual(aPEx, theTol)) {
        aLVC.Append(aVEx);
      }
    }
    //
    if (aItEx.More()) {
      continue;
    }
    //
    if (IsToUpdate) {
      // get parameter of the vertex on the edge
      aProjPC.Perform(aP);
      if (!aProjPC.NbPoints()) {
        continue;
      }
      //
      if (aProjPC.LowerDistance() > theTol) {
        continue;
      }
      //
      Standard_Real aT = aProjPC.LowerDistanceParameter();
      TopoDS_Shape aLocalShape = aV.Oriented(TopAbs_INTERNAL);
      BRep_Builder().UpdateVertex(TopoDS::Vertex(aLocalShape), aT, theEdge, theTol);
    }
    else {
      BRep_Builder().UpdateVertex(aV, theTol);
    }
    //
    if (aLVC.Extent()) {
      TopTools_ListIteratorOfListOfShape aItLV(aLVC);
      for (; aItLV.More(); aItLV.Next()) {
        const TopoDS_Shape& aVC = aItLV.Value();
        TopTools_ListOfShape* pLV = theDMVV.ChangeSeek(aVC);
        if (!pLV) {
          pLV = &theDMVV(theDMVV.Add(aVC, TopTools_ListOfShape()));
        }
        pLV->Append(aV);
      }
      //
      TopTools_ListOfShape* pLV = theDMVV.ChangeSeek(aV);
      if (!pLV) {
        pLV = &theDMVV(theDMVV.Add(aV, TopTools_ListOfShape()));
      }
      pLV->Append(aLVC);
    }
    theAsDes2d->Add(theEdge, aV);
  }
}

//=======================================================================
//function : Store
//purpose  : Store the intersection vertices between two edges into AsDes
//=======================================================================
static void  Store (const TopoDS_Edge& theE1,
                    const TopoDS_Edge& theE2,
                    const TopTools_ListOfShape& theLV1,
                    const TopTools_ListOfShape& theLV2,
                    const Standard_Real theTol,
                    Handle(BRepAlgo_AsDes) theAsDes2d,
                    TopTools_IndexedDataMapOfShapeListOfShape& theDMVV)
{
  for (Standard_Integer i = 0; i < 2; ++i) {
    const TopoDS_Edge& aE = !i ? theE1 : theE2;
    const TopTools_ListOfShape& aLV = !i ? theLV1 : theLV2;
    Store(aE, aLV, theTol, Standard_False, theAsDes2d, theDMVV);
  }
}

//=======================================================================
//function : EdgeInter
//purpose  : 
//=======================================================================

static void EdgeInter(const TopoDS_Face&              F,
                      const BRepAdaptor_Surface&      BAsurf,
                      const TopoDS_Edge&              E1,
                      const TopoDS_Edge&              E2,
                      const Handle(BRepAlgo_AsDes)&   AsDes,
                      Standard_Real                   Tol,
                      Standard_Boolean                WithOri,
                      TopTools_IndexedDataMapOfShapeListOfShape& aDMVV)
{
#ifdef DRAW
  if (Inter2dAffichInt2d) {
    char name[256];
    sprintf(name,"E2d_%d_%d",NbF2d,NbE2d++);
    DBRep::Set(name,E1);
    sprintf(name,"E2d_%d_%d",NbF2d,NbE2d++);
    DBRep::Set(name,E2);
  }
#endif

  if (E1.IsSame(E2))
    return;

  Standard_Real f[3],l[3];
  Standard_Real TolDub = 1.e-7;
  Standard_Integer i;

  BRep_Tool::Range(E1, f[1], l[1]);
  BRep_Tool::Range(E2, f[2], l[2]);

  BRepAdaptor_Curve CE1(E1,F);
  BRepAdaptor_Curve CE2(E2,F);

  TopoDS_Edge                 EI[3]; EI[1] = E1; EI[2] = E2;
  TopTools_ListOfShape        LV1;   
  TopTools_ListOfShape        LV2; 
  BRep_Builder                B;

  TopoDS_Vertex CV;
  if (!TopExp::CommonVertex( E1, E2, CV ))
    {
      BRepLib::BuildCurve3d(E1);
      BRepLib::BuildCurve3d(E2);
      
      Standard_Real TolSum = BRep_Tool::Tolerance(E1) + BRep_Tool::Tolerance(E2);
      TolSum = Max( TolSum, 1.e-5 );

      TColgp_SequenceOfPnt   ResPoints;
      TColStd_SequenceOfReal ResParamsOnE1, ResParamsOnE2;
      gp_Pnt DegPoint;
      Standard_Boolean WithDegen = BRep_Tool::Degenerated(E1) || BRep_Tool::Degenerated(E2);
      
      if (WithDegen)
        {
          Standard_Integer ideg = (BRep_Tool::Degenerated(E1))? 1 : 2;
          TopoDS_Iterator iter( EI[ideg] );
          if (iter.More())
            {
              const TopoDS_Vertex& vdeg = TopoDS::Vertex(iter.Value());
              DegPoint = BRep_Tool::Pnt(vdeg);
            }
          else
            {
              BRepAdaptor_Curve CEdeg( EI[ideg], F );
              DegPoint = CEdeg.Value( CEdeg.FirstParameter() );
            }
        }
        //
      Handle(Geom2d_Curve) pcurve1 = BRep_Tool::CurveOnSurface(E1, F, f[1], l[1]);
      Handle(Geom2d_Curve) pcurve2 = BRep_Tool::CurveOnSurface(E2, F, f[2], l[2]);
      Geom2dAdaptor_Curve GAC1(pcurve1, f[1], l[1]);
      Geom2dAdaptor_Curve GAC2(pcurve2, f[2], l[2]);
      Geom2dInt_GInter Inter2d( GAC1, GAC2, TolDub, TolDub );
      for (i = 1; i <= Inter2d.NbPoints(); i++)
        {
          gp_Pnt P3d;
          if (WithDegen)
            P3d = DegPoint;
          else
            {
              gp_Pnt2d P2d = Inter2d.Point(i).Value();
              P3d = BAsurf.Value( P2d.X(), P2d.Y() );
            }
          ResPoints.Append( P3d );
          ResParamsOnE1.Append( Inter2d.Point(i).ParamOnFirst() );
          ResParamsOnE2.Append( Inter2d.Point(i).ParamOnSecond() );
        }

      for (i = 1; i <= ResPoints.Length(); i++)
        {
          Standard_Real aT1 = ResParamsOnE1(i); //ponc1.Parameter();
          Standard_Real aT2 = ResParamsOnE2(i); //ponc2.Parameter();
          if (Precision::IsInfinite(aT1) || Precision::IsInfinite(aT2))
            {
#ifdef OCCT_DEBUG
              std::cout << "Inter2d : Solution rejected due to infinite parameter"<<std::endl;
#endif
              continue;
            }
          
          gp_Pnt P = ResPoints(i); //ponc1.Value();
          TopoDS_Vertex aNewVertex = BRepLib_MakeVertex(P);
          aNewVertex.Orientation(TopAbs_INTERNAL);
          B.UpdateVertex( aNewVertex, aT1, E1, Tol );
          B.UpdateVertex( aNewVertex, aT2, E2, Tol );
          gp_Pnt P1 = CE1.Value(aT1);
          gp_Pnt P2 = CE2.Value(aT2);
          Standard_Real dist1, dist2, dist3;
          dist1 = P1.Distance(P);
          dist2 = P2.Distance(P);
          dist3 = P1.Distance(P2);
          dist1 = Max( dist1, dist2 );
          dist1 = Max( dist1, dist3 );
          B.UpdateVertex( aNewVertex, dist1 );
          
#ifdef OCCT_DEBUG
          if (aT1 < f[1]-Tol  || aT1 > l[1]+Tol)
            {
              std::cout << "out of limit"<<std::endl;
              std::cout<<"aT1 = "<<aT1<<", f[1] = "<<f[1]<<", l[1] = "<<l[1]<<std::endl;
            }
          if (aT2 < f[2]-Tol  || aT2 > l[2]+Tol)
            {
              std::cout << "out of limit"<<std::endl;
              std::cout<<"aT2 = "<<aT2<<", f[2] = "<<f[2]<<", l[2] = "<<l[2]<<std::endl;
            }
          Standard_Real MilTol2 = 1000*Tol*Tol;
          if (P1.SquareDistance(P) >  MilTol2 || P2.SquareDistance(P) > MilTol2 || P1.Distance(P2) > 2.*Tol)
            {
              std::cout << "Inter2d : Solution rejected "<<std::endl;
              std::cout<<"P  = "<<P.X()<<" "<<P.Y()<<" "<<P.Z()<<std::endl;
              std::cout<<"P1 = "<<P1.X()<<" "<<P1.Y()<<" "<<P1.Z()<<std::endl;
              std::cout<<"P2 = "<<P2.X()<<" "<<P2.Y()<<" "<<P2.Z()<<std::endl;
              std::cout<<"MaxDist = "<<dist1<<std::endl;
            }
#endif
          //define the orientation of a new vertex
          TopAbs_Orientation OO1 = TopAbs_REVERSED;
          TopAbs_Orientation OO2 = TopAbs_REVERSED;
          if (WithOri)
            {
              BRepAdaptor_Curve2d PCE1( E1, F );
              BRepAdaptor_Curve2d PCE2( E2, F );
              gp_Pnt2d P2d1, P2d2;
              gp_Vec2d V1, V2, V1or, V2or;
              PCE1.D1( aT1, P2d1, V1 );
              PCE2.D1( aT2, P2d2, V2 );
              V1or = V1; V2or = V2;
              if (E1.Orientation() == TopAbs_REVERSED) V1or.Reverse();
              if (E2.Orientation() == TopAbs_REVERSED) V2or.Reverse();
              Standard_Real CrossProd = V2or ^ V1;
#ifdef OCCT_DEBUG
              if (Abs(CrossProd) <= gp::Resolution())
                std::cout<<std::endl<<"CrossProd = "<<CrossProd<<std::endl;
#endif
              if (CrossProd > 0.)
                OO1 = TopAbs_FORWARD;
              CrossProd = V1or ^ V2;
              if (CrossProd > 0.)
                OO2 = TopAbs_FORWARD;
            }
          LV1.Append( aNewVertex.Oriented(OO1) );
          LV2.Append( aNewVertex.Oriented(OO2) );
        }
    }
  
  //----------------------------------
  // Test at end.
  //---------------------------------
  Standard_Real U1,U2;
  Standard_Real TolConf = Tol;
  TopoDS_Vertex V1[2],V2[2];
  TopExp::Vertices(E1,V1[0],V1[1]);
  TopExp::Vertices(E2,V2[0],V2[1]);

  Standard_Integer j;
  for (j = 0; j < 2; j++) {
    if (V1[j].IsNull()) continue;
    for (Standard_Integer k = 0; k < 2; k++) {
      if (V2[k].IsNull()) continue;
      if (V1[j].IsSame(V2[k])) {
        if (AsDes->HasAscendant(V1[j])) {
          continue;
        }
      }
      //
      gp_Pnt P1 = BRep_Tool::Pnt(V1[j]);
      gp_Pnt P2 = BRep_Tool::Pnt(V2[k]);
      Standard_Real Dist = P1.Distance(P2); 
      if (Dist < TolConf) {
        Standard_Real aTol = 
          Max(BRep_Tool::Tolerance(V1[j]), BRep_Tool::Tolerance(V2[k]));
        TopoDS_Vertex V = BRepLib_MakeVertex(P1);
        U1 = (j == 0) ? f[1] : l[1];
        U2 = (k == 0) ? f[2] : l[2];
        //
        TopoDS_Shape aLocalShape = V.Oriented(TopAbs_INTERNAL);
        B.UpdateVertex(TopoDS::Vertex(aLocalShape),U1,E1,aTol);
        B.UpdateVertex(TopoDS::Vertex(aLocalShape),U2,E2,aTol);
        //
        LV1.Prepend(V.Oriented(V1[j].Orientation()));
        LV2.Prepend(V.Oriented(V2[k].Orientation()));
      }
    }
  }

  Standard_Boolean AffichPurge = Standard_False;

  if ( !LV1.IsEmpty()) {
    //----------------------------------
    // Remove all vertices.
    // There can be doubles
    //----------------------------------
    TopTools_ListIteratorOfListOfShape it1LV1,it1LV2,it2LV1;
    gp_Pnt P1,P2;
    Standard_Boolean Purge = Standard_True;

    while (Purge) {
      i = 1;
      Purge = Standard_False;
      for (it1LV1.Initialize(LV1),it1LV2.Initialize(LV2); 
           it1LV1.More(); it1LV1.Next(),it1LV2.Next()) {
        j = 1;
        it2LV1.Initialize(LV1);
        while (j < i) {      
          P1 = BRep_Tool::Pnt(TopoDS::Vertex(it1LV1.Value()));
          P2 = BRep_Tool::Pnt(TopoDS::Vertex(it2LV1.Value()));
//  Modified by skv - Thu Jan 22 18:19:04 2004 OCC4455 Begin
//           if (P1.IsEqual(P2,10*Tol)) {
          Standard_Real aTol;

          aTol = Max(BRep_Tool::Tolerance(TopoDS::Vertex(it1LV1.Value())),
                     BRep_Tool::Tolerance(TopoDS::Vertex(it2LV1.Value())));
          if (P1.IsEqual(P2,aTol)) {
//  Modified by skv - Thu Jan 22 18:19:05 2004 OCC4455 End
            LV1.Remove(it1LV1);
            LV2.Remove(it1LV2);
            if (AffichPurge) std::cout <<"Doubles removed in EdgeInter."<<std::endl;
            Purge = Standard_True;
            break;
          }
          j++;
          it2LV1.Next();
        }
        if (Purge) break;
        i++;
      }
    }
    //---------------------------------
    // Vertex storage in DS.
    //---------------------------------
    Standard_Real TolStore = BRep_Tool::Tolerance(E1) + BRep_Tool::Tolerance(E2);
    TolStore = Max (TolStore, Tol);
    Store (E1,E2,LV1,LV2,TolStore,AsDes, aDMVV);
  }
}
//=======================================================================
//function : EdgeInter
//purpose  : 
//=======================================================================

static void RefEdgeInter(const TopoDS_Face&              F,
                         const BRepAdaptor_Surface&      BAsurf,
                         const TopoDS_Edge&              E1,
                         const TopoDS_Edge&              E2,
                         const TopAbs_Orientation        theOr1,
                         const TopAbs_Orientation        theOr2,
                         const Handle(BRepAlgo_AsDes)&   AsDes,
                         Standard_Real                   Tol,
                         Standard_Boolean                WithOri,
                         const TopoDS_Vertex&            theVref,
                         BRepAlgo_Image&                 theImageVV,
                         TopTools_IndexedDataMapOfShapeListOfShape& aDMVV,
                         Standard_Boolean&               theCoincide,
                         TopoDS_Vertex&                  theVertexForE1,
                         TopoDS_Vertex&                  theVertexForE2,
                         Standard_Real&                  theParamForE1,
                         Standard_Real&                  theParamForE2)
{
#ifdef DRAW
  if (Inter2dAffichInt2d) {
    char name[256];
    sprintf(name,"E2d_%d_%d",NbF2d,NbE2d++);
    DBRep::Set(name,E1);
    sprintf(name,"E2d_%d_%d",NbF2d,NbE2d++);
    DBRep::Set(name,E2);
  }
#endif
  //
  theCoincide = Standard_False;
  //
  if (E1.IsSame(E2))
    return;

  Standard_Real f[3],l[3];
  Standard_Real TolDub = 1.e-7;
  Standard_Integer i;

  //BRep_Tool::Range(E1, f[1], l[1]);
  //BRep_Tool::Range(E2, f[2], l[2]);

  TopTools_DataMapOfShapeReal aVerParOnE1map;
  TopTools_DataMapOfShapeReal aVerParOnE2map;

  BRepAdaptor_Curve CE1(E1,F);
  BRepAdaptor_Curve CE2(E2,F);

  TopoDS_Edge                 EI[3]; EI[1] = E1; EI[2] = E2;
  TopTools_ListOfShape        LV1;   
  TopTools_ListOfShape        LV2; 
  BRep_Builder                B;

  BRepLib::BuildCurve3d(E1);
  BRepLib::BuildCurve3d(E2);

  Standard_Real TolSum = BRep_Tool::Tolerance(E1) + BRep_Tool::Tolerance(E2);
  TolSum = Max( TolSum, 1.e-5 );

  TColgp_SequenceOfPnt   ResPoints;
  TColStd_SequenceOfReal ResParamsOnE1, ResParamsOnE2;
  gp_Pnt DegPoint;
  Standard_Boolean WithDegen = BRep_Tool::Degenerated(E1) || BRep_Tool::Degenerated(E2);
  
  if (WithDegen)
  {
    Standard_Integer ideg = (BRep_Tool::Degenerated(E1))? 1 : 2;
    TopoDS_Iterator iter( EI[ideg] );
    if (iter.More())
    {
      const TopoDS_Vertex& vdeg = TopoDS::Vertex(iter.Value());
      DegPoint = BRep_Tool::Pnt(vdeg);
    }
    else
    {
      BRepAdaptor_Curve CEdeg( EI[ideg], F );
      DegPoint = CEdeg.Value( CEdeg.FirstParameter() );
    }
  }
  //
  Handle(Geom2d_Curve) pcurve1 = BRep_Tool::CurveOnSurface(E1, F, f[1], l[1]);
  Handle(Geom2d_Curve) pcurve2 = BRep_Tool::CurveOnSurface(E2, F, f[2], l[2]);
  Geom2dAdaptor_Curve GAC1(pcurve1, f[1], l[1]);
  Geom2dAdaptor_Curve GAC2(pcurve2, f[2], l[2]);
  if ((GAC1.GetType() == GeomAbs_Line) &&
      (GAC2.GetType() == GeomAbs_Line))
  {
    // Just quickly check if lines coincide
    if (GAC1.Line().Direction().IsParallel (GAC2.Line().Direction(), 1.e-8))
    {
      theCoincide = Standard_True;
      return;
    }
  }

  Geom2dInt_GInter Inter2d( GAC1, GAC2, TolDub, TolDub );
  //
  if (!Inter2d.IsDone() || !Inter2d.NbPoints()) {
    theCoincide = (Inter2d.NbSegments() &&
                  (GAC1.GetType() == GeomAbs_Line) &&
                  (GAC2.GetType() == GeomAbs_Line));
    return;
  }
  //
  for (i = 1; i <= Inter2d.NbPoints(); i++)
  {
    gp_Pnt P3d;
    if (WithDegen)
      P3d = DegPoint;
    else
    {
      gp_Pnt2d P2d = Inter2d.Point(i).Value();
      P3d = BAsurf.Value( P2d.X(), P2d.Y() );
    }
    ResPoints.Append( P3d );
    ResParamsOnE1.Append( Inter2d.Point(i).ParamOnFirst() );
    ResParamsOnE2.Append( Inter2d.Point(i).ParamOnSecond() );
  }
  
  for (i = 1; i <= ResPoints.Length(); i++)
  {
    Standard_Real aT1 = ResParamsOnE1(i); //ponc1.Parameter();
    Standard_Real aT2 = ResParamsOnE2(i); //ponc2.Parameter();
    if (Precision::IsInfinite(aT1) || Precision::IsInfinite(aT2))
    {
#ifdef OCCT_DEBUG
      std::cout << "Inter2d : Solution rejected due to infinite parameter"<<std::endl;
#endif
      continue;
    }
    
    gp_Pnt P = ResPoints(i); //ponc1.Value();
    TopoDS_Vertex aNewVertex = BRepLib_MakeVertex(P);
    aNewVertex.Orientation(TopAbs_INTERNAL);
    B.UpdateVertex( aNewVertex, aT1, E1, Tol );
    B.UpdateVertex( aNewVertex, aT2, E2, Tol );
    aVerParOnE1map.Bind (aNewVertex, aT1);
    aVerParOnE2map.Bind (aNewVertex, aT2);
    gp_Pnt P1 = CE1.Value(aT1);
    gp_Pnt P2 = CE2.Value(aT2);
    Standard_Real dist1, dist2, dist3;
    dist1 = P1.Distance(P);
    dist2 = P2.Distance(P);
    dist3 = P1.Distance(P2);
    dist1 = Max( dist1, dist2 );
    dist1 = Max( dist1, dist3 );
    B.UpdateVertex( aNewVertex, dist1 );
    
#ifdef OCCT_DEBUG
    if (aT1 < f[1]-Tol  || aT1 > l[1]+Tol)
    {
      std::cout << "out of limit"<<std::endl;
      std::cout<<"aT1 = "<<aT1<<", f[1] = "<<f[1]<<", l[1] = "<<l[1]<<std::endl;
    }
    if (aT2 < f[2]-Tol  || aT2 > l[2]+Tol)
    {
      std::cout << "out of limit"<<std::endl;
      std::cout<<"aT2 = "<<aT2<<", f[2] = "<<f[2]<<", l[2] = "<<l[2]<<std::endl;
    }
    Standard_Real MilTol2 = 1000*Tol*Tol;
    if (P1.SquareDistance(P) >  MilTol2 || P2.SquareDistance(P) > MilTol2 || P1.Distance(P2) > 2.*Tol)
    {
      std::cout << "Inter2d : Solution rejected"<<std::endl;
      std::cout<<"P  = "<<P.X()<<" "<<P.Y()<<" "<<P.Z()<<std::endl;
      std::cout<<"P1 = "<<P1.X()<<" "<<P1.Y()<<" "<<P1.Z()<<std::endl;
      std::cout<<"P2 = "<<P2.X()<<" "<<P2.Y()<<" "<<P2.Z()<<std::endl;
      std::cout<<"MaxDist = "<<dist1<<std::endl;
    }
#endif
    //define the orientation of a new vertex
    TopAbs_Orientation OO1 = TopAbs_REVERSED;
    TopAbs_Orientation OO2 = TopAbs_REVERSED;
    if (WithOri)
    {
      BRepAdaptor_Curve2d PCE1( E1, F );
      BRepAdaptor_Curve2d PCE2( E2, F );
      gp_Pnt2d P2d1, P2d2;
      gp_Vec2d V1, V2, V1or, V2or;
      PCE1.D1( aT1, P2d1, V1 );
      PCE2.D1( aT2, P2d2, V2 );
      V1or = V1; V2or = V2;
      if (E1.Orientation() == TopAbs_REVERSED) V1or.Reverse();
      if (E2.Orientation() == TopAbs_REVERSED) V2or.Reverse();
      Standard_Real CrossProd = V2or ^ V1;
#ifdef OCCT_DEBUG
      if (Abs(CrossProd) <= gp::Resolution())
        std::cout<<std::endl<<"CrossProd = "<<CrossProd<<std::endl;
#endif
      if (CrossProd > 0.)
        OO1 = TopAbs_FORWARD;
      CrossProd = V1or ^ V2;
      if (CrossProd > 0.)
        OO2 = TopAbs_FORWARD;
    }

    if (theOr1 != TopAbs_EXTERNAL)
      OO1 = theOr1;
    if (theOr2 != TopAbs_EXTERNAL)
      OO2 = theOr2;
    
    LV1.Append( aNewVertex.Oriented(OO1) );
    LV2.Append( aNewVertex.Oriented(OO2) );
  }
  
  //----------------------------------
  // Test at end.
  //---------------------------------
  Standard_Real U1,U2;
  Standard_Real TolConf = Tol;
  TopoDS_Vertex V1[2],V2[2];
  TopExp::Vertices(E1,V1[0],V1[1]);
  TopExp::Vertices(E2,V2[0],V2[1]);

  Standard_Integer j;
  /*
  for (j = 0; j < 2; j++) {
    if (V1[j].IsNull()) continue;
    for (Standard_Integer k = 0; k < 2; k++) {
      if (V2[k].IsNull()) continue;
      if (V1[j].IsSame(V2[k])) {
        if (AsDes->HasAscendant(V1[j])) {
          continue;
        }
      }
      //
      gp_Pnt P1 = BRep_Tool::Pnt(V1[j]);
      gp_Pnt P2 = BRep_Tool::Pnt(V2[k]);
      Standard_Real Dist = P1.Distance(P2); 
      if (Dist < TolConf) {
        TopoDS_Vertex V = BRepLib_MakeVertex(P1);
        U1 = (j == 0) ? f[1] : l[1];
        U2 = (k == 0) ? f[2] : l[2];
        TopoDS_Shape aLocalShape = V.Oriented(TopAbs_INTERNAL);
        B.UpdateVertex(TopoDS::Vertex(aLocalShape),U1,E1,Tol);
        B.UpdateVertex(TopoDS::Vertex(aLocalShape),U2,E2,Tol);
        LV1.Prepend(V.Oriented(V1[j].Orientation()));
        LV2.Prepend(V.Oriented(V2[k].Orientation()));
      }
    }
  }
  */

  Standard_Boolean AffichPurge = Standard_False;

  if ( !LV1.IsEmpty()) {
    //----------------------------------
    // Remove all vertices.
    // there can be doubles
    //----------------------------------
    TopTools_ListIteratorOfListOfShape it1LV1,it1LV2,it2LV1;
    gp_Pnt P1,P2;
    Standard_Boolean Purge = Standard_True;

    while (Purge) {
      i = 1;
      Purge = Standard_False;
      for (it1LV1.Initialize(LV1),it1LV2.Initialize(LV2); 
           it1LV1.More(); it1LV1.Next(),it1LV2.Next()) {
        j = 1;
        it2LV1.Initialize(LV1);
        while (j < i) {      
          P1 = BRep_Tool::Pnt(TopoDS::Vertex(it1LV1.Value()));
          P2 = BRep_Tool::Pnt(TopoDS::Vertex(it2LV1.Value()));
          if (P1.IsEqual(P2, Tol)) {
            LV1.Remove(it1LV1);
            LV2.Remove(it1LV2);
            if (AffichPurge) std::cout <<"Doubles removed in EdgeInter."<<std::endl;
            Purge = Standard_True;
            break;
          }
          j++;
          it2LV1.Next();
        }
        if (Purge) break;
        i++;
      }
    }
    //---------------------------------
    // Vertex storage in SD.
    //---------------------------------
////-----------------------------------------------------
    if(LV1.Extent() > 1) {
      //std::cout << "IFV - RefEdgeInter: remove vertex" << std::endl;
      gp_Pnt Pref = BRep_Tool::Pnt(theVref);
      Standard_Real dmin = RealLast();
      TopoDS_Vertex Vmin;
      for (it1LV1.Initialize(LV1); it1LV1.More(); it1LV1.Next()) {
        gp_Pnt P = BRep_Tool::Pnt(TopoDS::Vertex(it1LV1.Value()));
        Standard_Real d = P.SquareDistance(Pref);
        if(d < dmin) {
          dmin = d;
          Vmin = TopoDS::Vertex(it1LV1.Value());
        }
      }
      for (it1LV1.Initialize(LV1),it1LV2.Initialize(LV2); 
           it1LV1.More(); it1LV1.Next(),it1LV2.Next()) {
        if(!Vmin.IsSame(it1LV1.Value())) {
          LV1.Remove(it1LV1);
          LV2.Remove(it1LV2);
          if(!it1LV1.More()) break;
        }
      }
    }

    TopTools_ListIteratorOfListOfShape itl (LV1);
    for (; itl.More(); itl.Next())
    {
      TopoDS_Shape aNewVertex = itl.Value();
      aNewVertex.Orientation(TopAbs_FORWARD);
      if (theImageVV.HasImage (theVref))
        theImageVV.Add (theVref.Oriented(TopAbs_FORWARD), aNewVertex);
      else
        theImageVV.Bind (theVref.Oriented(TopAbs_FORWARD), aNewVertex);
    }
      
////-----------------------------------------------------
    Standard_Real TolStore = BRep_Tool::Tolerance(E1) + BRep_Tool::Tolerance(E2);
    TolStore = Max (TolStore, Tol);
    Store (E1,E2,LV1,LV2,TolStore,AsDes, aDMVV);
    
    theVertexForE1 = TopoDS::Vertex(LV1.First());
    theVertexForE2 = TopoDS::Vertex(LV2.First());
    theParamForE1 = aVerParOnE1map(theVertexForE1);
    theParamForE2 = aVerParOnE2map(theVertexForE2);
  }
}


//======================================================================
//function : EvaluateMaxSegment
//purpose  : return MaxSegment to pass in approximation
//======================================================================

static Standard_Integer evaluateMaxSegment(const Adaptor3d_CurveOnSurface& aCurveOnSurface)
{
  Handle(Adaptor3d_Surface) aSurf   = aCurveOnSurface.GetSurface();
  Handle(Adaptor2d_Curve2d) aCurv2d = aCurveOnSurface.GetCurve();

  Standard_Real aNbSKnots = 0, aNbC2dKnots = 0;
  
  if (aSurf->GetType() == GeomAbs_BSplineSurface) {
    Handle(Geom_BSplineSurface) aBSpline = aSurf->BSpline();
    aNbSKnots = Max(aBSpline->NbUKnots(), aBSpline->NbVKnots());
  }
  if (aCurv2d->GetType() == GeomAbs_BSplineCurve) {
    aNbC2dKnots = aCurv2d->NbKnots();
  }
  Standard_Integer aReturn = (Standard_Integer) (  30 + Max(aNbSKnots, aNbC2dKnots) ) ;
  return aReturn;
}


//=======================================================================
//function : ExtendPCurve
//purpose  : 
//=======================================================================

static Standard_Boolean ExtendPCurve(const Handle(Geom2d_Curve)& aPCurve,
                                     const Standard_Real anEf,
                                     const Standard_Real anEl,
                                     const Standard_Real a2Offset,
                                     Handle(Geom2d_Curve)& NewPCurve)
{
  NewPCurve = aPCurve;
  if (NewPCurve->IsInstance(STANDARD_TYPE(Geom2d_TrimmedCurve)))
    NewPCurve = Handle(Geom2d_TrimmedCurve)::DownCast (NewPCurve)->BasisCurve();
  
  Standard_Real FirstPar = NewPCurve->FirstParameter();
  Standard_Real LastPar  = NewPCurve->LastParameter();

  /*
  if (NewPCurve->IsKind(STANDARD_TYPE(Geom2d_BoundedCurve)) &&
      (FirstPar > anEf - a2Offset || LastPar < anEl + a2Offset))
  {
    if (NewPCurve->IsInstance(STANDARD_TYPE(Geom2d_BezierCurve)))
    {
      Handle(Geom2d_BezierCurve) aBezier = Handle(Geom2d_BezierCurve)::DownCast (NewPCurve);
      if (aBezier->NbPoles() == 2)
      {
        TColgp_Array1OfPnt2d thePoles(1,2);
        aBezier->Poles(thePoles);
        gp_Vec2d aVec(thePoles(1), thePoles(2));
        NewPCurve = new Geom2d_Line(thePoles(1), aVec);
        return Standard_True;
      }
    }
    else if (NewPCurve->IsInstance(STANDARD_TYPE(Geom2d_BSplineCurve)))
    {
      Handle(Geom2d_BSplineCurve) aBSpline = Handle(Geom2d_BSplineCurve)::DownCast (NewPCurve);
      if (aBSpline->NbKnots() == 2 && aBSpline->NbPoles() == 2)
      {
        TColgp_Array1OfPnt2d thePoles(1,2);
        aBSpline->Poles(thePoles);
        gp_Vec2d aVec(thePoles(1), thePoles(2));
        NewPCurve = new Geom2d_Line(thePoles(1), aVec);
        return Standard_True;
      }
    }
  }
  */
  
  FirstPar = aPCurve->FirstParameter();
  LastPar  = aPCurve->LastParameter();
  Handle(Geom2d_TrimmedCurve) aTrCurve = 
    new Geom2d_TrimmedCurve(aPCurve, FirstPar, LastPar);
  
  // The curve is not prolonged on begin or end.
  // Trying to prolong it adding a segment to its bound.
  gp_Pnt2d                              aPBnd;
  gp_Vec2d                              aVBnd;
  gp_Pnt2d                              aPBeg;
  gp_Dir2d                              aDBnd;
  Handle(Geom2d_Line)                   aLin;
  Handle(Geom2d_TrimmedCurve)           aSegment;
  Geom2dConvert_CompCurveToBSplineCurve aCompCurve(aTrCurve, Convert_RationalC1);
  Standard_Real                         aTol = Precision::Confusion();
  //Standard_Real                         aDelta = Max(a2Offset, 1.);
  Standard_Real                         aDelta = a2Offset;
  
  if (FirstPar > anEf - a2Offset) {
    aPCurve->D1(FirstPar, aPBnd, aVBnd);
    aDBnd.SetXY(aVBnd.XY());
    aPBeg    = aPBnd.Translated(gp_Vec2d(-aDelta*aDBnd.XY()));
    aLin     = new Geom2d_Line(aPBeg, aDBnd);
    aSegment = new Geom2d_TrimmedCurve(aLin, 0, aDelta);
    
    if (!aCompCurve.Add(aSegment, aTol))
      return Standard_False;
  }
  
  if (LastPar < anEl + a2Offset) {
    aPCurve->D1(LastPar, aPBeg, aVBnd);
    aDBnd.SetXY(aVBnd.XY());
    aLin     = new Geom2d_Line(aPBeg, aDBnd);
    aSegment = new Geom2d_TrimmedCurve(aLin, 0, aDelta);
    
    if (!aCompCurve.Add(aSegment, aTol))
      return Standard_False;
  }
  
  NewPCurve  = aCompCurve.BSplineCurve();
  return Standard_True;
}

//=======================================================================
//function : ExtentEdge
//purpose  : 
//=======================================================================

Standard_Boolean BRepOffset_Inter2d::ExtentEdge(const TopoDS_Edge&     E,
                                                TopoDS_Edge&           NE,
                                                const Standard_Real    theOffset,
                                                const Standard_Boolean theToProlongBefore,
                                                const Standard_Boolean theToProlongAfter)
{
  const Standard_Integer NCONTROL = 23;
  Standard_Real aTolEdge = BRep_Tool::Tolerance(E);

  TopoDS_Shape  aLocalShape = E.EmptyCopied();
  Standard_Real anEf;
  Standard_Real anEl;
  Standard_Real a2Offset = 2.*Abs(theOffset);
  BRep_Builder BB;

  BRep_Tool::Range(E, anEf, anEl);
  NE = TopoDS::Edge(aLocalShape);

  Standard_Integer NbPCurves = 0;
  Standard_Boolean IsSeam = Standard_False;

  BRep_ListIteratorOfListOfCurveRepresentation itr( (Handle(BRep_TEdge)::DownCast(NE.TShape()))->ChangeCurves() );
  for (; itr.More(); itr.Next())
  {
    Handle( BRep_CurveRepresentation ) CurveRep = itr.Value();
    if (CurveRep->IsCurveOnSurface())
    {
      NbPCurves++;
      if (CurveRep->IsCurveOnClosedSurface())
        IsSeam = Standard_True;
    }
  }

  if (IsSeam)
  {
    Standard_Real fpar, lpar;
    Handle(Geom_Curve) C3d = BRep_Tool::Curve (NE, fpar, lpar);

    if (!C3d.IsNull())
    {
      if (C3d->IsKind(STANDARD_TYPE(Geom_TrimmedCurve)))
        C3d = (Handle(Geom_TrimmedCurve)::DownCast(C3d))->BasisCurve();
      
      Standard_Real FirstPar = C3d->FirstParameter();
      Standard_Real LastPar  = C3d->LastParameter();
      
      if (Precision::IsInfinite(FirstPar) && Precision::IsInfinite(LastPar))
      {
        BB.Range (NE, FirstPar, LastPar);
        return Standard_True;
      }
    }
    
    Handle(Geom2d_Curve) aPCurve, aPCurve2;
    Handle(Geom_Surface) aSurf;
    TopLoc_Location aLoc;
    BRep_Tool::CurveOnSurface(E, aPCurve,  aSurf, aLoc, fpar, lpar, 1);
    BRep_Tool::CurveOnSurface(E, aPCurve2, aSurf, aLoc, fpar, lpar, 2);
    if (aPCurve->IsKind(STANDARD_TYPE(Geom2d_TrimmedCurve)))
      aPCurve = (Handle(Geom2d_TrimmedCurve)::DownCast(aPCurve))->BasisCurve();
    if (aPCurve2->IsKind(STANDARD_TYPE(Geom2d_TrimmedCurve)))
      aPCurve2 = (Handle(Geom2d_TrimmedCurve)::DownCast(aPCurve2))->BasisCurve();

    Standard_Real FirstPar = aPCurve->FirstParameter();
    Standard_Real LastPar  = aPCurve->LastParameter();
    Standard_Real FirstPar2 = aPCurve2->FirstParameter();
    Standard_Real LastPar2  = aPCurve2->LastParameter();

    Standard_Real aFirstPar = Min(FirstPar, FirstPar2);
    Standard_Real aLastPar  = Max(LastPar, LastPar2);

    gp_Pnt2d aPnt2d;
    gp_Vec2d aTangent;
    aPCurve->D1(fpar, aPnt2d, aTangent);
    gp_Dir2d aTangDir = aTangent;
    Standard_Real aCrossProd1 = aTangent ^ gp::DX2d(),
      aCrossProd2 = aTangent ^ gp::DY2d();
    GeomAdaptor_Surface aGAsurf(aSurf);
    Standard_Boolean IsParallelDX = (Abs(aCrossProd1) < Abs(aCrossProd2));
    Standard_Real anOffset2d = IsParallelDX ?
      aGAsurf.UResolution(a2Offset) : aGAsurf.VResolution(a2Offset);

    Standard_Boolean IsFirstPCurveBounded =
      (aPCurve->IsKind(STANDARD_TYPE(Geom2d_BoundedCurve)) &&
       (FirstPar > anEf - anOffset2d || LastPar < anEl + anOffset2d));
    
    Standard_Boolean IsSecondPCurveBounded =
      (aPCurve2->IsKind(STANDARD_TYPE(Geom2d_BoundedCurve)) &&
       (FirstPar2 > anEf - anOffset2d || LastPar2 < anEl + anOffset2d));

    if (IsFirstPCurveBounded && IsSecondPCurveBounded)
    {
      gp_Pnt2d aPnt2d2 = aPCurve2->Value(fpar);
      gp_Vec2d aTranslation (aPnt2d, aPnt2d2);
      ExtendPCurve (aPCurve, anEf, anEl, FirstPar, LastPar, anOffset2d);
      aPCurve2 = Handle(Geom2d_Curve)::DownCast(aPCurve->Copy());
      aPCurve2->Translate (aTranslation);
      BB.UpdateEdge (NE, aPCurve, aPCurve2, aSurf, aLoc, Precision::Confusion());
      aFirstPar = aPCurve->FirstParameter();
      aLastPar  = aPCurve->LastParameter();
    }

    if (Precision::IsInfinite(aFirstPar) || Precision::IsInfinite(aLastPar))
    {
      Standard_Real aUmin, aUmax, aVmin, aVmax;
      aSurf->Bounds(aUmin, aUmax, aVmin, aVmax);
      gp_Pnt2d FirstOrigin(aUmin, aVmin);
      gp_Pnt2d LastOrigin = IsParallelDX ? gp_Pnt2d(aUmax, aVmin) : gp_Pnt2d(aUmin, aVmax);
      gp_Dir2d aDir = IsParallelDX ? gp::DY2d() : gp::DX2d();

      Handle(Geom2d_Line) BoundLine [2];
      BoundLine[0] = new Geom2d_Line(FirstOrigin, aDir);
      BoundLine[1] = new Geom2d_Line(LastOrigin, aDir);

      Geom2dAdaptor_Curve aGAcurve(aPCurve, aFirstPar, aLastPar);
      Standard_Real NewPar [2];
      NewPar[0] = Precision::Infinite();
      NewPar[1] = Precision::Infinite();
      for (Standard_Integer ii = 0; ii < 2; ii++)
      {
        Geom2dAdaptor_Curve aGAbound (BoundLine[ii]);
        Geom2dInt_GInter anInter2d (aGAcurve, aGAbound,
                                    Precision::Confusion(), Precision::Confusion());
        if (anInter2d.IsDone() && anInter2d.NbPoints() > 0)
          NewPar[ii] = anInter2d.Point(1).ParamOnFirst();
      }
      //Temporary
      aFirstPar = Min(NewPar[0], NewPar[1]);
      aLastPar  = Max(NewPar[0], NewPar[1]);
      ///////////

      Standard_Real aDelta = 0.;
      if (IsParallelDX && aSurf->IsUClosed())
        aDelta = ((aUmax - aUmin) - (anEl - anEf))*0.5;
      else if (!IsParallelDX && aSurf->IsVClosed())
        aDelta = ((aVmax - aVmin) - (anEl - anEf))*0.5;
      if (aDelta != 0.)
      {
        aDelta *= 0.95;
        aFirstPar = anEf - aDelta;
        aLastPar  = anEl + aDelta;
      }
    }
    
    BB.Range (NE, aFirstPar, aLastPar);

    return Standard_True;
  } //if (IsSeam)

  if (BRep_Tool::Degenerated(E))
  {
    Handle(Geom2d_Curve) aPCurve;
    Handle(Geom_Surface) aSurf;
    TopLoc_Location aLoc;
    Standard_Real fpar, lpar;
    BRep_Tool::CurveOnSurface(E, aPCurve, aSurf, aLoc, fpar, lpar, 1);
    if (aPCurve->IsKind(STANDARD_TYPE(Geom2d_TrimmedCurve)))
      aPCurve = (Handle(Geom2d_TrimmedCurve)::DownCast(aPCurve))->BasisCurve();

    Standard_Real FirstPar = aPCurve->FirstParameter();
    Standard_Real LastPar  = aPCurve->LastParameter();

    gp_Pnt2d aPnt2d;
    gp_Vec2d aTangent;
    aPCurve->D1(fpar, aPnt2d, aTangent);
    gp_Dir2d aTangDir = aTangent;
    Standard_Real aCrossProd1 = aTangent ^ gp::DX2d(),
      aCrossProd2 = aTangent ^ gp::DY2d();
    Standard_Boolean IsParallelDX = (Abs(aCrossProd1) < Abs(aCrossProd2));
    GeomAdaptor_Surface aGAsurf(aSurf);
    Standard_Real anOffset2d = IsParallelDX ?
      aGAsurf.UResolution(a2Offset) : aGAsurf.VResolution(a2Offset);
  
    if (aPCurve->IsKind(STANDARD_TYPE(Geom2d_BoundedCurve)) &&
        (FirstPar > anEf - anOffset2d || LastPar < anEl + anOffset2d))
    {
      ExtendPCurve(aPCurve, anEf, anEl, FirstPar, LastPar, anOffset2d);
      FirstPar = aPCurve->FirstParameter();
      LastPar  = aPCurve->LastParameter();
      BB.UpdateEdge (NE, aPCurve, aSurf, aLoc, Precision::Confusion());
    }
    
    if (Precision::IsInfinite(FirstPar) || Precision::IsInfinite(LastPar))
    {
      Standard_Real aUmin, aUmax, aVmin, aVmax;
      aSurf->Bounds(aUmin, aUmax, aVmin, aVmax);
      gp_Pnt2d FirstOrigin(aUmin, aVmin);
      gp_Pnt2d LastOrigin = IsParallelDX ? gp_Pnt2d(aUmax, aVmin) : gp_Pnt2d(aUmin, aVmax);
      gp_Dir2d aDir = IsParallelDX ? gp::DY2d() : gp::DX2d();

      Handle(Geom2d_Line) BoundLine [2];
      BoundLine[0] = new Geom2d_Line(FirstOrigin, aDir);
      BoundLine[1] = new Geom2d_Line(LastOrigin, aDir);

      Geom2dAdaptor_Curve aGAcurve(aPCurve, FirstPar, LastPar);
      Standard_Real NewPar [2];
      NewPar[0] = Precision::Infinite();
      NewPar[1] = Precision::Infinite();
      for (Standard_Integer ii = 0; ii < 2; ii++)
      {
        Geom2dAdaptor_Curve aGAbound (BoundLine[ii]);
        Geom2dInt_GInter anInter2d (aGAcurve, aGAbound,
                                    Precision::Confusion(), Precision::Confusion());
        if (anInter2d.IsDone() && anInter2d.NbPoints() > 0)
          NewPar[ii] = anInter2d.Point(1).ParamOnFirst();
      }
      //Temporary
      FirstPar = Min(NewPar[0], NewPar[1]);
      LastPar  = Max(NewPar[0], NewPar[1]);
      ///////////
    }
    BB.Range (NE, FirstPar, LastPar);

    return Standard_True;
  } //if (BRep_Tool::Degenerated(E))
  
  Standard_Real fpar, lpar;
  Handle(Geom_Curve) C3d = BRep_Tool::Curve( NE, fpar, lpar );

  Handle(Geom2d_Curve) NullPCurve;

  Standard_Boolean aToProlongBefore = theToProlongBefore,
    aToProlongAfter = theToProlongAfter;
  Standard_Real anExtensionBefore = a2Offset, anExtensionAfter = a2Offset;
  Standard_Integer anIndPCurve = 0;
  for (;;)
  {
    anIndPCurve++;
    Handle(Geom2d_Curve) aPCurve;
    Handle(Geom_Surface) aSurf;
    TopLoc_Location aLoc;
    //Standard_Real fpar, lpar;
    BRep_Tool::CurveOnSurface(E, aPCurve, aSurf, aLoc, fpar, lpar, anIndPCurve);
    if (aPCurve.IsNull())
      break;

    Handle(Geom2d_TrimmedCurve) aTrCurve = new Geom2d_TrimmedCurve(aPCurve, fpar, lpar);
    //Geom2dConvert_CompCurveToBSplineCurve aCompCurve(aTrCurve, Convert_RationalC1);
    //Standard_Real aTol = Precision::Confusion();
    
    gp_Pnt2d aP2d;
    gp_Vec2d aVec2d;
    gp_Dir2d aTangent;
    Handle(Geom2d_TrimmedCurve) aSegment;
    if (aToProlongBefore)
    {
      aPCurve->D1 (fpar, aP2d, aVec2d);
      aVec2d.Reverse();
      aTangent = aVec2d;
      Standard_Real aLenToPnt = -1, aParamOnLine;
      Standard_Boolean IsFitted = ComputeEndOfExtension(aSurf, aP2d, aTangent, a2Offset,
                                                        aParamOnLine, aLenToPnt, aSegment);
      if (aLenToPnt == 0.)
        aToProlongBefore = Standard_False;
      
      else if (aLenToPnt != -1 && !IsFitted &&
               aLenToPnt < anExtensionBefore)
        anExtensionBefore = aLenToPnt;
      //aCompCurve.Add(aSegment, aTol);
    }
    if (aToProlongAfter)
    {
      aPCurve->D1 (lpar, aP2d, aVec2d);
      aTangent = aVec2d;
      Standard_Real aLenToPnt = -1, aParamOnLine;
      Standard_Boolean IsFitted = ComputeEndOfExtension(aSurf, aP2d, aTangent, a2Offset,
                                                        aParamOnLine, aLenToPnt, aSegment);
      if (aLenToPnt == 0.)
        aToProlongAfter = Standard_False;
      
      else if (aLenToPnt != -1 && !IsFitted &&
               aLenToPnt < anExtensionAfter)
        anExtensionAfter = aLenToPnt;
      //aCompCurve.Add(aSegment, aTol);
    }
    /*
    Handle(Geom2d_Curve) aNewPCurve = aCompCurve.BSplineCurve();
    fpar = aNewPCurve->FirstParameter();
    lpar = aNewPCurve->LastParameter();
    
    Handle(TColgp_HArray1OfPnt)   aPntArray = new TColgp_HArray1OfPnt(1, NCONTROL);
    Handle(TColStd_HArray1OfReal) aParArray = new TColStd_HArray1OfReal(1, NCONTROL);
    Standard_Real aDelta = (lpar - fpar)/(NCONTROL-1);
    for (Standard_Integer ii = 0; ii < NCONTROL; ii++)
    {
      Standard_Real aPar = fpar + ii*aDelta;
      aParArray->SetValue(ii+1, aPar);
      gp_Pnt2d aPnt2d = aNewPCurve->Value(aPar);
      gp_Pnt aPnt = aSurf->Value(aPnt2d.X(), aPnt2d.Y());
      if (!aLoc.IsIdentity())
        aPnt.Transform(aLoc.Transformation());
      aPntArray->SetValue(ii+1, aPnt);
    }
    GeomAPI_Interpolate anInterp(aPntArray, aParArray, Standard_False, Precision::Confusion());
    anInterp.Perform();
    C3d = anInterp.Curve();

    BB.UpdateEdge (NE, C3d, aTolEdge);
    BB.UpdateEdge (NE, aNewPCurve, aSurf, aLoc, aTolEdge);
    BB.Range (NE, fpar, lpar);
    return Standard_True;
    */
  } //for (;;)
  
  if (C3d.IsNull())
  {
    Handle(Geom2d_Curve) aPCurve;
    Handle(Geom_Surface) aSurf;
    TopLoc_Location aLoc;
    //Standard_Real fpar, lpar;
    BRep_Tool::CurveOnSurface(E, aPCurve, aSurf, aLoc, fpar, lpar, 1);

    Handle(TColgp_HArray1OfPnt)   aPntArray = new TColgp_HArray1OfPnt(1, NCONTROL);
    Handle(TColStd_HArray1OfReal) aParArray = new TColStd_HArray1OfReal(1, NCONTROL);
    Standard_Real aDelta = (lpar - fpar)/(NCONTROL-1);
    for (Standard_Integer ii = 0; ii < NCONTROL; ii++)
    {
      Standard_Real aPar = fpar + ii*aDelta;
      aParArray->SetValue(ii+1, aPar);
      gp_Pnt2d aPnt2d = aPCurve->Value(aPar);
      gp_Pnt aPnt = aSurf->Value(aPnt2d.X(), aPnt2d.Y());
      if (!aLoc.IsIdentity())
        aPnt.Transform(aLoc.Transformation());
      aPntArray->SetValue(ii+1, aPnt);
    }
    GeomAPI_Interpolate anInterp(aPntArray, aParArray, Standard_False, Precision::Confusion());
    anInterp.Perform();
    C3d = anInterp.Curve();
    /*
    Geom2dAdaptor_Curve AC2d (aPCurve, fpar, lpar);
    GeomAdaptor_Surface GAsurf (aSurf);
    Handle(Geom2dAdaptor_HCurve) HC2d  = new Geom2dAdaptor_HCurve (AC2d);
    Handle(GeomAdaptor_HSurface) HSurf = new GeomAdaptor_HSurface (GAsurf);
    Adaptor3d_CurveOnSurface ConS (HC2d, HSurf);
    Standard_Real max_deviation, average_deviation;
    GeomAbs_Shape Continuity = GeomAbs_C1;
    Standard_Integer MaxDegree = 14;
    Standard_Integer MaxSegment = evaluateMaxSegment(ConS);
    GeomLib::BuildCurve3d(Precision::Confusion(),
                          ConS, fpar, lpar,
                          C3d, max_deviation, average_deviation,
                          Continuity, MaxDegree, MaxSegment);
    BB.UpdateEdge (NE, C3d, max_deviation);
    */
    BB.UpdateEdge (NE, C3d, aTolEdge);
  }

  if (C3d->IsKind(STANDARD_TYPE(Geom_TrimmedCurve)))
    C3d = (Handle(Geom_TrimmedCurve)::DownCast(C3d))->BasisCurve();
  
  Standard_Real FirstPar = C3d->FirstParameter();
  Standard_Real LastPar  = C3d->LastParameter();
  
  if (C3d->IsKind(STANDARD_TYPE(Geom_BoundedCurve)) &&
      (FirstPar > anEf - a2Offset || LastPar < anEl + a2Offset))
  {
    Handle(Geom_TrimmedCurve) aTrCurve = 
      new Geom_TrimmedCurve(C3d, FirstPar, LastPar);
    
    // The curve is not prolonged on begin or end.
    // Trying to prolong it adding a segment to its bound.
    gp_Pnt                              aPBnd;
    gp_Vec                              aVBnd;
    gp_Pnt                              aPBeg;
    gp_Dir                              aDBnd;
    Handle(Geom_Line)                   aLin;
    Handle(Geom_TrimmedCurve)           aSegment;
    GeomConvert_CompCurveToBSplineCurve aCompCurve(aTrCurve, Convert_RationalC1);
    Standard_Real                       aTol = Precision::Confusion();
    //Standard_Real                       aDelta = Max(a2Offset, 1.);
    //Standard_Real                       aDelta = a2Offset;
    
    if (aToProlongBefore &&
        FirstPar > anEf - a2Offset)
    {
      Standard_Real aDelta = anExtensionBefore;
      C3d->D1(FirstPar, aPBnd, aVBnd);
      aDBnd.SetXYZ(aVBnd.XYZ());
      aPBeg    = aPBnd.Translated(gp_Vec(-aDelta*aDBnd.XYZ()));
      aLin     = new Geom_Line(aPBeg, aDBnd);
      aSegment = new Geom_TrimmedCurve(aLin, 0, aDelta);
      
      if (!aCompCurve.Add(aSegment, aTol))
        return Standard_True;
    }
    
    if (aToProlongAfter &&
        LastPar < anEl + a2Offset)
    {
      Standard_Real aDelta = anExtensionAfter;
      C3d->D1(LastPar, aPBeg, aVBnd);
      aDBnd.SetXYZ(aVBnd.XYZ());
      aLin     = new Geom_Line(aPBeg, aDBnd);
      aSegment = new Geom_TrimmedCurve(aLin, 0, aDelta);
      
      if (!aCompCurve.Add(aSegment, aTol))
        return Standard_True;
    }
    
    C3d = aCompCurve.BSplineCurve();
    FirstPar = C3d->FirstParameter();
    LastPar  = C3d->LastParameter();
    BB.UpdateEdge(NE, C3d, Precision::Confusion());
  }
  else if (C3d->IsPeriodic())
  {
    Standard_Real delta = (C3d->Period() - (anEl - anEf))*0.5;
    delta *= 0.95;
    FirstPar = anEf - delta;
    LastPar  = anEl + delta;
  }
  else if (C3d->IsClosed())
    LastPar -= 0.05*(LastPar - FirstPar);
  
  BB.Range (NE, FirstPar, LastPar);
  if (Precision::IsInfinite(FirstPar) && Precision::IsInfinite(LastPar))
    return Standard_True;
  
  for (Standard_Integer ii = 1; ii <= NbPCurves; ii++)
  {
    Handle(Geom2d_Curve) aPCurve;
    Handle(Geom_Surface) aSurf;
    TopLoc_Location aLoc;
    //Standard_Real fpar, lpar;
    BRep_Tool::CurveOnSurface(E, aPCurve, aSurf, aLoc, fpar, lpar, ii);
    GeomAdaptor_Surface aGAsurf(aSurf);
    if (aGAsurf.GetType() == GeomAbs_Plane)
      BB.UpdateEdge (NE, NullPCurve, aSurf, aLoc, 0.);
    else
    {
      ShapeConstruct_ProjectCurveOnSurface aToolProj;
      aToolProj.Init(aSurf, aTolEdge);
      Handle(Geom2d_Curve) aNewPCurve;
      aToolProj.Perform(C3d, FirstPar, LastPar, aNewPCurve);
      BB.UpdateEdge (NE, aNewPCurve, aSurf, aLoc, 0.);
    }
  }
  
  return Standard_True;
}

//=======================================================================
//function : UpdateVertex
//purpose  : 
//=======================================================================

static Standard_Boolean  UpdateVertex(TopoDS_Vertex V,
                                      TopoDS_Edge&  OE,
                                      TopoDS_Edge&  NE,
                                      Standard_Real TolConf)
{
  BRepAdaptor_Curve OC(OE);
  BRepAdaptor_Curve NC(NE);
  Standard_Real Of = OC.FirstParameter(); Standard_Real Ol = OC.LastParameter();
  Standard_Real Nf = NC.FirstParameter(); Standard_Real Nl = NC.LastParameter();
  Standard_Real U = 0.;
  Standard_Real ParTol = Precision::PConfusion();
  gp_Pnt           P  = BRep_Tool::Pnt(V);
  Standard_Boolean OK = Standard_False;

  if (P.Distance(OC.Value(Of)) < TolConf) {
    if (Of >= Nf + ParTol && Of <= Nl + ParTol  && P.Distance(NC.Value(Of)) < TolConf) {
      OK = Standard_True;
      U    = Of;
    }
  }
  if (P.Distance(OC.Value(Ol)) < TolConf) {
    if (Ol >= Nf + ParTol && Ol <= Nl + ParTol  && P.Distance(NC.Value(Ol)) < TolConf) {
      OK = Standard_True;
      U    = Ol;
    }
  }
  if (OK) {
    BRep_Builder B;
    TopoDS_Shape aLocalShape = NE.Oriented(TopAbs_FORWARD);
    TopoDS_Edge EE = TopoDS::Edge(aLocalShape);
//    TopoDS_Edge EE = TopoDS::Edge(NE.Oriented(TopAbs_FORWARD));
    aLocalShape = V.Oriented(TopAbs_INTERNAL);
    B.UpdateVertex(TopoDS::Vertex(aLocalShape),
                   U,NE,BRep_Tool::Tolerance(NE));
//    B.UpdateVertex(TopoDS::Vertex(V.Oriented(TopAbs_INTERNAL)),
//                   U,NE,BRep_Tool::Tolerance(NE));
  }
  return OK;  
}

//=======================================================================
//function : Compute
//purpose  : 
//=======================================================================

void BRepOffset_Inter2d::Compute (const Handle(BRepAlgo_AsDes)&     AsDes,
                                  const TopoDS_Face&                F,
                                  const TopTools_IndexedMapOfShape& NewEdges,
                                  const Standard_Real               Tol,
                                  const TopTools_DataMapOfShapeListOfShape& theEdgeIntEdges,
                                  TopTools_IndexedDataMapOfShapeListOfShape& theDMVV)
{
#ifdef DRAW
  NbF2d++;
  NbE2d = 0;
#endif 

  //Do not intersect the edges of face
  TopTools_MapOfShape EdgesOfFace;
  TopExp_Explorer Explo( F, TopAbs_EDGE );
  for (; Explo.More(); Explo.Next())
    EdgesOfFace.Add( Explo.Current() );

  //-----------------------------------------------------------
  // calculate intersections2d on faces touched by  
  // intersection3d
  //---------------------------------------------------------
  TopTools_ListIteratorOfListOfShape it1LE ;    
  TopTools_ListIteratorOfListOfShape it2LE ;  

  //-----------------------------------------------
  // Intersection of edges 2*2.
  //-----------------------------------------------
  TopTools_ListOfShape& LE = AsDes->ChangeDescendant(F);
  it1LE.Initialize(LE);
  while (it1LE.More())
  {
    const TopoDS_Edge& anEdge = TopoDS::Edge(it1LE.Value());
    Standard_Real aFirst, aLast;
    Handle(Geom2d_Curve) aPCurve = BRep_Tool::CurveOnSurface(anEdge, F, aFirst, aLast);
    if (aPCurve.IsNull())
      LE.Remove(it1LE);
    else
      it1LE.Next();
  }
  
  TopoDS_Vertex                      V1,V2;
  Standard_Integer                   j, i = 1;
  BRepAdaptor_Surface BAsurf(F);
  //
  for ( it1LE.Initialize(LE) ; it1LE.More(); it1LE.Next()) {
    const TopoDS_Edge& E1 = TopoDS::Edge(it1LE.Value());        
    j = 1;
    it2LE.Initialize(LE);
    
    while (j < i && it2LE.More()) {
      const TopoDS_Edge& E2 = TopoDS::Edge(it2LE.Value());

      Standard_Boolean ToIntersect = Standard_True;
      if (theEdgeIntEdges.IsBound(E1))
      {
        const TopTools_ListOfShape& aElist = theEdgeIntEdges(E1);
        TopTools_ListIteratorOfListOfShape itedges (aElist);
        for (; itedges.More(); itedges.Next())
          if (E2.IsSame (itedges.Value()))
            ToIntersect = Standard_False;
        
        if (ToIntersect)
        {
          for (itedges.Initialize(aElist); itedges.More(); itedges.Next())
          {
            const TopoDS_Shape& anEdge = itedges.Value();
            if (theEdgeIntEdges.IsBound(anEdge))
            {
              const TopTools_ListOfShape& aElist2 = theEdgeIntEdges(anEdge);
              TopTools_ListIteratorOfListOfShape itedges2 (aElist2);
              for (; itedges2.More(); itedges2.Next())
                if (E2.IsSame (itedges2.Value()))
                  ToIntersect = Standard_False;
            }
          }
        }
      }
      
      //--------------------------------------------------------------
      // Intersections of New edges obtained by intersection
      // between them and with edges of restrictions
      //------------------------------------------------------
      if (ToIntersect &&
          (!EdgesOfFace.Contains(E1) || !EdgesOfFace.Contains(E2)) &&
          (NewEdges.Contains(E1) || NewEdges.Contains(E2)) ) {
        
        TopoDS_Shape aLocalShape = F.Oriented(TopAbs_FORWARD);
        EdgeInter(TopoDS::Face(aLocalShape),BAsurf,E1,E2,AsDes,Tol,Standard_True, theDMVV);
//          EdgeInter(TopoDS::Face(F.Oriented(TopAbs_FORWARD)),E1,E2,AsDes,Tol,Standard_True);
      }
      it2LE.Next();
      j++;
    }
    i++;
  }
}

//=======================================================================
//function : ConnexIntByInt
//purpose  : 
//=======================================================================
Standard_Boolean BRepOffset_Inter2d::ConnexIntByInt
 (const TopoDS_Face&            FI,
  BRepOffset_Offset&            OFI,
  TopTools_DataMapOfShapeShape& MES,
  const TopTools_DataMapOfShapeShape& Build,
  const Handle(BRepAlgo_AsDes)& theAsDes,
  const Handle(BRepAlgo_AsDes)& AsDes2d,
  const Standard_Real           Offset,
  const Standard_Real           Tol,
  const BRepOffset_Analyse&     Analyse,
  TopTools_IndexedMapOfShape&   FacesWithVerts,
  BRepAlgo_Image&               theImageVV,
  TopTools_DataMapOfShapeListOfShape& theEdgeIntEdges,
  TopTools_IndexedDataMapOfShapeListOfShape& theDMVV,
  BRepOffset_DataMapOfFaceMapEE& theFaceEdgeEdge,
  TopTools_IndexedDataMapOfShapeListOfShape& theEFmap)
{  
  //TopTools_DataMapOfShapeListOfShape MVE;
  //BRepOffset_Tool::MapVertexEdges(FI,MVE);
  TopTools_IndexedDataMapOfShapeListOfShape MVE;
  TopExp::MapShapesAndUniqueAncestors (FI, TopAbs_VERTEX, TopAbs_EDGE, MVE);

  //---------------------
  // Extension of edges.
  //---------------------
  TopoDS_Edge  NE;
  //TopTools_DataMapIteratorOfDataMapOfShapeListOfShape it(MVE);
  //for  ( ; it.More(); it.Next()) {
  for (Standard_Integer ii = 1; ii <= MVE.Extent(); ii++) {
    //Temporary
    const TopoDS_Vertex& aVertex = TopoDS::Vertex(MVE.FindKey(ii));
    ///////////
    const TopTools_ListOfShape& aEdges = MVE(ii); //it.Value();
    Standard_Boolean   YaBuild = 0;
    TopTools_ListIteratorOfListOfShape itL (aEdges);
    for (; itL.More(); itL.Next())
    {
      const TopoDS_Edge& anEdge = TopoDS::Edge(itL.Value());
      YaBuild = Build.IsBound(anEdge);
      if (YaBuild) break;
    }
    if (YaBuild)
    {
      for (itL.Initialize(aEdges); itL.More(); itL.Next()) {
        const TopoDS_Edge& EI = TopoDS::Edge(itL.Value());
        if (EI.Orientation() != TopAbs_FORWARD &&
            EI.Orientation() != TopAbs_REVERSED)
          continue;
        TopoDS_Shape aLocalShape = OFI.Generated(EI);
        const TopoDS_Edge& OE = TopoDS::Edge(aLocalShape);
        if (!MES.IsBound(OE) && !Build.IsBound(EI)) {
          Standard_Boolean ToProlong [2] = {Standard_True, Standard_True};
          TopoDS_Vertex aVV [2];
          TopExp::Vertices (EI, aVV[0], aVV[1]);
          for (Standard_Integer ind = 0; ind < 2; ind++)
          {
            const TopTools_ListOfShape& aElist = MVE.FindFromKey(aVV[ind]);
            Standard_Boolean aBuildFound = Standard_False;
            
            TopTools_ListIteratorOfListOfShape anItl(aElist);
            for (; anItl.More(); anItl.Next())
            {
              const TopoDS_Edge& anEdge = TopoDS::Edge(anItl.Value());
              if (anEdge.IsSame(EI))
                continue;
              if (BRep_Tool::Degenerated(anEdge))
                ToProlong[ind] = Standard_False;
              else if (Build.IsBound(anEdge))
                aBuildFound = Standard_True;
            }
            if (!aBuildFound)
              ToProlong[ind] = Standard_False;
          }
          if (!ExtentEdge(OE, NE, Offset, ToProlong[0], ToProlong[1]))
          {
            return Standard_False;
          }
          MES.Bind  (OE,NE);
        }
      }
    } 
  }
  
  TopoDS_Face           FIO = TopoDS::Face(OFI.Face());
  if (MES.IsBound(FIO)) FIO = TopoDS::Face(MES(FIO));
  //
  BRepAdaptor_Surface BAsurf(FIO);
  BRep_Builder aBB;

  TopTools_DataMapOfShapeListOfShape aEdgeElistMap;

  TopExp_Explorer exp(FI.Oriented(TopAbs_FORWARD),TopAbs_WIRE);
  for (; exp.More(); exp.Next()) {
    const TopoDS_Wire&     W = TopoDS::Wire(exp.Current());

    TopTools_DataMapOfShapeShape aEdgeNewEdgeMap;
    TopTools_DataMapOfShapeShape aEEmap;
    TopTools_DataMapOfShapeShape anEdgeVer1map;
    TopTools_DataMapOfShapeShape anEdgeVer2map;
    TopTools_DataMapOfShapeReal  anEdgePar1map;
    TopTools_DataMapOfShapeReal  anEdgePar2map;
    
    BRepTools_WireExplorer wexp;
    Standard_Boolean       end = Standard_False ;
    TopoDS_Edge            FirstE,CurE,NextE;

    TopoDS_Shape aLocalWire = W .Oriented(TopAbs_FORWARD);
    TopoDS_Shape aLocalFace = FI.Oriented(TopAbs_FORWARD);
    wexp.Init(TopoDS::Wire(aLocalWire),TopoDS::Face(aLocalFace));
    if (!wexp.More())
      continue; // Protection from case when explorer does not contain edges.
    CurE = FirstE  = wexp.Current();
    TopTools_IndexedMapOfShape Edges;
    
    while (!end) {
      wexp.Next();
      if (wexp.More()) {
        NextE = wexp.Current();
      } 
      else {
        NextE = FirstE; end = Standard_True;
      }
      if (CurE.IsSame(NextE))
        continue;

      TopoDS_Vertex Vref = CommonVertex(CurE, NextE); 

      CurE = Analyse.EdgeReplacement (FI, CurE);
      NextE = Analyse.EdgeReplacement (FI, NextE);

      TopoDS_Shape aLocalShape = OFI.Generated(CurE);
      TopoDS_Edge CEO = TopoDS::Edge(aLocalShape);
      aLocalShape = OFI.Generated(NextE);
      TopoDS_Edge NEO = TopoDS::Edge(aLocalShape);
      //------------------------------------------
      // Inter processing of images of CurE NextE.
      //------------------------------------------
      TopTools_ListOfShape LV1,LV2;
      Standard_Boolean     DoInter = 1;
      TopoDS_Shape         NE1,NE2;
      TopTools_SequenceOfShape NE1seq, NE2seq;
      TopAbs_Orientation   anOr1 = TopAbs_EXTERNAL, anOr2 = TopAbs_EXTERNAL;

      Standard_Integer aChoice = 0;
      TopoDS_Edge InitE2;
      if (Build.IsBound(CurE) && Build.IsBound(NextE)) {
        aChoice = 1;
        NE1 = Build(CurE );
        NE2 = Build(NextE);
        GetEdgesOrientedInFace (NE1, FIO, theAsDes, NE1seq);
        GetEdgesOrientedInFace (NE2, FIO, theAsDes, NE2seq);
        anOr1 = TopAbs_REVERSED;
        anOr2 = TopAbs_FORWARD;
        //anOr1 = anOrVonEcur;
        //anOr2 = anOrVonEnext;
      }
      else if (Build.IsBound(CurE) && MES.IsBound(NEO)) {
        aChoice = 2;
        NE1 = Build(CurE);
        NE2 = MES  (NEO);
        NE2.Orientation (NextE.Orientation());
        GetEdgesOrientedInFace (NE1, FIO, theAsDes, NE1seq);
        NE2seq.Append (NE2);
        InitE2 = NEO;
        InitE2.Orientation (NextE.Orientation());
        anOr1 = TopAbs_REVERSED;
        anOr2 = TopAbs_FORWARD;
        //anOr1 = anOrVonEcur;
        //anOr2 = anOrVonEnext;
      }
      else if (Build.IsBound(NextE) && MES.IsBound(CEO)) {
        aChoice = 3;
        NE1 = Build(NextE);
        NE2 = MES(CEO);
        NE2.Orientation (CurE.Orientation());
        GetEdgesOrientedInFace (NE1, FIO, theAsDes, NE1seq);
        NE2seq.Append (NE2);
        InitE2 = CEO;
        InitE2.Orientation (CurE.Orientation());
        anOr1 = TopAbs_FORWARD;
        anOr2 = TopAbs_REVERSED;
        //anOr1 = anOrVonEnext;
        //anOr2 = anOrVonEcur;
      }
      else {
        DoInter = 0;
      }
      if (DoInter) {
        //------------------------------------
        // NE1,NE2 can be a compound of Edges.
        //------------------------------------
        Standard_Boolean bCoincide;
        TopoDS_Edge aE1, aE2;
        if (aChoice == 1 || aChoice == 2)
        {
          aE1 = TopoDS::Edge (NE1seq.Last());
          aE2 = TopoDS::Edge (NE2seq.First());
        }
        else // aChoice == 3
        {
          aE1 = TopoDS::Edge (NE1seq.First());
          aE2 = TopoDS::Edge (NE2seq.Last());
        }
        
        //Correct orientation of edges
        /*
        if (ToReverse1)
          aE1.Reverse();
        if (ToReverse2)
          aE2.Reverse();
        */
        //////////////////////////////
        
        AddEdgeWithFaceToMap(aE1, FIO, theEFmap);
        AddEdgeWithFaceToMap(aE2, FIO, theEFmap);

        if (aE1.Orientation() == TopAbs_REVERSED)
          anOr1 = TopAbs::Reverse(anOr1);
        if (aE2.Orientation() == TopAbs_REVERSED)
          anOr2 = TopAbs::Reverse(anOr2);
        
        TopoDS_Vertex aVforE1, aVforE2;
        Standard_Real aParForE1, aParForE2;
        RefEdgeInter(FIO, BAsurf, aE1, aE2, anOr1, anOr2, AsDes2d,
                     Tol, Standard_True, Vref, theImageVV, theDMVV, bCoincide,
                     aVforE1, aVforE2, aParForE1, aParForE2);
        
        if (theEdgeIntEdges.IsBound(aE1))
          theEdgeIntEdges(aE1).Append(aE2);
        else
        {
          TopTools_ListOfShape aElist;
          aElist.Append(aE2);
          theEdgeIntEdges.Bind (aE1, aElist);
        }
        if (theEdgeIntEdges.IsBound(aE2))
          theEdgeIntEdges(aE2).Append(aE1);
        else
        {
          TopTools_ListOfShape aElist;
          aElist.Append(aE1);
          theEdgeIntEdges.Bind (aE2, aElist);
        }
        
        TopoDS_Edge aNewE1, aNewE2;
        
        //Process new edge for <aE1>
        if (aEEmap.IsBound(aE1))
          aNewE1 = TopoDS::Edge(aEEmap(aE1));
        else
        {
          aNewE1 = TopoDS::Edge(aE1.EmptyCopied());
          aNewE1.Orientation(TopAbs_FORWARD);
          aEEmap.Bind(aE1, aNewE1);
        }
        if (aVforE1.Orientation() == TopAbs_FORWARD)
        {
          anEdgeVer1map.Bind (aNewE1, aVforE1);
          anEdgePar1map.Bind (aNewE1, aParForE1);
        }
        else
        {
          anEdgeVer2map.Bind (aNewE1, aVforE1);
          anEdgePar2map.Bind (aNewE1, aParForE1);
        }
        if (NE1seq.Length() > 1)
        {
          //must add constant vertex
          TopoDS_Vertex aV1, aV2;
          TopExp::Vertices (aE1, aV1, aV2);
          Standard_Real aFpar, aLpar;
          BRep_Tool::Range (aE1, aFpar, aLpar);
          if (aVforE1.Orientation() == TopAbs_FORWARD)
          {
            anEdgeVer2map.Bind (aNewE1, aV2);
            anEdgePar2map.Bind (aNewE1, aLpar);
          }
          else
          {
            anEdgeVer1map.Bind (aNewE1, aV1);
            anEdgePar1map.Bind (aNewE1, aFpar);
          }
        }
        aEdgeNewEdgeMap.Bind(aE1, aNewE1);
        
        //Process new edge for <aE2>
        if (aEEmap.IsBound(aE2))
          aNewE2 = TopoDS::Edge(aEEmap(aE2));
        else
        {
          aNewE2 = TopoDS::Edge(aE2.EmptyCopied());
          aNewE2.Orientation(TopAbs_FORWARD);
          aEEmap.Bind(aE2, aNewE2);
        }
        if (!InitE2.IsNull())
        {
          TopoDS_Vertex aV1, aV2;
          TopExp::Vertices (InitE2, aV1, aV2);
          Standard_Real aFpar, aLpar;
          BRep_Tool::Range (InitE2, aFpar, aLpar);
          
          if (aVforE2.Orientation() == TopAbs_FORWARD &&
              !anEdgeVer2map.IsBound(aNewE2))
          {
            anEdgeVer2map.Bind (aNewE2, aV2);
            anEdgePar2map.Bind (aNewE2, aLpar);
          }
          else if (aVforE2.Orientation() == TopAbs_REVERSED &&
                   !anEdgeVer1map.IsBound(aNewE2))
          {
            anEdgeVer1map.Bind (aNewE2, aV1);
            anEdgePar1map.Bind (aNewE2, aFpar);
          }
        }
        if (aVforE2.Orientation() == TopAbs_FORWARD)
        {
          anEdgeVer1map.Bind (aNewE2, aVforE2);
          anEdgePar1map.Bind (aNewE2, aParForE2);
        }
        else
        {
          anEdgeVer2map.Bind (aNewE2, aVforE2);
          anEdgePar2map.Bind (aNewE2, aParForE2);
        }
        aEdgeNewEdgeMap.Bind(aE2, aNewE2);
        

        //
        // check if some of the offset edges have been
        // generated out of the common vertex
        if (Build.IsBound(Vref)) {
          FacesWithVerts.Add(FI);
        }
      }
      else {
        TopoDS_Vertex  V = CommonVertex(CEO,NEO);
        if (!V.IsNull())
        {
          if (MES.IsBound(CEO)) {
            UpdateVertex  (V,CEO,TopoDS::Edge(MES(CEO)),Tol);
            AsDes2d->Add     (MES(CEO),V);
          }
          if (MES.IsBound(NEO)) {
            UpdateVertex (V,NEO,TopoDS::Edge(MES(NEO)),Tol);
            AsDes2d->Add    (MES(NEO),V);
          }

          Standard_Real fpar1, lpar1, fpar2, lpar2;
          BRep_Tool::Range(CEO, fpar1, lpar1);
          BRep_Tool::Range(NEO, fpar2, lpar2);

          TopoDS_Vertex aV1, aV2;
          TopExp::Vertices (CEO, aV1, aV2);
          if (aV1.IsSame(aV2)) //CEO is closed
          {
            anEdgeVer1map.Bind (CEO, V.Oriented(TopAbs_FORWARD));
            anEdgeVer2map.Bind (CEO, V.Oriented(TopAbs_REVERSED));
            if (CEO.Orientation() == TopAbs_FORWARD)
              anEdgePar2map.Bind (CEO, lpar1);
            else
              anEdgePar1map.Bind (CEO, fpar1);
          }
          else
          {
            if (CEO.Orientation() == TopAbs_FORWARD)
            {
              anEdgeVer2map.Bind (CEO, V.Oriented(TopAbs_REVERSED));
              anEdgePar2map.Bind (CEO, lpar1);
            }
            else
            {
              anEdgeVer1map.Bind (CEO, V.Oriented(TopAbs_FORWARD));
              anEdgePar1map.Bind (CEO, fpar1);
            }
          }

          TopExp::Vertices (NEO, aV1, aV2);
          if (aV1.IsSame(aV2)) //NEO is closed
          {
            anEdgeVer1map.Bind (NEO, V.Oriented(TopAbs_FORWARD));
            anEdgeVer2map.Bind (NEO, V.Oriented(TopAbs_REVERSED));
            if (NEO.Orientation() == TopAbs_FORWARD)
              anEdgePar1map.Bind (NEO, fpar2);
            else
              anEdgePar2map.Bind (NEO, lpar2);
          }
          else
          {
            if (NEO.Orientation() == TopAbs_FORWARD)
            {
              anEdgeVer1map.Bind (NEO, V.Oriented(TopAbs_FORWARD));
              anEdgePar1map.Bind (NEO, fpar2);
            }
            else
            {
              anEdgeVer2map.Bind (NEO, V.Oriented(TopAbs_REVERSED));
              anEdgePar2map.Bind (NEO, lpar2);
            }
          }
          
          aEdgeNewEdgeMap.Bind(CEO, CEO);
          aEdgeNewEdgeMap.Bind(NEO, NEO);
        }
      }
      CurE = wexp.Current();
      //ToReverse1 = ToReverse2;
    }

    //Update Edge-EdgeList map
    TopTools_DataMapIteratorOfDataMapOfShapeShape itmap(aEdgeNewEdgeMap);
    for (; itmap.More(); itmap.Next())
    {
      const TopoDS_Shape& anEdge = itmap.Key();
      TopoDS_Edge aNewEdge = TopoDS::Edge(itmap.Value());
      if (anEdgeVer1map.IsBound(aNewEdge) &&
          anEdgeVer2map.IsBound(aNewEdge))
      {
        Standard_Integer NbVer = 0;
        TopoDS_Iterator itv(aNewEdge);
        for (; itv.More(); itv.Next())
          NbVer++;
        if (NbVer < 2)
        {
          TopoDS_Vertex aV1 = TopoDS::Vertex(anEdgeVer1map(aNewEdge));
          TopoDS_Vertex aV2 = TopoDS::Vertex(anEdgeVer2map(aNewEdge));
          aBB.Add (aNewEdge, aV1);
          aBB.Add (aNewEdge, aV2);
          Standard_Real aFpar = anEdgePar1map(aNewEdge);
          Standard_Real aLpar = anEdgePar2map(aNewEdge);
          aBB.Range (aNewEdge, aFpar, aLpar);
        }

        TopTools_ListOfShape aElist;
        aElist.Append(aNewEdge);
        aEdgeElistMap.Bind (anEdge, aElist);
      }
    }

    //Create a new wire
    /*
    TopoDS_Wire aNewWire;
    aBB.MakeWire(aNewWire);
    TopTools_DataMapIteratorOfDataMapOfShapeListOfShape itmap(aEdgeElistMap);
    for (; itmap.More(); itmap.Next())
      aBB.Add (aNewWire, itmap.Value().First());
    aNewWire.Orientation (W.Orientation());
    */
  }

  theFaceEdgeEdge.Bind(FIO, aEdgeElistMap);
  
  return Standard_True;
}

//=======================================================================
//function : ConnexIntByIntInVert
//purpose  : Intersection of the edges generated out of vertices
//=======================================================================
void BRepOffset_Inter2d::ConnexIntByIntInVert
 (const TopoDS_Face&            FI,
  BRepOffset_Offset&            OFI,
  TopTools_DataMapOfShapeShape& MES,
  const TopTools_DataMapOfShapeShape& Build,
  const Handle(BRepAlgo_AsDes)& AsDes,
  const Handle(BRepAlgo_AsDes)& AsDes2d,
  const Standard_Real           Tol,
  const BRepOffset_Analyse&     Analyse,
  TopTools_IndexedDataMapOfShapeListOfShape& theDMVV)
{
  TopoDS_Face           FIO = TopoDS::Face(OFI.Face());
  if (MES.IsBound(FIO)) FIO = TopoDS::Face(MES(FIO));
  //
  TopTools_MapOfShape aME;
  const TopTools_ListOfShape& aLE = AsDes->Descendant(FIO);
  TopTools_ListIteratorOfListOfShape aItLE(aLE);
  for (; aItLE.More(); aItLE.Next()) {
    const TopoDS_Shape& aE = aItLE.Value();
    aME.Add(aE);
  }
  //
  BRepAdaptor_Surface BAsurf(FIO);
  //
  TopExp_Explorer exp(FI.Oriented(TopAbs_FORWARD),TopAbs_WIRE);
  for (; exp.More(); exp.Next()) {
    const TopoDS_Wire&     W = TopoDS::Wire(exp.Current());
    //
    BRepTools_WireExplorer wexp;
    Standard_Boolean       end = Standard_False ;
    TopoDS_Edge            FirstE,CurE,NextE;
    //
    TopoDS_Shape aLocalWire = W .Oriented(TopAbs_FORWARD);
    TopoDS_Shape aLocalFace = FI.Oriented(TopAbs_FORWARD);
    wexp.Init(TopoDS::Wire(aLocalWire),TopoDS::Face(aLocalFace));
    if (!wexp.More())
      continue; // Protection from case when explorer does not contain edges.
    //
    CurE = FirstE  = wexp.Current(); 
    while (!end) {
      wexp.Next();
      if (wexp.More()) {
        NextE = wexp.Current();
      } 
      else {
        NextE = FirstE; end = Standard_True;
      }
      if (CurE.IsSame(NextE)) continue;
      //
      TopoDS_Vertex Vref = CommonVertex(CurE, NextE); 
      if (!Build.IsBound(Vref)) {
        CurE = NextE;
        continue;
      }

      CurE = Analyse.EdgeReplacement (FI, CurE);
      NextE = Analyse.EdgeReplacement (FI, NextE);

      TopoDS_Shape aLocalShape = OFI.Generated(CurE);
      TopoDS_Edge CEO = TopoDS::Edge(aLocalShape);
      aLocalShape = OFI.Generated(NextE);
      TopoDS_Edge NEO = TopoDS::Edge(aLocalShape);
      //
      TopoDS_Shape         NE1,NE2;
      TopAbs_Orientation   anOr1 = TopAbs_EXTERNAL, anOr2 = TopAbs_EXTERNAL;
      
      if (Build.IsBound(CurE) && Build.IsBound(NextE)) {
        NE1 = Build(CurE );
        NE2 = Build(NextE);
      }
      else if (Build.IsBound(CurE) && MES.IsBound(NEO)) {
        NE1 = Build(CurE);
        NE2 = MES  (NEO);
      }
      else if (Build.IsBound(NextE) && MES.IsBound(CEO)) {
        NE1 = Build(NextE);
        NE2 = MES(CEO);
      }
      else {
        CurE = wexp.Current();
        continue;
      }
      //
      TopExp_Explorer Exp1, Exp2;
      Standard_Boolean bCoincide;
      // intersect edges generated from vertex with the edges of the face
      TopoDS_Shape NE3 = Build(Vref);
      //
      for (Exp2.Init(NE3, TopAbs_EDGE); Exp2.More(); Exp2.Next()) {
        const TopoDS_Edge& aE3 = *(TopoDS_Edge*)&Exp2.Current();
        if (!aME.Contains(aE3)) {
          continue;
        }
        //
        // intersection with first edge
        for (Exp1.Init(NE1, TopAbs_EDGE); Exp1.More(); Exp1.Next()) {
          const TopoDS_Edge& aE1 = TopoDS::Edge(Exp1.Current());
          BRepAlgo_Image anEmptyImage;
          TopoDS_Vertex aVforE1, aVforE2;
          Standard_Real aParForE1, aParForE2;
          RefEdgeInter(FIO, BAsurf, aE1, aE3, anOr1, anOr2, AsDes2d,
                       Tol, Standard_True, Vref, anEmptyImage, theDMVV, bCoincide,
                       aVforE1, aVforE2, aParForE1, aParForE2);
          if (bCoincide) {
            // in case of coincidence trim the edge E3 the same way as E1
            Store(aE3, AsDes2d->Descendant(aE1), Tol, Standard_True, AsDes2d, theDMVV);
          }
        }
        //
        // intersection with second edge
        for (Exp1.Init(NE2, TopAbs_EDGE); Exp1.More(); Exp1.Next()) {
          const TopoDS_Edge& aE2 = TopoDS::Edge(Exp1.Current());
          BRepAlgo_Image anEmptyImage;
          TopoDS_Vertex aVforE1, aVforE2;
          Standard_Real aParForE1, aParForE2;
          RefEdgeInter(FIO, BAsurf, aE2, aE3, anOr1, anOr2, AsDes2d,
                       Tol, Standard_True, Vref, anEmptyImage, theDMVV, bCoincide,
                       aVforE1, aVforE2, aParForE1, aParForE2);
          if (bCoincide) {
            // in case of coincidence trim the edge E3 the same way as E2
            Store(aE3, AsDes2d->Descendant(aE2), Tol, Standard_True, AsDes2d, theDMVV);
          }
        }
        //
        // intersection of the edges generated from vertex
        // among themselves
        for (Exp1.Init(NE3, TopAbs_EDGE); Exp1.More(); Exp1.Next()) {
          if (aE3.IsSame(Exp1.Current())) {
            break;
          }
        }
        //
        for (Exp1.Next(); Exp1.More(); Exp1.Next()) {
          const TopoDS_Edge& aE3Next = TopoDS::Edge(Exp1.Current());
          if (aME.Contains(aE3Next)) {
            BRepAlgo_Image anEmptyImage;
            TopoDS_Vertex aVforE1, aVforE2;
            Standard_Real aParForE1, aParForE2;
            RefEdgeInter(FIO, BAsurf, aE3Next, aE3, anOr1, anOr2, AsDes2d,
                         Tol, Standard_True, Vref, anEmptyImage, theDMVV, bCoincide,
                         aVforE1, aVforE2, aParForE1, aParForE2);
          }
        }
      }
      CurE = wexp.Current();
    }
  }
}

//=======================================================================
//function : MakeChain
//purpose  : 
//=======================================================================
static void MakeChain(const TopoDS_Shape& theV,
                      const TopTools_IndexedDataMapOfShapeListOfShape& theDMVV,
                      TopTools_MapOfShape& theMDone,
                      TopTools_ListOfShape& theChain)
{
  if (theMDone.Add(theV)) {
    theChain.Append(theV);
    const TopTools_ListOfShape* pLV = theDMVV.Seek(theV);
    if (pLV) {
      TopTools_ListIteratorOfListOfShape aIt(*pLV);
      for (; aIt.More(); aIt.Next()) {
        MakeChain(aIt.Value(), theDMVV, theMDone, theChain);
      }
    }
  }
}

//=======================================================================
//function : FuseVertices
//purpose  : 
//=======================================================================
Standard_Boolean BRepOffset_Inter2d::FuseVertices (const TopTools_IndexedDataMapOfShapeListOfShape& theDMVV,
                                                   const Handle(BRepAlgo_AsDes)& theAsDes,
                                                   BRepAlgo_Image&               theImageVV)
{
  BRep_Builder aBB;
  TopTools_MapOfShape aMVDone;
  Standard_Integer i, aNb = theDMVV.Extent();
  for (i = 1; i <= aNb; ++i) {
    const TopoDS_Vertex& aV = TopoDS::Vertex(theDMVV.FindKey(i));
    //
    // find chain of vertices
    TopTools_ListOfShape aLVChain;
    MakeChain(aV, theDMVV, aMVDone, aLVChain);
    //
    if (aLVChain.Extent() < 2) {
      continue;
    }
    //
    // make new vertex
    TopoDS_Vertex aVNew;
    BOPTools_AlgoTools::MakeVertex(aLVChain, aVNew);
    //
    TopoDS_Vertex aVNewInt = TopoDS::Vertex(aVNew.Oriented(TopAbs_INTERNAL));
    //
    TopTools_ListIteratorOfListOfShape aIt(aLVChain);
    for (; aIt.More(); aIt.Next()) {
      const TopoDS_Shape& aVOld = aIt.Value();
      // update the parameters on edges
      TopoDS_Vertex aVOldInt = TopoDS::Vertex(aVOld.Oriented(TopAbs_INTERNAL));
      const TopTools_ListOfShape& aLE = theAsDes->Ascendant(aVOld);
      //
      TopTools_ListIteratorOfListOfShape aItLE(aLE);
      for (; aItLE.More(); aItLE.Next()) {
        const TopoDS_Edge& aE = TopoDS::Edge(aItLE.Value());
        Standard_Real aTolE = BRep_Tool::Tolerance(aE);
        Standard_Real aT; 
        if (!BRep_Tool::Parameter(aVOldInt, aE, aT))
        {
          BRepAdaptor_Curve aBAcurve(aE);
          gp_Pnt aPnt = BRep_Tool::Pnt(aVOldInt);
          Extrema_ExtPC aProjector(aPnt, aBAcurve, Precision::Confusion());
          gp_Pnt aPnt1, aPnt2;
          Standard_Real aSqDist [4];
          Standard_Real aParams [4];
          aParams[1] = aBAcurve.FirstParameter();
          aParams[2] = aBAcurve.LastParameter();
          aProjector.TrimmedSquareDistances(aSqDist[1], aSqDist[2], aPnt1, aPnt2);
          aSqDist[3] = RealLast();
          if (aProjector.IsDone())
          {
            for (Standard_Integer ind = 1; ind <= aProjector.NbExt(); ind++)
            {
              Standard_Real aProjSqDist = aProjector.SquareDistance(ind);
              if (aProjSqDist < aSqDist[3])
              {
                aSqDist[3] = aProjSqDist;
                aParams[3] = aProjector.Point(ind).Parameter();
              }
            }
          }
          Standard_Real aMinSqDist = RealLast();
          for (Standard_Integer ii = 1; ii <= 3; ii++)
            if (aSqDist[ii] < aMinSqDist)
            {
              aMinSqDist = aSqDist[ii];
              aT = aParams[ii];
            }

          if (aMinSqDist > aTolE)
            return Standard_False;
        }
        aBB.UpdateVertex(aVNewInt, aT, aE, aTolE);
      }
      // and replace the vertex
      theAsDes->Replace(aVOld, aVNew);
      if (theImageVV.IsImage(aVOld))
      {
        const TopoDS_Vertex& aProVertex = TopoDS::Vertex (theImageVV.ImageFrom(aVOld));
        theImageVV.Add (aProVertex, aVNew.Oriented(TopAbs_FORWARD));
      }
    }
  }
  return Standard_True;
}

static Standard_Boolean ExtendPCurve(Handle(Geom2d_Curve)& thePCurve,
                                     Standard_Real         theFirstOnEdge,
                                     Standard_Real         theLastOnEdge,
                                     Standard_Real         theFirstPar,
                                     Standard_Real         theLastPar,
                                     Standard_Real         theDelta)
{
  Handle(Geom2d_TrimmedCurve) aTrPCurve = 
    new Geom2d_TrimmedCurve(thePCurve, theFirstPar, theLastPar);
  
  // The curve is not prolonged on begin or end.
  // Trying to prolong it adding a segment to its bound.
  gp_Pnt2d                              aPBnd;
  gp_Vec2d                              aVBnd;
  gp_Pnt2d                              aPBeg;
  gp_Dir2d                              aDBnd;
  Handle(Geom2d_Line)                   aLin;
  Handle(Geom2d_TrimmedCurve)           aSegment;
  Geom2dConvert_CompCurveToBSplineCurve aCompCurve(aTrPCurve, Convert_RationalC1);
  Standard_Real                         aTol = Precision::Confusion();
  //Standard_Real                       aDelta = Max(a2Offset, 1.);
  //Standard_Real                       aDelta = anOffset2d;
  
  if (theFirstPar > theFirstOnEdge - theDelta) {
    thePCurve->D1 (theFirstPar, aPBnd, aVBnd);
    aDBnd.SetXY (aVBnd.XY());
    aPBeg    = aPBnd.Translated (gp_Vec2d(-theDelta*aDBnd.XY()));
    aLin     = new Geom2d_Line (aPBeg, aDBnd);
    aSegment = new Geom2d_TrimmedCurve (aLin, 0, theDelta);
    
    if (!aCompCurve.Add(aSegment, aTol))
      return Standard_False;
  }
  
  if (theLastPar < theLastOnEdge + theDelta) {
    thePCurve->D1 (theLastPar, aPBeg, aVBnd);
    aDBnd.SetXY (aVBnd.XY());
    aLin     = new Geom2d_Line (aPBeg, aDBnd);
    aSegment = new Geom2d_TrimmedCurve (aLin, 0, theDelta);
    
    if (!aCompCurve.Add (aSegment, aTol))
      return Standard_False;
  }
  
  thePCurve = aCompCurve.BSplineCurve();
  return Standard_True;
}

static Standard_Boolean ComputeEndOfExtension(const Handle(Geom_Surface)&  theSurf,
                                              const gp_Pnt2d&              theStartPoint,
                                              const gp_Dir2d&              theTangent,
                                              const Standard_Real          the2Offset,
                                              //gp_Pnt2d&                   theEndPoint,
                                              Standard_Real&               theParamOnEnd,
                                              Standard_Real&               theLenToEnd,
                                              Handle(Geom2d_TrimmedCurve)& theSegment)
{
  Standard_Real aUmin, aUmax, aVmin, aVmax;
  theSurf->Bounds(aUmin, aUmax, aVmin, aVmax);

  Handle(Geom2d_Line) aLine;
  Handle(Geom2d_Curve) BoundLines [4];
  if (!Precision::IsInfinite(aUmin) && !Precision::IsInfinite(aVmin) &&
      !Precision::IsInfinite(aUmax) && !Precision::IsInfinite(aVmax))
  {
    gp_Pnt2d anOrigin(aUmin, aVmin);
    aLine = new Geom2d_Line(anOrigin, gp::DX2d());
    BoundLines[0] = new Geom2d_TrimmedCurve(aLine, 0., aUmax-aUmin);
    aLine = new Geom2d_Line(anOrigin, gp::DY2d());
    BoundLines[1] = new Geom2d_TrimmedCurve(aLine, 0., aVmax-aVmin);
    aLine = new Geom2d_Line(gp_Pnt2d(aUmin, aVmax), gp::DX2d());
    BoundLines[2] = new Geom2d_TrimmedCurve(aLine, 0., aUmax-aUmin);
    aLine = new Geom2d_Line(gp_Pnt2d(aUmax, aVmin), gp::DY2d());
    BoundLines[3] = new Geom2d_TrimmedCurve(aLine, 0., aVmax-aVmin);
  }
  else
    return Standard_True; //infinite case

  Standard_Real aMaxSize = Max(aUmax - aUmin, aVmax - aVmin);
  aMaxSize *= 2;
  aLine = new Geom2d_Line(theStartPoint, theTangent);
  //Handle(Geom_Curve) anExtension = new Geom2d_TrimmedCurve(aLine, 0., aMaxSize);
  Geom2dAdaptor_Curve aGAextension(aLine, 0., aMaxSize);

  gp_Pnt2d aPointOnBound;
  Standard_Real anEndParam = Precision::Infinite();
  for (Standard_Integer ii = 0; ii < 4; ii++)
  {
    Geom2dAdaptor_Curve aGAbound(BoundLines[ii]);
    Geom2dInt_GInter Inters(aGAextension, aGAbound, Precision::PConfusion(), Precision::PConfusion());
    if (Inters.IsDone() && !Inters.IsEmpty())
    {
      if (Inters.NbPoints() == 0) //coincidense
      {
        gp_Vec2d aVec = BoundLines[ii]->DN(0., 1);
        gp_Dir2d aDir = aVec;
        Standard_Real aScalProd = theTangent * aVec;
        if (aScalProd > 0)
          aPointOnBound = BoundLines[ii]->Value(BoundLines[ii]->LastParameter());
        else
          aPointOnBound = BoundLines[ii]->Value(BoundLines[ii]->FirstParameter());
        gp_Ax2d anAxis(theStartPoint, theTangent);
        anEndParam = ElCLib::LineParameter (anAxis, aPointOnBound);
      }
      else
      {
        const IntRes2d_IntersectionPoint& anIntPoint = Inters.Point(1);
        aPointOnBound = anIntPoint.Value();
        anEndParam = anIntPoint.ParamOnFirst();
      }
      break;
    }
  }

  Standard_Boolean IsFitted = Standard_False;
  theLenToEnd = 0.;

  if (anEndParam < Precision::Confusion())
    return IsFitted;
  
  const Standard_Integer NCONTROL = 23;
  Standard_Real aDelta = anEndParam / (NCONTROL-1);
  gp_Pnt aPrevPnt;
  for (Standard_Integer ii = 0; ii < NCONTROL; ii++)
  {
    //Standard_Real aPar = ii*aDelta;
    theParamOnEnd = ii*aDelta;
    gp_Pnt2d aPnt2d = aLine->Value(theParamOnEnd);
    gp_Pnt aPnt = theSurf->Value(aPnt2d.X(), aPnt2d.Y());
    if (ii > 0)
    {
      theLenToEnd += aPrevPnt.Distance(aPnt);
      if (theLenToEnd >= the2Offset)
      {
        IsFitted = Standard_True;
        break;
      }
    }
    aPrevPnt = aPnt;
  }

  theSegment = new Geom2d_TrimmedCurve(aLine, 0., theParamOnEnd);
  return IsFitted;
}

static void AddEdgeWithFaceToMap(const TopoDS_Edge& theEdge,
                                 const TopoDS_Face& theFace,
                                 TopTools_IndexedDataMapOfShapeListOfShape& theEFmap)
{
  if (theEFmap.Contains(theEdge))
  {
    TopTools_ListOfShape& aFlist = theEFmap.ChangeFromKey(theEdge);
    TopTools_ListIteratorOfListOfShape itface(aFlist);
    Standard_Boolean IsFaceNew = Standard_True;
    for (; itface.More(); itface.Next())
      if (theFace.IsSame(itface.Value()))
        IsFaceNew = Standard_False;
    if (IsFaceNew)
      aFlist.Append(theFace);
  }
  else
  {
    TopTools_ListOfShape aFlist;
    aFlist.Append(theFace);
    theEFmap.Add(theEdge, aFlist);
  }
}

static Standard_Boolean OrientationInEdge(const TopoDS_Vertex& theVertex,
                                          const TopoDS_Edge&   theEdge,
                                          TopAbs_Orientation&  theOr)
{
  theOr = TopAbs_EXTERNAL;
  
  TopoDS_Edge anEdge = theEdge;
  anEdge.Orientation(TopAbs_FORWARD);

  Standard_Integer NbFound = 0;
  TopoDS_Iterator itv(anEdge);
  for (; itv.More(); itv.Next())
  {
    const TopoDS_Shape& aVertex = itv.Value();
    if (aVertex.IsSame(theVertex))
    {
      theOr = aVertex.Orientation();
      NbFound++;
    }
  }

  return (NbFound == 1);
}

static Standard_Integer DefineClosedness(const TopoDS_Face& theFace)
{
  TopExp_Explorer anExplo (theFace, TopAbs_EDGE);
  for (; anExplo.More(); anExplo.Next())
  {
    const TopoDS_Edge& anEdge = TopoDS::Edge (anExplo.Current());
    if (BRepTools::IsReallyClosed(anEdge, theFace))
    {
      Standard_Real fpar, lpar;
      Handle(Geom2d_Curve) aPCurve = BRep_Tool::CurveOnSurface(anEdge, theFace, fpar, lpar);
      gp_Vec2d aTangent = aPCurve->DN(fpar, 1);
      Standard_Real aCrossProd1 = aTangent ^ gp::DX2d();
      Standard_Real aCrossProd2 = aTangent ^ gp::DY2d();
      if (Abs(aCrossProd2) < Abs(aCrossProd1)) //pcurve is parallel to OY
        return 1;
      else
        return 2;
    }
  }

  return 0;
}

static void GetEdgesOrientedInFace(const TopoDS_Shape& theShape,
                                   const TopoDS_Face&  theFace,
                                   const Handle(BRepAlgo_AsDes)& theAsDes,
                                   TopTools_SequenceOfShape&     theSeqEdges)
{
  const TopTools_ListOfShape& aEdges = theAsDes->Descendant (theFace);

  TopExp_Explorer anExplo (theShape, TopAbs_EDGE);
  for (; anExplo.More(); anExplo.Next())
  {
    const TopoDS_Shape& anEdge = anExplo.Current();
    TopTools_ListIteratorOfListOfShape itl (aEdges);
    for (; itl.More(); itl.Next())
    {
      const TopoDS_Shape& anEdgeInFace = itl.Value();
      if (anEdgeInFace.IsSame(anEdge))
      {
        theSeqEdges.Append (anEdgeInFace);
        break;
      }
    }
  }

  if (theSeqEdges.Length() == 1)
    return;

  TopTools_IndexedDataMapOfShapeListOfShape aVEmap;
  for (Standard_Integer ii = 1; ii <= theSeqEdges.Length(); ii++)
    TopExp::MapShapesAndAncestors (theSeqEdges(ii), TopAbs_VERTEX, TopAbs_EDGE, aVEmap);

  TopoDS_Vertex aFirstVertex;
  TopoDS_Edge aFirstEdge;
  for (Standard_Integer ii = 1; ii <= aVEmap.Extent(); ii++)
  {
    const TopoDS_Vertex& aVertex = TopoDS::Vertex (aVEmap.FindKey(ii));
    const TopTools_ListOfShape& aElist = aVEmap(ii);
    if (aElist.Extent() == 1)
    {
      const TopoDS_Edge& anEdge = TopoDS::Edge(aElist.First());
      TopoDS_Vertex aV1, aV2;
      TopExp::Vertices(anEdge, aV1, aV2, Standard_True); //with orientation
      if (aV1.IsSame(aVertex))
      {
        aFirstVertex = aVertex;
        aFirstEdge = anEdge;
        break;
      }
    }
  }

  if (aFirstEdge.IsNull()) //closed set of edges
  {
    //Standard_Real aPeriod = 0.;
    Standard_Integer IndCoord = DefineClosedness (theFace);
    /*
    BRepAdaptor_Surface aBAsurf (theFace, Standard_False);
    if (IndCoord == 1)
      aPeriod = aBAsurf.LastUParameter() - aBAsurf.FirstUParameter();
    else if (IndCoord == 2)
      aPeriod = aBAsurf.LastVParameter() - aBAsurf.FirstVParameter();
    */
    
    if (IndCoord != 0)
    {
      Standard_Real aMaxDelta = 0.;
      for (Standard_Integer ii = 1; ii <= aVEmap.Extent(); ii++)
      {
        const TopoDS_Vertex& aVertex = TopoDS::Vertex (aVEmap.FindKey(ii));
        const TopTools_ListOfShape& aElist = aVEmap(ii);
        const TopoDS_Edge& anEdge1 = TopoDS::Edge(aElist.First());
        const TopoDS_Edge& anEdge2 = TopoDS::Edge(aElist.Last());
        Standard_Real aParam1 = BRep_Tool::Parameter(aVertex, anEdge1);
        Standard_Real aParam2 = BRep_Tool::Parameter(aVertex, anEdge2);
        BRepAdaptor_Curve2d aBAcurve1 (anEdge1, theFace);
        BRepAdaptor_Curve2d aBAcurve2 (anEdge2, theFace);
        gp_Pnt2d aPnt1 = aBAcurve1.Value(aParam1);
        gp_Pnt2d aPnt2 = aBAcurve2.Value(aParam2);
        Standard_Real aDelta = Abs(aPnt1.Coord(IndCoord) - aPnt2.Coord(IndCoord));
        if (aDelta > aMaxDelta)
        {
          aMaxDelta = aDelta;
          aFirstVertex = aVertex;
        }
      }
      const TopTools_ListOfShape& aElist = aVEmap.FindFromKey(aFirstVertex);
      TopTools_ListIteratorOfListOfShape itl (aElist);
      for (; itl.More(); itl.Next())
      {
        const TopoDS_Edge& anEdge = TopoDS::Edge(itl.Value());
        TopoDS_Vertex aV1, aV2;
        TopExp::Vertices(anEdge, aV1, aV2, Standard_True); //with orientation
        if (aV1.IsSame(aFirstVertex))
        {
          aFirstEdge = anEdge;
          break;
        }
      }
    }
  }

  Standard_Integer aNbEdges = theSeqEdges.Length();
  theSeqEdges.Clear();
  theSeqEdges.Append (aFirstEdge);
  TopoDS_Edge anEdge = aFirstEdge;
  for (;;)
  {
    TopoDS_Vertex aLastVertex = TopExp::LastVertex (anEdge, Standard_True); //with orientation
    if (aLastVertex.IsSame(aFirstVertex))
      break;
    
    const TopTools_ListOfShape& aElist = aVEmap.FindFromKey(aLastVertex);
    if (aElist.Extent() == 1)
      break;
    
    if (aElist.First().IsSame(anEdge))
      anEdge = TopoDS::Edge(aElist.Last());
    else
      anEdge = TopoDS::Edge(aElist.First());

    theSeqEdges.Append (anEdge);
    if (theSeqEdges.Length() == aNbEdges)
      break;
  }
}
