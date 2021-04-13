#include "Precompiled.hpp"
//
//#include "WikiOperations.hpp"
//
//#define CURL_STATICLIB
//
//#include <curl/curl.h>
//#include "Common/String/CharacterTraits.hpp"
//#include "Support/StringMap.hpp"
//#include "Engine/EngineContainers.hpp"
//#include "Platform/FilePath.hpp"
//#include "Serialization/Text.hpp"
//#include "../TinyXml/tinyxml.h"
//#include "Configuration.hpp"
//#include "Pages.hpp"
//#include "Platform/FileSystem.hpp"
//#include "Serialization/Simple.hpp"
//#include "Logging.hpp"
//
//#pragma comment(lib, "ws2_32.lib") 
//#pragma comment(lib, "Wldap32.lib") 
//
//namespace Zero
//{
//
////------------------------------------------------------------------- Write Data
//struct WriteData
//{
//  virtual void Start() {};
//  virtual void End() {};
//
//  virtual void Parse(const char* data, uint sizeOfChar, uint numOfElements)
//  {
//    if(mVerbose)
//      printf("%s",data);
//  }
//
//  bool mVerbose;
//};
//
////---------------------------------------------------------------- Buffered Data
//struct BufferedData : public WriteData
//{
//  void Start() override{};
//  void End() override{};
//
//  void Parse(const char* data, uint sizeOfChar, uint numOfElements) override
//  {
//    WriteData::Parse(data,sizeOfChar,numOfElements);
//
//    String str = String(data,numOfElements * sizeOfChar);
//    mBuilder.Append(str);
//  }
//
//  Zero::StringBuilder mBuilder;
//};
//
////----------------------------------------------------------------- Token Parser
//struct TokenParser : public WriteData
//{
//  void Start() override
//  {
//    mValid = false;
//  };
//
//  void End() override {};
//
//  void Parse(const char* data, uint sizeOfChar, uint numOfElements) override
//  {
//    WriteData::Parse(data,sizeOfChar,numOfElements);
//
//    Zero::String str = Zero::String::Format("%s",data);
//    if(str.Size() < 8)
//      return;
//
//    if(str.SubStringFromByteIndices(0,7) == Zero::String("<token>"))
//    {
//      mToken = str.SubStringFromByteIndices(16,30);
//      mValid = true;
//    }
//  }
//
//  Zero::String mToken;
//  bool mValid;
//};
//
//static size_t write_data(void *ptr, size_t sizeOfChar, size_t numOfElements, void *stream)
//{
//  //int written = fwrite(ptr, size, nmemb, (FILE *)stream);
//  //return written;
//  const char* data = (const char*)ptr;
//  if(stream != NULL)
//  {
//    WriteData* writeData = static_cast<WriteData*>(stream);
//    writeData->Parse(data,sizeOfChar,numOfElements);
//  }
//
//  return sizeOfChar*numOfElements;
//}
//
//void LogOn(String& token, bool verbose)
//{
//  CURL *curl;
//  CURLcode res;
//
//  curl_global_init(CURL_GLOBAL_DEFAULT);
//  curl = curl_easy_init();
//
//  TokenParser tokenParser;
//  tokenParser.mVerbose = verbose;
//
//  String loginUrl = "http://zeroengine0.digipen.edu/api.asp?"
//                    "cmd=logon&email=LevelDesigner2D&password=letmein";
//  if(verbose)
//    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
//
//  curl_easy_setopt(curl, CURLOPT_URL,loginUrl.c_str());
//  curl_easy_setopt(curl, CURLOPT_WRITEDATA,&tokenParser);
//
//  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
//  res = curl_easy_perform(curl);
//
//  if(res != CURLE_OK)
//    printf("Failed to log in.\n");
//
//  if(tokenParser.mValid)
//    token = tokenParser.mToken;
//  else
//    printf("Failed to parse a valid token after logging in.\n");
//
//  /* always cleanup */ 
//  curl_easy_cleanup(curl);
//}
//
//void LogOff(StringParam token, bool verbose)
//{
//  CURL *curl;
//  CURLcode res;
//
//  curl_global_init(CURL_GLOBAL_DEFAULT);
//
//  curl = curl_easy_init();
//
//  String logoffUrl = Zero::BuildString("http://zeroengine0.digipen.edu/api.asp?"
//                                       "cmd=logoff&token=", token.c_str());
//
//  WriteData parser;
//  parser.mVerbose = verbose;
//
//  if(verbose)
//    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
//
//  curl_easy_setopt(curl, CURLOPT_URL,logoffUrl.c_str());
//  curl_easy_setopt(curl, CURLOPT_WRITEDATA,&parser);
//
//  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
//  res = curl_easy_perform(curl);
//
//  if(res != CURLE_OK)
//    printf("Failed to log off.\n");
//
//  /* always cleanup */ 
//  curl_easy_cleanup(curl);
//}
//
//void GetWikiArticleIds(StringParam token, StringMap& wikiIndices, bool verbose)
//{
//  CURL *curl;
//  CURLcode res;
//
//  curl_global_init(CURL_GLOBAL_DEFAULT);
//
//  curl = curl_easy_init();
//
//  String url = String::Format("http://zeroengine0.digipen.edu/api.asp?"
//                              "cmd=listArticles&ixWiki=1&token=%s", 
//                              token.c_str());
//
//  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
//  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
//  if(verbose)
//    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
//
//  BufferedData parser;
//  parser.mVerbose = false;
//  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &parser);
//
//  res = curl_easy_perform(curl);
//
//  String str = parser.mBuilder.ToString();
//
//  TiXmlDocument doc;
//  doc.Parse(str.c_str());
//
//  TiXmlElement* response = doc.FirstChildElement("response");
//  TiXmlElement* articles = response->FirstChildElement("articles");
//
//  TiXmlNode* article;
//  for(article = articles->FirstChild(); article != 0; article = article->NextSibling())
//  {
//    TiXmlNode* wikiIdTag = article->FirstChildElement("ixWikiPage");
//    TiXmlText* wikiIdData = (TiXmlText*)wikiIdTag->FirstChild();
//
//    TiXmlNode* headlineTag = wikiIdTag->NextSiblingElement("sHeadline");
//    TiXmlText* headlineData = (TiXmlText*)headlineTag->FirstChild();
//
//    String id = wikiIdData->Value();
//    String pageName = headlineData->Value();
//
//    wikiIndices[pageName] = id;
//  }
//
//  /* always cleanup */ 
//  curl_easy_cleanup(curl);
//}
//
//bool CreateWikiPage(StringParam token, StringParam pageName, 
//                    StringParam parentPageIndex, StringMap& wikiIndices, bool verbose)
//{
//  CURL *curl;
//  CURLcode res;
//
//  curl_global_init(CURL_GLOBAL_DEFAULT);
//
//  curl = curl_easy_init();
//
//  String url = String::Format("http://zeroengine0.digipen.edu/api.asp?"
//                              "cmd=newArticle&ixWiki=1&sHeadline=%s&token=%s", 
//                              pageName.c_str(), token.c_str());
//
//  curl_easy_setopt(curl, CURLOPT_URL,url.c_str());
//  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
//  if(verbose)
//    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
//
//  BufferedData parser;
//  parser.mVerbose = false;
//  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &parser);
//
//  res = curl_easy_perform(curl);
//
//  String str = parser.mBuilder.ToString();
//
//  TiXmlDocument doc;
//  doc.Parse(str.c_str());
//
//  TiXmlElement* response = doc.FirstChildElement("response");
//  if(response == NULL)
//  {
//    printf("Failed to find response tag for page %s\n",pageName.c_str());
//    return false;
//  }
//
//  TiXmlElement* wikipage = response->FirstChildElement("wikipage");
//  if(wikipage == NULL)
//  {
//    printf("Failed to find wikipage tag for page %s\n",pageName.c_str());
//    return false;
//  }
//  TiXmlElement* wikiPageIndex = wikipage->FirstChildElement("ixWikiPage");
//  if(wikiPageIndex == NULL)
//  {
//    printf("Failed to find ixWikiPage tag for page %s\n",pageName.c_str());
//    return false;
//  }
//
//  TiXmlText* wikiIdData = (TiXmlText*)wikiPageIndex->FirstChild();
//  if(wikiIdData == NULL)
//  {
//    printf("Failed to find wiki page id for new page %s\n",pageName.c_str());
//    return false;
//  }
//
//  String id = wikiIdData->Value();
//  wikiIndices[pageName] = id;
//
//  /* always cleanup */ 
//  curl_easy_cleanup(curl);
//
//  return true;
//}
//
//void UploadClassDoc(StringParam index, ClassDoc& classDoc, Replacements& replacements, 
//                    StringParam token, bool verbose)
//{
//  CURL *curl;
//  CURLcode res;
//
//  curl_global_init(CURL_GLOBAL_DEFAULT);
//
//  curl = curl_easy_init();
//
//  String url = String::Format("http://zeroengine0.digipen.edu/api.asp?"
//                              "cmd=editArticle&ixWikiPage=%s&token=%s", 
//                              index.c_str(), token.c_str());
//
//  curl_easy_setopt(curl, CURLOPT_URL,url.c_str());
//
//  curl_httppost* post = NULL;
//  curl_httppost* last = NULL;
//
//  String doc = BuildDoc(classDoc, replacements);
//
//  curl_formadd(&post, &last, CURLFORM_COPYNAME, "sHeadline", 
//               CURLFORM_COPYCONTENTS, classDoc.Name.c_str(), CURLFORM_END);
//
//  curl_formadd(&post, &last, CURLFORM_COPYNAME, "sBody", 
//               CURLFORM_PTRCONTENTS, doc.c_str(),
//               CURLFORM_CONTENTSLENGTH, doc.Size(),
//               CURLFORM_CONTENTTYPE, "text/html", CURLFORM_END);
//
//  /* Set the form info */
//  curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);
//
//  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
//
//  WriteData parser;
//  parser.mVerbose = verbose;
//  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &parser);
//
//  if(verbose)
//    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
//
//  res = curl_easy_perform(curl);
//
//  if(res != CURLE_OK)
//    printf("Failed to upload class doc %s.\n",classDoc.Name.c_str());
//
//  /* always cleanup */ 
//  curl_easy_cleanup(curl);
//}
//
//void UploadPageContent(StringParam pageIndex, StringParam pageTitle, 
//                       StringParam pageContent, StringParam token, bool verbose)
//{
//  CURL *curl;
//  CURLcode res;
//
//  curl_global_init(CURL_GLOBAL_DEFAULT);
//
//  curl = curl_easy_init();
//
//  String url = String::Format("http://zeroengine0.digipen.edu/api.asp?"
//                              "cmd=editArticle&ixWikiPage=%s&token=%s", 
//                              pageIndex.c_str(), token.c_str());
//
//  curl_easy_setopt(curl, CURLOPT_URL,url.c_str());
//
//  curl_httppost* post = NULL;
//  curl_httppost* last = NULL;
//
//  //String doc = BuildDoc(classDoc, replacements);
//
//
//  curl_formadd(&post, &last, CURLFORM_COPYNAME, "sHeadline", 
//               CURLFORM_COPYCONTENTS, pageTitle.c_str(), CURLFORM_END);
//
//  curl_formadd(&post, &last, CURLFORM_COPYNAME, "sBody", 
//               CURLFORM_PTRCONTENTS, pageContent.c_str(),
//               CURLFORM_CONTENTSLENGTH, pageContent.Size(),
//               CURLFORM_CONTENTTYPE, "text/html", CURLFORM_END);
//
//  /* Set the form info */
//  curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);
//
//  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
//
//  WriteData parser;
//  parser.mVerbose = verbose;
//  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &parser);
//
//  if(verbose)
//    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
//
//  res = curl_easy_perform(curl);
//
//  if(res != CURLE_OK)
//    printf("Failed to upload class doc %s.\n", pageTitle.c_str());
//
//  /* always cleanup */ 
//  curl_easy_cleanup(curl);
//}
//
//
//
//void UpdateEventList(StringMap& params, Replacements& replacements, 
//  StringBuilder* outputString)
//{
//  Configurations config = LoadConfigurations(params);
//
//  Array<EventEntry> eventsToUpdate;
//  TextLoader stream;
//  String eventsToUpdatePath = config.EventsFile;
//  if(!FileExists(eventsToUpdatePath.c_str()))
//  {
//    printf("%s does not exist.", eventsToUpdatePath.c_str());
//    return;
//  }
//  Status status;
//  stream.Open(status, eventsToUpdatePath.c_str());
//  SerializeName(eventsToUpdate);
//  stream.Close();
//
//  //Create the HTML for page that hosts the list of events.
//  String eventHeader("");
//  StringBuilder eventListHtml;
//  outputString->Append("<p>");
//  forRange(Array<EventEntry>::value_type& e, eventsToUpdate.All())
//  {
//    if(eventHeader != e.mEventType)
//    {
//      if(eventHeader != "") outputString->Append("</ul>");
//      eventHeader = e.mEventType;
//      outputString->Append("<h3>");
//      Replace(*outputString, replacements, e.mEventType);
//      outputString->Append("</h3>");
//      outputString->Append("<ul>");
//    }
//    outputString->Append("<li>");
//    Replace(*outputString, replacements, e.mEventName);
//    outputString->Append("</li>");
//
//  }
//  eventListHtml.Append("</p>");
//
//
//}
//
//void PushToWiki(StringMap& params)
//{
//  Configurations config = LoadConfigurations(params);
//
//  //There's a set list of classes we want to push to the wiki, load this list from a data file
//  Array<WikiUpdatePage> pagesToUpdate;
//  TextLoader stream;
//  String pagesToUpdatePath = BuildString(config.SourcePath.c_str(), 
//    "DevTools\\PagesToUpdate.txt");
//  if(!FileExists(pagesToUpdatePath))
//  {
//    printf("%s does not exist.",pagesToUpdatePath.c_str());
//    return;
//  }
//
//  Status status;
//  stream.Open(status, pagesToUpdatePath.c_str());
//  SerializeName(pagesToUpdate);
//  stream.Close();
//
//  //Now load the documentation file (the documentation for all the classes)
//  if(!FileExists(config.DocumentationFile.c_str()))
//  {
//    printf("%s does not exist.",config.DocumentationFile.c_str());
//    return;
//  }
//
//  DocumentationLibrary doc;
//  LoadFromDataFile(doc, config.DocumentationFile);
//  doc.Build();
//
//  //Warn for classes that have documentation but are not parked to push to the wiki
//  WarnNeedsWikiPage(pagesToUpdate, doc.Classes, config.mDoxygenPath, 
//    config.DocumentationRoot, config.Verbose, config.Log);
//
//  //Log onto the wiki and get our token to use for further operations
//  String token;
//  LogOn(token,config.Verbose);
//
//  //Extract the wiki article names and their indices from the wiki
//  StringMap wikiIndices;
//  GetWikiArticleIds(token, wikiIndices, config.Verbose);
//
//  //Create a map of the wiki names and ids for only what we want to push to the wiki.
//  typedef StringMap WikiMap;
//  WikiMap wikiIndex;
//  for(uint i = 0; i < pagesToUpdate.Size(); ++i)
//  {
//    String pageName = pagesToUpdate[i].mPageToUpdate;
//    String parentPageName = pagesToUpdate[i].mParentPage;
//    String index = wikiIndices.FindValue(pageName,"");
//    if(!index.Empty())
//      wikiIndex[pageName] = index;
//    else
//    {
//      String parentIndex = wikiIndices.FindValue(parentPageName,"");
//      if(!parentIndex.Empty())
//      {
//        bool success = CreateWikiPage(token,pageName,parentIndex,wikiIndex,config.Verbose);
//        if(!success)
//          printf("Failed to find wiki article for \"%s\" with parent \"%s\"\n",
//          pageName.c_str(), parentPageName.c_str());
//        else
//          printf("Creating wiki page for article \"%s\"\n",pageName.c_str());
//      }
//      else
//        printf("Failed to find wiki parent article \"%s\" for child page \"%s\"\n",
//        parentPageName.c_str(), pageName.c_str());
//    }
//  }
//
//  //set up a replacement for class names to replace in the wiki, this will set
//  //up links on the page to other classes when they are referenced
//  Array<Replacement> classReplacements;
//  WikiMap::range r = wikiIndex.All();
//  forRange(WikiMap::value_type& v, r)
//  {
//    String name = BuildString(v.first, "");
//    String link = String::Format("<a class=\"uvb\" href=\"default.asp?W%s\">%s</a>", 
//      v.second.c_str(), v.first.c_str());
//    classReplacements.PushBack(Replacement(name, link));
//  }
//  Sort(classReplacements.All());
//
//  //Update all of the events on the event page, link relevant pages to them.
//  StringBuilder eventList;
//  UpdateEventList(params, classReplacements, &eventList);
//  const String eventListPageTitle("Event List");
//  const String eventListPage = wikiIndices[eventListPageTitle.c_str()];
//  UploadPageContent(eventListPage, eventListPageTitle, eventList.ToString(), 
//    token, config.Verbose);
//
//  //Upload the class' page to the wiki, making sure to perform the link replacements
//  forRange(ClassDoc& classDoc, doc.Classes.All())
//  {
//    String index = wikiIndex.FindValue(classDoc.Name, "");
//    if(!index.Empty())
//    {
//      //should uncomment when testing (so we don't accidentally destroy the whole wiki)
//      //if(classDoc.Name == "PhysicsEffect")
//      UploadClassDoc(index, classDoc, classReplacements, token, config.Verbose);
//    }
//    else
//    {
//      //fprintf(f,"need page %s\r\n", classDoc.Name.c_str() );
//      //printf("need page %s\n", classDoc.Name.c_str());
//    }
//  }
//
//  LogOff(token,config.Verbose);
//}
//
//}//namespace Zero
//
