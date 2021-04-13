#pragma once
#include "String\StringBuilder.hpp"
#include "Engine\Documentation.hpp"
#include "Support\StringReplacement.hpp"
#include "RawDocumentation.hpp"

class TiXmlElement;
class TiXmlNode;


#define cDirectorySeparatorChar '/'

namespace Zero
{
  /// convertes class name passed in into something that would actually be in a doxygen filename
  String GetDoxygenName(String name);

  /// gets name of type frim the TiXmlElement pointer the cstr is unused no idea why it is there
  String ToTypeName(TiXmlElement* parent, cstr name);

  /// finds next element that has the passed attribute of value 'value'
  TiXmlElement* FindElementWithAttribute(TiXmlElement* element, cstr attribute, cstr value);

  /// extracts text recurs from all children
  void RecursiveExtract(StringBuilder& builder, TiXmlNode* node);

  /// calls recursiveExtract on first element to try to get name information
  String DoxyToString(TiXmlElement* parent, cstr name);

  /// get text from child of name 'name' or if second is passed, of 'name's child named 'second'
  String GetElementValue(TiXmlElement* parent, cstr name, cstr second = nullptr);

  /// find all classes with base 'baseClass' and iterates through each classes base classes
  void FindClassesWithBase(StringParam doxyPath, HashSet<String>& classes, HashSet<String>& baseClassesToInclude,
    HashSet<String>& baseClassesToIgnore, HashSet<String>& classesToIgnore);

  /// fixes all the weird formatting crap that doxygen leaves in descriptions in the xml files
  String NormalizeDocumentation(StringRange text);

  /// recursivly search directory and subdirectories of 'basePath' searching for file named 'fileName'
  String FindFile(StringParam basePath, StringParam fileName);

  /// recursivly search directory and subdirectories of 'basePath' searching for file named 'fileName'
  /// however, all files/directories inside of the ignoreList are skipped
  String FindFile(StringParam basePath, StringParam fileName, IgnoreList &ignoreList);
}//namespace Zero
