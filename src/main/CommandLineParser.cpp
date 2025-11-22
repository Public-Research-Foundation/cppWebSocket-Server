#include "main/CommandLineParser.hpp"
#include "utils/Logger.hpp"
#include <iostream>
#include <algorithm>

WEBSOCKET_NAMESPACE_BEGIN

CommandLineParser::CommandLineParser() {
    initializeDefaultOptions();
}

void CommandLineParser::initializeDefaultOptions() {
    // Server configuration options
    addOption("config", 'c', "Configuration file path", "FILE", true);
    addOption("port", 'p', "Server port number", "PORT", true, 8080);
    addOption("threads", 't', "Number of worker threads", "COUNT", true, 0);
    addOption("max-connections", 'm', "Maximum concurrent connections", "COUNT", true, 1000);

    // Logging options
    addOption("log-level", 'l', "Log level (TRACE|DEBUG|INFO|WARN|ERROR|FATAL)", "LEVEL", true, "INFO");
    addOption("log-file", 'f', "Log file path", "FILE", true);

    // Operational options
    addFlag("daemon", 'd', "Run as daemon process");
    addFlag("help", 'h', "Show help message");
    addFlag("version", 'v', "Show version information");

    // Security options
    addOption("ssl-cert", 's', "SSL certificate file", "FILE", true);
    addOption("ssl-key", 'k', "SSL private key file", "FILE", true);
}

void CommandLineParser::addOption(const std::string& longName, char shortName,
    const std::string& description,
    const std::string& valueName,
    bool requiresValue,
    const std::any& defaultValue) {
    CommandLineOption option;
    option.longName = longName;
    option.shortName = shortName;
    option.description = description;
    option.valueName = valueName;
    option.requiresValue = requiresValue;
    option.defaultValue = defaultValue;
    option.isSet = false;

    options_[longName] = option;
    shortToLongMap_[shortName] = longName;
}

void CommandLineParser::addFlag(const std::string& longName, char shortName,
    const std::string& description) {
    addOption(longName, shortName, description, "", false, false);
}

Result CommandLineParser::parse(int argc, char* argv[]) {
    if (argc == 0 || argv == nullptr) {
        return Result::ERROR;
    }

    // Store executable name
    if (argc > 0) {
        executableName_ = argv[0];
    }

    // Convert to vector for easier processing
    std::vector<std::string> args;
    for (int i = 1; i < argc; ++i) {
        if (argv[i] != nullptr) {
            args.push_back(argv[i]);
        }
    }

    return parse(args);
}

Result CommandLineParser::parse(const std::vector<std::string>& args) {
    validationErrors_.clear();
    positionalArgs_.clear();
    unknownOptions_.clear();

    // Reset all options to default state
    for (auto& [name, option] : options_) {
        option.isSet = false;
        option.value = option.defaultValue;
    }

    size_t i = 0;
    while (i < args.size()) {
        const std::string& arg = args[i];
        const std::string& nextArg = (i + 1 < args.size()) ? args[i + 1] : "";

        Result result = processOption(arg, nextArg, i, args);
        if (result != Result::SUCCESS) {
            return result;
        }
    }

    // Validate parsed options
    if (!validate()) {
        return Result::ERROR;
    }

    return Result::SUCCESS;
}

Result CommandLineParser::processOption(const std::string& arg, const std::string& nextArg,
    size_t& index, const std::vector<std::string>& args) {
    if (arg.empty()) {
        index++;
        return Result::SUCCESS;
    }

    if (arg == "--") {
        // End of options, remaining are positional
        index++;
        while (index < args.size()) {
            positionalArgs_.push_back(args[index++]);
        }
        return Result::SUCCESS;
    }

    if (arg.starts_with("--")) {
        return processLongOption(arg, nextArg, index, args);
    }
    else if (arg.starts_with("-")) {
        return processShortOption(arg, nextArg, index, args);
    }
    else {
        // Positional argument
        positionalArgs_.push_back(arg);
        index++;
    }

    return Result::SUCCESS;
}

