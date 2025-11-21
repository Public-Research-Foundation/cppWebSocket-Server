#pragma once

#include "../common/Types.hpp"
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>

WEBSOCKET_NAMESPACE_BEGIN

class FileUtils {
public:
    // File existence and properties
    static bool exists(const std::string& path);
    static bool isFile(const std::string& path);
    static bool isDirectory(const std::string& path);
    static bool isReadable(const std::string& path);
    static bool isWritable(const std::string& path);
    static bool isExecutable(const std::string& path);

    // File operations
    static bool createFile(const std::string& path);
    static bool createDirectory(const std::string& path);
    static bool createDirectories(const std::string& path);
    static bool removeFile(const std::string& path);
    static bool removeDirectory(const std::string& path);
    static bool copyFile(const std::string& source, const std::string& destination);
    static bool moveFile(const std::string& source, const std::string& destination);
    static bool renameFile(const std::string& oldPath, const std::string& newPath);

    // File information
    static size_t getFileSize(const std::string& path);
    static std::string getFileName(const std::string& path);
    static std::string getFileExtension(const std::string& path);
    static std::string getFileDirectory(const std::string& path);
    static std::string getAbsolutePath(const std::string& path);
    static std::string getCanonicalPath(const std::string& path);

    // File times
    static std::time_t getCreationTime(const std::string& path);
    static std::time_t getModificationTime(const std::string& path);
    static std::time_t getAccessTime(const std::string& path);

    // File reading/writing
    static ByteBuffer readFile(const std::string& path);
    static std::string readTextFile(const std::string& path);
    static std::vector<std::string> readLines(const std::string& path);

    static bool writeFile(const std::string& path, const ByteBuffer& data);
    static bool writeTextFile(const std::string& path, const std::string& content);
    static bool appendToFile(const std::string& path, const std::string& content);

    // Directory operations
    static std::vector<std::string> listFiles(const std::string& path);
    static std::vector<std::string> listFilesRecursive(const std::string& path);
    static std::vector<std::string> listDirectories(const std::string& path);

    // Search operations
    static std::vector<std::string> findFiles(const std::string& path, const std::string& pattern);
    static std::vector<std::string> findFilesRecursive(const std::string& path, const std::string& pattern);

    // Path operations
    static std::string combinePaths(const std::string& path1, const std::string& path2);
    static std::string getCurrentWorkingDirectory();
    static bool setCurrentWorkingDirectory(const std::string& path);

    // Temporary files
    static std::string getTempDirectory();
    static std::string createTempFile(const std::string& prefix = "websocket_");
    static std::string createTempDirectory(const std::string& prefix = "websocket_");

    // File permissions
    static bool setPermissions(const std::string& path, uint32_t permissions);
    static uint32_t getPermissions(const std::string& path);

    // File watching (basic)
    static bool hasFileChanged(const std::string& path, std::time_t& lastCheck);
};

WEBSOCKET_NAMESPACE_END