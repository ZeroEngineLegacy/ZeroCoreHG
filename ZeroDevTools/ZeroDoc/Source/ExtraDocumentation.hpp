#pragma once
#include "Serialization\Serialization.hpp"
#include "String\String.hpp"
#include "Containers\Array.hpp"
#include "Engine\Documentation.hpp"
#include "Support\StringReplacement.hpp"

namespace Zero
{

//Extra documentation that needs to be injected into a class for some reason.
//Currently used to add comments for joints because the comment is in a macro.
struct ExtraDocumentation
{
  void Serialize(Serializer& stream)
  {
    SerializeName(mClassName);
    SerializeName(mProperites);
    SerializeName(mMethods);
  }

  struct Properties
  {
    void Serialize(Serializer& stream)
    {
      SerializeName(mName);
      SerializeName(mType);
      SerializeName(mDescription);
    }
    String mName;
    String mType;
    String mDescription;
  };
  struct Methods
  {
    void Serialize(Serializer& stream)
    {
      SerializeName(mName);
      SerializeName(mArguments);
      SerializeName(mReturnValue);
      SerializeName(mDescription);
    }
    String mName;
    String mArguments;
    String mReturnValue;
    String mDescription;
  };

  String mClassName;
  Array<Properties> mProperites;
  Array<Methods> mMethods;
};

void LoadAndReplaceDocumentation(StringParam path, DocumentationLibrary& libary, Array<Replacement>& replacements);

void NormalizeDocumentation(Zero::DocumentationLibrary &doc, Replacements &replacements);

}//namespace Zero
