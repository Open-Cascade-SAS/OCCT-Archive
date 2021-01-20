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

#ifndef _WasmOcctView_HeaderFile
#define _WasmOcctView_HeaderFile

#include <AIS_InteractiveContext.hxx>
#include <AIS_ViewController.hxx>
#include <V3d_View.hxx>

#include <emscripten.h>
#include <emscripten/html5.h>

class AIS_ViewCube;

//! Sample class creating 3D Viewer within Emscripten canvas.
class WasmOcctView : protected AIS_ViewController
{
public:

  //! Return global viewer instance.
  static WasmOcctView& Instance();

public: //! @name methods exported by Module

  //! Returns TRUE if dynamic highlighting is turned ON.
  static bool toDynamicHighlight();

  //! Set if dynamic highlighting should be enabled or not.
  static void setDynamicHighlight (bool theToEnable);

  //! Returns TRUE if shadows are turned ON.
  static bool toCastShadows();

  //! Turn shadows on/off.
  static void setCastShadows (bool theUseShadows,
                              bool theToUpdate);

  //! Returns TRUE if anti-aliasing is turned ON.
  static bool isAntiAliasingOn();

  //! Turn antialiasing on/off.
  static void setAntiAliasingOn (bool theToEnable, bool theToUpdate);

  //! Set solid color background.
  static void setBackgroundColor (float theR, float theG, float theB,
                                  bool theToUpdate);

  //! Set cubemap background.
  //! File will be loaded asynchronously.
  //! @param theImagePath [in] image path to load
  static void setBackgroundCubemap (const std::string& theImagePath);

  //! Clear all named objects from viewer.
  static void removeAllObjects (bool theToUpdate);

  //! Fit all/selected objects into view.
  //! @param theAuto [in] fit selected objects (TRUE) or all objects (FALSE)
  static void fitAllObjects (bool theAuto,
                             bool theToUpdate);

  //! Return the list of displayed objects.
  static std::string displayedObjects();

  //! Return the list of selected objects.
  static std::string selectedObjects();

  //! Return the list of erased objects.
  static std::string erasedObjects();

  //! Display temporarily hidden object.
  //! @param theNames [in] object name list
  //! @return FALSE if object was not found
  static bool displayObjectList (const std::string& theNames,
                                 bool theToUpdate);

  //! Remove named objects from viewer.
  //! @param theNames [in] object name found
  //! @return FALSE if object was not found
  static bool removeObjectList (const std::string& theNames,
                                bool theToUpdate);

  //! Temporarily hide named objects.
  //! @param theNames [in] object name list
  //! @return FALSE if object was not found
  static bool eraseObjectList (const std::string& theNames,
                               bool theToUpdate);

  //! Return TRUE if ground is displayed.
  static bool toShowGround();

  //! Show/hide ground.
  //! @param theToShow [in] show or hide flag
  static void setShowGround (bool theToShow,
                             bool theToUpdate);

  //! Open object from the given URL.
  //! File will be loaded asynchronously.
  //! @param theName      [in] object name
  //! @param theModelPath [in] model path
  //! @param theToExpand  [in] expand model or display as a single part
  static void openFromUrl (const std::string& theName,
                           const std::string& theModelPath,
                           const bool theToExpand);

  //! Open object from memory.
  //! @param theName     [in] object name
  //! @param theToExpand [in] expand model or display as a single part
  //! @param theBuffer   [in] pointer to data
  //! @param theDataLen  [in] data length
  //! @param theToFree   [in] free theBuffer if set to TRUE
  //! @return FALSE on reading error
  static bool openFromMemory (const std::string& theName,
                              const bool theToExpand,
                              uintptr_t theBuffer, int theDataLen,
                              bool theToFree);

  //! Open BRep object from memory.
  //! @param theName     [in] object name
  //! @param theToExpand [in] expand model or display as a single part
  //! @param theBuffer   [in] pointer to data
  //! @param theDataLen  [in] data length
  //! @param theToFree   [in] free theBuffer if set to TRUE
  //! @return FALSE on reading error
  static bool openBRepFromMemory (const std::string& theName,
                                  const bool theToExpand,
                                  uintptr_t theBuffer, int theDataLen,
                                  bool theToFree);

  //! Open glTF object from memory.
  //! @param theName     [in] object name
  //! @param theToExpand [in] expand model or display as a single part
  //! @param theBuffer   [in] pointer to data
  //! @param theDataLen  [in] data length
  //! @param theToFree   [in] free theBuffer if set to TRUE
  //! @return FALSE on reading error
  static bool openGltfFromMemory (const std::string& theName,
                                  const bool theToExpand,
                                  uintptr_t theBuffer, int theDataLen,
                                  bool theToFree);

  //! Displayed map changed notification.
  void onDisplayedObjectsChanged();

public:

  //! Default constructor.
  WasmOcctView();

  //! Destructor.
  virtual ~WasmOcctView();

  //! Main application entry point.
  void run();

  //! Return interactive context.
  const Handle(AIS_InteractiveContext)& Context() const { return myContext; }

  //! Return view.
  const Handle(V3d_View)& View() const { return myView; }

  //! Return device pixel ratio for handling high DPI displays.
  float DevicePixelRatio() const { return myDevicePixelRatio; }

  //! Request view redrawing.
  void UpdateView();

private:

  //! Create window.
  void initWindow();

  //! Create 3D Viewer.
  bool initViewer();

  //! Fill 3D Viewer with a DEMO items.
  void initDemoScene();

  //! Application event loop.
  void mainloop();

  //! Request view redrawing.
  void updateView();

  //! Flush events and redraw view.
  void redrawView();

