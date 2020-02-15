using System;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
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
        private bool _needsRefresh;
        private readonly List<string> _sendQueue = new List<string>();

        public event Action<string> TextDataReceived;

        public WebSocketServer()
        {
        }

        public async Task Listen(IPAddress address, int port)
        {
            _listener = new TcpListener(address, port);
            _listener.Start();

            await Process().ConfigureAwait(false);
        }

        public void PushMessage(string msg, bool highPriority = false)
        {
            if (_stream == null)
            {
                return;
            }

            lock ((_sendQueue as ICollection).SyncRoot)
            {
                if (highPriority)
                {
                    _sendQueue.Insert(0, msg);
                }
                else
                {
                    _sendQueue.Add(msg);
                }
            }
        }

        private async Task Process()
        {
            TcpClient client = null;

            while (true)
            {
                try
                {
                    // 切断されていたら掃除
                    if ((client != null && !client.Connected) || _needsRefresh)
                    {
                        _needsRefresh = false;
                        Debug.WriteLine("client disposing...");
                        client.Close();
                        client.Dispose();
                        client = null;
                        Debug.WriteLine("client disposed!");
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

                    OnDataReceived(buf.AsSpan());
                }
                catch (IOException e) when (e.InnerException is SocketException)
                {
                    Debug.WriteLine(e.Message);
                }
            }
        }

        private void OnDataReceived(ReadOnlySpan<byte> span)
        {
            string data = Encoding.UTF8.GetString(span);

            if (data.StartsWith("GET"))
            {
                OnGetRequestReceived(data);
            }
            else
            {
                OnDataFrameReceived(span);
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

            string s = sb.ToString();
            byte[] res = Encoding.UTF8.GetBytes(s);
            try
            {
                await _stream.WriteAsync(res, 0, res.Length).ConfigureAwait(false);
            }
            catch (IOException e) when (e.InnerException is SocketException)
            {
                Debug.WriteLine(e.Message);
            }
        }

        private void OnDataFrameReceived(ReadOnlySpan<byte> span)
        {
            // Console.WriteLine("--------------------------------------------------");
            // Console.WriteLine(string.Join(" ", span.ToArray().Select(x => x.ToString("X2"))));

            var header = WebSocketHeader.Parse(span);
            Console.WriteLine(header);

            switch (header.OpCode)
            {
                case OpCode.Text:
                    OnTextFrameReceived(span, header);
                    break;

                case OpCode.Close:
                    OnCloseReceived();
                    break;
            }
        }

        private void OnTextFrameReceived(ReadOnlySpan<byte> span, WebSocketHeader header)
        {
            if (header.PayloadLength <= 125)
            {
                if (header.Mask)
                {
                    string text = DecodeText(span);
                    TextDataReceived?.Invoke(text);
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
            _needsRefresh = true;
        }

        private async Task ProcessSendQueue()
        {
            string item = "";

            lock ((_sendQueue as ICollection).SyncRoot)
            {
                if (_sendQueue.Count == 0)
                {
                    return;
                }
                item = _sendQueue[0];
                _sendQueue.RemoveAt(0);
            }

            var bytes = new List<byte>();
            var data = Encoding.UTF8.GetBytes(item);
            var header = WebSocketHeader.Create(true, OpCode.Text, data.Length);
            bytes.AddRange(header.ToBinary());
            bytes.AddRange(data);
            var arr = bytes.ToArray();
            try
            {
                await _stream.WriteAsync(arr, 0, arr.Length).ConfigureAwait(false);
            }
            catch (IOException e) when (e.InnerException is SocketException)
            {
                Debug.WriteLine(e.Message);
            }
        }

        private static string DecodeText(ReadOnlySpan<byte> span)
        {
            const int maskingKeyOffset = 2;
            const int maskingKeyLength = 4;
            var maskingKey = span.Slice(maskingKeyOffset, maskingKeyLength);

            const int dataOffset = maskingKeyOffset + maskingKeyLength;
            var data = span.Slice(dataOffset);

            var bytes = new byte[data.Length];

            for (int i = 0; i < data.Length; i++)
            {
                bytes[i] = (byte)(data[i] ^ maskingKey[i % 4]);
            }

            return Encoding.UTF8.GetString(bytes);
        }
    }
}
