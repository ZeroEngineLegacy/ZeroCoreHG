using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace ErrorDialog
{
  public partial class CodeMessage : Form
  {
    class Info
    {
      public Info()
            {
                string[] commandLine = System.Environment.GetCommandLineArgs();

                expression = "";

                if (commandLine.Length >= 5)
                {
                    expression = commandLine[2];
                    file = commandLine[3];
                    buildNumber = commandLine[4];
                    environment = System.Environment.OSVersion.ToString();

                    shortfile = file;
                    int index = file.LastIndexOf('\\');
                    if(index != -1)
                      shortfile = file.Substring(index + 1);
                }

                if (expression.Length == 0)
                    expression = "No Expression";

            }
      private string expression;

      [Description("Expression that triggered this warning"), CategoryAttribute("Code Information")]
      public string Expression
      {
        get { return expression; }
        set { expression = value; }
      }

      private string shortfile;
      [Description("Code file that generated this warning. Format, FileName:(LineNumber)"), CategoryAttribute("Code Information")]
      public string File
      {
        get { return shortfile; }
        set { shortfile = value; }
      }

      private string file;
      [Description("Code file that generated this warning. Format, FileName:(LineNumber)"), CategoryAttribute("Code Information")]
      public string FullPath
      {
        get { return file; }
        set { file = value; }
      }

      private string buildNumber;
      [Description("Build number of this executable. Format, Year.Month.Day.Revision.Desc"), CategoryAttribute("Code Information")]
      public string BuildNumber
      {
        get { return buildNumber; }
        set { buildNumber = value; }
      }

      private string environment;

      [Description("Run Time Environment"), CategoryAttribute("Code Information")]
      public string Environment
      {
        get { return environment; }
        set { environment = value; }
      }

    };

    public CodeMessage()
    {
      InitializeComponent();
      this.DataGrid.SelectedObject = new Info();
      string[] commandLine = System.Environment.GetCommandLineArgs();

      //Command Line 0 is path
      if (commandLine.Length > 1)
        this.MessageText.Text = commandLine[1];
      else
        this.MessageText.Text = "No Message";

      this.Continue.KeyPress += new KeyPressEventHandler(CodeMessage_KeyPress);
      this.DebugBreak.KeyPress += new KeyPressEventHandler(CodeMessage_KeyPress);
    }

    private void CodeMessage_KeyPress(object sender, KeyPressEventArgs e)
    {
      switch (e.KeyChar)
      {
        // Backspaces
        case '\b':
          System.Environment.Exit(2);
          break;
        case (char)27:
          System.Environment.Exit(2);
          break;
        case 'd':
          System.Environment.Exit(1);
          break;
        //default:
        //  System.Environment.Exit(1);
        //break; 
      }
    }

    private void DebugBreak_Click(object sender, EventArgs e)
    {
      System.Environment.Exit(1);

    }

    private void Continue_Click(object sender, EventArgs e)
    {
      System.Environment.Exit(0);
    }

    private void Log_Click(object sender, EventArgs e)
    {
    }

    private void Ignore_Click(object sender, EventArgs e)
    {
        System.Environment.Exit(3);
    }


  }
}