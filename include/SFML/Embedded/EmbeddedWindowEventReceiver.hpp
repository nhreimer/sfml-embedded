#pragma once

namespace sf
{

class RenderWindow;

class EmbeddedWindowEventReceiver
{
public:

  ////////////////////////////////////////////////////////////
  /// \brief Called whenever a render window is first created (just before the message pump)
  /// \param embeddedWindow the EmbeddedWindow that manages sf::RenderWindow lifetime
  /// \param window the sf::RenderWindow created
  ////////////////////////////////////////////////////////////
  virtual void onWindowCreated( const EmbeddedWindow& embeddedWindow, RenderWindow& window ) = 0;

  ////////////////////////////////////////////////////////////
  /// \brief Called during the destruction of sf::RenderWindow (just before)
  /// \param embeddedWindow the EmbeddedWindow that manages sf::RenderWindow lifetime
  /// \param window the sf::RenderWindow to be destroyed
  ////////////////////////////////////////////////////////////
  virtual void onWindowDestroyed( const EmbeddedWindow& embeddedWindow, RenderWindow& window ) = 0;

  ////////////////////////////////////////////////////////////
  /// \brief Called whenever an error occurs. This indicates that a window cannot be created.
  ////////////////////////////////////////////////////////////
  virtual void onError() = 0;

  ////////////////////////////////////////////////////////////
  /// \brief Called whenever the next frame should be processed.
  ///
  ///  The entire event loop should be processed, i.e.,
  ///  polling until all events are cleared and rendering.
  ///  This gets called on the same thread that sf::RenderWindow was created
  ///
  /// \param embeddedWindow the EmbeddedWindow that manages sf::RenderWindow lifetime
  /// \param window the sf::RenderWindow
  ////////////////////////////////////////////////////////////
  virtual void onFrame( const EmbeddedWindow& embeddedWindow, RenderWindow& window ) = 0;

};

}