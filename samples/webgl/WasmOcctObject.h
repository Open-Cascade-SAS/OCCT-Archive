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

#ifndef _WasmOcctObject_HeaderFile
#define _WasmOcctObject_HeaderFile

#include <AIS_InteractiveObject.hxx>
#include <RWMesh_NodeAttributes.hxx>
#include <TopTools_SequenceOfShape.hxx>

//! Sample presentation.
class WasmOcctObject : public AIS_InteractiveObject
{
public:
  //! Default constructor.
  Standard_EXPORT WasmOcctObject();

  //! Destructor.
  Standard_EXPORT virtual ~WasmOcctObject();

  //! Return sequence of root shapes.
  TopTools_SequenceOfShape& ChangeShapes() { return myRootShapes; }

  //! Return shape attributes.
  RWMesh_NodeAttributeMap& ChangeAttributes() { return myAttribMap; }

  //! Set a single shape.
  void SetShape (const TopoDS_Shape& theShape)
  {
    myRootShapes.Clear();
    myRootShapes.Append (theShape);
  }

protected:

  //! Return TRUE for supported display mode.
  virtual Standard_Boolean AcceptDisplayMode (const Standard_Integer theMode) const Standard_OVERRIDE { return theMode == 0; }

  //! Compute 3D part of View Cube.
  //! @param thePrsMgr [in] presentation manager.
  //! @param thePrs [in] input presentation that is to be filled with flat presentation primitives.
  //! @param theMode [in] display mode.
  //! @warning this object accept only 0 display mode.
  Standard_EXPORT virtual void Compute (const Handle(PrsMgr_PresentationManager3d)& thePrsMgr,
                                        const Handle(Prs3d_Presentation)& thePrs,
                                        const Standard_Integer theMode) Standard_OVERRIDE;

  //! Redefine computing of sensitive entities for View Cube.
  //! @param theSelection [in] input selection object that is to be filled with sensitive entities.
  //! @param theMode [in] selection mode.
  //! @warning object accepts only 0 selection mode.
  Standard_EXPORT virtual void ComputeSelection (const Handle(SelectMgr_Selection)& theSel,
                                                 const Standard_Integer theMode) Standard_OVERRIDE;

private:

  TopTools_SequenceOfShape myRootShapes; //!< sequence of result root shapes
  RWMesh_NodeAttributeMap  myAttribMap;  //!< shape attributes

};

#endif // _WasmOcctObject_HeaderFile
