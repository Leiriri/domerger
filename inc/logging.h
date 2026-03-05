#pragma once

#include <cstring>
#include <fstream>
#include <iostream>

enum class log_level { DEBUG, INFO, WARNING, ERROR, ALWAYS };
class Logger {
public:
  Logger(log_level level);

  template <typename T> Logger &operator<<(const T &message) {
    if (current_level <= logging_level) {
      std::cout << message;
      if (log_file.is_open()) {
        if (first_message_in_chain) {
          std::cout << "[" << log_level_to_string(current_level) << "] ";
          first_message_in_chain = false;
        }
        log_file << message;
      }
      if constexpr (std::is_convertible_v<T, std::string_view>) {
        std::string_view msg_view = message;
        if (msg_view.find('\n') != std::string_view::npos) {
          first_message_in_chain = true; // Reset for next message chain
        }
      }
    }
    return *this;
  }

  static void set_log_level(log_level level);

  static log_level get_log_level();

  static void set_persistent_log(bool persistent);

  ~Logger();

private:
  log_level logging_level;
  static std::ofstream log_file;
  static log_level current_level;
  bool first_message_in_chain = true; // Tracks if this is the first message in a chain

  static std::string log_level_to_string(log_level level) {
    switch (level) {
    case log_level::DEBUG:
      return "DEBUG";
    case log_level::INFO:
      return "INFO";
    case log_level::WARNING:
      return "WARNING";
    case log_level::ERROR:
      return "ERROR";
    case log_level::ALWAYS:
      return "ALWAYS";
    default:
      return "UNKNOWN";
    }
  }
};

extern Logger debug_logger;
extern Logger info_logger;
extern Logger warning_logger;
extern Logger error_logger;
extern Logger always_logger;
