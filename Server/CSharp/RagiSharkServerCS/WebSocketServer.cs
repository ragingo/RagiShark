using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Security.Cryptography;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;

namespace RagiSharkServerCS
{
    class WebSocketServer
    {
        private const string CommonResponseHeader =
            "HTTP/1.1 101 Switching Protocols\r\n" +
            "Connection: Upgrade\r\n" +
            "Upgrade: websocket\r\n";
        private const string WsGuid = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

        private TcpListener _listener;
        private NetworkStream _stream;
        private readonly ConcurrentQueue<string> _sendQueue = new ConcurrentQueue<string>();

        public WebSocketServer()
        {
        }

        public async Task Listen(IPAddress address, int port)
        {
            _listener = new TcpListener(address, port);
            _listener.Start();

            await Process().ConfigureAwait(false);
        }

        public void SendMessage(string msg)
        {
            if (_stream == null)
            {
                return;
            }
            _sendQueue.Enqueue(msg);
        }

        private async Task Process()
        {
            TcpClient client = null;

            while (true)
            {
                try
                {
                    // 切断されていたら掃除
                    if (client != null)
                    {
                        if (!client.Connected)
                        {
                            Console.WriteLine("client disposing...");
                            client.Close();
                            client.Dispose();
                            client = null;
                            Console.WriteLine("client disposed!");
                        }
                    }

                    // 新たにクライアントの接続を待つ
                    if (client == null)
                    {
                        client = await _listener.AcceptTcpClientAsync().ConfigureAwait(false);
                        _stream = client.GetStream();
                    }

                    // クライアントからの受信物がない空き時間に、こっちからデータを送信
                    if (!_stream.DataAvailable)
                    {
                        await ProcessSendQueue().ConfigureAwait(false);
                        continue;
                    }

                    // クライアントから受信したデータを処理
                    byte[] buf = new byte[client.Available];
                    int len = await _stream.ReadAsync(buf, 0, buf.Length).ConfigureAwait(false);
                    if (len == 0)
                    {
                        continue;
                    }

                    OnDataReceived(buf);
                }
                catch (IOException e) when (e.InnerException is SocketException)
                {
                    Console.WriteLine(e.Message);
                }
            }
        }

        private void OnDataReceived(byte[] buf)
        {
            string data = Encoding.UTF8.GetString(buf);

            if (data.StartsWith("GET"))
            {
                OnGetRequestReceived(data);
            }
            else
            {
                OnDataFrameReceived(buf);
            }
        }

        private async void OnGetRequestReceived(string data)
        {
            var r = new Regex("Sec-WebSocket-Key: (.*)");
            var m = r.Match(data);
            if (m.Groups.Count == 0)
            {
                return;
            }

            string key = m.Groups[1].Value.Trim();
            byte[] newKey = Encoding.UTF8.GetBytes(key + WsGuid);
            using var sha1 = SHA1.Create();
            byte[] hash = sha1.ComputeHash(newKey);
            string hashStr = Convert.ToBase64String(hash);

            var sb = new StringBuilder();
            sb.Append(CommonResponseHeader);
            sb.Append($"Sec-WebSocket-Accept: {hashStr}\r\n");
            sb.Append("\r\n");

            byte[] res = Encoding.UTF8.GetBytes(sb.ToString());
            await _stream.WriteAsync(res, 0, res.Length).ConfigureAwait(false);
            await _stream.FlushAsync().ConfigureAwait(false);
        }

        private void OnDataFrameReceived(byte[] buf)
        {
            Console.WriteLine("--------------------------------------------------");
            Console.WriteLine(string.Join(" ", buf.Select(x => x.ToString("X2"))));

            var header = WebSocketHeader.Parse(buf);
            Console.WriteLine(header);

            switch (header.OpCode)
            {
                case OpCode.Text:
                    OnTextFrameReceived(buf, header);
                    break;

                case OpCode.Close:
                    OnCloseReceived();
                    break;
            }
        }

        private void OnTextFrameReceived(byte[] buf, WebSocketHeader header)
        {
            if (header.PayloadLength <= 125)
            {
                if (header.Mask)
                {
                    var decodedValues = new List<byte>();
                    byte[] maskKey = new[] { buf[2], buf[3], buf[4], buf[5] }; // TODO: span slice でもっと見やすく
                    for (int i = 6; i < buf.Length; i++)
                    {
                        byte e = buf[i];
                        byte m = maskKey[(i - 6) % 4];
                        decodedValues.Add((byte)(e ^ m));
                    }
                    Console.WriteLine($"decoded (hex): {string.Join(" ", decodedValues.Select(x => x.ToString("X2")))}");
                    Console.WriteLine($"decoded (str): {Encoding.UTF8.GetString(decodedValues.ToArray())}");
                }
            }
            else if (header.PayloadLength == 126)
            {
                // TODO:
            }
            else if (header.PayloadLength == 127)
            {
                // TODO:
            }
        }

        private void OnCloseReceived()
        {
            Console.WriteLine("onClose");
        }

        private async Task ProcessSendQueue()
        {
            if (_sendQueue.Count == 0)
            {
                return;
            }

            if (!_sendQueue.TryDequeue(out string str))
            {
                return;
            }

            var bytes = new List<byte>();
            var data = Encoding.UTF8.GetBytes(str);
            var header = WebSocketHeader.Create(true, OpCode.Text, data.Length);
            bytes.AddRange(header.ToBinary());
            bytes.AddRange(data);
            var arr = bytes.ToArray();
            await _stream.WriteAsync(arr, 0, arr.Length).ConfigureAwait(false);
        }

    }
}
