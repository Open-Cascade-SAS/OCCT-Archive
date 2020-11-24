// Created on: 2017-06-16
// Created by: Natalia ERMOLAEVA
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

#include <inspector/MessageView_Window.hxx>
#include <inspector/MessageView_VisibilityState.hxx>
#include <inspector/MessageView_ActionsTest.hxx>

#include <inspector/MessageModel_Actions.hxx>
#include <inspector/MessageModel_ItemAlert.hxx>
#include <inspector/MessageModel_ItemReport.hxx>
#include <inspector/MessageModel_ItemRoot.hxx>
#include <inspector/MessageModel_Tools.hxx>
#include <inspector/MessageModel_TreeModel.hxx>

#include <inspector/TreeModel_ContextMenu.hxx>
#include <inspector/TreeModel_Tools.hxx>

#include <inspector/ViewControl_PropertyView.hxx>
#include <inspector/ViewControl_TableModelValues.hxx>
#include <inspector/ViewControl_TreeView.hxx>
#include <inspector/Convert_Tools.hxx>

#include <inspector/View_Viewer.hxx>
#include <inspector/View_Widget.hxx>

#include <AIS_Shape.hxx>
#include <Graphic3d_Camera.hxx>
#include <OSD_Environment.hxx>
#include <OSD_Directory.hxx>
#include <OSD_Protection.hxx>
#include <Message.hxx>
#include <TCollection_AsciiString.hxx>

//#define DEBUG_ALERTS

//#include <XmlDrivers_MessageReportStorage.hxx>

#include <inspector/ViewControl_Tools.hxx>
#include <inspector/View_Displayer.hxx>
#include <inspector/View_ToolBar.hxx>
#include <inspector/View_Widget.hxx>
#include <inspector/View_Window.hxx>
#include <inspector/View_Viewer.hxx>

#include <BRep_Builder.hxx>
#include <BRepTools.hxx>
#include <OSD_File.hxx>
#include <OSD_Path.hxx>

#include <QApplication>
#include <QAction>
#include <QComboBox>
#include <QDockWidget>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QHeaderView>
#include <QMainWindow>
#include <QMenu>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QTextStream>
#include <QToolBar>
#include <QToolButton>
#include <QTreeView>
#include <QWidget>
#include <QVBoxLayout>

const int DEFAULT_TEXT_VIEW_WIDTH = 800;// 800;
const int DEFAULT_TEXT_VIEW_HEIGHT = 700;
const int DEFAULT_TEXT_VIEW_POSITION_X = 430;
const int DEFAULT_TEXT_VIEW_POSITION_Y = 30;
const int DEFAULT_TEXT_VIEW_DELTA = 100;

const int DEFAULT_SHAPE_VIEW_WIDTH = 400;// 900;
const int DEFAULT_SHAPE_VIEW_HEIGHT = 450;
const int DEFAULT_SHAPE_VIEW_POSITION_X = 60;
const int DEFAULT_SHAPE_VIEW_POSITION_Y = 60;

const int DEFAULT_DETACH_POSITION_X = 5;
const int DEFAULT_DETACH_POSITION_Y = 450;

const int MESSAGEVIEW_DEFAULT_TREE_VIEW_WIDTH = 950; //600
const int MESSAGEVIEW_DEFAULT_TREE_VIEW_HEIGHT = 500;

const int MESSAGEVIEW_DEFAULT_VIEW_WIDTH = 200;// 400;
const int MESSAGEVIEW_DEFAULT_VIEW_HEIGHT = 300;// 1000;

#include <Prs3d_PointAspect.hxx>
#include <Prs3d_ShadingAspect.hxx>
Handle(Prs3d_Drawer) GetPreviewAttributes (const Handle(AIS_InteractiveContext)& theContext)
{
  Handle(Prs3d_Drawer) myDrawer = new Prs3d_Drawer();
  myDrawer->Link (theContext->DefaultDrawer());

  Quantity_Color aColor(Quantity_NOC_TOMATO);//Quantity_NOC_GREENYELLOW));//Quantity_NOC_BLUE1));
  Standard_ShortReal aTransparency = 0.8;

  // point parameters
  myDrawer->SetPointAspect (new Prs3d_PointAspect (Aspect_TOM_O_PLUS, aColor, 3.0));

  // shading parameters
  Graphic3d_MaterialAspect aShadingMaterial;
  aShadingMaterial.SetReflectionModeOff (Graphic3d_TOR_SPECULAR);
  aShadingMaterial.SetMaterialType (Graphic3d_MATERIAL_ASPECT);

  myDrawer->SetShadingAspect (new Prs3d_ShadingAspect());
  myDrawer->ShadingAspect()->Aspect()->SetInteriorStyle (Aspect_IS_SOLID);
  myDrawer->ShadingAspect()->SetColor (aColor);
  myDrawer->ShadingAspect()->SetMaterial (aShadingMaterial);

  myDrawer->ShadingAspect()->Aspect()->ChangeFrontMaterial().SetTransparency (aTransparency);
  myDrawer->ShadingAspect()->Aspect()->ChangeBackMaterial() .SetTransparency (aTransparency);
  myDrawer->SetTransparency (aTransparency);

  // common parameters
  myDrawer->SetZLayer (Graphic3d_ZLayerId_Topmost);

  return myDrawer;
}

