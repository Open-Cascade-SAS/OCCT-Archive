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

// 22.12.98 dce S3767
// 21.12.98 rln, gka S4054
//#74 rln,pdn 11.03.99 S4135: Setting minimum and maximum tolerances according to static parameters
// sln 13.06.2002 OCC448 : Correction in  method TransferGeometry to avoid transferring invisible sub entities

#include <BRep_Builder.hxx>
#include <BRepLib.hxx>
#include <Geom_Surface.hxx>
#include <GeomAdaptor_Surface.hxx>
#include <gp_GTrsf.hxx>
#include <gp_Trsf.hxx>
#include <gp_Vec.hxx>
#include <IGESBasic_GroupWithoutBackP.hxx>
#include <IGESBasic_SingularSubfigure.hxx>
#include <IGESBasic_SubfigureDef.hxx>
#include <IGESData_IGESEntity.hxx>
#include <IGESData_IGESModel.hxx>
#include <IGESData_ToolLocation.hxx>
#include <IGESToBRep.hxx>
#include <IGESToBRep_BRepEntity.hxx>
#include <IGESToBRep_CurveAndSurface.hxx>
#include <IGESToBRep_TopoCurve.hxx>
#include <IGESToBRep_TopoSurface.hxx>
#include <Interface_Macros.hxx>
#include <Interface_Static.hxx>
#include <Message_Messenger.hxx>
#include <Message_Msg.hxx>
#include <Message_ProgressScope.hxx>
#include <Precision.hxx>
#include <Standard_ErrorHandler.hxx>
#include <Standard_Failure.hxx>
#include <TopLoc_Location.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Shape.hxx>
#include <Transfer_TransientProcess.hxx>
#include <TransferBRep_ShapeBinder.hxx>
#include <TransferBRep_ShapeListBinder.hxx>

#include <stdio.h>
//=======================================================================
//function : IGESToBRep_CurveAndSurface
//purpose  :
//=======================================================================
IGESToBRep_CurveAndSurface::IGESToBRep_CurveAndSurface()
: myEps         (1.e-04),
  myEpsCoeff    (1.e-06),
  myEpsGeom     (1.e-04),
  myMinTol      (-1.0),
  myMaxTol      (-1.0),
  myModeIsTopo  (Standard_True),
  myModeApprox  (Standard_False),
  myContIsOpti  (Standard_False),
  myUnitFactor  (1.0),
  mySurfaceCurve(0),
  myContinuity  (0),
  myUVResolution(0.0),
  myIsResolCom  (Standard_False),
  myTP          (new Transfer_TransientProcess())
{
  UpdateMinMaxTol();
}

//=======================================================================
//function : IGESToBRep_CurveAndSurface
//purpose  : 
//=======================================================================

IGESToBRep_CurveAndSurface::IGESToBRep_CurveAndSurface
  (const Standard_Real    eps,
   const Standard_Real    epsCoeff,
   const Standard_Real    epsGeom,
   const Standard_Boolean mode,
   const Standard_Boolean modeapprox,
   const Standard_Boolean optimized)
: myEps         (eps),
  myEpsCoeff    (epsCoeff),
  myEpsGeom     (epsGeom),
  myMinTol      (-1.0),
  myMaxTol      (-1.0),
  myModeIsTopo  (mode),
  myModeApprox  (modeapprox),
  myContIsOpti  (optimized),
  myUnitFactor  (1.0),
  mySurfaceCurve(0),
  myContinuity  (0),
  myUVResolution(0.0),
  myIsResolCom  (Standard_False),
  myTP          (new Transfer_TransientProcess())
{
  UpdateMinMaxTol();
}

//=======================================================================
//function : Init
//purpose  : 
//=======================================================================

void IGESToBRep_CurveAndSurface::Init()
{  
  myEps        = 1.E-04;
  myEpsCoeff   = 1.E-06;
  myEpsGeom    = 1.E-04;
  myModeIsTopo = Standard_True;
  myModeApprox = Standard_False;
  myContIsOpti = Standard_False;
  myUnitFactor = 1.;
  mySurfaceCurve = 0;
  myContinuity = 0;
  myTP         = new Transfer_TransientProcess();
  
  mySurface.Nullify();
  myIsResolCom = Standard_False;
  myUVResolution = 0.;
  UpdateMinMaxTol();
}

