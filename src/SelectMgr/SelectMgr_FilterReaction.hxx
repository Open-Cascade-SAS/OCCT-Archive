// Copyright (c) 2019 OPEN CASCADE SAS
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

#ifndef _SelectMgr_FilterReaction_HeaderFile
#define _SelectMgr_FilterReaction_HeaderFile

//! Enumeration defines picking reaction - which action causes the filter check
enum SelectMgr_FilterReaction
{
  SelectMgr_FilterReaction_Empty = 0x0000, //!< do nothing
  SelectMgr_FilterReaction_Highlight = 0x0001, //!< the reaction to highlght presentation (MoveTo)
  SelectMgr_FilterReaction_Select = 0x0002, //!< the reaction to highlght presentation (Select)
  SelectMgr_FilterReaction_SelectAndHighlight = SelectMgr_FilterReaction_Highlight | SelectMgr_FilterReaction_Select
};

#endif // _SelectMgr_FilterReaction_HeaderFile
