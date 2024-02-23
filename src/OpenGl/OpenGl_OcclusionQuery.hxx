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

#ifndef OpenGl_View_HeaderFile
#define OpenGl_View_HeaderFile

#include <Graphic3d_OcclusionQuery.hxx>

class OpenGl_Context;

DEFINE_STANDARD_HANDLE(OpenGl_OcclusionQuery, Graphic3d_OcclusionQuery)

//! Implementation of OpenGl view.
class OpenGl_OcclusionQuery : public Graphic3d_OcclusionQuery {

public:
  //! Constructor.
  Standard_EXPORT OpenGl_OcclusionQuery(const Handle(OpenGl_Context) & theCtx);

  //! Default destructor.
  Standard_EXPORT virtual ~OpenGl_OcclusionQuery();

  //! Begins occlusion query.
  Standard_EXPORT virtual void BeginQuery() const Standard_OVERRIDE;

  //! Ends occlusion query and caches the result
  Standard_EXPORT virtual void EndQuery() Standard_OVERRIDE;

public:
  DEFINE_STANDARD_ALLOC
  DEFINE_STANDARD_RTTIEXT(OpenGl_OcclusionQuery,
                          Graphic3d_OcclusionQuery) // Type definition

private:
  const Handle(OpenGl_Context) & aCtx;
};

#endif // _OpenGl_View_Header