//=======================================================================
//function : SetEpsGeom
//purpose  : 
//=======================================================================
void IGESToBRep_CurveAndSurface::SetEpsGeom(const Standard_Real eps)
{
  myEpsGeom = eps;
  UpdateMinMaxTol();
}


//=======================================================================
//function : UpdateMinMaxTol
//purpose  : 
//=======================================================================

void IGESToBRep_CurveAndSurface::UpdateMinMaxTol()
{
  //#74 rln 11.03.99 S4135: Setting maximum tolerances according to
  //static parameter
  myMaxTol = Max (Interface_Static::RVal ("read.maxprecision.val"), myEpsGeom * myUnitFactor);
  myMinTol = Precision::Confusion();
}

//=======================================================================
//function : SetModel
//purpose  : 
//=======================================================================
void IGESToBRep_CurveAndSurface::SetModel(const Handle(IGESData_IGESModel)& model)
{  
  myModel = model;  
  Standard_Real unitfactor = myModel->GlobalSection().UnitValue();
  if (unitfactor != 1.)
    {
      if ( myTP->TraceLevel() > 2 )
	myTP->Messenger()->SendInfo() << "UnitFactor = "<< unitfactor << std::endl;
      myUnitFactor = unitfactor;
    }
  UpdateMinMaxTol();
}

//=======================================================================
//function : TransferCurveAndSurface
//purpose  : 
//=======================================================================

TopoDS_Shape IGESToBRep_CurveAndSurface::TransferCurveAndSurface
       (const Handle(IGESData_IGESEntity)& start,
        const Message_ProgressRange& theProgress)
{
  TopoDS_Shape res;
  if (start.IsNull()) {
    Message_Msg msg1005("IGES_1005");
    SendFail(start, msg1005);
    return res;
  }
  Handle(TCollection_HAsciiString) label = GetModel()->StringLabel(start);
//  Standard_Integer typeNumber = start->TypeNumber();
  
  if (IGESToBRep::IsTopoCurve(start)) {
    IGESToBRep_TopoCurve TC(*this);
    res = TC.TransferTopoCurve(start);
  }  
  else if (IGESToBRep::IsTopoSurface(start)) {
    IGESToBRep_TopoSurface TS(*this);
    res = TS.TransferTopoSurface(start);
  }
  else if (IGESToBRep::IsBRepEntity(start)) {
    IGESToBRep_BRepEntity TS(*this);
    res = TS.TransferBRepEntity(start, theProgress);
  }
  else {
    Message_Msg msg1015("IGES_1015");
    SendFail(start, msg1015);
    // AddFail(start, "The IGESEntity is not a curve a Surface or a BRep Entity.");
  }
  // mjm le 12/09/96
/*
  if (!res.IsNull()) {
    try {
      OCC_CATCH_SIGNALS
      Standard_Real Eps = GetEpsGeom()*GetUnitFactor();
      BRepLib::SameParameter(res,Eps);
    }
    catch(Standard_Failure) {
    Message_Msg msg1010("IGES_1010");
      SendWarning (start,msg1010);
    }
  }
*/
  return res;
}



