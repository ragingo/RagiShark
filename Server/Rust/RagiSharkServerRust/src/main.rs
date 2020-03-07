extern crate regex;
extern crate sha1;
extern crate base64;

use std::io::{Read, Write};
use std::net::{Ipv4Addr, SocketAddrV4, TcpListener, TcpStream};
use std::str;
use regex::Regex;
use sha1::{Sha1, Digest};

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

fn on_received(mut stream: &TcpStream) {
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

fn main() {
    let addr = SocketAddrV4::new(Ipv4Addr::LOCALHOST, 8080);
    let listener = TcpListener::bind(addr).unwrap();

    for stream in listener.incoming() {
        match stream {
            Ok(s) => {
                on_received(&s);
            },
            Err(e) => {
                println!("{}", e);
            }
        }
    }
}