Result CommandLineParser::processLongOption(const std::string& arg, const std::string& nextArg,
    size_t& index, const std::vector<std::string>& args) {
    std::string optionName = extractOptionName(arg);

    CommandLineOption* option = findOption(optionName);
    if (!option) {
        unknownOptions_.push_back(arg);
        index++;
        return Result::SUCCESS;
    }

    option->isSet = true;

    if (option->requiresValue) {
        std::string value;
        size_t equalsPos = arg.find('=');

        if (equalsPos != std::string::npos) {
            // Value provided with = syntax: --option=value
            value = arg.substr(equalsPos + 1);
        }
        else if (!nextArg.empty() && !nextArg.starts_with("-")) {
            // Value provided as next argument: --option value
            value = nextArg;
            index++; // Skip next argument
        }
        else {
            addValidationError("Option --" + optionName + " requires a value");
            return Result::ERROR;
        }

        // Simple type conversion based on default value type
        try {
            if (option->defaultValue.type() == typeid(int)) {
                option->value = std::stoi(value);
            }
            else if (option->defaultValue.type() == typeid(bool)) {
                option->value = (value == "true" || value == "1" || value == "yes");
            }
            else {
                option->value = value;
            }
        }
        catch (const std::exception& e) {
            addValidationError("Invalid value for --" + optionName + ": " + value);
            return Result::ERROR;
        }
    }
    else {
        // Flag option, set to true
        option->value = true;
    }

    index++;
    return Result::SUCCESS;
}

Result CommandLineParser::processShortOption(const std::string& arg, const std::string& nextArg,
    size_t& index, const std::vector<std::string>& args) {
    std::string optionChars = arg.substr(1); // Remove leading '-'

    for (size_t j = 0; j < optionChars.length(); ++j) {
        char shortName = optionChars[j];
        CommandLineOption* option = findOptionByShortName(shortName);

        if (!option) {
            unknownOptions_.push_back(std::string("-") + shortName);
            continue;
        }

        option->isSet = true;

        if (option->requiresValue) {
            std::string value;

            if (j == optionChars.length() - 1) {
                // Last character in group, value might be next argument
                if (!nextArg.empty() && !nextArg.starts_with("-")) {
                    value = nextArg;
                    index++; // Skip next argument
                }
                else {
                    addValidationError("Option -" + std::string(1, shortName) + " requires a value");
                    return Result::ERROR;
                }
            }
            else {
                // Value is rest of the string: -ovalue
                value = optionChars.substr(j + 1);
                j = optionChars.length(); // Processed all characters
            }

            try {
                if (option->defaultValue.type() == typeid(int)) {
                    option->value = std::stoi(value);
                }
                else if (option->defaultValue.type() == typeid(bool)) {
                    option->value = (value == "true" || value == "1" || value == "yes");
                }
                else {
                    option->value = value;
                }
            }
            catch (const std::exception& e) {
                addValidationError("Invalid value for -" + std::string(1, shortName) + ": " + value);
                return Result::ERROR;
            }
        }
        else {
            // Flag option
            option->value = true;
        }
    }

    index++;
    return Result::SUCCESS;
}

bool CommandLineParser::hasOption(const std::string& longName) const {
    const CommandLineOption* option = findOption(longName);
    return option && option->isSet;
}

bool CommandLineParser::hasOption(char shortName) const {
    const CommandLineOption* option = findOptionByShortName(shortName);
    return option && option->isSet;
}

std::string CommandLineParser::getExecutableName() const {
    return executableName_;
}

std::vector<std::string> CommandLineParser::getPositionalArguments() const {
    return positionalArgs_;
}

std::vector<std::string> CommandLineParser::getUnknownOptions() const {
    return unknownOptions_;
}

void CommandLineParser::showHelp() const {
    std::cout << generateHelpText() << std::endl;
}

void CommandLineParser::showUsage() const {
    std::cout << generateUsageText() << std::endl;
}