//=======================================================================
//function : TransferGeometry
//purpose  : 
//=======================================================================
TopoDS_Shape IGESToBRep_CurveAndSurface::TransferGeometry
                               (const Handle(IGESData_IGESEntity)& theStart,
                                const Message_ProgressRange& theProgress)
{
// Declaration of messages// 
// DCE 22/12/98
//Message_Msg aMsg1005("IGES_1001");  //  The type of the Start is not recognized
//Message_Msg aMsg1005("IGES_1005");  //  Software error :  the Start IsNull.
//Message_Msg aMsg1015("IGES_1015");  //  invalid type or exception raising (software error).
//Message_Msg aMsg1010("IGES_1010");  //  Not sameparameter.
//Message_Msg aMsg1015("IGES_1020");  //  Associated entity IsNull
//Message_Msg aMsg1015("IGES_1025");  //  No shape is found for the associated entity for the type 308
//Message_Msg aMsg1015("IGES_1030");  //  No shape is found for the associated entity for the type 402
//Message_Msg aMsg1015("IGES_1035");  //  The conversion of a Location is not possible
//Message_Msg aMsg210 ("XSTEP_210");  //  No associated entities for the type 308
//Message_Msg aMsg202 ("XSTEP_202");  //  No associated entities for the type 402
////////////////////////////
  TopoDS_Shape aRes;
  gp_Trsf aT408;
  if (theStart.IsNull())
  {
    const Message_Msg aMsg1005("IGES_1005");  //  Software error :  start IsNull.
    SendFail(theStart, aMsg1005);
    return aRes;
  }

  // sln 13.06.2002 OCC448: Avoid transferring invisible sub entities which
  // logically depend on the one
  const Standard_Integer anOnlyvisible = Interface_Static::IVal("read.iges.onlyvisible");

  if (IGESToBRep::IsCurveAndSurface(theStart))
  {
    if(anOnlyvisible && theStart->BlankStatus() == 1)
      return aRes;
    try
    {
      OCC_CATCH_SIGNALS
      aRes = TransferCurveAndSurface(theStart, theProgress);
    }
    catch(Standard_Failure const&)
    {
      const Message_Msg aMsg1015("IGES_1015");
      SendFail(theStart, aMsg1015);
    }
    return aRes;
  }

  //408 : SingularSubfigure
  if (theStart->IsKind(STANDARD_TYPE(IGESBasic_SingularSubfigure)))
    {
      if(anOnlyvisible && theStart->BlankStatus() == 1)
        return aRes;

      DeclareAndCast(IGESBasic_SingularSubfigure, aSt408, theStart);
      Handle (IGESBasic_SubfigureDef) aStsub = aSt408->Subfigure();
      const gp_XYZ aTrans = aSt408->Translation();
      gp_Vec aVectr(aTrans);
      const Standard_Real aScunit = GetUnitFactor();
      aVectr.Multiply(aScunit);
      aT408.SetTranslation(aVectr);
      if (aSt408->HasScaleFactor())
      {
        const Standard_Real aScalef = aSt408->ScaleFactor();
        aT408.SetScaleFactor(aScalef);
      }
      if (HasShapeResult(aStsub))
      {
        aRes = GetShapeResult(aStsub);
      }
      else
      {
        try
        {
          OCC_CATCH_SIGNALS
          aRes = TransferGeometry(aStsub, theProgress);
        }
        catch(Standard_Failure const&)
        {
          aRes.Nullify();
          const Message_Msg aMsg1015("IGES_1015");
          SendFail(aSt408, aMsg1015);
        }
        if (!aRes.IsNull())
        {
          SetShapeResult(aStsub, aRes);
        }
      }
    }

  // 308 : SubfigureDefinition
  else if (theStart->IsKind(STANDARD_TYPE(IGESBasic_SubfigureDef)))
  {
    DeclareAndCast(IGESBasic_SubfigureDef, aSt308, theStart);
    TopoDS_Compound aGroup;
    BRep_Builder aBuilder;
    aBuilder.MakeCompound (aGroup);
    if (aSt308->NbEntities() < 1)
    {
      const Message_Msg aMsg210 ("XSTEP_210");
      SendFail(aSt308, aMsg210);
      return aRes;
    }
    Message_ProgressScope aPS (theProgress, "Subfigure item", aSt308->NbEntities());
    for (Standard_Integer anIndx =1; anIndx <= aSt308->NbEntities() && aPS.More(); anIndx++)
    {
      Message_ProgressRange aRange = aPS.Next();
      TopoDS_Shape anItem;
      if (aSt308->AssociatedEntity(anIndx).IsNull())
      {
        Message_Msg aMsg1020("IGES_1020");
        aMsg1020.Arg(anIndx);
        SendWarning(aSt308, aMsg1020);
        continue;
      }
      if(anOnlyvisible && aSt308->AssociatedEntity(anIndx)->BlankStatus() == 1)
        continue;

      if (HasShapeResult(aSt308->AssociatedEntity(anIndx)))
      {
        anItem = GetShapeResult(aSt308->AssociatedEntity(anIndx));
      }
      else
      {
        try
        {
          OCC_CATCH_SIGNALS
          anItem = TransferGeometry (aSt308->AssociatedEntity(anIndx), aRange);
        }
        catch(Standard_Failure const&)
        {
          anItem.Nullify();
          const Message_Msg aMsg1015("IGES_1015");
          SendFail(aSt308->AssociatedEntity(anIndx), aMsg1015);
        }
      }
      if (anItem.IsNull())
      {
        Message_Msg aMsg1025("IGES_1025");
        aMsg1025.Arg(anIndx);
        SendWarning (theStart, aMsg1025);
      }
      else
      {
        aBuilder.Add(aGroup, anItem);
        SetShapeResult (aSt308->AssociatedEntity(anIndx), anItem);
      }
    }
    aRes = aGroup;
  }
  // 402 : Group Associativity
  else if (theStart->IsKind(STANDARD_TYPE(IGESBasic_Group)))
  {
    if(anOnlyvisible && theStart->BlankStatus() == 1)
      return aRes;

    DeclareAndCast(IGESBasic_Group, aSt402, theStart);
    TopoDS_Compound aGroup;
    BRep_Builder aBuilder;
    aBuilder.MakeCompound (aGroup);
    const Standard_Integer aNbEnt = aSt402->NbEntities();
    if (aNbEnt < 1)
    {
      Message_Msg aMsg202 ("XSTEP_202");
      aMsg202.Arg(aSt402->FormNumber());
      SendFail(aSt402, aMsg202);
      return aRes;
    }
    Message_ProgressScope aPS (theProgress, "Group item", aNbEnt);
    Standard_Boolean aProblemInGroup = Standard_False;
    for (Standard_Integer anIndx=1; anIndx <= aNbEnt && aPS.More(); anIndx++)
    {
      Message_ProgressRange aRange = aPS.Next();
      TopoDS_Shape anItem;
      if (aSt402->Entity(anIndx).IsNull())
      {
        Message_Msg aMsg1020("IGES_1020");
        aMsg1020.Arg(anIndx);
        SendFail(aSt402, aMsg1020);
        continue;
      }

      if(anOnlyvisible && aSt402->Entity(anIndx)->BlankStatus() == 1)
        continue;

      if (HasShapeResult(aSt402->Entity(anIndx)))
      {
        anItem = GetShapeResult(aSt402->Entity(anIndx));
      }
      else
      {
        try
        {
          OCC_CATCH_SIGNALS
          anItem = TransferGeometry (aSt402->Entity(anIndx), aRange);
        }
        catch(Standard_Failure const&)
        {
          anItem.Nullify();
          const Message_Msg aMsg1015("IGES_1015");
          SendFail(aSt402->Entity(anIndx), aMsg1015);
        }
      }
      if (anItem.IsNull())
      {
        aProblemInGroup = Standard_True;
      }
      else
      {
        aBuilder.Add(aGroup, anItem);
        SetShapeResult (aSt402->Entity(anIndx), anItem);
      }
    }
    aRes = aGroup;
    if(aProblemInGroup)
    {
      Message_Msg aMsg1030("IGES_1030");
      aMsg1030.Arg(aSt402->FormNumber());
      SendWarning (aSt402, aMsg1030);
    }
  }
  else
  {
    Message_Msg aMsg1001("IGES_1001");
    aMsg1001.Arg(theStart->FormNumber());
    SendFail (theStart, aMsg1001);
    return aRes;
  }

  if (theStart->HasTransf())
  {
    gp_Trsf aT;
    SetEpsilon(1.E-04);
    if (IGESData_ToolLocation::ConvertLocation(GetEpsilon(), theStart->CompoundLocation(),
                                               aT, GetUnitFactor()))
    {
      if (theStart->IsKind(STANDARD_TYPE(IGESBasic_SingularSubfigure)))
      {
        gp_XYZ aTra = aT.TranslationPart();
        const gp_XYZ aTrans = aT408.TranslationPart();
        aTra.Add(aTrans);
        aT.SetTranslationPart(aTra);
        Standard_Real aSc = aT.ScaleFactor();
        const Standard_Real aScalef = aT408.ScaleFactor();
        aSc = aSc*aScalef;
        aT.SetScaleFactor(aSc);
      }
      const TopLoc_Location aLoc(aT);
      aRes.Move(aLoc, Standard_False);
    }
    else
    {
      const Message_Msg aMsg1035("IGES_1035");
      SendWarning (theStart, aMsg1035);
    }
  }
  else
  {
    if (theStart->IsKind(STANDARD_TYPE(IGESBasic_SingularSubfigure)))
    {
      const TopLoc_Location aLoc(aT408);
      aRes.Move(aLoc);
    }
  }
  return aRes;
}


