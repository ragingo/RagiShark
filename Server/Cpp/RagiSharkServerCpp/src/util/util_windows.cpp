#include "util.h"

#ifdef RAGII_WINDOWS

#include <iostream>
#include <wincrypt.h>
#pragma comment(lib, "crypt32")

std::string createSha1Hash(std::string_view text)
{
    HCRYPTPROV prov = 0;
    HCRYPTHASH hash = 0;

    auto dispose = [&]()-> void {
        if (hash) {
            CryptDestroyHash(hash);
        }
        if (prov) {
            CryptReleaseContext(prov, 0);
        }
    };

    int ret = CryptAcquireContext(&prov, 0, 0, PROV_RSA_FULL, 0);
    if (!ret) {
        dispose();
        return "";
    }

    ret = CryptCreateHash(prov, CALG_SHA1, 0, 0, &hash);
    if (!ret) {
        dispose();
        return "";
    }

    ret = CryptHashData(hash, reinterpret_cast<const uint8_t*>(text.data()), text.size(), 0);
    if (!ret) {
        dispose();
        return "";
    }

    DWORD hash_len = 0;
    ret = CryptGetHashParam(hash, HP_HASHVAL, nullptr, &hash_len, 0);
    if (!ret) {
        dispose();
        return "";
    }

    std::vector<uint8_t> hash_value;
    hash_value.resize(hash_len);
    ret = CryptGetHashParam(hash, HP_HASHVAL, hash_value.data(), &hash_len, 0);
    if (!ret) {
        dispose();
        return "";
    }

    DWORD base64_len = 0;
    ret = CryptBinaryToString(hash_value.data(), hash_value.size(), CRYPT_STRING_BASE64, nullptr, &base64_len);
    if (!ret) {
        dispose();
        return "";
    }

    std::vector<char> base64;
    base64.resize(base64_len);
    ret = CryptBinaryToString(hash_value.data(), hash_value.size(), CRYPT_STRING_BASE64, base64.data(), &base64_len);
    if (!ret) {
        dispose();
        return "";
    }

    dispose();
    return std::string(base64.data());
}

void createProcess(std::string_view app, std::vector<std::string_view> args)
{
    args.insert(args.begin(), app);
    auto commadline = ragii::join(" ", args);
    STARTUPINFO si = {};
    si.wShowWindow = SW_HIDE;
    PROCESS_INFORMATION pi = {};

    BOOL ret = CreateProcess(
        nullptr,
        const_cast<char*>(commadline.c_str()),
        nullptr,
        nullptr,
        FALSE,
        CREATE_NO_WINDOW,
        nullptr,
        nullptr,
        &si,
        &pi
    );

    if (!ret) {
        outputSystemLastError();
        return;
    }
}

void outputSystemLastError()
{
    int err = GetLastError();
    const int BUF_SIZE = 1024;
    wchar_t buf[BUF_SIZE] = {0};
    FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, err, 0, buf, BUF_SIZE, nullptr);
    std::wcout << buf << std::endl;
}

#endif
