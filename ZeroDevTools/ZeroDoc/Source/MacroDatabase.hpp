#pragma once
#include <Engine/EngineStandard.hpp>
#include "DocTypeTokens.hpp"
#include "RawDocumentation.hpp"
#include "DocTypeParser.hpp"

namespace Zero
{

  DeclareEnum3(MacroOptions, Location, Comment, StartCustomOptions);

  static const char *MacroOptionStrings[] = {
    "location",
    "comment"
  };

  struct MacroData
  {
    /// constructs MacroData by parsing code in string form
    MacroData(StringRef macroString);

    /// constructs MacroData by getting code from given codeline range
    MacroData(TiXmlNode* startCodeline, TiXmlNode* endCodeline, TypeTokens& startTokens);

    String mName;

    TypeTokens mMacroBody;

    Array<String> mParameters;
  };

  struct MacroCall
  {
    /// Parses macroComment options that were in the comment of this MacroCall
    void ParseOptions(TypeTokens &tokens);

    /// Expands the macro by replacing it with the macro itself while subing in passed arguments
    void ExpandCall(void);

    /// Adds any documentatble code to the RawClassDoc of the class this call was made from
    void AddExpandedMacroDocToRawClass(void);

    /// Macro call will search for macro by name instead of by location MacroOption
    bool LoadMacroWithName(StringParam name);

    /// Allows user to query for any MacroOptions by name. NOTE: will also check MacroArguments.
    StringRef GetOption(StringParam optionName);

    /// Allows user to add Macro options by name, will override exising option if name already exists
    void AddOption(StringParam optionName, StringParam optionValue);

    /// When called, all options are check to see if they themselves should expand into a prev option
    void DoOptionExpansion(void);

    MacroData *mMacro;

    RawClassDoc *mClass;

    TypeTokens mExpandedMacro;

    Array<String> mMacroArgs;

  private:
    UnsortedMap<String, String> mOptions;

  };

  class MacroDatabase
  {
  public:
    /// get current instance of the macro database
    static MacroDatabase *GetInstance(void);

    /// returns id for macro
    MacroData* FindMacro(StringParam name, StringParam location);

    /// saves macro into a macrodata object witha unique id so we are guarenteed not over override macros
    MacroData* SaveMacroFromDoxyfile(TiXmlDocument *macroFile, StringParam name, StringParam id);

    /// Saves MacroCall and comment from code IF it has the 'MacroComment' keyword in comment
    void SaveMacroCallFromClass(RawClassDoc *classDoc, TiXmlElement* element, 
      TiXmlNode* currMethod);

    /// Iterates over all saved MacroCall, expands them, and does any macro call substitution
    void ProcessMacroCalls(void);

    /// returns option if it exists in one of the expanded macros on the stack. Otherwise returns empty string
    StringRef SearchMacroExpandStackForOption(StringRef option);

    String mDoxyPath;

    Array<MacroCall *> mMacroExpandStack;

    Array<MacroCall> mMacroCalls;

    UnsortedMap<String, MacroData *> mUniqueMacros;

    UnsortedMap<String, MacroData *> mMacrosByName;
  };

}

