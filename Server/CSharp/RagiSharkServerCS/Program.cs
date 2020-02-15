using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
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
    }

    class AppState
    {
        public AppConfig Config { get; set; }
        public int[] CaptureInterfaces { get; set; } = new int[] { };
        public string CaptureFilter { get; set; }
        public bool Sendable { get; set; }
        public bool NeedsRestart { get; set; }
        public string ReceivedCommandRaw { get; set; }
        public AppCommand ReceivedCommand { get; set; }
    }

    enum AppCommand
    {
        None,
        Start,
        Pause,
        Resume,
        ChangeCF,
        ChangeDF,
        GetIFList,
        SetIFList
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
                }
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
                case string s when s.StartsWith("start "):
                    _state.ReceivedCommand = AppCommand.Start;
                    break;
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

                case string s when s.StartsWith("set if list "):
                    _state.ReceivedCommand = AppCommand.SetIFList;
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

                if (cmd == AppCommand.Start)
                {
                    var args = ParseStartCommandArgs(cmdRaw);
                    _state.CaptureInterfaces = args.CaptureInterfaces;
                    _state.CaptureFilter = args.CaptureFilter;
                    _state.Sendable = true;
                    _state.NeedsRestart = true;
                }
                else if (cmd == AppCommand.Pause)
                {
                    _state.Sendable = false;
                }
                else if (cmd == AppCommand.Resume)
                {
                    _state.Sendable = true;
                }
                else if (cmd == AppCommand.SetIFList)
                {
                    if (TryParseSetIFCommand(cmdRaw, out int[] value))
                    {
                        _state.CaptureInterfaces = value;
                        _state.NeedsRestart = true;
                    }
                }
                else if (cmd == AppCommand.ChangeCF)
                {
                    _state.CaptureFilter = cmdRaw.Replace("change cf ", "").Trim();
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
                if (_state.NeedsRestart)
                {
                    _state.NeedsRestart = false;
                    _state.Sendable = true;

                    app.Args = new TSharkAppArgs
                    {
                        CaptureFilter = _state.CaptureFilter,
                        CaptureInterfaces = _state.CaptureInterfaces.Select(x => new CaptureInterface { No = x }).ToArray(),
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

        private static bool TryParseSetIFCommand(string cmd, out int[] values)
        {
            var m = Regex.Match(cmd, @"set if list ((\d+)(,(\d+))*)");
            if (!m.Success)
            {
                values = new int[] { };
                return false;
            }
            if (m.Groups.Count < 2)
            {
                values = new int[] { };
                return false;
            }
            values =
                m.Groups[1].Value
                    .Split(",", StringSplitOptions.RemoveEmptyEntries)
                    .Select(x => int.TryParse(x, out int a) ? a : -1)
                    .Where(x => x >= 0)
                    .ToArray();
            return values.Length > 0;
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

        class StartCommandArgs
        {
            public int[] CaptureInterfaces { get; set; } = new int[] { };
            public string CaptureFilter { get; set; } = "";
        }

        private static StartCommandArgs ParseStartCommandArgs(string cmd)
        {
            var defs = new[] {
                new CommandLineArgDefinition {
                    Keys = new [] { "-i" },
                    Converter = x => {
                        if (!Regex.IsMatch(x, @"(\d+)(\,\d+)*"))
                        {
                            return new int[]{};
                        }
                        return
                            x.Split(",", StringSplitOptions.RemoveEmptyEntries)
                            .Select(y => int.TryParse(y, out int z) ? z : 0)
                            .Where(x => x > 0)
                            .ToArray();
                    }
                },
                new CommandLineArgDefinition {
                    Keys = new [] { "-cf" }
                }
            };

            var result = new StartCommandArgs();
            var args = cmd.Split(' ', StringSplitOptions.RemoveEmptyEntries);

            foreach (var arg in CommandLineParser.Parse(args, defs))
            {
                switch (arg.Key)
                {
                    case "-i":
                        result.CaptureInterfaces = arg.Value as int[];
                        break;
                    case "-cf":
                        result.CaptureFilter = arg.Value as string;
                        break;
                }
            }

            return result;
        }
    }
}
