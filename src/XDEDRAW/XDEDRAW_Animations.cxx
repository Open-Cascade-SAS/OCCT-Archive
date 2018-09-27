// Created on: 2017-10-02
// Created by: Elena MOZOKHINA
// Copyright (c) 2016 OPEN CASCADE SAS
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

#include <DDocStd.hxx>
#include <OSD_File.hxx>
#include <OSD_Protection.hxx>
#include <TDocStd_Document.hxx>
#include <TDF_Tool.hxx>
#include <TCollection_HAsciiString.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_AnimationTool.hxx>
#include <XDEDRAW_Animations.hxx>
#include <XCAFDoc_Animation.hxx>

struct cmd
{
  const char*      name;
  Standard_Integer nargsreq;
  const char*      use;
};

//=======================================================================
//function : animations
//purpose  : returns list of all animations
//=======================================================================
static const cmd XAnimations = {
  "XAnimations", 2, "XAnimations Doc"
};
static Standard_Integer
animations(Draw_Interpretor& di, Standard_Integer argc, const char** argv)
{
  static const cmd& myCommand = XAnimations;

  if (argc < myCommand.nargsreq)
  {
    di << "Use: " << myCommand.use << "\n";
    return 1;
  }

  Handle(TDocStd_Document) aDoc;
  DDocStd::GetDocument(argv[1], aDoc);
  if (aDoc.IsNull())
  {
    return 1;
  }

  Handle(XCAFDoc_AnimationTool) anAnimTool = XCAFDoc_DocumentTool::AnimationTool(aDoc->Main());

  TDF_LabelSequence anAnimations;
  anAnimTool->GetAnimationLabels(anAnimations);
  for (TDF_LabelSequence::Iterator anIt(anAnimations); anIt.More(); anIt.Next())
  {
    TCollection_AsciiString anEntry;
    TDF_Tool::Entry(anIt.Value(), anEntry);
    di << anEntry << " ";
  }
  return 0;
}

