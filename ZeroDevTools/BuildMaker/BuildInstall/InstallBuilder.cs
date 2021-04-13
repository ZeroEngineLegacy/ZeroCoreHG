///////////////////////////////////////////////////////////////////////////////
///
/// \file InstallBuilder.cs
/// The entirety of the InstallBuilder tool.
/// 
/// Authors: Benjamin Strukus
/// Copyright 2010-2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Diagnostics;
using System.IO;
using System.ComponentModel;

namespace BuildMaker
{
  // Represents a property to output for the build machine/server
  class BuildProperty
  {
    public String mType;
    public String mName;
    public String mValue;

    public BuildProperty() { }

    public BuildProperty(String type, String name, String value)
    {
      mType = type;
      mName = name;
      mValue = value;
    }
  }

  // Represents meta information for the build needed for the build server
  class BuildMeta
  {
    public Dictionary<String, BuildProperty> mProperties = new Dictionary<String, BuildProperty>();

    public BuildMeta()
    {
      // Populate a whole bunch of properties with default values. We only update properties if it already exists here
      mProperties.Add("MajorVersion", new BuildProperty("int", "MajorVersion", "0"));
      mProperties.Add("MinorVersion", new BuildProperty("int", "MinorVersion", "0"));
      mProperties.Add("PatchVersion", new BuildProperty("int", "PatchVersion", "0"));
      mProperties.Add("RevisionId", new BuildProperty("int", "RevisionId", "0"));
      mProperties.Add("ExperimentalBranchName", new BuildProperty("string", "ExperimentalBranchName", ""));
      mProperties.Add("BuildId", new BuildProperty("string", "BuildId", "0.0.0.0"));
      mProperties.Add("ShortChangeSet", new BuildProperty("string", "ShortChangeSet", "0"));
      mProperties.Add("ChangeSet", new BuildProperty("string", "ChangeSet", "\"0\""));
      mProperties.Add("ChangeSetDate", new BuildProperty("string", "ChangeSetDate", "\"0-0-0\""));
      mProperties.Add("Platform", new BuildProperty("string", "Platform", "\"Win32\""));
    }

    public String GetProperty(String name, String defaultValue)
    {
      if (!mProperties.ContainsKey(name))
        return defaultValue;
      return mProperties[name].mValue;
    }

    public String GetProperty(String name)
    {
      return GetProperty(name, "");
    }

    public String StripQuotes(String str)
    {
      return str.Trim('\"');
    }

    public String GetBuildId()
    {
      var str = GetProperty("BuildId");
      return StripQuotes(str);
    }

    public String GetBuildNameWithoutExtension(String rootName)
    {
      return rootName + "." + GetBuildId();
    }

    public void SaveMeta(String filePath)
    {
      var builder = new StringBuilder();
      foreach (var property in mProperties.Values)
      {
        // Skip experimental branch if it's an emtpy string
        if (property.mName == "ExperimentalBranchName" && property.mValue.Length == 0)
          continue;

        String propStr = String.Format("{0} {1} = {2},", property.mType, property.mName, property.mValue);
        builder.AppendLine(propStr);
      }
      File.WriteAllText(filePath, builder.ToString());
    }
  }

  class InstallBuilder
  {
    public static String GetRevisionInfo(String cZeroSource)
    {
      // Build up a string to get as much information from mercurial as
      // possible for old versions (no major/minor/etc... version numbers)
      var builder = new StringBuilder();
      builder.Append("log -q -r \"max(parents())\" ");
      builder.Append("--template \"RevisionId {rev}\\n");
      builder.Append("BuildId \\\"0.0.0.{rev}\\\"\\n");
      builder.Append("ChangeSet \\\"{node}\\\"\\n");
      builder.Append("ShortChangeSet \\\"{node|short}\\\"\\n");
      builder.Append("ChangeSetDate \\\"{date|shortdate}\\\"\"");

      ProcessStartInfo processInfo = new ProcessStartInfo();
      processInfo.FileName = "hg";
      processInfo.Arguments = builder.ToString();
      processInfo.WorkingDirectory = cZeroSource;
      processInfo.RedirectStandardOutput = true;
      processInfo.UseShellExecute = false;
      Process process = Process.Start(processInfo);
      String result = process.StandardOutput.ReadToEnd();
      process.WaitForExit();
      return result;
    }
    public void OutputBuildInfo(String buildFilePath, String buildVersion, String metaFileName, String fileName)
    {
      StringBuilder builder = new StringBuilder();
      builder.Append("{");
      builder.Append(String.Format("\"BuildVersion\": \"{0}\",", buildVersion));
      builder.Append(String.Format("\"InstallerName\": \"{0}\",", fileName));
      builder.Append(String.Format("\"MetaFileName\": \"{0}\"", metaFileName));
      builder.Append("}");

      String fileData = builder.ToString();
      File.WriteAllText(buildFilePath, fileData);
    }

