#pragma once

#include "../common/Types.hpp"
#include "../common/Macros.hpp"

WEBSOCKET_NAMESPACE_BEGIN

namespace FrameOpcodes {
    // RFC 6455 Section 5.2 - Base Framing Protocol
    // and Section 11.8 - Opcode Registry

    enum Opcode : uint8_t {
        // Standard data frame opcodes
        CONTINUATION = 0x0,  // %x0 denotes a continuation frame
        TEXT = 0x1,  // %x1 denotes a text frame
        BINARY = 0x2,  // %x2 denotes a binary frame

        // Standard control frame opcodes
        CLOSE = 0x8,  // %x8 denotes a connection close
        PING = 0x9,  // %x9 denotes a ping
        PONG = 0xA,  // %xA denotes a pong

        // Reserved non-control frame opcodes
        RESERVED_3 = 0x3,  // %x3-7 are reserved for further non-control frames
        RESERVED_4 = 0x4,
        RESERVED_5 = 0x5,
        RESERVED_6 = 0x6,
        RESERVED_7 = 0x7,

        // Reserved control frame opcodes
        RESERVED_B = 0xB,  // %xB-F are reserved for further control frames
        RESERVED_C = 0xC,
        RESERVED_D = 0xD,
        RESERVED_E = 0xE,
        RESERVED_F = 0xF
    };

    // Validation functions
    constexpr bool isControlFrame(Opcode opcode) {
        return (opcode & 0x8) != 0;
    }

    constexpr bool isDataFrame(Opcode opcode) {
        return !isControlFrame(opcode) && opcode <= BINARY;
    }

    constexpr bool isReserved(Opcode opcode) {
        return (opcode >= RESERVED_3 && opcode <= RESERVED_7) ||
            (opcode >= RESERVED_B && opcode <= RESERVED_F);
    }

    constexpr bool isValidOpcode(Opcode opcode) {
        return opcode <= RESERVED_F;
    }

    constexpr bool isKnownOpcode(Opcode opcode) {
        return opcode <= PONG || (opcode >= RESERVED_3 && opcode <= RESERVED_7);
    }

    // String representation for logging/debugging
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
        default:           return "UNKNOWN";
        }
    }
}

WEBSOCKET_NAMESPACE_END