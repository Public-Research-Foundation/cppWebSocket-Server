#pragma once
#ifndef WEBSOCKET_FRAME_HPP
#define WEBSOCKET_FRAME_HPP

#include "../common/Types.hpp"
#include "../constants/FrameOpcodes.hpp"
#include "../constants/Limits.hpp"
#include <cstdint>
#include <vector>
#include <string>

WEBSOCKET_NAMESPACE_BEGIN

/**
 * @class WebSocketFrame
 * @brief Represents a WebSocket frame according to RFC 6455
 *
 * This class handles the complete frame structure:
 *
 *  0                   1                   2                   3
 *  0 1 2 3 4 5 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-------+-+-------------+-------------------------------+
 * |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
 * |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
 * |N|V|V|V|       |S|             |   (if payload len==126/127)   |
 * | |1|2|3|       |K|             |                               |
 * +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
 * |     Extended payload length continued, if payload len == 127  |
 * + - - - - - - - - - - - - - - - +-------------------------------+
 * |                               |Masking-key, if MASK set to 1  |
 * +-------------------------------+-------------------------------+
 * | Masking-key (continued)       |          Payload Data         |
 * +-------------------------------- - - - - - - - - - - - - - - - +
 * :                     Payload Data continued ...                :
 * + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
 * |                     Payload Data continued ...                |
 * +---------------------------------------------------------------+
 */
    class WebSocketFrame {
    public:
        /**
         * @brief Default constructor
         */
        WebSocketFrame() = default;

        /**
         * @brief Construct a frame with specific parameters
         * @param opcode Frame opcode
         * @param payload Frame payload data
         * @param fin FIN flag (true for final frame in message)
         * @param masked Whether payload is masked
         */
        WebSocketFrame(Opcode opcode, const Buffer& payload, bool fin = true, bool masked = false);

        /**
         * @brief Parse raw data into WebSocket frame
         * @param data Raw byte data to parse
         * @param frame Output frame structure
         * @return Number of bytes consumed, or 0 if incomplete frame
         * @throws ProtocolError on invalid frame data
         */
        static size_t parse(const Buffer& data, WebSocketFrame& frame);

        /**
         * @brief Serialize frame into raw bytes
         * @return Serialized frame data
         */
        Buffer serialize() const;

        /**
         * @brief Validate frame according to RFC 6455
         * @return true if frame is valid, false otherwise
         */
        bool validate() const;

        /**
         * @brief Get frame size including headers
         * @return Total frame size in bytes
         */
        size_t getTotalSize() const;

        /**
         * @brief Apply masking to payload (if masked)
         */
        void applyMasking();

        /**
         * @brief Remove masking from payload (if masked)
         */
        void removeMasking();

        /**
         * @brief Check if frame is a control frame
         * @return true if control frame (close, ping, pong)
         */
        bool isControlFrame() const;

        /**
         * @brief Check if frame is a data frame
         * @return true if data frame (text, binary, continuation)
         */
        bool isDataFrame() const;

        // Getters and setters
        bool getFin() const { return fin_; }
        void setFin(bool fin) { fin_ = fin; }

        Opcode getOpcode() const { return opcode_; }
        void setOpcode(Opcode opcode) { opcode_ = opcode; }

        bool getMasked() const { return masked_; }
        void setMasked(bool masked) { masked_ = masked; }

        const Buffer& getPayload() const { return payload_; }
        void setPayload(const Buffer& payload) { payload_ = payload; }

        uint32_t getMaskingKey() const { return masking_key_; }
        void setMaskingKey(uint32_t key) { masking_key_ = key; }

        size_t getPayloadLength() const { return payload_.size(); }

    private:
        /**
         * @brief Parse the basic frame header (first 2 bytes)
         * @param data Input data buffer
         * @param offset Current offset in buffer
         * @return Number of bytes consumed
         */
        size_t parseHeader(const Buffer& data, size_t offset);

        /**
         * @brief Parse extended payload length (if needed)
         * @param data Input data buffer
         * @param offset Current offset in buffer
         * @return Number of bytes consumed
         */
        size_t parseExtendedLength(const Buffer& data, size_t offset);

        /**
         * @brief Parse masking key (if present)
         * @param data Input data buffer
         * @param offset Current offset in buffer
         * @return Number of bytes consumed
         */
        size_t parseMaskingKey(const Buffer& data, size_t offset);

        /**
         * @brief Parse payload data
         * @param data Input data buffer
         * @param offset Current offset in buffer
         * @return Number of bytes consumed
         */
        size_t parsePayload(const Buffer& data, size_t offset);

        /**
         * @brief Calculate header size based on payload length and flags
         * @return Header size in bytes
         */
        size_t calculateHeaderSize() const;

        /**
         * @brief Write basic header to output buffer
         * @param output Output buffer to write to
         */
        void writeHeader(Buffer& output) const;

        /**
         * @brief Write extended length to output buffer
         * @param output Output buffer to write to
         */
        void writeExtendedLength(Buffer& output) const;

        /**
         * @brief Write masking key to output buffer
         * @param output Output buffer to write to
         */
        void writeMaskingKey(Buffer& output) const;

        // Frame components
        bool fin_{ true };                ///< FIN bit (1 = final frame in message)
        bool rsv1_{ false };              ///< RSV1 bit (for extensions)
        bool rsv2_{ false };              ///< RSV2 bit (for extensions)
        bool rsv3_{ false };              ///< RSV3 bit (for extensions)
        Opcode opcode_{ Opcode::TEXT };   ///< Frame opcode
        bool masked_{ false };            ///< MASK bit (1 = payload is masked)
        uint64_t payload_length_{ 0 };    ///< Payload length
        uint32_t masking_key_{ 0 };       ///< Masking key (if masked)
        Buffer payload_;                ///< Payload data
};

WEBSOCKET_NAMESPACE_END

#endif // WEBSOCKET_FRAME_HPP