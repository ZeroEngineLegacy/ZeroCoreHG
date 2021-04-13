using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net;
using System.Text;
using System.Threading.Tasks;

namespace ZeroLauncherVersionIdUpdater
{
  public class HttpDownloadHelpers
  {
    public delegate void HttpRequestCallback(Stream dataStream);

    public static void StringReaderCallback(Stream dataStream, out String data)
    {
      // Open the stream using a StreamReader for easy access.
      StreamReader reader = new StreamReader(dataStream);
      // Read the content. 
      data = reader.ReadToEnd();

      reader.Close();
    }

    public static bool RequestData(String url, HttpRequestCallback callback)
    {
      WebRequest request = WebRequest.Create(url);
      request.Proxy = null;
      //special credentials we have for downloading newer versions
      request.Credentials = new NetworkCredential("zero.updates", "1CB858EE08E9");

      HttpWebResponse response = (HttpWebResponse)request.GetResponse();
      if (response.StatusCode != HttpStatusCode.OK)
        return false;

      Stream dataStream = response.GetResponseStream();
      //let the user handle any custom logic with the data stream
      callback(dataStream);

      dataStream.Close();
      response.Close();

      return true;
    }

    public static String GetStringData(String url)
    {
      String textData = "";
      RequestData(url, (dataStream) => { StringReaderCallback(dataStream, out textData); });
      return textData;
    }

    public static bool GetStringData(String url, out String textData)
    {
      String data = "";
      bool success = RequestData(url, (dataStream) => { StringReaderCallback(dataStream, out data); });
      textData = data;

      return success;
    }

    static void WriteStreamToDataFile(Stream dataStream, String outputFilePath)
    {
      //make sure the directory exists (otherwise the file create fails)
      var dir = Path.GetDirectoryName(outputFilePath);
      if (!Directory.Exists(dir))
        Directory.CreateDirectory(dir);

      FileStream fs = File.Create(outputFilePath);
      BinaryWriter bw = new BinaryWriter(fs);

      //read a fixed buffer size from the stream and write it
      //out to our file until there's nothing left to write
      int bufferSize = 1000;
      byte[] buffer = new byte[bufferSize];
      int bytesRead = 1;
      while (bytesRead != 0)
      {
        bytesRead = dataStream.Read(buffer, 0, bufferSize);
        bw.Write(buffer, 0, bytesRead);
      }

      bw.Close();
      fs.Close();
    }

    public static bool WriteToDataFile(String url, String outputFilePath)
    {
      return RequestData(url, (dataStream) => { WriteStreamToDataFile(dataStream, outputFilePath); });
    }
  }
}
