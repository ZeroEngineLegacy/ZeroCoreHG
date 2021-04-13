#include "Precompiled.hpp"

#include "Logging.hpp"
#include "Support\FileSupport.hpp"
#include "TinyXmlHelpers.hpp"
#include "Serialization\Text.hpp"
#include "Platform\FileSystem.hpp"

namespace Zero
{

//class to sort the ClassDoc based upon its name
struct ClassDocSorter
{
  bool operator()(ClassDoc* lhs, ClassDoc* rhs) const
  {
    return strcmp(lhs->mName.c_str(),rhs->mName.c_str()) < 0;
  }
};

void WarnAndLogUndocumented(Array<ClassDoc *>& classes, StringParam doxyPath,
                            StringParam docPath, bool verbose, String log)
{
  //if we are not marked as verbose and there is no log to write to then there's nothing to do
  if(!verbose && log.Empty())
    return;

  //to make the output in a reasonable order, sort the classes
  Sort(classes.All(),ClassDocSorter());

  //build up both undocumented classes and properties into one builder
  StringBuilder builder;
  HashSet<String> classesToDocument;
  Array<String> undocumentedClasses;
  FilterIgnoredClasses(classes, classesToDocument, undocumentedClasses, doxyPath, docPath);
  WarnUndocumentedClasses(classes, classesToDocument, builder);
  WarnAndLogUndocumentedProperties(classes, builder);

  //if there is no warnings then there is nothing to do
  String outStr = builder.ToString();
  if(outStr.Empty())
    return;

  //verbose means we should print the warnings
  if(verbose)
    printf("%s",outStr.c_str());

  //if there is a log then we need to write the file
  if(!log.Empty())
  {
    //if we aren't verbose then tell the user that there were warnings
    //put out to the log file that they should go and check.
    if(!verbose)
      printf("Warning: Properties are not documented. Check log file %s.\n",log.c_str());
    WriteStringRangeToFile(log, outStr);
  }
}

void LoadSet(StringParam fileName, HashSet<String>& data)
{
  TextLoader stream;
  if(FileExists(fileName))
  {
    Status status;
    stream.Open(status, fileName.c_str());
    SerializeName(data);
    stream.Close();
  }
}

void FilterIgnoredClasses(Array<ClassDoc *>& classes, HashSet<String>& classesToDocument, Array<String>& undocumentedClasses,
                          StringParam doxyPath, StringParam docPath)
{
  HashSet<String> basesToInclude;
  HashSet<String> basesToIgnore;
  HashSet<String> classesToIgnore;

  //load all of the ignore/include data
  LoadSet(BuildString(docPath.c_str(),"BasesToInclude.txt"),basesToInclude);
  LoadSet(BuildString(docPath.c_str(),"BasesToIgnore.txt"),basesToIgnore);
  LoadSet(BuildString(docPath.c_str(),"ClassesToIgnore.txt"),classesToIgnore);

  FindClassesWithBase(doxyPath,classesToDocument,basesToInclude,basesToIgnore,classesToIgnore);

  uint index = 0;
  while(index < classes.Size())
  {
    ClassDoc* classDoc = classes[index];
    String findVal = classesToDocument.FindValue(classDoc->mName,"");
    if(findVal.Empty() == true)
    {
      classes[index] = classes[classes.Size() - 1];
      classes.PopBack();
    }
    
    ++index;
  }
}

void WarnAndLogUndocumentedProperties(Array<ClassDoc *>& classes, StringBuilder& builder)
{
  //if any property or method is has an empty description then it is
  //considered to not be documented. If we encounter any of these then build
  //the string that says this.
  forRange(ClassDoc* classDoc, classes.All())
  {
    if(classDoc->mDescription.Empty())
    {
      String str = String::Format("%s: Class itself is not documented.\n", classDoc->mName.c_str());
      builder.Append(str);
    }

    for(uint i = 0; i < classDoc->mProperties.Size(); ++i)
    {
      if(classDoc->mProperties[i]->mDescription.Empty())
      {
        String str = String::Format("%s: %s is not documented.\n",
          classDoc->mName.c_str(),classDoc->mProperties[i]->mName.c_str());
        builder.Append(str);
      }
    }

    for(uint i = 0; i < classDoc->mMethods.Size(); ++i)
    {
      if(classDoc->mMethods[i]->mDescription.Empty())
      {
        String str = String::Format("%s: %s is not documented.\n",
          classDoc->mName.c_str(),classDoc->mMethods[i]->mName.c_str());
        builder.Append(str);
      }
    }
  }
}

void WarnUndocumentedClasses(Array<ClassDoc *>& classes, HashSet<String>& classesToDocument, StringBuilder& builder)
{
  //go through all of the classes that are marked as documented and remove them
  //from the set of what should be documented. What's left is what should be documented but isn't.
  forRange(ClassDoc* classDoc, classes.All())
  {
    String findVal = classesToDocument.FindValue(classDoc->mName,"");
    if(findVal.Empty() == false)
    {
      classesToDocument.Erase(classDoc->mName);
    }
  }

  //if there is nothing that isn't documented that should be then there's nothing to do
  if(classesToDocument.Empty())
    return;

  //in order to display the undocumented classes in a reasonable way, sort the
  //classes. To do this first convert them to an array then sort them.
  Array<String> undocumentedClasses;
  for(HashSet<String>::range r = classesToDocument.All(); !r.Empty(); r.PopFront())
    undocumentedClasses.PushBack(r.Front());
  Sort(undocumentedClasses.All());

  //now build up the string that is all of the undocumented classes.
  for(uint i = 0; i < undocumentedClasses.Size(); ++i)
    builder.Append(BuildString("Warning: Class ",undocumentedClasses[i]," is undocumented.\n"));\
}

void WarnNeedsWikiPage(Array<WikiUpdatePage>& pagesToUpdate, Array<ClassDoc *>& documentedClasses,
                       StringParam doxyPath, StringParam docPath, 
                       bool verbose, StringParam log)
{
  //if we are not marked as verbose and there is no log to write to then there's nothing to do
  if(!verbose && log.Empty())
    return;

  //build up both undocumented classes and properties into one builder
  StringBuilder builder;

  HashSet<String> docClasses;
  for(uint i = 0; i < documentedClasses.Size(); ++i)
    docClasses.Insert(documentedClasses[i]->mName);

  for(uint i = 0; i < pagesToUpdate.Size(); ++i)
  {
    String className = pagesToUpdate[i].mPageToUpdate;
    String findVal = docClasses.FindValue(className,"");
    if(findVal.Empty() == false)
    {
      docClasses.Erase(className);
    }
  }
  
  //in order to display the undocumented classes in a reasonable way, sort the
  //classes. To do this first convert them to an array then sort them.
  Array<String> noWikiClasses;
  for(HashSet<String>::range r = docClasses.All(); !r.Empty(); r.PopFront())
    noWikiClasses.PushBack(r.Front());
  Sort(noWikiClasses.All());

  //now build up the string that is all of the undocumented classes.
  for(uint i = 0; i < noWikiClasses.Size(); ++i)
    builder.Append(BuildString("Warning: Class ",noWikiClasses[i]," has no wiki page.\n"));

  //if there is no warnings then there is nothing to do
  String outStr = builder.ToString();
  if(outStr.Empty())
    return;

  //verbose means we should print the warnings
  if(verbose)
    printf("%s",outStr.c_str());

  //if there is a log then we need to write the file
  if(!log.Empty())
  {
    //if we aren't verbose then tell the user that there were warnings
    //put out to the log file that they should go and check.
    if(!verbose)
      printf("Warning: Classes that were documented did not have their wiki pages updated.Check log file %s.\n",log.c_str());
    WriteStringRangeToFile(log, outStr);
  }
}

}//namespace Zero
