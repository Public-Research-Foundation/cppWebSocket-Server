#pragma once

#include "../common/Types.hpp"
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>

WEBSOCKET_NAMESPACE_BEGIN

/**
 * Cross-platform file system operations
 * Wrapper around std::filesystem with additional utilities
 * Exception-safe operations with proper error handling
 */
    class FileUtils {
    public:
        // ===== FILE EXISTENCE AND PROPERTIES =====

        static bool exists(const std::string& path);
        static bool isFile(const std::string& path);
        static bool isDirectory(const std::string& path);
        static bool isReadable(const std::string& path);
        static bool isWritable(const std::string& path);
        static bool isExecutable(const std::string& path);

        // ===== FILE OPERATIONS =====

        /**
         * Create empty file
         * @param path File path to create
         * @return true if successful
         */
        static bool createFile(const std::string& path);

        /**
         * Create single directory
         * @param path Directory path to create
         * @return true if successful
         */
        static bool createDirectory(const std::string& path);

        /**
         * Create directory tree (like mkdir -p)
         * @param path Directory path to create
         * @return true if successful
         */
        static bool createDirectories(const std::string& path);

        static bool removeFile(const std::string& path);
        static bool removeDirectory(const std::string& path);
        static bool copyFile(const std::string& source, const std::string& destination);
        static bool moveFile(const std::string& source, const std::string& destination);
        static bool renameFile(const std::string& oldPath, const std::string& newPath);

        // ===== FILE INFORMATION =====

        static size_t getFileSize(const std::string& path);
        static std::string getFileName(const std::string& path);
        static std::string getFileExtension(const std::string& path);
        static std::string getFileDirectory(const std::string& path);
        static std::string getAbsolutePath(const std::string& path);
        static std::string getCanonicalPath(const std::string& path);

        // ===== FILE TIMES =====

        static std::time_t getCreationTime(const std::string& path);
        static std::time_t getModificationTime(const std::string& path);
        static std::time_t getAccessTime(const std::string& path);

        // ===== FILE READING/WRITING =====

        /**
         * Read entire file as binary data
         * @param path File to read
         * @return File contents as byte buffer
         */
        static ByteBuffer readFile(const std::string& path);

        /**
         * Read entire file as text
         * @param path File to read
         * @return File contents as string
         */
        static std::string readTextFile(const std::string& path);

        /**
         * Read file line by line
         * @param path File to read
         * @return Vector of lines
         */
        static std::vector<std::string> readLines(const std::string& path);

        static bool writeFile(const std::string& path, const ByteBuffer& data);
        static bool writeTextFile(const std::string& path, const std::string& content);
        static bool appendToFile(const std::string& path, const std::string& content);

        // ===== DIRECTORY OPERATIONS =====

        static std::vector<std::string> listFiles(const std::string& path);
        static std::vector<std::string> listFilesRecursive(const std::string& path);
        static std::vector<std::string> listDirectories(const std::string& path);

        // ===== SEARCH OPERATIONS =====

        static std::vector<std::string> findFiles(const std::string& path, const std::string& pattern);
        static std::vector<std::string> findFilesRecursive(const std::string& path, const std::string& pattern);

        // ===== PATH OPERATIONS =====

        static std::string combinePaths(const std::string& path1, const std::string& path2);
        static std::string getCurrentWorkingDirectory();
        static bool setCurrentWorkingDirectory(const std::string& path);

        // ===== TEMPORARY FILES =====

        static std::string getTempDirectory();
        static std::string createTempFile(const std::string& prefix = "websocket_");
        static std::string createTempDirectory(const std::string& prefix = "websocket_");

        // ===== FILE PERMISSIONS =====

        static bool setPermissions(const std::string& path, uint32_t permissions);
        static uint32_t getPermissions(const std::string& path);

        // ===== FILE WATCHING =====
        // Basic file change detection

        static bool hasFileChanged(const std::string& path, std::time_t& lastCheck);
};

WEBSOCKET_NAMESPACE_END