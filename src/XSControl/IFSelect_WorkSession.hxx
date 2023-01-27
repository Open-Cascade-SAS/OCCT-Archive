// Created on: 1992-12-15
// Created by: Christian CAILLET
// Copyright (c) 1992-1999 Matra Datavision
// Copyright (c) 1999-2014 OPEN CASCADE SAS
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

#ifndef _IFSelect_WorkSession_HeaderFile
#define _IFSelect_WorkSession_HeaderFile

#include <Standard.hxx>
#include <Standard_Type.hxx>

#include <TCollection_AsciiString.hxx>
#include <Interface_CheckIterator.hxx>
#include <TColStd_IndexedDataMapOfTransientTransient.hxx>
#include <Standard_Transient.hxx>
#include <NCollection_Vector.hxx>
#include <NCollection_DataMap.hxx>
#include <Standard_CString.hxx>
#include <XSControl_ReturnStatus.hxx>
#include <Standard_Integer.hxx>
#include <TColStd_HSequenceOfTransient.hxx>
#include <TColStd_HSequenceOfInteger.hxx>
#include <TColStd_HSequenceOfHAsciiString.hxx>

class IFSelect_WorkLibrary;
class Interface_Protocol;
class Interface_InterfaceModel;
class Interface_HGraph;
class Interface_GTool;

class TCollection_HAsciiString;
class Interface_Graph;
class Interface_EntityIterator;

class IFSelect_WorkSession;
DEFINE_STANDARD_HANDLE(IFSelect_WorkSession, Standard_Transient)

//! This class can be used to simply manage a process such as
//! splitting a file, extracting a set of Entities ...
//! It allows to manage different types of Variables : Integer or
//! Text Parameters, Selections, Dispatches, in addition to a
//! ShareOut. To each of these variables, a unique Integer
//! Identifier is attached. A Name can be attached too as desired.
class IFSelect_WorkSession : public Standard_Transient
{

public:

  //! Creates a Work Session
  //! It provides default, empty ShareOut and ModelCopier, which can
  //! be replaced (if required, should be done just after creation).
  Standard_EXPORT IFSelect_WorkSession();
  
  //! Changes the Error Handler status (by default, it is not set)
  Standard_EXPORT void SetErrorHandle (const Standard_Boolean toHandle);
  
  //! Returns the Error Handler status
  Standard_Boolean ErrorHandle() const
  { return theerrhand; }

  //! Set value of mode responsible for presence of selections after loading
  //! If mode set to true that different selections will be accessible after loading
  //! else selections will be not accessible after loading( for economy memory in applications)
  void SetModeStat (const Standard_Boolean theMode)
  { themodelstat = theMode; }

  //! Return value of mode defining of filling selection during loading
  Standard_Boolean GetModeStat() const
  { return themodelstat; }
  
  //! Sets a WorkLibrary, which will be used to Read and Write Files
  void SetLibrary (const Handle(IFSelect_WorkLibrary) &theLib)
  { thelibrary = theLib; }
  
  //! Returns the WorkLibrary. Null Handle if not yet set
  //! should be C++ : return const &
  const Handle(IFSelect_WorkLibrary) & WorkLibrary() const
  { return thelibrary; }
  
  //! Sets a Protocol, which will be used to determine Graphs, to
  //! Read and to Write Files
  Standard_EXPORT void SetProtocol (const Handle(Interface_Protocol)& protocol);
  
  //! Returns the Protocol. Null Handle if not yet set
  //! should be C++ : return const &
  const Handle(Interface_Protocol) & Protocol() const
  { return theprotocol; }
  
  //! Returns True is a Model has been set
  Standard_Boolean HasModel() const
  { return (!myModel.IsNull()); }
  
  //! Sets a Model as input : this will be the Model from which the
  //! ShareOut will work
  //! if <clearpointed> is True (default) all SelectPointed items
  //! are cleared, else they must be managed by the caller
  //! Remark : SetModel clears the Graph, recomputes it if a
  //! Protocol is set and if the Model is not empty, of course
  Standard_EXPORT void SetModel (const Handle(Interface_InterfaceModel)& model, const Standard_Boolean clearpointed = Standard_True);
  
  //! Returns the Model of the Work Session (Null Handle if none)
  //! should be C++ : return const &
  const Handle(Interface_InterfaceModel) & Model () const
  { return myModel; }
  
  //! Stores the filename used for read for setting the model
  //! It is cleared by SetModel and ClearData(1)
  void SetLoadedFile (const Standard_CString theFileName)
  { theloaded = theFileName; }
  
  //! Returns the filename used to load current model
  //! empty if unknown
  Standard_CString LoadedFile() const
  { return theloaded.ToCString(); }
  
  //! Reads a file with the WorkLibrary (sets Model and LoadedFile)
  //! Returns a integer status which can be :
  //! RetDone if OK,  RetVoid if no Protocol not defined,
  //! RetError for file not found, RetFail if fail during read
  Standard_EXPORT XSControl_ReturnStatus ReadFile (const Standard_CString filename);

