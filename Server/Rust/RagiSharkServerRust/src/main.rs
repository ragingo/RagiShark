mod platform;
mod net;

use crate::net::websocket::WebSocketServer;
use std::net::{TcpStream};
use std::process::Command;
use std::str;

fn on_connected(mut stream: &TcpStream) {
    // let mut s = stream;
    // let _handle = std::thread::spawn(move || {
    //     loop {
    //         std::thread::sleep(std::time::Duration::from_millis(500));
    //         let mut buf: [u8;10];
    //         s.write_all(&buf);
    //     }
    // });
}

fn test() {
    if cfg!(windows) {
        unsafe {
            platform::windows::SetConsoleOutputCP(65001);
            platform::windows::MessageBoxA(0, "text\0".as_ptr(), "caption\0".as_ptr(), 0);
        };
    }

    let output = Command::new("tshark").arg("-D").output().unwrap();
    println!("{}", str::from_utf8(&output.stdout).unwrap());
}

fn main() {
    // test();

    let mut ws = WebSocketServer::new();
    ws.client_connect_handler = |stream: &mut TcpStream| {
        on_connected(stream);
    };
    ws.listen(8080);
}
