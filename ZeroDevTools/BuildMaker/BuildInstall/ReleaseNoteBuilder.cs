///////////////////////////////////////////////////////////////////////////////
///
/// \file ReleaseNoteBuilder.cs
/// The entirety of the ReleaseNoteBuilder tool.
/// 
/// Authors: Benjamin Strukus
/// Copyright 2010-2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net;
using System.IO;
using System.Xml.Linq;
using System.Xml;

namespace BuildMaker
{
  class ReleaseNoteBuilder
  {
    public void Run(String installerDirectory)
    {
      //Token's used for wiki info, the FogBugzClient login is used to get the
      //cases from the bug database. Could be done with one "login" but it works.
      GetLoginToken();
      GetInstallPage();
      GetFogBugzCases(installerDirectory);
    }

    bool LoginToFogBugz()
    {
      //Use the LevelDesigner2D account.
      FogBugzClient.LogOn("http://zeroengine0.digipen.edu/api.asp",
                          "LevelDesigner2D", "letmein");
      Console.WriteLine("Logged in!");

      return true;
    }

    void GetFogBugzCases(String installerDirectory)
    {
      //Login as LevelDesigner2D to get a list of cases. This account has the 
      //"every case resolved in the last 2 months" filter.
      LoginToFogBugz();

      //Find the ID of the "Release Notes" filter so we can get all the cases
      //that matter to us.
      IEnumerable<Filter> filters = FogBugzClient.GetFilters();
      Filter currentFilter = filters.FirstOrDefault(filter => filter.Name == "Release Notes");
      FogBugzClient.SetFilter(currentFilter.Content);

      //Get all the resolved bugs since the last posted build of the Zero Engine
      //and build the HTML to paste into the wiki article.
      String htmlForCases = "<ul>";
      DateTime lastBuildDate = GetLastBuildDate();
      IEnumerable<Case> cases = FogBugzClient.GetAllCases();
      foreach(var bugCase in cases)
      {
        if (bugCase.DateResolved > lastBuildDate)
        {
          htmlForCases += CaseToHtmlEntry(bugCase);
        }
      }
      htmlForCases += "</ul>";
      System.IO.File.WriteAllText(Path.Combine(installerDirectory, "release_notes.txt"),
                                               htmlForCases);
    }

    String CaseToHtmlEntry(Case bugCase)
    {
      //Builds the following HTML, where "42" is the case number example and
      //"Title" is the case title. Allows for cool listing of the bugs in the wiki page.
      //<li><a href="default.asp?42" onmouseover="b1(42, this);" rel="nofollow">42: Title</a></li>
      String caseNumber = bugCase.Index.ToString();
      String htmlEntry = "<li><a href=\"default.asp?";
      htmlEntry += caseNumber + "\" onmouseover=\"b1(" + caseNumber;
      htmlEntry += ", this);\" rel=\"nofollow\">" + caseNumber + ": ";
      htmlEntry += bugCase.Title + "</a></li>";
      return htmlEntry;
    }

    void GetLoginToken()
    {
      String loginUrl = "http://zeroengine0.digipen.edu/api.asp?cmd=logon&email=LevelDesigner2D&password=letmein";
      XDocument doc = XmlFromUrl(loginUrl);

      //Get the token value from the new document.
      mLoginToken = doc.Root.Element("token").Value;
      Console.WriteLine("Login token is: " + mLoginToken);
    }

    void GetInstallPage()
    {
      const String pageToFind = "Installer";
      String installUrl = FindTheInstallerPage(pageToFind);
      mInstallPage = XmlFromUrl(installUrl);
    }

    DateTime GetLastBuildDate()
    {
      const int cMonthStart = 5;
      const int cDayStart = 8;
      const String cZeroEngineDate = @"ZeroEngineSetup";
      int dateOffset = cZeroEngineDate.Length;

      //Get the date for all the bug comparisons
      String pageBody = mInstallPage.Element("response")
                                    .Element("wikipage")
                                    .Element("sBody")
                                    .Value;

      int date = pageBody.IndexOf(cZeroEngineDate) + dateOffset;
      int year = Convert.ToInt32(pageBody.Substring(date, 4));
      int month = Convert.ToInt32(pageBody.Substring(date + cMonthStart, 2));
      int day = Convert.ToInt32(pageBody.Substring(date + cDayStart, 2));
      return new DateTime(year, month, day);
    }

    String FindTheInsidePage()
    {
      //Get the first wiki page on the fogbugz wiki.
      Console.WriteLine("Get the main wiki page...");
      String wikiUrl = "http://zeroengine0.digipen.edu/api.asp?cmd=listWikis&token=";
      wikiUrl += mLoginToken;
      XDocument mainPage = XmlFromUrl(wikiUrl);

      //Find the "Inside" page as this contains the installer page.
      XElement page = (from p in mainPage.Descendants("wiki")
                       where p.Element("sWiki").Value == "Inside"
                       select p).FirstOrDefault();
      if (page == null)
      {
        Console.WriteLine("\"Inside\" page could not be found.");
        return null;
      }

      //Get the ID of the "Inside" page for further use.
      String insideId = page.Element("ixWiki").Value;

      Console.WriteLine(insideId);

      String insideUrl = "http://zeroengine0.digipen.edu/api.asp?cmd=listArticles&ixWiki=";
      insideUrl += insideId + "&token=" + mLoginToken;
      return insideUrl;
    }

    String FindTheInstallerPage(String pageTitle)
    {
      String insideUrl = FindTheInsidePage();
      XDocument insidePage = XmlFromUrl(insideUrl);

      //Find the "Install" page for further use.
      XElement page = (from p in insidePage.Descendants("article")
                       where p.Element("sHeadline").Value == pageTitle
                       select p).FirstOrDefault();

      String installerId = page.Element("ixWikiPage").Value;

      String installerUrl = "http://zeroengine0.digipen.edu/api.asp?cmd=viewArticle&ixWikiPage=";
      installerUrl += installerId + "&token=" + mLoginToken;

      return installerUrl;
    }

    XDocument XmlFromUrl(String url)
    {
      WebRequest urlPage = WebRequest.Create(url);
      
      //Get the response from the server.
      Console.WriteLine("Get response from server...");
      HttpWebResponse response = (HttpWebResponse)urlPage.GetResponse();

      //Get the stream containing content returned by the server.
      Console.WriteLine("Get stream with server response...");
      Stream dataStream = response.GetResponseStream();

      //Open the stream using a StreamReader for easy access.
      Console.WriteLine("Open the stream...");
      StreamReader reader = new StreamReader(dataStream);

      //Read the content.
      Console.WriteLine("Read the content...");
      String responseFromServer = reader.ReadToEnd();

      //Save off the response from the server into an XML object.
      XDocument xmlPage = XDocument.Parse(responseFromServer);

      reader.Close();
      dataStream.Close();
      response.Close();

      return xmlPage;
    }

    String mLoginToken;
    XDocument mInstallPage;
  }
}
