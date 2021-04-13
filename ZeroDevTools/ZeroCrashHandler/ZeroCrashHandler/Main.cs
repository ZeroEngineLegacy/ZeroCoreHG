#define NO_SMTP_FOR_VIRUS_SCANNERS
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO;
using UBR.Products.TimeTrakker.Client.Lib.FogBugz;
using System.Configuration;
using System.Xml;
using System.Security.Permissions;
using Microsoft.Win32;
using System.Net.Mail;
using System.Net;
using System.DirectoryServices.AccountManagement;
using ICSharpCode.SharpZipLib.Zip;
using System.Collections.Specialized;
using System.Threading;
using CommandLine;
using CommandLine.Text;
using Newtonsoft.Json;
using System.Diagnostics;

namespace ZeroCrashHandler
{
	public partial class Main : Form
	{
		// How would we like to report bugs?
		BugReportMode Mode = BugReportMode.JsonPhp;

		// Fog bugz settings data
		const String URL = "---";
		const String FBUsername = "---";
		const String FBPassword = "---";
		const String Title = "Crash";
		const String Project = "---";
		const String Priority = "1 - Must Fix";

		// Email settings data
		const String EUsername = "---";
		const String EPassword = "---";
		const String From = "---";
		const String To = "---";
		const String Host = "---";
		const int Port = 587;
		const bool SSL = true;

		// Php settings data
		const String PhpRedirectUrl = "http://zerocrash.digipen.edu/crash.php";
		const String PhpFileId = "userfile";
		const String PhpEmailId = "email";
		const String PhpBodyId = "body";
		const String PhpVersionId = "version";

		// Was the file sent?
		bool WasSent = false;

		// Additional text to be attached to the email
		StringBuilder AdditionalText = new StringBuilder();

		// Store all the files that need to be uploaded (dump files and whatnot)
		List<String> Files = new List<String>();

		HashSet<String> UserInfoValues = new HashSet<String>();

		Options Options;

		// The bug reporting mechanism
		enum BugReportMode
		{
			FogBugz,
			Email,
			Php,
			JsonPhp,
		}

		// A pair class for serializing data to json
		public class Pair
		{
			public Pair() { }
			public Pair(string name, string value) { Name = name; Value = value; }

			public string Name { get; set; }
			public string Value { get; set; }
		}

		List<Pair> TextFilePairs = new List<Pair>();

		public Main(Options options)
		{
			// Store the command line options
			Options = options;

			// Keep the old list of files around since it's easier to copy it over than change everything to string[]
			if (options.Files != null)
				Files.AddRange(options.Files.ToList());

			// Process any text files
			// (we send them through to the organizer pre-pended with a | so they can be parsed special)
			foreach (String fileName in Files)
			{
				ProcessTextFile(fileName, "|" + Path.GetFileNameWithoutExtension(fileName));
			}

			// Add the special file types (also add them to Files so they get included in the zip, etc...)
			if (options.Stack != null)
			{
				ProcessTextFile(options.Stack, ":Stack");
				Files.Add(options.Stack);
			}
			if (options.Log != null)
			{
				ProcessTextFile(options.Log, ":Log");
				Files.Add(options.Log);
			}

			// Initialize all the form components
			InitializeComponent();
		}


		// The pair name allows us to overwrite the name in the json file to
		// differentiate special file types (such as stack) by preceding them with a ':'
		void ProcessTextFile(String fileName, String pairName = "")
		{
			// Get the extension from the current file (if it's a text file...)
			if (Path.GetExtension(fileName) == ".txt")
			{
				String fileText = "";
				for (int i = 0; i < 10; ++i)
				{
					try
					{
						fileText = File.ReadAllText(fileName);
						break;
					}
					catch (System.Exception ex)
					{
						Thread.Sleep(200);
					}
				}

				var pair = new Pair();
				var fileNameWithoutExt = Path.GetFileNameWithoutExtension(fileName);
				// Add the data to a pair of fileName - fileData
				if (pairName == String.Empty)
					pairName = fileNameWithoutExt;
				pair.Name = pairName;
				pair.Value = fileText;
				TextFilePairs.Add(pair);

				// Append the data to one string for old methods of sending
				AdditionalText.AppendLine("\r\n\r\n" + "---------------------" + fileNameWithoutExt + "---------------------");
				AdditionalText.Append(pair.Value);
			}
		}