// =======================================================================
// function : Constructor
// purpose :
// =======================================================================
MessageView_Window::MessageView_Window (QWidget* theParent)
: QObject (theParent)
{
  myMainWindow = new QMainWindow (theParent);

  myTreeView = new ViewControl_TreeView (myMainWindow);
  ((ViewControl_TreeView*)myTreeView)->SetPredefinedSize (QSize (MESSAGEVIEW_DEFAULT_TREE_VIEW_WIDTH,
                                                                 MESSAGEVIEW_DEFAULT_TREE_VIEW_HEIGHT));
  MessageModel_TreeModel* aModel = new MessageModel_TreeModel (myTreeView);
  aModel->InitColumns();
  //aModel->SetReversed (Standard_True);

  connect (myTreeView->header(), SIGNAL (sectionResized (int, int, int)),
           this, SLOT(onHeaderResized (int, int, int)));

  myTreeView->setModel (aModel);
  MessageView_VisibilityState* aVisibilityState = new MessageView_VisibilityState (aModel);
  aModel->SetVisibilityState (aVisibilityState);
  connect (aVisibilityState, SIGNAL (itemClicked (const QModelIndex&)), this, SLOT(onTreeViewVisibilityClicked(const QModelIndex&)));

  TreeModel_Tools::UseVisibilityColumn (myTreeView);
  TreeModel_Tools::SetDefaultHeaderSections (myTreeView);

  QItemSelectionModel* aSelectionModel = new QItemSelectionModel (aModel);
  myTreeView->setSelectionMode (QAbstractItemView::ExtendedSelection);
  myTreeView->setSelectionModel (aSelectionModel);
  connect (aSelectionModel, SIGNAL (selectionChanged (const QItemSelection&, const QItemSelection&)),
           this, SLOT (onTreeViewSelectionChanged (const QItemSelection&, const QItemSelection&)));

  myTreeViewActions = new MessageModel_Actions (myMainWindow, aModel, aSelectionModel);
  myTestViewActions = new MessageView_ActionsTest (myMainWindow, aModel, aSelectionModel);

  myTreeView->setContextMenuPolicy (Qt::CustomContextMenu);
  connect (myTreeView, SIGNAL (customContextMenuRequested (const QPoint&)),
          this, SLOT (onTreeViewContextMenuRequested (const QPoint&)));
  //new TreeModel_ContextMenu (myTreeView);

  connect (myTreeView->header(), SIGNAL (sectionResized (int, int, int)),
           this, SLOT(onHeaderResized (int, int, int)));

  QModelIndex aParentIndex = myTreeView->model()->index (0, 0);
  myTreeView->setExpanded (aParentIndex, true);

  myMainWindow->setCentralWidget (myTreeView);

  // property view
  myPropertyView = new ViewControl_PropertyView (myMainWindow);
  myPropertyPanelWidget = new QDockWidget (tr ("PropertyPanel"), myMainWindow);
  myPropertyPanelWidget->setObjectName (myPropertyPanelWidget->windowTitle());
  myPropertyPanelWidget->setTitleBarWidget (new QWidget(myMainWindow));
  myPropertyPanelWidget->setWidget (myPropertyView->GetControl());
  myMainWindow->addDockWidget (Qt::RightDockWidgetArea, myPropertyPanelWidget);
  connect (myPropertyPanelWidget->toggleViewAction(), SIGNAL(toggled(bool)), this, SLOT (onPropertyPanelShown (bool)));
  connect (myPropertyView, SIGNAL (propertyViewDataChanged()), this, SLOT (onPropertyViewDataChanged()));


  // view
  myViewWindow = new View_Window (myMainWindow, false);
  connect (myViewWindow, SIGNAL(eraseAllPerformed()), this, SLOT(onEraseAllPerformed()));
  aVisibilityState->SetDisplayer (myViewWindow->Displayer());
  aVisibilityState->SetPresentationType (View_PresentationType_Main);
  myViewWindow->ViewWidget()->SetPredefinedSize (MESSAGEVIEW_DEFAULT_VIEW_WIDTH, MESSAGEVIEW_DEFAULT_VIEW_HEIGHT);

  myViewDockWidget = new QDockWidget (tr ("View"), myMainWindow);
  myViewDockWidget->setObjectName (myViewDockWidget->windowTitle());
  myViewDockWidget->setWidget (myViewWindow);
  myMainWindow->addDockWidget (Qt::RightDockWidgetArea, myViewDockWidget);

  myMainWindow->resize (DEFAULT_SHAPE_VIEW_WIDTH, DEFAULT_SHAPE_VIEW_HEIGHT);
  myMainWindow->move (DEFAULT_SHAPE_VIEW_POSITION_X, DEFAULT_SHAPE_VIEW_POSITION_Y);

  updateVisibleColumns();
}

