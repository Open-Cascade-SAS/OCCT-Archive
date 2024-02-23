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

#include "OpenGl_OcclusionQuery.hxx"

#include <OpenGl_Context.hxx>
#include <OpenGl_GlCore15.hxx>

IMPLEMENT_STANDARD_RTTIEXT(OpenGl_OcclusionQuery, Graphic3d_OcclusionQuery)

// =======================================================================
// function :
// purpose  :
// =======================================================================
Standard_EXPORT
OpenGl_OcclusionQuery::OpenGl_OcclusionQuery(const Handle(OpenGl_Context) &
                                             theCtx)
    : aCtx(theCtx) {
  aCtx->core15->glGenQueries(1, &queryID);
}

// =======================================================================
// function :
// purpose  :
// =======================================================================
Standard_EXPORT OpenGl_OcclusionQuery::~OpenGl_OcclusionQuery() {
  aCtx->core15->glDeleteQueries(1, &queryID);
  queryID = 0;
}

// =======================================================================
// function : BeginQuery()
// purpose  :
// =======================================================================
Standard_EXPORT void OpenGl_OcclusionQuery::BeginQuery() const {
  aCtx->core15->glBeginQuery(GL_SAMPLES_PASSED, queryID);
}

// =======================================================================
// function : EndQuery()
// purpose  :
// =======================================================================
Standard_EXPORT void OpenGl_OcclusionQuery::EndQuery() {
  aCtx->core15->glEndQuery(GL_SAMPLES_PASSED);
  aCtx->core15->glGetQueryObjectiv(queryID, GL_QUERY_RESULT, &samplesPassed);
}
