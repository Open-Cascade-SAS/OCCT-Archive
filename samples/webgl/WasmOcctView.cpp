// Copyright (c) 2019 OPEN CASCADE SAS
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

#include "WasmOcctView.h"

#include "WasmVKeys.h"
#include "WasmOcctPixMap.h"

#include <AIS_Shape.hxx>
#include <AIS_ViewCube.hxx>
#include <Aspect_Handle.hxx>
#include <Aspect_DisplayConnection.hxx>
#include <Aspect_NeutralWindow.hxx>
#include <Message.hxx>
#include <Message_Messenger.hxx>
#include <Graphic3d_CubeMapPacked.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <Prs3d_DatumAspect.hxx>
#include <Prs3d_ToolCylinder.hxx>
#include <Prs3d_ToolDisk.hxx>

#include <BRep_Builder.hxx>
#include <BRepBndLib.hxx>
#include <BRepTools.hxx>
#include <Standard_ArrayStreamBuffer.hxx>
#include <TopExp_Explorer.hxx>

#include <emscripten/bind.h>

#include <iostream>

#define THE_CANVAS_ID "canvas"

namespace
{
  EM_JS(int, jsCanvasGetWidth, (), {
    return Module.canvas.width;
  });

  EM_JS(int, jsCanvasGetHeight, (), {
    return Module.canvas.height;
  });

  EM_JS(float, jsDevicePixelRatio, (), {
    var aDevicePixelRatio = window.devicePixelRatio || 1;
    return aDevicePixelRatio;
  });

  //! Return cavas size in pixels.
  static Graphic3d_Vec2i jsCanvasSize()
  {
    return Graphic3d_Vec2i (jsCanvasGetWidth(), jsCanvasGetHeight());
  }

  //! Auxiliary wrapper for loading model.
  struct ModelAsyncLoader
  {
    std::string Name;
    std::string Path;
    bool ToExpand;
    OSD_Timer Timer;

    ModelAsyncLoader (const char* theName, const char* thePath, const bool theToExpand)
    : Name (theName), Path (thePath), ToExpand (theToExpand)
    {
      Timer.Start();
    }

    //! File data read event.
    static void onDataRead (void* theOpaque, void* theBuffer, int theDataLen)
    {
      ModelAsyncLoader* aTask = (ModelAsyncLoader* )theOpaque;
      WasmOcctView::openFromMemory (aTask->Name, aTask->ToExpand, reinterpret_cast<uintptr_t>(theBuffer), theDataLen, false);
      aTask->Timer.Stop();
      Message::SendInfo() << aTask->Path << " loaded in " << aTask->Timer.ElapsedTime() << " s";
      delete aTask;
    }

    //! File read error event.
    static void onReadFailed (void* theOpaque)
    {
      const ModelAsyncLoader* aTask = (ModelAsyncLoader* )theOpaque;
      Message::DefaultMessenger()->Send (TCollection_AsciiString("Error: unable to load file ") + aTask->Path.c_str(), Message_Fail);
      delete aTask;
    }
  };

  //! Auxiliary wrapper for loading cubemap.
  struct CubemapAsyncLoader
  {
    //! Image file read event.
    static void onImageRead (const char* theFilePath)
    {
      Handle(Graphic3d_CubeMapPacked) aCubemap;
      Handle(WasmOcctPixMap) anImage = new WasmOcctPixMap();
      if (anImage->Init (theFilePath))
      {
        aCubemap = new Graphic3d_CubeMapPacked (anImage);
        /// TODO WebGL 2.0 ensures sRGB conformance within glGenerateMipmap()
        /// which leads to extremely SLOW generation (5 seconds instead of 0.035)
        aCubemap->SetColorMap (false);
      }
      WasmOcctView::Instance().View()->SetBackgroundCubeMap (aCubemap, true, false);
      WasmOcctView::Instance().UpdateView();
    }

    //! Image file failed read event.
    static void onImageFailed (const char* theFilePath)
    {
      Message::DefaultMessenger()->Send (TCollection_AsciiString("Error: unable to load image ") + theFilePath, Message_Fail);
    }
  };

  //! Object list separator.
  static const char THE_LIST_SEPARATOR = '\n';
}

// ================================================================
// Function : Instance
// Purpose  :
// ================================================================
WasmOcctView& WasmOcctView::Instance()
{
  static WasmOcctView aViewer;
  return aViewer;
}

// ================================================================
// Function : WasmOcctView
// Purpose  :
// ================================================================
WasmOcctView::WasmOcctView()
: myDevicePixelRatio (1.0f),
  myUpdateRequests (0)
{
  SetLockOrbitZUp (true);

  addActionHotKeys (Aspect_VKey_NavForward,        Aspect_VKey_W, Aspect_VKey_W | Aspect_VKeyFlags_SHIFT);
  addActionHotKeys (Aspect_VKey_NavBackward ,      Aspect_VKey_S, Aspect_VKey_S | Aspect_VKeyFlags_SHIFT);
  addActionHotKeys (Aspect_VKey_NavSlideLeft,      Aspect_VKey_A, Aspect_VKey_A | Aspect_VKeyFlags_SHIFT);
  addActionHotKeys (Aspect_VKey_NavSlideRight,     Aspect_VKey_D, Aspect_VKey_D | Aspect_VKeyFlags_SHIFT);
  addActionHotKeys (Aspect_VKey_NavRollCCW,        Aspect_VKey_Q, Aspect_VKey_Q | Aspect_VKeyFlags_SHIFT);
  addActionHotKeys (Aspect_VKey_NavRollCW,         Aspect_VKey_E, Aspect_VKey_E | Aspect_VKeyFlags_SHIFT);

  addActionHotKeys (Aspect_VKey_NavSpeedIncrease,  Aspect_VKey_Plus,  Aspect_VKey_Plus  | Aspect_VKeyFlags_SHIFT,
                                                   Aspect_VKey_Equal,
                                                   Aspect_VKey_NumpadAdd, Aspect_VKey_NumpadAdd | Aspect_VKeyFlags_SHIFT);
  addActionHotKeys (Aspect_VKey_NavSpeedDecrease,  Aspect_VKey_Minus, Aspect_VKey_Minus | Aspect_VKeyFlags_SHIFT,
                                                   Aspect_VKey_NumpadSubtract, Aspect_VKey_NumpadSubtract | Aspect_VKeyFlags_SHIFT);

  // arrow keys conflict with browser page scrolling, so better be avoided in non-fullscreen mode
  addActionHotKeys (Aspect_VKey_NavLookUp,         Aspect_VKey_Numpad8); // Aspect_VKey_Up
  addActionHotKeys (Aspect_VKey_NavLookDown,       Aspect_VKey_Numpad2); // Aspect_VKey_Down
  addActionHotKeys (Aspect_VKey_NavLookLeft,       Aspect_VKey_Numpad4); // Aspect_VKey_Left
  addActionHotKeys (Aspect_VKey_NavLookRight,      Aspect_VKey_Numpad6); // Aspect_VKey_Right
  addActionHotKeys (Aspect_VKey_NavSlideLeft,      Aspect_VKey_Numpad1); // Aspect_VKey_Left |Aspect_VKeyFlags_SHIFT
  addActionHotKeys (Aspect_VKey_NavSlideRight,     Aspect_VKey_Numpad3); // Aspect_VKey_Right|Aspect_VKeyFlags_SHIFT
  addActionHotKeys (Aspect_VKey_NavSlideUp,        Aspect_VKey_Numpad9); // Aspect_VKey_Up   |Aspect_VKeyFlags_SHIFT
  addActionHotKeys (Aspect_VKey_NavSlideDown,      Aspect_VKey_Numpad7); // Aspect_VKey_Down |Aspect_VKeyFlags_SHIFT
}

// ================================================================
// Function : ~WasmOcctView
// Purpose  :
// ================================================================
WasmOcctView::~WasmOcctView()
{
}

// ================================================================
// Function : run
// Purpose  :
// ================================================================
void WasmOcctView::run()
{
  initWindow();
  initViewer();
  initDemoScene();
  if (myView.IsNull())
  {
    return;
  }

  myView->MustBeResized();
  myView->Redraw();

  // There is no infinite message loop, main() will return from here immediately.
  // Tell that our Module should be left loaded and handle events through callbacks.
  //emscripten_set_main_loop (redrawView, 60, 1);
  //emscripten_set_main_loop (redrawView, -1, 1);
  EM_ASM(Module['noExitRuntime'] = true);
}

