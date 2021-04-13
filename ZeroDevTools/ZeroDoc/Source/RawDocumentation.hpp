///////////////////////////////////////////////////////////////////////////////
///
///\file InterDocumentation.hpp
/// contains classes for unpacking data from doxygen's xml output into a nicer format
///
/// Authors: Joshua Shlemmer
/// Copyright 2015-2016, DigiPen Institute of Technology
///
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include <Engine/EngineStandard.hpp>
#include <Engine/Documentation.hpp>
#include "..\TinyXml\tinyxml.h"
#include "DocTypeTokens.hpp"

#define WriteLog(...) DocLogger::Get()->Write(__VA_ARGS__)
namespace Zero
{
  // change this to macro magic later
  enum gElementTagsEnum
  {
    eDOXYGEN,
    eBASECOMPOUNDREF,
    eCOMPOUNDDEF,
    eMEMBERDEF,
    eMEMBER,
    eBRIEFDESCRIPTION,
    ePARA,
    ePARAM,
    eKIND,
    eNAME,
    eARGSSTRING,
    eTYPE,
    eREF,
    eDECLNAME,
    eDEFINITION,
    eCODELINE,
    eHIGHLIGHT,
    eSECTIONDEF,
    eINVALID_TAG
  };

  static const char* gElementTags[eINVALID_TAG] = {
    "doxygen",
    "basecompoundref",
    "compounddef",
    "memberdef",
    "member",
    "briefdescription",
    "para",
    "param",
    "kind",
    "name",
    "argsstring",
    "type",
    "ref",
    "declname",
    "definition",
    "codeline",
    "highlight",
    "sectiondef"
  };


  ///// FORWARD DEC ///// 
  class RawTypedefDoc;
  class RawMethodDoc;
  class RawVariableDoc;
  class RawTypedefLibrary;
  class RawNamespaceDoc;
  class IgnoreList;
  class RawDocumentationLibrary;

  ///// HELPERS ///// 
  /// Gets all of the text from the passed in node and all its children, text is added to output
  void GetTextFromAllChildrenNodesRecursively(TiXmlNode* node, StringBuilder* output);

  /// Get text from passed in node and all of its children and returns it as a string
  String GetTextFromAllChildrenNodesRecursively(TiXmlNode* node);

  /// Gets argument at pos if it is a string, otherwise returns empty
  String GetArgumentIfString(TypeTokens &fnCall, uint argPos);

  /// Adds all codelines in a document to the stringbuilder passed for output
  String GetCodeFromDocumentFile(TiXmlDocument *doc);

  /// make doxyfile string from source file name
  String GetDoxyfileNameFromSourceFileName(StringParam fileName);

  /// turns token list to string placing spaces after qualifiers
  String TrimTypeTokens(const TypeTokens& tokens);

  /// returns first child of element with value containing tag type 'type'
  TiXmlNode* GetFirstNodeOfChildType(TiXmlElement* element, StringRef type);

  /// returns one past the last chiled of tag type 'type', returns null if it DNE
  TiXmlNode* GetEndNodeOfChildType(TiXmlElement* element, StringRef type);

  /// does as it says, removes all spaces from str and outputs it into the stringbuilder
  void CullAllSpacesFromString(StringRef str, StringBuilder* output);

  /// gets value text from node, does assume node it of type ref or text
  //const char* GetTextFromNode(TiXmlNode* node);

  /// gets text from children type nodes
  void GetTextFromChildrenNodes(TiXmlNode* node, StringBuilder* output);

  /// gets text from paragraph nodes
  void getTextFromParaNodes(TiXmlNode* node, StringBuilder* output);

  /// replaces token at location with the typdef
  uint ReplaceTypedefAtLocation(TypeTokens& tokenArray
    , DocToken* location, RawTypedefDoc& tDef);

  /// takes the element that expresses a type, and parse its children to build type
  void BuildFullTypeString(TiXmlElement* typeNode, StringBuilder* output);

  /// replaces type tokens with corresponding typedefs (warning, all typedefs global scoped)
  bool NormalizeTokensFromTypedefs(TypeTokens& tokens, RawTypedefLibrary* defLib,
    RawNamespaceDoc& classNamespace);

