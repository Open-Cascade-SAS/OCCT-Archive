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
  TopoDS_Shape res;
  gp_Trsf T408;
  if (theStart.IsNull())
  {
    Message_Msg msg1005("IGES_1005");  //  Software error :  start IsNull.
    SendFail(theStart, msg1005);
    return res;
  }

  // sln 13.06.2002 OCC448: Avoid transferring invisible sub entities which
  // logically depend on the one
  const Standard_Integer anOnlyvisible = Interface_Static::IVal("read.iges.onlyvisible");

  if (IGESToBRep::IsCurveAndSurface(theStart))
  {
    if(anOnlyvisible && theStart->BlankStatus() == 1)
      return res;
    try
    {
      OCC_CATCH_SIGNALS
      res = TransferCurveAndSurface(theStart, theProgress);
    }
    catch(Standard_Failure const&)
    {
      Message_Msg msg1015("IGES_1015");
      SendFail(theStart, msg1015);
    }
    return res;
  }

  //408 : SingularSubfigure
  if (theStart->IsKind(STANDARD_TYPE(IGESBasic_SingularSubfigure)))
    {
      if(anOnlyvisible && theStart->BlankStatus() == 1)
        return res;

      DeclareAndCast(IGESBasic_SingularSubfigure, st408, theStart);
      Handle (IGESBasic_SubfigureDef) stsub = st408->Subfigure();
      const gp_XYZ trans = st408->Translation();
      gp_Vec vectr(trans);
      const Standard_Real scunit = GetUnitFactor();
      vectr.Multiply(scunit);
      T408.SetTranslation(vectr);
      if (st408->HasScaleFactor())
      {
        const Standard_Real scalef = st408->ScaleFactor();
        T408.SetScaleFactor(scalef);
      }
      if (HasShapeResult(stsub))
      {
        res = GetShapeResult(stsub);
      }
      else
      {
        try
        {
          OCC_CATCH_SIGNALS
          res = TransferGeometry(stsub, theProgress);
        }
        catch(Standard_Failure const&)
        {
          res.Nullify();
          const Message_Msg msg1015("IGES_1015");
          SendFail(st408, msg1015);
        }
        if (!res.IsNull())
        {
          SetShapeResult(stsub, res);
        }
      }
    }

  // 308 : SubfigureDefinition
  else if (theStart->IsKind(STANDARD_TYPE(IGESBasic_SubfigureDef)))
  {
    DeclareAndCast(IGESBasic_SubfigureDef, st308, theStart);
    TopoDS_Compound aGroup;
    BRep_Builder aBuilder;
    aBuilder.MakeCompound (aGroup);
    if (st308->NbEntities() < 1)
    {
      const Message_Msg msg210 ("XSTEP_210");
      SendFail(st308, msg210);
      return res;
    }
    Message_ProgressScope aPS (theProgress, "Subfigure item", st308->NbEntities());
    for (Standard_Integer anIndx =1; anIndx <= st308->NbEntities() && aPS.More(); anIndx++)
    {
      Message_ProgressRange aRange = aPS.Next();
      TopoDS_Shape anItem;
      if (st308->AssociatedEntity(anIndx).IsNull())
      {
        Message_Msg msg1020("IGES_1020");
        msg1020.Arg(anIndx);
        SendWarning(st308, msg1020);
        continue;
      }
      if(anOnlyvisible && st308->AssociatedEntity(anIndx)->BlankStatus() == 1)
        continue;

      if (HasShapeResult(st308->AssociatedEntity(anIndx)))
      {
        anItem = GetShapeResult(st308->AssociatedEntity(anIndx));
      }
      else
      {
        try
        {
          OCC_CATCH_SIGNALS
          anItem = TransferGeometry (st308->AssociatedEntity(anIndx), aRange);
        }
        catch(Standard_Failure const&)
        {
          anItem.Nullify();
          const Message_Msg msg1015("IGES_1015");
          SendFail(st308->AssociatedEntity(anIndx), msg1015);
        }
      }
      if (anItem.IsNull())
      {
        Message_Msg msg1025("IGES_1025");
        msg1025.Arg(anIndx);
        SendWarning (theStart, msg1025);
      }
      else
      {
        aBuilder.Add(aGroup, anItem);
        SetShapeResult (st308->AssociatedEntity(anIndx), anItem);
      }
    }
    res = aGroup;
  }
  // 402 : Group Associativity
  else if (theStart->IsKind(STANDARD_TYPE(IGESBasic_Group)))
  {
    if(anOnlyvisible && theStart->BlankStatus() == 1)
      return res;

    DeclareAndCast(IGESBasic_Group, st402f1, theStart);
    TopoDS_Compound aGroup;
    BRep_Builder aBuilder;
    aBuilder.MakeCompound (aGroup);
    const Standard_Integer aNbEnt = st402f1->NbEntities();
    if (aNbEnt < 1)
    {
      Message_Msg msg202 ("XSTEP_202");
      msg202.Arg(st402f1->FormNumber());
      SendFail(st402f1, msg202);
      return res;
    }
    Message_ProgressScope aPS (theProgress, "Group item", aNbEnt);
    Standard_Boolean ProblemInGroup = Standard_False;
    for (Standard_Integer anIndx=1; anIndx <= aNbEnt && aPS.More(); anIndx++)
    {
      Message_ProgressRange aRange = aPS.Next();
      TopoDS_Shape anItem;
      if (st402f1->Entity(anIndx).IsNull())
      {
        Message_Msg msg1020("IGES_1020");
        msg1020.Arg(anIndx);
        SendFail(st402f1, msg1020);
        continue;
      }

      if(anOnlyvisible && st402f1->Entity(anIndx)->BlankStatus() == 1)
        continue;

      if (HasShapeResult(st402f1->Entity(anIndx)))
      {
        anItem = GetShapeResult(st402f1->Entity(anIndx));
      }
      else
      {
        try
        {
          OCC_CATCH_SIGNALS
          anItem = TransferGeometry (st402f1->Entity(anIndx), aRange);
        }
        catch(Standard_Failure const&)
        {
          anItem.Nullify();
          Message_Msg msg1015("IGES_1015");
          SendFail(st402f1->Entity(anIndx), msg1015);
        }
      }
      if (anItem.IsNull())
      {
        ProblemInGroup = Standard_True;
      }
      else
      {
        aBuilder.Add(aGroup, anItem);
        SetShapeResult (st402f1->Entity(anIndx), anItem);
      }
    }
    res = aGroup;
    if(ProblemInGroup)
    {
      Message_Msg msg1030("IGES_1030");
      msg1030.Arg(st402f1->FormNumber());
      SendWarning (st402f1, msg1030);
    }
  }
  else
  {
    Message_Msg msg1001("IGES_1001");
    msg1001.Arg(theStart->FormNumber());
    SendFail (theStart, msg1001);
    return res;
  }

  if (theStart->HasTransf())
  {
    gp_Trsf T;
    SetEpsilon(1.E-04);
    if (IGESData_ToolLocation::ConvertLocation(GetEpsilon(), theStart->CompoundLocation(),
                                               T, GetUnitFactor()))
    {
      if (theStart->IsKind(STANDARD_TYPE(IGESBasic_SingularSubfigure)))
      {
        gp_XYZ tra = T.TranslationPart();
        const gp_XYZ trans = T408.TranslationPart();
        tra.Add(trans);
        T.SetTranslationPart(tra);
        Standard_Real sc = T.ScaleFactor();
        const Standard_Real scalef = T408.ScaleFactor();
        sc = sc*scalef;
        T.SetScaleFactor(sc);
      }
      TopLoc_Location L(T);
      res.Move(L, Standard_False);
    }
    else
    {
      Message_Msg msg1035("IGES_1035");
      SendWarning (theStart, msg1035);
    }
  }
  else
  {
    if (theStart->IsKind(STANDARD_TYPE(IGESBasic_SingularSubfigure)))
    {
      TopLoc_Location L(T408);
      res.Move(L);
    }
  }
  return res;
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






