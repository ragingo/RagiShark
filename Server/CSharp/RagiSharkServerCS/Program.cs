using System;
using System.Net;
using System.Threading.Tasks;
using System.Text.RegularExpressions;
using RagiSharkServerCS.TShark;

namespace RagiSharkServerCS
{
    class AppConfig
    {
        public string ListenIPAddress { get; set; }
        public int ListenPort { get; set; }
        public int CaptureInterface { get; set; }
        public string CaptureFilter { get; set; }
    }

    class AppState
    {
        public AppConfig Config { get; set; }
        public bool Sendable { get; set; }
        public bool Restarting { get; set; }
    }

    class Program
    {
        private static readonly AppState _state = new AppState();

        private static async Task Main(string[] args)
        {
            Console.WriteLine($"args: {string.Join(", ", args)}");

            var config = ParseCommandLineArgs(args);

            _state.Config = config;
            _state.Restarting = false;
            _state.Sendable = true;

            var ws = new WebSocketServer();
            ws.TextDataReceived += OnTextDataReceived;
            _ = Task.Run(Work(ws)).ConfigureAwait(false);
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
                    _state.Sendable = false;
                    break;
                case "resume":
                    _state.Sendable = true;
                    break;

                case string s when s.StartsWith("change cf"):
                    _state.Sendable = false;
                    _state.Restarting = true;
                    _state.Config.CaptureFilter = s.Replace("change cf ", "").Trim(); // TODO: かなり雑。直す。
                    break;
            }
        }

        private static Func<Task> Work(WebSocketServer ws)
        {
            return async () =>
            {
                var app = new TSharkApp();

                while (true)
                {
                    if (!app.IsRunning || _state.Restarting)
                    {
                        _state.Restarting = false;
                        _state.Sendable = true;

                        app.Args = new TSharkAppArgs {
                            CaptureFilter = _state.Config.CaptureFilter,
                            CaptureInterface = new CaptureInterface { No = _state.Config.CaptureInterface },
                        };
                        app.Restart();
                        if (!app.IsRunning)
                        {
                            await Task.Delay(1000).ConfigureAwait(false);
                            continue;
                        }
                    }

                    var reader = app.StandardOutput;

                    if (reader.EndOfStream)
                    {
                        continue;
                    }

                    if (!_state.Sendable)
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
