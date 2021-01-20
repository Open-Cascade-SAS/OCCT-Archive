// Copyright (c) 2021 OPEN CASCADE SAS
//
// This file is part of the examples of the Open CASCADE Technology software library.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE

#include "WasmOcctObject.h"

#include <BRep_Builder.hxx>
#include <BRep_Tool.hxx>
#include <Graphic3d_ArrayOfTriangles.hxx>
#include <Prs3d_ShadingAspect.hxx>
#include <Select3D_SensitiveTriangulation.hxx>
#include <SelectMgr_EntityOwner.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <XCAFDoc_VisMaterial.hxx>

//! Gets triangulation of every face of shape and fills output array of triangles
static Handle(Graphic3d_ArrayOfTriangles) fillTriangles (const TopoDS_Shape& theShape,
                                                         const bool theHasTexels,
                                                         const gp_Pnt2d& theUVOrigin,
                                                         const gp_Pnt2d& theUVRepeat,
                                                         const gp_Pnt2d& theUVScale)
{
  TopLoc_Location aLoc;
  Standard_Integer aNbTriangles = 0, aNbVertices = 0;
  bool hasNormals = true;
  for (TopExp_Explorer aFaceIt (theShape, TopAbs_FACE); aFaceIt.More(); aFaceIt.Next())
  {
    const TopoDS_Face& aFace = TopoDS::Face (aFaceIt.Current());
    if (const Handle(Poly_Triangulation)& aT = BRep_Tool::Triangulation (aFace, aLoc))
    {
      aNbTriangles += aT->NbTriangles();
      aNbVertices  += aT->NbNodes();
      hasNormals = hasNormals && aT->HasNormals();
    }
  }
  if (aNbVertices < 3 || aNbTriangles < 1)
  {
    return Handle(Graphic3d_ArrayOfTriangles)();
  }

  const Graphic3d_ArrayFlags aFlags = (hasNormals   ? Graphic3d_ArrayFlags_VertexNormal : Graphic3d_ArrayFlags_None)
                                    | (theHasTexels ? Graphic3d_ArrayFlags_VertexTexel  : Graphic3d_ArrayFlags_None);
  Handle(Graphic3d_ArrayOfTriangles) anArray = new Graphic3d_ArrayOfTriangles (aNbVertices, 3 * aNbTriangles, aFlags);
  Standard_Real aUmin (0.0), aUmax (1.0), aVmin (0.0), aVmax (1.0), dUmax (1.0), dVmax (1.0);
  for (TopExp_Explorer aFaceIt(theShape, TopAbs_FACE); aFaceIt.More(); aFaceIt.Next())
  {
    const TopoDS_Face& aFace = TopoDS::Face (aFaceIt.Current());
    const Handle(Poly_Triangulation)& aT = BRep_Tool::Triangulation (aFace, aLoc);
    if (aT.IsNull())
    {
      continue;
    }

    // Determinant of transform matrix less then 0 means that mirror transform applied.
    const gp_Trsf& aTrsf = aLoc.Transformation();
    const bool isMirrored = aTrsf.VectorialPart().Determinant() < 0;

    // Extracts vertices & normals from nodes
    const TColgp_Array1OfPnt&   aNodes   = aT->Nodes();
    const TColgp_Array1OfPnt2d* aUVNodes = theHasTexels && aT->HasUVNodes() && aT->UVNodes().Upper() == aNodes.Upper()
                                         ? &aT->UVNodes()
                                         : NULL;

    const TShort_Array1OfShortReal* aNormals = aT->HasNormals() ? &aT->Normals() : NULL;
    const Standard_ShortReal*       aNormArr = aNormals != NULL ? &aNormals->First() : NULL;

    const Standard_Integer aVertFrom = anArray->VertexNumber();
    for (Standard_Integer aNodeIter = aNodes.Lower(); aNodeIter <= aNodes.Upper(); ++aNodeIter)
    {
      gp_Pnt aPoint = aNodes (aNodeIter);
      const Standard_Integer anId = 3 * (aNodeIter - aNodes.Lower());
      gp_Dir aNorm = aNormArr != NULL ? gp_Dir (aNormArr[anId + 0], aNormArr[anId + 1], aNormArr[anId + 2]) : gp::DZ();
      if ((aFace.Orientation() == TopAbs_REVERSED) ^ isMirrored)
      {
        aNorm.Reverse();
      }
      if (!aLoc.IsIdentity())
      {
        aPoint.Transform (aTrsf);
        aNorm .Transform (aTrsf);
      }

      if (aUVNodes != NULL)
      {
        const gp_Pnt2d aTexel = (dUmax == 0.0 || dVmax == 0.0)
                              ? aUVNodes->Value (aNodeIter)
                              : gp_Pnt2d ((-theUVOrigin.X() + (theUVRepeat.X() * (aUVNodes->Value (aNodeIter).X() - aUmin)) / dUmax) / theUVScale.X(),
                                          (-theUVOrigin.Y() + (theUVRepeat.Y() * (aUVNodes->Value (aNodeIter).Y() - aVmin)) / dVmax) / theUVScale.Y());
        anArray->AddVertex (aPoint, aNorm, aTexel);
      }
      else
      {
        anArray->AddVertex (aPoint, aNorm);
      }
    }

    // Fill array with vertex and edge visibility info
    const Poly_Array1OfTriangle& aTriangles = aT->Triangles();
    Standard_Integer anIndex[3] = {};
    for (Standard_Integer aTriIter = 1; aTriIter <= aT->NbTriangles(); ++aTriIter)
    {
      aTriangles (aTriIter).Get (anIndex[0], anIndex[1], anIndex[2]);
      if (aFace.Orientation() == TopAbs_REVERSED) { std::swap (anIndex[1], anIndex[2]); }
      anArray->AddEdges (anIndex[0] + aVertFrom, anIndex[1] + aVertFrom, anIndex[2] + aVertFrom);
    }
  }
  return anArray;
}

