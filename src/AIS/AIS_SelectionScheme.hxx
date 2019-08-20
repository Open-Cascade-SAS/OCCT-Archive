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


#ifndef _AIS_SelectionScheme_HeaderFile
#define _AIS_SelectionScheme_HeaderFile


//! Sets selection schemes for interactive contexts.
//! It is possible to use combination of schemes.
enum AIS_SelectionScheme
{
  AIS_SelectionScheme_Empty         = 0x0000, // do nothing
  AIS_SelectionScheme_Clear         = 0x0001, // clears current selection
  AIS_SelectionScheme_Add           = 0x0002, // add detected object to current selection
  AIS_SelectionScheme_Switch        = 0x0004, // switch selection state in values selected/deselected
  AIS_SelectionScheme_PickedIfEmpty = 0x0008, // if after switch, result selection is empty, select picked objects
  AIS_SelectionScheme_ClearAndSwitch = AIS_SelectionScheme_Clear | AIS_SelectionScheme_Switch,
  AIS_SelectionScheme_ClearAndAdd = AIS_SelectionScheme_Clear | AIS_SelectionScheme_Add,
  AIS_SelectionScheme_ClearAndSwitchAndPicked = AIS_SelectionScheme_ClearAndSwitch | AIS_SelectionScheme_PickedIfEmpty,
  AIS_SelectionScheme_Custom // reserved item for custom selection scheme
};

#endif // _AIS_SelectionScheme_HeaderFile
