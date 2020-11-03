// Created on: 1993-08-12
// Created by: Bruno DUMORTIER
// Copyright (c) 1993-1999 Matra Datavision
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


#include <Draw.hxx>
#include <Draw_Appli.hxx>
#include <Draw_Color.hxx>
#include <DrawTrSurf.hxx>
#include <DrawTrSurf_BezierCurve.hxx>
#include <DrawTrSurf_BezierCurve2d.hxx>
#include <DrawTrSurf_BezierSurface.hxx>
#include <DrawTrSurf_BSplineCurve.hxx>
#include <DrawTrSurf_BSplineCurve2d.hxx>
#include <DrawTrSurf_BSplineSurface.hxx>
#include <DrawTrSurf_Curve.hxx>
#include <DrawTrSurf_Curve2d.hxx>
#include <DrawTrSurf_Drawable.hxx>
#include <DrawTrSurf_Point.hxx>
#include <DrawTrSurf_Polygon2D.hxx>
#include <DrawTrSurf_Polygon3D.hxx>
#include <DrawTrSurf_Surface.hxx>
#include <Draw_SaveAndRestore.hxx>
#include <DrawTrSurf_Triangulation.hxx>
#include <Geom2d_BezierCurve.hxx>
#include <Geom2d_BSplineCurve.hxx>
#include <Geom2d_Curve.hxx>
#include <Geom_BezierCurve.hxx>
#include <Geom_BezierSurface.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Geom_BSplineSurface.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Geometry.hxx>
#include <Geom_Surface.hxx>
#include <GeomTools_Curve2dSet.hxx>
#include <GeomTools_CurveSet.hxx>
#include <GeomTools_SurfaceSet.hxx>
#include <gp_Ax2.hxx>
#include <gp_Ax2d.hxx>
#include <gp_Pln.hxx>
#include <gp_Pnt.hxx>
#include <gp_Pnt2d.hxx>
#include <gp_Trsf2d.hxx>
#include <Poly.hxx>
#include <Poly_Polygon2D.hxx>
#include <Poly_Polygon3D.hxx>
#include <Poly_Triangulation.hxx>
#include <Standard_Stream.hxx>
#include <TCollection_AsciiString.hxx>

static Draw_Color       PntColor(Draw_rouge);
static Draw_Color       CurvColor(Draw_jaune);
static Draw_Color       BoundsColor(Draw_vert);
static Draw_Color       IsosColor(Draw_bleu);
static Draw_Color       PolesColor(Draw_rouge);
static Draw_Color       KnotsColor(Draw_violet);

static Draw_MarkerShape PntShape   = Draw_Plus;
static Draw_MarkerShape KnotsShape = Draw_Losange;
static Standard_Boolean ShowPoles  = Standard_True;
static Standard_Boolean ShowKnots  = Standard_True;
static Standard_Boolean knotsIsos  =Standard_True;
static Standard_Real    Deflection = 0.01;
static Standard_Integer KnotsSize  = 5;
static Standard_Integer Discret    = 30;
static Standard_Integer DrawMode   = 0;
static Standard_Integer NbUIsos    = 10;
static Standard_Integer NbVIsos    = 10;

static TCollection_AsciiString ColorsHint(
"The possible colors are: \n\
  white, red, green, blue, cyan,\n\
  golden, magenta, brown, orange, pink,\n\
  salmon, violet, yellow, darkgreen, coral");

static TCollection_AsciiString MarkersHint(
"The possible markers are: \n\
  square, diamond, x, plus, circle, circle_zoom");

//=======================================================================
//function : DrawTrSurf_CurveColor
//purpose  : Sets new color for rendering of curves. Returns the
//           previous one to keep possibility to restore the initial
//           state
//=======================================================================

Standard_EXPORT Draw_Color DrawTrSurf_CurveColor(const Draw_Color col)
{
  Draw_Color c = CurvColor;
  CurvColor = col;
  return c;
}

//=======================================================================
//function : DrawTrSurf_PointColor
//purpose  : Sets new color for rendering of points. Returns the
//           previous one to keep possibility to restore the initial
//           state
//=======================================================================

Standard_EXPORT Draw_Color DrawTrSurf_PointColor(const Draw_Color col)
{
  Draw_Color c = PntColor;
  PntColor = col;
  return c;
}

//=======================================================================
//function : DrawTrSurf_PointMarker
//purpose  : Sets new marker for rendering of points. Returns the
//           previous one to keep possibility to restore the initial
//           state
//=======================================================================

Standard_EXPORT Draw_MarkerShape DrawTrSurf_PointMarker(const Draw_MarkerShape marker)
{
  Draw_MarkerShape prev = PntShape;
  PntShape = marker;
  return prev;
}

//=======================================================================
//function : GetDrawable
//purpose  : 
//=======================================================================
static Handle(DrawTrSurf_Drawable) GetDrawable(Standard_CString& Name)
{
  Handle(Draw_Drawable3D) D = Draw::Get(Name);
  Handle(DrawTrSurf_Drawable) Dr;
  if (!D.IsNull()) 
    Dr = Handle(DrawTrSurf_Drawable)::DownCast(D);
  return Dr;
}

//=======================================================================
//function : GetSurface
//purpose  : 
//=======================================================================
static Handle(DrawTrSurf_Surface) GetSurface(Standard_CString& Name)
{
  Handle(Draw_Drawable3D) D = Draw::Get(Name);
  Handle(DrawTrSurf_Surface) Dr;
  if (!D.IsNull()) 
    Dr = Handle(DrawTrSurf_Surface)::DownCast(D);
  return Dr;
}

//=======================================================================
//function : GetBezierSurface
//purpose  : 
//=======================================================================
static Handle(DrawTrSurf_BezierSurface) GetBezierSurface(Standard_CString& Name)
{
  Handle(Draw_Drawable3D) D = Draw::Get(Name);
  Handle(DrawTrSurf_BezierSurface) Dr;
  if (!D.IsNull()) 
    Dr = Handle(DrawTrSurf_BezierSurface)::DownCast(D);
  return Dr;
}

//=======================================================================
//function : GetBSplineSurface
//purpose  : 
//=======================================================================
static Handle(DrawTrSurf_BSplineSurface) GetBSplineSurface(Standard_CString& Name)
{
  Handle(Draw_Drawable3D) D = Draw::Get(Name);
  Handle(DrawTrSurf_BSplineSurface) Dr;
  if (!D.IsNull()) 
    Dr = Handle(DrawTrSurf_BSplineSurface)::DownCast(D);
  return Dr;
}

//=======================================================================
//function : GetBezierCurve
//purpose  : 
//=======================================================================
static Handle(DrawTrSurf_BezierCurve) GetBezierCurve(Standard_CString& Name)
{
  Handle(Draw_Drawable3D) D = Draw::Get(Name);
  Handle(DrawTrSurf_BezierCurve) Dr;
  if (!D.IsNull()) 
    Dr = Handle(DrawTrSurf_BezierCurve)::DownCast(D);
  return Dr;
}

//=======================================================================
//function : GetBSplineCurve
//purpose  : 
//=======================================================================
static Handle(DrawTrSurf_BSplineCurve) GetBSplineCurve(Standard_CString& Name)
{
  Handle(Draw_Drawable3D) D = Draw::Get(Name);
  Handle(DrawTrSurf_BSplineCurve) Dr;
  if (!D.IsNull()) 
    Dr = Handle(DrawTrSurf_BSplineCurve)::DownCast(D);
  return Dr;
}

//=======================================================================
//function : GetBezierCurve2d
//purpose  : 
//=======================================================================
static Handle(DrawTrSurf_BezierCurve2d) GetBezierCurve2d(Standard_CString& Name)
{
  Handle(Draw_Drawable3D) D = Draw::Get(Name);
  Handle(DrawTrSurf_BezierCurve2d) Dr;
  if (!D.IsNull()) 
    Dr = Handle(DrawTrSurf_BezierCurve2d)::DownCast(D);
  return Dr;
}

//=======================================================================
//function : GetBSplineCurve2d
//purpose  : 
//=======================================================================
static Handle(DrawTrSurf_BSplineCurve2d) GetBSplineCurve2d(Standard_CString& Name)
{
  Handle(Draw_Drawable3D) D = Draw::Get(Name);
  Handle(DrawTrSurf_BSplineCurve2d) Dr;
  if (!D.IsNull()) 
    Dr = Handle(DrawTrSurf_BSplineCurve2d)::DownCast(D);
  return Dr;
}