// ================================================================
// Function : initWindow
// Purpose  :
// ================================================================
void WasmOcctView::initWindow()
{
  myDevicePixelRatio = jsDevicePixelRatio();
  myCanvasId = THE_CANVAS_ID;
  const char* aTargetId = !myCanvasId.IsEmpty() ? myCanvasId.ToCString() : EMSCRIPTEN_EVENT_TARGET_WINDOW;
  const EM_BOOL toUseCapture = EM_TRUE;
  emscripten_set_resize_callback     (EMSCRIPTEN_EVENT_TARGET_WINDOW, this, toUseCapture, onResizeCallback);

  emscripten_set_mousedown_callback  (aTargetId, this, toUseCapture, onMouseCallback);
  emscripten_set_mouseup_callback    (aTargetId, this, toUseCapture, onMouseCallback);
  emscripten_set_mousemove_callback  (aTargetId, this, toUseCapture, onMouseCallback);
  emscripten_set_dblclick_callback   (aTargetId, this, toUseCapture, onMouseCallback);
  emscripten_set_click_callback      (aTargetId, this, toUseCapture, onMouseCallback);
  emscripten_set_mouseenter_callback (aTargetId, this, toUseCapture, onMouseCallback);
  emscripten_set_mouseleave_callback (aTargetId, this, toUseCapture, onMouseCallback);
  emscripten_set_wheel_callback      (aTargetId, this, toUseCapture, onWheelCallback);

  emscripten_set_touchstart_callback (aTargetId, this, toUseCapture, onTouchCallback);
  emscripten_set_touchend_callback   (aTargetId, this, toUseCapture, onTouchCallback);
  emscripten_set_touchmove_callback  (aTargetId, this, toUseCapture, onTouchCallback);
  emscripten_set_touchcancel_callback(aTargetId, this, toUseCapture, onTouchCallback);

  //emscripten_set_keypress_callback   (EMSCRIPTEN_EVENT_TARGET_WINDOW, this, toUseCapture, onKeyCallback);
  emscripten_set_keydown_callback    (EMSCRIPTEN_EVENT_TARGET_WINDOW, this, toUseCapture, onKeyDownCallback);
  emscripten_set_keyup_callback      (EMSCRIPTEN_EVENT_TARGET_WINDOW, this, toUseCapture, onKeyUpCallback);
}

// ================================================================
// Function : dumpGlInfo
// Purpose  :
// ================================================================
void WasmOcctView::dumpGlInfo (bool theIsBasic)
{
  TColStd_IndexedDataMapOfStringString aGlCapsDict;
  myView->DiagnosticInformation (aGlCapsDict, theIsBasic ? Graphic3d_DiagnosticInfo_Basic : Graphic3d_DiagnosticInfo_Complete);
  if (theIsBasic)
  {
    TCollection_AsciiString aViewport;
    aGlCapsDict.FindFromKey ("Viewport", aViewport);
    aGlCapsDict.Clear();
    aGlCapsDict.Add ("Viewport", aViewport);
  }
  aGlCapsDict.Add ("Display scale", TCollection_AsciiString(myDevicePixelRatio));

  // beautify output
  {
    TCollection_AsciiString* aGlVer   = aGlCapsDict.ChangeSeek ("GLversion");
    TCollection_AsciiString* aGlslVer = aGlCapsDict.ChangeSeek ("GLSLversion");
    if (aGlVer   != NULL
     && aGlslVer != NULL)
    {
      *aGlVer = *aGlVer + " [GLSL: " + *aGlslVer + "]";
      aGlslVer->Clear();
    }
  }

  TCollection_AsciiString anInfo;
  for (TColStd_IndexedDataMapOfStringString::Iterator aValueIter (aGlCapsDict); aValueIter.More(); aValueIter.Next())
  {
    if (!aValueIter.Value().IsEmpty())
    {
      if (!anInfo.IsEmpty())
      {
        anInfo += "\n";
      }
      anInfo += aValueIter.Key() + ": " + aValueIter.Value();
    }
  }

  ::Message::DefaultMessenger()->Send (anInfo, Message_Warning);
}

// ================================================================
// Function : initPixelScaleRatio
// Purpose  :
// ================================================================
void WasmOcctView::initPixelScaleRatio()
{
  SetTouchToleranceScale (myDevicePixelRatio);
  if (!myView.IsNull())
  {
    myView->ChangeRenderingParams().Resolution = (unsigned int )(96.0 * myDevicePixelRatio + 0.5);
  }
  if (!myContext.IsNull())
  {
    myContext->SetPixelTolerance (int(myDevicePixelRatio * 6.0));
    if (!myViewCube.IsNull())
    {
      static const double THE_CUBE_SIZE = 60.0;
      myViewCube->SetSize (myDevicePixelRatio * THE_CUBE_SIZE, false);
      myViewCube->SetBoxFacetExtension (myViewCube->Size() * 0.15);
      myViewCube->SetAxesPadding (myViewCube->Size() * 0.10);
      myViewCube->SetFontHeight  (THE_CUBE_SIZE * 0.16);
      if (myViewCube->HasInteractiveContext())
      {
        myContext->Redisplay (myViewCube, false);
      }
    }
  }
}

// ================================================================
// Function : initViewer
// Purpose  :
// ================================================================
bool WasmOcctView::initViewer()
{
  // Build with "--preload-file MyFontFile.ttf" option
  // and register font in Font Manager to use custom font(s).
  /*const char* aFontPath = "MyFontFile.ttf";
  if (Handle(Font_SystemFont) aFont = Font_FontMgr::GetInstance()->CheckFont (aFontPath))
  {
    Font_FontMgr::GetInstance()->RegisterFont (aFont, true);
  }
  else
  {
    Message::DefaultMessenger()->Send (TCollection_AsciiString ("Error: font '") + aFontPath + "' is not found", Message_Fail);
  }*/

  const bool toAntialias = myDevicePixelRatio <= 1.25f;

  Handle(Aspect_DisplayConnection) aDisp;
  Handle(OpenGl_GraphicDriver) aDriver = new OpenGl_GraphicDriver (aDisp, false);
  aDriver->ChangeOptions().buffersNoSwap = true; // swap has no effect in WebGL
  aDriver->ChangeOptions().buffersOpaqueAlpha = true; // avoid unexpected blending of canvas with page background
  aDriver->ChangeOptions().useSystemBuffer = false; ///
  //aDriver->ChangeOptions().useSystemBuffer = true; ///
  if (!aDriver->InitContext())
  {
    Message::DefaultMessenger()->Send (TCollection_AsciiString ("Error: EGL initialization failed"), Message_Fail);
    return false;
  }

  Handle(V3d_Viewer) aViewer = new V3d_Viewer (aDriver);
  aViewer->SetComputedMode (false);
  aViewer->SetDefaultShadingModel (Graphic3d_TOSM_FRAGMENT);
  aViewer->SetDefaultLights();
  aViewer->SetLightOn();
  for (V3d_ListOfLight::Iterator aLightIter (aViewer->ActiveLights()); aLightIter.More(); aLightIter.Next())
  {
    const Handle(V3d_Light)& aLight = aLightIter.Value();
    if (aLight->Type() == Graphic3d_TOLS_DIRECTIONAL)
    {
      aLight->SetCastShadows (true);
      aLight->SetHeadlight (false);
      aLight->SetDirection (gp_Dir (0.098f, -0.20f, -0.98f));
    }
  }

  Handle(Aspect_NeutralWindow) aWindow = new Aspect_NeutralWindow();
  Graphic3d_Vec2i aWinSize = jsCanvasSize();
  if (aWinSize.x() < 10 || aWinSize.y() < 10)
  {
    Message::DefaultMessenger()->Send (TCollection_AsciiString ("Warning: invalid canvas size"), Message_Warning);
  }
  aWindow->SetSize (aWinSize.x(), aWinSize.y());

  myTextStyle = new Prs3d_TextAspect();
  myTextStyle->SetFont (Font_NOF_ASCII_MONO);
  myTextStyle->SetHeight (12);
  myTextStyle->Aspect()->SetColor (Quantity_NOC_GRAY95);
  myTextStyle->Aspect()->SetColorSubTitle (Quantity_NOC_BLACK);
  myTextStyle->Aspect()->SetDisplayType (Aspect_TODT_SHADOW);
  myTextStyle->Aspect()->SetTextFontAspect (Font_FA_Bold);
  myTextStyle->Aspect()->SetTextZoomable (false);
  myTextStyle->SetHorizontalJustification (Graphic3d_HTA_LEFT);
  myTextStyle->SetVerticalJustification (Graphic3d_VTA_BOTTOM);

  myView = new V3d_View (aViewer);
  myView->Camera()->SetProjectionType (Graphic3d_Camera::Projection_Perspective);
  myView->Camera()->SetFOV2d (360.0f);
  myView->Camera()->SetFOVy (45.0f);
  myView->SetImmediateUpdate (false);
  //myView->ChangeRenderingParams().NbMsaaSamples = toAntialias ? 4 : 0;
  myView->ChangeRenderingParams().RenderResolutionScale = toAntialias ? 2.0f : 1.0f;
  myView->ChangeRenderingParams().Resolution = (unsigned int )(96.0 * myDevicePixelRatio + 0.5);
  myView->ChangeRenderingParams().ToShowStats = true;
  myView->ChangeRenderingParams().StatsTextAspect = myTextStyle->Aspect();
  myView->ChangeRenderingParams().StatsTextHeight = (int )myTextStyle->Height();
  myView->SetWindow (aWindow);
  dumpGlInfo (false);

  myView->SetShadingModel (aDriver->InquireLimit (Graphic3d_TypeOfLimit_HasPBR) == 1 ? Graphic3d_TOSM_PBR : Graphic3d_TOSM_FRAGMENT);
  ///myView->ChangeRenderingParams().IsShadowEnabled = aDriver->InquireLimit (Graphic3d_TypeOfLimit_HasPBR) == 1; /// TODO
  myView->ChangeRenderingParams().IsShadowEnabled = true; ///

  myContext = new AIS_InteractiveContext (aViewer);
  {
    {
      const Handle(Prs3d_Drawer)& aHiStyle = myContext->HighlightStyle();
      aHiStyle->SetTransparency (0.8f);

      Handle(Graphic3d_AspectFillArea3d) anAspect = new Graphic3d_AspectFillArea3d();
      *anAspect = *myContext->DefaultDrawer()->ShadingAspect()->Aspect();
      Graphic3d_MaterialAspect aMat (Graphic3d_NOM_STONE);
      aMat.SetColor (aHiStyle->Color());
      aMat.SetTransparency (aHiStyle->Transparency());
      anAspect->SetFrontMaterial (aMat);
      anAspect->SetInteriorColor (aHiStyle->Color());
      aHiStyle->SetBasicFillAreaAspect (anAspect);
    }
  }
  initPixelScaleRatio();
  return true;
}

