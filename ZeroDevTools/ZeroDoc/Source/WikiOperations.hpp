#pragma once

#include "String\String.hpp"
#include "Engine\Documentation.hpp"
#include "Parsing.hpp"

namespace Zero
{

void PushToWiki(StringMap& params);

void LogOn(String& token, bool verbose);
void LogOff(StringParam token, bool verbose);
void GetWikiArticleIds(StringParam token, StringMap& wikiIndices, bool verbose);
bool CreateWikiPage(StringParam token, StringParam pageName, StringParam parentPageIndex, 
                    StringMap& wikiIndices, bool verbose);
void UploadClassDoc(StringParam index, ClassDoc& classDoc, Replacements& replacements, 
                    StringParam token, bool verbose);
void UploadPageContent(StringParam pageIndex, StringParam pageTitle, 
                       StringParam pageContent, StringParam token, bool verbose);

}//namespace Zero
