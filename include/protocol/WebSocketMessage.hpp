#pragma once
#ifndef WEBSOCKET_MESSAGE_HPP
#define WEBSOCKET_MESSAGE_HPP

#include "../common/Types.hpp"
#include "WebSocketFrame.hpp"
#include <vector>
#include <memory>

WEBSOCKET_NAMESPACE_BEGIN

/**
 * @class WebSocketMessage
 * @brief Represents a complete WebSocket message composed of one or more frames
 *
 * A WebSocket message can be fragmented across multiple frames:
 * - First frame: FIN=0, opcode=TEXT/BINARY
 * - Middle frames: FIN=0, opcode=CONTINUATION
 * - Final frame: FIN=1, opcode=CONTINUATION
 *
 * This class handles message reassembly from frames and fragmentation into frames.
 */
    class WebSocketMessage {
    public:
        /**
         * @brief Message types
         */
        enum class Type {
            TEXT,       ///< UTF-8 text message
            BINARY      ///< Binary data message
        };

        /**
         * @brief Default constructor
         */
        WebSocketMessage() = default;

        /**
         * @brief Construct a message with data and type
         * @param data Message payload data
         * @param type Message type (TEXT or BINARY)
         */
        WebSocketMessage(const Buffer& data, Type type = Type::TEXT);

        /**
         * @brief Construct a text message from string
         * @param text UTF-8 text data
         */
        explicit WebSocketMessage(const std::string& text);

        /**
         * @brief Add a frame to the message (for reassembly)
         * @param frame Frame to add
         * @return true if frame was added successfully, false if invalid
         */
        bool addFrame(const WebSocketFrame& frame);

        /**
         * @brief Check if message is complete (all frames received)
         * @return true if message is complete
         */
        bool isComplete() const;

        /**
         * @brief Get the complete message data
         * @return Concatenated payload from all frames
         */
        const Buffer& getData() const;

        /**
         * @brief Get message as UTF-8 string
         * @return String representation (for TEXT messages)
         * @throws std::runtime_error if message is not valid UTF-8
         */
        std::string getText() const;

        /**
         * @brief Get message type
         * @return Message type (TEXT or BINARY)
         */
        Type getType() const;

        /**
         * @brief Get number of frames in this message
         * @return Frame count
         */
        size_t getFrameCount() const;

        /**
         * @brief Get total message size in bytes
         * @return Size of complete message
         */
        size_t getSize() const;

        /**
         * @brief Split message into frames for transmission
         * @param max_frame_size Maximum size for each frame
         * @return Vector of frames representing this message
         */
        std::vector<WebSocketFrame> toFrames(size_t max_frame_size = MAX_FRAME_SIZE) const;

        /**
         * @brief Reset message for reuse
         */
        void clear();

        /**
         * @brief Check if message is valid (proper UTF-8 for text messages)
         * @return true if message is valid
         */
        bool isValid() const;

    private:
        /**
         * @brief Validate UTF-8 data for text messages
         * @return true if data is valid UTF-8
         */
        bool validateUtf8() const;

        /**
         * @brief Process the first frame of a message
         * @param frame First frame
         * @return true if frame is valid as first frame
         */
        bool processFirstFrame(const WebSocketFrame& frame);

        /**
         * @brief Process a continuation frame
         * @param frame Continuation frame
         * @return true if frame is valid continuation
         */
        bool processContinuationFrame(const WebSocketFrame& frame);

        /**
         * @brief Process a control frame (handled separately)
         * @param frame Control frame
         * @return true if frame was processed
         */
        bool processControlFrame(const WebSocketFrame& frame);

        // Message data
        Type type_{ Type::TEXT };                 ///< Message type
        Buffer data_;                           ///< Complete message data
        std::vector<WebSocketFrame> frames_;    ///< Individual frames (for tracking)
        bool complete_{ false };                  ///< Whether message is complete
        Opcode initial_opcode_{ Opcode::TEXT };   ///< Opcode of first frame
};

WEBSOCKET_NAMESPACE_END

#endif // WEBSOCKET_MESSAGE_HPP