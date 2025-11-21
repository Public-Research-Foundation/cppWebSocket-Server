#pragma once

#include "../common/Types.hpp"
#include <string>
#include <vector>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/hmac.h>

WEBSOCKET_NAMESPACE_BEGIN

class CryptoUtils {
public:
    static CryptoUtils& getInstance();

    // Hashing functions
    static ByteBuffer sha1(const ByteBuffer& data);
    static ByteBuffer sha256(const ByteBuffer& data);
    static std::string sha1Hex(const std::string& data);
    static std::string sha256Hex(const std::string& data);

    // HMAC functions
    static ByteBuffer hmacSha1(const ByteBuffer& data, const ByteBuffer& key);
    static ByteBuffer hmacSha256(const ByteBuffer& data, const ByteBuffer& key);

    // Base64 encoding/decoding
    static std::string base64Encode(const ByteBuffer& data);
    static ByteBuffer base64Decode(const std::string& encoded);

    // WebSocket specific
    static std::string generateWebSocketAcceptKey(const std::string& clientKey);
    static bool verifyWebSocketKey(const std::string& clientKey, const std::string& serverAccept);

    // Random generation
    static ByteBuffer generateRandomBytes(size_t length);
    static std::string generateRandomString(size_t length);

    // Masking (WebSocket frame masking)
    static void applyMask(ByteBuffer& data, const ByteBuffer& mask);
    static void applyMask(Byte* data, size_t length, const Byte* mask);

    // Utility functions
    static std::string bytesToHex(const ByteBuffer& data);
    static ByteBuffer hexToBytes(const std::string& hex);

private:
    CryptoUtils() = default;
    ~CryptoUtils() = default;

    static void initializeOpenSSL();
    static void cleanupOpenSSL();
};

WEBSOCKET_NAMESPACE_END