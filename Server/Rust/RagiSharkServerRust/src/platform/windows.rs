#[link(name = "user32")]
#[allow(non_snake_case)]
#[allow(dead_code)]
extern {
    pub fn MessageBoxA(handle: i32, text: *const u8, caption: *const u8, utype: u32) -> i32;
}

#[link(name = "kernel32")]
#[allow(non_snake_case)]
#[allow(dead_code)]
extern {
    pub fn SetConsoleOutputCP(codepage: i32) -> i32;
}