//=======================================================================
//function : addAnimation
//purpose  : add animation
//=======================================================================
static const cmd XAnimationAdd = {
  "XAnimationAdd", 6, "XAnimationAdd Doc name --file image_path animation_path | --data image_data animation_data"
};
static Standard_Integer
addAnimation(Draw_Interpretor& di, Standard_Integer argc, const char** argv)
{
  static const cmd& myCommand = XAnimationAdd;

  if (argc < myCommand.nargsreq)
  {
    di << "Use: " << myCommand.use << "\n";
    return 1;
  }

  Standard_Integer iarg = 0;

  Handle(TDocStd_Document) aDoc;
  DDocStd::GetDocument(argv[++iarg], aDoc);
  if (aDoc.IsNull()) 
  {
    return 1;
  }

  TCollection_ExtendedString aName = argv[++iarg];
  Standard_Boolean aFromFile = Standard_False;
  Standard_Boolean aFromData = Standard_False;
  TCollection_ExtendedString anImageFileStr;
  TCollection_ExtendedString anAnimationFileStr;
  Handle(TColStd_HArray1OfByte) anImage;
  Handle(TColStd_HArray1OfByte) anAnimation;
  for (++iarg; iarg < argc; ++iarg)
  {
    TCollection_AsciiString opt = argv[iarg];
    if (opt == "--file")
    {
      if (aFromData)
      {
        di << "Error: data can be taken either from a file or a data array.\n";
        di << "Use: " << myCommand.use << "\n";
        return 1;
      }
      if (++iarg >= argc)
      {
        di << "Error: file image path is expected.\n";
        di << "Use: " << myCommand.use << "\n";
        return 1;
      }
      anImageFileStr = argv[iarg];
      if (++iarg >= argc)
      {
        di << "Error: file animation path is expected.\n";
        di << "Use: " << myCommand.use << "\n";
        return 1;
      }
      anAnimationFileStr = argv[iarg];
      aFromFile = Standard_True;
    }
    else if (opt == "--data")
    {
      if (aFromFile)
      {
        di << "Error: data can be taken either from a file or a data array.\n";
        di << "Use: " << myCommand.use << "\n";
        return 1;
      }
      if (++iarg >= argc)
      {
        di << "Error: image data array is expected.\n";
        di << "Use: " << myCommand.use << "\n";
        return 1;
      }
      Standard_SStream ss(std::ios_base::in | std::ios_base::out | std::ios_base::binary);
      ss << argv[iarg];
      Standard_Integer len = static_cast<Standard_Integer>(ss.tellp());
      anImage = new TColStd_HArray1OfByte(1, len);
      for (Standard_Integer i = 1; i <= len && !ss.eof(); ++i)
      {
        ss >> anImage->ChangeValue(i);
      }

      if (++iarg >= argc)
      {
        di << "Error: animation data array is expected.\n";
        di << "Use: " << myCommand.use << "\n"; 
        return 1;
      }
      Standard_SStream ssAn(std::ios_base::in | std::ios_base::out | std::ios_base::binary);
      ssAn << argv[iarg];
      len = static_cast<Standard_Integer>(ss.tellp());
      anAnimation = new TColStd_HArray1OfByte(1, len);
      for (Standard_Integer i = 1; i <= len && !ss.eof(); ++i)
      {
        ssAn >> anAnimation->ChangeValue(i);
      }
      aFromData = Standard_True;
    }
  }
  Handle(XCAFDoc_AnimationTool) anAnimationTool = XCAFDoc_DocumentTool::AnimationTool(aDoc->Main());
  TDF_Label aLabel;
  if (aFromFile)
  {
    OSD_Path anImagePath(anImageFileStr);
    OSD_File anImageFile(anImagePath);
    anImageFile.Open(OSD_ReadOnly, OSD_Protection());
    
    OSD_Path anAnimPath(anAnimationFileStr);
    OSD_File anAnimFile(anAnimPath);
    anAnimFile.Open(OSD_ReadOnly, OSD_Protection());
    
    aLabel = anAnimationTool->AddAnimation(new TCollection_HAsciiString(aName), anImageFile, anAnimFile);
  }
  else if (aFromData)
  {
    aLabel = anAnimationTool->AddAnimation(new TCollection_HAsciiString(aName), anImage, anAnimation);
  }
  else
  {
    di << "Error: data can be taken either from a file or a data array.\n";
    di << "Use: " << myCommand.use << "\n";
    return 1;
  }
  TCollection_AsciiString anEntry;
  TDF_Tool::Entry(aLabel, anEntry);
  di << anEntry;
  return 0;
}

//=======================================================================
//function : removeAnimation
//purpose  : deletes an animation  
//=======================================================================
static const cmd XAnimationRemove = {
  "XAnimationRemove", 3, "XAnimationRemove Doc animation"
};
static Standard_Integer
removeAnimation(Draw_Interpretor& di, Standard_Integer argc, const char** argv)
{
  static const cmd& myCommand = XAnimationRemove;

  if (argc < myCommand.nargsreq)
  {
    di << "Use: " << myCommand.use << "\n";
    return 1;
  }

  Handle(TDocStd_Document) aDoc;
  DDocStd::GetDocument(argv[1], aDoc);
  if (aDoc.IsNull())
  {
    return 1;
  }

  TCollection_ExtendedString anEntry = argv[2];

  TDF_Label aLabel;
  TDF_Tool::Label(aDoc->GetData(), anEntry, aLabel);
  if (aLabel.IsNull())
  {
    di << anEntry << ": invalid animation entry.\n";
    return 1;
  }

  Handle(XCAFDoc_AnimationTool) anAnimationTool = XCAFDoc_DocumentTool::AnimationTool(aDoc->Main());
  if (!anAnimationTool->RemoveAnimation(aLabel))
  {
    di << "Error: couldn't remove animation.\n";
    return 1;
  }
  return 0;
}

