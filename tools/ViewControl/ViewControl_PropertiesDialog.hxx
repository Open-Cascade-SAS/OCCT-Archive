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

#ifndef ViewControl_PropertiesDialog_H
#define ViewControl_PropertiesDialog_H

#include <Standard.hxx>
#include <Standard_Macro.hxx>

#include <inspector/ViewControl_Table.hxx>

#include <Standard_WarningsDisable.hxx>
#include <QDialog>
#include <Standard_WarningsRestore.hxx>

class TreeModel_ItemProperties;

class QWidget;
class QPushButton;

//! \class ViewControl_PropertiesDialog
//! Dialog providing table of stream values
class ViewControl_PropertiesDialog : public QDialog
{
  Q_OBJECT
public:

  //! Constructor
  Standard_EXPORT ViewControl_PropertiesDialog (QWidget* theParent);

  //! Destructor
  virtual ~ViewControl_PropertiesDialog() {}

  Standard_EXPORT void Init (const Handle(TreeModel_ItemProperties)& theProperties);

private:
  ViewControl_Table* myTable; //!< table view

  QPushButton* myOkButton; //!< accept button
  QPushButton* myCancelButton; //!< reject button
};


#endif
