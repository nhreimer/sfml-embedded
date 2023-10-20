#pragma once

#include <memory>

#include <SFML/Graphics/RenderWindow.hpp>
#include "EmbeddedWindowEventState.hpp"

// forward declaration
namespace sf::priv
{
class EmbeddedWindowImpl;
}

namespace sf
{

// forward declaration
class EmbeddedWindowEventReceiver;

class EmbeddedWindow
{
public:

  /// \brief Constructs an EmbeddedWindow object
  /// \param parentHandle Native handle of the parent window
  EmbeddedWindow( WindowHandle parentHandle,
                  EmbeddedWindowEventReceiver& embeddedWindowEvent );


  /// \brief Constructs an EmbeddedWindow object
  /// \param parentHandle Native handle of the parent window
  /// \param embeddedWindowEvent Event recipient (used for event callbacks)
  /// \param contextSettings SFML context settings
  EmbeddedWindow( WindowHandle parentHandle,
                  EmbeddedWindowEventReceiver& embeddedWindowEvent,
                  sf::ContextSettings contextSettings );

  /// \brief Constructs an EmbeddedWindow object
  /// \param parentHandle Native handle of the parent window
  /// \param embeddedWindowEvent Event recipient (used for event callbacks)
  /// \param contextSettings SFML context settings
  /// \param startingSize starting window size (default is parent's window size)
  EmbeddedWindow( WindowHandle parentHandle,
                  EmbeddedWindowEventReceiver& embeddedWindowEvent,
                  sf::ContextSettings contextSettings,
                  const sf::Vector2u& startingSize );

  /// \brief destroys the platform-specific embedded window
  virtual ~EmbeddedWindow();

  /// \brief gets the native handle of the embedded window
  [[nodiscard]]
  WindowHandle getSystemHandle() const;

  /// \brief gets the native handle of the embedded window's parent
  [[nodiscard]]
  WindowHandle getParentSystemHandle() const;

  /// \brief gets the parent's window size
  [[nodiscard]]
  sf::Vector2u getParentWindowSize() const;

  /// \brief attempts to retrieve the size of a system's titlebar
  [[nodiscard]]
  int getNativeTitlebarHeight() const;

  /// \brief gets the refresh/update rate (will differ by platform)
  [[nodiscard]]
  uint32_t getPollRateInMS() const;

  /// \brief gets the embedded window location relative to the parent
  [[nodiscard]]
  sf::Vector2i getRelativeWindowPosition() const;

  /// \brief gets the cursor position relative to the embedded window
  [[nodiscard]]
  sf::Vector2i getCursorPosition() const;

protected:

  /// \brief Construct the child window and attach it to a parent control
  ///
  /// \param state Platform-specific handle of the parent control
  virtual void onObservation( E_EmbeddedWindowEventState state );

private:

  // the event callback associated with this window
  EmbeddedWindowEventReceiver& m_embeddedWindowEvent;

  // platform-specific implementation of child window
  priv::EmbeddedWindowImpl * m_impl { nullptr };

  // the SFML render window as a child window
  sf::RenderWindow m_window;
};

}