  //! Reads a file from stream with the WorkLibrary (sets Model and LoadedFile)
  //! Returns a integer status which can be :
  //! RetDone if OK,  RetVoid if no Protocol not defined,
  //! RetError for file not found, RetFail if fail during read
  Standard_EXPORT XSControl_ReturnStatus ReadStream (const Standard_CString theName, std::istream& theIStream);
  
  //! Returns the count of Entities stored in the Model, or 0
  Standard_EXPORT Standard_Integer NbStartingEntities() const;
  
  //! Returns an  Entity stored in the Model of the WorkSession
  //! (Null Handle is no Model or num out of range)
  Standard_EXPORT Handle(Standard_Transient) StartingEntity (const Standard_Integer num) const;
  
  //! Returns the Number of an Entity in the Model
  //! (0 if no Model set or <ent> not in the Model)
  Standard_EXPORT Standard_Integer StartingNumber (const Handle(Standard_Transient)& ent) const;
  
  //! From a given label in Model, returns the corresponding number
  //! Starts from first entity by Default, may start after a given
  //! number : this number may be given negative, its absolute value
  //! is then considered. Hence a loop on NumberFromLabel may be
  //! programmed (stop test is : returned value positive or null)
  //!
  //! Returns 0 if not found, < 0 if more than one found (first
  //! found in negative).
  //! If <val> just gives an integer value, returns it
  Standard_EXPORT Standard_Integer NumberFromLabel (const Standard_CString val, const Standard_Integer afternum = 0) const;
  
  //! Returns the label for <ent>, as the Model does
  //! If <ent> is not in the Model or if no Model is loaded, a Null
  //! Handle is returned
  Standard_EXPORT Handle(TCollection_HAsciiString) EntityLabel (const Handle(Standard_Transient)& ent) const;
  
  //! Returns the Name of an Entity
  //! This Name is computed by the general service Name
  //! Returns a Null Handle if fails
  Standard_EXPORT Handle(TCollection_HAsciiString) EntityName (const Handle(Standard_Transient)& ent) const;
  
  //! Clears recorded data (not the items) according mode :
  //! 1 : all Data : Model, Graph, CheckList, + ClearData 4
  //! 2 : Graph and CheckList (they will then be recomputed later)
  //! 3 : CheckList (it will be recomputed by ComputeCheck)
  //! 4 : just content of SelectPointed and Counters
  //! Plus 0 : does nothing but called by SetModel
  //! ClearData is virtual, hence it can be redefined to clear
  //! other data of a specialised Work Session
  Standard_EXPORT virtual void ClearData (const Standard_Integer mode);
  
  //! Computes the Graph used for Selections, Displays ...
  //! If a HGraph is already set, with same model as given by method
  //! Model, does nothing. Else, computes a new Graph.
  //! If <enforce> is given True, computes a new Graph anyway.
  //! Remark that a call to ClearGraph will cause ComputeGraph to
  //! really compute a new Graph
  //! Returns True if Graph is OK, False else (i.e. if no Protocol
  //! is set, or if Model is absent or empty).
  Standard_EXPORT Standard_Boolean ComputeGraph (const Standard_Boolean enforce = Standard_False);
  
  //! Returns the Computed Graph as HGraph (Null Handle if not set)
  Standard_EXPORT Handle(Interface_HGraph) HGraph();
  
  //! Returns the Computed Graph, for Read only
  Standard_EXPORT const Interface_Graph& Graph();
  
  //! Returns the list of entities shared by <ent> (can be empty)
  //! Returns a null Handle if <ent> is unknown
  Standard_EXPORT Handle(TColStd_HSequenceOfTransient) Shareds (const Handle(Standard_Transient)& ent);
  
  //! Returns the list of entities sharing <ent> (can be empty)
  //! Returns a null Handle if <ent> is unknown
  Standard_EXPORT Handle(TColStd_HSequenceOfTransient) Sharings (const Handle(Standard_Transient)& ent);

  DEFINE_STANDARD_RTTIEXT(IFSelect_WorkSession,Standard_Transient)

 protected:

  Handle(Interface_HGraph) thegraph;
  Interface_CheckIterator thecheckrun;
  NCollection_DataMap<TCollection_AsciiString, Handle(Standard_Transient)> thenames;

 private:

  Standard_Boolean theerrhand;
  Handle(IFSelect_WorkLibrary) thelibrary;
  Handle(Interface_Protocol) theprotocol;
  Handle(Interface_InterfaceModel) myModel;
  TCollection_AsciiString theloaded;
  Handle(Interface_GTool) thegtool;
  Standard_Boolean thecheckdone;
  Interface_CheckIterator thechecklist;
  TCollection_AsciiString thecheckana;
  Handle(Interface_InterfaceModel) theoldel;
  Standard_Boolean themodelstat;
};

#endif // _IFSelect_WorkSession_HeaderFile
