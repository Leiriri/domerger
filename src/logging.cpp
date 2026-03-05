#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>
#include <chrono>

#include "logging.h"

std::string get_timestamped_log_name() {
    // Get the current time
    auto now = std::chrono::system_clock::now();

    // Convert to time_t to work with std::tm
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);

    // Convert to local time
    std::tm local_time = *std::localtime(&now_time);

    // Format the timestamp
    std::ostringstream oss;
    oss << std::put_time(&local_time, "%Y-%m-%d_%H-%M-%S"); // Format: YYYY-MM-DD_HH-MM-SS

    // Append the log file extension
    return "merger_" + oss.str() + ".log";
}

Logger::Logger(log_level level) : logging_level(level) {}

void Logger::set_log_level(log_level level) { current_level = level; }

log_level Logger::get_log_level() { return current_level; }

void Logger::set_persistent_log(bool persistent) {
  if (persistent) {
    if (log_file.is_open()) {
      log_file.close();
    } else {
      std::string file_path = get_timestamped_log_name();
      log_file.open(file_path, std::ios::app); // Open in append mode
      if (!log_file.is_open()) {
        std::cerr << "Failed to open log file: " << file_path << std::endl;
        exit(1);
      }
    }
  }
}

Logger::~Logger() {
  // do nothing as the log_file is static and shared
}

// Initialize the static member
log_level Logger::current_level = log_level::INFO;
std::ofstream Logger::log_file;


Logger debug_logger(log_level::DEBUG);
Logger info_logger(log_level::INFO);
Logger warning_logger(log_level::WARNING);
Logger error_logger(log_level::ERROR);
Logger always_logger(log_level::ALWAYS);
