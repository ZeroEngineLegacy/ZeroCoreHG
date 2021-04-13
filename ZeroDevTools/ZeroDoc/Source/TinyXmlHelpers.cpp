#include "Precompiled.hpp"

#include "TinyXmlHelpers.hpp"
#include "..\TinyXml\tinyxml.h"
#include "Parsing.hpp"
#include "Platform\FileSystem.hpp"
#include "String\CharacterTraits.hpp"

namespace Zero
{

String GetDoxygenName(String name)
{
  char buffer[1000];
  const char* input = name.c_str();

  char* output = buffer;

  while(*input != '\0')
  {
    if( isupper(*input) )
    {
      *output = '_';
      ++output;
    }

    *output = tolower(*input);
    ++output;
    ++input;
  }

  *output = '\0';

  return buffer;
}

String ToTypeName(TiXmlElement* parent, cstr name)
{
  String typeName = GetElementValue(parent, "type", "ref");
  if(!typeName.Empty())
    return typeName;
  return GetElementValue(parent->FirstChildElement(), "type");
}

TiXmlElement* FindElementWithAttribute(TiXmlElement* element, cstr attribute, cstr value)
{
  TiXmlElement* pChild = NULL;
  for ( pChild = element->FirstChildElement(); pChild != 0; pChild = pChild->NextSiblingElement()) 
  {
    cstr elementAttValue = pChild->Attribute(attribute);
    if(elementAttValue && strcmp(elementAttValue,value)==0)
    {
      return pChild;
    }
  }
  return NULL;
}

void RecursiveExtract(StringBuilder& builder, TiXmlNode* node)
{
  if(TiXmlText* text = node->ToText())
    builder << " " << text->Value();

  if(TiXmlElement* element = node->ToElement())
  {
    TiXmlNode* child = element->FirstChild();
    while(child)
    {
      RecursiveExtract(builder, child);

      child = child->NextSibling();
    }
    builder << " ";
  }

}

String DoxyToString(TiXmlElement* parent, cstr name)
{
  StringBuilder builder;

  TiXmlElement* mainElement = parent->FirstChildElement(name);

  if(mainElement)
  {
    RecursiveExtract(builder, mainElement);
  }

  return builder.ToString();
}

String GetElementValue(TiXmlElement* parent, cstr name, cstr second)
{
  TiXmlElement* element = parent->FirstChildElement(name);
  if(element)
  {
    if(second)
      element = element->FirstChildElement(second);

    if(element)
    {
      const char* text = element->GetText();
      if( text )
      {
        return text;
      }
    }
  }
  return String();
}

void FindClassesWithBase(StringParam doxyPath, HashSet<String>& classes, HashSet<String>& baseClassesToInclude,
                         HashSet<String>& baseClassesToIgnore, HashSet<String>& classesToIgnore)
{
  typedef HashMap<String,String> ClassBaseMap;
  ClassBaseMap classToBase;

  FileRange files(BuildString(doxyPath.c_str(),"\\xml\\"));
  
  //iterate through all of the files searching for classes
  for(; !files.Empty(); files.PopFront())
  {
    String fileName = files.Front();
  
    String classHeader = "class_zero_1_1";
    String structHeader = "struct_zero_1_1";
  
    //only open class or struct documentation files
    if(String(fileName.SubStringFromByteIndices(0,classHeader.SizeInBytes())) != classHeader &&
      String(fileName.SubStringFromByteIndices(0,structHeader.SizeInBytes())) != structHeader)
      continue;
  
    String fullFileName = BuildString(doxyPath.c_str(),"\\xml\\",fileName.c_str());

    //try to open the class file
    TiXmlDocument doc(fullFileName.c_str());
    bool loadOkay = doc.LoadFile();

    //should never happen since we were told this file exists, but check to be safe
    if(!loadOkay)
      return;

    TiXmlElement* doxygenElement = doc.FirstChildElement("doxygen");
    TiXmlElement* compounddef = doxygenElement->FirstChildElement("compounddef");

    //get the name of the class (strip out the Zero:: parts)
    String className = GetElementValue(compounddef, "compoundname");
    uint nameStart = className.FindLastOf(':').SizeInBytes() + 1;
    className = className.SubStringFromByteIndices(nameStart, className.SizeInBytes() - nameStart);

    //get the name of the base class (this doesn't have the namespace,
    //but it will have templates so strip out anything after the first <)
    String baseName = GetElementValue(compounddef, "basecompoundref");
    uint templateStart = baseName.FindFirstOf('<').SizeInBytes();
    if(templateStart < baseName.SizeInBytes())
      baseName = baseName.SubStringFromByteIndices(0,templateStart);
    
    //mark that this class has the given base
    classToBase[className] = baseName;
  }

  //now we want to iterate through all of the classes and see if this class should be documented
  //it should be document if it has one of the bases to include but doesn't have an ignore base first.
  //Also, if it itself is marked as ignore then don't add it
  ClassBaseMap::range r = classToBase.All();
  for(; !r.Empty(); r.PopFront())
  {
    String className = r.Front().first;

    //see if we ignore this class such as random events (e.g. ChatEvent)
    if(classesToIgnore.FindValue(className,"").Empty() == false)
      continue;

    //iterate through the bases until we have no base
    String baseName = r.Front().second;
    while(!baseName.Empty())
    {
      //if we ignore this base then stop. e.g. Widget
      if(baseClassesToIgnore.FindValue(baseName,"").Empty() == false)
        break;

      //if this base should be included then insert the class and go to the next one. e.g. Component
      if(baseClassesToInclude.FindValue(baseName,"").Empty() == false)
      {
        classes.Insert(className);
        break;
      }

      baseName = classToBase.FindValue(baseName,"");
    }
  }
}

namespace AddedType
{
enum DataType {None, Property, Method};
}


String NormalizeDocumentation(StringRange text)
{
  if(text.Empty())
    return String();

  uint size = text.SizeInBytes();
  char* buffer = (char*)alloca(size + 1);

  uint outIndex = 0;
  forRange(Rune current, text.All())
  {
    if(IsSpace(current))
    {
      // Skip all leading spaces
      if(outIndex == 0)
        continue;

      // Skip multiple spaces
      if(outIndex > 0 && IsSpace(buffer[outIndex  - 1]))
        continue;
    }

    // Remove spaces before symbols
    if(IsSymbol(current) && outIndex > 0 && IsSpace(buffer[outIndex - 1]))
    {
      buffer[outIndex - 1] = current.value;
      continue;
    }

    buffer[outIndex++] = current.value;
  }

  // Remove trailing white space
  while(outIndex > 0 && IsSpace(buffer[outIndex - 1]))
    --outIndex;

  // Null terminate
  buffer[outIndex] = '\0';

  return buffer;
}

String FindFile(StringParam basePath, StringParam fileName)
{
  String fullPath = FilePath::Combine(basePath, fileName);
  if (FileExists(fullPath))
    return fullPath;

  FileRange range(basePath);
  for (; !range.Empty(); range.PopFront())
  {
    FileEntry entry = range.frontEntry();
    String subPath = entry.GetFullPath();
    if (IsDirectory(subPath))
    {
      String subFilePath = FindFile(subPath, fileName);
      if (!subFilePath.Empty())
        return subFilePath;
    }
  }
  return String();
}

String FindFile(StringParam basePath, StringParam fileName, IgnoreList &ignoreList)
{
  String fullPath = FilePath::Combine(basePath, fileName);
  
  if (ignoreList.DirectoryIsOnIgnoreList(fullPath))
  {
    WriteLog("Ignoring File: %s\n", fullPath);
    return String();
  }

  if (FileExists(fullPath))
    return fullPath;

  FileRange range(basePath);
  for (; !range.Empty(); range.PopFront())
  {
    FileEntry entry = range.frontEntry();
    String subPath = entry.GetFullPath();

    if (ignoreList.DirectoryIsOnIgnoreList(subPath))
    {
      WriteLog("Ignoring File: %s\n", subPath);
      return String();
    }

    if (IsDirectory(subPath))
    {
      String subFilePath = FindFile(subPath, fileName, ignoreList);
      if (!subFilePath.Empty())
        return subFilePath;
    }
  }
  return String();
}

}//namespace Zero