		// Load any text files into the "additional text" field
		void ProcessTextFiles()
		{
			// Loop through all the files
			foreach (String fileName in Files)
			{
				ProcessTextFile(fileName);
			}
		}

        private void UpdateShowButtonsLayout()
        {
            // Make all of the show buttons visible if we have something to show
            int extraTextFilesCount = 0;
            if (Options.Files != null)
            {
                foreach (var fileName in Options.Files)
                {
                    if (Path.GetExtension(fileName) == ".txt")
                        ++extraTextFilesCount;
                }
            }

            this.ShowScripts.Visible = extraTextFilesCount != 0;
            this.ShowStack.Visible = Options.Stack != null;
            this.ShowLog.Visible = Options.Log != null;

            int visibleItems = 0;
            if (extraTextFilesCount != 0)
                ++visibleItems;
            if (Options.Stack != null)
                ++visibleItems;
            if (Options.Log != null)
                ++visibleItems;

            if (visibleItems != 0)
            {
                int itemWidth = this.ShowFilesPanel.Size.Width / visibleItems;
                int position = 0;// this.ShowFilesPanel.Location.X;
                if (Options.Stack != null)
                {
                    this.ShowStack.Location = new Point(position, this.ShowStack.Location.Y);
                    this.ShowStack.Size = new Size(itemWidth, this.ShowStack.Size.Height);
                    position += itemWidth;
                }
                if (Options.Log != null)
                {
                    this.ShowLog.Location = new Point(position, this.ShowLog.Location.Y);
                    this.ShowLog.Size = new Size(itemWidth, this.ShowLog.Size.Height);
                    position += itemWidth;
                }
                if (extraTextFilesCount != 0)
                {
                    this.ShowScripts.Location = new Point(position, this.ShowScripts.Location.Y);
                    this.ShowScripts.Size = new Size(itemWidth, this.ShowScripts.Size.Height);
                    position += itemWidth;
                }
            }
        }

		// When the form loads...
		private void Main_Load(object sender, EventArgs e)
		{
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.None;

			// Change the text that tells the user we really want their crash to include the name of the program.
			this.CrashMessageLabel.Text = String.Format(this.CrashMessageLabel.Text, Options.ProgramName);

			// Create the tooltips
			new ToolTip().SetToolTip(this.RestartEngine, "Restarts the engine and loads the last project you had set");
			new ToolTip().SetToolTip(this.RestartEngineSafe, "Restarts the engine, erases your config, and doesn't load anything extra");
			new ToolTip().SetToolTip(this.DoNotRestart, "Doesn't restart the engine at all");

            UpdateShowButtonsLayout();

			// Get the email that we might already have saved away
			String email = Application.UserAppDataRegistry.GetValue("Email") as String;

			// If we found a saved away email...
			if (email != null)
			{
				// Set the email field
				Email.Text = email;
			}
			else
			{
				// Set the email field
				Email.Text = Environment.UserName.Replace(" ", "").ToLower() + "@digipen.edu";
			}

			// Get the restart mode
			String restartMode = Application.UserAppDataRegistry.GetValue("RestartMode") as String;

			// Do we have a restart mode?
			if (restartMode != null)
			{
				// Select the restart engine control
				if (restartMode == "RestartEngine")
				{
					RestartEngine.Select();
				}
				// Select the restart engine safe control
				else if (restartMode == "RestartEngineSafe")
				{
					RestartEngineSafe.Select();
				}
				// Select the do not restart control
				else if (restartMode == "DoNotRestart")
				{
					DoNotRestart.Select();
				}
			}
		}