//=======================================================================
//function : nbiso
//purpose  : 
//=======================================================================
static Standard_Integer nbiso (Draw_Interpretor& di, Standard_Integer n, const char** a)
{
  if (n < 4) {
    if (n == 3) {
      NbUIsos = Draw::Atoi(a[1]);
      NbVIsos = Draw::Atoi(a[2]);
    }
    di << NbUIsos << " " << NbVIsos;
  }
  else {
    for (Standard_Integer i = 1; i < n - 2; i++) {
      Handle(DrawTrSurf_Surface) DS = GetSurface(a[i]);
      if (!DS.IsNull()) {
	DS->ShowIsos(Draw::Atoi(a[n-2]),Draw::Atoi(a[n-1]));
	Draw::Repaint();
      }
    }
  }
  return 0;
}

//=======================================================================
//function : drawpoles
//purpose  : 
//=======================================================================

static Standard_Integer drawpoles(Draw_Interpretor&, 
				  Standard_Integer n,
				  const char** a)
{
  if ( n == 1) {
    if ( !strcmp(a[0],"shpoles")) {
      ShowPoles = Standard_True;
    }
    else if ( !strcmp(a[0],"clpoles")) {
      ShowPoles = Standard_False;
    }
    else if ( !strcmp(a[0],"shknots")) {
      ShowKnots = Standard_True;
    }
    else if ( !strcmp(a[0],"clknots")) {
      ShowKnots = Standard_False;
    }
  }

  if ( n<2) return 0;

  Handle(DrawTrSurf_BezierSurface) BZS;
  BZS = GetBezierSurface(a[1]);
  if ( !BZS.IsNull()) {
    if ( !strcmp(a[0],"shpoles")) {
      BZS->ShowPoles();
    }
    else if ( !strcmp(a[0],"clpoles")) {
      BZS->ClearPoles();
    }
    else {
      return 1;
    }
  }
  else {
    Handle(DrawTrSurf_BSplineSurface) BSS = GetBSplineSurface(a[1]);
    if ( !BSS.IsNull()) {
      if ( !strcmp(a[0],"shpoles")) {
	BSS->ShowPoles();
      }
      else if ( !strcmp(a[0],"clpoles")) {
	BSS->ClearPoles();
      }
      else if ( !strcmp(a[0],"shknots")) {
	BSS->ShowKnots();
      }
      else if ( !strcmp(a[0],"clknots")) {
	BSS->ClearKnots();
      }
    }
    else {
      Handle(DrawTrSurf_BezierCurve) BZC = GetBezierCurve(a[1]);
      if ( !BZC.IsNull()) {
	if ( !strcmp(a[0],"shpoles")) {
	  BZC->ShowPoles();
	}
	else if ( !strcmp(a[0],"clpoles")) {
	  BZC->ClearPoles();
	}
	else {
	  return 1;
	}
      }
      else {
	Handle(DrawTrSurf_BSplineCurve) BSC = GetBSplineCurve(a[1]);
	if ( !BSC.IsNull()) {
	  if ( !strcmp(a[0],"shpoles")) {
	    BSC->ShowPoles();
	  }
	  else if ( !strcmp(a[0],"clpoles")) {
	    BSC->ClearPoles();
	  }
	  else if ( !strcmp(a[0],"shknots")) {
	    BSC->ShowKnots();
	  }
	  else if ( !strcmp(a[0],"clknots")) {
	    BSC->ClearKnots();
	  }
	}
	else {
	  Handle(DrawTrSurf_BezierCurve2d) BZ2 = GetBezierCurve2d(a[1]);
	  if ( !BZ2.IsNull()) {
	    if ( !strcmp(a[0],"shpoles")) {
	      BZ2->ShowPoles();
	    }
	    else if ( !strcmp(a[0],"clpoles")) {
	      BZ2->ClearPoles();
	    }
	    else {
	      return 1;
	    }
	  }
	  else {
	    Handle(DrawTrSurf_BSplineCurve2d) BS2 = GetBSplineCurve2d(a[1]);
	    if ( !BS2.IsNull()) {
	      if ( !strcmp(a[0],"shpoles")) {
		BS2->ShowPoles();
	      }
	      else if ( !strcmp(a[0],"clpoles")) {
		BS2->ClearPoles();
	      }
	      else if ( !strcmp(a[0],"shknots")) {
		BS2->ShowKnots();
	      }
	      else if ( !strcmp(a[0],"clknots")) {
		BS2->ClearKnots();
	      }
	    }
	    else {
	      return 1;
	    }
	  }
	}
      }
    }
  }
  Draw::Repaint();
  return 0;
}

//=======================================================================
//function : draw
//purpose  : 
//=======================================================================

static Standard_Integer draw (Draw_Interpretor& di, Standard_Integer n, const char** a)
{
  if (n <= 2) {
	if (!strcmp(a[0],"dmode")) {
	  if (n == 2) {
	    DrawMode = 1;
	  }
	  if (DrawMode)
	    di << "u";
	  else
	    di << "d";
	}
	
	else if (!strcmp(a[0],"discr")) {
	  if (n == 2)
	    Discret = Draw::Atoi(a[n-1]);
	  di << Discret;
	}
	
	else if (!strcmp(a[0],"defle")) {
	  if (n == 2)
	    Deflection = Draw::Atof(a[n-1]);
	  di << Deflection;
	}
  }
  else {
    for (Standard_Integer i = 1; i < n - 1; i++) {
      Handle(DrawTrSurf_Drawable) D = GetDrawable(a[1]);
      if (!D.IsNull()) {
	if (!strcmp(a[0],"dmode")) {
	  Standard_Integer mod = 0;
	  if ((*a[n-1] == 'U')||(*a[n-1] == 'u')) mod = 1;
	  D->SetDrawMode(mod);
	}
	
	else if (!strcmp(a[0],"discr")) {
	  D->SetDiscretisation(Draw::Atoi(a[n-1]));
	}
	
	else if (!strcmp(a[0],"defle")) {
	  D->SetDeflection(Draw::Atof(a[n-1]));
	}
	
	Draw::Repaint();
      }
    }
  }
  return 0;
}

//=======================================================================
//function : transform
//purpose  : 
//=======================================================================

static Standard_Integer transform (Draw_Interpretor& di, Standard_Integer n, const char** a)
{
  if (n < 5) return 1;
  gp_Trsf T;
  Standard_Integer i,last = n-1;
  if (!strcmp(a[0],"pscale")) {
    Standard_Real s = Draw::Atof(a[last]);
    last--;
    if (last < 4) return 1;
    gp_Pnt P(Draw::Atof(a[last-2]),Draw::Atof(a[last-1]),Draw::Atof(a[last]));
    T.SetScale(P,s);
  }
  else if (!strcmp(a[0]+1,"mirror")) {
    if (last < 4) return 1;
    gp_Pnt P(Draw::Atof(a[last-2]),Draw::Atof(a[last-1]),Draw::Atof(a[last]));
    if (*a[0] == 'p') {
      T.SetMirror(P);
    }
    else {
      last -= 3;
      if (last < 4) return 1;
      gp_Pnt O(Draw::Atof(a[last-2]),Draw::Atof(a[last-1]),Draw::Atof(a[last]));
      last -= 3;
      gp_Dir D(P.X(),P.Y(),P.Z());
      if (*a[0] == 'l') {
	T.SetMirror(gp_Ax1(O,D));
      }
      else if (*a[0] == 's') {
	gp_Pln Pl(O,D);
	T.SetMirror(Pl.Position().Ax2());
      }
    }
  }

  else if (!strcmp(a[0],"translate")) {
    if (last < 4) return 1;
    gp_Vec V(Draw::Atof(a[last-2]),Draw::Atof(a[last-1]),Draw::Atof(a[last]));
    last -= 3;
    T.SetTranslation(V);
  }

  else if (!strcmp(a[0],"rotate")) {
    if (last < 8) return 1;
    Standard_Real ang = Draw::Atof(a[last]) * (M_PI / 180.0);
    last --;
    gp_Dir D(Draw::Atof(a[last-2]),Draw::Atof(a[last-1]),Draw::Atof(a[last]));
    last -= 3;
    gp_Pnt P(Draw::Atof(a[last-2]),Draw::Atof(a[last-1]),Draw::Atof(a[last]));
    last -= 3;
    T.SetRotation(gp_Ax1(P,D),ang);
  }

  for (i = 1; i <= last; i++) {
    Handle(Geom_Geometry) G = DrawTrSurf::Get(a[i]);
    if (!G.IsNull()) {
      G->Transform(T);
      Draw::Repaint();
    }
    else {
      gp_Pnt P;
      if (DrawTrSurf::GetPoint(a[i],P)) {
	P.Transform(T);
	DrawTrSurf::Set(a[i],P);
      }
    }
    di << a[i] << " ";
  }
  return 0;
}

//=======================================================================
//function : d2transform
//purpose  : 
//=======================================================================

