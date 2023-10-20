#include "../../../include/SFML/Embedded/EmbeddedWindow.hpp"

#include "EmbeddedWindowImpl.hpp"
#include "../../../include/SFML/Embedded/EmbeddedWindowEventReceiver.hpp"
#include "../../../include/SFML/Embedded/EmbeddedLogger.hpp"

namespace sf
{

////////////////////////////////////////////////////////////
// PUBLIC
EmbeddedWindow::EmbeddedWindow( WindowHandle parentHandle,
                                EmbeddedWindowEventReceiver &embeddedWindowEvent )
  : EmbeddedWindow(
      parentHandle,
      embeddedWindowEvent,
      sf::ContextSettings {},
      { 0, 0 } )
{}

////////////////////////////////////////////////////////////
// PUBLIC
EmbeddedWindow::EmbeddedWindow( WindowHandle parentHandle,
                                EmbeddedWindowEventReceiver& embeddedWindowEvent,
                                ContextSettings contextSettings )
  : EmbeddedWindow(
      parentHandle,
      embeddedWindowEvent,
      contextSettings,
      { 0, 0 } )
{}

////////////////////////////////////////////////////////////
// PUBLIC
EmbeddedWindow::EmbeddedWindow( WindowHandle parentHandle,
                                EmbeddedWindowEventReceiver &embeddedWindowEvent,
                                ContextSettings contextSettings,
                                const Vector2u &startingSize )
  : m_embeddedWindowEvent( embeddedWindowEvent )
{
  m_impl = priv::EmbeddedWindowImpl::create(
    parentHandle,
    [this]( E_EmbeddedWindowEventState status ) { onObservation( status ); } );

  if ( m_impl )
  {
    m_window.create( m_impl->getNativeHandle(), contextSettings );

    // if the size is 0 then use the parent's size
    if ( startingSize.x == 0 || startingSize.y == 0 )
      m_window.setSize( m_impl->getParentWindowSize());
    else
      m_window.setSize( startingSize );

    // notify successful window creation here
    LOG_DEBUG( "created embedded window" );

    m_embeddedWindowEvent.onWindowCreated( *this, m_window );
  }
  else
    LOG_ERROR( "embedded window is invalid" );
}

////////////////////////////////////////////////////////////
// PUBLIC
EmbeddedWindow::~EmbeddedWindow()
{
  delete m_impl;
}

////////////////////////////////////////////////////////////
// PUBLIC
[[nodiscard]]
WindowHandle EmbeddedWindow::getSystemHandle() const
{
  return m_impl->getNativeHandle();
}

////////////////////////////////////////////////////////////
// PUBLIC
[[nodiscard]]
WindowHandle EmbeddedWindow::getParentSystemHandle() const
{
  return m_impl->getParentNativeHandle();
}

////////////////////////////////////////////////////////////
// PUBLIC
[[nodiscard]]
sf::Vector2u EmbeddedWindow::getParentWindowSize() const
{
  return m_impl->getParentWindowSize();
}

////////////////////////////////////////////////////////////
// PUBLIC
[[nodiscard]]
int EmbeddedWindow::getNativeTitlebarHeight() const
{
  return m_impl->getNativeTitlebarHeight();
}

////////////////////////////////////////////////////////////
// PUBLIC
[[nodiscard]]
uint32_t EmbeddedWindow::getPollRateInMS() const
{
  return m_impl->getPollRateInMS();
}

////////////////////////////////////////////////////////////
// PUBLIC
sf::Vector2i EmbeddedWindow::getRelativeWindowPosition() const
{
  return m_impl->getRelativeWindowPosition();
}

////////////////////////////////////////////////////////////
// PUBLIC
[[nodiscard]]
sf::Vector2i EmbeddedWindow::getCursorPosition() const
{
  return m_impl->getCursorPosition();
}

////////////////////////////////////////////////////////////
// PROTECTED
void EmbeddedWindow::onObservation( E_EmbeddedWindowEventState state )
{
  switch ( state )
  {
    case E_WindowCreated:
      // cannot call this because the impl is null.
      // must call this AFTER everything has been created successfully
      // m_embeddedWindowEvent.onWindowCreated( *this, m_window );
      break;

    case E_FrameReady:
      m_embeddedWindowEvent.onFrame( *this, m_window );
      break;

    case E_WindowDestroyed:
      m_embeddedWindowEvent.onWindowDestroyed( *this, m_window );
      break;

    default:
      m_embeddedWindowEvent.onError();
      break;
  }
}

}