#include "constants/FrameOpcodes.hpp"

WEBSOCKET_NAMESPACE_BEGIN

namespace FrameOpcodes {

    // Frame opcode string representations
    const char* toString(Opcode opcode) {
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

} // namespace FrameOpcodes

WEBSOCKET_NAMESPACE_END