static Standard_Integer d2transform (Draw_Interpretor& di, Standard_Integer n, const char** a)
{
  if (n < 4) return 1;
  gp_Trsf2d T;
  Standard_Integer i,last = n-1;
  if (!strcmp(a[0],"2dpscale")) {
    Standard_Real s = Draw::Atof(a[last]);
    last--;
    if (last < 3) return 1;
    gp_Pnt2d P(Draw::Atof(a[last-1]),Draw::Atof(a[last]));
    T.SetScale(P,s);
  }
  else if ( (!strcmp(a[0],"2dpmirror")) || 
	    (!strcmp(a[0],"2dlmirror"))   ) {
    if (last < 3) return 1;
    gp_Pnt2d P(Draw::Atof(a[last-1]),Draw::Atof(a[last]));
    if (!strcmp(a[0],"2dpmirror")) {
      T.SetMirror(P);
    }
    else {
      last -= 2;
      if (last < 3) return 1;
      gp_Pnt2d O(Draw::Atof(a[last-1]),Draw::Atof(a[last]));
      last -= 2;
      gp_Dir2d D(P.X(),P.Y());
      T.SetMirror(gp_Ax2d(O,D));
    }
  }

  else if (!strcmp(a[0],"2dtranslate")) {
    if (last < 3) return 1;
    gp_Vec2d V(Draw::Atof(a[last-1]),Draw::Atof(a[last]));
    last -= 2;
    T.SetTranslation(V);
  }

  else if (!strcmp(a[0],"2drotate")) {
    if (last < 4) return 1;
    Standard_Real ang = Draw::Atof(a[last]) * (M_PI / 180.0);
    last --;
    gp_Pnt2d P(Draw::Atof(a[last-1]),Draw::Atof(a[last]));
    last -= 2;
    T.SetRotation(P,ang);
  }

  for (i = 1; i <= last; i++) {
    Handle(Geom2d_Curve) G = DrawTrSurf::GetCurve2d(a[i]);
    if (!G.IsNull()) {
      G->Transform(T);
      Draw::Repaint();
    }
    else {
      gp_Pnt2d P;
      if (DrawTrSurf::GetPoint2d(a[i],P)) {
	P.Transform(T);
	DrawTrSurf::Set(a[i],P);
      }
    }
    di << a[i] << " ";
  }
  return 0;
}

//=======================================================================
//function : Set
//purpose  : point
//=======================================================================
void  DrawTrSurf::Set(const Standard_CString Name, 
		      const gp_Pnt& P)
{
  Handle(DrawTrSurf_Point) DP = new DrawTrSurf_Point(P,PntShape,PntColor);
 Draw::Set(Name,DP);
}

//=======================================================================
//function : Set
//purpose  : point
//=======================================================================
void  DrawTrSurf::Set(const Standard_CString Name, 
		      const gp_Pnt2d& P)
{
  Handle(DrawTrSurf_Point) DP = new DrawTrSurf_Point(P,PntShape,PntColor);
 Draw::Set(Name,DP);
}

//=======================================================================
//function : Set
//purpose  : Geometry from Geom
//=======================================================================
void  DrawTrSurf::Set(const Standard_CString Name, 
                      const Handle(Geom_Geometry)& G,
                      const Standard_Boolean isSenseMarker)
{
  Handle(DrawTrSurf_Drawable) D;
  if (!G.IsNull()) {

    Handle(Geom_Curve) C = Handle(Geom_Curve)::DownCast(G);
    if (!C.IsNull()) {
      
      Handle(Geom_BezierCurve) Bez = 
	Handle(Geom_BezierCurve)::DownCast(C);
      if (!Bez.IsNull()) {
	Handle(DrawTrSurf_BezierCurve) DBez = 
	  new DrawTrSurf_BezierCurve(Bez,CurvColor,PolesColor,ShowPoles,
				     Discret,Deflection,DrawMode);
	D = DBez;
      }


      Handle(Geom_BSplineCurve) BS = 
	Handle(Geom_BSplineCurve)::DownCast(C);
      if (!BS.IsNull()) {
	Handle(DrawTrSurf_BSplineCurve) DBS = 
	  new DrawTrSurf_BSplineCurve(BS,
				      CurvColor,PolesColor,
				      KnotsColor,
				      KnotsShape,KnotsSize,
				      ShowPoles,ShowKnots,
				      Discret,Deflection,DrawMode);
	D = DBS;
      }

      if (Bez.IsNull() && BS.IsNull()) {
	Handle(DrawTrSurf_Curve) DC =
	  new DrawTrSurf_Curve(C,CurvColor,Discret,Deflection,DrawMode,isSenseMarker);
	D = DC;
      }
    }


    Handle(Geom_Surface) S = Handle(Geom_Surface)::DownCast(G);
    if (!S.IsNull()) {
      
      Handle(Geom_BezierSurface) Bez = 
	Handle(Geom_BezierSurface)::DownCast(S);
      if (!Bez.IsNull()) {
	Handle(DrawTrSurf_BezierSurface) DBez = 
	  new DrawTrSurf_BezierSurface(Bez,NbUIsos,NbVIsos,
				       BoundsColor,IsosColor,PolesColor,
				       ShowPoles,
				       Discret,Deflection,DrawMode);
	D = DBez;
      }


      Handle(Geom_BSplineSurface) BS = 
	Handle(Geom_BSplineSurface)::DownCast(S);
      if (!BS.IsNull()) {
	Handle(DrawTrSurf_BSplineSurface) DBS ;
	if (!knotsIsos) 
	  DBS = new DrawTrSurf_BSplineSurface(BS,
					      NbUIsos,NbVIsos,
					      BoundsColor,IsosColor,
					      PolesColor,KnotsColor,
					      KnotsShape,KnotsSize,
					      ShowPoles,ShowKnots,
					      Discret,Deflection,DrawMode);
	else
	  DBS = new DrawTrSurf_BSplineSurface(BS,
					      BoundsColor,IsosColor,
					      PolesColor,KnotsColor,
					      KnotsShape,KnotsSize,
					      ShowPoles,ShowKnots,
					      Discret,Deflection,DrawMode);
	
	D = DBS;
      }

      if (Bez.IsNull() && BS.IsNull()) {
	Handle(DrawTrSurf_Surface) DS =
	  new DrawTrSurf_Surface(S,
				 NbUIsos,NbVIsos,
				 BoundsColor,IsosColor,
				 Discret,Deflection,DrawMode);
	D = DS;
      }
    }

  }
  Draw::Set(Name,D);
}


//=======================================================================
//function : Set
//purpose  : Curve from Geom2d
//=======================================================================
void  DrawTrSurf::Set(const Standard_CString Name, 
                      const Handle(Geom2d_Curve)& C,
                      const Standard_Boolean isSenseMarker)
{
  Handle(DrawTrSurf_Drawable) D;
  if (!C.IsNull()) {

      Handle(Geom2d_BezierCurve) Bez = 
	Handle(Geom2d_BezierCurve)::DownCast(C);
      if (!Bez.IsNull()) {
	Handle(DrawTrSurf_BezierCurve2d) DBez = 
	  new DrawTrSurf_BezierCurve2d(Bez,CurvColor,PolesColor,ShowPoles,
				       Discret);
	D = DBez;
      }


      Handle(Geom2d_BSplineCurve) BS = 
	Handle(Geom2d_BSplineCurve)::DownCast(C);
      if (!BS.IsNull()) {
	Handle(DrawTrSurf_BSplineCurve2d) DBS = 
	  new DrawTrSurf_BSplineCurve2d(BS,
					CurvColor,PolesColor,
					KnotsColor,
					KnotsShape,KnotsSize,
					ShowPoles,ShowKnots,
					Discret);
	D = DBS;
      }
      
      if (Bez.IsNull() && BS.IsNull()) {
	Handle(DrawTrSurf_Curve2d) DC =
	  new DrawTrSurf_Curve2d(C,CurvColor,Discret,isSenseMarker);
	D = DC;
      }
    }

  Draw::Set(Name,D);
}

//=======================================================================
//function : Set
//purpose  : 
//=======================================================================
void DrawTrSurf::Set(const Standard_CString Name, 
		     const Handle(Poly_Triangulation)& T)
{
  Handle(DrawTrSurf_Triangulation) D = new DrawTrSurf_Triangulation(T);
  Draw::Set(Name,D);
}
//=======================================================================
//function : Set
//purpose  : 
//=======================================================================
void DrawTrSurf::Set(const Standard_CString Name, 
		     const Handle(Poly_Polygon3D)& P)
{
  Handle(DrawTrSurf_Polygon3D) D = new DrawTrSurf_Polygon3D(P);
  Draw::Set(Name,D);
}

//=======================================================================
//function : Set
//purpose  : 
//=======================================================================
void DrawTrSurf::Set(const Standard_CString Name, 
		     const Handle(Poly_Polygon2D)& P)
{
  Handle(DrawTrSurf_Polygon2D) D = new DrawTrSurf_Polygon2D(P);
  Draw::Set(Name,D);
}