//=======================================================================
//function : HasShapeResult
//purpose  : 
//=======================================================================

Standard_Boolean IGESToBRep_CurveAndSurface::HasShapeResult 
  (const Handle(IGESData_IGESEntity)& start) const
{
  DeclareAndCast(TransferBRep_ShapeBinder,binder,myTP->Find(start));
  if (binder.IsNull()) return Standard_False;
  return binder->HasResult();
}


//=======================================================================
//function : GetShapeResult
//purpose  : 
//=======================================================================

TopoDS_Shape IGESToBRep_CurveAndSurface::GetShapeResult 
  (const Handle(IGESData_IGESEntity)& start) const
{
  TopoDS_Shape res;

  DeclareAndCast(TransferBRep_ShapeBinder, binder, myTP->Find(start));
  if (binder.IsNull()) return res;
  if (binder->HasResult())
    res = binder->Result();
  return res;
}


//=======================================================================
//function : SetShapeResult
//purpose  : 
//=======================================================================

void IGESToBRep_CurveAndSurface::SetShapeResult 
  (const Handle(IGESData_IGESEntity)& start,
   const TopoDS_Shape& result)
{
  Handle(TransferBRep_ShapeBinder) binder = new TransferBRep_ShapeBinder;
  myTP->Bind(start,binder);
  binder->SetResult(result);
}

