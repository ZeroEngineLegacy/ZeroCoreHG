using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Forms;

using CommandLine;
using CommandLine.Text;

namespace ZeroCrashHandler
{
  

	static class Program
	{
		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static void Main(String[] arguments)
		{
      var options = new Options();

      var settings = new CommandLine.ParserSettings();
      settings.IgnoreUnknownArguments = false;
      CommandLine.Parser parser = new Parser(with => with.HelpWriter = Console.Error);
      
      // Try to parse the command line arguments
      if (parser.ParseArguments(arguments, options) == false)
      {
        // If we failed to parse print out the usage of this program
        Console.Write(options.GetUsage());

        return;
      }

			// Enable visuals
			Application.EnableVisualStyles();
			Application.SetCompatibleTextRenderingDefault(false);

            try { Application.Run(new Main(options)); }
            catch { }
		}
	}
}
