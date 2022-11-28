Data Exchange Wrapper (DE_Wrapper)  {#occt_user_guides__de_wrapper}
============================

@tableofcontents

@section occt_de_wrapper_1 Introduction

This manual explains how to use the **Data Exchange Wrapper** (DE Wrapper).
It provides basic documentation on setting up, using and creating files via DE_Wrapper.

The Data Exchange Wrapper (DE Wrapper) module allows reading and writing supported CAD formats to shape object or special XDE document, setting up the transfer process for all CAD files.

It is also possible to add support for new CAD formats by prototyping existing tools.

The DE Wrapper component requires @ref occt_user_guides__xde "XDE" toolkit for operation.

This manual mainly explains how to convert CAD files to an Open CASCADE Technology (OCCT) shape and vice versa. It provides basic documentation on conversion.
This manual principally deals with the next OCCT classes:
  * The Provider class, which loads CAD files and translates their contents to OCCT shapes or XDE documents, or translates OCCT shapes or XDE documents to CAD entities and then writes these entities to CAD file.
  * The Configuration class, which contains all information for the transfer process, such as a unit, tolerance, and all internal information for the OCC readers or writers.
  * The wrapper class, which contains all loaded configuration objects with own CAD format, reads or writes CAD files according file format derived from file extension or content and saves or loads configuration settings for loaded configuration objects.

@section occt_de_wrapper_2 Supported CAD formats

| CAD format | Extensions | RW support | Thread Safaty | Presentation | Package |
| :--------- | :--------- | :--------- | :----------- | :----------- | :------ |
| STEP | .stp, .step .stepz | RW | No | BRep, Mesh | STEPCAFControl |
| XCAF | .xbf | RW | Yes | BRep, Mesh | DEXCAFCascade |
| BREP | .brep | RW | Yes | BRep, Mesh | DEBRepCascade |
| IGES | .igs, .iges | RW | No | BRep | IGESCAFControl |
| OBJ | .obj | RW | Yes | Mesh | RWObj |
| STL | .stl | RW | Yes | Mesh | RWStl |
| PLY | .ply | W | Yes | Mesh | RWPly |
| GLTF | .glTF .glb | RW | Yes | Mesh | RWGltf |
| VRML | .wrl .vrml | RW | Yes | Mesh | Vrml |

**Note** :
  * A value from column "CAD format" is the same with confuguration CAD format name
  * A provider name for all presented CAD formats is "OCC"

@section occt_de_wrapper_3 Configuration DE Session

Any providers can have their own read/write parameters. To set up a transfer process DE configuration node is used. All needed parameters exist in this one. There are two ways to change parameter values: directly from code, external resource file/string.
The session is a global or static DE_Wrapper object that keeps registered DE configuration nodes and wraps DE commands to work with them. It has the own configuration parameters contains information of loaded nodes and specilal global parameters.

@subsection occt_de_wrapper_3_1 Getting DE session. Sample. Code

To work with DE session it is nesessary to load or create a DE_Wrapper object.

Getting global DE_Wrapping object:
~~~~{.cpp}
Handle(DE_Wrapper) aSession = DE_Wrapper::GlobalWrapper();
~~~~
Creating a local DE_Wrapper:
~~~~{.cpp}
Handle(DE_Wrapper) aSession = new DE_Wrapper();
~~~~
It is recommended to create a local one-time copy to work with the session, if no global changes are intended.
~~~~{.cpp}
Handle(DE_Wrapper) aOneTimeSession = aSession->Copy();
~~~~
@subsection occt_de_wrapper_3_2 Configuration resource

Configuration resource is an external file or string of the following format:
~~~~{.cpp}
global.priority.STEP :   OCC DTK
global.general.length.unit :     1
provider.STEP.OCC.read.precision.val :   0.0001
~~~~
@subsubsection occt_de_wrapper_3_2_1 Configuration resource graph of scopes
   * **global.** is a scope of global parameters
     *  **priority.** is a scope of priority to use vendors with their providers.
     *  **general.** is a scope of global configuration parameter values
     *  **"..."** is an internal configuration with any internal scopes
     *  **" : "** is a separator of key-value
     *  **"..."** parameter value, can't contain new line symbols.
   * **provider.** is a scope of configuration providers
     * **STEP.** is a scope of CAD format to configure
     * **OCC.** is a scope of a vendor or provider
     *  **"..."** is an internal configuration with any internal scopes
     *  **" : "** is a separator of key-value
     *  **"..."** parameter value, can't contain new line symbols.

@subsubsection occt_de_wrapper_3_2_2 Load configuration resource. Configuration DE Session

Resource should be loaded after registration of all providers that should be configured. The resource impacts on only registered parameters. To configure new registered provider it is necessary to load resource again. The parameters that are not representated in resource will not change and will have the previous value.

There are two ways to load a resource: recursive and only global parameters. Recursive is a way to configure all registered providers. Not recursive is a way to configure only global parameters, for example, to update the priority of vendors.

@subsubsection occt_de_wrapper_3_2_3 Load configuration resource. Sample. Code.

Configure using a resource string:
~~~~{.cpp}
  Handle(DE_Wrapper) aSession = DE_Wrapper::GlobalWrapper();
  TCollection_AsciiString aString =
    "global.priority.STEP :   OCC DTK\n"
    "global.general.length.unit : 1\n"
    "provider.STEP.OCC.read.precision.val : 0.\n";
  Standard_Boolean aIsRecursive = Standard_True;
  if (!aSession->Load(aString, aIsRecursive))
  {
    Message::SendFail() << "Error: configuration is incorrect";
  }
~~~~
Configure using a resource file:
~~~~{.cpp}
  Handle(DE_Wrapper) aSession = DE_Wrapper::GlobalWrapper();
  TCollection_AsciiString aPathToFile = "";
  Standard_Boolean aIsRecursive = Standard_True;
  if (!aSession->Load(aPathToFile, aIsRecursive))
  {
    Message::SendFail() << "Error: configuration is incorrect";
  }
~~~~
@subsubsection occt_de_wrapper_3_2_4 Load configuration resource. Sample. DRAW.

Configure using a resource string:
~~~~{.cpp}
set conf "
global.priority.STEP :   OCC
global.general.length.unit :     1
provider.STEP.OCC.read.iges.bspline.continuity :         1
provider.STEP.OCC.read.precision.mode :  0
provider.STEP.OCC.read.precision.val :   0.0001
"
LoadConfiguration ${conf} -recursive on
~~~~

Configure using a resource file:
~~~~{.cpp}
set pathToFile ""
LoadConfiguration ${pathToFile} -recursive on
~~~~

@subsubsection occt_de_wrapper_3_2_5 Saving configuration resource. Dump of configuration DE Session

The saving configuration of DE Session is a dump of all parameters of registered providers.
If the parameter didn't change in the DE Session, its value keeps as default.

There are two ways to save a resource: recursive and only global parameters. Recursive is a way to dump all registered providers information. Not recursive is a way to dump only global parameters, for example, save priority of vendors or length unit.

There are options to filter vendors or providers to save. It is necessary to provide correct name of vendor or provider to save only their information.

@subsubsection occt_de_wrapper_3_2_6 Save configuration resource. Sample. Code.

Dump to resource string. If a vendors list is empty, saves all vendors. If a providers list is empty, saves all providers of valid vendors:
~~~~{.cpp}
  Handle(DE_Wrapper) aSession = DE_Wrapper::GlobalWrapper();
  TColStd_ListOfAsciiString aFormats;
  TColStd_ListOfAsciiString aVendors;
  aFormats.Appends("STEP");
  aVendors.Appends("OCC");
  Standard_Boolean aIsRecursive = Standard_True;
  TCollection_AsciiString aConf = aSession->aConf->Save(aIsRecursive, aFormats, aVendors);
~~~~
Configure using a resource file. If a vendors list is empty, saves all vendors. If a providers list is empty, saves all providers of valid vendors:
~~~~{.cpp}
  Handle(DE_Wrapper) aSession = DE_Wrapper::GlobalWrapper();
  TCollection_AsciiString aPathToFile = "";
  TColStd_ListOfAsciiString aFormats;
  TColStd_ListOfAsciiString aVendors;
  aFormats.Appends("STEP");
  aVendors.Appends("OCC");
  Standard_Boolean aIsRecursive = Standard_True;
  if (!aSession->Save(aPathToFile, aIsRecursive, aFormats,aVendors))
  {
    Message::SendFail() << "Error: configuration is not saved";
  }
~~~~
@subsubsection occt_de_wrapper_3_2_7 Save configuration resource. Sample. DRAW.

Dump configuration to string. If no vendors lists are set as an argument or it is empty, saves all vendors. If no providers list as argument or it is empty, saves all providers of valid vendors:
~~~~{.cpp}
set vendors "OCC"
set format "STEP"
set dump_conf [DumpConfiguration -recursive on -format ${format} -vendor ${vendors}]
~~~~

Dump configuration to file. If no vendors list are set as an argument or it is empty, saves all vendors. If no providers list as argument or it is empty, saves all providers of valid vendors:
~~~~{.cpp}
set vendors "OCC"
set format "STEP"
set pathToFile ""
DumpConfiguration -path ${pathToFile} -recursive on -format ${format} -vendor ${vendors}
~~~~

@subsection occt_de_wrapper_3_3 Register providers

To transfer CAD file using DE Wrapper it is necessary to register CAD provider.
Provider contains internal and global parameters that has default value in the creating stage.
All registered providers are set to the map with information about its vendor and keep as smart handle. That is why it is possible to change value via handle in the external code.

@subsubsection occt_de_wrapper_3_3_1 Register providers. Sample. Code.

It is nesessary to register only ConfigurationNode for all needed formats.
~~~~{.cpp}
Handle(DE_Wrapper) aSession = DE_Wrapper::GlobalWrapper();
Handle(DE_ConfigurationNode) aNode = new STEPCAFControl_ConfigurationNode();
aSession->Bind(aNode);
~~~~
@subsubsection occt_de_wrapper_3_3_2 Register providers. Sample. DRAW.

Use a DRAW with all providers registered by following command:
~~~~{.cpp}
pload XDE
~~~~

@subsubsection occt_de_wrapper_3_3_3 Realtime initialization. Sample. Code.

It is possible to change a paramater value by code using smart pointer. To do this it is necessary to keep pointer in user variable.

~~~~{.cpp}
// global variable
static Handle(STEPCAFControl_ConfigurationNode) THE_STEP_NODE;

static Handle(DE_ConfigurationNode) RegisterStepNode()
{
  Handle(DE_Wrapper) aSession = DE_Wrapper::GlobalWrapper();
  if (!THE_STEP_NODE.IsNull())
  {
    return THE_STEP_NODE;
  }

  THE_STEP_NODE = new STEPCAFControl_ConfigurationNode();
  aSession->Bind(THE_STEP_NODE);
  return THE_STEP_NODE;
}

// Change parameter value
THE_STEP_NODE->InternalParameters.ReadRelationship = false;
THE_STEP_NODE->InternalParameters.ReadName = false;
THE_STEP_NODE->InternalParameters.ReadProps = false;
~~~~

@subsection occt_de_wrapper_3_4 Priority of Vendors

DE session is able to work with several vendors with the same supported CAD format. To choose the preffered vendor for the each format you should use a special priority list.

If high priority vendors provider is not supported, the transfer operation is needed (write/read), then the next vendor will be chosen.

@subsubsection occt_de_wrapper_3_4_1 Priority of Vendors. Sample. Code.

~~~~{.cpp}
  Handle(DE_Wrapper) aSession = DE_Wrapper::GlobalWrapper();
  TCollection_AsciiString aFormat = "STEP";
  TColStd_ListOfAsciiString aVendors;
  aVendors.Appends("OCC"); // high priority
  aVendors.Appends("DTK");
  // Flag to disable not choosen vendors, in this case configuration is possible
  // otherwise, lower their priority and continue to check ability to transfer
  Standard_Boolean aToDisable = Standard_True;
  aSession->ChangePriority(aFormat, aVendors, aToDisable);
~~~~

@subsubsection occt_de_wrapper_3_4_2 Priority of Vendors. Sample. DRAW.

~~~~{.cpp}
set conf "
global.priority.STEP :   OCC DTK
"
LoadConfiguration ${conf} -recursive off
~~~~

@section occt_de_wrapper_4 Transfer CAD file

To transfer from CAD file to OCC or from OCC to XCAF file it is necessary to use configured DE_Wrapper object. It can be local, one-time or global. Global configuration of DE_Wrapper propagates to all nodes via transfer. There are two options to transfer: using OCC shape or XCAF document. It is possible to work only with real path to/from the file. Streaming is not supported (in process).

Format of input/output file is automatically determined by extension or content.

@subsection occt_de_wrapper_4_1 Transfer CAD file. Samples. Code.

Code sample to read Step file to shape
~~~~{.cpp}
  Handle(DE_Wrapper) aSession = DE_Wrapper::GlobalWrapper();
  TCollection_AsciiString aPathToFile = "example.stp";
  TopoDS_Shape aShRes;
  if (!aSession->Read(aPathToFile, aShRes))
  {
    Message::SendFail() << "Error: Can't read file";
  }
~~~~

Code sample to write Step file from shape
~~~~{.cpp}
  Handle(DE_Wrapper) aSession = DE_Wrapper::GlobalWrapper();
  TCollection_AsciiString aPathToFile = "example.stp";
  TopoDS_Shape aShFrom = ...;
  if (!aSession->Write(aPathToFile, aShRes))
  {
    Message::SendFail() << "Error: Can't write file";
  }
~~~~

Code sample to read Step file to XCAF document
~~~~{.cpp}
  Handle(DE_Wrapper) aSession = DE_Wrapper::GlobalWrapper();
  TCollection_AsciiString aPathToFile = "example.stp";
  Handle(TDocStd_Document) aDoc = ...;
  if (!aSession->Read(aPathToFile, aDoc))
  {
    Message::SendFail() << "Error: Can't read file";
  }
~~~~

Code sample to write Step file from XCAF document
~~~~{.cpp}
  Handle(DE_Wrapper) aSession = DE_Wrapper::GlobalWrapper();
  TCollection_AsciiString aPathToFile = "example.stp";
  Handle(TDocStd_Document) aDoc = ...;
  if (!aSession->Write(aPathToFile, aDoc))
  {
    Message::SendFail() << "Error: Can't write file";
  }
~~~~

@subsection occt_de_wrapper_4_2 Transfer CAD file. Samples. DRAW.

DRAW sample to read Step file to shape
~~~~{.cpp}
set fileName "sample.stp"
readfile shape ${fileName}
~~~~

DRAW sample to write Step file from shape
~~~~{.cpp}
set fileName "sample.stp"
writefile shape ${fileName}
~~~~

DRAW sample to read Step file to XCAF document
~~~~{.cpp}
set fileName "sample.stp"
ReadFile D ${fileName}
~~~~

DRAW sample to write Step file from XCAF document
~~~~{.cpp}
set fileName "sample.stp"
WriteFile D ${fileName}
~~~~

@subsection occt_de_wrapper_4_3 Transfer using DE Provider. Sample. Code.

It is possible to read and write CAD file directly from special provider.

~~~~{.cpp}
// Creating or getting node
Handle(STEPCAFControl_ConfigurationNode) aNode = new STEPCAFControl_ConfigurationNode();
// Creationg an one-time provider
Handle(DE_Provider) aProvider = aNode->BuildProvider();
// Setting configuration with all parameters
aProvider->SetNode(aNode);
if (!aProvider->Read(...))
{
  Message::SendFail() << "Error: Can't read STEP file";
}
if (!aProvider->Write(...))
{
  Message::SendFail() << "Error: Can't write STEP file";
}
~~~~

@subsection occt_de_wrapper_4_4 Templary configuration via transfer

It is possible to change configuration of only one transfer operation. To avoid changing parameters in the session, one-time clone of session can be created and used for transfer. This way is recommended to use in multithreaded mode.

@subsubsection occt_de_wrapper_4_4_1 Templary configuration via transfer. Sample. Code.

Code sample to configure via transfer
~~~~{.cpp}
  Handle(DE_Wrapper) aSession = DE_Wrapper::GlobalWrapper()->Copy();
  TCollection_AsciiString aString =
    "global.priority.STEP :   OCC DTK\n"
    "global.general.length.unit : 1\n"
    "provider.STEP.OCC.read.precision.val : 0.\n";
  if (!aSession->Load(aString, aIsRecursive))
  {
    Message::SendFail() << "Error: configuration is incorrect";
  }
  TCollection_AsciiString aPathToFile = "example.stp";
  TopoDS_Shape aShRes;
  if (!aSession->Read(aPathToFile, aShRes))
  {
    Message::SendFail() << "Error: Can't read file";
  }
~~~~

@subsubsection occt_de_wrapper_4_4_2 Templary configuration via transfer. Sample. DRAW.

Code sample to configure via transfer within DRAW command
~~~~{.cpp}
set fileName "sample.stp"
readfile S5 $filename -conf "global.general.length.unit : 1000 "
~~~~

Code sample to configure via transfer as variable
~~~~{.cpp}
set fileName "sample.stp"
set conf "
global.priority.STEP :   OCC
global.general.length.unit :     1
provider.STEP.OCC.read.iges.bspline.continuity :         1
provider.STEP.OCC.read.precision.mode :  0
provider.STEP.OCC.read.precision.val :   0.0001
"
readfile S5 $filename -conf ${conf}
~~~~