//=======================================================================
//function : Get
//purpose  : 
//=======================================================================
Handle(Geom_Geometry)  DrawTrSurf::Get(Standard_CString& Name)
{
  Handle(Draw_Drawable3D) D = Draw::Get(Name);

  Handle(DrawTrSurf_Curve) DC = 
    Handle(DrawTrSurf_Curve)::DownCast(D);
  if (!DC.IsNull())
    return DC->GetCurve();

  Handle(DrawTrSurf_Surface) DS = 
    Handle(DrawTrSurf_Surface)::DownCast(D);
  if (!DS.IsNull())
    return DS->GetSurface();

  return Handle(Geom_Geometry) ();
}


//=======================================================================
//function : GetPoint
//purpose  : 
//=======================================================================
Standard_Boolean DrawTrSurf::GetPoint(Standard_CString& Name,
				      gp_Pnt& P)
{
  Handle(DrawTrSurf_Point) D = 
    Handle(DrawTrSurf_Point)::DownCast(Draw::Get(Name));
  if (D.IsNull())
    return Standard_False;
  else if (!D->Is3D())
    return Standard_False;
  else {
    P = D->Point();
    return Standard_True;
  }
}

//=======================================================================
//function : GetPoint2d
//purpose  : 
//=======================================================================
Standard_Boolean DrawTrSurf::GetPoint2d(Standard_CString& Name,
					gp_Pnt2d& P)
{
  Handle(DrawTrSurf_Point) D = 
    Handle(DrawTrSurf_Point)::DownCast(Draw::Get(Name));
  if (D.IsNull())
    return Standard_False;
  else if (D->Is3D()) 
    return Standard_False;
  else {
    P = D->Point2d();
    return Standard_True;
  }
}

//=======================================================================
//function : GetCurve
//purpose  : 
//=======================================================================
Handle(Geom_Curve)  DrawTrSurf::GetCurve(Standard_CString& Name)
{
  Handle(DrawTrSurf_Curve) D = 
    Handle(DrawTrSurf_Curve)::DownCast(Draw::Get(Name));
  if (D.IsNull())
    return Handle(Geom_Curve)();
  else
    return D->GetCurve();
}


//=======================================================================
//function : GetBezierCurve
//purpose  : 
//=======================================================================
Handle(Geom_BezierCurve)  DrawTrSurf::GetBezierCurve(Standard_CString& Name)
{
  Handle(DrawTrSurf_BezierCurve) D = 
    Handle(DrawTrSurf_BezierCurve)::DownCast(Draw::Get(Name));
  if (D.IsNull())
    return Handle(Geom_BezierCurve)();
  else
    return Handle(Geom_BezierCurve)::DownCast(D->GetCurve());
}


//=======================================================================
//function : GetBSplineCurve
//purpose  : 
//=======================================================================
Handle(Geom_BSplineCurve)  DrawTrSurf::GetBSplineCurve(Standard_CString& Name)
{
  Handle(DrawTrSurf_BSplineCurve) D = 
    Handle(DrawTrSurf_BSplineCurve)::DownCast(Draw::Get(Name));
  if (D.IsNull())
    return Handle(Geom_BSplineCurve)();
  else
    return Handle(Geom_BSplineCurve)::DownCast(D->GetCurve());
}
//=======================================================================
//function : GetCurve2d
//purpose  : 
//=======================================================================
Handle(Geom2d_Curve)  DrawTrSurf::GetCurve2d(Standard_CString& Name)
{
  Handle(DrawTrSurf_Curve2d) D = 
    Handle(DrawTrSurf_Curve2d)::DownCast(Draw::Get(Name));
  if (D.IsNull())
    return Handle(Geom2d_Curve)();
  else
    return D->GetCurve();
}
//=======================================================================
//function : GetBezierCurve2d
//purpose  : 
//=======================================================================
Handle(Geom2d_BezierCurve)  DrawTrSurf::GetBezierCurve2d(Standard_CString& Name)
{
  Handle(DrawTrSurf_Curve2d) D = 
    Handle(DrawTrSurf_Curve2d)::DownCast(Draw::Get(Name));
  if (D.IsNull())
    return Handle(Geom2d_BezierCurve)();
  else
    return Handle(Geom2d_BezierCurve)::DownCast(D->GetCurve());
}
//=======================================================================
//function : GetBSplineCurve2d
//purpose  : 
//=======================================================================
Handle(Geom2d_BSplineCurve)  DrawTrSurf::GetBSplineCurve2d
       (Standard_CString& Name)
{
  Handle(DrawTrSurf_Curve2d) D = 
    Handle(DrawTrSurf_Curve2d)::DownCast(Draw::Get(Name));
  if (D.IsNull())
    return Handle(Geom2d_BSplineCurve)();
  else
    return Handle(Geom2d_BSplineCurve)::DownCast(D->GetCurve());
}
//=======================================================================
//function : GetSurface
//purpose  : 
//=======================================================================
Handle(Geom_Surface)  DrawTrSurf::GetSurface
       (Standard_CString& Name)
{
  Handle(DrawTrSurf_Surface) D = 
    Handle(DrawTrSurf_Surface)::DownCast(Draw::Get(Name));
  if (D.IsNull())
    return Handle(Geom_Surface)();
  else
    return D->GetSurface();
}
//=======================================================================
//function : GetBezierSurface
//purpose  : 
//=======================================================================
Handle(Geom_BezierSurface)  DrawTrSurf::GetBezierSurface
       (Standard_CString& Name)
{
  Handle(DrawTrSurf_BezierSurface) D = 
    Handle(DrawTrSurf_BezierSurface)::DownCast(Draw::Get(Name));
  if (D.IsNull())
    return Handle(Geom_BezierSurface)();
  else
    return Handle(Geom_BezierSurface)::DownCast(D->GetSurface());
}
//=======================================================================
//function : GetBSplineSurface
//purpose  : 
//=======================================================================
Handle(Geom_BSplineSurface) DrawTrSurf::GetBSplineSurface
       (Standard_CString& Name)
{
  Handle(DrawTrSurf_BSplineSurface) D = 
    Handle(DrawTrSurf_BSplineSurface)::DownCast(Draw::Get(Name));
  if (D.IsNull())
    return Handle(Geom_BSplineSurface)();
  else
    return Handle(Geom_BSplineSurface)::DownCast(D->GetSurface());
}
//=======================================================================
//function : GetTriangulation
//purpose  : 
//=======================================================================
Handle(Poly_Triangulation) DrawTrSurf::GetTriangulation(Standard_CString& Name)
{
  Handle(DrawTrSurf_Triangulation) D =
    Handle(DrawTrSurf_Triangulation)::DownCast(Draw::Get(Name));
  if (D.IsNull())
    return Handle(Poly_Triangulation)();
  else
    return D->Triangulation();
}
//=======================================================================
//function : GetPolygon3D
//purpose  : 
//=======================================================================
Handle(Poly_Polygon3D) DrawTrSurf::GetPolygon3D(Standard_CString& Name)
{
  Handle(DrawTrSurf_Polygon3D) D =
    Handle(DrawTrSurf_Polygon3D)::DownCast(Draw::Get(Name));
  if (D.IsNull())
    return Handle(Poly_Polygon3D)();
  else
    return D->Polygon3D();
}
//=======================================================================
//function : GetPolygon2D
//purpose  : 
//=======================================================================
Handle(Poly_Polygon2D) DrawTrSurf::GetPolygon2D(Standard_CString& Name)
{
  Handle(DrawTrSurf_Polygon2D) D =
    Handle(DrawTrSurf_Polygon2D)::DownCast(Draw::Get(Name));
  if (D.IsNull())
    return Handle(Poly_Polygon2D)();
  else
    return D->Polygon2D();
}

//=======================================================================
//function : printColor
//purpose  : 
//=======================================================================

static void printColor(Draw_Interpretor& di, const Draw_Color& theColor)
{
  switch ( theColor.ID() )
  {
    case Draw_blanc:   di << "white "            << "\n"; break;
    case Draw_rouge:   di << "red "              << "\n"; break;
    case Draw_vert:    di << "green "            << "\n"; break;
    case Draw_bleu:    di << "blue "             << "\n"; break;
    case Draw_cyan:    di << "cyan "             << "\n"; break;
    case Draw_or:      di << "golden "           << "\n"; break;
    case Draw_magenta: di << "magenta "          << "\n"; break;
    case Draw_marron:  di << "brown "            << "\n"; break;
    case Draw_orange:  di << "orange "           << "\n"; break;
    case Draw_rose:    di << "pink "             << "\n"; break;
    case Draw_saumon:  di << "salmon "           << "\n"; break;
    case Draw_violet:  di << "violet "           << "\n"; break;
    case Draw_jaune:   di << "yellow "           << "\n"; break;
    case Draw_kaki:    di << "dark-olive green \n"; break;
    case Draw_corail:  di << "coral "            << "\n"; break;
  }
}

