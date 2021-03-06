///////////////////////////////////////////////////////////////////////////////
///
/// \file DataContent.hpp
/// Declaration of the Data content classes.
/// 
/// Authors: Chris Peters
/// Copyright 2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

/// DataContent item. Data content is a content item that is stored in
/// the engine's text data format. These files Contains many resource types
/// and are usually edited by the editor directly.
class DataContent : public ContentComposition
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  DataContent();
  DataContent(ContentInitializer& initializer);
};

/// Data builder content item. Data content builder basically just copies
/// the data file out. It also Contains the 'type' of what is in the file
/// to determine what loader to use.
class DataBuilder : public BuilderComponent
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  String Name;
  ResourceId mResourceId;
  String LoaderType;
  uint Version;
  bool mShowInEditor;

  String GetOutputFile();

  //BuilderComponent Interface
  void Generate(ContentInitializer& initializer) override;
  void Serialize(Serializer& stream) override;

  void SetShowInEditor(bool state) override;
  void Rename(StringParam newName) override;
  bool NeedsBuilding(BuildOptions& options) override;
  void BuildContent(BuildOptions& buildOptions) override;
  void BuildListing(ResourceListing& listing) override;
};

}//namespace Zero