// ================================================================
// Function : initDemoScene
// Purpose  :
// ================================================================
void WasmOcctView::initDemoScene()
{
  if (myContext.IsNull())
  {
    return;
  }

  //myView->TriedronDisplay (Aspect_TOTP_LEFT_LOWER, Quantity_NOC_GOLD, 0.08, V3d_WIREFRAME);

  myViewCube = new AIS_ViewCube();
  // presentation parameters
  initPixelScaleRatio();
  myViewCube->SetTransformPersistence (new Graphic3d_TransformPers (Graphic3d_TMF_TriedronPers, Aspect_TOTP_RIGHT_LOWER, Graphic3d_Vec2i (100, 100)));
  myViewCube->Attributes()->SetDatumAspect (new Prs3d_DatumAspect());
  myViewCube->Attributes()->DatumAspect()->SetTextAspect (myTextStyle);
  // animation parameters
  myViewCube->SetViewAnimation (myViewAnimation);
  myViewCube->SetFixedAnimationLoop (false);
  myViewCube->SetAutoStartAnimation (true);
  myContext->Display (myViewCube, false);

  // Build with "--preload-file MySampleFile.brep" option to load some shapes here.
}

// ================================================================
// Function : UpdateView
// Purpose  :
// ================================================================
void WasmOcctView::UpdateView()
{
  if (!myView.IsNull())
  {
    myView->Invalidate();
    updateView();
  }
}

// ================================================================
// Function : updateView
// Purpose  :
// ================================================================
void WasmOcctView::updateView()
{
  if (!myView.IsNull())
  {
    if (++myUpdateRequests == 1)
    {
      emscripten_async_call (onRedrawView, this, 0);
    }
  }
}

// ================================================================
// Function : redrawView
// Purpose  :
// ================================================================
void WasmOcctView::redrawView()
{
  if (!myView.IsNull())
  {
    FlushViewEvents (myContext, myView, true);
  }
}

// ================================================================
// Function : handleViewRedraw
// Purpose  :
// ================================================================
void WasmOcctView::handleViewRedraw (const Handle(AIS_InteractiveContext)& theCtx,
                                     const Handle(V3d_View)& theView)
{
  myUpdateRequests = 0;
  AIS_ViewController::handleViewRedraw (theCtx, theView);

  for (NCollection_DataMap<unsigned int, Aspect_VKey>::Iterator aNavKeyIter (myNavKeyMap);
       !myToAskNextFrame && aNavKeyIter.More(); aNavKeyIter.Next())
  {
    const Aspect_VKey aVKey = aNavKeyIter.Key() & ~Aspect_VKeyFlags_ALL;
    myToAskNextFrame = myKeys.IsKeyDown (aVKey);
  }

  if (myToAskNextFrame)
  {
    // ask more frames
    ++myUpdateRequests;
    emscripten_async_call (onRedrawView, this, 0);
  }
}

// ================================================================
// Function : onResizeEvent
// Purpose  :
// ================================================================
EM_BOOL WasmOcctView::onResizeEvent (int theEventType, const EmscriptenUiEvent* theEvent)
{
  (void )theEventType; // EMSCRIPTEN_EVENT_RESIZE or EMSCRIPTEN_EVENT_CANVASRESIZED
  (void )theEvent;
  if (myView.IsNull())
  {
    return EM_FALSE;
  }

  Handle(Aspect_NeutralWindow) aWindow = Handle(Aspect_NeutralWindow)::DownCast (myView->Window());
  Graphic3d_Vec2i aWinSizeOld, aWinSizeNew (jsCanvasSize());
  if (aWinSizeNew.x() < 10 || aWinSizeNew.y() < 10)
  {
    Message::DefaultMessenger()->Send (TCollection_AsciiString ("Warning: invalid canvas size"), Message_Warning);
  }
  aWindow->Size (aWinSizeOld.x(), aWinSizeOld.y());
  const float aPixelRatio = jsDevicePixelRatio();
  if (aWinSizeNew != aWinSizeOld
   || aPixelRatio != myDevicePixelRatio)
  {
    if (myDevicePixelRatio != aPixelRatio)
    {
      myDevicePixelRatio = aPixelRatio;
      initPixelScaleRatio();
    }
    aWindow->SetSize (aWinSizeNew.x(), aWinSizeNew.y());
    myView->MustBeResized();
    myView->Invalidate();
    myView->Redraw();
    dumpGlInfo (true);
  }
  return EM_TRUE;
}