    public BuildMeta GetBuildInfo(String sourcePath, String outputPath, String zeroEditorOutputSuffix, int zeroEngineTimeoutSeconds)
    {
      // Run the engine with the command "WriteBuildInfo" to have it output all build meta information. 
      // Also pass it -newProject to force it to not open a project from the recent project list to help it load faster.
      var tempFile = Path.GetTempFileName();
      ProcessStartInfo startInfo = new ProcessStartInfo();
      
      startInfo.Arguments = String.Format("-WriteBuildInfo {0} -newProject", tempFile);
      startInfo.FileName = Path.Combine(outputPath, zeroEditorOutputSuffix, "ZeroEditor.exe");
      var process = Process.Start(startInfo);

      // Old versions of the engine won't understand this argument, so wait a
      // max amount of time and then kill the process
      process.WaitForExit(zeroEngineTimeoutSeconds * 1000);
      if (!process.HasExited)
      {
        try { process.Kill(); }
        catch (Exception e) { }
      }

      // Try to read all lines from the output file.
      var lines = File.ReadAllLines(tempFile);
      if(lines.Length == 0)
      {
        // If the file doesn't exist then parse information for the revision from mercurial
        // into the a string that has the same format as what zero should output, this way
        // we can run the same update logic as normal.
        string revisionInfo = InstallBuilder.GetRevisionInfo(sourcePath);
        lines = revisionInfo.Split(new string[] { "\n" }, StringSplitOptions.RemoveEmptyEntries);
      }

      BuildMeta meta = new BuildMeta();
      // Load each line as a "property"
      var splitTokens = new string[] { " " };
      foreach (var line in lines)
      {
        // A property should be a name-value pair separated by a space (spaces in strings is not supported for now)
        var entries = line.Split(splitTokens, StringSplitOptions.RemoveEmptyEntries);
        if (entries.Length != 2)
          continue;

        var propertyName = entries[0];
        var propertyValue = entries[1];
        // Update the property if it existed
        if (meta.mProperties.ContainsKey(propertyName))
          meta.mProperties[propertyName].mValue = propertyValue;
      }
      return meta;
    }

    public String Run(String sourcePath, String outputPath, String installerPrefix, String zeroEditorOutputSuffix, int zeroEngineTimeoutSeconds)
    {
      // Get the build's meta information from the current exe
      var buildMeta = this.GetBuildInfo(sourcePath, outputPath, zeroEditorOutputSuffix, zeroEngineTimeoutSeconds);
      
      String cBuildOutput = Path.Combine(sourcePath, "Build");
      String buildId = buildMeta.GetBuildId();
      String newFileNameWithoutExtension = buildMeta.GetBuildNameWithoutExtension(installerPrefix);
      String newFileName = newFileNameWithoutExtension + ".exe";

      //Print a nice message signifying the start of the install build.
      Console.WriteLine("Building the latest Zero Engine installer...");

      //Generate the Zero Engine installer using Inno Setup.
      ProcessStartInfo innoSetupInfo = new ProcessStartInfo();
      StringBuilder definesBuilder = new StringBuilder();
      definesBuilder.Append(" /DMyAppVersion=\"{0}\"");
      definesBuilder.Append(" /DZeroEditorOutputSuffix=\"{1}\"");
      definesBuilder.Append(" /DZeroSource=\"{2}\"");
      definesBuilder.Append(" /DZeroOutput=\"{3}\"");
      String defines = String.Format(definesBuilder.ToString(), buildId, zeroEditorOutputSuffix, sourcePath, outputPath);
      innoSetupInfo.Arguments = Path.Combine(cBuildOutput, "ZeroEngineInstall.iss") + defines;
      innoSetupInfo.FileName = @"C:\Program Files (x86)\Inno Setup 5\iscc.exe";
      
      Process innoSetup;
      try
      {
        innoSetup = Process.Start(innoSetupInfo);
      }
      catch (Win32Exception /*win32*/)
      {
        Console.WriteLine("Inno Setup not found. Please install it and rerun " +
                          "the build install maker.");
        Console.WriteLine("Press any key to continue...");
        Console.ReadKey();
        return null;
      
      }
      innoSetup.WaitForExit();

      //Rename the install executable.
      File.Copy(Path.Combine(cBuildOutput, "Output", "ZeroEngineSetup.exe"),
                Path.Combine(cBuildOutput, "Output", newFileName), true);
      File.Delete(Path.Combine(cBuildOutput, "Output", "ZeroEngineSetup.exe"));

      String buildFilePath = Path.Combine(cBuildOutput, "Output", "BuildInfo.data");
      String metaFileName = newFileNameWithoutExtension + ".meta";
      var metaFilePath = Path.Combine(cBuildOutput, "Output", metaFileName);
      this.OutputBuildInfo(buildFilePath, buildId, metaFileName, newFileName);

      
      buildMeta.SaveMeta(metaFilePath);
      //Open the folder
      //Process.Start("explorer.exe", 
      //              "/select, " + Path.Combine(cBuildOutput, "Output", newFileName));
      return Path.Combine(cBuildOutput, "Output");
    }
  }
}
