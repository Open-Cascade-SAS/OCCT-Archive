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

#include <inspector/ViewControl_PropertiesDialog.hxx>

#include <inspector/ViewControl_TableModel.hxx>
#include <inspector/TreeModel_ItemProperties.hxx>

#include <Standard_WarningsDisable.hxx>
#include <QGridLayout>
#include <QPushButton>
#include <QWidget>
#include <Standard_WarningsRestore.hxx>

// =======================================================================
// function : Constructor
// purpose :
// =======================================================================
ViewControl_PropertiesDialog::ViewControl_PropertiesDialog (QWidget* theParent)
: QDialog (theParent)
{
  setWindowTitle ("Properties");

  QGridLayout* aLayout = new QGridLayout (this);
  myTable = new ViewControl_Table (this);
  ViewControl_TableModel* aModel = new ViewControl_TableModel (myTable->TableView());
  myTable->SetModel (aModel);

  aLayout->addWidget(myTable->TableView(), 0, 0, 1, 3);

  myOkButton = new QPushButton ("Ok", this);
  myCancelButton = new QPushButton ("Cancel", this);
  connect (myOkButton, SIGNAL (clicked()), this, SLOT (accept()));
  connect (myCancelButton, SIGNAL (clicked()), this, SLOT (reject()));
  aLayout->addWidget (myOkButton, 1, 1);
  aLayout->addWidget (myCancelButton, 1, 2);

  aLayout->setColumnStretch (0, 1);

  myCancelButton->setDefault (true);

  setMinimumSize (200, 600);
}

// =======================================================================
// function : Init
// purpose :
// =======================================================================
void ViewControl_PropertiesDialog::Init (const Handle(TreeModel_ItemProperties)& theProperties)
{
  ViewControl_TableModelValues* aTableValues = new ViewControl_TableModelValues();
  aTableValues->SetProperties (theProperties);

  myTable->Init (aTableValues);
}
