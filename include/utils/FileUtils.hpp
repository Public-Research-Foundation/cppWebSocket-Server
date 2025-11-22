#pragma once
#ifndef WEBSOCKET_FILE_UTILS_HPP
#define WEBSOCKET_FILE_UTILS_HPP

#include "../common/Types.hpp"
#include "../common/Macros.hpp"
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <system_error>
#include <chrono>

WEBSOCKET_NAMESPACE_BEGIN

/**
 * @class FileUtils
 * @brief Cross-platform file system operations with comprehensive error handling
 *
 * Wrapper around std::filesystem with additional utilities and consistent error handling.
 * All methods are thread-safe and provide detailed error information.
 */
    class FileUtils {
    public:
        /**
         * @brief File operation result structure
         */
        struct Result {
            bool success{ false };              ///< Operation success status
            std::error_code errorCode;        ///< System error code if operation failed
            std::string errorMessage;         ///< Human-readable error message

            explicit operator bool() const { return success; }
        };

        // ===== FILE EXISTENCE AND PROPERTIES =====

        /**
         * @brief Check if path exists
         * @param path File or directory path
         * @return true if path exists, false otherwise
         */
        static bool exists(const std::string& path);

        /**
         * @brief Check if path is a regular file
         * @param path Path to check
         * @return true if path exists and is a regular file
         */
        static bool isFile(const std::string& path);

        /**
         * @brief Check if path is a directory
         * @param path Path to check
         * @return true if path exists and is a directory
         */
        static bool isDirectory(const std::string& path);

        /**
         * @brief Check if file is readable
         * @param path File path
         * @return true if file exists and is readable
         */
        static bool isReadable(const std::string& path);

        /**
         * @brief Check if file is writable
         * @param path File path
         * @return true if file exists and is writable
         */
        static bool isWritable(const std::string& path);

        /**
         * @brief Check if file is executable
         * @param path File path
         * @return true if file exists and is executable
         */
        static bool isExecutable(const std::string& path);

        // ===== FILE OPERATIONS =====

        /**
         * @brief Create empty file
         * @param path File path to create
         * @return Result indicating success or failure
         */
        static Result createFile(const std::string& path);

        /**
         * @brief Create single directory
         * @param path Directory path to create
         * @return Result indicating success or failure
         */
        static Result createDirectory(const std::string& path);

        /**
         * @brief Create directory tree (like mkdir -p)
         * @param path Directory path to create
         * @return Result indicating success or failure
         */
        static Result createDirectories(const std::string& path);

        /**
         * @brief Remove file
         * @param path File path to remove
         * @return Result indicating success or failure
         */
        static Result removeFile(const std::string& path);

        /**
         * @brief Remove directory (must be empty)
         * @param path Directory path to remove
         * @return Result indicating success or failure
         */
        static Result removeDirectory(const std::string& path);

        /**
         * @brief Remove directory recursively
         * @param path Directory path to remove
         * @return Result indicating success or failure
         */
        static Result removeDirectoryRecursive(const std::string& path);

        /**
         * @brief Copy file
         * @param source Source file path
         * @param destination Destination file path
         * @return Result indicating success or failure
         */
        static Result copyFile(const std::string& source, const std::string& destination);

        /**
         * @brief Move or rename file
         * @param source Source file path
         * @param destination Destination file path
         * @return Result indicating success or failure
         */
        static Result moveFile(const std::string& source, const std::string& destination);

        // ===== FILE INFORMATION =====

        /**
         * @brief Get file size in bytes
         * @param path File path
         * @return File size in bytes, or 0 if file doesn't exist
         */
        static size_t getFileSize(const std::string& path);

        /**
         * @brief Get file name from path
         * @param path File path
         * @return File name with extension
         */
        static std::string getFileName(const std::string& path);

        /**
         * @brief Get file extension
         * @param path File path
         * @return File extension (including dot), or empty string if no extension
         */
        static std::string getFileExtension(const std::string& path);

        /**
         * @brief Get directory containing file
         * @param path File path
         * @return Directory path
         */
        static std::string getFileDirectory(const std::string& path);

        /**
         * @brief Get absolute path
         * @param path File or directory path
         * @return Absolute path
         */
        static std::string getAbsolutePath(const std::string& path);

        /**
         * @brief Get canonical path (resolves symlinks and relative components)
         * @param path File or directory path
         * @return Canonical path
         */
        static std::string getCanonicalPath(const std::string& path);

        // ===== FILE TIMES =====

        /**
         * @brief Get file creation time
         * @param path File path
         * @return Creation time as time_t, or 0 if error
         */
        static std::time_t getCreationTime(const std::string& path);

        /**
         * @brief Get file modification time
         * @param path File path
         * @return Modification time as time_t, or 0 if error
         */
        static std::time_t getModificationTime(const std::string& path);

        /**
         * @brief Get file access time
         * @param path File path
         * @return Access time as time_t, or 0 if error
         */
        static std::time_t getAccessTime(const std::string& path);

        // ===== FILE READING/WRITING =====

        /**
         * @brief Read entire file as binary data
         * @param path File to read
         * @return File contents as byte buffer, or empty buffer on error
         */
        static ByteBuffer readFile(const std::string& path);

        /**
         * @brief Read entire file as text
         * @param path File to read
         * @return File contents as string, or empty string on error
         */
        static std::string readTextFile(const std::string& path);

        /**
         * @brief Read file line by line
         * @param path File to read
         * @return Vector of lines, or empty vector on error
         */
        static std::vector<std::string> readLines(const std::string& path);

        /**
         * @brief Write binary data to file
         * @param path File path
         * @param data Data to write
         * @return Result indicating success or failure
         */
        static Result writeFile(const std::string& path, const ByteBuffer& data);

        /**
         * @brief Write text to file
         * @param path File path
         * @param content Text content to write
         * @return Result indicating success or failure
         */
        static Result writeTextFile(const std::string& path, const std::string& content);

        /**
         * @brief Append text to file
         * @param path File path
         * @param content Text content to append
         * @return Result indicating success or failure
         */
        static Result appendToFile(const std::string& path, const std::string& content);

        // ===== DIRECTORY OPERATIONS =====

        /**
         * @brief List files in directory (non-recursive)
         * @param path Directory path
         * @return Vector of file names, or empty vector on error
         */
        static std::vector<std::string> listFiles(const std::string& path);

        /**
         * @brief List files in directory recursively
         * @param path Directory path
         * @return Vector of file paths (relative to start directory), or empty vector on error
         */
        static std::vector<std::string> listFilesRecursive(const std::string& path);

        /**
         * @brief List subdirectories
         * @param path Directory path
         * @return Vector of directory names, or empty vector on error
         */
        static std::vector<std::string> listDirectories(const std::string& path);

        // ===== SEARCH OPERATIONS =====

        /**
         * @brief Find files matching pattern in directory
         * @param path Directory path
         * @param pattern File pattern (wildcards: *, ?)
         * @return Vector of matching file paths
         */
        static std::vector<std::string> findFiles(const std::string& path, const std::string& pattern);

        /**
         * @brief Find files matching pattern recursively
         * @param path Directory path
         * @param pattern File pattern (wildcards: *, ?)
         * @return Vector of matching file paths
         */
        static std::vector<std::string> findFilesRecursive(const std::string& path, const std::string& pattern);

        // ===== PATH OPERATIONS =====

        /**
         * @brief Combine two paths
         * @param path1 First path
         * @param path2 Second path
         * @return Combined path
         */
        static std::string combinePaths(const std::string& path1, const std::string& path2);

        /**
         * @brief Get current working directory
         * @return Current working directory path
         */
        static std::string getCurrentWorkingDirectory();

        /**
         * @brief Set current working directory
         * @param path New working directory
         * @return Result indicating success or failure
         */
        static Result setCurrentWorkingDirectory(const std::string& path);

        // ===== TEMPORARY FILES =====

        /**
         * @brief Get system temporary directory
         * @return Temporary directory path
         */
        static std::string getTempDirectory();

        /**
         * @brief Create temporary file
         * @param prefix File name prefix
         * @param suffix File name suffix
         * @return Path to created temporary file
         */
        static std::string createTempFile(const std::string& prefix = "websocket_",
            const std::string& suffix = ".tmp");

        /**
         * @brief Create temporary directory
         * @param prefix Directory name prefix
         * @return Path to created temporary directory
         */
        static std::string createTempDirectory(const std::string& prefix = "websocket_");

        // ===== FILE PERMISSIONS =====

        /**
         * @brief Set file permissions
         * @param path File path
         * @param permissions Permission flags
         * @return Result indicating success or failure
         */
        static Result setPermissions(const std::string& path, std::filesystem::perms permissions);

        /**
         * @brief Get file permissions
         * @param path File path
         * @return Permission flags, or std::filesystem::perms::none on error
         */
        static std::filesystem::perms getPermissions(const std::string& path);

        // ===== FILE WATCHING =====

        /**
         * @brief Check if file has changed since last check
         * @param path File path
         * @param lastCheck Last known modification time
         * @return true if file has been modified, false otherwise
         */
        static bool hasFileChanged(const std::string& path, std::time_t& lastCheck);

    private:
        /**
         * @brief Convert filesystem error to result
         * @param ec Error code
         * @return Result structure
         */
        static Result errorToResult(const std::error_code& ec);
};

WEBSOCKET_NAMESPACE_END

#endif // WEBSOCKET_FILE_UTILS_HPP