// ================================================================
// Function : WasmOcctObject
// Purpose  :
// ================================================================
WasmOcctObject::WasmOcctObject()
{
}

// ================================================================
// Function : ~WasmOcctObject
// Purpose  :
// ================================================================
WasmOcctObject::~WasmOcctObject()
{
}

// ================================================================
// Function : Compute
// Purpose  :
// ================================================================
void WasmOcctObject::Compute (const Handle(PrsMgr_PresentationManager3d)& thePrsMgr,
                              const Handle(Prs3d_Presentation)& thePrs,
                              const Standard_Integer theMode)
{
  if (theMode != 0)
  {
    return;
  }

  gp_Pnt2d anUVOrigin, anUVRepeat, anUVScale;
  bool hasTexCoords = false;

  NCollection_DataMap<Handle(XCAFDoc_VisMaterial), TopoDS_Compound> aMatMap;

  RWMesh_NodeAttributes aDefAttribs;
  {
    Handle(XCAFDoc_VisMaterial) aDefMat = new XCAFDoc_VisMaterial();
    XCAFDoc_VisMaterialPBR aPbrMar;
    aPbrMar.IsDefined = true;
    aPbrMar.BaseColor.SetValues (0.243137f, 0.243137f, 0.243137f, 1.0f);
    aDefMat->SetPbrMaterial (aPbrMar);
    aDefAttribs.Style.SetMaterial (aDefMat);
  }
  for (TopTools_SequenceOfShape::Iterator aRootIter (myRootShapes); aRootIter.More(); aRootIter.Next())
  {
    const TopoDS_Shape& aShape = aRootIter.Value();
    /*if (Handle(Graphic3d_ArrayOfTriangles) aPArray = fillTriangles (aShape, hasTexCoords, anUVOrigin, anUVRepeat, anUVScale))
    {
      Handle(Graphic3d_Group) aGroup = thePrs->NewGroup();
      aGroup->SetGroupPrimitivesAspect (myDrawer->ShadingAspect()->Aspect());
      aGroup->AddPrimitiveArray (aPArray);
    }*/

    for (TopExp_Explorer aFaceIter (aShape, TopAbs_FACE); aFaceIter.More(); aFaceIter.Next())
    {
      const TopoDS_Face& aFace = TopoDS::Face (aFaceIter.Value());
      const RWMesh_NodeAttributes* anAttribs = myAttribMap.Seek (aFace);
      if (anAttribs == NULL)
      {
        anAttribs = myAttribMap.Seek (aFace.Located (TopLoc_Location()));
        if (anAttribs == NULL)
        {
          anAttribs = &aDefAttribs;
        }
      }
      if (Handle(XCAFDoc_VisMaterial) aVisMat = anAttribs->Style.Material())
      {
        TopoDS_Compound* aComp = aMatMap.ChangeSeek (aVisMat);
        if (aComp == NULL)
        {
          aComp = aMatMap.Bound (aVisMat, TopoDS_Compound());
          BRep_Builder().MakeCompound (*aComp);
        }
        BRep_Builder().Add (*aComp, aFace);
      }
    }
  }

  for (NCollection_DataMap<Handle(XCAFDoc_VisMaterial), TopoDS_Compound>::Iterator aMatIter (aMatMap); aMatIter.More(); aMatIter.Next())
  {
    const Handle(XCAFDoc_VisMaterial)& aVisMat = aMatIter.Key();
    const TopoDS_Compound& aShape = aMatIter.Value();
    if (Handle(Graphic3d_ArrayOfTriangles) aPArray = fillTriangles (aShape, hasTexCoords, anUVOrigin, anUVRepeat, anUVScale))
    {
      Handle(Graphic3d_AspectFillArea3d) anAspects = new Graphic3d_AspectFillArea3d();
      *anAspects = *myDrawer->ShadingAspect()->Aspect();
      aVisMat->FillAspect (anAspects);

      Handle(Graphic3d_Group) aGroup = thePrs->NewGroup();
      aGroup->SetGroupPrimitivesAspect (anAspects);
      aGroup->AddPrimitiveArray (aPArray);
    }
  }
}

// ================================================================
// Function : ComputeSelection
// Purpose  :
// ================================================================
void WasmOcctObject::ComputeSelection (const Handle(SelectMgr_Selection)& theSel,
                                       const Standard_Integer theMode)
{
  if (theMode != 0)
  {
    return;
  }

  Handle(SelectMgr_EntityOwner) anOwner = new SelectMgr_EntityOwner (this, 5);
  for (TopTools_SequenceOfShape::Iterator aRootIter (myRootShapes); aRootIter.More(); aRootIter.Next())
  {
    const TopoDS_Shape& aShape = aRootIter.Value();
    for (TopExp_Explorer aFaceIter (aShape, TopAbs_FACE); aFaceIter.More(); aFaceIter.Next())
    {
      const TopoDS_Face& aFace = TopoDS::Face (aFaceIter.Value());
      TopLoc_Location aLoc;
      if (Handle(Poly_Triangulation) aPolyTri = BRep_Tool::Triangulation (aFace, aLoc))
      {
        Handle(Select3D_SensitiveTriangulation) aSensTris = new Select3D_SensitiveTriangulation (anOwner, aPolyTri, aLoc);
        theSel->Add (aSensTris);
      }
    }
  }
}