		// When the form is shown...
		private void Main_Shown(object sender, EventArgs e)
		{
			// Select the "What happened" text
			WhatHappened.Focus();
			WhatHappened.SelectionStart = 0;
			WhatHappened.SelectionLength = WhatHappened.Text.Length;
		}

		// Do the functionality for engine restarting (if it's specified)
		private void DoEngineRestart()
		{
			try
			{
				if (Options.ExePath == null || Options.ExePath == String.Empty)
					return;

				// If we should restart the engine...
				if (RestartEngine.Checked)
				{
					// Start the process
					Application.UserAppDataRegistry.SetValue("RestartMode", "RestartEngine");
					System.Diagnostics.Process.Start(Options.ExePath, @"-crashed");
				}
				// If we should restart the engine in safe mode...
				else if (RestartEngineSafe.Checked)
				{
					// Start the process with the "safe" flag
					Application.UserAppDataRegistry.SetValue("RestartMode", "RestartEngineSafe");
					System.Diagnostics.Process.Start(Options.ExePath, @"-crashed -safe");
				}
				else if (DoNotRestart.Checked)
				{
					// Save the settings
					Application.UserAppDataRegistry.SetValue("RestartMode", "DoNotRestart");
				}
			}
			catch
			{
			}
		}

		// Close the crash handler
		private void CloseCrashHandler()
		{
			// Close out of this application
			Application.Exit();
		}

		private void AddUserInfo(List<Pair> dataPairs, String category, String value)
		{
			if (value == null || value.Trim() == String.Empty)
			{
				return;
			}

			if (UserInfoValues.Contains(value) == false)
			{
				UserInfoValues.Add(value);

				dataPairs.Add(new Pair(category, value));
			}
		}

		private void AddPair(List<Pair> dataPairs, String category, String value)
		{
			if (value == null || value.Trim() == String.Empty)
			{
				return;
			}

			dataPairs.Add(new Pair(category, value));
		}

