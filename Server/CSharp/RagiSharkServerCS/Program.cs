using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Net;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
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
        public bool NeedsRestart { get; set; }
        public string ReceivedCommandRaw { get; set; }
        public AppCommand ReceivedCommand { get; set; }
    }

    enum AppCommand
    {
        None,
        Pause,
        Resume,
        ChangeCF,
        ChangeDF,
        GetIFList,
        SetIF
    }

    class Program
    {
        private static readonly AppState _state = new AppState();

        private static async Task Main(string[] args)
        {
            Console.WriteLine($"args: {string.Join(", ", args)}");

            var config = ParseCommandLineArgs(args);
            _state.Config = config;

            var ws = new WebSocketServer();
            ws.TextDataReceived += OnTextDataReceived;
            _ = Task.Run(() => ProcessStdOut(ws)).ConfigureAwait(false);
            _ = Task.Run(() => ProcessClientCommand(ws)).ConfigureAwait(false);
            await ws.Listen(IPAddress.Parse(config.ListenIPAddress), config.ListenPort).ConfigureAwait(false);
        }

        private static AppConfig ParseCommandLineArgs(string[] args)
        {
            var defs = new[] {
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
            Debug.WriteLine($"received: {text}");

            _state.ReceivedCommandRaw = text;

            switch (text)
            {
                case "pause":
                    _state.ReceivedCommand = AppCommand.Pause;
                    break;
                case "resume":
                    _state.ReceivedCommand = AppCommand.Resume;
                    break;

                case string s when s.StartsWith("change cf "):
                    _state.ReceivedCommand = AppCommand.ChangeCF;
                    break;

                case "get if list":
                    _state.ReceivedCommand = AppCommand.GetIFList;
                    break;

                case string s when s.StartsWith("set if "):
                    _state.ReceivedCommand = AppCommand.SetIF;
                    break;

                default:
                    Debug.WriteLine("unknown command");
                    break;
            }
        }

        private static async Task ProcessClientCommand(WebSocketServer ws)
        {
            while (true)
            {
                var cmd = _state.ReceivedCommand;
                string cmdRaw = _state.ReceivedCommandRaw;
                _state.ReceivedCommand = AppCommand.None;
                _state.ReceivedCommandRaw = "";

                if (cmd == AppCommand.Pause)
                {
                    _state.Sendable = false;
                }
                else if (cmd == AppCommand.Resume)
                {
                    _state.Sendable = true;
                }
                else if (cmd == AppCommand.SetIF)
                {
                    if (TryParseSetIFCommand(cmdRaw, out int value))
                    {
                        _state.Config.CaptureInterface = value;
                        _state.NeedsRestart = true;
                    }
                }
                else if (cmd == AppCommand.ChangeCF)
                {
                    _state.Config.CaptureFilter = cmdRaw.Replace("change cf ", "").Trim();
                    _state.NeedsRestart = true;
                }
                else if (cmd == AppCommand.GetIFList)
                {
                    string msg = ToGetIFListJson(TSharkApp.GetInterfaceList());
                    Debug.WriteLine($"send msg: {msg}");
                    ws.PushMessage(msg, true);
                }

                await Task.Delay(50).ConfigureAwait(false);
            }
        }

        private static async Task ProcessStdOut(WebSocketServer ws)
        {
            var app = new TSharkApp();
            app.StdOutLineReceived = (line) =>
            {
                if (_state.Sendable)
                {
                    ws.PushMessage(line);
                }
            };

            while (true)
            {
                if (!app.IsRunning || _state.NeedsRestart)
                {
                    _state.NeedsRestart = false;
                    _state.Sendable = true;

                    app.Args = new TSharkAppArgs
                    {
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
            }
        }

        private static bool TryParseSetIFCommand(string cmd, out int value)
        {
            var m = Regex.Match(cmd, @"set if (\d+)");
            if (!m.Success)
            {
                value = -1;
                return false;
            }
            if (m.Groups.Count != 2)
            {
                value = -1;
                return false;
            }
            return int.TryParse(m.Groups[1].Value, out value);
        }

        private static string ToGetIFListJson(IEnumerable<CaptureInterface> items)
        {
            var sb = new StringBuilder();
            sb.Append($@" {{");
            sb.Append($@" ""type"": ""get_if_list_response"", ");
            sb.Append($@" ""data"": [ ");
            int i = 0;
            foreach (var item in items)
            {
                if (i > 0)
                {
                    sb.Append(",");
                }
                string name = item.Name.Replace("\\", "\\\\");
                sb.Append($@" {{ ""no"": {item.No}, ""name"": ""{name}"" }}");
                i++;
            }
            sb.Append($@" ] ");
            sb.Append($@" }}");
            return sb.ToString();
        }
    }
}