  /// recursivly get filepaths in a directory that contain passed in partial string in the filename
  void GetFilesWithPartialName(StringParam basePath, StringParam partialName, Array<String>* output);

  /// get files in directory that have partial string in them, but ignores anything in ignoreList
  void GetFilesWithPartialName(StringParam basePath, StringParam partialName,
    IgnoreList &ignoreList, Array<String>* output);

  /// recusivly search directory for file with exact name passed in
  String GetFileWithExactName(StringParam basePath, StringParam exactName);

  /// gets rid of any duplicate spaces that doxygen left in descriptions
  String CleanRedundantSpacesInDesc(StringParam description);

  /// takes in a trim doc and outputs anything missing description
  void OutputListOfObjectsWithoutDesc(const DocumentationLibrary &trimDoc);

  bool ContainsFirstTypeInSecondType(TypeTokens &firstType, TypeTokens &secondType);

  /// compares two document classes by alphebetical comparison of names
  template<typename T>
  bool DocCompareFn(const T& lhs, const T& rhs);

  /// compares two document classes by alphebetical comparison of names (for pointer types)
  template<typename T>
  bool DocComparePtrFn(const T& lhs, const T& rhs);

  ///// CLASSES /////

  class IgnoreList : public Object
  {
  public:
    ZeroDeclareType(IgnoreList);
    /// returns true if directory/file passed in is on the ignore list
    bool DirectoryIsOnIgnoreList(StringParam dir);

    bool NameIsOnIgnoreList(StringParam name);

    bool empty(void);

    void SortList(void);

    void CreateIgnoreListFromDocLib(StringParam doxyPath, DocumentationLibrary &doc);

    void Serialize(Serializer& stream);

    /// list of ignored directories relative to documentation location (can be single files too)
    Array<String> mDirectories;

    Array<String> mIgnoredNames;

    String mDoxyPath;
  };

  class DocLangDfa
  {
  public:
    static DocDfaState* Get(void);
  };

  class DocLogger
  {
  public:
    DocLogger():mStarted(false) {}

    ~DocLogger();

    /// opens up the log file in write mode at path
    void StartLogger(StringParam path);

    /// writes message to stdout as well as the log file
    void Write(const char*fmt...);

    /// gets a pointer to the doclogger
    static DocLogger* Get(void);

  private:
    File mLog;

    String mPath;

    bool mStarted;
  };

  class RawNamespaceDoc
  {
  public:
    RawNamespaceDoc(): mNames() {}

    void Serialize(Serializer& stream);

    /// gets all the namespace strings from a list of tokens
    void GetNamesFromTokens(TypeTokens& tokens);

    Array<String> mNames;
  };

  class RawMethodDoc
  {
  public:
    struct Parameter
    {
      Parameter(void);
      ~Parameter(void);
      void Serialize(Serializer& stream);

      TypeTokens* mTokens;

      String mName;
      String mDescription;
    };

    RawMethodDoc(void);
    ~RawMethodDoc(void);

    /// constructs class by loading the method info from both the def and the element
    RawMethodDoc(TiXmlElement* element, TiXmlNode* methodDef);

    void FillTrimmedMethod(MethodDoc* trimMethod);

    /// serializes the method
    void Serialize(Serializer& stream);

    /// replaces types with the typedefs 
    void NormalizeAllTypes(RawTypedefLibrary* defLib, RawNamespaceDoc& classNamespace);

    TypeTokens* mReturnTokens;
    Array<Parameter*> mParsedParameters;
    Array<ExceptionDoc *> mPossibleExceptionThrows;

    String mName;
    String mDescription;
   };

  
  class RawVariableDoc
  {
  public:
    RawVariableDoc(void);

    ~RawVariableDoc(void);

    /// constructs class by loading the variable info from the element
    RawVariableDoc(TiXmlElement* element);

    /// serialize the variable
    void Serialize(Serializer& stream);

    TypeTokens* mTokens;

    String mName;
    String mDescription;

    bool mProperty;
  };


  class RawTypedefDoc
  {
  public:
    ///// PUBLIC METHODS///// 
    RawTypedefDoc() {}
    /// load typedef from element
    RawTypedefDoc(TiXmlElement* element);

