extern crate regex;
extern crate sha1;
extern crate base64;

use std::io::{Read, Write};
use std::net::{TcpStream, TcpListener, SocketAddrV4, Ipv4Addr};
use std::str;
use regex::Regex;
use sha1::{Sha1, Digest};

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

const WS_GUID: &str = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

fn on_get_request_received(mut stream: &TcpStream, data: &str) {
    let r = Regex::new("Sec-WebSocket-Key: (.*)").unwrap();
    let captures = r.captures(data).unwrap();
    if captures.len() < 2 {
        return;
    }

    let m = captures.get(1).unwrap();
    let key = m.as_str().trim();
    let mut new_key = key.to_owned();
    new_key.push_str(WS_GUID);

    let result = Sha1::digest(new_key.as_bytes());
    let sha1_binary: &[u8] = &result;
    let sha1_base64 = base64::encode(sha1_binary);

    let header = concat!(
        "HTTP/1.1 101 Switching Protocols\r\n",
        "Connection: Upgrade\r\n",
        "Upgrade: websocket\r\n"
    );

    let mut send_data = header.to_owned();
    send_data.push_str(&format!("Sec-WebSocket-Accept: {}\r\n", sha1_base64));
    send_data.push_str("\r\n");
    println!("{}", send_data);

    stream.write_all(send_data.as_bytes()).unwrap();
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
                    self.on_connected(&mut s);
                },
                Err(e) => {
                    println!("{}", e);
                }
            }
        }
    }

    fn on_connected(&mut self, stream: &mut TcpStream) {
        (self.client_connect_handler)(stream);

        loop {
            let mut buf = [0; 1024];
            let size = stream.read(&mut buf).unwrap();
            if size == 0 {
                break;
            }
            let data = str::from_utf8(&mut buf[0..size]).unwrap();
            if data.starts_with("GET /") {
                on_get_request_received(stream, data);
            }
        }
    }
}
