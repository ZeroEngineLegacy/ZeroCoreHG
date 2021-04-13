using System;
using System.Collections.Generic;
using System.IO;
using System.IO.Compression;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace PatchZeroBadUrls
{
    class Program
    {
        static void Main(string[] args)
        {
            var path = @"C:\Users\Trevor\Desktop\StandAlones";

            var builds = new List<String>();
            RecurseFiles(path, builds);

            var i = 0;
            foreach (var buildPath in builds)
            {
                FixArchive(buildPath);
                Console.WriteLine(i + " / " + builds.Count);
                ++i;
            }
        }

        static void RecurseFiles(String path, List<String> paths)
        {
            foreach (var dir in Directory.EnumerateDirectories(path))
            {
                RecurseFiles(dir, paths);
            }

            foreach (var file in Directory.EnumerateFiles(path))
            {
                if (Path.GetExtension(file) == ".zerobuild")
                {
                    paths.Add(file);
                }
            }
        }

        static void FixArchive(String path)
        {
            using (ZipArchive archive = ZipFile.Open(path, ZipArchiveMode.Update))
            {
                foreach (ZipArchiveEntry entry in archive.Entries)
                {
                    if (entry.Name == "ZeroEditor.exe")
                    {
                        byte[] contents = null;

                        using (var entryStream = entry.Open())
                        using (var memoryStream = new MemoryStream())
                        {
                            entryStream.CopyTo(memoryStream);
                            contents = memoryStream.ToArray();

                            var length = contents.Length;
                            
                            var find1 = Encoding.ASCII.GetBytes("User name or password is incorrect. Please contact zeroengineservice@digipen.edu for support.");
                            var repl1 = Encoding.ASCII.GetBytes("The login service is currently undergoing maintenance.                                       ");
                            
                            var find2 = Encoding.ASCII.GetBytes("http://zeroservice.digipen.edu/editorlogin?user=%s&password=%s");
                            var repl2 = Encoding.ASCII.GetBytes("http://zeroservice.digipen.edu/checkserver?guid=%p%p&validated");
                            
                            contents = ReplaceBytes(contents, find1, repl1);
                            if (contents == null)
                                return;
                            contents = ReplaceBytes(contents, find2, repl2);
                            if (contents == null)
                                return;

                            if (length != contents.Length)
                                throw new Exception("Invalid length of file!");
                        }

                        using (var entryStream = entry.Open())
                        {
                            var totalLength = entryStream.Length;
                            entryStream.Write(contents, 0, contents.Length);

                            if (totalLength != entryStream.Length && contents.Length != entryStream.Length)
                                throw new Exception("Invalid length of file!");
                        }
                    }
                }
            }
        }

        public static int FindBytes(byte[] src, byte[] find)
        {
            int index = -1;
            int matchIndex = 0;
            // handle the complete source array
            for (int i = 0; i < src.Length; i++)
            {
                if (src[i] == find[matchIndex])
                {
                    if (matchIndex == (find.Length - 1))
                    {
                        index = i - matchIndex;
                        break;
                    }
                    matchIndex++;
                }
                else
                {
                    matchIndex = 0;
                }

            }
            return index;
        }

        public static byte[] ReplaceBytes(byte[] src, byte[] search, byte[] repl)
        {
            byte[] dst = null;
            int index = FindBytes(src, search);
            if (index >= 0)
            {
                dst = new byte[src.Length - search.Length + repl.Length];
                // before found array
                Buffer.BlockCopy(src, 0, dst, 0, index);
                // repl copy
                Buffer.BlockCopy(repl, 0, dst, index, repl.Length);
                // rest of src array
                Buffer.BlockCopy(
                    src,
                    index + search.Length,
                    dst,
                    index + repl.Length,
                    src.Length - (index + search.Length));
            }
            return dst;
        }
    }
}