// =======================================================================
// function : SetParent
// purpose :
// =======================================================================
void MessageView_Window::SetParent (void* theParent)
{
  QWidget* aParent = (QWidget*)theParent;
  if (aParent)
  {
    QLayout* aLayout = aParent->layout();
    if (aLayout)
      aLayout->addWidget (GetMainWindow());
  }
  else
  {
    GetMainWindow()->setParent (0);
    GetMainWindow()->setVisible (true);
  }
}

// =======================================================================
// function : FillActionsMenu
// purpose :
// =======================================================================
void MessageView_Window::FillActionsMenu (void* theMenu)
{
  QMenu* aMenu = (QMenu*)theMenu;
  QList<QDockWidget*> aDockwidgets = myMainWindow->findChildren<QDockWidget*>();
  for (QList<QDockWidget*>::iterator it = aDockwidgets.begin(); it != aDockwidgets.end(); ++it)
  {
    QDockWidget* aDockWidget = *it;
    if (aDockWidget->parentWidget() == myMainWindow)
      aMenu->addAction (aDockWidget->toggleViewAction());
  }
}

// =======================================================================
// function : GetPreferences
// purpose :
// =======================================================================
void MessageView_Window::GetPreferences (TInspectorAPI_PreferencesDataMap& theItem)
{
  theItem.Clear();
  theItem.Bind ("geometry",  TreeModel_Tools::ToString (myMainWindow->saveState()).toStdString().c_str());

  QMap<QString, QString> anItems;
  TreeModel_Tools::SaveState (myTreeView, anItems);
  for (QMap<QString, QString>::const_iterator anItemsIt = anItems.begin(); anItemsIt != anItems.end(); anItemsIt++)
  {
    theItem.Bind (anItemsIt.key().toStdString().c_str(), anItemsIt.value().toStdString().c_str());
  }

  anItems.clear();
  View_Window::SaveState(myViewWindow, anItems);
  for (QMap<QString, QString>::const_iterator anItemsIt = anItems.begin(); anItemsIt != anItems.end(); anItemsIt++)
  {
    theItem.Bind (anItemsIt.key().toStdString().c_str(), anItemsIt.value().toStdString().c_str());
  }
}

// =======================================================================
// function : SetPreferences
// purpose :
// =======================================================================
void MessageView_Window::SetPreferences (const TInspectorAPI_PreferencesDataMap& theItem)
{
  for (TInspectorAPI_IteratorOfPreferencesDataMap anItemIt (theItem); anItemIt.More(); anItemIt.Next())
  {
    if (anItemIt.Key().IsEqual ("geometry"))
      myMainWindow->restoreState (TreeModel_Tools::ToByteArray (anItemIt.Value().ToCString()));
    else if (TreeModel_Tools::RestoreState (myTreeView, anItemIt.Key().ToCString(), anItemIt.Value().ToCString()))
      continue;
    else if (myViewWindow && View_Window::RestoreState(myViewWindow, anItemIt.Key().ToCString(), anItemIt.Value().ToCString()))
      continue;
  }
}

// =======================================================================
// function : UpdateContent
// purpose :
// =======================================================================
void MessageView_Window::UpdateContent()
{
  bool isUpdated = false;
  TCollection_AsciiString aName = "TKMessageView";
  if (myParameters->FindParameters (aName))
  {
    NCollection_List<Handle(Standard_Transient)> aParameters = myParameters->Parameters (aName);
    // Init will remove from parameters those, that are processed only one time (TShape)
    Init (aParameters);
    myParameters->SetParameters (aName, aParameters);
    isUpdated = true;
  }
  if (myParameters->FindFileNames (aName))
  {
    for (NCollection_List<TCollection_AsciiString>::Iterator aFilesIt (myParameters->FileNames (aName));
         aFilesIt.More(); aFilesIt.Next())
      openFile (aFilesIt.Value());

    NCollection_List<TCollection_AsciiString> aNames;
    myParameters->SetFileNames (aName, aNames);
    isUpdated = true;
  }
  Handle(Message_Report) aDefaultReport = Message::DefaultReport();
  MessageModel_TreeModel* aViewModel = dynamic_cast<MessageModel_TreeModel*> (myTreeView->model());
  if (!aDefaultReport.IsNull() && !aViewModel->HasReport (aDefaultReport))
  {
    addReport (aDefaultReport);
  }
  // reload report of selected item
  //onReloadReport();

  updateTreeModel();
  updateVisibleColumns();
}

