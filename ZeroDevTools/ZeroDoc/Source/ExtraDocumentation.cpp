#include "Precompiled.hpp"

#include "ExtraDocumentation.hpp"
#include "Serialization\Text.hpp"
#include "Platform\FileSystem.hpp"
#include "Parsing.hpp"

namespace Zero
{
  /// loop over documentation library and replace specified types and symbols
  void NormalizeDocumentation(Zero::DocumentationLibrary &doc, Replacements &replacements)
  {
    forRange(ClassDoc& classDoc, doc.Classes.all())
    {
      // replace matched properties types
      forRange(PropertyDoc &propDoc, classDoc.Properties.all())
      {
        propDoc.Type = Replace(replacements, propDoc.Type);
      }

      // replace matched method return types and argument types
      forRange(MethodDoc &methDoc, classDoc.Methods.all())
      {
        methDoc.ReturnValue = Replace(replacements, methDoc.ReturnValue);

        // for now since I am not sure if we ever rebuild the arguments string
        // I am going to replace both the args array and the arguments string itself
        methDoc.Arguments = Replace(replacements, methDoc.Arguments);
        forRange(MethodDoc::Argument &arg, methDoc.ParsedArguments.all())
        {
          arg.Type = Replace(replacements, arg.Type);
        }
      }

      // replace matched event types
      forRange(EventDoc &eventDoc, classDoc.EventsSent.all())
      {
        eventDoc.EventType = Replace(replacements, eventDoc.EventType);
      }
    }
  }

void LoadAndReplaceDocumentation(StringParam path, DocumentationLibrary& libary, Array<Replacement>& replacements)
{
  //read all of the extra documentation

  Array<ExtraDocumentation> extraDoc;
  TextLoader stream;
  if(FileExists(path))
  {
    Status status;
    stream.Open(status, path.c_str());
    SerializeName(extraDoc);
    stream.Close();
  }

  //loop through all the extra documentation and run the replacements
  //on the properties and methods (aka rename Vec3Param to Vec3)
  for(uint i = 0; i < extraDoc.size(); ++i)
  {
    for(uint j = 0; j < extraDoc[i].mProperites.size(); ++j)
    {
      ExtraDocumentation::Properties& prop = extraDoc[i].mProperites[j];
      prop.mDescription = Replace(replacements,prop.mDescription);
      prop.mType = Replace(replacements,prop.mType);
    }

    for(uint j = 0; j < extraDoc[i].mMethods.size(); ++j)
    {
      ExtraDocumentation::Methods& method = extraDoc[i].mMethods[j];
      method.mArguments = Replace(replacements,method.mArguments);
      method.mReturnValue = Replace(replacements,method.mReturnValue);
      method.mReturnValue = Replace(replacements,method.mReturnValue);
    }
  }
  
  //now actually go and replace the classes documentation with this extra documentation
  //(could combine this into the above loop but whatev...
  for(uint i = 0; i < extraDoc.size(); ++i)
  {
    //find the class 
    ClassDoc* classDoc = libary.ClassMap.findValue(extraDoc[i].mClassName,NULL);
    if(classDoc == NULL)
      continue;

    //loop through all the new properties
    for(uint j = 0; j < extraDoc[i].mProperites.size(); ++j)
    {
      ExtraDocumentation::Properties& prop = extraDoc[i].mProperites[j];
      PropertyDoc* propDoc = classDoc->PropertyMap.findValue(prop.mName,NULL);
      //make sure the property exists and replace it
      if(propDoc != NULL)
      {
        propDoc->Description = prop.mDescription;
        propDoc->Type = prop.mType;
      }
    }
    //loop through all the new methods
    for(uint j = 0; j < extraDoc[i].mMethods.size(); ++j)
    {
      ExtraDocumentation::Methods& method = extraDoc[i].mMethods[j];
      MethodDoc* metDoc = classDoc->MethodMap.findValue(method.mName,NULL);
      //make sure the method exists and replace it
      if(metDoc != NULL)
      {
        metDoc->Description = method.mDescription;
        metDoc->Arguments = method.mArguments;
        metDoc->ReturnValue = method.mReturnValue;
      }
    }
  }
}

}//namespace Zero
