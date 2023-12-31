#pragma once

#ifdef SFML_EMBEDDED_LOGGING

#if !defined(__PRETTY_FUNCTION__) && !defined(__GNUC__)
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

#include <spdlog/spdlog.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/null_sink.h>
#include <iostream>
#include <mutex>

namespace sf
{
class EmbeddedLogger
{
public:
  inline static std::shared_ptr< spdlog::logger > log;

  static void initializeConsole();
  static void initializeLogger( const std::string& filename );
  static void initializeNullLogger();
  static void addSink( std::shared_ptr< spdlog::sinks::sink > sink );
  static bool isInitialized();

private:

  static void ensureLoggerExists();

  static inline std::mutex m_mutex;
};
}

#define LOG_TRACE( ... ) SPDLOG_LOGGER_TRACE( sf::EmbeddedLogger::log, __VA_ARGS__ )
#define LOG_DEBUG( ... ) SPDLOG_LOGGER_DEBUG( sf::EmbeddedLogger::log, __VA_ARGS__ )
#define LOG_INFO( ... ) SPDLOG_LOGGER_INFO( sf::EmbeddedLogger::log, __VA_ARGS__ )
#define LOG_WARN( ... ) SPDLOG_LOGGER_WARN( sf::EmbeddedLogger::log, __VA_ARGS__ )
#define LOG_ERROR( ... ) SPDLOG_LOGGER_ERROR( sf::EmbeddedLogger::log, __VA_ARGS__ )
#define LOG_CRITICAL( ... ) SPDLOG_LOGGER_CRITICAL( sf::EmbeddedLogger::log, __VA_ARGS__ )

#else

#define LOG_TRACE( ... )
#define LOG_DEBUG( ... ) LOG_TRACE( __VA_ARGS__ )
#define LOG_INFO( ... ) LOG_TRACE( __VA_ARGS__ )
#define LOG_WARN( ... ) LOG_TRACE( __VA_ARGS__ )
#define LOG_ERROR( ... ) LOG_TRACE( __VA_ARGS__ )
#define LOG_CRITICAL( ... ) LOG_TRACE( __VA_ARGS__ )

#endif


