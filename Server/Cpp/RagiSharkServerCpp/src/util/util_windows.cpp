#if defined(_WINDOWS) || defined(_WIN32) || defined(_WIN64)

#include "util.h"
#include <vector>

#define _WIN32_WINNT 0x0A00
#include <sdkddkver.h>

#define NOMINMAX
#include <Windows.h>

#include <wincrypt.h>
#pragma comment(lib, "crypt32")

std::string createSha1Hash(std::string text)
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

    ret = CryptHashData(hash, reinterpret_cast<const uint8_t*>(text.c_str()), text.size(), 0);
    if (!ret) {
        dispose();
        return "";
    }

    DWORD len = 0;
    ret = CryptGetHashParam(hash, HP_HASHVAL, nullptr, &len, 0);
    if (!ret) {
        dispose();
        return "";
    }

    std::vector<uint8_t> hash_value;
    hash_value.resize(len);
    ret = CryptGetHashParam(hash, HP_HASHVAL, hash_value.data(), &len, 0);
    if (!ret) {
        dispose();
        return "";
    }

    DWORD len2 = 1024;
    char buf2[1024] = {0};
    ret = CryptBinaryToString(hash_value.data(), hash_value.size(), CRYPT_STRING_BASE64, buf2, &len2);
    if (!ret) {
        dispose();
        return "";
    }

    dispose();
    return std::string(buf2);
}

#endif