// =======================================================================
// function : Init
// purpose :
// =======================================================================
void MessageView_Window::Init (NCollection_List<Handle(Standard_Transient)>& theParameters)
{
  Handle(AIS_InteractiveContext) aContext;
  NCollection_List<Handle(Standard_Transient)> aParameters;

  Handle(Graphic3d_Camera) aViewCamera;

  for (NCollection_List<Handle(Standard_Transient)>::Iterator aParamsIt (theParameters);
       aParamsIt.More(); aParamsIt.Next())
  {
    Handle(Standard_Transient) anObject = aParamsIt.Value();
    Handle(Message_Report) aMessageReport = Handle(Message_Report)::DownCast (anObject);
    if (!aMessageReport.IsNull())
    {
      addReport (aMessageReport);
    }
    else if (!Handle(AIS_InteractiveContext)::DownCast (anObject).IsNull())
    {
      aParameters.Append (anObject);
      if (aContext.IsNull())
        aContext = Handle(AIS_InteractiveContext)::DownCast (anObject);
    }
    else if (!Handle(Graphic3d_Camera)::DownCast (anObject).IsNull())
    {
      aViewCamera = Handle(Graphic3d_Camera)::DownCast (anObject);
    }
  }
  MessageModel_TreeModel* aTreeModel = dynamic_cast<MessageModel_TreeModel*> (myTreeView->model());
  if (!aTreeModel)
    return;

  aTreeModel->EmitLayoutChanged();

  if (!aContext.IsNull())
  {
    myViewWindow->SetContext (View_ContextType_External, aContext);
    //myViewWindow->GetViewToolBar()->SetCurrentContextType (View_ContextType_External);
  }

  //if (!aViewCamera.IsNull())
  //  myViewWindow->View()->Viewer()->View()->Camera()->Copy (aViewCamera);

  theParameters = aParameters;
}

// =======================================================================
// function : openFile
// purpose :
// =======================================================================
void MessageView_Window::openFile(const TCollection_AsciiString& theFileName)
{
#ifdef DEBUG_ALERTS
  if (theFileName.IsEmpty())
    return;

  const Handle(Message_Report)& aReport = Message::DefaultReport (Standard_True);

  //Handle(Message_Report) aReport = new Message_Report();
  if (aReport->MessageWriter().IsNull())
    aReport->SetMessageWriter (new XmlDrivers_MessageReportStorage());

  aReport->MessageWriter()->SetFileName (TCollection_AsciiString (theFileName));
  aReport->MessageWriter()->ImportReport (aReport);

  addReport (aReport, theFileName);
#endif
}

// =======================================================================
// function : updateTreeModel
// purpose :
// =======================================================================
void MessageView_Window::updateTreeModel()
{
  MessageModel_TreeModel* aViewModel = dynamic_cast<MessageModel_TreeModel*> (myTreeView->model());
  if (!aViewModel)
    return;

  aViewModel->UpdateTreeModel();
}

// =======================================================================
// function : addReport
// purpose :
// =======================================================================
void MessageView_Window::addReport (const Handle(Message_Report)& theReport,
                                    const TCollection_AsciiString& theReportDescription)
{
  MessageModel_TreeModel* aModel = dynamic_cast<MessageModel_TreeModel*> (myTreeView->model());
  aModel->AddReport (theReport, theReportDescription);

  //updateVisibleColumns();
}

// =======================================================================
// function : onTreeViewVisibilityClicked
// purpose :
// =======================================================================
void MessageView_Window::onTreeViewVisibilityClicked(const QModelIndex& theIndex)
{
  MessageModel_TreeModel* aTreeModel = dynamic_cast<MessageModel_TreeModel*> (myTreeView->model());
  TreeModel_VisibilityState* aVisibilityState = aTreeModel->GetVisibilityState();
  if (!aVisibilityState->IsVisible (theIndex))
    myPropertyView->ClearActiveTablesSelection();
}

// =======================================================================
// function : onTreeViewSelectionChanged
// purpose :
// =======================================================================
void MessageView_Window::onTreeViewSelectionChanged (const QItemSelection&, const QItemSelection&)
{
  if (!myPropertyPanelWidget->toggleViewAction()->isChecked())
    return;

  updatePropertyPanelBySelection();
  updatePreviewPresentation();
}