		private void Send_Click(object sender, EventArgs e)
		{
			// An error was sent
			WasSent = true;

			// Restart the engine if specified
			DoEngineRestart();

			// Hide the form (it will close when its done sending)
			this.Hide();

			// Collect all of the pairs of data to send off 
			List<Pair> dataPairs = new List<Pair>();

			// Add the data that came from the command line
			AddPair(dataPairs, ":Guid", Options.ProgramGuid);
			AddPair(dataPairs, ":ProgramName", Options.ProgramName);
			AddPair(dataPairs, ":Revision", Options.Revision);
			AddPair(dataPairs, ":Version", Options.Version);
			AddPair(dataPairs, ":ChangeSet", Options.ChangeSet);
			AddPair(dataPairs, ":ChangeSetDate", Options.ChangeSetDate);
			AddPair(dataPairs, ":Platform", Options.Platform);
			AddPair(dataPairs, ":Configuration", Options.Configuration);
			// Add the user's email
			dataPairs.Add(new Pair(":UserEmail", Email.Text));


			try { AddUserInfo(dataPairs, "RegisteredOrganization", (String)Microsoft.Win32.Registry.GetValue(@"HKEY_LOCAL_MACHINE\Software\Microsoft\Windows NT\CurrentVersion", "RegisteredOrganization", "")); }
			catch { }
			try { AddUserInfo(dataPairs, "RegisteredOwner", (String)Microsoft.Win32.Registry.GetValue(@"HKEY_LOCAL_MACHINE\Software\Microsoft\Windows NT\CurrentVersion", "RegisteredOwner", "")); }
			catch { }
			try { AddUserInfo(dataPairs, "DisplayName", UserPrincipal.Current.DisplayName); }
			catch { }
			try { AddUserInfo(dataPairs, "DistinguishedName", UserPrincipal.Current.DistinguishedName); }
			catch { }
			try { AddUserInfo(dataPairs, "DomainEmailAddress", UserPrincipal.Current.EmailAddress); }
			catch { }
			try { AddUserInfo(dataPairs, "EmployeeId", UserPrincipal.Current.EmployeeId); }
			catch { }
			try { AddUserInfo(dataPairs, "GivenName", UserPrincipal.Current.GivenName); }
			catch { }
			try { AddUserInfo(dataPairs, "UserPrincipalName", UserPrincipal.Current.UserPrincipalName); }
			catch { }
			try { AddUserInfo(dataPairs, "HostName", Dns.GetHostName()); }
			catch { }
			try { AddUserInfo(dataPairs, "MachineName", Environment.MachineName); }
			catch { }
			try { AddUserInfo(dataPairs, "UserName", Environment.UserName); }
			catch { }
			try { AddUserInfo(dataPairs, "UserDomainName", Environment.UserDomainName); }
			catch { }


			// Assume they did not fill it in
			String whatHappened = String.Empty;

			StringBuilder userInfo = new StringBuilder();
			foreach (var pair in dataPairs)
			{
				userInfo.AppendLine(pair.Name + ": " + pair.Value);
			}

			dataPairs.Add(new Pair(":UserText", WhatHappened.Text));
			// If they actually filled in the "what happened" field...
			whatHappened = userInfo.ToString() + "\r\n\r\n" + WhatHappened.Text;

			// Add the additional text
			whatHappened += AdditionalText.ToString();

			// If extra data was passed in to describe something that went wrong then add that as one string
			if (Options.ExtraData != null)
			{
				StringBuilder extraData = new StringBuilder();
				foreach (var entry in Options.ExtraData)
					extraData.AppendLine(entry);
				TextFilePairs.Add(new Pair("ExtraData", extraData.ToString()));

				// For the old methods of viewing (non-jsonphp)
				whatHappened += "\r\n\r\n" + "---------------------ExtraData---------------------\r\n" + extraData.ToString();
			}

			// Add all of the pairs of text data that we saved off to our list of entries for the json.
			// This is done last here so as to not break the previous methods of sending.
			dataPairs.AddRange(TextFilePairs);

			// Create the bug report
			CreateBugReport(whatHappened, Email.Text, dataPairs);

			// Exit out
			CloseCrashHandler();
		}

		private void CreateBugReport(String whatHappened, String email, List<Pair> dataPairs)
		{
			// Save the email that we're using now for the next time this dialog appears
			Application.UserAppDataRegistry.SetValue("Email", email);

			// Depending on the mode...
			if (Mode == BugReportMode.FogBugz)
			{
				// Create the bug report
				CreateFogBugzReport(whatHappened, Email.Text);
			}
			else if (Mode == BugReportMode.Email)
			{
				// Create the bug report
				CreateEmailReport(whatHappened, Email.Text);
			}
			else if (Mode == BugReportMode.Php)
			{
				// Create the bug report
				CreatePhpReport(whatHappened, Email.Text);
			}
			else if (Mode == BugReportMode.JsonPhp)
			{
				// Create the json data to send to php
				CreateJsonPhpReport(whatHappened, Email.Text, dataPairs);
			}
		}

		// Create the email message
		private void SendEmail(String whatHappened, String email, bool doAttachments)
		{
#if NO_SMTP_FOR_VIRUS_SCANNERS
			throw new Exception("Cannot send an email because we disabled it");
#else
			// Create a new email client
			SmtpClient emailClient = new SmtpClient();

			// Set the host and port to be gmail
			emailClient.Host = Host;
			emailClient.Port = Port;

			// Enable SSL since gmail requires it
			emailClient.EnableSsl = SSL;

			// Set the credentials
			emailClient.Credentials = new NetworkCredential(EUsername, EPassword);


			// Create a message that will be sent
			MailMessage message = new MailMessage();

			// Setup the message
			message.From = new MailAddress(From);
			message.To.Add(To);
			message.Subject = String.Format("{0} Crash - [{1} {2} {3} {4} {5} {6}]", Options.ProgramName, Options.Version, Options.Revision, Options.ChangeSet, Options.ChangeSetDate, Options.Configuration, Options.Platform);
			message.Body = whatHappened;

			// Should we attach anything to the email?
			if (doAttachments)
			{
				// Loop through all the files
				for (int i = 0; i < Files.Count; i++)
				{
					try
					{
						// Add the attachment to the message
						message.Attachments.Add(new Attachment(Files[i], "application/octet-stream"));
					}
					catch
					{
					}
				}
			}

			// Send the message
			emailClient.Send(message);
#endif
		}


