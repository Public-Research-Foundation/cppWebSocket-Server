#pragma once
#ifndef WEBSOCKET_FRAME_OPCODES_HPP
#define WEBSOCKET_FRAME_OPCODES_HPP

#include "../common/Types.hpp"
#include "../common/Macros.hpp"

WEBSOCKET_NAMESPACE_BEGIN

/**
 * @namespace FrameOpcodes
 * @brief WebSocket frame opcode definitions and validation (RFC 6455 Section 5.2)
 *
 * Opcodes define the type and purpose of WebSocket frames:
 * - Data frames: TEXT, BINARY, CONTINUATION
 * - Control frames: CLOSE, PING, PONG
 * - Reserved frames: For future protocol extensions
 */
    namespace FrameOpcodes {
    // ============================================================================
    // FRAME OPCODE DEFINITIONS (RFC 6455 Section 5.2)
    // ============================================================================

    /**
     * @enum Opcode
     * @brief WebSocket frame opcodes defining frame type and purpose
     */
    enum Opcode : uint8_t {
        // ==========================================
        // DATA FRAME OPCODES
        // ==========================================

        /**
         * @brief Continuation frame (0x0)
         *
         * Used for fragmented messages when FIN bit is 0 in previous frame.
         * Follows a TEXT or BINARY frame with FIN=0.
         */
        CONTINUATION = 0x0,

        /**
         * @brief Text frame (0x1)
         *
         * Contains UTF-8 encoded text data. Payload must be valid UTF-8.
         */
        TEXT = 0x1,

        /**
         * @brief Binary frame (0x2)
         *
         * Contains arbitrary binary data without encoding restrictions.
         */
        BINARY = 0x2,

        // ==========================================
        // CONTROL FRAME OPCODES (Section 5.5)
        // ==========================================

        /**
         * @brief Connection close frame (0x8)
         *
         * Signals connection termination. May contain status code and reason.
         */
        CLOSE = 0x8,

        /**
         * @brief Ping frame (0x9)
         *
         * Heartbeat request. Recipient must respond with PONG containing same data.
         */
        PING = 0x9,

        /**
         * @brief Pong frame (0xA)
         *
         * Heartbeat response to PING frame. Echoes PING payload data.
         */
        PONG = 0xA,

        // ==========================================
        // RESERVED NON-CONTROL FRAME OPCODES
        // ==========================================

        /**
         * @brief Reserved for future non-control frames (0x3)
         */
        RESERVED_3 = 0x3,

        /**
         * @brief Reserved for future non-control frames (0x4)
         */
        RESERVED_4 = 0x4,

        /**
         * @brief Reserved for future non-control frames (0x5)
         */
        RESERVED_5 = 0x5,

        /**
         * @brief Reserved for future non-control frames (0x6)
         */
        RESERVED_6 = 0x6,

        /**
         * @brief Reserved for future non-control frames (0x7)
         */
        RESERVED_7 = 0x7,

        // ==========================================
        // RESERVED CONTROL FRAME OPCODES
        // ==========================================

        /**
         * @brief Reserved for future control frames (0xB)
         */
        RESERVED_B = 0xB,

        /**
         * @brief Reserved for future control frames (0xC)
         */
        RESERVED_C = 0xC,

        /**
         * @brief Reserved for future control frames (0xD)
         */
        RESERVED_D = 0xD,

        /**
         * @brief Reserved for future control frames (0xE)
         */
        RESERVED_E = 0xE,

        /**
         * @brief Reserved for future control frames (0xF)
         */
        RESERVED_F = 0xF
    };

    // ============================================================================
    // OPCODE VALIDATION FUNCTIONS
    // ============================================================================

    /**
     * @brief Check if opcode represents a control frame
     * @param opcode The opcode to check
     * @return true if opcode is a control frame (CLOSE, PING, PONG, or reserved control)
     *
     * @note Control frames are identified by the most significant bit of opcode being set
     */
    constexpr bool isControlFrame(Opcode opcode) {
        return (opcode & 0x8) != 0;
    }

    /**
     * @brief Check if opcode represents a data frame
     * @param opcode The opcode to check
     * @return true if opcode is TEXT, BINARY, or CONTINUATION
     */
    constexpr bool isDataFrame(Opcode opcode) {
        return !isControlFrame(opcode) && opcode <= BINARY;
    }

    /**
     * @brief Check if opcode is reserved for future use
     * @param opcode The opcode to check
     * @return true if opcode is in reserved range
     */
    constexpr bool isReserved(Opcode opcode) {
        return (opcode >= RESERVED_3 && opcode <= RESERVED_7) ||
            (opcode >= RESERVED_B && opcode <= RESERVED_F);
    }

    /**
     * @brief Check if opcode is a valid WebSocket opcode
     * @param opcode The opcode to check
     * @return true if opcode is within valid range (0x0-0xF)
     */
    constexpr bool isValidOpcode(Opcode opcode) {
        return opcode <= RESERVED_F;
    }

    /**
     * @brief Check if opcode is known (not reserved)
     * @param opcode The opcode to check
     * @return true if opcode is defined in RFC 6455
     */
    constexpr bool isKnownOpcode(Opcode opcode) {
        return opcode <= PONG || (opcode >= RESERVED_3 && opcode <= RESERVED_7);
    }

    /**
     * @brief Check if opcode can initiate a message
     * @param opcode The opcode to check
     * @return true if opcode can start a new message (TEXT or BINARY)
     */
    constexpr bool isMessageInitiator(Opcode opcode) {
        return opcode == TEXT || opcode == BINARY;
    }

    // ============================================================================
    // STRING REPRESENTATION
    // ============================================================================

    /**
     * @brief Convert opcode to human-readable string for logging
     * @param opcode The opcode to convert
     * @return String representation of the opcode
     */
    inline const char* toString(Opcode opcode) {
        switch (opcode) {
        case CONTINUATION: return "CONTINUATION";
        case TEXT:         return "TEXT";
        case BINARY:       return "BINARY";
        case CLOSE:        return "CLOSE";
        case PING:         return "PING";
        case PONG:         return "PONG";
        case RESERVED_3:   return "RESERVED_3";
        case RESERVED_4:   return "RESERVED_4";
        case RESERVED_5:   return "RESERVED_5";
        case RESERVED_6:   return "RESERVED_6";
        case RESERVED_7:   return "RESERVED_7";
        case RESERVED_B:   return "RESERVED_B";
        case RESERVED_C:   return "RESERVED_C";
        case RESERVED_D:   return "RESERVED_D";
        case RESERVED_E:   return "RESERVED_E";
        case RESERVED_F:   return "RESERVED_F";
        default:           return "UNKNOWN_OPCODE";
        }
    }
}

WEBSOCKET_NAMESPACE_END

#endif // WEBSOCKET_FRAME_OPCODES_HPP