//=======================================================================
//function : animationDump
//purpose  : dump an animation
//=======================================================================
static const cmd XAnimationDump = {
  "XAnimationDump", 3, "XAnimationDump Doc animation"
};
static Standard_Integer
animationDump(Draw_Interpretor& di, Standard_Integer argc, const char** argv)
{
  static const cmd& myCommand = XAnimationDump;

  if (argc < myCommand.nargsreq)
  {
    di << "Use: " << myCommand.use << "\n";
    return 1;
  }

  Standard_Integer iarg = 0;

  Handle(TDocStd_Document) aDoc;
  DDocStd::GetDocument(argv[++iarg], aDoc);
  if (aDoc.IsNull())
  {
    return 1;
  }

  TCollection_AsciiString anEntry = argv[++iarg];

  TDF_Label aLabel;
  TDF_Tool::Label(aDoc->GetData(), anEntry, aLabel);
  if (aLabel.IsNull())
  {
    TCollection_AsciiString anEntry;
    TDF_Tool::Entry(aLabel, anEntry);
    di << anEntry << ": invalid animation entry.\n";
    return 1;
  }

  Handle(XCAFDoc_AnimationTool) anAnimTool = XCAFDoc_DocumentTool::AnimationTool(aDoc->Main());
  Handle(TCollection_HAsciiString) aName;
  Handle(TColStd_HArray1OfByte) anImage, anAnimation;
  
  anAnimTool->GetAnimation(aLabel, aName, anImage, anAnimation);
  if (!aName.IsNull())
  {
    di <<"Name  : " << aName->ToCString() << "\n";
  }
  static Standard_Integer theMaxLen = 64;
  if (!anImage.IsNull())
  {
    di << "Image size     : " << anImage->Length()<<"\n";
  }
  else
  {
    di << "Image is NULL \n ";
  }

  if (!anAnimation.IsNull())
  {
    di << "Animation  size    : " << anAnimation->Length()<<"\n";
  }
  else
  {
    di << "Animation is NULL \n ";
  }
  return 0;
}

//=======================================================================
//function : setName
//purpose  : rename animation
//=======================================================================
static const cmd XAnimationSetName = {
  "XAnimationSetName", 4, "XAnimationSetName Doc animation new_name"
};
static Standard_Integer
setName(Draw_Interpretor& di, Standard_Integer argc, const char** argv)
{
  static const cmd& myCommand = XAnimationSetName;

  if (argc < myCommand.nargsreq)
  {
    di << "Use: " << myCommand.use << "\n";
    return 1;
  }

  Handle(TDocStd_Document) aDoc;
  DDocStd::GetDocument(argv[1], aDoc);
  if (aDoc.IsNull())
  {
    return 1;
  }

  TCollection_ExtendedString anEntry = argv[2];

  TDF_Label aLabel;
  TDF_Tool::Label(aDoc->GetData(), anEntry, aLabel);
  if (aLabel.IsNull())
  {
    di << anEntry << ": invalid animation entry.\n";
    return 1;
  }
  
  Handle(XCAFDoc_AnimationTool) anAnimTool = XCAFDoc_DocumentTool::AnimationTool(aDoc->Main());
  anAnimTool->SetName(aLabel, new TCollection_HAsciiString(argv[3]));
  return 0;
}

//=======================================================================
//function : InitCommands
//purpose  : 
//=======================================================================
void XDEDRAW_Animations::InitCommands(Draw_Interpretor& di) 
{
  static Standard_Boolean initialized = Standard_False;
  if (initialized)
  {
    return;
  }
  initialized = Standard_True;
  
  Standard_CString g = "XDE Animations commands";

  di.Add(XAnimations.name, XAnimations.use, __FILE__, animations, g);
  di.Add(XAnimationAdd.name, XAnimationAdd.use, __FILE__, addAnimation, g);
  di.Add(XAnimationRemove.name, XAnimationRemove.use, __FILE__, removeAnimation, g);
  di.Add(XAnimationDump.name, XAnimationDump.use, __FILE__, animationDump, g);
  di.Add(XAnimationSetName.name, XAnimationSetName.use, __FILE__, setName, g);
}