    /// loads typedef from a tiny xml element
    void LoadFromElement(TiXmlElement* element);

    /// generates a namespace'd name for uniqueness in the typedef map
    String GenerateMapKey(void);

    /// serialize the typedef
    void Serialize(Serializer& stream);

    RawNamespaceDoc mNamespace;

    TypeTokens mDefinition;

    String mType;
  };

  class RawEnumDoc
  {
  public:
    ///// PUBLIC METHODS ///// 

    RawEnumDoc() {}

    /// load enum from the element and the enum Def tag
    RawEnumDoc(TiXmlElement* element, TiXmlNode* enumDef);

    /// serialize the list of enums
    void Serialize(Serializer& stream);

    ///// PUBLIC DATA ///// 

    Array<String> mEnumValues;

    String mName;
    String mDescription;
  };

  class RawClassDoc : public Object
  {
  public:
    ///// PUBLIC METHODS ///// 
    ZeroDeclareType(RawClassDoc);

    RawClassDoc();
    RawClassDoc(StringParam name);

    ~RawClassDoc();

    void LoadFromSkeleton(const ClassDoc &skeleClass);

    /// builds the maps from the arrays of document data
    void Build(void);

    /// clears the maps then calls build
    void Rebuild(void);

    /// serialize the class doc
    void Serialize(Serializer& stream);

    /// add data from passed in classdoc into this one
    void Add(RawClassDoc& classDoc);

    void FillTrimmedClass(ClassDoc* trimClass);

    /// load class data from the doc file we have saved
    bool LoadFromDoxygen(StringParam string);

    /// looks for bindevent macro calls in cpp docs
    void LoadEventsFromCppDoc(TiXmlDocument *doc);

    /// looks for notifyException macro calls in hpp docs
    void LoadEventsFromHppDoc(TiXmlDocument *doc);

    /// loads events for class with doxName in the doxyPath
    bool LoadEvents(String doxName, String doxyPath);

    /// calls passed in function on every docline until scope ends
    void ParseCodelinesInDoc(TiXmlDocument *doc, void(*fn)(RawClassDoc *,StringParam));

    void ParseFnCodelinesInDoc(TiXmlDocument *doc);

    /// sorts the event array and removes duplicates
    void SortAndPruneEventArray(void);

    /// get the path for this classDoc from the root of the doxygen directory
    bool SetRelativePath(StringRef doxyPath, StringRef filePath);

    /// loads from xmlDoc that will be loaded from doxyPath
    bool LoadFromXmlDoc(TiXmlDocument* doc, StringRef doxyPath,
      StringRef filePath, IgnoreList *ignoreList = nullptr);

    /// loads from the xmldoc that is passed in as a parameter
    bool LoadFromXmlDoc(TiXmlDocument* doc);

    /// try to get description for method by name passed, will also check base class
    const StringRef GetDescriptionForMethod(StringRef methodName);

    /// replaces typedef'd types with the underlying type
    void NormalizeAllTypes(RawTypedefLibrary* defLib);

    /// grabs meta database and loads tags and events from meta
    void LoadEventsAndTagsFromMeta(void);

    /// generates key fror classmap that incorperates namespace into classname
    String GenerateMapKey(void);

    void FillErrorInformation(StringParam fnTokenName, StringRef fnName, TypeTokens &tokens);

    void AddIfNewException(StringParam fnName, ExceptionDoc *errorDoc);

    ///// PUBLIC DATA ///// 

    HashMap<String, Array<RawMethodDoc*> > mMethodMap;
    HashMap<String, RawVariableDoc*> mVariableMap;

    Array<RawMethodDoc*> mMethods;
    Array<RawVariableDoc*> mVariables;

    Array<RawTypedefDoc*> mTypedefs;
    Array<RawEnumDoc*> mEnums;

    Array<EventDoc*> mEvents;

    Array<EventDoc*> mEventsSent;

    Array<EventDoc*> mEventsListened;

    Array<String> mFriends;

    Array<String> mTags;

    RawNamespaceDoc mNamespace;

    String mName;
    String mBaseClass;
    String mDescription;

    // relative to root of source directory
    String mRelativePath;

