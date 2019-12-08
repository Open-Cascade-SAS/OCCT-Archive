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

#ifndef _SelectMgr_DisabledObjectsFilter_HeaderFile
#define _SelectMgr_DisabledObjectsFilter_HeaderFile

#include <Standard.hxx>
#include <Standard_Type.hxx>

#include <Graphic3d_NMapOfTransient.hxx>
#include <Standard_Boolean.hxx>

class SelectMgr_EntityOwner;


class SelectMgr_DisabledObjectsFilter;
DEFINE_STANDARD_HANDLE(SelectMgr_DisabledObjectsFilter, SelectMgr_Filter)

//! A framework to define an or selection filter.
//! This selects one or another type of sensitive entity.
class SelectMgr_DisabledObjectsFilter : public SelectMgr_Filter
{

public:

  //! Constructs an empty or selection filter.
  Standard_EXPORT SelectMgr_DisabledObjectsFilter();
  
  Standard_EXPORT Standard_Boolean IsOk (const Handle(SelectMgr_EntityOwner)& anobj) const Standard_OVERRIDE;

  //! Disable selection of specified objects.
  Standard_EXPORT void SetDisabledObjects (const Handle(Graphic3d_NMapOfTransient)& theObjects);


  DEFINE_STANDARD_RTTIEXT(SelectMgr_DisabledObjectsFilter, SelectMgr_Filter)

private:

  Handle(Graphic3d_NMapOfTransient) myDisabledObjects;


};







#endif // _SelectMgr_DisabledObjectsFilter_HeaderFile
