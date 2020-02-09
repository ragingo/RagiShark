using System;
using System.Diagnostics;
using System.Net;
using System.Threading.Tasks;
using System.Runtime.InteropServices;
using System.Text.RegularExpressions;
using System.IO;
using System.Text;

namespace RagiSharkServerCS
{
    class AppConfig
    {
        public string ListenIPAddress { get; set; }
        public int ListenPort { get; set; }
        public int CaptureInterface { get; set; }
        public string CaptureFilter { get; set; }
    }

    class Program
    {
        private static bool _sendable = true;

        private static async Task Main(string[] args)
        {
            Console.WriteLine($"args: {string.Join(", ", args)}");

            var config = ParseCommandLineArgs(args);

            var ws = new WebSocketServer();
            ws.TextDataReceived += OnTextDataReceived;
            _ = Task.Run(Work(ws, config)).ConfigureAwait(false);
            await ws.Listen(IPAddress.Parse(config.ListenIPAddress), config.ListenPort).ConfigureAwait(false);
        }

        private static AppConfig ParseCommandLineArgs(string[] args)
        {
            var defs = new [] {
                new CommandLineArgDefinition {
                    Keys = new [] { "-l", "--listen-addr" },
                    Converter = x => Regex.IsMatch(x, @"\d+\.\d+\.\d+\.\d+") ? x : "127.0.0.1"
                },
                new CommandLineArgDefinition {
                    Keys = new [] { "-p", "--listen-port" },
                    Converter = x => int.TryParse(x, out int value) ? value : -1
                },
                new CommandLineArgDefinition {
                    Keys = new [] { "-i", "--capture-interface" },
                    Converter = x => int.TryParse(x, out int value) ? value : -1
                },
                new CommandLineArgDefinition {
                    Keys = new [] { "-f", "--capture-filter" }
                },
            };

            var config = new AppConfig();

            foreach (var arg in CommandLineParser.Parse(args, defs))
            {
                switch (arg.Key)
                {
                    case "-l":
                        config.ListenIPAddress = arg.Value as string;
                        break;
                    case "-p":
                        config.ListenPort = arg.Value is int p ? p : -1;
                        break;
                    case "-i":
                        config.CaptureInterface = arg.Value is int i ? i : -1;
                        break;
                    case "-f":
                        config.CaptureFilter = arg.Value as string;
                        break;
                }
            }

            return config;
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

        private static string GetToolPath()
        {
            if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
            {
                string defaultPath = @"C:\Program Files\Wireshark\tshark.exe";
                return File.Exists(defaultPath) ? defaultPath : "tshark";
            }
            else if (RuntimeInformation.IsOSPlatform(OSPlatform.OSX))
            {
                return "tshark";
            }
            else
            {
                return "tshark";
            }
        }

        private static Process StartProcess(AppConfig config)
        {
            string captureInterface = "";
            if (config.CaptureInterface > 0)
            {
                captureInterface = $"-i {config.CaptureInterface}";
            }

            string captureFilter = "";
            if (!string.IsNullOrEmpty(config.CaptureFilter))
            {
                captureFilter = $@"-f ""{config.CaptureFilter}""";
            }

            var sb = new StringBuilder();
            sb.Append($"{captureInterface} ");
            sb.Append($"{captureFilter} ");
            sb.Append("-T ek ");
            sb.Append("-e ip.src ");
            sb.Append("-e tcp.srcport ");

            string args = sb.ToString().Trim();
            Console.WriteLine($"tshark args: {args}");

            var psi = new ProcessStartInfo(GetToolPath(), args);
            psi.UseShellExecute = false;
            psi.RedirectStandardError = false;
            psi.RedirectStandardOutput = true;
            psi.RedirectStandardInput = false;
            psi.CreateNoWindow = false;

            Process p = null;

            try
            {
                p = Process.Start(psi);
            }
            catch (Exception e)
            {
                Console.WriteLine(e);
            }

            return p;
        }

        private static Func<Task> Work(WebSocketServer ws, AppConfig config)
        {
            var p = StartProcess(config);
            if (p == null)
            {
                return () => Task.CompletedTask;
            }

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