// ================================================================
// Function : onMouseEvent
// Purpose  :
// ================================================================
EM_BOOL WasmOcctView::onMouseEvent (int theEventType, const EmscriptenMouseEvent* theEvent)
{
  if (myView.IsNull())
  {
    return EM_FALSE;
  }

  Graphic3d_Vec2i aWinSize;
  myView->Window()->Size (aWinSize.x(), aWinSize.y());
  const Graphic3d_Vec2i aNewPos = convertPointToBacking (Graphic3d_Vec2i (theEvent->targetX, theEvent->targetY));
  Aspect_VKeyFlags aFlags = 0;
  if (theEvent->ctrlKey  == EM_TRUE) { aFlags |= Aspect_VKeyFlags_CTRL;  }
  if (theEvent->shiftKey == EM_TRUE) { aFlags |= Aspect_VKeyFlags_SHIFT; }
  if (theEvent->altKey   == EM_TRUE) { aFlags |= Aspect_VKeyFlags_ALT;   }
  if (theEvent->metaKey  == EM_TRUE) { aFlags |= Aspect_VKeyFlags_META;  }

  const bool isEmulated = false;
  const Aspect_VKeyMouse aButtons = WasmVKeys_MouseButtonsFromNative (theEvent->buttons);
  switch (theEventType)
  {
    case EMSCRIPTEN_EVENT_MOUSEMOVE:
    {
      if ((aNewPos.x() < 0 || aNewPos.x() > aWinSize.x()
        || aNewPos.y() < 0 || aNewPos.y() > aWinSize.y())
        && PressedMouseButtons() == Aspect_VKeyMouse_NONE)
      {
        return EM_FALSE;
      }
      if (UpdateMousePosition (aNewPos, aButtons, aFlags, isEmulated))
      {
        updateView();
      }
      break;
    }
    case EMSCRIPTEN_EVENT_MOUSEDOWN:
    case EMSCRIPTEN_EVENT_MOUSEUP:
    {
      if (aNewPos.x() < 0 || aNewPos.x() > aWinSize.x()
       || aNewPos.y() < 0 || aNewPos.y() > aWinSize.y())
      {
        return EM_FALSE;
      }
      if (UpdateMouseButtons (aNewPos, aButtons, aFlags, isEmulated))
      {
        updateView();
      }
      break;
    }
    case EMSCRIPTEN_EVENT_CLICK:
    case EMSCRIPTEN_EVENT_DBLCLICK:
    {
      if (aNewPos.x() < 0 || aNewPos.x() > aWinSize.x()
       || aNewPos.y() < 0 || aNewPos.y() > aWinSize.y())
      {
        return EM_FALSE;
      }
      break;
    }
    case EMSCRIPTEN_EVENT_MOUSEENTER:
    {
      break;
    }
    case EMSCRIPTEN_EVENT_MOUSELEAVE:
    {
      // there is no SetCapture() support, so that mouse unclick events outside canvas will not arrive,
      // so we have to forget current state...
      if (UpdateMouseButtons (aNewPos, Aspect_VKeyMouse_NONE, aFlags, isEmulated))
      {
        updateView();
      }
      break;
    }
  }
  return EM_TRUE;
}

// ================================================================
// Function : onWheelEvent
// Purpose  :
// ================================================================
EM_BOOL WasmOcctView::onWheelEvent (int theEventType, const EmscriptenWheelEvent* theEvent)
{
  if (myView.IsNull()
   || theEventType != EMSCRIPTEN_EVENT_WHEEL)
  {
    return EM_FALSE;
  }

  Graphic3d_Vec2i aWinSize;
  myView->Window()->Size (aWinSize.x(), aWinSize.y());
  const Graphic3d_Vec2i aNewPos = convertPointToBacking (Graphic3d_Vec2i (theEvent->mouse.targetX, theEvent->mouse.targetY));
  if (aNewPos.x() < 0 || aNewPos.x() > aWinSize.x()
   || aNewPos.y() < 0 || aNewPos.y() > aWinSize.y())
  {
    return EM_FALSE;
  }

  double aDelta = 0.0;
  switch (theEvent->deltaMode)
  {
    case DOM_DELTA_PIXEL:
    {
      aDelta = theEvent->deltaY / (5.0 * myDevicePixelRatio);
      break;
    }
    case DOM_DELTA_LINE:
    {
      aDelta = theEvent->deltaY * 8.0;
      break;
    }
    case DOM_DELTA_PAGE:
    {
      aDelta = theEvent->deltaY >= 0.0 ? 24.0 : -24.0;
      break;
    }
  }

  if (UpdateZoom (Aspect_ScrollDelta (aNewPos, -aDelta)))
  {
    updateView();
  }
  return EM_TRUE;
}

// ================================================================
// Function : onTouchEvent
// Purpose  :
// ================================================================
EM_BOOL WasmOcctView::onTouchEvent (int theEventType, const EmscriptenTouchEvent* theEvent)
{
  const double aClickTolerance = 5.0;
  if (myView.IsNull())
  {
    return EM_FALSE;
  }

  Graphic3d_Vec2i aWinSize;
  myView->Window()->Size (aWinSize.x(), aWinSize.y());
  bool hasUpdates = false;
  for (int aTouchIter = 0; aTouchIter < theEvent->numTouches; ++aTouchIter)
  {
    const EmscriptenTouchPoint& aTouch = theEvent->touches[aTouchIter];
    if (!aTouch.isChanged)
    {
      continue;
    }

    const Standard_Size aTouchId = (Standard_Size )aTouch.identifier;
    const Graphic3d_Vec2i aNewPos = convertPointToBacking (Graphic3d_Vec2i (aTouch.targetX, aTouch.targetY));
    switch (theEventType)
    {
      case EMSCRIPTEN_EVENT_TOUCHSTART:
      {
        if (aNewPos.x() >= 0 && aNewPos.x() < aWinSize.x()
         && aNewPos.y() >= 0 && aNewPos.y() < aWinSize.y())
        {
          hasUpdates = true;
          AddTouchPoint (aTouchId, Graphic3d_Vec2d (aNewPos));
          myClickTouch.From.SetValues (-1.0, -1.0);
          if (myTouchPoints.Extent() == 1)
          {
            myClickTouch.From = Graphic3d_Vec2d (aNewPos);
          }
        }
        break;
      }
      case EMSCRIPTEN_EVENT_TOUCHMOVE:
      {
        const int anOldIndex = myTouchPoints.FindIndex (aTouchId);
        if (anOldIndex != 0)
        {
          hasUpdates = true;
          UpdateTouchPoint (aTouchId, Graphic3d_Vec2d (aNewPos));
          if (myTouchPoints.Extent() == 1
           && (myClickTouch.From - Graphic3d_Vec2d (aNewPos)).cwiseAbs().maxComp() > aClickTolerance)
          {
            myClickTouch.From.SetValues (-1.0, -1.0);
          }
        }
        break;
      }
      case EMSCRIPTEN_EVENT_TOUCHEND:
      case EMSCRIPTEN_EVENT_TOUCHCANCEL:
      {
        if (RemoveTouchPoint (aTouchId))
        {
          if (myTouchPoints.IsEmpty()
           && myClickTouch.From.minComp() >= 0.0)
          {
            if (myDoubleTapTimer.IsStarted()
             && myDoubleTapTimer.ElapsedTime() <= myMouseDoubleClickInt)
            {
              myView->FitAll (0.01, false);
              myView->Invalidate();
            }
            else
            {
              myDoubleTapTimer.Stop();
              myDoubleTapTimer.Reset();
              myDoubleTapTimer.Start();
              SelectInViewer (Graphic3d_Vec2i (myClickTouch.From), false);
            }
          }
          hasUpdates = true;
        }
        break;
      }
    }
  }
  if (hasUpdates)
  {
    updateView();
  }
  return hasUpdates || !myTouchPoints.IsEmpty() ? EM_TRUE : EM_FALSE;
}

// ================================================================
// Function : navigationKeyModifierSwitch
// Purpose  :
// ================================================================
bool WasmOcctView::navigationKeyModifierSwitch (unsigned int theModifOld,
                                                unsigned int theModifNew,
                                                double       theTimeStamp)
{
  bool hasActions = false;
  for (unsigned int aKeyIter = 0; aKeyIter < Aspect_VKey_ModifiersLower; ++aKeyIter)
  {
    if (!myKeys.IsKeyDown (aKeyIter))
    {
      continue;
    }

    Aspect_VKey anActionOld = Aspect_VKey_UNKNOWN, anActionNew = Aspect_VKey_UNKNOWN;
    myNavKeyMap.Find (aKeyIter | theModifOld, anActionOld);
    myNavKeyMap.Find (aKeyIter | theModifNew, anActionNew);
    if (anActionOld == anActionNew)
    {
      continue;
    }

    if (anActionOld != Aspect_VKey_UNKNOWN)
    {
      myKeys.KeyUp (anActionOld, theTimeStamp);
    }
    if (anActionNew != Aspect_VKey_UNKNOWN)
    {
      hasActions = true;
      myKeys.KeyDown (anActionNew, theTimeStamp);
    }
  }
  return hasActions;
}