std::string CommandLineParser::generateHelpText() const {
    std::ostringstream oss;

    oss << "Usage: " << (executableName_.empty() ? "websocket-server" : executableName_)
        << " [OPTIONS]" << std::endl;
    oss << std::endl;
    oss << "Options:" << std::endl;

    // Find maximum option length for alignment
    size_t maxOptionLength = 0;
    for (const auto& [name, option] : options_) {
        size_t length = 2 + name.length(); // --name
        if (option.requiresValue) {
            length += 1 + option.valueName.length(); // =VALUE
        }
        if (option.shortName != '\0') {
            length += 4; // , -x
            if (option.requiresValue) {
                length += 1 + option.valueName.length(); // VALUE
            }
        }
        maxOptionLength = std::max(maxOptionLength, length);
    }

    // Generate option descriptions
    for (const auto& [name, option] : options_) {
        std::string optionStr = "  --" + name;
        if (option.requiresValue) {
            optionStr += "=" + option.valueName;
        }

        if (option.shortName != '\0') {
            optionStr += ", -" + std::string(1, option.shortName);
            if (option.requiresValue) {
                optionStr += " " + option.valueName;
            }
        }

        // Pad to align descriptions
        optionStr.resize(maxOptionLength + 4, ' ');
        oss << optionStr << option.description << std::endl;
    }

    return oss.str();
}

std::string CommandLineParser::generateUsageText() const {
    std::ostringstream oss;

    oss << "Usage examples:" << std::endl;
    oss << "  " << (executableName_.empty() ? "websocket-server" : executableName_)
        << " --config server.json --port 8080" << std::endl;
    oss << "  " << (executableName_.empty() ? "websocket-server" : executableName_)
        << " -p 9000 --daemon --log-level DEBUG" << std::endl;
    oss << "  " << (executableName_.empty() ? "websocket-server" : executableName_)
        << " --help" << std::endl;

    return oss.str();
}

bool CommandLineParser::validate() const {
    return validationErrors_.empty();
}

std::vector<std::string> CommandLineParser::getValidationErrors() const {
    return validationErrors_;
}

std::unordered_map<std::string, std::any> CommandLineParser::generateConfig() const {
    std::unordered_map<std::string, std::any> config;

    for (const auto& [name, option] : options_) {
        if (option.isSet) {
            // Map command line options to configuration keys
            std::string configKey = name;
            std::replace(configKey.begin(), configKey.end(), '-', '_');

            if (name == "port") configKey = "server.port";
            else if (name == "threads") configKey = "server.threads";
            else if (name == "max-connections") configKey = "server.max_connections";
            else if (name == "log-level") configKey = "logging.level";
            else if (name == "log-file") configKey = "logging.file";
            else if (name == "ssl-cert") configKey = "security.cert_file";
            else if (name == "ssl-key") configKey = "security.key_file";

            config[configKey] = option.value;
        }
    }

    return config;
}

// Private helper methods
CommandLineOption* CommandLineParser::findOption(const std::string& longName) {
    auto it = options_.find(longName);
    return it != options_.end() ? &it->second : nullptr;
}

const CommandLineOption* CommandLineParser::findOption(const std::string& longName) const {
    auto it = options_.find(longName);
    return it != options_.end() ? &it->second : nullptr;
}

CommandLineOption* CommandLineParser::findOptionByShortName(char shortName) {
    auto it = shortToLongMap_.find(shortName);
    return it != shortToLongMap_.end() ? findOption(it->second) : nullptr;
}

const CommandLineOption* CommandLineParser::findOptionByShortName(char shortName) const {
    auto it = shortToLongMap_.find(shortName);
    return it != shortToLongMap_.end() ? findOption(it->second) : nullptr;
}

std::string CommandLineParser::extractOptionName(const std::string& arg) const {
    size_t start = arg.starts_with("--") ? 2 : 1;
    size_t end = arg.find('=');
    return arg.substr(start, end - start);
}

std::string CommandLineParser::extractOptionValue(const std::string& arg) const {
    size_t equalsPos = arg.find('=');
    if (equalsPos == std::string::npos) {
        return "";
    }
    return arg.substr(equalsPos + 1);
}

void CommandLineParser::addValidationError(const std::string& error) {
    validationErrors_.push_back(error);
}

WEBSOCKET_NAMESPACE_END