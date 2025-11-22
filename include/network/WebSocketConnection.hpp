#pragma once
#ifndef WEBSOCKET_CONNECTION_HPP
#define WEBSOCKET_CONNECTION_HPP

#include "../common/Types.hpp"
#include "../common/NonCopyable.hpp"
#include "Endpoint.hpp"
#include <memory>
#include <atomic>

// Forward declaration for ASIO
namespace asio {
    class io_context;
    namespace ip {
        class tcp;
    }
    template <typename Protocol> class basic_stream_socket;
    using ip::tcp;
}

WEBSOCKET_NAMESPACE_BEGIN

/**
 * @class WebSocketConnection
 * @brief Represents a single TCP connection with WebSocket protocol handling
 *
 * Responsibilities:
 * - Raw TCP socket management
 * - Data reading/writing
 * - Connection state tracking
 * - Error handling and cleanup
 */
    class WebSocketConnection : public NonCopyable,
    public std::enable_shared_from_this<WebSocketConnection> {
    public:
        using Pointer = std::shared_ptr<WebSocketConnection>;
        using Socket = asio::basic_stream_socket<asio::ip::tcp>;

        /**
         * @brief Connection states
         */
        enum class State {
            DISCONNECTED,   ///< Not connected
            CONNECTING,     ///< Connection in progress
            CONNECTED,      ///< Active connection
            CLOSING,        ///< Closing in progress
            CLOSED          ///< Connection closed
        };

        /**
         * @brief Construct a new WebSocket Connection
         * @param io_context ASIO I/O context
         */
        explicit WebSocketConnection(asio::io_context& io_context);
        ~WebSocketConnection();

        /**
         * @brief Get the underlying socket
         * @return Reference to ASIO socket
         */
        Socket& getSocket();

        /**
         * @brief Start the connection (begin reading)
         */
        void start();

        /**
         * @brief Close the connection
         * @param graceful Whether to wait for pending writes
         */
        void close(bool graceful = true);

        /**
         * @brief Send raw data over the connection
         * @param data Data to send
         * @return true if data queued for sending
         */
        bool send(const Buffer& data);

        /**
         * @brief Send string data over the connection
         * @param data String data to send
         * @return true if data queued for sending
         */
        bool send(const std::string& data);

        /**
         * @brief Get connection endpoint
         * @return Remote endpoint information
         */
        Endpoint getRemoteEndpoint() const;

        /**
         * @brief Get connection state
         * @return Current connection state
         */
        State getState() const;

        /**
         * @brief Check if connection is active
         * @return true if connected and active
         */
        bool isConnected() const;

        /**
         * @brief Get connection statistics
         * @return Connection statistics
         */
        ConnectionStats getStats() const;

        /**
         * @brief Set receive callback
         * @param callback Function to call when data is received
         */
        void setReceiveCallback(std::function<void(const Buffer&)> callback);

        /**
         * @brief Set close callback
         * @param callback Function to call when connection closes
         */
        void setCloseCallback(std::function<void()> callback);

        /**
         * @brief Reset connection for reuse (when returning to pool)
         */
        void reset();

    private:
        /**
         * @brief Start async read operation
         */
        void startRead();

        /**
         * @brief Handle async read completion
         * @param error Error code
         * @param bytes_transferred Number of bytes read
         */
        void handleRead(const asio::error_code& error, size_t bytes_transferred);

        /**
         * @brief Handle async write completion
         * @param error Error code
         * @param bytes_transferred Number of bytes written
         */
        void handleWrite(const asio::error_code& error, size_t bytes_transferred);

        /**
         * @brief Process received data
         * @param data Received data buffer
         */
        void processReceivedData(const Buffer& data);

        /**
         * @brief Close connection with error
         * @param error Error that caused closure
         */
        void closeWithError(const asio::error_code& error);

        // Member variables
        asio::io_context& io_context_;
        std::unique_ptr<Socket> socket_;
        std::atomic<State> state_{ State::DISCONNECTED };

        // I/O buffers
        Buffer read_buffer_;
        std::vector<Buffer> write_queue_;

        // Callbacks
        std::function<void(const Buffer&)> receive_callback_;
        std::function<void()> close_callback_;

        // Statistics
        ConnectionStats stats_;
        Endpoint remote_endpoint_;
};

WEBSOCKET_NAMESPACE_END

#endif // WEBSOCKET_CONNECTION_HPP