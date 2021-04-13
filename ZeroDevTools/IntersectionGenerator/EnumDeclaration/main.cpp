#pragma once

#include "SharedUtilities.hpp"
#include <fstream>

typedef Zero::String String;
typedef const String& StringParam;
typedef Zero::StringBuilder StringBuilder;

typedef unsigned int uint;

const uint maxLength = 80 - 1;

void AddHeader(StringBuilder& builder, StringParam fileName)
{
  builder.Append("///////////////////////////////////////////////////////////////////////////////\n");
  builder.Append("///\n");
  builder.Append(String::Format("/// \\file %s.hpp\n",fileName.c_str()));
  builder.Append("/// Declaration of enum declaration macros.\n");
  builder.Append("///\n");
  builder.Append("/// Authors: Joshua Claeys, Joshua Davis, Auto-Generated\n");
  builder.Append("/// Copyright 2010-2011, DigiPen Institute of Technology\n");
  builder.Append("///\n");
  builder.Append("///////////////////////////////////////////////////////////////////////////////\n");
  builder.Append("#pragma once\n");
  builder.Append("\n");
  builder.Append("//Used to determine whether or not the enum is going to be used as a bit field\n");
  builder.Append("//or an index.  As a bit field, each index needs to be shifted.\n");
  builder.Append("#define _BitField() 1 << \n");
  builder.Append("#define _Indexed() \n");
  builder.Append("#define _AddNone(name) namespace name { enum {None}; }  \n");
  builder.Append("\n");
}

void WordWrapAppend(StringParam item, StringBuilder& builder, uint& currOffset, uint startingOffset)
{
  uint itemSize = item.size();

  //if the new string item will hit 80 characters minus one
  //(account for \ length), then wrap them to a new line starting
  //at the parenthesis length
  if(currOffset + itemSize >= maxLength)
  {
    //add white space padding till the "\"
    for(uint i = 0; i < maxLength - currOffset - 1; ++i)
      builder.Append(" ");
    builder.Append("\\\n");

    //add white space padding at the front to line up the next line
    currOffset = startingOffset;
    for(uint i = 0; i < currOffset; ++i)
      builder.Append(" ");
  }
  currOffset += itemSize;
  builder.Append(item);
}

void FillToEnd(StringBuilder& builder, uint currOffset)
{
  //add white space padding till the "\" and close the define
  for(uint i = 0; i < maxLength - currOffset - 1; ++i)
    builder.Append(" ");
  builder.Append("\\\n");
}

void BuildMacroDefinition(uint values, StringParam macroName, StringBuilder& builder, bool uniqueNames = false)
{
  String str = String::Format("#define %s%d(",macroName.c_str(),values);
  builder.Append(str);
  uint startingCharDistance = str.size();
  builder.Append("name,mode,");

  uint currSize = startingCharDistance + String("name,mode,").size();
  for(uint i = 0; i < values; ++i)
  {
    String item;
    if(uniqueNames)
    {
      item = Zero::BuildString("value",String::Format("%d",i + 1),",");
      WordWrapAppend(item,builder,currSize,startingCharDistance);
      if(i == values - 1)
        item = Zero::BuildString("name",String::Format("%d",i + 1),")");
      else
        item = Zero::BuildString("name",String::Format("%d",i + 1),",");
      WordWrapAppend(item,builder,currSize,startingCharDistance);
    }
    else
    {
      if(i == values - 1)
        item = Zero::BuildString("value",String::Format("%d",i + 1),")");
      else
        item = Zero::BuildString("value",String::Format("%d",i + 1),",");
      WordWrapAppend(item,builder,currSize,startingCharDistance);
    }
  }

  FillToEnd(builder,currSize);
}

