// Created on: 2024-02-20
// Created by: Hossam Ali
// Copyright (c) 2024 OPEN CASCADE SAS
//
// This file is part of Open CASCADE Technology software library.
//
// This library is free software; you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License version 2.1 as published
// by the Free Software Foundation, with special exception defined in the file
// OCCT_LGPL_EXCEPTION.txt. Consult the file LICENSE_LGPL_21.txt included in
// OCCT distribution for complete text of the license and disclaimer of any
// warranty.
//
// Alternatively, this file may be used under the terms of Open CASCADE
// commercial license or contractual agreement.

#ifndef _Graphic3d_OcclusionQuery_HeaderFile
#define _Graphic3d_OcclusionQuery_HeaderFile

#include <Standard.hxx>
#include <Standard_Transient.hxx>
#include <Standard_Type.hxx>

//! Base class provides access to occlusion query functionality.
class Graphic3d_OcclusionQuery : public Standard_Transient {
public:
  //! Empty constructor.
  Graphic3d_OcclusionQuery() : queryID(0), samplesPassed(0) {}

  //! Begins occlusion query. Until the query is ended, samples that pass the
  //! rendering pipeline are counted.
  Standard_EXPORT virtual void BeginQuery() const = 0;

  //! Ends occlusion query and caches the result - number of samples that passed
  //! the rendering pipeline.
  Standard_EXPORT virtual void EndQuery() = 0;

  //! Gets number of samples that have passed the rendering pipeline.
  unsigned int GetNumSamplesPassed() const;

  //! Helper method that returns if any samples have passed the rendering
  //! pipeline.
  bool AnySamplesPassed() const;

  //! Dumps the content of me into the stream
  Standard_EXPORT void DumpJson(Standard_OStream &theOStream,
                                Standard_Integer theDepth = -1) const;

protected:
  unsigned int queryID; // Query object ID
  int samplesPassed;    // Number of samples passed in last query

public:
  DEFINE_STANDARD_RTTIEXT(Graphic3d_OcclusionQuery, Standard_Transient)
};

DEFINE_STANDARD_HANDLE(Graphic3d_OcclusionQuery, Standard_Transient)

#endif // _Graphic3d_OcclusionQuery_HeaderFile
