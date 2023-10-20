#pragma once

#include <memory>
#include <functional>

#include <SFML/Window/WindowHandle.hpp>
#include <SFML/Graphics/Rect.hpp>

#include "../../../include/SFML/Embedded/EmbeddedWindowEventState.hpp"

namespace sf::priv
{

class EmbeddedWindowImpl
{
public:

  [[nodiscard]]
  static EmbeddedWindowImpl * create( WindowHandle parentHandle,
                                      const std::function< void( E_EmbeddedWindowEventState ) >& observer );

  virtual ~EmbeddedWindowImpl() = default;

  [[nodiscard]]
  virtual WindowHandle getNativeHandle() const { return nullptr; }

  [[nodiscard]]
  virtual WindowHandle getParentNativeHandle() const { return nullptr; }

  [[nodiscard]]
  virtual sf::Vector2u getParentWindowSize() const { return { 0, 0}; }

  [[nodiscard]]
  virtual int getNativeTitlebarHeight() const { return 0; }

  [[nodiscard]]
  virtual uint32_t getPollRateInMS() const { return UINT32_MAX; }

  [[nodiscard]]
  virtual sf::Vector2i getRelativeWindowPosition() const { return { 0, 0 }; }

  [[nodiscard]]
  virtual sf::Vector2i getCursorPosition() const
  {
    return { 0, 0 };
  }
};
}