//=======================================================================
//function : recognizeColor
//purpose  : 
//=======================================================================

static Draw_Color recognizeColor(const char* theColorStr,
                                 const Draw_Color& theDefaultColor)
{
  Draw_Color aResult = theDefaultColor;

  if ( !strcasecmp(theColorStr,"white") )
    aResult = Draw_blanc;
  if ( !strcasecmp(theColorStr, "red") )
    aResult = Draw_rouge;
  if ( !strcasecmp(theColorStr, "green") )
    aResult = Draw_vert;
  if ( !strcasecmp(theColorStr, "blue") )
    aResult = Draw_bleu;
  if ( !strcasecmp(theColorStr, "cyan") )
    aResult = Draw_cyan;
  if ( !strcasecmp(theColorStr, "golden") )
    aResult = Draw_or;
  if ( !strcasecmp(theColorStr, "magenta") )
    aResult = Draw_magenta;
  if ( !strcasecmp(theColorStr, "brown") )
    aResult = Draw_marron;
  if ( !strcasecmp(theColorStr, "orange") )
    aResult = Draw_orange;
  if ( !strcasecmp(theColorStr, "pink") )
    aResult = Draw_rose;
  if ( !strcasecmp(theColorStr, "salmon") )
    aResult = Draw_saumon;
  if ( !strcasecmp(theColorStr, "violet") )
    aResult = Draw_violet;
  if ( !strcasecmp(theColorStr, "yellow") )
    aResult = Draw_jaune;
  if ( !strcasecmp(theColorStr, "darkgreen") )
    aResult = Draw_kaki;
  if ( !strcasecmp(theColorStr, "coral") )
    aResult = Draw_corail;

  return aResult;
}

//=======================================================================
//function : setcurvcolor
//purpose  : 
//=======================================================================

static Standard_Integer setcurvcolor(Draw_Interpretor& di,
                                     Standard_Integer n, const char** a)
{
  Draw_Color col, savecol;

  savecol = DrawTrSurf_CurveColor(Draw_Color(Draw_jaune));
  DrawTrSurf_CurveColor(savecol);

  if (n < 2)
  {
    printColor(di, savecol);
  }
  else {
    col = recognizeColor(a[1], savecol);
    DrawTrSurf_CurveColor(col);
  }
  return 0;
}

//=======================================================================
//function : changecurvcolor
//purpose  : 
//=======================================================================

static Standard_Integer changecurvcolor(Draw_Interpretor&,
                                        Standard_Integer n, const char** a)
{
  Draw_Color col, savecol;

  savecol = DrawTrSurf_CurveColor(Draw_Color(Draw_jaune));
  DrawTrSurf_CurveColor(savecol);

  if ( n < 3 )
    return 1;

  col = recognizeColor(a[1], savecol);
    
  Handle(DrawTrSurf_Curve) D = Handle(DrawTrSurf_Curve)::DownCast( Draw::Get(a[2]) );
  if ( !D.IsNull() )
  {
    D->SetColor(col);
    Draw::Repaint();
  }

  return 0;
}

//=======================================================================
//function : setpointcolor
//purpose  : 
//=======================================================================

static Standard_Integer setpointcolor(Draw_Interpretor& di,
                                      Standard_Integer n, const char** a)
{
  Draw_Color col, savecol;

  savecol = DrawTrSurf_PointColor(Draw_Color(Draw_jaune));
  DrawTrSurf_PointColor(savecol);

  if (n < 2)
  {
    printColor(di, savecol);
  }
  else {
    col = recognizeColor(a[1], savecol);
    DrawTrSurf_PointColor(col);
  }
  return 0;
}

//=======================================================================
//function : changepointcolor
//purpose  : 
//=======================================================================

static Standard_Integer changepointcolor(Draw_Interpretor&,
                                         Standard_Integer n, const char** a)
{
  Draw_Color col, savecol;

  savecol = DrawTrSurf_PointColor(Draw_Color(Draw_jaune));
  DrawTrSurf_PointColor(savecol);

  if ( n < 3 )
    return 1;

  col = recognizeColor(a[1], savecol);
    
  Handle(DrawTrSurf_Point) D = Handle(DrawTrSurf_Point)::DownCast( Draw::Get(a[2]) );
  if ( !D.IsNull() )
  {
    D->Color(col);
    Draw::Repaint();
  }

  return 0;
}

//=======================================================================
//function : printMarker
//purpose  : 
//=======================================================================

static void printMarker(Draw_Interpretor& di, const Draw_MarkerShape& theMarker)
{
  switch ( theMarker )
  {
    case Draw_Square:     di << "square "      << "\n"; break;
    case Draw_Losange:    di << "diamond "     << "\n"; break;
    case Draw_X:          di << "x "           << "\n"; break;
    case Draw_Plus:       di << "plus "        << "\n"; break;
    case Draw_Circle:     di << "circle "      << "\n"; break;
    case Draw_CircleZoom: di << "circle_zoom \n"; break;
  }
}

//=======================================================================
//function : recognizeMarker
//purpose  : 
//=======================================================================

static Draw_MarkerShape recognizeMarker(const char* theMarkerStr,
                                        const Draw_MarkerShape& theDefaultMarker)
{
  Draw_MarkerShape aResult = theDefaultMarker;

  if ( !strcasecmp(theMarkerStr, "square") )
    aResult = Draw_Square;
  if ( !strcasecmp(theMarkerStr, "diamond") )
    aResult = Draw_Losange;
  if ( !strcasecmp(theMarkerStr, "x") )
    aResult = Draw_X;
  if ( !strcasecmp(theMarkerStr, "plus") )
    aResult = Draw_Plus;
  if ( !strcasecmp(theMarkerStr, "circle") )
    aResult = Draw_Circle;
  if ( !strcasecmp(theMarkerStr, "circle_zoom") )
    aResult = Draw_CircleZoom;

  return aResult;
}

//=======================================================================
//function : setpointmarker
//purpose  : 
//=======================================================================

static Standard_Integer setpointmarker(Draw_Interpretor& di,
                                       Standard_Integer n, const char** a)
{
  Draw_MarkerShape mark, savemark;

  savemark = DrawTrSurf_PointMarker(Draw_MarkerShape(Draw_Plus));
  DrawTrSurf_PointMarker(savemark);

  if ( n < 2 )
  {
    printMarker(di, savemark);
  }
  else {
    mark = recognizeMarker(a[1], savemark);
    DrawTrSurf_PointMarker(mark);
  }
  return 0;
}

//=======================================================================
//function : changepointmarker
//purpose  : 
//=======================================================================

static Standard_Integer changepointmarker(Draw_Interpretor&,
                                          Standard_Integer n, const char** a)
{
  Draw_MarkerShape mark, savemark;

  savemark = DrawTrSurf_PointMarker(Draw_MarkerShape(Draw_Plus));
  DrawTrSurf_PointMarker(savemark);

  if ( n < 3 )
    return 1;

  mark = recognizeMarker(a[1], savemark);
    
  Handle(DrawTrSurf_Point) D = Handle(DrawTrSurf_Point)::DownCast( Draw::Get(a[2]) );
  if ( !D.IsNull() )
  {
    D->Shape(mark);
    Draw::Repaint();
  }

  return 0;
}

//=======================================================================
//function : BasicCommands
//purpose  : 
//=======================================================================