// ================================================================
// Function : onKeyDownEvent
// Purpose  :
// ================================================================
EM_BOOL WasmOcctView::onKeyDownEvent (int theEventType, const EmscriptenKeyboardEvent* theEvent)
{
  if (myView.IsNull()
   || theEventType != EMSCRIPTEN_EVENT_KEYDOWN) // EMSCRIPTEN_EVENT_KEYPRESS
  {
    return EM_FALSE;
  }

  const double aTimeStamp = EventTime();
  const Aspect_VKey aVKey = WasmVKeys_VirtualKeyFromNative (theEvent->keyCode);
  if (aVKey == Aspect_VKey_UNKNOWN)
  {
    return EM_FALSE;
  }
  if (theEvent->repeat == EM_TRUE)
  {
    return EM_FALSE;
  }

  const unsigned int aModifOld = myKeys.Modifiers();
  AIS_ViewController::KeyDown (aVKey, aTimeStamp);

  const unsigned int aModifNew = myKeys.Modifiers();
  if (aModifNew != aModifOld
   && navigationKeyModifierSwitch (aModifOld, aModifNew, aTimeStamp))
  {
    // modifier key just pressed
  }

  Aspect_VKey anAction = Aspect_VKey_UNKNOWN;
  if (myNavKeyMap.Find (aVKey | myKeys.Modifiers(), anAction)
  &&  anAction != Aspect_VKey_UNKNOWN)
  {
    AIS_ViewController::KeyDown (anAction, aTimeStamp);
    UpdateView();
  }
  return EM_FALSE;
}

// ================================================================
// Function : onKeyUpEvent
// Purpose  :
// ================================================================
EM_BOOL WasmOcctView::onKeyUpEvent (int theEventType, const EmscriptenKeyboardEvent* theEvent)
{
  if (myView.IsNull()
   || theEventType != EMSCRIPTEN_EVENT_KEYUP)
  {
    return EM_FALSE;
  }

  const double aTimeStamp = EventTime();
  const Aspect_VKey aVKey = WasmVKeys_VirtualKeyFromNative (theEvent->keyCode);
  if (aVKey == Aspect_VKey_UNKNOWN)
  {
    return EM_FALSE;
  }

  const unsigned int aModifOld = myKeys.Modifiers();
  AIS_ViewController::KeyUp (aVKey, aTimeStamp);

  Aspect_VKey anAction = Aspect_VKey_UNKNOWN;
  if (myNavKeyMap.Find (aVKey | myKeys.Modifiers(), anAction)
  &&  anAction != Aspect_VKey_UNKNOWN)
  {
    AIS_ViewController::KeyUp (anAction, aTimeStamp);
    UpdateView();
  }

  const unsigned int aModifNew = myKeys.Modifiers();
  if (aModifNew != aModifOld
   && navigationKeyModifierSwitch (aModifOld, aModifNew, aTimeStamp))
  {
    // modifier key released
  }

  return processKeyPress (aVKey | aModifNew) ? EM_TRUE : EM_FALSE;
}

//==============================================================================
//function : processKeyPress
//purpose  :
//==============================================================================
bool WasmOcctView::processKeyPress (Aspect_VKey theKey)
{
  switch (theKey)
  {
    case Aspect_VKey_F:
    {
      myView->FitAll (0.01, false);
      UpdateView();
      return true;
    }
  }
  return false;
}

// ================================================================
// Function : toDynamicHighlight
// Purpose  :
// ================================================================
bool WasmOcctView::toDynamicHighlight()
{
  return WasmOcctView::Instance().ToAllowHighlight();
}

// ================================================================
// Function : setDynamicHighlight
// Purpose  :
// ================================================================
void WasmOcctView::setDynamicHighlight (bool theToEnable)
{
  WasmOcctView::Instance().SetAllowHighlight (theToEnable);
}

// ================================================================
// Function : toCastShadows
// Purpose  :
// ================================================================
bool WasmOcctView::toCastShadows()
{
  return WasmOcctView::Instance().View()->RenderingParams().IsShadowEnabled;
}

// ================================================================
// Function : setCastShadows
// Purpose  :
// ================================================================
void WasmOcctView::setCastShadows (bool theUseShadows,
                                   bool theToUpdate)
{
  if (WasmOcctView::Instance().View()->RenderingParams().IsShadowEnabled != theUseShadows)
  {
    WasmOcctView::Instance().View()->ChangeRenderingParams().IsShadowEnabled = theUseShadows;
    if (theToUpdate)
    {
      WasmOcctView::Instance().UpdateView();
    }
  }
}

// ================================================================
// Function : isAntiAliasingOn
// Purpose  :
// ================================================================
bool WasmOcctView::isAntiAliasingOn()
{
  return WasmOcctView::Instance().View()->RenderingParams().RenderResolutionScale > 1.1f;
}

// ================================================================
// Function : setAntiAliasingOn
// Purpose  :
// ================================================================
void WasmOcctView::setAntiAliasingOn (bool theToEnable, bool theToUpdate)
{
  if (isAntiAliasingOn() != theToEnable)
  {
    WasmOcctView::Instance().View()->ChangeRenderingParams().RenderResolutionScale = theToEnable ? 2.0f : 1.0f;
    if (theToUpdate)
    {
      WasmOcctView::Instance().UpdateView();
    }
  }
}

// ================================================================
// Function : setBackgroundColor
// Purpose  :
// ================================================================
void WasmOcctView::setBackgroundColor (float theR, float theG, float theB,
                                       bool theToUpdate)
{
  Quantity_Color aColor (theR, theG, theB, Quantity_TOC_RGB);
  WasmOcctView::Instance().View()->SetBackgroundColor (aColor);
  WasmOcctView::Instance().View()->SetBackgroundCubeMap (Handle(Graphic3d_CubeMap)(), true);
  if (theToUpdate)
  {
    WasmOcctView::Instance().UpdateView();
  }
}

// ================================================================
// Function : setBackgroundCubemap
// Purpose  :
// ================================================================
void WasmOcctView::setBackgroundCubemap (const std::string& theImagePath)
{
  if (!theImagePath.empty())
  {
    emscripten_async_wget (theImagePath.c_str(), "/emulated/cubemap.jpg", CubemapAsyncLoader::onImageRead, CubemapAsyncLoader::onImageFailed);
  }
  else
  {
    WasmOcctView::Instance().View()->SetBackgroundCubeMap (Handle(Graphic3d_CubeMapPacked)(), true, false);
    WasmOcctView::Instance().UpdateView();
  }
}

// ================================================================
// Function : fitAllObjects
// Purpose  :
// ================================================================
void WasmOcctView::fitAllObjects (bool theAuto,
                                  bool theToUpdate)
{
  WasmOcctView& aViewer = Instance();
  if (theAuto)
  {
    aViewer.FitAllAuto (aViewer.Context(), aViewer.View());
  }
  else
  {
    aViewer.View()->FitAll (0.01, false);
  }
  if (theToUpdate)
  {
    aViewer.UpdateView();
  }
}

EM_JS(void, jsOnDisplayedObjectsChanged, (), {
  Module.onDisplayedObjectsChanged();
});
EM_JS(void, jsOnSelectedObjectsChanged, (), {
  Module.onSelectedObjectsChanged();
});

// ================================================================
// Function : onDisplayedObjectsChanged
// Purpose  :
// ================================================================
void WasmOcctView::onDisplayedObjectsChanged()
{
  jsOnDisplayedObjectsChanged();
}

// ================================================================
// Function : OnSelectionChanged
// Purpose  :
// ================================================================
void WasmOcctView::OnSelectionChanged (const Handle(AIS_InteractiveContext)& ,
                                       const Handle(V3d_View)& )
{
  jsOnSelectedObjectsChanged();
}

// ================================================================
// Function : displayedObjects
// Purpose  :
// ================================================================
std::string WasmOcctView::displayedObjects()
{
  WasmOcctView& aViewer = Instance();
  std::string aList;
  for (NCollection_IndexedDataMap<TCollection_AsciiString, Handle(AIS_InteractiveObject)>::Iterator anObjIter (aViewer.myObjects);
       anObjIter.More(); anObjIter.Next())
  {
    if (!aList.empty()) { aList += THE_LIST_SEPARATOR; }
    aList += anObjIter.Key().ToCString();
  }
  return aList;
}

// ================================================================
// Function : selectedObjects
// Purpose  :
// ================================================================
std::string WasmOcctView::selectedObjects()
{
  WasmOcctView& aViewer = Instance();
  std::string aList;
  for (NCollection_IndexedDataMap<TCollection_AsciiString, Handle(AIS_InteractiveObject)>::Iterator anObjIter (aViewer.myObjects);
       anObjIter.More(); anObjIter.Next())
  {
    if (aViewer.Context()->IsSelected (anObjIter.Value()))
    {
      if (!aList.empty()) { aList += THE_LIST_SEPARATOR; }
      aList += anObjIter.Key().ToCString();
    }
  }
  return aList;
}

