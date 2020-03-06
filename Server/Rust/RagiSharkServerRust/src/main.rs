extern crate regex;

use std::io::Read;
use std::net::{Ipv4Addr, SocketAddrV4, TcpListener};
use std::str;
use regex::Regex;

fn main() {
    let addr = SocketAddrV4::new(Ipv4Addr::LOCALHOST, 8080);
    let listener = TcpListener::bind(addr).unwrap();

    for stream in listener.incoming() {
        match stream {
            Ok(mut stream) => {
                loop {
                    let mut buf = [0; 1024];
                    let size = stream.read(&mut buf).unwrap();
                    if size == 0 {
                        break;
                    }
                    let data = str::from_utf8(&mut buf[0..size]).unwrap();
                    if data.starts_with("GET /") {
                        let r = Regex::new("Sec-WebSocket-Key: (.*)").unwrap();
                        let captures = r.captures(data).unwrap();
                        if captures.len() < 2 {
                            break;
                        }
                        let m = captures.get(1).unwrap();
                        println!("{}", m.as_str()); // ok
                    }
                }
            },
            Err(e) => {
                println!("{}", e);
            }
        }
    }
}
