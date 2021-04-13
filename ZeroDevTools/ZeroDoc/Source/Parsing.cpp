#include "Precompiled.hpp"
#include "Parsing.hpp"
#include "Common/String/CharacterTraits.hpp"

namespace Zero
{

String BuildDoc(ClassDoc& classDoc, Replacements& replacements)
{
  StringBuilder builder;

  if(!classDoc.mBaseClass.Empty())
  {
    builder << "<h4 class=\"FunctionDoc\">";
    builder << "Base Class - ";
    Replace(builder, replacements, classDoc.mBaseClass);
    builder << "</h4>";
  }

  builder << "<h2> Description </h2>";
  builder << "<p>";
  Check(classDoc.mName, "Class Desc", classDoc.mDescription);
  Replace(builder, replacements, classDoc.mDescription);
  builder << "</p>";

  builder << "<h2> Events </h2>";
  if(!classDoc.mEventsSent.Empty())
  {
    forRange(EventDoc* eventDoc, classDoc.mEventsSent.All())
    {
      builder << "<h4 class=\"FunctionDoc\">";
      builder << "On Events." << eventDoc->mName << " Sends ";
      Replace(builder, replacements, eventDoc->mType);
      builder << "</h4>";
    }
  }
  else
  {
    builder << "<h3> No Events </h3>";
  }

  builder << "<h2> Properties </h2>";

  if(!classDoc.mProperties.Empty())
  {
    forRange(PropertyDoc* propertyDoc, classDoc.mProperties.All())
    {
      builder << "<h4 class=\"FunctionDoc\">";
      builder << propertyDoc->mName;
      builder << " - ";
      Replace(builder, replacements, propertyDoc->mType);

      builder << "</h4>";

      builder << "<p>";
      Check(classDoc.mName, propertyDoc->mName, propertyDoc->mDescription);
      Replace(builder, replacements, propertyDoc->mDescription);
      builder << "</p>";

    }
  }
  else
  {
    builder << "<h3> No Properties </h3>";
  }


  builder << "<h2> Methods </h2>";
  if(!classDoc.mMethods.Empty())
  {
    forRange(MethodDoc* methodDoc, classDoc.mMethods.All())
    {
      builder << "<h4 class=\"FunctionDoc\"> ";
      Replace(builder, replacements, methodDoc->mReturnType);

      builder << " " << methodDoc->mName << " ";
      Replace(builder, replacements, methodDoc->mParameters);
      builder << "</h4>";

      builder << "<p>";
      Check(classDoc.mName, methodDoc->mName, methodDoc->mDescription);
      Replace(builder, replacements, methodDoc->mDescription);
      builder << "</p>";
    }
  }
  else
  {
    builder << "<h3> No Methods </h3>";
  }

  return builder.ToString();
}



void Check(StringRef className, StringRef element, StringRef string)
{
  bool nonWhitespace = false;
  forRange(Rune c,  string.All())
  {
    if(!IsSpace(c))
      nonWhitespace = true;
  }

  if(!nonWhitespace)
  { 
    OutputDebugStringA(className.c_str());
    OutputDebugStringA("  Missing ");
    OutputDebugStringA(element.c_str());
    OutputDebugStringA("\n");
  }
}

}//namespace Zero