// ================================================================
// Function : erasedObjects
// Purpose  :
// ================================================================
std::string WasmOcctView::erasedObjects()
{
  WasmOcctView& aViewer = Instance();
  std::string aList;
  for (NCollection_IndexedDataMap<TCollection_AsciiString, Handle(AIS_InteractiveObject)>::Iterator anObjIter (aViewer.myObjects);
       anObjIter.More(); anObjIter.Next())
  {
    if (!aViewer.Context()->IsDisplayed (anObjIter.Value()))
    {
      if (!aList.empty()) { aList += THE_LIST_SEPARATOR; }
      aList += anObjIter.Key().ToCString();
    }
  }
  return aList;
}

// ================================================================
// Function : displayObjectList
// Purpose  :
// ================================================================
bool WasmOcctView::displayObjectList (const std::string& theNames,
                                      bool theToUpdate)
{
  WasmOcctView& aViewer = Instance();

  std::stringstream aListStream (theNames);
  std::string aName;
  bool hasChanged = false;
  while (std::getline (aListStream, aName, THE_LIST_SEPARATOR))
  {
    Handle(AIS_InteractiveObject) anObj;
    if (!aViewer.myObjects.FindFromKey (aName.c_str(), anObj))
    {
      Message::SendFail() << "Error: unknown object '" << aName << "'";
      return false;
    }

    hasChanged = true;
    aViewer.Context()->Display (anObj, false);
  }
  if (theToUpdate
   && hasChanged)
  {
    aViewer.UpdateView();
  }
  return true;
}

// ================================================================
// Function : removeObjectList
// Purpose  :
// ================================================================
bool WasmOcctView::removeObjectList (const std::string& theNames,
                                     bool theToUpdate)
{
  WasmOcctView& aViewer = Instance();

  std::stringstream aListStream (theNames);
  std::string aName;
  bool hasChanged = false;
  while (std::getline (aListStream, aName, THE_LIST_SEPARATOR))
  {
    /// TODO filtering should be done for all keys starting with aName
    Handle(AIS_InteractiveObject) anObj;
    if (!aViewer.myObjects.FindFromKey (aName.c_str(), anObj))
    {
      //Message::SendFail() << "Error: unknown object '" << aName << "'";
      return false;
    }

    hasChanged = true;
    aViewer.Context()->Remove (anObj, false);
    aViewer.myObjects.RemoveKey (aName.c_str());
  }

  if (hasChanged)
  {
    if (theToUpdate) { aViewer.UpdateView(); }
    aViewer.onDisplayedObjectsChanged();
  }
  return true;
}

// ================================================================
// Function : eraseObjectList
// Purpose  :
// ================================================================
bool WasmOcctView::eraseObjectList (const std::string& theNames,
                                    bool theToUpdate)
{
  WasmOcctView& aViewer = Instance();

  std::stringstream aListStream (theNames);
  std::string aName;
  bool hasChanged = false;
  while (std::getline (aListStream, aName, THE_LIST_SEPARATOR))
  {
    /// TODO filtering should be done for all keys starting with aName
    Handle(AIS_InteractiveObject) anObj;
    if (!aViewer.myObjects.FindFromKey (aName.c_str(), anObj))
    {
      Message::SendFail() << "Error: unknown object '" << aName << "'";
      return false;
    }

    hasChanged = true;
    aViewer.Context()->Erase (anObj, false);
  }

  if (hasChanged
   && theToUpdate)
  {
    aViewer.UpdateView();
  }
  return true;
}

// ================================================================
// Function : removeAllObjects
// Purpose  :
// ================================================================
void WasmOcctView::removeAllObjects (bool theToUpdate)
{
  WasmOcctView& aViewer = Instance();
  if (aViewer.myObjects.IsEmpty())
  {
    return;
  }

  for (NCollection_IndexedDataMap<TCollection_AsciiString, Handle(AIS_InteractiveObject)>::Iterator anObjIter (aViewer.myObjects);
       anObjIter.More(); anObjIter.Next())
  {
    aViewer.Context()->Remove (anObjIter.Value(), false);
  }
  aViewer.myObjects.Clear();
  if (theToUpdate)
  {
    aViewer.UpdateView();
  }
  aViewer.onDisplayedObjectsChanged();
}

// ================================================================
// Function : openFromUrl
// Purpose  :
// ================================================================
void WasmOcctView::openFromUrl (const std::string& theName,
                                const std::string& theModelPath,
                                const bool theToExpand)
{
  ModelAsyncLoader* aTask = new ModelAsyncLoader (theName.c_str(), theModelPath.c_str(), theToExpand);
  emscripten_async_wget_data (theModelPath.c_str(), (void* )aTask, ModelAsyncLoader::onDataRead, ModelAsyncLoader::onReadFailed);
}

// ================================================================
// Function : openFromMemory
// Purpose  :
// ================================================================
bool WasmOcctView::openFromMemory (const std::string& theName,
                                   const bool theToExpand,
                                   uintptr_t theBuffer, int theDataLen,
                                   bool theToFree)
{
  removeObjectList (theName, false);
  char* aBytes = reinterpret_cast<char*>(theBuffer);
  if (aBytes == nullptr
   || theDataLen <= 0)
  {
    return false;
  }

  // Function to check if specified data stream starts with specified header.
  #define dataStartsWithHeader(theData, theHeader) (::strncmp(theData, theHeader, sizeof(theHeader) - 1) == 0)

  if (dataStartsWithHeader(aBytes, "DBRep_DrawableShape"))
  {
    return openBRepFromMemory (theName, theToExpand, theBuffer, theDataLen, theToFree);
  }
  else if (dataStartsWithHeader(aBytes, "glTF"))
  {
    return openGltfFromMemory (theName, theToExpand, theBuffer, theDataLen, theToFree);
  }
  if (theToFree)
  {
    free (aBytes);
  }

  Message::SendFail() << "Error: file '" << theName.c_str() << "' has unsupported format";
  return false;
}

// ================================================================
// Function : openBRepFromMemory
// Purpose  :
// ================================================================
bool WasmOcctView::openBRepFromMemory (const std::string& theName,
                                       const bool theToExpand,
                                       uintptr_t theBuffer, int theDataLen,
                                       bool theToFree)
{
  removeObjectList (theName, false);

  /*WasmOcctView& aViewer = Instance();
  TopoDS_Shape aShape;
  BRep_Builder aBuilder;
  bool isLoaded = false;
  {
    char* aRawData = reinterpret_cast<char*>(theBuffer);
    Standard_ArrayStreamBuffer aStreamBuffer (aRawData, theDataLen);
    std::istream aStream (&aStreamBuffer);
    BRepTools::Read (aShape, aStream, aBuilder);
    if (theToFree)
    {
      free (aRawData);
    }
    isLoaded = true;
  }
  if (!isLoaded)
  {
    return false;
  }

  Handle(AIS_Shape) aShapePrs = new AIS_Shape (aShape);
  if (!theName.empty())
  {
    aViewer.myObjects.Add (theName.c_str(), aShapePrs);
  }
  aShapePrs->SetMaterial (Graphic3d_NameOfMaterial_Silver);
  aViewer.Context()->Display (aShapePrs, AIS_Shaded, 0, false);
  aViewer.View()->FitAll (0.01, false);
  aViewer.UpdateView();

  Message::DefaultMessenger()->Send (TCollection_AsciiString("Loaded file ") + theName.c_str(), Message_Info);
  Message::DefaultMessenger()->Send (OSD_MemInfo::PrintInfo(), Message_Trace);
  return true;*/
  return false;
}

#include <OSD_OpenFile.hxx>
#include <RWGltf_GltfJsonParser.hxx>
#include <RWGltf_TriangulationReader.hxx>
#include "WasmOcctObject.h"

class WasmTriangulationReader : public RWGltf_TriangulationReader
{
public:

  WasmTriangulationReader (std::istream& theStream,
                           const TCollection_AsciiString& theFile)
  : myRootStream (&theStream), myRootPath (theFile)
  {
    //
  }

