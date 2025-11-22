#pragma once
#ifndef WEBSOCKET_SERVICE_LOCATOR_HPP
#define WEBSOCKET_SERVICE_LOCATOR_HPP

#include "../common/Types.hpp"
#include "../common/NonCopyable.hpp"
#include <memory>
#include <unordered_map>
#include <string>
#include <typeindex>
#include <mutex>

WEBSOCKET_NAMESPACE_BEGIN

/**
 * @class ServiceLocator
 * @brief Dependency injection container for managing service instances
 *
 * Provides a centralized registry for services with support for:
 * - Interface-based service resolution
 * - Singleton service management
 * - Scoped service lifetimes
 * - Thread-safe service access
 * - Service dependency resolution
 *
 * Design Pattern: Service Locator + Dependency Injection Container
 */
    class ServiceLocator : public NonCopyable {
    public:
        /**
         * @brief Service lifetime options
         */
        enum class Lifetime {
            SINGLETON,      ///< Single instance shared across all requests
            TRANSIENT,      ///< New instance created for each resolution
            SCOPED          ///< Instance shared within a specific scope
        };

        /**
         * @brief Service registration information
         */
        struct ServiceRegistration {
            std::type_index type;           ///< Service interface type
            std::string name;               ///< Service name (for named services)
            Lifetime lifetime;              ///< Service lifetime
            std::function<std::shared_ptr<void>()> factory; ///< Instance factory
            std::shared_ptr<void> instance; ///< Cached instance (for singletons)
        };

        /**
         * @brief Default constructor
         */
        ServiceLocator() = default;

        /**
         * @brief Destructor
         */
        ~ServiceLocator() = default;

        /**
         * @brief Register a service with the locator
         * @tparam Interface Service interface type
         * @tparam Implementation Service implementation type
         * @param name Service name (empty for default)
         * @param lifetime Service lifetime mode
         * @return true if registration successful
         */
        template<typename Interface, typename Implementation>
        bool registerService(const std::string& name = "", Lifetime lifetime = Lifetime::SINGLETON) {
            static_assert(std::is_base_of<Interface, Implementation>::value,
                "Implementation must derive from Interface");

            std::lock_guard lock(mutex_);

            std::type_index type = typeid(Interface);
            std::string service_name = name.empty() ? type.name() : name;
            std::string key = createKey(type, service_name);

            // Check if service already registered
            if (services_.find(key) != services_.end()) {
                return false; // Service already registered
            }

            ServiceRegistration registration{
                type,
                service_name,
                lifetime,
                []() -> std::shared_ptr<void> {
                    return std::static_pointer_cast<void>(
                        std::make_shared<Implementation>()
                    );
                },
                nullptr
            };

            services_.emplace(key, std::move(registration));
            return true;
        }

        /**
         * @brief Register a service with custom factory function
         * @tparam Interface Service interface type
         * @param factory Factory function that creates service instances
         * @param name Service name (empty for default)
         * @param lifetime Service lifetime mode
         * @return true if registration successful
         */
        template<typename Interface>
        bool registerFactory(std::function<std::shared_ptr<Interface>()> factory,
            const std::string& name = "",
            Lifetime lifetime = Lifetime::SINGLETON) {
            std::lock_guard lock(mutex_);

            std::type_index type = typeid(Interface);
            std::string service_name = name.empty() ? type.name() : name;
            std::string key = createKey(type, service_name);

            if (services_.find(key) != services_.end()) {
                return false;
            }

            ServiceRegistration registration{
                type,
                service_name,
                lifetime,
                [factory]() -> std::shared_ptr<void> {
                    return std::static_pointer_cast<void>(factory());
                },
                nullptr
            };

            services_.emplace(key, std::move(registration));
            return true;
        }

        /**
         * @brief Register a service instance (singleton)
         * @tparam Interface Service interface type
         * @param instance Service instance to register
         * @param name Service name (empty for default)
         * @return true if registration successful
         */
        template<typename Interface>
        bool registerInstance(std::shared_ptr<Interface> instance, const std::string& name = "") {
            std::lock_guard lock(mutex_);

            std::type_index type = typeid(Interface);
            std::string service_name = name.empty() ? type.name() : name;
            std::string key = createKey(type, service_name);

            if (services_.find(key) != services_.end()) {
                return false;
            }

            ServiceRegistration registration{
                type,
                service_name,
                Lifetime::SINGLETON,
                nullptr,
                std::static_pointer_cast<void>(instance)
            };

            services_.emplace(key, std::move(registration));
            return true;
        }

        /**
         * @brief Resolve a service by type and name
         * @tparam T Service type to resolve
         * @param name Service name (empty for default)
         * @return Shared pointer to service instance, nullptr if not found
         */
        template<typename T>
        std::shared_ptr<T> resolve(const std::string& name = "") {
            std::lock_guard lock(mutex_);

            std::type_index type = typeid(T);
            std::string service_name = name.empty() ? type.name() : name;
            std::string key = createKey(type, service_name);

            auto it = services_.find(key);
            if (it == services_.end()) {
                return nullptr;
            }

            auto& registration = it->second;

            switch (registration.lifetime) {
            case Lifetime::SINGLETON:
                if (!registration.instance) {
                    registration.instance = registration.factory();
                }
                return std::static_pointer_cast<T>(registration.instance);

            case Lifetime::TRANSIENT:
                return std::static_pointer_cast<T>(registration.factory());

            case Lifetime::SCOPED:
                // For scoped lifetime, we'd typically use a scoped container
                // For now, treat as transient
                return std::static_pointer_cast<T>(registration.factory());
            }

            return nullptr;
        }

        /**
         * @brief Check if a service is registered
         * @tparam T Service type to check
         * @param name Service name (empty for default)
         * @return true if service is registered
         */
        template<typename T>
        bool isRegistered(const std::string& name = "") {
            std::lock_guard lock(mutex_);

            std::type_index type = typeid(T);
            std::string service_name = name.empty() ? type.name() : name;
            std::string key = createKey(type, service_name);

            return services_.find(key) != services_.end();
        }

        /**
         * @brief Unregister a service
         * @tparam T Service type to unregister
         * @param name Service name (empty for default)
         * @return true if service was unregistered
         */
        template<typename T>
        bool unregister(const std::string& name = "") {
            std::lock_guard lock(mutex_);

            std::type_index type = typeid(T);
            std::string service_name = name.empty() ? type.name() : name;
            std::string key = createKey(type, service_name);

            return services_.erase(key) > 0;
        }

        /**
         * @brief Clear all service registrations
         */
        void clear();

        /**
         * @brief Get number of registered services
         * @return Service count
         */
        size_t getServiceCount() const;

    private:
        /**
         * @brief Create a unique key for service lookup
         * @param type Service type index
         * @param name Service name
         * @return Unique key string
         */
        std::string createKey(const std::type_index& type, const std::string& name) const {
            return std::string(type.name()) + ":" + name;
        }

        // Member variables
        mutable std::mutex mutex_;
        std::unordered_map<std::string, ServiceRegistration> services_;
};

WEBSOCKET_NAMESPACE_END

#endif // WEBSOCKET_SERVICE_LOCATOR_HPP