#include "common/common.h"

#ifdef RAGII_MAC

#include "util/util.h"
#include <iostream>
#include <CommonCrypto/CommonCrypto.h>

namespace ragii::util
{
    std::string createSha1Hash(std::string_view text)
    {
        CC_SHA1_CTX ctx;
        CC_SHA1_Init(&ctx);
        CC_SHA1_Update(&ctx, text.data(), text.length());

        std::vector<uint8_t> buf(CC_SHA1_DIGEST_LENGTH + 1, 0);
        CC_SHA1_Final(buf.data(), &ctx);

        std::string hash(reinterpret_cast<char*>(buf.data()));
        return hash;
    }

    void outputSystemLastError()
    {
        std::cout << strerror(errno) << std::endl;
    }
}

#endif
