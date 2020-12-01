// Created on: 2020-12-01
// Created by: Svetlana SHUTINA
// Copyright (c) 2017 OPEN CASCADE SAS
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

#pragma once

#include <inspector/TreeModel_ItemBase.hxx>

#include <Message_Alert.hxx>

#include <QAbstractTableModel>

//! @class MessageView_UnitByNameModel
//! Table model that sums the number of calls and
//! the time spent on the functionality inside the value.
//! It visualizes container of string values,
//! count of the values and time, sorted by descending time.
class MessageView_UnitByNameModel : public QAbstractTableModel
{
public:

  // Consists the table fields.
  struct RowValues
  {
    QString myName; //!< string values.
    int myCounter;  //!< count of the values.
    double myTime;  //!< total time.
  };

  //! Constructor
  MessageView_UnitByNameModel (QObject* theParent = 0) : QAbstractTableModel (theParent)
  {}

  //! Destructor
  virtual ~MessageView_UnitByNameModel()
  {}

  //! Fills map of the fields values.
  //! @param theItemBase a parent item.
  void Init (const TreeModel_ItemBasePtr theItemBase);

  //! Returns content of the model index for the given role,
  //! it is obtained from internal container of values.
  //! It returns value only for DisplayRole.
  //! @param theIndex a model index.
  //! @param theRole a view role.
  //! @return value intepreted depending on the given role.
  virtual QVariant data (const QModelIndex& theIndex, int theRole = Qt::DisplayRole) const Standard_OVERRIDE;

  //! Returns number of rows.
  //! @param theParent an index of the parent item.
  //! @return an integer value.
  virtual int rowCount (const QModelIndex& theParent = QModelIndex()) const Standard_OVERRIDE
  { 
    (void)theParent;
    return myValues.size();
  }

  //! Returns number of columns.
  //! @param theParent an index of the parent item.
  //! @return an integer value.
  virtual int columnCount (const QModelIndex& theParent = QModelIndex()) const Standard_OVERRIDE
  { (void)theParent; return 3; }

private:

  //! Sortes values and fills map of the fields values depends on unique text identifier.
  //! @param theAlert unique text identifier.
  void initByAlert (const Handle(Message_Alert)& theAlert);

  //! @param theName the unique text identifier.
  //! @return serial number in the map for the unique text identifier.
  int getValueIndex (const QString theName);

  //! Adds theValues in the map to position theIndex.
  //! If theIndex is -1, the element will be added in the end of the map.
  //! @param theIndex the serial number in the map.
  //! @param theValues the field values.
  void setValueByIndex (const int theIndex, const RowValues theValues);

private:

  QMap<QString, QPair<int, double> > myValues; //!< map of fields values.
  QMap<int, RowValues> mySortValues;              //!< sorted map of fields values.
};
