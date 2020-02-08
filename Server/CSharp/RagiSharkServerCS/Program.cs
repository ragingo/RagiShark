using System;
using System.Diagnostics;
using System.Net;
using System.Threading.Tasks;
using System.Runtime.InteropServices;
using System.Text.RegularExpressions;

namespace RagiSharkServerCS
{
    class Program
    {
        private static string _listenIPAddress = "";
        private static int _listenPort = -1;
        private static int _captureInterface = -1;
        private static string _captureFilter = "";
        private static bool _sendable = true;

        private static async Task Main(string[] args)
        {
            Console.WriteLine($"args: {string.Join(", ", args)}");

            for (int i = 0; i < args.Length; i++)
            {
                string arg = args[i];
                bool hasNext = i + 1 < args.Length;
                switch (arg)
                {
                    case "-l" when hasNext && Regex.IsMatch(args[i + 1], @"\d+\.\d+\.\d+\.\d+"):
                        _listenIPAddress = args[i + 1];
                        i++;
                        break;
                    case "-p" when hasNext && int.TryParse(args[i + 1], out int port):
                        _listenPort = port;
                        i++;
                        break;
                    case "-i" when hasNext && int.TryParse(args[i + 1], out int nicNumber):
                        _captureInterface = nicNumber;
                        i++;
                        break;
                    case "-f" when hasNext:
                        _captureFilter = args[i + 1];
                        i++;
                        break;
                    default:
                        break;
                }
            }

            if (string.IsNullOrEmpty(_listenIPAddress))
            {
                _listenIPAddress = "127.0.0.1";
            }

            if (_listenPort <= 0)
            {
                Console.WriteLine("-p !");
                return;
            }

            var ws = new WebSocketServer();
            ws.TextDataReceived += OnTextDataReceived;
            _ = Task.Run(Work(ws)).ConfigureAwait(false);
            await ws.Listen(IPAddress.Parse(_listenIPAddress), _listenPort).ConfigureAwait(false);
        }

        private static void OnTextDataReceived(string text)
        {
            Console.WriteLine($"received: {text}");

            switch (text)
            {
                case "pause":
                    _sendable = false;
                    break;
                case "resume":
                    _sendable = true;
                    break;
            }
        }

        private static Tuple<string, string> GetToolInfo()
        {
            string captureInterface = "";
            if (_captureInterface > 0)
            {
                captureInterface = $"-i {_captureInterface}";
            }

            string captureFilter = "";
            if (!string.IsNullOrEmpty(_captureFilter))
            {
                captureFilter = $@"-f ""{_captureFilter}""";
            }

            string args = $"{captureInterface} {captureFilter}".Trim();

            if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
            {
                return Tuple.Create(@"C:\Program Files\Wireshark\tshark.exe", args);
            }
            else if (RuntimeInformation.IsOSPlatform(OSPlatform.OSX))
            {
                return Tuple.Create("tshark", args);
            }
            else
            {
                throw new NotSupportedException();
            }
        }

        private static Func<Task> Work(WebSocketServer ws)
        {
            var (path, args) = GetToolInfo();
            Console.WriteLine($"tshark path: {path}");
            Console.WriteLine($"tshark args: {args}");

            var psi = new ProcessStartInfo(path, args);
            psi.UseShellExecute = false;
            psi.RedirectStandardError = false;
            psi.RedirectStandardOutput = true;
            psi.RedirectStandardInput = false;
            psi.CreateNoWindow = false;

            var p = Process.Start(psi);
            var reader = p.StandardOutput;

            return async () =>
            {
                while (true)
                {
                    if (reader.EndOfStream)
                    {
                        continue;
                    }

                    if (!_sendable)
                    {
                        continue;
                    }

                    string line = await reader.ReadLineAsync().ConfigureAwait(false);
                    ws.PushMessage(line);
                }
            };
        }
    }
}