  virtual bool load (const Handle(RWGltf_GltfLatePrimitiveArray)& theMesh) override
  {
    reset();
    if (theMesh.IsNull()
     || theMesh->PrimitiveMode() == RWGltf_GltfPrimitiveMode_UNKNOWN)
    {
      return false;
    }

    for (NCollection_Sequence<RWGltf_GltfPrimArrayData>::Iterator aDataIter (theMesh->Data()); aDataIter.More(); aDataIter.Next())
    {
      const RWGltf_GltfPrimArrayData& aData = aDataIter.Value();
      if (!aData.StreamData.IsNull())
      {
        Standard_ArrayStreamBuffer aStreamBuffer ((const char* )aData.StreamData->Data(), aData.StreamData->Size());
        std::istream aStream (&aStreamBuffer);
        aStream.seekg ((std::streamoff )aData.StreamOffset, std::ios_base::beg);
        if (!readBuffer (aStream, theMesh->Id(), aData.Accessor, aData.Type, theMesh->PrimitiveMode()))
        {
          return false;
        }
        continue;
      }
      else if (aData.StreamUri.IsEmpty())
      {
        reportError (TCollection_AsciiString ("Buffer '") + theMesh->Id() + "' does not define uri.");
        return false;
      }

      std::istream* aStream = &mySharedStream.Stream;
      if (aData.StreamUri == myRootPath
       && myRootStream != NULL)
      {
        aStream = myRootStream;
      }
      else if (mySharedStream.Path != aData.StreamUri)
      {
        mySharedStream.Stream.close();
        mySharedStream.Path = aData.StreamUri;
      }
      if (aStream == &mySharedStream.Stream
      && !mySharedStream.Stream.is_open())
      {
        OSD_OpenStream (mySharedStream.Stream, aData.StreamUri.ToCString(), std::ios::in | std::ios::binary);
        if (!mySharedStream.Stream.is_open())
        {
          mySharedStream.Stream.close();
          reportError (TCollection_AsciiString ("Buffer '") + theMesh->Id() + "refers to non-existing file '" + aData.StreamUri + "'.");
          return false;
        }
      }

      aStream->seekg ((std::streamoff )aData.StreamOffset, std::ios_base::beg);
      if (!aStream->good())
      {
        mySharedStream.Stream.close();
        reportError (TCollection_AsciiString ("Buffer '") + theMesh->Id() + "refers to invalid location.");
        return false;
      }

      if (!readBuffer (*aStream, theMesh->Id(), aData.Accessor, aData.Type, theMesh->PrimitiveMode()))
      {
        return false;
      }
    }
    return true;
  }

private:

  std::istream* myRootStream;
  TCollection_AsciiString myRootPath;

};

//! Parse glTF data.
static bool parseGltfFromMemory (RWGltf_GltfJsonParser& theParser,
                                 std::istream& theStream,
                                 const TCollection_AsciiString& theFile)
{
  bool isBinaryFile = false;
  char aGlbHeader[12] = {};
  theStream.read (aGlbHeader, sizeof(aGlbHeader));
  int64_t aBinBodyOffset = 0, aBinBodyLen = 0, aJsonBodyOffset = 0, aJsonBodyLen = 0;
  if (::strncmp (aGlbHeader, "glTF", 4) == 0)
  {
    isBinaryFile = true;
    const uint32_t* aVer = (const uint32_t* )(aGlbHeader + 4);
    const uint32_t* aLen = (const uint32_t* )(aGlbHeader + 8);
    if (*aVer != 2)
    {
      Message::SendFail (TCollection_AsciiString ("File '") + theFile + "' is written using unknown version " + int(*aVer));
      return false;
    }

    for (int aChunkIter = 0; !theStream.eof() && aChunkIter < 2; ++aChunkIter)
    {
      char aChunkHeader2[8] = {};
      if (int64_t(theStream.tellg()) + int64_t(sizeof(aChunkHeader2)) > int64_t(*aLen))
      {
        break;
      }

      theStream.read (aChunkHeader2, sizeof(aChunkHeader2));
      if (!theStream.good())
      {
        Message::SendFail (TCollection_AsciiString ("File '") + theFile + "' is written using unsupported format");
        return false;
      }

      const uint32_t* aChunkLen  = (const uint32_t* )(aChunkHeader2 + 0);
      const uint32_t* aChunkType = (const uint32_t* )(aChunkHeader2 + 4);
      if (*aChunkType == 0x4E4F534A)
      {
        aJsonBodyOffset = int64_t(theStream.tellg());
        aJsonBodyLen    = int64_t(*aChunkLen);
      }
      else if (*aChunkType == 0x004E4942)
      {
        aBinBodyOffset = int64_t(theStream.tellg());
        aBinBodyLen    = int64_t(*aChunkLen);
      }
      if (*aChunkLen != 0)
      {
        theStream.seekg (*aChunkLen, std::ios_base::cur);
      }
    }

    theStream.seekg ((std::streamoff )aJsonBodyOffset, std::ios_base::beg);
  }
  else
  {
    theStream.seekg (0, std::ios_base::beg);
  }
  if (isBinaryFile)
  {
    theParser.SetBinaryFormat (aBinBodyOffset, aBinBodyLen);
  }

  rapidjson::ParseResult aRes;
  rapidjson::IStreamWrapper aFileStream (theStream);
  if (isBinaryFile)
  {
    aRes = theParser.ParseStream<rapidjson::kParseStopWhenDoneFlag, rapidjson::UTF8<>, rapidjson::IStreamWrapper> (aFileStream);
  }
  else
  {
    aRes = theParser.ParseStream (aFileStream);
  }
  if (aRes.IsError())
  {
    if (aRes.Code() == rapidjson::kParseErrorDocumentEmpty)
    {
      Message::SendFail (TCollection_AsciiString ("File '") + theFile + "' is empty");
      return false;
    }
    TCollection_AsciiString anErrDesc (RWGltf_GltfJsonParser::FormatParseError (aRes.Code()));
    Message::SendFail (TCollection_AsciiString ("File '") + theFile + "' defines invalid JSON document!\n"
                     + anErrDesc + " [at offset " + (int )aRes.Offset() + "].");
    return false;
  }

  if (!theParser.Parse (Message_ProgressRange()))
  {
    return false;
  }

  return true;
}