    // path to file where function was implemented
    String mBodyFile;

    String mHeaderFile;

    RawDocumentationLibrary *mParentLibrary;

  private:
    ///// PRIVATE METHODS ///// 
    /// tests if a function is actually a macro call (assumes you pass it valid funciton doc)
    bool fnIsMacroCall(TiXmlElement* element, TiXmlNode* currMethod);

    /// loads the Doxy xml file into doc. Returns False on failure to load
    bool loadDoxyfile(StringParam doxyPath, TiXmlDocument& doc);

    /// loads the Doxy xml file into doc and ignores anything in the ignoreList
    bool loadDoxyfile(StringParam doxyPath, TiXmlDocument& doc, IgnoreList &ignoreList);

    ///// PRIVATE MEMBERS ///// 
    bool mHasBeenLoaded;
  };

  // exactly the same so far as the full version, just different name
  class RawDocumentationLibrary : public Object
  {
  public:
    ///// PUBLIC METHODS ///// 

    ZeroDeclareType(RawDocumentationLibrary);

    ~RawDocumentationLibrary();

    /// build the class hashmap
    void Build(void);

    /// serialize this object
    void Serialize(Serializer& stream);

    /// fills the internal documentation class from this raw documentation library
    void FillTrimmedDocumentation(DocumentationLibrary &trimLib);

    /// loop over all classes, save their doc strings into files at directory
    void GenerateCustomDocumentationFiles(StringParam directory);

    /// creates a new class with name 'className', stores in internally, then returns it
    RawClassDoc* AddNewClass(StringParam className);

    /// grabs comments for overloaded functions
    void FillOverloadDescriptions(void);

    /// replaces typedef'd types with underlying types
    void NormalizeAllTypes(RawTypedefLibrary* defLib);

    /// load the doc library from the documentation directory (abs path)
    bool LoadFromDocumentationDirectory(StringParam directory);

    /// loads all the documentation from the entire doxygen directory minus ignored files
    bool LoadFromDoxygenDirectory(StringParam doxyPath);

    /// loads list of classes, tags, and events from skeleton documentation library
    /// returns false if list of classes was empty (i.e. there was nothing to load)
    bool LoadFromSkeletonFile(StringParam doxyPath, const DocumentationLibrary &library);

    /// loads the ignore list from the file at absPath
    void LoadIgnoreList(StringParam absPath);

    /// loads the events list from the file at absPath
    void LoadEventsList(StringParam absPath);

    void SaveEventListToFile(StringParam absPath);

    /// each namespace passed will be tried,
    /// combinations of namespaces must be passed explicitly
    RawClassDoc *GetClassByName(StringParam name, Array<String> &namespaces);

    ///// PUBLIC DATA ///// 
    HashMap<String, RawClassDoc*> mClassMap;

    Array<RawClassDoc*> mClasses;

    Array<String> mClassPaths;

    IgnoreList mIgnoreList;

    EventDocList mEvents;
  };


  class RawTypedefLibrary : public Object
  {
  public:
    ///// PUBLIC METHODS ///// 

    ZeroDeclareType(RawTypedefLibrary);

    /// loads any typedefs saved in a doc library
    void LoadTypedefsFromDocLibrary(RawDocumentationLibrary& docLib);

    /// loads typedefs from doxygen namespace documentation
    void LoadTypedefsFromNamespaceDocumentation(StringParam doxypath);

    ~RawTypedefLibrary(void);

    /// serialize the type def library to the specified directory (abs path)
    void GenerateTypedefDataFile(StringRef directory);

    /// serialize this typedef library
    void Serialize(Serializer& stream);

    /// load data from file at 'filepath' into this directory
    bool LoadFromFile(StringRef filepath);

    /// this is called after parsing all of the typedefs for easy reference
    void BuildMap(void);

    /// recursivly replace types until all typedefs are fully expanded
    void ExpandAllTypedefs(void);

    ///// PUBLIC DATA///// 

    Zero::Array<RawTypedefDoc> mTypedefArray;

    // points to typedefdocs in the TypedefArray
    Zero::UnsortedMap<String, RawTypedefDoc*> mTypedefs;

    IgnoreList mIgnoreList;
  };
  
}
