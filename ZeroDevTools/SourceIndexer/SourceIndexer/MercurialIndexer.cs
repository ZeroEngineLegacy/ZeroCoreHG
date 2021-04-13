using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SourceIndexer
{
  class MercurialIndexer
  {
    // Get the revision id of the depo.
    // This is used to know what revision we want to revert each file back to.
    static String GetCurrentRevision(String depoPath)
    {
      ProcessStartInfo info = new ProcessStartInfo();
      info.Arguments = "--cwd " + depoPath + " log -q -r \"max(parents())\" --template {node}";
      info.FileName = "hg.exe";
      info.RedirectStandardOutput = true;
      info.UseShellExecute = false;
      Process process = Process.Start(info);
      String currentRevision = process.StandardOutput.ReadToEnd();
      process.WaitForExit();

      return currentRevision;
    }

    public static String BuildSourceIndex(List<DepoInfo> depos, String pdbPath, String pdbStrPath, bool isSilent)
    {
      // Get the current date and time to put into the pdb.
      String dateString;
      DateTime date = DateTime.Now;
      dateString = date.ToString();

      StringBuilder builder = new StringBuilder();
      builder.AppendLine("SRCSRV: ini ------------------------------------------------");
      builder.AppendLine("VERSION=1");
      builder.AppendLine("INDEXVERSION=2");
      builder.AppendLine("VERCTRL=HG");
      builder.AppendLine("DATETIME=" + dateString);
      builder.AppendLine("SRCSRV: variables ------------------------------------------");
      builder.AppendLine(@"SRCLOCATION=%ZERO_SOURCE%");
      // The target location to output the file to is whatever root VS decides
      // then our revision number then the original relative path.
      builder.AppendLine(@"SRCSRVTRG=%targ%\%var2%\%var3%");
      // Grab the file of the desired revision from the local depo at SRCLOCATION
      // and output that to the source target directory location.
      // Unfortunately mercurial is unable to execute commands on remote repositories...
      // Also, mercurial is unable to check out a file from a sub depo using the main depo's path. 
      // Because of this var4 is added for zero to access a sub depo which is located underneath
      // the main depo. Due to how the expansion of %fnvar% works I think this line must be
      // altered at the top here, not on each individual file's line.
      builder.AppendLine("SRCSRVCMD=hg.exe --cwd \"%fnvar%(SRCLOCATION)%var4%\" -y cat %var3% -r %var2% > %SRCSRVTRG%");
      builder.AppendLine("SRCSRV: source files ---------------------------------------");

      // Iterate over each depo and write out their respective files
      foreach(var depo in depos)
      {
        // Get the depo's current revision
        String currentRevision = GetCurrentRevision(depo.DepoPath);
        if (isSilent == false)
          Console.WriteLine("Embedding revision {0}", currentRevision);

        // Quick and dirty hack to detect if this depo is zilch and if so add extra info to the file's paths.
        var extraDepo = "";
        var lastSlash = depo.DepoPath.LastIndexOf('\\');
        var depoName = depo.DepoPath.Substring(lastSlash).Trim('\\');
        if (depoName == "Zilch")
          extraDepo += "\\Zilch";
        else if (depoName == "ExtensionLibraries")
          extraDepo += "\\ExtensionLibraries";
        else if (depoName == "StandardLibraries")
          extraDepo += "\\ExtensionLibraries\\StandardLibraries";
        else if (depoName == "AudioEngine")
          extraDepo += "\\AudioEngine";
        
        // Now output the format string for the file
        foreach(var file in depo.Files)
        {
          // Instead of figuring out where each file is supposed to go with the original depo
          // paths, just put each separate depo in its own folder.
          String relativePath = file.RelativePath;
          builder.AppendLine(String.Format("{0}*{1}*{2}*{3}", file.FullPath, currentRevision, relativePath, extraDepo));
        }
      }
      builder.AppendLine("SRCSRV: end ------------------------------------------------");

      return builder.ToString();
    }
  }
}