// =======================================================================
// function : onTreeViewContextMenuRequested
// purpose :
// =======================================================================
void MessageView_Window::onTreeViewContextMenuRequested (const QPoint& thePosition)
{
  QMenu* aMenu = new QMenu (GetMainWindow());

  MessageModel_ItemRootPtr aRootItem;
  MessageModel_ItemReportPtr aReportItem;
  QModelIndexList aSelectedIndices = myTreeView->selectionModel()->selectedIndexes();

  for (QModelIndexList::const_iterator aSelIt = aSelectedIndices.begin(); aSelIt != aSelectedIndices.end(); aSelIt++)
  {
    QModelIndex anIndex = *aSelIt;
    if (anIndex.column() != 0)
      continue;

    TreeModel_ItemBasePtr anItemBase = TreeModel_ModelBase::GetItemByIndex (anIndex);
    if (!anItemBase)
      continue;

    aRootItem = itemDynamicCast<MessageModel_ItemRoot> (anItemBase);
    if (aRootItem)
      break;
    aReportItem = itemDynamicCast<MessageModel_ItemReport> (anItemBase);
    if (aReportItem)
      break;
  }
  if (aRootItem)
  {
    aMenu->addAction (ViewControl_Tools::CreateAction (tr ("Create Default Report"),
                      SLOT (onCreateDefaultReport()), myMainWindow, this));
    // unite
    //MessageModel_TreeModel* aTreeModel = dynamic_cast<MessageModel_TreeModel*> (myTreeView->model());
    //aMenu->addAction (ViewControl_Tools::CreateAction (aTreeModel->IsUniteAlerts() ? tr ("SetUniteAlerts - OFF") : tr ("SetUniteAlerts - ON"),
    //                  SLOT (onUniteAlerts()), myMainWindow, this));
    // reversed
    //aMenu->addAction (ViewControl_Tools::CreateAction (aTreeModel->IsReversed() ? tr ("SetReversed - OFF") : tr ("SetReversed - ON"),
    //                  SLOT (onSetReversedAlerts()), myMainWindow, this));
  }
  else if (aReportItem)
  {
    aMenu->addAction (ViewControl_Tools::CreateAction (tr ("Export Report"), SLOT (onExportReport()), myMainWindow, this));
    const TCollection_AsciiString& aDescription = aReportItem->GetDescription();
    if (!aDescription.IsEmpty())
    {
      OSD_Path aPath(aDescription);
      OSD_File aDescriptionFile (aPath);
      if (aDescriptionFile.IsReadable())
        aMenu->addAction (ViewControl_Tools::CreateAction (tr ("Reload"), SLOT (onReloadReport()), myMainWindow, this));
    }
    Handle(Message_Report) aReport = aReportItem->GetReport();
    /*QAction* anAction = ViewControl_Tools::CreateAction (tr ("Export by alert"), SLOT (onAutoExportActivate()), myMainWindow, this);
    anAction->setCheckable (true);
    anAction->setChecked (aReport->WriteFileOnEachAlert());
    aMenu->addAction (anAction);

    anAction = ViewControl_Tools::CreateAction (tr ("Export trace only"), SLOT (onExportTraceOnly()), myMainWindow, this);
    anAction->setCheckable (true);
    bool isTraceOnly = aReport->MessageWriter().IsNull() ? false : aReport->MessageWriter()->Gravity() == Message_Trace;
    anAction->setChecked (isTraceOnly);
    aMenu->addAction (anAction);*/
  }
  aMenu->addSeparator();

  aMenu->addAction (ViewControl_Tools::CreateAction (tr ("Preview children presentations"), SLOT (onPreviewChildren()), myMainWindow, this));
  aMenu->addSeparator();

  myTreeViewActions->AddMenuActions (aSelectedIndices, aMenu);
  addActivateMetricActions (aMenu);

#ifdef DEBUG_ALERTS
  aMenu->addSeparator();
  myTestViewActions->AddMenuActions (aSelectedIndices, aMenu);
#endif

  QPoint aPoint = myTreeView->mapToGlobal (thePosition);
  aMenu->exec (aPoint);
}

// =======================================================================
// function : onPropertyPanelShown
// purpose :
// =======================================================================
void MessageView_Window::onPropertyPanelShown (bool isToggled)
{
  if (!isToggled)
    return;

  updatePropertyPanelBySelection();
}

// =======================================================================
// function : onPropertyViewDataChanged
// purpose :
// =======================================================================
void MessageView_Window::onPropertyViewDataChanged()
{
  QItemSelectionModel* aModel = myTreeView->selectionModel();
  if (!aModel)
    return;
  QModelIndex anIndex = TreeModel_ModelBase::SingleSelected (aModel->selectedIndexes(), 0);
  TreeModel_ItemBasePtr anItemBase = TreeModel_ModelBase::GetItemByIndex (anIndex);
  if (!anItemBase)
    return;

  updatePropertyPanelBySelection();
  updatePreviewPresentation();
}