		private void CreatePhpReport(String whatHappened, String email)
		{
			var zipFile = CreateZipFile(Files);

			var collection = new NameValueCollection();
			collection.Add(PhpBodyId, whatHappened);
			collection.Add(PhpEmailId, email);
			collection.Add(PhpVersionId, Options.Revision);

			HttpUploadFile(PhpRedirectUrl, zipFile, PhpFileId, "application/octet-stream", collection);
		}

		private void CreateJsonPhpReport(String whatHappened, String email, List<Pair> dataPairs)
		{
			// Create a stream and a writer to that stream so we can write it out to json
			MemoryStream stream = new MemoryStream();
			StreamWriter streamWriter = new StreamWriter(stream);

			// Create a json writer object and tell it to not close the
			// output stream when it's done (so we can read from it again)
			JsonWriter jsonWriter = new JsonTextWriter(streamWriter);
			jsonWriter.CloseOutput = false;
			jsonWriter.Formatting = Newtonsoft.Json.Formatting.Indented;

			// Serialize out our data
			JsonSerializer serializer = new JsonSerializer();
			List<string> errors = new List<string>();
			// Log errors for debugging (just during VS runs)
			serializer.Error += delegate(object sender, Newtonsoft.Json.Serialization.ErrorEventArgs args)
			{
				// only log an error once
				if (args.CurrentObject == args.ErrorContext.OriginalObject)
					errors.Add(args.ErrorContext.Error.Message);
			};
			serializer.NullValueHandling = NullValueHandling.Ignore;
			serializer.Serialize(jsonWriter, dataPairs);

			// Close the json writer so that it flushes to the memory stream
			jsonWriter.Close();

			// Seek back to the beginning of the stream and read out all the json data
			streamWriter.Flush();
			stream.Seek(0, SeekOrigin.Begin);
			StreamReader streamReader = new StreamReader(stream);
			String jsonData = streamReader.ReadToEnd();


			// Now create the php data.
			// First grab all of the files and zip them together
			var zipFile = CreateZipFile(Files);

			// Then add the json data and the email to the php data and send it
			var collection = new NameValueCollection();
			collection.Add(PhpBodyId, jsonData);
			collection.Add(PhpEmailId, email);
			collection.Add(PhpVersionId, String.Format("{1} {2} {3} {4} {5} {6}", Options.ProgramName, Options.Version, Options.Revision, Options.ChangeSet, Options.ChangeSetDate, Options.Configuration, Options.Platform));

            try { HttpUploadFile(PhpRedirectUrl, zipFile, PhpFileId, "application/octet-stream", collection); }
            catch { }
			
		}

		private void CreateEmailReport(String whatHappened, String email)
		{
			try
			{
				SendEmail(whatHappened, email, true);
			}
			catch
			{
				try
				{
					SendEmail(whatHappened, email, false);
				}
				catch
				{
				}
			}
		}