void BuildInternalEnum(uint values, StringBuilder& builder, StringParam tab)
{
  uint startingCharDistance,currSize;
  //add the Enum values
  String start = "enum       Enum {";
  start = Zero::BuildString(tab,start);

  startingCharDistance = start.size();
  currSize = startingCharDistance;
  builder.Append(start);
  for(uint i = 0; i < values; ++i)
  {
    String item;
    if(i == values - 1)
      item = String::Format("value%d = mode %d};",i + 1, i);
    else
      item = String::Format("value%d = mode %d, ",i + 1, i);
    WordWrapAppend(item,builder,currSize,startingCharDistance);
  }
  FillToEnd(builder,currSize);
}

void BuildEnumSize(uint values, StringBuilder& builder, StringParam tab)
{
  String str;

  //Add the Size value
  str = String::Format("%senum {Size = %d};",tab.c_str(),values);
  builder.Append(str);
  FillToEnd(builder,str.size());
}

void BuildNamesArray(uint values, StringBuilder& builder, StringParam tab, bool uniqueNames = false)
{
  String start,item;
  uint startingCharDistance,currSize;

  String nameVar;
  if(uniqueNames)
    nameVar = "name";
  else
    nameVar = "value";

  start = "static const cstr Names[] = {";
  start = Zero::BuildString(tab,start);
  startingCharDistance = start.size();
  currSize = startingCharDistance;
  builder.Append(start);
  for(uint i = 0; i < values + 1; ++i)
  {
    String item;
    if(i == values)
      item = String::Format("NULL};");
    else
      item = String::Format("#%s%d, ",nameVar.c_str(),i + 1, i);
    WordWrapAppend(item,builder,currSize,startingCharDistance);
  }
  FillToEnd(builder,currSize);
}

void BuildValuesArray(uint values, StringBuilder& builder, StringParam tab, bool uniqueNames = false)
{
  String start, item;
  uint startingCharDistance, currSize;

  String nameVar;
  if (uniqueNames)
    nameVar = "name";
  else
    nameVar = "value";

  start = "static const uint Values[] = {";
  start = Zero::BuildString(tab, start);
  startingCharDistance = start.size();
  currSize = startingCharDistance;
  builder.Append(start);
  for (uint i = 0; i < values + 1; ++i)
  {
    String item;
    if (i == values)
      item = String::Format("};");
    else
    {
      const char *fmt = (i == values - 1) ? "%s%d" : "%s%d, ";
      item = String::Format(fmt, nameVar.c_str(), i + 1);
    }      
    WordWrapAppend(item, builder, currSize, startingCharDistance);
  }
  FillToEnd(builder, currSize);
}

void ExpandNames(uint values, StringBuilder& builder, bool uniqueNames = false)
{
  String str,start;

  String name = "_ExpandNames";
  if(uniqueNames)
    name = "_ExpandUniqueNames";

  BuildMacroDefinition(values,name,builder,uniqueNames);
  
  String tab = String("  ");
  //add the Type typedef
  str = Zero::BuildString(tab,String("namespace name"));
  builder.Append(str);
  FillToEnd(builder,str.size());
  str = Zero::BuildString(tab,String("{"));
  builder.Append(str);
  FillToEnd(builder,str.size());
  str = Zero::BuildString(tab,String("typedef uint Type;"));
  builder.Append(str);
  FillToEnd(builder,str.size());
  str = Zero::BuildString(tab, String("static const cstr EnumName = #name;"));
  builder.Append(str);
  FillToEnd(builder, str.size());

  ////add the Enum values
  BuildInternalEnum(values,builder,tab);
  
  //Add the Size value
  BuildEnumSize(values,builder,tab);

  //Add the array of strings for each enum value
  BuildNamesArray(values,builder,tab,uniqueNames);

  //Add the array of values for each enum constant
  BuildValuesArray(values,builder,tab,uniqueNames);

  str = Zero::BuildString(tab,"}");
  builder.Append(str);
  FillToEnd(builder,str.size());

  //builder.Append(Zero::BuildString(tab,"enum {}////used to force a closing semi-colon\n"));
  builder.Append("\n");
}