// =======================================================================
// function : onHeaderResized
// purpose :
// =======================================================================
void MessageView_Window::onHeaderResized (int theSectionId, int, int)
{
  TreeModel_ModelBase* aViewModel = dynamic_cast<TreeModel_ModelBase*> (myTreeView->model());

  TreeModel_HeaderSection* aSection = aViewModel->ChangeHeaderItem (theSectionId);
  aSection->SetWidth (myTreeView->columnWidth (theSectionId));
}

// =======================================================================
// function : onEraseAllPerformed
// purpose :
// =======================================================================
void MessageView_Window::onEraseAllPerformed()
{
  MessageModel_TreeModel* aTreeModel = dynamic_cast<MessageModel_TreeModel*> (myTreeView->model());

  // TODO: provide update for only visibility state for better performance  TopoDS_Shape myCustomShape;

  aTreeModel->Reset();
  aTreeModel->EmitLayoutChanged();
}

// =======================================================================
// function : onExportReport
// purpose :
// =======================================================================
void MessageView_Window::onExportReport()
{
  QItemSelectionModel* aModel = myTreeView->selectionModel();
  if (!aModel)
    return;
  QModelIndex anIndex = TreeModel_ModelBase::SingleSelected (aModel->selectedIndexes(), 0);
  TreeModel_ItemBasePtr anItemBase = TreeModel_ModelBase::GetItemByIndex (anIndex);
  if (!anItemBase)
    return;
  MessageModel_ItemReportPtr aReportItem = itemDynamicCast<MessageModel_ItemReport>(anItemBase);
  if (!aReportItem)
    return;

  QString aFilter (tr ("Document file (*.json *)"));
  QString aSelectedFilter;
  QString aFileName = QFileDialog::getSaveFileName (0, tr ("Export report to file"), QString(), aFilter, &aSelectedFilter);

  Handle(Message_Report) aReport = aReportItem->GetReport();
  Standard_SStream aStream;
  aReport->DumpJson(aStream);

  QFile aLogFile(aFileName);
  if (!aLogFile.open(QFile::WriteOnly | QFile::Text))
  {
    return;
  }
  QTextStream anOut( &aLogFile );
  anOut << Standard_Dump::FormatJson (aStream).ToCString();//aStream.str().c_str();
  aLogFile.close();
}

// =======================================================================
// function : onCreateDefaultReport
// purpose :
// =======================================================================
void MessageView_Window::onCreateDefaultReport()
{
  if (!Message::DefaultReport().IsNull())
  {
    return;
  }

  addReport (Message::DefaultReport (Standard_True));
}

// =======================================================================
// function : onImportReport
// purpose :
// =======================================================================
//void MessageView_Window::onUniteAlerts()
//{
  //MessageModel_TreeModel* aTreeModel = dynamic_cast<MessageModel_TreeModel*> (myTreeView->model());
  //Standard_Boolean isUniteAlerts = aTreeModel->IsUniteAlerts();

  //aTreeModel->SetUniteAlerts (!isUniteAlerts);
//}

// =======================================================================
// function : onSetReversedAlerts
// purpose :
// =======================================================================
void MessageView_Window::onSetReversedAlerts()
{
  MessageModel_TreeModel* aTreeModel = dynamic_cast<MessageModel_TreeModel*> (myTreeView->model());
  Standard_Boolean isReversed = aTreeModel->IsReversed();

  aTreeModel->SetReversed (!isReversed);
}

// =======================================================================
// function : onReloadReport
// purpose :
// =======================================================================
void MessageView_Window::onReloadReport()
{
  QItemSelectionModel* aModel = myTreeView->selectionModel();
  if (!aModel)
    return;
  QModelIndex anIndex = TreeModel_ModelBase::SingleSelected (aModel->selectedIndexes(), 0);
  TreeModel_ItemBasePtr anItemBase = TreeModel_ModelBase::GetItemByIndex (anIndex);
  if (!anItemBase)
    return;
  MessageModel_ItemReportPtr aReportItem = itemDynamicCast<MessageModel_ItemReport>(anItemBase);
  if (!aReportItem)
    aReportItem = MessageModel_ItemReport::FindReportItem (anItemBase);

  if (!aReportItem)
    return;

  const TCollection_AsciiString aDescription = aReportItem->GetDescription();
  if (aDescription.IsEmpty())
    return;

  Handle(Message_Report) aReport = aReportItem->GetReport();
  /*aReport->Clear();
  if (aReport->MessageWriter().IsNull())
    aReport->SetMessageWriter (new XmlDrivers_MessageReportStorage());

  aReport->MessageWriter()->SetFileName (TCollection_AsciiString (aDescription));
  if (!aReport->MessageWriter()->ImportReport (aReport))
    return;*/

  MessageModel_TreeModel* aTreeModel = dynamic_cast<MessageModel_TreeModel*> (myTreeView->model());
  aModel->clearSelection();
  aTreeModel->SetReport (aReportItem->Row(), aReport, aDescription);
}

