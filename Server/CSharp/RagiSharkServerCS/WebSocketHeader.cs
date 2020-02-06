namespace RagiSharkServerCS
{
    struct WebSocketHeader
    {
        public bool Fin;
        public byte Rcv1;
        public byte Rcv2;
        public byte Rcv3;
        public OpCode OpCode;
        public bool Mask;
        public int PayloadLength;

        public static WebSocketHeader Parse(byte[] bytes)
        {
            var header = new WebSocketHeader();
            if (bytes == null || bytes.Length < 2)
            {
                return header;
            }

            header.Fin = (bytes[0] >> 7) == 1;
            header.Rcv1 = (byte)(bytes[0] >> 6 & 1);
            header.Rcv2 = (byte)(bytes[0] >> 5 & 1);
            header.Rcv3 = (byte)(bytes[0] >> 4 & 1);
            header.OpCode = (OpCode)(bytes[0] & 0x0f);
            header.Mask = (bytes[1] >> 7) == 1;
            header.PayloadLength = bytes[1] & 0x7f;
            return header;
        }

        public static WebSocketHeader Create(bool fin, OpCode opCode, int payloadLength)
        {
            var header = new WebSocketHeader();
            header.Fin = fin;
            header.Rcv1 = 0;
            header.Rcv2 = 0;
            header.Rcv3 = 0;
            header.OpCode = opCode;
            header.Mask = false;
            header.PayloadLength = payloadLength;
            return header;
        }

        public byte[] ToBinary()
        {
            if (PayloadLength <= 125)
            {
                var bytes = new byte[2];
                bytes[0] = (byte)(((Fin ? 1 : 0) << 7) | (byte)OpCode);
                bytes[1] = (byte)PayloadLength;
                return bytes;
            }
            else if (PayloadLength <= 0xffff)
            {
                short len = (short)PayloadLength;
                var bytes = new byte[2 + 2];
                bytes[0] = (byte)(((Fin ? 1 : 0) << 7) | (byte)OpCode);
                bytes[1] = 126;
                bytes[2] = (byte)((len & 0xff00) >> 8);
                bytes[3] = (byte)(len & 0x00ff);
                return bytes;
            }
            else
            {
                long len = PayloadLength;
                var bytes = new byte[2 + 8];
                bytes[0] = (byte)(((Fin ? 1 : 0) << 7) | (byte)OpCode);
                bytes[1] = 127;
                bytes[2] = (byte)(len >> 56);
                bytes[3] = (byte)((len >> 48) & 0xff);
                bytes[4] = (byte)((len >> 40) & 0xff);
                bytes[5] = (byte)((len >> 32) & 0xff);
                bytes[6] = (byte)((len >> 24) & 0xff);
                bytes[7] = (byte)((len >> 16) & 0xff);
                bytes[8] = (byte)((len >> 8) & 0xff);
                bytes[9] = (byte)((len >> 0) & 0xff);
                return bytes;
            }
        }

        public override string ToString()
        {
            return $"fin: {Fin}, rcv1: {Rcv1}, rcv2: {Rcv2}, rcv3: {Rcv3}, opcode: {OpCode}, mask: {Mask}, payloadLength: {PayloadLength}";
        }
    }
}
