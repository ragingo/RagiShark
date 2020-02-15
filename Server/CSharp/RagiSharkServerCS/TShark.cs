using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;
using System.Text.RegularExpressions;

namespace RagiSharkServerCS.TShark
{
    class CaptureInterface
    {
        public int No { get; set; }
        public string Name { get; set; }
    }

    class TSharkAppArgs
    {
        public CaptureInterface CaptureInterface { get; set; }
        public string CaptureFilter { get; set; }
    }

    class TSharkApp
    {
        private Process _process;

        public TSharkAppArgs Args { get; set; }
        public bool IsRunning => _process != null && !_process.HasExited;
        public Action<string> StdOutLineReceived { get; set; }

        public void Start()
        {
            if (IsRunning)
            {
                throw new InvalidOperationException();
            }

            var psi = CreateStartInfo();
            psi.Arguments = CreateCaptureArguments(Args);

            try
            {
                _process = Process.Start(psi);
                _process.BeginOutputReadLine();
                _process.OutputDataReceived += OnOutputDataReceived;
            }
            catch (Exception e)
            {
                Debug.WriteLine(e);
            }
        }

        private void OnOutputDataReceived(object sender, DataReceivedEventArgs e)
        {
            if (string.IsNullOrEmpty(e.Data))
            {
                return;
            }
            StdOutLineReceived?.Invoke(e.Data);
        }

        public void Restart()
        {
            Kill();
            Start();
        }

        public void Kill()
        {
            if (_process == null)
            {
                return;
            }

            try
            {
                _process.OutputDataReceived -= OnOutputDataReceived;
                _process.Kill(true);
                _process.Close();
            }
            catch (Exception) { }
            finally
            {
                _process.Dispose();
                _process = null;
            }
        }

        public static IEnumerable<CaptureInterface> GetInterfaceList()
        {
            var list = new List<CaptureInterface>();
            var psi = CreateStartInfo();
            psi.Arguments = CreateInterfaceListArguments();

            try
            {
                var p = Process.Start(psi);
                var r = new Regex(@"^(\d+)\. (.+)$", RegexOptions.Singleline | RegexOptions.Compiled);
                while (!p.StandardOutput.EndOfStream)
                {
                    string line = p.StandardOutput.ReadLine();
                    var m = r.Match(line);
                    if (!m.Success)
                    {
                        continue;
                    }
                    if (m.Groups.Count < 3)
                    {
                        continue;
                    }
                    if (!int.TryParse(m.Groups[1].Value, out int no))
                    {
                        continue;
                    }
                    list.Add(new CaptureInterface
                    {
                        No = no,
                        Name = m.Groups[2].Value
                    });
                }
            }
            catch (Exception e)
            {
                Debug.WriteLine(e);
            }

            return list;
        }

        private static ProcessStartInfo CreateStartInfo()
        {
            var psi = new ProcessStartInfo();
            psi.FileName = FindPath();
            psi.UseShellExecute = false;
            psi.RedirectStandardError = false;
            psi.RedirectStandardOutput = true;
            psi.RedirectStandardInput = false;
            psi.CreateNoWindow = false;
            return psi;
        }

        private static string FindPath()
        {
            if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
            {
                const string DefaultPath = @"C:\Program Files\Wireshark\tshark.exe";
                return File.Exists(DefaultPath) ? DefaultPath : "tshark";
            }
            else if (RuntimeInformation.IsOSPlatform(OSPlatform.OSX))
            {
                const string DefaultPath = "/Applications/Wireshark.app/Contents/MacOS/tshark";
                return File.Exists(DefaultPath) ? DefaultPath : "tshark";
            }
            else
            {
                return "tshark";
            }
        }

        private static string CreateInterfaceListArguments()
        {
            return "-D";
        }

        private static string CreateCaptureArguments(TSharkAppArgs args)
        {
            string captureInterface = "";
            if (args.CaptureInterface.No > 0)
            {
                captureInterface = $"-i {args.CaptureInterface.No}";
            }

            string captureFilter = "";
            if (!string.IsNullOrEmpty(args.CaptureFilter))
            {
                captureFilter = $@"-f ""{args.CaptureFilter}""";
            }

            var sb = new StringBuilder();
            sb.Append($"{captureInterface} ");
            sb.Append($"{captureFilter} ");
            sb.Append("-T ek ");
            sb.Append("-e frame.number ");
            sb.Append("-e ip.proto ");
            sb.Append("-e ip.src ");
            sb.Append("-e ip.dst ");
            sb.Append("-e tcp.srcport ");
            sb.Append("-e tcp.dstport ");

            return sb.ToString();
        }
    }
}
