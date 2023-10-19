#include "EmbeddedWindowImpl.hpp"

#ifdef WIN32
#include "EmbeddedWindowImplWin32.hpp"
using EmbeddedWindowImplType = sf::priv::EmbeddedWindowImplWin32;
#endif

namespace sf::priv
{

EmbeddedWindowImpl * EmbeddedWindowImpl::create(
  ::sf::WindowHandle parentHandle,
  const std::function< void( E_EmbeddedWindowEventState ) >& observer )
{
  return new EmbeddedWindowImplType( parentHandle, observer );
}

}