static Standard_Boolean done = Standard_False;
void  DrawTrSurf::BasicCommands(Draw_Interpretor& theCommands)
{
  if (done) return;
  done = Standard_True;

  const char* g;
  g = "geometric display commands";
  
  theCommands.Add("nbiso",
		  "nbiso name [names...] nuiso nviso",
                  __FILE__,
		  nbiso,g);
  
  theCommands.Add("clpoles",
		  "clpoles [name], no args : modal ",
                  __FILE__,
		  drawpoles,g);
  
  theCommands.Add("shpoles",
		  "shpoles [name], no args : modal ",
                  __FILE__,
		  drawpoles,g);
  
  theCommands.Add("clknots",
		  "clknots [name], no args : modal ",
                  __FILE__,
		  drawpoles,g);
  
  theCommands.Add("shknots",
		  "shknots [name], no args : modal ",
                  __FILE__,
		  drawpoles,g);
  
  theCommands.Add("dmode",
		  "dmode [names...] Uniform/Discret",
                  __FILE__,
		  draw,g);

  theCommands.Add("discr",
		  "discr [names...] nbintervals",
                  __FILE__,
		  draw,g);

  theCommands.Add("defle",
		  "defle [names...] defle",
                  __FILE__,
		  draw,g);

  theCommands.Add("setcurvcolor",
                  TCollection_AsciiString("setcurvcolor [color] : set curve color\
 by default, or print the current curve color if no argument (this does not modify\
 the color of the curve)\n\n").Cat(ColorsHint).ToCString(),
  		  __FILE__,setcurvcolor,g);

  theCommands.Add("changecurvcolor",
                  TCollection_AsciiString("changecurvcolor color curve: change\
 color of the curve\n\n").Cat(ColorsHint).ToCString(),
  		  __FILE__,changecurvcolor,g);

  theCommands.Add("setpointcolor",
                  TCollection_AsciiString("setpointcolor [color] : set point color\
 by default, or print the current point color if no argument (this does not modify\
 the color of the point)\n\n").Cat(ColorsHint).ToCString(),
  		  __FILE__,setpointcolor,g);

  theCommands.Add("changepointcolor",
                  TCollection_AsciiString("changepointcolor color point: change\
 color of the point\n\n").Cat(ColorsHint).ToCString(),
  		  __FILE__,changepointcolor,g);
  
  theCommands.Add("setpointmarker",
                  TCollection_AsciiString("setpointmarker [marker] : set point marker\
 by default, or print the current point marker if no argument (this does not modify\
 the marker of the point)\n\n").Cat(MarkersHint).ToCString(),
  		  __FILE__,setpointmarker,g);

  theCommands.Add("changepointmarker",
                  TCollection_AsciiString("changepointmarker marker point: change\
 marker of the point\n\n").Cat(MarkersHint).ToCString(),
  		  __FILE__,changepointmarker,g);

  g = "Geometric tranformations";
  
  theCommands.Add("translate",
		  "translate name [names...] dx dy dz",
                  __FILE__,
		  transform,g);

  theCommands.Add("rotate",
		  "rotate name [names...] x y z dx dy dz angle",
                  __FILE__,
		  transform,g);

  theCommands.Add("pmirror",
		  "pmirror name [names...] x y z",
                  __FILE__,
		  transform,g);

  theCommands.Add("lmirror",
		  "lmirror name [names...] x y z dx dy dz",
                  __FILE__,
		  transform,g);

  theCommands.Add("smirror",
		  "smirror name [names...] x y z dx dy dz",
                  __FILE__,
		  transform,g);

  theCommands.Add("pscale",
		  "pscale name [names...] x y z s",
                  __FILE__,
		  transform,g);

  theCommands.Add("2dtranslate",
		  "translate name [names...] dx dy",
                  __FILE__,
		  d2transform,g);

  theCommands.Add("2drotate",
		  "rotate name [names...] x y dx dy  angle",
                  __FILE__,
		  d2transform,g);

  theCommands.Add("2dpmirror",
		  "pmirror name [names...] x y",
                  __FILE__,
		  d2transform,g);

  theCommands.Add("2dlmirror",
		  "lmirror name [names...] x y dx dy",
                  __FILE__,
		  d2transform,g);

  theCommands.Add("2dpscale",
		  "pscale name [names...] x y s",
                  __FILE__,
		  d2transform,g);
}


//=================================================================
// class : DrawTrSurf_SaveAndRestoreCurve
//=================================================================

class DrawTrSurf_SaveAndRestoreCurve : public Draw_SaveAndRestoreBase
{
public:

  DrawTrSurf_SaveAndRestoreCurve()
  : Draw_SaveAndRestoreBase("DrawTrSurf_Curve") {}

  virtual Standard_Boolean Test (const Handle(Draw_Drawable3D)& d) const Standard_OVERRIDE
  {
    return d->IsInstance(STANDARD_TYPE(DrawTrSurf_Curve));
  }

  virtual void Save (const Handle(Draw_Drawable3D)&d, std::ostream& OS, TopTools_FormatVersion theVersion) const Standard_OVERRIDE
  {
    (void)theVersion;
    Handle(DrawTrSurf_Curve) N = Handle(DrawTrSurf_Curve)::DownCast(d);
    GeomTools_CurveSet::PrintCurve(N->GetCurve(),OS,Standard_True);
  }

  virtual Handle(Draw_Drawable3D) Restore (std::istream& is) const Standard_OVERRIDE
  {
    Handle(Geom_Curve) G = GeomTools_CurveSet::ReadCurve(is);
    Handle(DrawTrSurf_Curve) N =
      new DrawTrSurf_Curve(G,CurvColor,Discret,Deflection,DrawMode);
    return N;
  }

};

static DrawTrSurf_SaveAndRestoreCurve saveAndRestoreCurve;

//=================================================================
// class : DrawTrSurf_SaveAndRestoreBezierCurve
//=================================================================

class DrawTrSurf_SaveAndRestoreBezierCurve : public Draw_SaveAndRestoreBase
{
public:

  DrawTrSurf_SaveAndRestoreBezierCurve()
  : Draw_SaveAndRestoreBase("DrawTrSurf_BezierCurve") {}

  virtual Standard_Boolean Test (const Handle(Draw_Drawable3D)& d) const Standard_OVERRIDE
  {
    return d->IsInstance(STANDARD_TYPE(DrawTrSurf_BezierCurve));
  }

  virtual void Save (const Handle(Draw_Drawable3D)&d, std::ostream& OS, TopTools_FormatVersion theVersion) const  Standard_OVERRIDE
  {
    (void)theVersion;
    Handle(DrawTrSurf_BezierCurve) N = Handle(DrawTrSurf_BezierCurve)::DownCast(d);
    GeomTools_CurveSet::PrintCurve(N->GetCurve(),OS,Standard_True);
  }

  virtual Handle(Draw_Drawable3D) Restore (std::istream& is) const Standard_OVERRIDE
  {
    Handle(Geom_BezierCurve) G = Handle(Geom_BezierCurve)::DownCast (GeomTools_CurveSet::ReadCurve(is));
    Handle(DrawTrSurf_BezierCurve) N =
      new DrawTrSurf_BezierCurve(G,CurvColor,PolesColor,ShowPoles,
			         Discret,Deflection,DrawMode);
    return N;
  }

};

static DrawTrSurf_SaveAndRestoreBezierCurve saveAndRestoreBezierCurve;

//=================================================================
// class : DrawTrSurf_SaveAndRestoreBSplineCurve
//=================================================================

class DrawTrSurf_SaveAndRestoreBSplineCurve : public Draw_SaveAndRestoreBase
{
public:

  DrawTrSurf_SaveAndRestoreBSplineCurve()
  : Draw_SaveAndRestoreBase("DrawTrSurf_BSplineCurve") {}

  virtual Standard_Boolean Test (const Handle(Draw_Drawable3D)& d) const Standard_OVERRIDE
  {
    return d->IsInstance(STANDARD_TYPE(DrawTrSurf_BSplineCurve));
  }

  virtual void Save (const Handle(Draw_Drawable3D)&d, std::ostream& OS, TopTools_FormatVersion theVersion) const  Standard_OVERRIDE
  {
    (void) theVersion;
    Handle(DrawTrSurf_BSplineCurve) N = Handle(DrawTrSurf_BSplineCurve)::DownCast(d);
    GeomTools_CurveSet::PrintCurve(N->GetCurve(),OS,Standard_True);
  }

  virtual Handle(Draw_Drawable3D) Restore(std::istream& is) const Standard_OVERRIDE
  {
    Handle(Geom_BSplineCurve) G = Handle(Geom_BSplineCurve)::DownCast (GeomTools_CurveSet::ReadCurve(is));
    Handle(DrawTrSurf_BSplineCurve) N =
      new DrawTrSurf_BSplineCurve(G, CurvColor,PolesColor,
				  KnotsColor,
				  KnotsShape,KnotsSize,
				  ShowPoles,ShowKnots,
				  Discret,Deflection,DrawMode);
    return N;
  }

};

static DrawTrSurf_SaveAndRestoreBSplineCurve saveAndRestoreBSplineCurve;

//=================================================================
// class : DrawTrSurf_SaveAndRestoreCurve2d
//=================================================================

class DrawTrSurf_SaveAndRestoreCurve2d : public Draw_SaveAndRestoreBase
{
public:

  DrawTrSurf_SaveAndRestoreCurve2d()
  : Draw_SaveAndRestoreBase("DrawTrSurf_Curve2d") {}

  virtual Standard_Boolean Test (const Handle(Draw_Drawable3D)& d) const Standard_OVERRIDE
  {
    return d->IsInstance(STANDARD_TYPE(DrawTrSurf_Curve2d));
  }