//=======================================================================
//function : NbShapeResult
//purpose  : 
//=======================================================================

Standard_Integer IGESToBRep_CurveAndSurface::NbShapeResult 
  (const Handle(IGESData_IGESEntity)& start) const
{
  Standard_Integer nbres = 0;
  DeclareAndCast(TransferBRep_ShapeListBinder,binder,myTP->Find(start));
  if (binder.IsNull()) return nbres;
  nbres = binder->NbShapes();
  return nbres;
}


//=======================================================================
//function : GetShapeResult
//purpose  : 
//=======================================================================

TopoDS_Shape IGESToBRep_CurveAndSurface::GetShapeResult 
  (const Handle(IGESData_IGESEntity)& start, const Standard_Integer num) const
{
  TopoDS_Shape res;

  DeclareAndCast(TransferBRep_ShapeListBinder,binder,myTP->Find(start));
  if (binder.IsNull()) return res;
  
  if (num <= binder->NbShapes()) 
    res = binder->Shape(num);
  return res;
}


//=======================================================================
//function : AddShapeResult
//purpose  : 
//=======================================================================

void IGESToBRep_CurveAndSurface::AddShapeResult 
  (const Handle(IGESData_IGESEntity)& start,
   const TopoDS_Shape& result)
{
  DeclareAndCast(TransferBRep_ShapeListBinder,binder,myTP->Find(start));
  if (binder.IsNull()){
    binder = new TransferBRep_ShapeListBinder;
    myTP->Bind(start,binder);
  }
  binder->AddResult(result);
}

void IGESToBRep_CurveAndSurface::SetSurface(const Handle(Geom_Surface)& theSurface)
{
  if(mySurface!=theSurface) {
    mySurface = theSurface;
    myIsResolCom = Standard_False;
    myUVResolution = 0.;
  }
}

Handle(Geom_Surface) IGESToBRep_CurveAndSurface::Surface() const 
{
  return mySurface;
}

Standard_Real IGESToBRep_CurveAndSurface::GetUVResolution()
{
  if(!myIsResolCom && !mySurface.IsNull()) {
    myIsResolCom = Standard_True;
    GeomAdaptor_Surface aGAS(mySurface);
    myUVResolution = Min(aGAS.UResolution(1.), aGAS.VResolution(1.));
  }
  return myUVResolution;
}






