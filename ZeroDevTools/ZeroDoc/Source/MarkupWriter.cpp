#include "Precompiled.hpp"
#include "Engine/EngineContainers.hpp"
#include "DocConfiguration.hpp"
#include "Platform/FileSystem.hpp"
#include "Engine/Documentation.hpp"
#include "Serialization/Simple.hpp"
#include "Support/FileSupport.hpp"
#include "Platform/FilePath.hpp"
#include "MarkupWriter.hpp"

/*
#include "Serialization/Text.hpp"
#include "Pages.hpp"
*/

namespace Zero
{
void WriteTagIndices(String outputDir, DocToTags& tagged, DocumentationLibrary &docLib)
{
  String fileName = FilePath::Combine(outputDir, "CodeIndex.rst");

  DocRange r = tagged.All();

  StringBuilder markup;
  markup << "Code Index\n";
  markup << "=================================" << "\n\n";


  markup << "Vector Math\n";
  markup << "----------------------------------\n\n";
  markup << "*  :doc:`Reference/Vector`\n";
  markup << "*  :doc:`Reference/Quat`\n\n";

  for(;!r.Empty();r.PopFront())
  {
    Array<ClassDoc*>& stuff = r.Front().second;
    Sort(stuff.All());

    String tag = r.Front().first;

    markup << tag << "\n";
    markup << "----------------------------------" << "\n\n";

    // create the reference list
    forRange(ClassDoc* doc, stuff.All())
    {
        markup << String::Format("*  :doc:`Reference/%s`\n", doc->mName.c_str(), doc->mName.c_str());
    }

    markup << "\n";
  }

  markup << "\
.. toctree::\n\
\t:hidden:\n\
\t:includehidden:\n\
\t:name: mastertoc\n\
\t:maxdepth: 1\n\
\t:titlesonly:\n\n";
  //output the doctree
  forRange(ClassDoc* doc, docLib.mClasses.All())
  {
    markup << "\tReference/" << doc->mName << "\n";
  }

  String text = markup.ToString();

  WriteStringRangeToFile(fileName, text);
}

void WriteOutAllMarkdownFiles(Zero::DocGeneratorConfig& config)
{
  printf("Mark up\n");

  // check if we are outputting class markup
  if (FileExists(config.mTrimmedOutput.c_str()))
  {
    StringRef directory = config.mMarkupDirectory;

    CreateDirectoryAndParents(directory);

    DocumentationLibrary doc;
    LoadFromDataFile(doc, config.mTrimmedOutput);
    doc.FinalizeDocumentation();

    DocToTags tagged;

    StringBuilder fileList;

    //Upload the class' page to the wiki, making sure to perform the link replacements
    forRange(ClassDoc* classDoc, doc.mClasses.All())
    {
      String filename = BuildString(classDoc->mName, ".rst");

      String fullPath = FilePath::Combine(directory, "Reference");

      CreateDirectoryAndParents(fullPath);

      fullPath = FilePath::Combine(fullPath, filename);

      fullPath = FilePath::Normalize(fullPath);

      ClassMarkupWriter::WriteClass(fullPath, classDoc, doc, tagged);

      fileList << classDoc->mName << '\t' << FilePath::Combine(".", "Reference", filename) << '\n';
    }

    WriteStringRangeToFile(FilePath::Combine(directory, "classList.txt"), fileList.ToString());

    WriteTagIndices(directory, tagged, doc);
  }


  // check if we outputting commands
  if (!config.mCommandListFile.Empty())
  {
    String output = FilePath::Combine(config.mMarkupDirectory, "CommandRef.rst");
    output = FilePath::Normalize(output);
    CommandRefWriter::WriteCommandRef(config.mCommandListFile, output);
  }

  // check if we are outputing events
  if (!config.mEventsOutputLocation.Empty())
  {
    String output = FilePath::Combine(config.mMarkupDirectory, "EventList.rst");
    output = FilePath::Normalize(output);
    EventListWriter::WriteEventList(config.mEventsOutputLocation, output);
  }
}


////////////////////////////////////////////////////////////////////////
// Macro Toolbox
////////////////////////////////////////////////////////////////////////
// start indent Section
#define StartIndentSection(x) do {++x.mCurrentIndentationLevel
// end indent section
#define EndIndentSection(x) --x.mCurrentIndentationLevel;}while(0)

// start header section
#define StartHeaderSection(x) if((0)){}else{++x.mCurrentSectionHeaderLevel;
// end header section
#define EndHeaderSection(x) --x.mCurrentSectionHeaderLevel;};

////////////////////////////////////////////////////////////////////////
// BaseMarkupWriter
////////////////////////////////////////////////////////////////////////
void BaseMarkupWriter::IndentToCurrentLevel(void)
{
  mOutput << String::Repeat('\t', mCurrentIndentationLevel);
}

void BaseMarkupWriter::WriteOutputToFile(StringRef file)
{
  WriteStringRangeToFile(file, mOutput.ToString());
}

void BaseMarkupWriter::InsertNewUnderline(uint length, uint headerLevel)
{
  switch (headerLevel + mCurrentSectionHeaderLevel)
  {
  case 0:
    mOutput << String::Repeat('=', length);
    break;
  case 1:
    mOutput << String::Repeat('-', length);
    break;
  case 2:
    mOutput << String::Repeat('^', length);
    break;
  case 3:
    mOutput << String::Repeat('~', length);
    break;
    // will create purposfully invalid underline if invalid header level chosen
  default:
    mOutput << String::Repeat('?', length);
    break;
  }

  mOutput << "\n\n";
}

void BaseMarkupWriter::InsertNewSectionHeader(StringRef sectionName)
{
  mOutput << sectionName << "\n";
  InsertNewUnderline(sectionName.SizeInBytes());
}

void BaseMarkupWriter::InsertCollapsibleSection()
{
  // all functions that output anything besides headers will probably do this
  mOutput << ".. rst-class:: collapsible\n\n";
}

////////////////////////////////////////////////////////////////////////
// ClassMarkupWriter
////////////////////////////////////////////////////////////////////////
void ClassMarkupWriter::WriteClass(
  StringParam outputFile,
  ClassDoc* classDoc,
  DocumentationLibrary &lib,
  DocToTags& tagged)
{
  // first things first, set up the tags for this class
  forRange(StringRef tag, classDoc->mTags.All())
  {
    tagged[tag].PushBack(classDoc);
  }

  // do the magic for getting directory and file
  ClassMarkupWriter writer(classDoc->mName, classDoc);

  // top of the file
  writer.InsertClassRstHeader();
  StartHeaderSection(writer);

  // Properties
  writer.mOutput << ".. _Reference" << writer.mName << "Properties:\n\n";

  writer.InsertNewSectionHeader("Properties");

  forRange(PropertyDoc *prop, classDoc->mProperties.All())
  {
    writer.InsertProperty(*prop);
  }

  // Methods
  writer.mOutput << ".. _Reference" << writer.mName << "Methods:\n\n";

  writer.InsertNewSectionHeader("Methods");

  forRange(MethodDoc *method, classDoc->mMethods.All())
  {
    writer.InsertMethod(*method);
  }

  // bottom of the file
  EndHeaderSection(writer);
  writer.InsertClassRstFooter();

  writer.WriteOutputToFile(outputFile);
}

ClassMarkupWriter::ClassMarkupWriter(StringParam name, ClassDoc* classDoc) 
  : BaseMarkupWriter(name), mClassDoc(classDoc)
{
}

void ClassMarkupWriter::InsertClassRstHeader(void)
{
  mOutput << ".. _Reference" << mName << ":\n\n" << ".. rst-class:: searchtitle\n\n";

  InsertNewSectionHeader(mName);

  mOutput << ".. rst-class:: searchdescripton\n\n"
    << mClassDoc->mDescription << "\n\n"
    << ".. include:: Description/Action.rst\n\n"
    << ".. cpp:class:: " << mName;

  if (mBases.Size() > 0)
  {
    mOutput << " : public " << mBases[0];

    for (uint i = 1; i < mBases.Size(); ++i)
    {
      mOutput << ", " << mBases[i];
    }
  }

  mOutput << "\n\n";
}

void ClassMarkupWriter::InsertClassRstFooter(void)
{
  mOutput << ".. include:: Remarks/" << mName << ".rst\n";
}

void ClassMarkupWriter::InsertMethod(MethodDoc &method)
{
  InsertCollapsibleSection();
  StartIndentSection((*this));

  IndentToCurrentLevel();

  mOutput << ".. cpp:function:: " << method.mReturnType << " " 
    << mName << "::" << method.mName << method.mParameters << "\n\n";

  if (!method.mDescription.Empty())
  {
    IndentToCurrentLevel();

    mOutput << "\t" << method.mDescription << "\n\n";
  }

  EndIndentSection((*this));
}

void ClassMarkupWriter::InsertProperty(PropertyDoc &propDoc)
{
  //TODO: we need to figure out why so many things are not properly getting type
  if (propDoc.mType.Empty())
    return;

  InsertCollapsibleSection();
  StartIndentSection((*this));

  IndentToCurrentLevel();

  mOutput << ".. cpp:member:: " <<  propDoc.mType << " " << mName << "::" << propDoc.mName << "\n\n";

  if (!propDoc.mDescription.Empty())
  {
    IndentToCurrentLevel();

    mOutput << "\t" << propDoc.mDescription << "\n\n";
  }

  EndIndentSection((*this));
}

////////////////////////////////////////////////////////////////////////
// EventListWriter
////////////////////////////////////////////////////////////////////////
void EventListWriter::WriteEventList(StringRef eventListFilepath, StringRef outputPath)
{
  // load the file if we can

  // get outta here with that nonexistent file
  if (!FileExists(eventListFilepath))
  {
    printf("%s does not exist.", eventListFilepath.c_str());
    return;
  }

  // actually load event list now. (If this fails it probably means the file is mis-formatted)
  EventDocList eventListDoc;
  LoadFromDataFile(eventListDoc, eventListFilepath);

  Array<EventDoc *> &eventArray = eventListDoc.mEvents;

  // create the eventList
  EventListWriter writer("Event List");

  writer.InsertNewSectionHeader("Event List");

  // create top of file
  StartHeaderSection(writer);

  // iterate over all events and insert them
  forRange(EventDoc *eventDoc, eventArray.All())
  {
    writer.WriteEventEntry(eventDoc->mName, eventDoc->mType);
  }

  EndHeaderSection(writer);

  writer.WriteOutputToFile(outputPath);
}

EventListWriter::EventListWriter(StringParam name) : BaseMarkupWriter(name)
{
}

/*
// The tilde are there so we create a shortened link to the class DO NOT REMOVE
classMarkup << "\n^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n"
<< "Properties From: :cpp:type:`" << IterClass->mName
<< "`\n^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n";
*/

void EventListWriter::WriteEventEntry(StringParam eventEntry, StringParam type)
{
  InsertNewSectionHeader(eventEntry);

  // #ticks matter around links
  mOutput << ".. include:: EventListDescriptions/" << eventEntry << ".rst\n\n"
    << ":cpp:type:`" << type <<"`\n\n";
  //
}

////////////////////////////////////////////////////////////////////////
// CommandRefWriter
////////////////////////////////////////////////////////////////////////
void CommandRefWriter::WriteCommandRef(StringParam commandListFilepath, StringRef outputPath)
{
  // load the file
  // get outta here with that nonexistent file
  if (!FileExists(commandListFilepath))
  {
    printf("%s does not exist.", commandListFilepath.c_str());
    return;
  }

  // actually load event list now. (If this fails it probably means the file is mis-formatted)
  CommandDocList cmdListDoc;
  LoadFromDataFile(cmdListDoc, commandListFilepath);

  Array<CommandDoc *> &cmdArray = cmdListDoc.mCommands;

  // create the command writer
  CommandRefWriter writer("Zero Commands");

  // create top of file
  writer.InsertNewSectionHeader("Zero Commands");

  StartHeaderSection(writer);

  // iterate over all commands and insert them
  forRange(CommandDoc *cmdDoc, cmdArray.All())
  {
    writer.WriteCommandEntry(*cmdDoc);
  }

  EndHeaderSection(writer);

  writer.WriteOutputToFile(outputPath);
}

CommandRefWriter::CommandRefWriter(StringParam name) : BaseMarkupWriter(name)
{
}

void CommandRefWriter::WriteCommandEntry(const CommandDoc &cmdDoc)
{
  InsertNewSectionHeader(cmdDoc.mName);

  if (!cmdDoc.mDescription.Empty())
    mOutput << cmdDoc.mDescription << "\n\n";

  StartHeaderSection((*this));
  InsertNewSectionHeader("Tags");

  if (cmdDoc.mTags.Empty())
  {
    mOutput << "**No Tags**\n\n";
  }
  else
  {
    // actually list tags
    forRange(StringRef tag, cmdDoc.mTags.All())
    {
      mOutput << "*\t" << tag << "\n\n";
    }
  }

  InsertNewSectionHeader("Shortcut");

  if (cmdDoc.mShortcut.Empty())
  {
    mOutput << "**No Keyboard Shortcut**\n\n";
  }
  else
  {
    mOutput << "*\t" << cmdDoc.mShortcut << "\n\n";
  }

  mOutput << ".. include:: CommandPageExtensions/" << cmdDoc.mName << ".rst\n\n";

  EndHeaderSection((*this));
}



}