  virtual void Save (const Handle(Draw_Drawable3D)&d, std::ostream& OS, TopTools_FormatVersion theVersion) const  Standard_OVERRIDE
  {
    (void) theVersion;
    Handle(DrawTrSurf_Curve2d) N = Handle(DrawTrSurf_Curve2d)::DownCast(d);
    GeomTools_Curve2dSet::PrintCurve2d(N->GetCurve(),OS,Standard_True);
  }

  virtual Handle(Draw_Drawable3D) Restore (std::istream& is) const Standard_OVERRIDE
  {
    Handle(Geom2d_Curve) G = GeomTools_Curve2dSet::ReadCurve2d(is);
    Handle(DrawTrSurf_Curve2d) N = new DrawTrSurf_Curve2d(G,CurvColor,Discret);
    return N;
  }

};

static DrawTrSurf_SaveAndRestoreCurve2d saveAndRestoreCurve2d;

//=================================================================
// class : DrawTrSurf_SaveAndRestoreBezierCurve2d
//=================================================================

class DrawTrSurf_SaveAndRestoreBezierCurve2d : public Draw_SaveAndRestoreBase
{
public:

  DrawTrSurf_SaveAndRestoreBezierCurve2d()
  : Draw_SaveAndRestoreBase("DrawTrSurf_BezierCurve2d") {}

  virtual Standard_Boolean Test (const Handle(Draw_Drawable3D)& d) const Standard_OVERRIDE
  {
    return d->IsInstance(STANDARD_TYPE(DrawTrSurf_BezierCurve2d));
  }

  virtual void Save (const Handle(Draw_Drawable3D)&d, std::ostream& OS, TopTools_FormatVersion theVersion) const  Standard_OVERRIDE
  {
    (void) theVersion;
    Handle(DrawTrSurf_BezierCurve2d) N = Handle(DrawTrSurf_BezierCurve2d)::DownCast(d);
    GeomTools_Curve2dSet::PrintCurve2d(N->GetCurve(),OS,Standard_True);
  }

  virtual Handle(Draw_Drawable3D) Restore (std::istream& is) const Standard_OVERRIDE
  {
    Handle(Geom2d_BezierCurve) G = Handle(Geom2d_BezierCurve)::DownCast (GeomTools_Curve2dSet::ReadCurve2d(is));
    Handle(DrawTrSurf_BezierCurve2d) N =
      new DrawTrSurf_BezierCurve2d(G,CurvColor,PolesColor,ShowPoles,
			         Discret);
    return N;
  }

};

static DrawTrSurf_SaveAndRestoreBezierCurve2d saveAndRestoreBezierCurve2d;

//=================================================================
// class : DrawTrSurf_SaveAndRestoreBSplineCurve2d
//=================================================================

class DrawTrSurf_SaveAndRestoreBSplineCurve2d : public Draw_SaveAndRestoreBase
{
public:

  DrawTrSurf_SaveAndRestoreBSplineCurve2d()
  : Draw_SaveAndRestoreBase("DrawTrSurf_BSplineCurve2d") {}

  virtual Standard_Boolean Test (const Handle(Draw_Drawable3D)& d) const Standard_OVERRIDE
  {
    return d->IsInstance(STANDARD_TYPE(DrawTrSurf_BSplineCurve2d));
  }

  virtual void Save (const Handle(Draw_Drawable3D)&d, std::ostream& OS, TopTools_FormatVersion theVersion) const  Standard_OVERRIDE
  {
    (void) theVersion;
    Handle(DrawTrSurf_BSplineCurve2d) N = Handle(DrawTrSurf_BSplineCurve2d)::DownCast(d);
    GeomTools_Curve2dSet::PrintCurve2d(N->GetCurve(),OS,Standard_True);
  }

  virtual Handle(Draw_Drawable3D) Restore (std::istream& is) const Standard_OVERRIDE
  {
    Handle(Geom2d_BSplineCurve) G = Handle(Geom2d_BSplineCurve)::DownCast (GeomTools_Curve2dSet::ReadCurve2d(is));
    Handle(DrawTrSurf_BSplineCurve2d) N =
      new DrawTrSurf_BSplineCurve2d(G, CurvColor,PolesColor,
				  KnotsColor,
				  KnotsShape,KnotsSize,
				  ShowPoles,ShowKnots,
				  Discret);
    return N;
  }

};

static DrawTrSurf_SaveAndRestoreBSplineCurve2d saveAndRestoreBSplineCurve2d;

//=================================================================
// class : DrawTrSurf_SaveAndRestoreSurface
//=================================================================
class DrawTrSurf_SaveAndRestoreSurface : public Draw_SaveAndRestoreBase
{
public:

  DrawTrSurf_SaveAndRestoreSurface()
  : Draw_SaveAndRestoreBase("DrawTrSurf_Surface") {}

  virtual Standard_Boolean Test (const Handle(Draw_Drawable3D)& d) const Standard_OVERRIDE
  {
    return d->IsInstance(STANDARD_TYPE(DrawTrSurf_Surface));
  }

  virtual void Save (const Handle(Draw_Drawable3D)&d, std::ostream& OS, TopTools_FormatVersion theVersion) const  Standard_OVERRIDE
  {
    (void) theVersion;
    Handle(DrawTrSurf_Surface) N = Handle(DrawTrSurf_Surface)::DownCast(d);
    GeomTools_SurfaceSet::PrintSurface(N->GetSurface(),OS,Standard_True);
  }

  virtual Handle(Draw_Drawable3D) Restore (std::istream& is) const Standard_OVERRIDE
  {
    Handle(Geom_Surface) G = GeomTools_SurfaceSet::ReadSurface(is);
    Handle(DrawTrSurf_Surface) N =
      new DrawTrSurf_Surface(G,
			     NbUIsos,NbVIsos,
			     BoundsColor,IsosColor,
			     Discret,Deflection,DrawMode);
    return N;
  }

};

static DrawTrSurf_SaveAndRestoreSurface saveAndRestoreSurface;

//=================================================================
// class : DrawTrSurf_SaveAndRestoreBezierSurface
//=================================================================

class DrawTrSurf_SaveAndRestoreBezierSurface : public Draw_SaveAndRestoreBase
{
public:

  DrawTrSurf_SaveAndRestoreBezierSurface()
  : Draw_SaveAndRestoreBase("DrawTrSurf_BezierSurface") {}

  virtual Standard_Boolean Test (const Handle(Draw_Drawable3D)& d) const Standard_OVERRIDE
  {
    return d->IsInstance(STANDARD_TYPE(DrawTrSurf_BezierSurface));
  }

  virtual void Save (const Handle(Draw_Drawable3D)&d, std::ostream& OS, TopTools_FormatVersion theVersion) const  Standard_OVERRIDE
  {
    (void) theVersion;
    Handle(DrawTrSurf_BezierSurface) N = Handle(DrawTrSurf_BezierSurface)::DownCast(d);
    GeomTools_SurfaceSet::PrintSurface(N->GetSurface(),OS,Standard_True);
  }

  virtual Handle(Draw_Drawable3D) Restore (std::istream& is) const Standard_OVERRIDE
  {
    Handle(Geom_BezierSurface) G = Handle(Geom_BezierSurface)::DownCast (GeomTools_SurfaceSet::ReadSurface(is));
    Handle(DrawTrSurf_BezierSurface) N =
      new DrawTrSurf_BezierSurface(G,NbUIsos,NbVIsos,
				   BoundsColor,IsosColor,PolesColor,
				   ShowPoles,
				   Discret,Deflection,DrawMode);
    return N;
  }

};

static DrawTrSurf_SaveAndRestoreBezierSurface saveAndRestoreBezierSurface;

//=================================================================
// class : DrawTrSurf_SaveAndRestoreBSplineSurface
//=================================================================

class DrawTrSurf_SaveAndRestoreBSplineSurface : public Draw_SaveAndRestoreBase
{
public:

  DrawTrSurf_SaveAndRestoreBSplineSurface()
  : Draw_SaveAndRestoreBase("DrawTrSurf_BSplineSurface") {}

  virtual Standard_Boolean Test (const Handle(Draw_Drawable3D)& d) const Standard_OVERRIDE
  {
    return d->IsInstance(STANDARD_TYPE(DrawTrSurf_BSplineSurface));
  }

  virtual void Save (const Handle(Draw_Drawable3D)&d, std::ostream& OS, TopTools_FormatVersion theVersion) const  Standard_OVERRIDE
  {
    (void) theVersion;
    Handle(DrawTrSurf_BSplineSurface) N = Handle(DrawTrSurf_BSplineSurface)::DownCast(d);
    GeomTools_SurfaceSet::PrintSurface(N->GetSurface(),OS,Standard_True);
  }