// =======================================================================
// function : onAutoExportActivate
// purpose :
// =======================================================================
void MessageView_Window::onAutoExportActivate()
{
  /*QItemSelectionModel* aModel = myTreeView->selectionModel();
  if (!aModel)
    return;
  QModelIndex anIndex = TreeModel_ModelBase::SingleSelected (aModel->selectedIndexes(), 0);
  TreeModel_ItemBasePtr anItemBase = TreeModel_ModelBase::GetItemByIndex (anIndex);
  if (!anItemBase)
    return;
  MessageModel_ItemReportPtr aReportItem = itemDynamicCast<MessageModel_ItemReport>(anItemBase);
  if (!aReportItem)
    aReportItem = MessageModel_ItemReport::FindReportItem (anItemBase);

  if (!aReportItem)
    return;

  Handle(Message_Report) aReport = aReportItem->GetReport();
  QAction* anAction = (QAction*)(sender());
  aReport->SetWriteFileOnEachAlert (anAction->isChecked());*/
}

// =======================================================================
// function : onExportTraceOnly
// purpose :
// =======================================================================
void MessageView_Window::onExportTraceOnly()
{
  /*QItemSelectionModel* aModel = myTreeView->selectionModel();
  if (!aModel)
    return;
  QModelIndex anIndex = TreeModel_ModelBase::SingleSelected (aModel->selectedIndexes(), 0);
  TreeModel_ItemBasePtr anItemBase = TreeModel_ModelBase::GetItemByIndex (anIndex);
  if (!anItemBase)
    return;
  MessageModel_ItemReportPtr aReportItem = itemDynamicCast<MessageModel_ItemReport>(anItemBase);
  if (!aReportItem)
    aReportItem = MessageModel_ItemReport::FindReportItem (anItemBase);

  if (!aReportItem)
    return;

  Handle(Message_Report) aReport = aReportItem->GetReport();
  if (aReport->MessageWriter().IsNull())
    return;

  QAction* anAction = (QAction*)(sender());
  aReport->MessageWriter()->SetGravity (anAction->isChecked() ? Message_Trace : Message_Info);*/
}

// =======================================================================
// function : onPreviewChildren
// purpose :
// =======================================================================
void MessageView_Window::onPreviewChildren()
{
  QItemSelectionModel* aModel = myTreeView->selectionModel();
  if (!aModel)
    return;

  QModelIndexList aSelectedIndices = myTreeView->selectionModel()->selectedIndexes();
  NCollection_List<Handle(Standard_Transient)> aPresentations;
  TreeModel_ModelBase::SubItemsPresentations (aSelectedIndices, aPresentations);

  displayer()->UpdatePreview (View_DisplayActionType_DisplayId, aPresentations);
}

// =======================================================================
// function : addActivateMetricActions
// purpose :
// =======================================================================
void MessageView_Window::addActivateMetricActions (QMenu* theMenu)
{
  Handle(Message_Report) aReport = Message::DefaultReport();
  if (aReport.IsNull())
  {
    return;
  }

  QMenu* aSubMenu = new QMenu ("Activate metric");
  for (int aMetricId = (int)Message_MetricType_None + 1; aMetricId <= (int)Message_MetricType_MemHeapUsage; aMetricId++)
  {
    Message_MetricType aMetricType = (Message_MetricType)aMetricId;
    QAction* anAction = ViewControl_Tools::CreateAction (Message::MetricToString (aMetricType),
      SLOT (OnActivateMetric()), parent(), this);
    anAction->setCheckable (true);
    anAction->setChecked (aReport->ActiveMetrics().Contains (aMetricType));
    aSubMenu->addAction (anAction);
  }
  aSubMenu->addSeparator();
  aSubMenu->addAction (ViewControl_Tools::CreateAction ("Deactivate all", SLOT (OnDeactivateAllMetrics()), parent(), this));

  theMenu->addMenu (aSubMenu);
}

// =======================================================================
// function : OnActivateMetric
// purpose :
// =======================================================================
void MessageView_Window::OnActivateMetric()
{
  QAction* anAction = (QAction*)(sender());

  Message_MetricType aMetricType;
  if (!Message::MetricFromString (anAction->text().toStdString().c_str(), aMetricType))
    return;

  Handle(Message_Report) aReport = Message::DefaultReport();
  const NCollection_IndexedMap<Message_MetricType>& anActiveMetrics = aReport->ActiveMetrics();
  aReport->SetActiveMetric (aMetricType, !anActiveMetrics.Contains (aMetricType));

  updateVisibleColumns();
}