// ================================================================
// Function : openGltfFromMemory
// Purpose  :
// ================================================================
bool WasmOcctView::openGltfFromMemory (const std::string& theName,
                                       const bool theToExpand,
                                       uintptr_t theBuffer, int theDataLen,
                                       bool theToFree)
{
  removeObjectList (theName, false);

  WasmOcctView& aViewer = Instance();

  char* aRawData = reinterpret_cast<char*>(theBuffer);
  Standard_ArrayStreamBuffer aStreamBuffer (aRawData, theDataLen);
  std::istream aStream (&aStreamBuffer);

  Handle(WasmOcctObject) aShapePrs = new WasmOcctObject();

  RWMesh_CoordinateSystemConverter aTrsf;
  aTrsf.SetInputLengthUnit (1.0); // meters
  aTrsf.SetInputCoordinateSystem (RWMesh_CoordinateSystem_glTF);
  aTrsf.SetOutputLengthUnit (1.0); // meters
  aTrsf.SetOutputCoordinateSystem(RWMesh_CoordinateSystem_Zup);

  RWGltf_GltfJsonParser aParser (aShapePrs->ChangeShapes());
  aParser.SetFilePath (theName.c_str());
  aParser.SetErrorPrefix (TCollection_AsciiString ("File '") + theName.c_str() + "' defines invalid glTF!\n");
  aParser.SetAttributeMap (aShapePrs->ChangeAttributes());
  aParser.SetCoordinateSystemConverter (aTrsf);
  //aParser.SetSkipEmptyNodes (myToSkipEmptyNodes);
  //aParser.SetMeshNameAsFallback (myUseMeshNameAsFallback);
  bool isParsed = parseGltfFromMemory (aParser, aStream, theName.c_str());
  if (isParsed)
  {
    Handle(RWGltf_PrimitiveArrayReader) aReader = new WasmTriangulationReader (aStream, theName.c_str());
    aReader->SetCoordinateSystemConverter (aTrsf);
    for (NCollection_Vector<TopoDS_Face>::Iterator aFaceIter (aParser.FaceList()); aFaceIter.More(); aFaceIter.Next())
    {
      TopoDS_Face& aFace = aFaceIter.ChangeValue();
      TopLoc_Location aDummyLoc;
      Handle(RWGltf_GltfLatePrimitiveArray) aLateData = Handle(RWGltf_GltfLatePrimitiveArray)::DownCast (BRep_Tool::Triangulation (aFace, aDummyLoc));
      Handle(Poly_Triangulation) aPolyData = aReader->Load (aLateData);
      BRep_Builder aBuilder;
      aBuilder.UpdateFace (aFace, aPolyData);
    }
  }
  if (theToFree)
  {
    free (aRawData);
  }

  if (!isParsed)
  {
    return false;
  }

  if (theToExpand)
  {
    /// TODO this is just a dummy logic for testing - expanding should be done by assembly tree, not faces!
    Standard_Integer aSubIndex = 0;
    for (TopTools_SequenceOfShape::Iterator aShapeIter (aShapePrs->ChangeShapes()); aShapeIter.More(); aShapeIter.Next())
    {
      for (TopExp_Explorer aFaceIter (aShapeIter.Value(), TopAbs_FACE); aFaceIter.More(); aFaceIter.Next())
      {
        const TopoDS_Shape& aSubShape = aFaceIter.Current();
        Handle(WasmOcctObject) aSubShapePrs = new WasmOcctObject();
        aSubShapePrs->SetShape (aSubShape);
        if (const RWMesh_NodeAttributes* anAttribs = aShapePrs->ChangeAttributes().Seek (aSubShape.Located (TopLoc_Location())))
        {
          aSubShapePrs->ChangeAttributes().Bind (aSubShape.Located (TopLoc_Location()), *anAttribs);
        }
        if (!theName.empty())
        {
          ++aSubIndex;
          TCollection_AsciiString aName = TCollection_AsciiString (theName.c_str()) + "/" + aSubIndex;
          aViewer.myObjects.Add (aName, aSubShapePrs);
        }
        aViewer.Context()->Display (aSubShapePrs, 0, 0, false);
      }
    }
  }
  else
  {
    if (!theName.empty())
    {
      aViewer.myObjects.Add (theName.c_str(), aShapePrs);
    }
    aViewer.Context()->Display (aShapePrs, 0, 0, false);
  }

  aViewer.View()->FitAll (0.01, false);
  aViewer.View()->Invalidate();
  aViewer.updateView();

  setShowGround (aViewer.myToShowGround, false);

  Message::DefaultMessenger()->Send (TCollection_AsciiString("Loaded file ") + theName.c_str(), Message_Info);
  Message::DefaultMessenger()->Send (OSD_MemInfo::PrintInfo(), Message_Trace);
  if (!theName.empty())
  {
    aViewer.onDisplayedObjectsChanged();
  }
  return true;
}

// ================================================================
// Function : toShowGround
// Purpose  :
// ================================================================
bool WasmOcctView::toShowGround()
{
  return Instance().myToShowGround;
}

// ================================================================
// Function : setShowGround
// Purpose  :
// ================================================================
void WasmOcctView::setShowGround (bool theToShow,
                                  bool theToUpdate)
{
  static Handle(WasmOcctObject) aGroundPrs = new WasmOcctObject();

  WasmOcctView& aViewer = Instance();
  aViewer.myToShowGround = theToShow;
  Bnd_Box aBox;
  if (theToShow)
  {
    aViewer.Context()->Remove (aGroundPrs, false);
    aBox = aViewer.View()->View()->MinMaxValues();
  }
  if (aBox.IsVoid()
  ||  aBox.IsZThin (Precision::Confusion()))
  {
    if (!aGroundPrs.IsNull()
      && aGroundPrs->HasInteractiveContext())
    {
      aViewer.Context()->Remove (aGroundPrs, false);
      if (theToUpdate)
      {
        aViewer.UpdateView();
      }
    }
    return;
  }

  const gp_XYZ aSize   = aBox.CornerMax().XYZ() - aBox.CornerMin().XYZ();
  const double aRadius = Max (aSize.X(), aSize.Y());
  const double aZValue = aBox.CornerMin().Z() - Min (10.0, aSize.Z() * 0.01);
  const double aZSize  = aRadius * 0.01;
  gp_XYZ aGroundCenter ((aBox.CornerMin().X() + aBox.CornerMax().X()) * 0.5,
                        (aBox.CornerMin().Y() + aBox.CornerMax().Y()) * 0.5,
                         aZValue);

  TopoDS_Compound aGround;
  gp_Trsf aTrsf1, aTrsf2;
  aTrsf1.SetTranslation (gp_Vec (aGroundCenter - gp_XYZ(0.0, 0.0, aZSize)));
  aTrsf2.SetTranslation (gp_Vec (aGroundCenter));
  Prs3d_ToolCylinder aCylTool  (aRadius, aRadius, aZSize, 50, 1);
  Prs3d_ToolDisk     aDiskTool (0.0, aRadius, 50, 1);
  TopoDS_Face aCylFace, aDiskFace1, aDiskFace2;
  BRep_Builder().MakeFace (aCylFace,   aCylTool .CreatePolyTriangulation (aTrsf1));
  BRep_Builder().MakeFace (aDiskFace1, aDiskTool.CreatePolyTriangulation (aTrsf1));
  BRep_Builder().MakeFace (aDiskFace2, aDiskTool.CreatePolyTriangulation (aTrsf2));

  BRep_Builder().MakeCompound (aGround);
  BRep_Builder().Add (aGround, aCylFace);
  BRep_Builder().Add (aGround, aDiskFace1);
  BRep_Builder().Add (aGround, aDiskFace2);

  aGroundPrs->SetShape (aGround);
  aGroundPrs->SetToUpdate();
  aGroundPrs->SetMaterial (Graphic3d_NameOfMaterial_Stone);
  aGroundPrs->SetInfiniteState (false);
  aViewer.Context()->Display (aGroundPrs, 0, -1, false);
  aGroundPrs->SetInfiniteState (true);
  if (theToUpdate)
  {
    aViewer.UpdateView();
  }
}

// Module exports
EMSCRIPTEN_BINDINGS(OccViewerModule) {
  emscripten::function("toDynamicHighlight",   &WasmOcctView::toDynamicHighlight);
  emscripten::function("setDynamicHighlight",  &WasmOcctView::setDynamicHighlight);
  emscripten::function("toCastShadows",        &WasmOcctView::toCastShadows);
  emscripten::function("setCastShadows",       &WasmOcctView::setCastShadows);
  //emscripten::property("toCastShadows", &WasmOcctView::toCastShadows, &WasmOcctView::setCastShadows); // this could be only in class
  emscripten::function("isAntiAliasingOn",     &WasmOcctView::isAntiAliasingOn);
  emscripten::function("setAntiAliasingOn",    &WasmOcctView::setAntiAliasingOn);
  emscripten::function("setBackgroundColor",   &WasmOcctView::setBackgroundColor);
  emscripten::function("setBackgroundCubemap", &WasmOcctView::setBackgroundCubemap);
  emscripten::function("fitAllObjects",        &WasmOcctView::fitAllObjects);
  emscripten::function("removeAllObjects",     &WasmOcctView::removeAllObjects);
  emscripten::function("removeObjectList",     &WasmOcctView::removeObjectList);
  emscripten::function("eraseObjectList",      &WasmOcctView::eraseObjectList);
  emscripten::function("displayedObjects",     &WasmOcctView::displayedObjects);
  emscripten::function("selectedObjects",      &WasmOcctView::selectedObjects);
  emscripten::function("erasedObjects",        &WasmOcctView::erasedObjects);
  emscripten::function("displayObjectList",    &WasmOcctView::displayObjectList);
  emscripten::function("toShowGround",         &WasmOcctView::toShowGround);
  emscripten::function("setShowGround",        &WasmOcctView::setShowGround);
  emscripten::function("openFromUrl",          &WasmOcctView::openFromUrl);
  emscripten::function("openFromMemory",       &WasmOcctView::openFromMemory, emscripten::allow_raw_pointers());
  emscripten::function("openBRepFromMemory",   &WasmOcctView::openBRepFromMemory, emscripten::allow_raw_pointers());
  emscripten::function("openGltfFromMemory",   &WasmOcctView::openGltfFromMemory, emscripten::allow_raw_pointers());
}
