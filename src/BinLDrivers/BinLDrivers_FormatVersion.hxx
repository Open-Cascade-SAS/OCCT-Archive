// Copyright (c) 2020 OPEN CASCADE SAS
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

#ifndef _BinLDriversFormatVersion_HeaderFile
#define _BinLDriversFormatVersion_HeaderFile

//! Defined BinLDrivers format version
enum BinLDrivers_FormatVersion
{
  BIN_LDRIVERS_VERSION_2 = 2, //!< First supported version
  BIN_LDRIVERS_VERSION_3,     //!< Add Delta to numbers data, changes in ShapeSection
  BIN_LDRIVERS_VERSION_4,     //!< entry, ContextLabel for tree
  BIN_LDRIVERS_VERSION_5,     //!< Convert old format to new
  BIN_LDRIVERS_VERSION_6,     //!< Add location
  BIN_LDRIVERS_VERSION_7,     //!< Add orientation, type migration
  BIN_LDRIVERS_VERSION_8,     //!< Stop convert old format
  BIN_LDRIVERS_VERSION_9,     //!< Add GUIDs, ReadTOC changed
  BIN_LDRIVERS_VERSION_10,    //!< Process user defined guid
  BIN_LDRIVERS_VERSION_11     //!< Add normals to Shape
};

#endif
