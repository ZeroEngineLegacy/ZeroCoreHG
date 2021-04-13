using CommandLine;
using CommandLine.Text;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ZeroLauncherVersionIdUpdater
{

  // The command line options that we parse
  public class CommandArgs
  {
    [Option("ZeroOutDir", Required = true, HelpText = "The output directory of where to grab the dll from.")]
    public string ZeroOutDir { get; set; }

    [Option("OutDir", Required = true, HelpText = "Where to put the resultant package (and id file).")]
    public string OutDir { get; set; }

    [Option("SourceDir", Required = true, HelpText = "The directory of the source (where the tools and data can be found).")]
    public string SourceDir { get; set; }

    [Option("OpenExplorer", Required = false, HelpText = "Create the new version id file that is what's on the server + 1")]
    public bool OpenExplorerProperty { get { return OpenExplorer; } set { OpenExplorer = value; } }
    public bool OpenExplorer = false;

    [Option("CreatePackage", Required = false, HelpText = "Should the program package up the tools/resources or just create the version id file")]
    public bool CreatePackageProperty { get { return CreatePackage; } set {CreatePackage = value;} }
    public bool CreatePackage = false;

    [Option("CreatePatch", Required = false, HelpText = "Should a patch installer for the launcher be created. Only  meant for devs to locally update their launcher easily")]
    public bool CreatePatchProperty { get { return CreatePatch; } set { CreatePatch = value; } }
    public bool CreatePatch = false;

    [Option("Install", Required = false, HelpText = "Should the program package up the tools/resources or just create the version id file")]
    public bool GenerateInstallerProperty { get { return GenerateInstaller; } set { GenerateInstaller = value; } }
    public bool GenerateInstaller = false;

    [HelpOption(HelpText = "Display this help screen.")]
    public string GetUsage()
    {
      return HelpText.AutoBuild(this, current => HelpText.DefaultParsingErrorsHandler(this, current));
    }
  }
}