// =======================================================================
// function : OnDeactivateAllMetrics
// purpose :
// =======================================================================
void MessageView_Window::OnDeactivateAllMetrics()
{
  Handle(Message_Report) aReport = Message::DefaultReport();
  if (aReport.IsNull())
    return;
  aReport->ClearMetrics();

  updateVisibleColumns();
}

// =======================================================================
// function : displayer
// purpose :
// =======================================================================
View_Displayer* MessageView_Window::displayer()
{
  return myViewWindow->Displayer();
}

// =======================================================================
// function : updatePropertyPanelBySelection
// purpose :
// =======================================================================
void MessageView_Window::updatePropertyPanelBySelection()
{
  /*QItemSelectionModel* aModel = myTreeView->selectionModel();
  if (!aModel)
    return;

  QModelIndex anIndex = TreeModel_ModelBase::SingleSelected (aModel->selectedIndexes(), 0);
  TreeModel_ItemBasePtr anItemBase = TreeModel_ModelBase::GetItemByIndex (anIndex);
  if (!anItemBase)
    return;

  QList<ViewControl_TableModelValues*> aTableValues;
  MessageModel_Tools::GetPropertyTableValues (anItemBase, aTableValues);

  myPropertyView->Init (aTableValues);*/
  ViewControl_TableModelValues* aTableValues = 0;

  QItemSelectionModel* aModel = myTreeView->selectionModel();
  if (!aModel)
    return;

  QModelIndex anIndex = TreeModel_ModelBase::SingleSelected (aModel->selectedIndexes(), 0);
  TreeModel_ItemBasePtr anItemBase = TreeModel_ModelBase::GetItemByIndex (anIndex);
  if (anItemBase)
  {
    Handle(TreeModel_ItemProperties) anItemProperties = anItemBase->Properties ();
    if (!anItemProperties.IsNull())
    {
      aTableValues = new ViewControl_TableModelValues();
      aTableValues->SetProperties (anItemProperties);
    }
  }
  myPropertyView->Init (aTableValues);
}

// =======================================================================
// function : updatePreviewPresentation
// purpose :
// =======================================================================
void MessageView_Window::updatePreviewPresentation()
{
  Handle(AIS_InteractiveContext) aContext = myViewWindow->ViewToolBar()->CurrentContext();
  if (aContext.IsNull())
    return;

  NCollection_List<Handle(Standard_Transient)> aPresentations;
  QModelIndexList aSelectedIndices = myTreeView->selectionModel()->selectedIndexes();
  for (QModelIndexList::const_iterator aSelIt = aSelectedIndices.begin(); aSelIt != aSelectedIndices.end(); aSelIt++)
  {
    QModelIndex anIndex = *aSelIt;
    if (anIndex.column() != 0)
      continue;

    TreeModel_ItemBasePtr anItemBase = TreeModel_ModelBase::GetItemByIndex (anIndex);
    if (!anItemBase)
      continue;

    anItemBase->Presentations (aPresentations);
  }

  displayer()->UpdatePreview (View_DisplayActionType_DisplayId, aPresentations);
}

// =======================================================================
// function : updateVisibleColumns
// purpose :
// =======================================================================
void MessageView_Window::updateVisibleColumns()
{
  MessageModel_TreeModel* aViewModel = dynamic_cast<MessageModel_TreeModel*> (myTreeView->model());

  NCollection_IndexedMap<Message_MetricType> anActiveMetrics;
  for (NCollection_List<MessageModel_ReportInformation>::Iterator anIterator (aViewModel->Reports()); anIterator.More(); anIterator.Next())
  {
    Handle(Message_Report) aReport = anIterator.Value().myReport;
    for (NCollection_IndexedMap<Message_MetricType>::Iterator aMetricsIterator (aReport->ActiveMetrics()); aMetricsIterator.More(); aMetricsIterator.Next())
    {
      if (anActiveMetrics.Contains (aMetricsIterator.Value()))
        continue;
      anActiveMetrics.Add (aMetricsIterator.Value());
    }
  }

  for (int aMetricId = (int)Message_MetricType_None + 1; aMetricId <= (int)Message_MetricType_MemHeapUsage; aMetricId++)
  {
    Message_MetricType aMetricType = (Message_MetricType)aMetricId;
    QList<int> aMetricColumns;
    aViewModel->GetMetricColumns (aMetricType, aMetricColumns);
    bool isColumnHidden = !anActiveMetrics.Contains (aMetricType);
    for (int i = 0; i < aMetricColumns.size(); i++)
    {
      int aColumnId = aMetricColumns[i];
      myTreeView->setColumnHidden (aColumnId, isColumnHidden);
      TreeModel_HeaderSection* aSection = aViewModel->ChangeHeaderItem (aColumnId);
      aSection->SetIsHidden (isColumnHidden);
    }
  }
}
