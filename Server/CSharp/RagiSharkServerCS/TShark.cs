using System;
using System.Diagnostics;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;

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

        public StreamReader StandardOutput => _process?.StandardOutput;

        public void Start()
        {
            if (IsRunning)
            {
                throw new InvalidOperationException();
            }

            var psi = CreateStartInfo();
            psi.Arguments = CreateCaptureArgments(Args);
            try
            {
                _process = Process.Start(psi);
            }
            catch (Exception e)
            {
                Debug.WriteLine(e);
            }
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
                _process.Kill(true);
                _process.Close();
            }
            catch (Exception) {}
            finally
            {
                _process.Dispose();
                _process = null;
            }
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

        private static string CreateCaptureArgments(TSharkAppArgs args)
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