		private void CreateFogBugzReport(String whatHappened, String email)
		{
			// Set the URL that we'll connect to (why is this a static and not on the FB object?)
			FBApi.Url = URL;

			// Create a fog bugz api object
			FBApi fogBugz = null;

			try
			{
				// Create the object and give it the username and password that we'll login with
				fogBugz = new FBApi(FBUsername, FBPassword);
			}
			catch
			{
				// If it failed, do the email report then exit out
				CreateEmailReport(whatHappened, email);
				return;
			}

			// Create a dictionary 
			Dictionary<String, String> args = new Dictionary<String, String>();

			// Add arguments to the fog bugz data
			args.Add("sTitle", Title);
			args.Add("sProject", Project);
			args.Add("sPriority", Priority);
			args.Add("sVersion", Options.Revision);
			args.Add("sComputer", Environment.MachineName + " - " + Environment.UserName);
			args.Add("sEvent", whatHappened);

			// Select the columns we would like to be uploading
			args.Add("cols", "fOpen,sTitle,sProject,sPriority,sVersion,sComputer,sCustomerEmail,dtOpened");

			// Encoding for binary files
			ASCIIEncoding encoding = new ASCIIEncoding();

			// Create a map of strings to loaded files
			Dictionary<String, Byte[]>[] files = null;

			// If any files were specified to be uploaded (hopefully the dump and other project files)
			if (Files.Count != 0)
			{
				// Create the files dictionary (othrewise it would be left null if we had no files)
				files = new Dictionary<String, Byte[]>[Files.Count];

				// Loop through all the files
				for (int i = 0; i < Files.Count; i++)
				{
					// Create the settings for each file
					files[i] = new Dictionary<String, Byte[]>();

					// Set the name, file name, and the content type of each file
					files[i]["name"] = encoding.GetBytes("File" + (i + 1).ToString());
					files[i]["filename"] = encoding.GetBytes(Files[i].Substring(Files[i].LastIndexOf("\\") + 1));
					files[i]["contenttype"] = encoding.GetBytes(GetMIMEType(Files[i]));

					// Create a stream and binary reader to read the file
					FileStream fs = new FileStream(Files[i], FileMode.Open);
					BinaryReader br = new BinaryReader(fs);

					// Read in all the data and add it to our dictionary under "data"
					files[i]["data"] = br.ReadBytes((int)fs.Length);

					// Close the file stream
					fs.Close();
				}

				// Add the file count to the list
				args.Add("nFileCount", Files.Count.ToString());
			}

			// We're making a new bug, use that command
			fogBugz.Cmd("new", args, files);
		}

		// Get the MIME type of a given file
		// From http://www.codeproject.com/dotnet/ContentType.asp
		private string GetMIMEType(string filepath)
		{
			RegistryPermission regPerm = new RegistryPermission(RegistryPermissionAccess.Read, "\\\\HKEY_CLASSES_ROOT");
			FileInfo fi = new FileInfo(filepath);
			RegistryKey classesRoot = Registry.ClassesRoot;
			String dotExt = fi.Extension.ToLower();
			RegistryKey typeKey = classesRoot.OpenSubKey("MIME\\Database\\Content Type");

			foreach (String keyname in typeKey.GetSubKeyNames())
			{
				RegistryKey curKey = classesRoot.OpenSubKey("MIME\\Database\\Content Type\\" + keyname);
				if (curKey.GetValue("Extension") != null && curKey.GetValue("Extension").ToString().ToLower() == dotExt)
					return keyname;
			}
			return String.Empty;
		}

		private void Main_FormClosing(object sender, FormClosingEventArgs e)
		{
			if (WasSent == false)
			{
				var ask = MessageBox.Show("We'd really appreciate it if you sent the crash report. Are you sure you want to close?", "Close", MessageBoxButtons.YesNo, MessageBoxIcon.Exclamation);
				if (ask == System.Windows.Forms.DialogResult.No)
					e.Cancel = true;
			}
		}

