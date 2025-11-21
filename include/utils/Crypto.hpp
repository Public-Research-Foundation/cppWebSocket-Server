#pragma once

#include "../common/Types.hpp"
#include <string>
#include <vector>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/hmac.h>

WEBSOCKET_NAMESPACE_BEGIN

/**
 * Cryptographic utilities for WebSocket protocol compliance and security
 * Implements RFC 6455 required cryptographic operations
 * Wrapper around OpenSSL with RAII and exception safety
 */
    class CryptoUtils {
    public:
        static CryptoUtils& getInstance();

        // ===== HASHING FUNCTIONS =====
        // For WebSocket handshake and data integrity

        /**
         * Compute SHA-1 hash of data (used in WebSocket handshake)
         * @param data Input data to hash
         * @return SHA-1 hash as byte buffer
         */
        static ByteBuffer sha1(const ByteBuffer& data);

        /**
         * Compute SHA-256 hash of data
         * @param data Input data to hash
         * @return SHA-256 hash as byte buffer
         */
        static ByteBuffer sha256(const ByteBuffer& data);

        /**
         * Compute SHA-1 hash and return as hex string
         * @param data Input string to hash
         * @return SHA-1 hash as hexadecimal string
         */
        static std::string sha1Hex(const std::string& data);

        /**
         * Compute SHA-256 hash and return as hex string
         * @param data Input string to hash
         * @return SHA-256 hash as hexadecimal string
         */
        static std::string sha256Hex(const std::string& data);

        // ===== HMAC FUNCTIONS =====
        // For message authentication

        static ByteBuffer hmacSha1(const ByteBuffer& data, const ByteBuffer& key);
        static ByteBuffer hmacSha256(const ByteBuffer& data, const ByteBuffer& key);

        // ===== BASE64 ENCODING =====
        // For WebSocket handshake and data encoding

        /**
         * Base64 encode binary data
         * @param data Binary data to encode
         * @return Base64 encoded string
         */
        static std::string base64Encode(const ByteBuffer& data);

        /**
         * Base64 decode string to binary data
         * @param encoded Base64 encoded string
         * @return Decoded binary data
         */
        static ByteBuffer base64Decode(const std::string& encoded);

        // ===== WEBSOCKET SPECIFIC =====
        // RFC 6455 required cryptographic operations

        /**
         * Generate WebSocket accept key from client key
         * Implements RFC 6455 Section 1.3
         * @param clientKey Client's Sec-WebSocket-Key
         * @return Sec-WebSocket-Accept value
         */
        static std::string generateWebSocketAcceptKey(const std::string& clientKey);

        /**
         * Verify WebSocket key handshake
         * @param clientKey Client's Sec-WebSocket-Key
         * @param serverAccept Server's Sec-WebSocket-Accept
         * @return true if handshake is valid
         */
        static bool verifyWebSocketKey(const std::string& clientKey, const std::string& serverAccept);

        // ===== RANDOM GENERATION =====
        // For masking keys and nonces

        static ByteBuffer generateRandomBytes(size_t length);
        static std::string generateRandomString(size_t length);

        // ===== FRAME MASKING =====
        // RFC 6455 frame masking operations

        /**
         * Apply XOR mask to WebSocket frame data
         * @param data Data to mask (modified in-place)
         * @param mask 4-byte masking key
         */
        static void applyMask(ByteBuffer& data, const ByteBuffer& mask);

        /**
         * Apply XOR mask to raw byte array
         * @param data Pointer to data
         * @param length Data length
         * @param mask 4-byte masking key
         */
        static void applyMask(Byte* data, size_t length, const Byte* mask);

        // ===== UTILITY FUNCTIONS =====

        static std::string bytesToHex(const ByteBuffer& data);
        static ByteBuffer hexToBytes(const std::string& hex);

    private:
        CryptoUtils() = default;
        ~CryptoUtils() = default;

        static void initializeOpenSSL();
        static void cleanupOpenSSL();
};

WEBSOCKET_NAMESPACE_END