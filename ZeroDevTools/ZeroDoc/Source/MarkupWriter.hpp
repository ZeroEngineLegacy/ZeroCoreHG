#include "Engine/EngineContainers.hpp"
namespace Zero
{
  typedef HashMap<String, Array<ClassDoc*> > DocToTags;
  typedef DocToTags::range DocRange;

  void WriteOutAllMarkdownFiles(Zero::DocGeneratorConfig& config);

  class BaseMarkupWriter
  {
  public:
    BaseMarkupWriter(StringParam name) 
      : mName(name)
      , mCurrentIndentationLevel(0)
      , mCurrentSectionHeaderLevel(0){}

    void WriteOutputToFile(StringRef outputDirectory);

  protected:
    void IndentToCurrentLevel(void);

    void InsertNewUnderline(uint length, uint headerLevel = 0);

    void InsertNewSectionHeader(StringRef sectionName);

    void InsertCollapsibleSection(void);

    StringBuilder mOutput;

    String mName;

    // can be moved up in the case of large sections that need to be indented
    uint mCurrentIndentationLevel;

    uint mCurrentSectionHeaderLevel;
  };

  class ClassMarkupWriter : public BaseMarkupWriter
  {
  public:
    static void WriteClass(
      StringParam outputFile,
      ClassDoc* classDoc,
      DocumentationLibrary &lib,
      DocToTags& tagged);

    ClassMarkupWriter(StringParam name, ClassDoc* classDoc);

  protected:
    void InsertClassRstHeader(void);

    void InsertClassRstFooter(void);

    void InsertMethod(MethodDoc &method);

    void InsertProperty(PropertyDoc &propDoc);

    Array<String> mBases;

    ClassDoc *mClassDoc;
  };

  class EventListWriter : public BaseMarkupWriter
  {
  public:
    static void WriteEventList(StringRef eventListFilepath, StringRef outputPath);

    EventListWriter(StringParam name);

    void WriteEventEntry(StringParam eventEntry, StringParam type);
  };

  class CommandRefWriter : public BaseMarkupWriter
  {
  public:
    static void WriteCommandRef(StringParam commandListFilepath, StringRef outputPath);

    CommandRefWriter(StringParam name);

    void WriteCommandEntry(const CommandDoc &cmdDoc);
  };
}