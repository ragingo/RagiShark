use std::net::{TcpStream, TcpListener, SocketAddrV4, Ipv4Addr};

pub struct WebSocketHeader {
    fin: bool,
    rcv1: u8,
    rcv2: u8,
    rcv3: u8,
    opcode: u8,
    mask: bool,
    payload_length: u8
}

pub fn websocket_header_parse(header: &mut WebSocketHeader, data: &[u8])
{
    if data.len() < 2 {
        return;
    }

    let b0 = data[0];
    let b1 = data[1];
    header.fin = (b0 & 0x80) == 0x80;
    header.rcv1 = if b0 & 0x40 == 0x40 { 1 } else { 0 };
    header.rcv2 = if b0 & 0x20 == 0x20 { 1 } else { 0 };
    header.rcv3 = if b0 & 0x10 == 0x10 { 1 } else { 0 };
    header.opcode = b0 & 0x0f;
    header.mask = (b1 & 0x80) == 0x80;
    header.payload_length = b1 & 0x7f;
}

pub fn websocket_header_create(header: &mut Vec<u8>, fin: bool, opcode: u8, len: i32)
{
    let fin_val: u8 = if fin { 1 } else { 0 };

    if len <= 125 {
        header.resize(2, 0);
        header[0] = (fin_val << 7) | opcode;
        header[1] = len as u8;
    }
    else if len <= 0xffff {
        header.resize(4, 0);
        header[0] = (fin_val << 7) | opcode;
        header[1] = 126;
        header[2] = ((len & 0xff00) >> 8) as u8;
        header[3] = (len & 0x00ff) as u8;
    }
    else {
        header.resize(10, 0);
        header[0] = (fin_val << 7) | opcode;
        header[1] = 127;
        header[2] = (len >> 56) as u8;
        header[3] = ((len >> 48) & 0xff) as u8;
        header[4] = ((len >> 40) & 0xff) as u8;
        header[5] = ((len >> 32) & 0xff) as u8;
        header[6] = ((len >> 24) & 0xff) as u8;
        header[7] = ((len >> 16) & 0xff) as u8;
        header[8] = ((len >> 8) & 0xff) as u8;
        header[9] = ((len >> 0) & 0xff) as u8;
    }
}

pub struct WebSocketServer {
    pub client_connect_handler: fn(stream: &mut TcpStream) -> ()
}

impl WebSocketServer {
    pub fn new() -> WebSocketServer {
        WebSocketServer {
            client_connect_handler: |_| ()
        }
    }
    pub fn listen(&mut self, port: u16) {
        let addr = SocketAddrV4::new(Ipv4Addr::LOCALHOST, port);
        let listener = TcpListener::bind(addr).unwrap();

        for stream in listener.incoming() {
            match stream {
                Ok(mut s) => {
                    (self.client_connect_handler)(&mut s);
                },
                Err(e) => {
                    println!("{}", e);
                }
            }
        }
    }
}
