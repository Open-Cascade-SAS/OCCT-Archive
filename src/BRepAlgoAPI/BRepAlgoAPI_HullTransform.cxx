#include <BRepAlgoAPI_HullTransform.hxx>

#include <BOPAlgo_Splitter.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_NurbsConvert.hxx>
#include <BRep_Tool.hxx>
#include <Geom_Plane.hxx>
#include <ShapeAnalysis_ShapeTolerance.hxx>
#include <ShapeFix_Shape.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>

//=======================================================================
//function : BRepAlgoAPI_HullTransform
//purpose  : 
//=======================================================================
BRepAlgoAPI_HullTransform::BRepAlgoAPI_HullTransform()
{
  myIsAutoSection = false;
  myTolerance = 0;
}

//=======================================================================
//function : InitLinear
//purpose  : 
//=======================================================================
void BRepAlgoAPI_HullTransform::InitLinear(const double theCM,
                                           const double theCBNew,
                                           const double theCBOld,
                                           const double theLPP)
{
  myLinear = true;
  _cm = theCM;
  _cb_new = theCBNew;
  _cb_old = theCBOld;
  _lpp = theLPP;
}

//=======================================================================
//function : InitQuad
//purpose  : 
//=======================================================================
void BRepAlgoAPI_HullTransform::InitQuad(const double theAftlim,
                                         const double theCCA,
                                         const double theCCF,
                                         const double theForelim,
                                         const double theAftCoef,
                                         const double theForeCoef,
                                         const bool theModifyAftZone,
                                         const bool theModifyForeZone)
{
  myLinear = false;

  _aftlim = theAftlim;
  _cca = theCCA;
  _ccf = theCCF;
  _forelim = theForelim;
  _aft_coef = theAftCoef;
  _fore_coef = theForeCoef;
  _modify_aft_zone = theModifyAftZone;
  _modify_fore_zone = theModifyForeZone;
}

//=======================================================================
//function : Perform
//purpose  : 
//=======================================================================
TopoDS_Shape BRepAlgoAPI_HullTransform::Perform()
{
  // Do Nurbs convert
  BRepBuilderAPI_NurbsConvert aNC;
  aNC.Perform(myHull);
  myHull = aNC.Shape();

  // Pre calculations for automatic sectioning
  std::list<TopoDS_Shape> aHullsList;
  std::list<std::list<double>> aSectionsList;
  std::list<double> aTolerances;
  bool isToProceed = myIsAutoSection;

  // Set tolerance to 5% of the Hull length if it was not specified.
  if (fabs(myTolerance) < Precision::Confusion())
  {
    myTolerance = (myLinear) ? _lpp : _forelim;
    myTolerance *= 0.05;
  }

  // Perform
  do
  {
    TopoDS_Shape aCurShape = myHull;
    if (mySections.size() > 0)
    {
      // Add splitting edges
      aCurShape = split();
    }

    // Perform transformation
    BRepBuilderAPI_HullTransform aHTrsf;
    if (myLinear)
      aHTrsf.InitLinear(_cm, _cb_new, _cb_old, _lpp);
    else
      aHTrsf.InitQuad(_aftlim, _cca, _ccf, _forelim, _aft_coef, _fore_coef, _modify_aft_zone, _modify_fore_zone);
    aHTrsf.Perform(aCurShape, true);

    if (aHTrsf.IsDone())
    {
      // Fix shape
      Handle(ShapeFix_Shape) aShapeFixTool = new ShapeFix_Shape;
      aShapeFixTool->Init(aHTrsf.Shape());
      aShapeFixTool->Perform();
      TopoDS_Shape aResShape = aShapeFixTool->Shape();
      ShapeAnalysis_ShapeTolerance aSat;
      aSat.InitTolerance();
      aSat.AddTolerance(aResShape);

      // Check the necessarity of the next step
      if (myIsAutoSection && aHullsList.size() > 0 && aSat.GlobalTolerance(0) >= aTolerances.back())
      {
        isToProceed = false;
        mySections.pop_back();
      }
      else
      {
        aTolerances.push_back(aSat.GlobalTolerance(0));
        aHullsList.push_back(aResShape);
        aSectionsList.push_back(mySections);
        if (myIsAutoSection)
          addSection(aResShape);
      }
    }
  } while (isToProceed);

  // Return the result
  if (aHullsList.size() == 0)
    return TopoDS_Shape();
  return aHullsList.back();
}

//=======================================================================
//function : split
//purpose  : 
//=======================================================================
TopoDS_Shape BRepAlgoAPI_HullTransform::split()
{
  TopTools_ListOfShape aLSObjects;
  aLSObjects.Append(myHull);
  TopTools_ListOfShape aLSTools;
  for (double aPlaneX : mySections)
  {
    Handle(Geom_Plane) aPlane = new Geom_Plane(gp_Pnt(aPlaneX, 0, 0), gp_Dir(1, 0, 0));
    TopoDS_Face aFace =
      BRepBuilderAPI_MakeFace(aPlane, -1000, 1000, -1000, 1000, Precision::Confusion());
    aLSTools.Append(aFace);
  }

  BOPAlgo_Splitter pSplitter;
  pSplitter.Clear();
  pSplitter.SetArguments(aLSObjects);
  pSplitter.SetTools(aLSTools);
  pSplitter.Perform();
  return pSplitter.Shape();
}

//=======================================================================
//function : addSection
//purpose  : 
//=======================================================================
void BRepAlgoAPI_HullTransform::addSection(const TopoDS_Shape& theHull)
{
  std::list<std::pair<double, double>> aVertices;
  TopExp_Explorer anExp(theHull, TopAbs_VERTEX);
  for (; anExp.More(); anExp.Next())
  {
    TopoDS_Vertex aVrt = TopoDS::Vertex(anExp.Current());
    gp_Pnt aPnt = BRep_Tool::Pnt(aVrt);
    double aTol = BRep_Tool::Tolerance(aVrt);
    aVertices.push_back(std::pair<double, double>(aTol, aPnt.X()));
  }
  aVertices.sort();
  aVertices.reverse();

  bool isToStop = false;

  // Find new section not too close for the existing set
  for (const auto& aVertex : aVertices)
  {
    if (isToStop)
      break;
    bool isToAdd = true;
    for (const double aSection : mySections)
      if (fabs(aVertex.second - aSection) < myTolerance)
        isToAdd = false;
    if (isToAdd)
    {
      mySections.push_back(aVertex.second);
      isToStop = true;
    }
  }
}
