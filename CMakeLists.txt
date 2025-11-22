# CMakeList.txt : CMake project for cppWebSocket-Server, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("cppWebSocket-Server")

# Add source to this project's executable.
add_executable (cppWebSocket-Server "cppWebSocket-Server.cpp" "cppWebSocket-Server.h"  "src/common/GlobalConfig.cpp" "src/common/Types.cpp" "src/common/Version.cpp" "src/common/Platform.cpp" "src/common/ErrorHandling.cpp" "src/config/ConfigManager.cpp" "src/config/ConfigParser.cpp" "src/config/RuntimeConfig.cpp" "src/config/ConfigValidator.cpp" "src/constants/WebSocketConstants.cpp" "src/constants/FrameOpcodes.cpp" "src/constants/StatusCodes.cpp" "src/constants/Limits.cpp" "src/constants/ProtocolConstants.cpp" "src/constants/ErrorCodes.cpp" "src/core/Engine.cpp" "src/core/ComponentManager.cpp" "src/core/interfaces/IEngine.cpp" "src/core/interfaces/IComponent.cpp" "src/core/interfaces/IConfigurable.cpp" "src/core/interfaces/IInitializable.cpp" "src/core/interfaces/IShutdownHandler.cpp" "src/core/interfaces/ServiceBase.cpp" "src/core/interfaces/ComponentBase.cpp" "src/main/CommandLineParser.cpp" "src/main/main.cpp" "src/monitoring/HealthCheck.cpp" "src/monitoring/PerformanceMonitor.cpp" "src/main/StatsCollector.cpp" "src/main/AlertManager.cpp" "src/monitoring/AlertManager.cpp" "src/network/ServerBase.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET cppWebSocket-Server PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