void DeclareBlock(uint values, StringBuilder& builder, StringParam declareStr, StringParam modeStr, bool uniqueNames = false, bool addNone = false)
{
  String str;

  str = String::Format("#define Declare%s%d(",declareStr.c_str(),values);
  builder.Append(str);
  uint startingCharDistance = str.size();
  builder.Append("name,");

  uint currSize = startingCharDistance + String("name,").size();
  for(uint i = 0; i < values; ++i)
  {
    String item;
    if(uniqueNames)
    {
      item = String::Format("v%d,",i + 1);
      WordWrapAppend(item,builder,currSize,startingCharDistance);
      if(i == values - 1)
        item = String::Format("n%d)",i + 1);
      else
        item = String::Format("n%d,",i + 1);
      WordWrapAppend(item,builder,currSize,startingCharDistance);
    }
    else
    {
      if(i == values - 1)
        item = String::Format("v%d)",i + 1);
      else
        item = String::Format("v%d,",i + 1);
      WordWrapAppend(item,builder,currSize,startingCharDistance);
    }
  }

  FillToEnd(builder,currSize);


  startingCharDistance = String("#define ").size();
  for(uint i = 0; i < startingCharDistance; ++i)
    builder.Append(" ");

  String start = String::Format("_ExpandNames%d(",values);
  if(uniqueNames)
    start = String::Format("_ExpandUniqueNames%d(",values);
  startingCharDistance += start.size();
  builder.Append(start);

  str = String::Format("name,%s,",modeStr.c_str());
  builder.Append(str);
  
  currSize = startingCharDistance + str.size();
  for(uint i = 0; i < values; ++i)
  {
    String item;
    if(uniqueNames)
    {
      item = String::Format("v%d,",i + 1);
      WordWrapAppend(item,builder,currSize,startingCharDistance);
      if(i == values - 1)
        item = String::Format("n%d)",i + 1);
      else
        item = String::Format("n%d,",i + 1);
      WordWrapAppend(item,builder,currSize,startingCharDistance);
    }
    else
    {
      if(i == values - 1)
        item = String::Format("v%d)",i + 1);
      else
        item = String::Format("v%d,",i + 1);
      WordWrapAppend(item,builder,currSize,startingCharDistance);
    }
  }

  if(addNone)
  {
    FillToEnd(builder,currSize);
    uint startSize = String("#define ").size();
    //add white space padding at the front to line up the next line
    for(uint i = 0; i < startSize; ++i)
      builder.Append(" ");
    builder.Append("_AddNone(name)");
  }

  builder.Append("\n");
}

int main()
{
  Zero::StringBuilder builder;

  AddHeader(builder,"EnumDeclaration");
  for(uint i = 0; i < 32; ++i)
    ExpandNames(i + 1,builder);
  builder.Append("\n");
  for(uint i = 0; i < 32; ++i)
    DeclareBlock(i + 1,builder,"Enum","_Indexed()");
  builder.Append("\n");
  for(uint i = 0; i < 32; ++i)
    DeclareBlock(i + 1,builder,"BitField","_BitField()", false, true);
  builder.Append("\n");

  String sourceStr = builder.ToString();
  cstr source = sourceStr.c_str();

  std::ofstream stream;
  stream.open("EnumDeclaration.hpp");
  stream.write(sourceStr.c_str(),sourceStr.size());
  stream.close();
  
  builder.Deallocate();
  AddHeader(builder,"NamedEnumDeclaration");
  for(uint i = 0; i < 32; ++i)
    ExpandNames(i + 1,builder,true);
  builder.Append("\n");
  for(uint i = 0; i < 32; ++i)
    DeclareBlock(i + 1,builder,"Enum","_Indexed()",true);
  builder.Append("\n");
  for(uint i = 0; i < 32; ++i)
    DeclareBlock(i + 1,builder,"BitField","_BitField()",true, true);
  builder.Append("\n");

  sourceStr = builder.ToString();
  source = sourceStr.c_str();

  stream.open("NamedEnumDeclaration.hpp");
  stream.write(sourceStr.c_str(),sourceStr.size());
  stream.close();

  return 0;
}
