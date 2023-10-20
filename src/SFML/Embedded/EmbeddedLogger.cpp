#include "../../../include/SFML/Embedded/EmbeddedLogger.hpp"

#ifdef USING_LOGGER

////////////////////////////////////////////////////////////
// PUBLIC
void sf::EmbeddedLogger::initializeConsole()
{
  std::unique_lock< std::mutex > lock( m_mutex );

  if ( !log )
  {
    ensureLoggerExists();

    auto console = std::make_shared< spdlog::sinks::stdout_color_sink_mt >();

    console->set_level( spdlog::level::debug );
    console->set_pattern( "[%L][%t][%H:%M:%S.%e][%!:%#] %v" );
    log->sinks().push_back( console );
  }
}

////////////////////////////////////////////////////////////
// PUBLIC
void sf::EmbeddedLogger::initializeLogger( const std::string &filename )
{
  std::unique_lock< std::mutex > lock( m_mutex );

  if ( !log )
  {
    ensureLoggerExists();
    // this can throw an error if the file can't be created or written to
    auto file = std::make_shared< spdlog::sinks::basic_file_sink_mt >( filename, true );
    file->set_pattern( "[%L][%t][%H:%M:%S.%e][%!:%#] %v" );
    log->sinks().push_back( file );
  }
}

////////////////////////////////////////////////////////////
// PUBLIC
void sf::EmbeddedLogger::initializeNullLogger()
{
  std::unique_lock< std::mutex > lock( m_mutex );

  if ( !log )
  {
    ensureLoggerExists();
    log->sinks().push_back( std::make_shared< spdlog::sinks::null_sink_mt >() );
  }
}

////////////////////////////////////////////////////////////
// PRIVATE
void sf::EmbeddedLogger::ensureLoggerExists()
{
  if ( log )
  {
    LOG_WARN( "request to initialize log when log already exists. ignoring request." );
    return;
  }

  log = std::make_shared< spdlog::logger >( "nxgl" );
  log->set_level( spdlog::level::debug );
  log->flush_on( spdlog::level::debug );
  log->set_pattern( "[%L][%t][%H:%M:%S.%e][%!:%#] %v" );
}

#else

////////////////////////////////////////////////////////////
// PUBLIC
void sf::EmbeddedLogger::initializeConsole()
{}

////////////////////////////////////////////////////////////
// PUBLIC
void sf::EmbeddedLogger::initializeLogger( const std::string &filename )
{}

////////////////////////////////////////////////////////////
// PUBLIC
void sf::EmbeddedLogger::initializeNullLogger()
{}



#endif