		public static String CreateZipFile(List<String> filenames)
		{
			var zipFilePath = Path.GetTempFileName() + ".zip";
			var zipFile = File.Create(zipFilePath);
			// 'using' statements guarantee the stream is closed properly which is a big source
			// of problems otherwise.  Its exception safe as well which is great.
			using (ZipOutputStream s = new ZipOutputStream(zipFile))
			{
				s.SetLevel(9); // 0 - store only to 9 - means best compression

				byte[] buffer = new byte[4096];

				foreach (string file in filenames)
				{
					int tries = 0;
					FileStream fs = null;

					while (fs == null && tries < 10)
					{
						try
						{
							fs = File.OpenRead(file);
						}
						catch
						{
							Thread.Sleep(200);
							++tries;
						}
					}

					if (fs == null)
						continue;

					try
					{
						// Using GetFileName makes the result compatible with XP
						// as the resulting path is not absolute.
						ZipEntry entry = new ZipEntry(Path.GetFileName(file));

						// Setup the entry data as required.

						// Crc and size are handled by the library for seakable streams
						// so no need to do them here.

						// Could also use the last write time or similar for the file.
						entry.DateTime = DateTime.Now;
						s.PutNextEntry(entry);

						// Using a fixed size buffer here makes no noticeable difference for output
						// but keeps a lid on memory usage.
						int sourceBytes;
						do
						{
							sourceBytes = fs.Read(buffer, 0, buffer.Length);
							s.Write(buffer, 0, sourceBytes);
						}
						while (sourceBytes > 0);
					}
					catch
					{
					}
					finally
					{
						fs.Close();
					}
				}

				// Finish/Close aren't needed strictly as the using statement does this automatically

				// Finish is important to ensure trailing information for a Zip file is appended.  Without this
				// the created file would be invalid.
				s.Finish();

				// Close is important to wrap things up and unlock the file.
				s.Close();
			}

			return zipFilePath;
		}

		public static Uri GetRedirectUrl(string redirectUrl)
		{
			var request = (HttpWebRequest)WebRequest.Create(redirectUrl);
			var response = request.GetResponse();
			return response.ResponseUri;
		}

		public static void HttpUploadFile(string redirectUrl, string file, string paramName, string contentType, NameValueCollection nvc)
		{
			var url = GetRedirectUrl(redirectUrl);

			string boundary = "---------------------------" + DateTime.Now.Ticks.ToString("x");
			byte[] boundarybytes = System.Text.Encoding.ASCII.GetBytes("\r\n--" + boundary + "\r\n");

			HttpWebRequest wr = (HttpWebRequest)WebRequest.Create(url);
			wr.ContentType = "multipart/form-data; boundary=" + boundary;
			wr.Method = "POST";
			wr.KeepAlive = true;
			wr.Credentials = System.Net.CredentialCache.DefaultCredentials;

			Stream rs = wr.GetRequestStream();

			string formdataTemplate = "Content-Disposition: form-data; name=\"{0}\"\r\n\r\n{1}";
			foreach (string key in nvc.Keys)
			{
				rs.Write(boundarybytes, 0, boundarybytes.Length);
				string formitem = string.Format(formdataTemplate, key, nvc[key]);
				byte[] formitembytes = System.Text.Encoding.UTF8.GetBytes(formitem);
				rs.Write(formitembytes, 0, formitembytes.Length);
			}
			rs.Write(boundarybytes, 0, boundarybytes.Length);

			string headerTemplate = "Content-Disposition: form-data; name=\"{0}\"; filename=\"{1}\"\r\nContent-Type: {2}\r\n\r\n";
			string header = string.Format(headerTemplate, paramName, file, contentType);
			byte[] headerbytes = System.Text.Encoding.UTF8.GetBytes(header);
			rs.Write(headerbytes, 0, headerbytes.Length);

			FileStream fileStream = new FileStream(file, FileMode.Open, FileAccess.Read);
			byte[] buffer = new byte[4096];
			int bytesRead = 0;
			while ((bytesRead = fileStream.Read(buffer, 0, buffer.Length)) != 0)
			{
				rs.Write(buffer, 0, bytesRead);
			}
			fileStream.Close();

			byte[] trailer = System.Text.Encoding.ASCII.GetBytes("\r\n--" + boundary + "--\r\n");
			rs.Write(trailer, 0, trailer.Length);
			rs.Close();

			WebResponse wresp = null;
			try
			{
				wresp = wr.GetResponse();
				Stream stream2 = wresp.GetResponseStream();
				StreamReader reader2 = new StreamReader(stream2);
				var result = reader2.ReadToEnd();
				result = result;
			}
			catch (Exception ex)
			{
				if (wresp != null)
				{
					wresp.Close();
					wresp = null;
				}
			}
			finally
			{
				wr = null;
			}
		}