  //! Handle view redraw.
  virtual void handleViewRedraw (const Handle(AIS_InteractiveContext)& theCtx,
                                 const Handle(V3d_View)& theView) override;

  //! Dump WebGL context information.
  void dumpGlInfo (bool theIsBasic);

  //! Initialize pixel scale ratio.
  void initPixelScaleRatio();

  //! Return point from logical units to backing store.
  Graphic3d_Vec2d convertPointToBacking (const Graphic3d_Vec2d& thePnt) const
  {
    return thePnt * myDevicePixelRatio;
  }

  //! Return point from logical units to backing store.
  Graphic3d_Vec2i convertPointToBacking (const Graphic3d_Vec2i& thePnt) const
  {
    Graphic3d_Vec2d aPnt = Graphic3d_Vec2d (thePnt) * myDevicePixelRatio + Graphic3d_Vec2d (0.5);
    return Graphic3d_Vec2i (aPnt);
  }

//! @name Emscripten callbacks
private:
  //! Window resize event.
  EM_BOOL onResizeEvent (int theEventType, const EmscriptenUiEvent* theEvent);

  //! Mouse event.
  EM_BOOL onMouseEvent (int theEventType, const EmscriptenMouseEvent* theEvent);

  //! Scroll event.
  EM_BOOL onWheelEvent (int theEventType, const EmscriptenWheelEvent* theEvent);

  //! Touch event.
  EM_BOOL onTouchEvent (int theEventType, const EmscriptenTouchEvent* theEvent);

  //! Key down event.
  EM_BOOL onKeyDownEvent (int theEventType, const EmscriptenKeyboardEvent* theEvent);

  //! Key up event.
  EM_BOOL onKeyUpEvent (int theEventType, const EmscriptenKeyboardEvent* theEvent);

//! @name Emscripten callbacks (static functions)
private:

  static EM_BOOL onResizeCallback (int theEventType, const EmscriptenUiEvent* theEvent, void* theView)
  { return ((WasmOcctView* )theView)->onResizeEvent (theEventType, theEvent); }

  static void onRedrawView (void* theView)
  { return ((WasmOcctView* )theView)->redrawView(); }

  static EM_BOOL onMouseCallback (int theEventType, const EmscriptenMouseEvent* theEvent, void* theView)
  { return ((WasmOcctView* )theView)->onMouseEvent (theEventType, theEvent); }

  static EM_BOOL onWheelCallback (int theEventType, const EmscriptenWheelEvent* theEvent, void* theView)
  { return ((WasmOcctView* )theView)->onWheelEvent (theEventType, theEvent); }

  static EM_BOOL onTouchCallback (int theEventType, const EmscriptenTouchEvent* theEvent, void* theView)
  { return ((WasmOcctView* )theView)->onTouchEvent (theEventType, theEvent); }

  static EM_BOOL onKeyDownCallback (int theEventType, const EmscriptenKeyboardEvent* theEvent, void* theView)
  { return ((WasmOcctView* )theView)->onKeyDownEvent (theEventType, theEvent); }

  static EM_BOOL onKeyUpCallback (int theEventType, const EmscriptenKeyboardEvent* theEvent, void* theView)
  { return ((WasmOcctView* )theView)->onKeyUpEvent (theEventType, theEvent); }

  //! Callback called by handleMoveTo() on Selection in 3D Viewer.
  //! This method is expected to be called from rendering thread.
  virtual void OnSelectionChanged (const Handle(AIS_InteractiveContext)& theCtx,
                                   const Handle(V3d_View)& theView) override;

private:

  //! Register hot-keys for specified Action.
  void addActionHotKeys (Aspect_VKey theAction,
                         unsigned int theHotKey1 = 0,
                         unsigned int theHotKey2 = 0,
                         unsigned int theHotKey3 = 0,
                         unsigned int theHotKey4 = 0,
                         unsigned int theHotKey5 = 0)
  {
    if (theHotKey1 != 0) { myNavKeyMap.Bind (theHotKey1, theAction); }
    if (theHotKey2 != 0) { myNavKeyMap.Bind (theHotKey2, theAction); }
    if (theHotKey3 != 0) { myNavKeyMap.Bind (theHotKey3, theAction); }
    if (theHotKey4 != 0) { myNavKeyMap.Bind (theHotKey4, theAction); }
    if (theHotKey5 != 0) { myNavKeyMap.Bind (theHotKey5, theAction); }
  }

  //! Handle navigation keys.
  bool navigationKeyModifierSwitch (unsigned int theModifOld,
                                    unsigned int theModifNew,
                                    double       theTimeStamp);

  //! Handle hot-key.
  bool processKeyPress (Aspect_VKey theKey);

private:

  NCollection_IndexedDataMap<TCollection_AsciiString, Handle(AIS_InteractiveObject)> myObjects; //!< map of named objects
  bool myToShowGround = true;


  NCollection_DataMap<unsigned int, Aspect_VKey> myNavKeyMap; //!< map of Hot-Key (key+modifiers) to Action

  Handle(AIS_InteractiveContext) myContext;          //!< interactive context
  Handle(V3d_View)               myView;             //!< 3D view
  Handle(Prs3d_TextAspect)       myTextStyle;        //!< text style for OSD elements
  Handle(AIS_ViewCube)           myViewCube;         //!< view cube object
  TCollection_AsciiString        myCanvasId;         //!< canvas element id on HTML page
  Aspect_Touch                   myClickTouch;       //!< single touch position for handling clicks
  OSD_Timer                      myDoubleTapTimer;   //!< timer for handling double tap
  float                          myDevicePixelRatio; //!< device pixel ratio for handling high DPI displays
  unsigned int                   myUpdateRequests;   //!< counter for unhandled update requests

};

#endif // _WasmOcctView_HeaderFile
