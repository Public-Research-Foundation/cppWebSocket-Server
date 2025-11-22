#pragma once
#ifndef WEBSOCKET_ENDPOINT_HPP
#define WEBSOCKET_ENDPOINT_HPP

#include "../common/Types.hpp"
#include <string>
#include <cstdint>
#include <optional>

WEBSOCKET_NAMESPACE_BEGIN

/**
 * @class Endpoint
 * @brief Represents a network endpoint (address and port)
 *
 * Supports:
 * - IPv4 and IPv6 addresses
 * - Hostname resolution
 * - String representation parsing/formatting
 * - Comparison and hashing for use in containers
 */
    class Endpoint {
    public:
        /**
         * @brief Address family types
         */
        enum class Family {
            IPv4,
            IPv6,
            UNSPECIFIED
        };

        /**
         * @brief Default constructor (unspecified endpoint)
         */
        Endpoint();

        /**
         * @brief Construct from address and port
         * @param address IP address or hostname
         * @param port Network port
         */
        Endpoint(const std::string& address, uint16_t port);

        /**
         * @brief Construct from IP address bytes and port
         * @param family Address family
         * @param data IP address bytes
         * @param port Network port
         */
        Endpoint(Family family, const Buffer& data, uint16_t port);

        /**
         * @brief Parse endpoint from string (e.g., "192.168.1.1:8080")
         * @param endpoint_string String representation of endpoint
         * @return Endpoint object if parsing successful, empty optional otherwise
         */
        static std::optional<Endpoint> fromString(const std::string& endpoint_string);

        /**
         * @brief Resolve hostname to endpoint(s)
         * @param hostname Hostname to resolve
         * @param service Service name or port number
         * @param family Preferred address family
         * @return Vector of resolved endpoints
         */
        static std::vector<Endpoint> resolve(const std::string& hostname,
            const std::string& service = "",
            Family family = Family::UNSPECIFIED);

        /**
         * @brief Get endpoint as string (e.g., "192.168.1.1:8080")
         * @return String representation
         */
        std::string toString() const;

        /**
         * @brief Get address as string
         * @return IP address string
         */
        std::string getAddress() const;

        /**
         * @brief Get port number
         * @return Port number
         */
        uint16_t getPort() const;

        /**
         * @brief Get address family
         * @return Address family
         */
        Family getFamily() const;

        /**
         * @brief Check if endpoint is valid
         * @return true if endpoint has valid address and port
         */
        bool isValid() const;

        /**
         * @brief Check if endpoint is IPv4
         * @return true if IPv4 address
         */
        bool isIPv4() const;

        /**
         * @brief Check if endpoint is IPv6
         * @return true if IPv6 address
         */
        bool isIPv6() const;

        /**
         * @brief Check if endpoint is loopback address
         * @return true if loopback address
         */
        bool isLoopback() const;

        /**
         * @brief Check if endpoint is private address (RFC 1918)
         * @return true if private address
         */
        bool isPrivate() const;

        /**
         * @brief Comparison operators
         */
        bool operator==(const Endpoint& other) const;
        bool operator!=(const Endpoint& other) const;
        bool operator<(const Endpoint& other) const;

        /**
         * @brief Hash function for use in unordered containers
         */
        struct Hash {
            std::size_t operator()(const Endpoint& endpoint) const;
        };

    private:
        /**
         * @brief Parse IPv4 address from string
         * @param address String representation
         * @return true if parsing successful
         */
        bool parseIPv4(const std::string& address);

        /**
         * @brief Parse IPv6 address from string
         * @param address String representation
         * @return true if parsing successful
         */
        bool parseIPv6(const std::string& address);

        /**
         * @brief Normalize address representation
         */
        void normalize();

        // Member variables
        Family family_{ Family::UNSPECIFIED };
        Buffer address_data_;
        uint16_t port_{ 0 };
        std::string address_string_;
};

WEBSOCKET_NAMESPACE_END

#endif // WEBSOCKET_ENDPOINT_HPP