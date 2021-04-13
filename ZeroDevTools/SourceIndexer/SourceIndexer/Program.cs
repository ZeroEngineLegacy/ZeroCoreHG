using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SourceIndexer
{
  // Simple structure that stores the full path and relative path of a file.
  class FileInfo
  {
    public String FullPath;
    public String RelativePath;
  }

  class DepoInfo
  {
    public String DepoPath;
    public List<FileInfo> Files = new List<FileInfo>();
  }

  class SourceIndexer
  {
    // The pdb contains a list of what files it uses. This can be extracted using srctool.exe.
    // Note: this list contains files we don't care about (such as new and malloc).
    static String[] GetFileListing(String pdbLocation, String srcsrvLocation)
    {
      ProcessStartInfo info = new ProcessStartInfo();
      info.Arguments = pdbLocation + " -r";
      info.WorkingDirectory = srcsrvLocation;
      info.FileName = Path.Combine(srcsrvLocation, "srctool.exe");
      info.RedirectStandardOutput = true;
      info.UseShellExecute = false;
      Process process = Process.Start(info);
      String files = process.StandardOutput.ReadToEnd();
      process.WaitForExit();

      return files.Split('\n');
    }

    static void PruneFileListIntoDepos(String[] files, List<DepoInfo> depos)
    {
      depos.Sort(delegate(DepoInfo depo0, DepoInfo depo1) { return -depo0.DepoPath.Length.CompareTo(depo1.DepoPath.Length); });

      foreach(var file in files)
      {
        var toLoweredFile = file.ToLower();

        foreach(var depo in depos)
        {
          var toLoweredDepo = depo.DepoPath.ToLower();

          if(toLoweredFile.Contains(toLoweredDepo))
          {
            // Make sure to trim out any newlines or other characters at the end
            String fileName = file.Trim();
            // Get the exact name so that mercurial can look it up
            String caseSensitiveFileName = GetExactPathName(fileName);
            // Chop of the beginning of the file path that matches the depo path.
            String relativeFile = caseSensitiveFileName.Substring(depo.DepoPath.Length);
            relativeFile = relativeFile.TrimStart('\\');

            FileInfo info = new FileInfo();
            info.FullPath = caseSensitiveFileName;
            info.RelativePath = relativeFile;
            depo.Files.Add(info);
            break;
          }
        }
      }
    }

    // Unfortunately, the pdb changes the case of the path. Mercurial is case
    // sensitive with some commands so this poses problems. Luckily windows can
    // find the file case-insenstive and then reconstruct the original path.
    public static string GetExactPathName(string pathName)
    {
      if (!(File.Exists(pathName) || Directory.Exists(pathName)))
        return pathName;

      var di = new DirectoryInfo(pathName);

      if (di.Parent != null)
      {
        return Path.Combine(
            GetExactPathName(di.Parent.FullName),
            di.Parent.GetFileSystemInfos(di.Name)[0].Name);
      }
      else
      {
        return di.Name.ToUpper();
      }
    }

    // Simple helper to turn command line arguments into a dictionary lookup.
    static Dictionary<String, List<String> > ParseCommandLineArgs(string[] args)
    {
      Dictionary<String, List<String> > parsedArgs = new Dictionary<String, List<String> >();

      int i = 0;
      while(i < args.Length)
      {
        // If this argument starts with a '-' then check the next argument.
        // If the next argument doesn't have a '-' then it's the value of this argument.
        // If it does have a '-' then assume this is a flag is a boolean being set to true.
        String flag = args[i];
        if (flag.StartsWith("-"))
        {
          int nextIndex = i + 1;
          String val = "true";
          if (nextIndex < args.Length && !args[nextIndex].StartsWith("-"))
            val = args[nextIndex];

          // To support multiple depos this was turned into a multi-map. Unfortunately this
          // means other parameters can get more than one key, but oh well...
          if (parsedArgs.ContainsKey(flag) == false)
            parsedArgs.Add(flag, new List<String>());
          parsedArgs[flag].Add(val);
          i += 2;
        }
        else
          ++i;
      }

      return parsedArgs;
    }

    // Test function to write each file into the pdb. Unfortunately there's no way to extract the files back out.
    static void WriteEachFileToDataStream(List<FileInfo> relativeFiles, String psbStrLocation, String pdbLocation)
    {
      for (int i = 0; i < relativeFiles.Count; ++i)
      {
        String relativePath = relativeFiles[i].RelativePath;
        String fullPath = relativeFiles[i].FullPath;


        ProcessStartInfo info = new ProcessStartInfo();
        info.Arguments = String.Format("-w -p:\"{0}\" -s:{1} -i:\"{2}\"", pdbLocation, relativePath, fullPath);
        info.FileName = psbStrLocation;
        info.RedirectStandardOutput = true;
        info.RedirectStandardError = true;
        info.UseShellExecute = false;
        Process process = Process.Start(info);
        String fileRevision = process.StandardOutput.ReadToEnd();
        String errs = process.StandardError.ReadToEnd();
        process.WaitForExit();
      }
    }

    static void Main(string[] args)
    {
      Dictionary<String, List<String> > parsedArgs = ParseCommandLineArgs(args);

      List<DepoInfo> depos = new List<DepoInfo>();
      String pdbLocation = "ZeroEditor.pdb";
      String srcSrvLocation = "srcsrv";
      String mode = "hg";

      // Should we not print diagnostic messages?
      bool isSilent = false;

      // Parse the arguments we need
      String outLocation = "";
      if (parsedArgs.ContainsKey("-depo"))
      {
        List<String> depoParams = parsedArgs["-depo"];
        for(int i = 0; i < depoParams.Count; ++i)
        {
          DepoInfo depoInfo = new DepoInfo();
          // Some of the srcsrv tools don't work with relative paths.
          // Convert all of our paths to full paths.
          depoInfo.DepoPath = Path.GetFullPath(depoParams[i]);
          depos.Add(depoInfo);
        }
      }

      // Add a default path primarily for testing
      if(depos.Count == 0)
      {
        DepoInfo depoInfo = new DepoInfo();
        depoInfo.DepoPath = @"C:\BuildBot\slave\Zero\ZeroCore";
        depos.Add(depoInfo);
        depoInfo = new DepoInfo();
        depoInfo.DepoPath = @"C:\BuildBot\slave\Zero\ZeroCore\Zilch";
        depos.Add(depoInfo);
      }
      
      if (parsedArgs.ContainsKey("-pdb"))
        pdbLocation = parsedArgs["-pdb"][0];
      if (parsedArgs.ContainsKey("-srcsrv"))
        srcSrvLocation = parsedArgs["-srcsrv"][0];
      if (parsedArgs.ContainsKey("-mode"))
        mode = parsedArgs["-mode"][0];
      if (parsedArgs.ContainsKey("-silent"))
        isSilent = true;

      // If the user specifies where to output the text data we need to
      // save then use that path. Otherwise put it next to the pdb.
      if (parsedArgs.ContainsKey("-out"))
        outLocation = parsedArgs["-out"][0];
      else
      {
        String fullPath = Path.GetDirectoryName(pdbLocation);
        outLocation = Path.Combine(fullPath, "PdbData.txt");
      }
      
      pdbLocation = Path.GetFullPath(pdbLocation);
      srcSrvLocation = Path.GetFullPath(srcSrvLocation);
      outLocation = Path.GetFullPath(outLocation);

      // Get the path to the tool used to modify the pdb.
      String pdbStrLocation = Path.Combine(srcSrvLocation, "pdbstr.exe");

      if (isSilent == false)
      {
        // For debugging: output what the directory of each path we need is.
        Console.Write("DepoPath: ");
        foreach (var depo in depos)
        {
          Console.Write("{0}; ", depo.DepoPath);
        }
        Console.WriteLine();
        
        Console.WriteLine("PdbPath: {0}", pdbLocation);
        Console.WriteLine("SrcSrvLocation: {0}", srcSrvLocation);
        Console.WriteLine("OutLocation: {0}", outLocation);
      }

      String[] allFiles = GetFileListing(pdbLocation, srcSrvLocation);
      if (isSilent == false)
        Console.WriteLine("Found {0} files in the pdb", allFiles.Length);

      // Get a list of what files belong to which depo
      PruneFileListIntoDepos(allFiles, depos);

      String pdbData = "";
      if (mode == "hg")
        pdbData = MercurialIndexer.BuildSourceIndex(depos, pdbLocation, pdbStrLocation, isSilent);

      File.WriteAllText(outLocation, pdbData);


      // Write the text file into the pdb
      ProcessStartInfo info = new ProcessStartInfo();
      info.Arguments = String.Format("-w -p:\"{0}\" -s:srcsrv -i:\"{1}\"", pdbLocation, outLocation);
      info.FileName = pdbStrLocation;
      info.RedirectStandardOutput = true;
      info.RedirectStandardError = true;
      info.UseShellExecute = false;
      Process process = Process.Start(info);
      String fileRevision = process.StandardOutput.ReadToEnd();
      String errs = process.StandardError.ReadToEnd();
      process.WaitForExit();
    }
  }
}