        private void OpenFile(string file)
        {
            ProcessStartInfo info = new ProcessStartInfo();
            info.FileName = file;
            info.Verb = "Open";
            System.Diagnostics.Process.Start(info);
        }

        private void ShowStack_Click(object sender, EventArgs e)
        {
            if(Options.Stack != null)
            {
                OpenFile(Options.Stack);
            }
        }

        private void ShowLog_Click(object sender, EventArgs e)
        {
            if (Options.Log != null)
            {
                OpenFile(Options.Log);
            }
        }

        private void ShowScripts_Click(object sender, EventArgs e)
        {
            if(Options.Files != null)
            {
                foreach (var file in Options.Files)
                {
                    if (Path.GetExtension(file) == ".txt")
                    {
                        OpenFile(file);
                    }
                }
            }
        }
	}

	// The command line options that we parse
	public class Options
	{
		// Required options
		[Option("Guid", Required = true, HelpText = "The guid of the program")]
		public string ProgramGuid { get; set; }

		[Option("Name", Required = true, HelpText = "The name of the program")]
		public string ProgramName { get; set; }

		[Option("Revision", Required = true, HelpText = "The revision of the program. This needs to be some ever increasing number, but should be tied to source control.")]
		public string Revision { get; set; }

		// Optional options
		[Option("Version", Required = false, HelpText = "Some identifier of what version of the engine this is. Typically this is Major.Minor.Patch.")]
		public string Version { get; set; }
		[Option("ChangeSet", Required = false, HelpText = "The changeset of a build is a unique identifier in source control. Revision numbers are not always unique so this can be used to revert code to a specific changeset.")]
		public string ChangeSet { get; set; }
		[Option("ChangeSetDate", Required = false, HelpText = "The date this changeset was made. Useful to help identify when a certain build was made.")]
		public string ChangeSetDate { get; set; }
		[Option("Platform", Required = false, HelpText = "The platform of this build (such as Win32).")]
		public string Platform { get; set; }
		[Option("Configuration", Required = false, HelpText = "What configuration this build was made in. Typically this should always be Release")]
		public string Configuration { get; set; }

		// Stack and log are special from other files so that we can send them to the crash
		// organizer with a special format to prevent any other files from accidentally taking the stack's place.
		[Option("Stack", Required = false, HelpText = "The file that contains the stack trace of the crash. This is used to help uniquely identify where a crash happened.")]
		public string Stack { get; set; }
		[Option("Log", Required = false, HelpText = "A log file containing information about what the user has been doing before the crash.")]
		public string Log { get; set; }

		[OptionArray("Files", HelpText = "Any extra files to add to the crash report such as a script file they were running during the crash. Allows multiple arguments.")]
		public string[] Files { get; set; }

		[OptionArray("ExtraData", HelpText = "Extra data to add to the report (such as a step of the crash handler failing). Allows multiple arguments.")]
		public string[] ExtraData { get; set; }

		[Option("ExePath", Required = false, HelpText = "The location of the program that we will restart if the user chooses the restart option. The program will be invoked with the argument /crashed. If the user requests to restart in safe mode then the arguments will be /crashed /safe.")]
		public string ExePath { get; set; }

		[HelpOption(HelpText = "Display this help screen.")]
		public string GetUsage()
		{
			return HelpText.AutoBuild(this, current => HelpText.DefaultParsingErrorsHandler(this, current));
		}
	}
}