  virtual Handle(Draw_Drawable3D) Restore (std::istream& is) const Standard_OVERRIDE
  {
    Handle(Geom_BSplineSurface) G = Handle(Geom_BSplineSurface)::DownCast (GeomTools_SurfaceSet::ReadSurface(is));
    Handle(DrawTrSurf_BSplineSurface) N;
    if (!knotsIsos)
    {
      N   = new DrawTrSurf_BSplineSurface(G,
					  NbUIsos,NbVIsos,
					  BoundsColor,IsosColor,
					  PolesColor,KnotsColor,
					  KnotsShape,KnotsSize,
					  ShowPoles,ShowKnots,
					        Discret,Deflection,DrawMode);
    }
    else
    {
      N   = new DrawTrSurf_BSplineSurface(G,
					  BoundsColor,IsosColor,
					  PolesColor,KnotsColor,
					  KnotsShape,KnotsSize,
					  ShowPoles,ShowKnots,
					  Discret,Deflection,DrawMode);
    }
    return N;
  }

};

static DrawTrSurf_SaveAndRestoreBSplineSurface saveAndRestoreBSplineSurface;

//=================================================================
// class : DrawTrSurf_SaveAndRestorePoint
//=================================================================

class DrawTrSurf_SaveAndRestorePoint : public Draw_SaveAndRestoreBase
{
public:

  DrawTrSurf_SaveAndRestorePoint()
  : Draw_SaveAndRestoreBase("DrawTrSurf_Point") {}

  virtual Standard_Boolean Test (const Handle(Draw_Drawable3D)& d) const Standard_OVERRIDE
  {
    return d->IsInstance(STANDARD_TYPE(DrawTrSurf_Point));
  }

  virtual void Save (const Handle(Draw_Drawable3D)&d, std::ostream& OS, TopTools_FormatVersion theVersion) const  Standard_OVERRIDE
  {
    (void) theVersion;
    Handle(DrawTrSurf_Point) N = Handle(DrawTrSurf_Point)::DownCast(d);
  #if !defined(_MSC_VER) && !defined(__sgi) && !defined(IRIX)
    std::ios::fmtflags F = OS.flags();
    OS.setf(std::ios::scientific,std::ios::floatfield);
    OS.precision(15);
  #else
    long form = OS.setf(std::ios::scientific);
    std::streamsize prec = OS.precision(15);
  #endif
    gp_Pnt P = N->Point();
    if (N->Is3D()) {
      OS << "1 ";
      OS << P.X() << " " << P.Y() << " " << P.Z() << "\n";
    }
    else {
      OS << "0 ";
      OS << P.X() << " " << P.Y() << "\n";
    }
  #if !defined(_MSC_VER) && !defined(__sgi) && !defined(IRIX)
    OS.setf(F);
  #else
    OS.setf(form);
    OS.precision(prec);
  #endif
  }

  virtual Handle(Draw_Drawable3D) Restore (std::istream& is) const Standard_OVERRIDE
  {
    Standard_Integer is3d;
    is >> is3d;
    Standard_Real x,y,z = 0.;
    if (is3d)
      is >> x >> y >> z;
    else
      is >> x >> y;
    Handle(DrawTrSurf_Point) N;
    if (is3d)
      N = new DrawTrSurf_Point(gp_Pnt(x,y,z),PntShape,PntColor);
    else
      N = new DrawTrSurf_Point(gp_Pnt2d(x,y),PntShape,PntColor);

    return N;
  }

};

static DrawTrSurf_SaveAndRestorePoint saveAndRestorePoint;

//=================================================================
// class : DrawTrSurf_SaveAndRestoreTriangulation
//=================================================================

class DrawTrSurf_SaveAndRestoreTriangulation : public Draw_SaveAndRestoreBase
{
public:

  DrawTrSurf_SaveAndRestoreTriangulation()
  : Draw_SaveAndRestoreBase("DrawTrSurf_Triangulation") {}

  virtual Standard_Boolean Test (const Handle(Draw_Drawable3D)& d) const Standard_OVERRIDE
  {
    return d->IsInstance(STANDARD_TYPE(DrawTrSurf_Triangulation));
  }

  virtual void Save (const Handle(Draw_Drawable3D)&d, std::ostream& OS, TopTools_FormatVersion theVersion) const  Standard_OVERRIDE
  {
    (void) theVersion;
    Handle(DrawTrSurf_Triangulation) T = Handle(DrawTrSurf_Triangulation)::DownCast(d);
  #if !defined(_MSC_VER) && !defined(__sgi) && !defined(IRIX)
    std::ios::fmtflags F = OS.flags();
    OS.setf(std::ios::scientific,std::ios::floatfield);
    OS.precision(15);
  #else
    long form = OS.setf(std::ios::scientific);
    std::streamsize prec = OS.precision(15);
  #endif
    Poly::Write(T->Triangulation(),OS);
  #if !defined(_MSC_VER) && !defined(__sgi) && !defined(IRIX)
    OS.setf(F);
  #else
    OS.setf(form);
    OS.precision(prec);
  #endif
  }

  virtual Handle(Draw_Drawable3D) Restore (std::istream& is) const Standard_OVERRIDE
  {
    return new DrawTrSurf_Triangulation(Poly::ReadTriangulation(is));
  }

};

static DrawTrSurf_SaveAndRestoreTriangulation saveAndRestoreTriangulation;



//=================================================================
// class : DrawTrSurf_SaveAndRestorePolygon3D
//=================================================================

class DrawTrSurf_SaveAndRestorePolygon3D : public Draw_SaveAndRestoreBase
{
public:

  DrawTrSurf_SaveAndRestorePolygon3D()
  : Draw_SaveAndRestoreBase("DrawTrSurf_Polygon3D") {}

  virtual Standard_Boolean Test (const Handle(Draw_Drawable3D)& d) const Standard_OVERRIDE
  {
    return d->IsInstance(STANDARD_TYPE(DrawTrSurf_Polygon3D));
  }

  virtual void Save (const Handle(Draw_Drawable3D)&d, std::ostream& OS, TopTools_FormatVersion theVersion) const  Standard_OVERRIDE
  {
    (void) theVersion;
    Handle(DrawTrSurf_Polygon3D) T = Handle(DrawTrSurf_Polygon3D)::DownCast(d);
  #if !defined(_MSC_VER) && !defined(__sgi) && !defined(IRIX)
    std::ios::fmtflags F = OS.flags();
    OS.setf(std::ios::scientific,std::ios::floatfield);
    OS.precision(15);
  #else
    long form = OS.setf(std::ios::scientific);
    std::streamsize prec = OS.precision(15);
  #endif
    Poly::Write(T->Polygon3D(),OS);
  #if !defined(_MSC_VER) && !defined(__sgi) && !defined(IRIX)
    OS.setf(F);
  #else
    OS.setf(form);
    OS.precision(prec);
  #endif
  }

  virtual Handle(Draw_Drawable3D) Restore (std::istream& is) const Standard_OVERRIDE
  {
    return new DrawTrSurf_Polygon3D(Poly::ReadPolygon3D(is));
  }

};

static DrawTrSurf_SaveAndRestorePolygon3D saveAndRestorePolygon3D;

//=================================================================
// class : DrawTrSurf_SaveAndRestorePolygon2D
//=================================================================

class DrawTrSurf_SaveAndRestorePolygon2D : public Draw_SaveAndRestoreBase
{
public:

  DrawTrSurf_SaveAndRestorePolygon2D()
  : Draw_SaveAndRestoreBase("DrawTrSurf_Polygon2D") {}

  virtual Standard_Boolean Test (const Handle(Draw_Drawable3D)& d) const Standard_OVERRIDE
  {
    return d->IsInstance(STANDARD_TYPE(DrawTrSurf_Polygon2D));
  }

  virtual void Save (const Handle(Draw_Drawable3D)&d, std::ostream& OS, TopTools_FormatVersion theVersion) const  Standard_OVERRIDE
  {
    (void) theVersion;
    Handle(DrawTrSurf_Polygon2D) T = Handle(DrawTrSurf_Polygon2D)::DownCast(d);
  #if !defined(_MSC_VER) && !defined(__sgi) && !defined(IRIX)
    std::ios::fmtflags F = OS.flags();
    OS.setf(std::ios::scientific, std::ios::floatfield);
    OS.precision(15);
  #else
    long form = OS.setf(std::ios::scientific);
    std::streamsize prec = OS.precision(15);
  #endif
    Poly::Write(T->Polygon2D(),OS);
  #if !defined(_MSC_VER) && !defined(__sgi) && !defined(IRIX)
    OS.setf(F);
  #else
    OS.setf(form);
    OS.precision(prec);
  #endif
  }

  virtual Handle(Draw_Drawable3D) Restore (std::istream& is) const Standard_OVERRIDE
  {
    return new DrawTrSurf_Polygon2D(Poly::ReadPolygon2D(is));
  }

};

static DrawTrSurf_SaveAndRestorePolygon2D saveAndRestorePolygon2D;

