////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////

#include "EmbeddedWindowImplWin32.hpp"
#include "SFML/Embedded/EmbeddedLogger.hpp"

#include <iostream>
#include <tuple>

#include <rpc.h>

//#ifdef RPC_USE_NATIVE_WCHAR
#ifdef UNICODE
using PTR_RPC_STR = RPC_WSTR*;
#else
using PTR_RPC_STR = RPC_CSTR*;
#endif

// TODO: this requires MSVC and may preclude mingw or other non-msvc windows toolchains
// BUG: from time to time, in non-embedded applications, a child window may fail when running multiple children
#ifdef _MSC_VER
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#pragma warning(disable: 4047)
HINSTANCE hImageBaseInstance = (HINSTANCE)&__ImageBase;
#pragma warning(default: 4047)
#endif

namespace sf::priv
{

////////////////////////////////////////////////////////////
// PUBLIC
EmbeddedWindowImplWin32::EmbeddedWindowImplWin32(sf::WindowHandle parentHandle,
                                           const std::function<void(E_EmbeddedWindowEventState)>& observer)
    : m_observer( observer)
{
    if (!createChildWindow(parentHandle))
    {
        // notify that an error has occurred
        m_observer(E_Error);
        LOG_ERROR( "failed to create child window" );
    }
}

////////////////////////////////////////////////////////////
// PUBLIC
EmbeddedWindowImplWin32::~EmbeddedWindowImplWin32()
{
    if ( m_win32.childHwnd != nullptr )
    {
        // notify that window is about to be destroyed
        m_observer(E_WindowDestroyed);

        // stop the callbacks
        if ( m_win32.timerResult != 0 )
        {
            ::KillTimer( m_win32.childHwnd, m_win32.timerResult );
            m_win32.timerResult = 0;
        }

        // SFML takes care of the following whenever it closes the window
        //        ::UnregisterClass( m_wndData.classname.data(), m_wndData.hInstance );
        //        ::DestroyWindow( m_wndData.childHwnd );

        auto it = smWndMap.find( m_win32.childHwnd );
        if ( it != smWndMap.end() )
            smWndMap.erase( it );
        else // something terrible has gone wrong!
            LOG_ERROR( "unable to properly shut down child HWND because it is not mapped!" );

        // shutdown can get called prior to the destructor, so mark this as invalid
        m_win32.childHwnd = nullptr;
    }
    // else: child has already been shutdown, which is odd
}

////////////////////////////////////////////////////////////
// PUBLIC
sf::WindowHandle EmbeddedWindowImplWin32::getNativeHandle() const
{
    return m_win32.childHwnd;
}

////////////////////////////////////////////////////////////
// PUBLIC
sf::WindowHandle EmbeddedWindowImplWin32::getParentNativeHandle() const
{
    return m_win32.parentHwnd;
}

////////////////////////////////////////////////////////////
// PUBLIC
sf::Vector2u EmbeddedWindowImplWin32::getParentWindowSize() const
{
  return Win32Helper::getWin32WindowSize( m_win32.parentHwnd );
}

////////////////////////////////////////////////////////////
// PUBLIC
int EmbeddedWindowImplWin32::getNativeTitlebarHeight() const
{
  return ( ::GetSystemMetrics( SM_CYFRAME ) +
           ::GetSystemMetrics( SM_CYCAPTION ) +
           ::GetSystemMetrics( SM_CXPADDEDBORDER ) );
}

////////////////////////////////////////////////////////////
// PUBLIC
unsigned int EmbeddedWindowImplWin32::getPollRateInMS() const
{
    return static_cast< unsigned int >( USER_TIMER_MINIMUM );
}

////////////////////////////////////////////////////////////
// PUBLIC
sf::Vector2i EmbeddedWindowImplWin32::getRelativeWindowPosition() const
{
  // there might be an easier way but the working model seems to be to
  // 1. get the current screen coordinates of the child window
  // 2. map those screen coordinates to the parent window
  // 3. subtract 2 - 1

  ::RECT childRect { 0 };
  if ( !::GetWindowRect( m_win32.childHwnd, &childRect ) )
  {
    LOG_ERROR( "failed to get window rect: {}", ::GetLastError() );
    return { -1, -1 };
  }

  ::RECT mapChildRect = childRect;

  // https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-mapwindowpoints
  // If the function fails, the return value is zero. Call SetLastError prior to calling this method to differentiate
  // an error return value from a legitimate "0" return value.
//  ::SetLastError( -1 );

  // this tends to fail but is successful nonetheless
  ::MapWindowPoints( m_win32.childHwnd,  m_win32.parentHwnd, (LPPOINT)&mapChildRect, 2 );


  return { mapChildRect.left - childRect.left, mapChildRect.right - childRect.right };
}

////////////////////////////////////////////////////////////
// PUBLIC
sf::Vector2i EmbeddedWindowImplWin32::getCursorPosition() const
{
  ::POINT rpos = { 0, 0 };
  ::GetCursorPos( &rpos );
  ::ScreenToClient( m_win32.childHwnd, &rpos );

  return { rpos.x, rpos.y };
}

////////////////////////////////////////////////////////////
// PRIVATE
bool EmbeddedWindowImplWin32::createChildWindow(HWND parentHwnd)
{
    m_win32.parentHwnd = parentHwnd;
    m_win32.classname = Win32Helper::createUniqueName();

    if ( m_win32.classname.empty() )
        LOG_WARN( "failed to create class name for Win32 Window. attempting to create window anyway." );

    if ( registerWindowClass() && createWindow() )
    {
        smWndMap.insert( { m_win32.childHwnd, this } );
        if ( !::AllowSetForegroundWindow( ASFW_ANY ) )
          LOG_WARN( "unable to allow foreground settings" );
        else
          ::SetForegroundWindow( m_win32.childHwnd );

        ::SetFocus( m_win32.childHwnd );

        m_observer( E_WindowCreated );

        if ( !startMessagePump() )
        {
            LOG_ERROR( "failed to start message pump." );
            return false;
        }

        // started message pump
        LOG_DEBUG( "started message pump" );
        return true;
    }

    LOG_ERROR( "failed to create child window" );
    return false;
}

/////////////////////////////////////////////////////////////////////////////
// PRIVATE
bool EmbeddedWindowImplWin32::createWindow()
{
    // https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowexw
    // CW_USEDEFAULT = 0 whenever it's a child, which will completely
    // screw with SFML's scaling. if it returns zero then the program should
    // not continue!
    auto parentWndSize = Win32Helper::getWin32WindowSize( m_win32.parentHwnd );
    if ( parentWndSize.x == 0 || parentWndSize.y == 0 )
        LOG_WARN( "parent wnd size not found. scaling issues may occur" );

    m_win32.childHwnd = ::CreateWindowEx(
        WS_EX_NOINHERITLAYOUT,
        reinterpret_cast< PTR_STR >( m_win32.classname.data() ),
        reinterpret_cast< PTR_STR >( "__innerWindowName" ),
        WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        ( int )parentWndSize.x,
        ( int )parentWndSize.y,
        m_win32.parentHwnd,
        nullptr,
        hImageBaseInstance,
        nullptr );

    if ( m_win32.childHwnd == nullptr )
    {
        LOG_ERROR( "failed to create child window. Error Code: {}", ::GetLastError() );
        return false;
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////
// PRIVATE
bool EmbeddedWindowImplWin32::registerWindowClass()
{
    const ::WNDCLASS wndClass =
        {
            CS_GLOBALCLASS | CS_DBLCLKS,
            processWndEvent,
            0,
            0,
            hImageBaseInstance,
            nullptr, nullptr, nullptr, nullptr,
            reinterpret_cast< PTR_STR >( m_win32.classname.data() )
        };

    if ( ::RegisterClass( &wndClass ) == 0 )
    {
        LOG_ERROR( "failed to register window class. Error code: {}", ::GetLastError() );
        return false;
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////
// PRIVATE
bool EmbeddedWindowImplWin32::startMessagePump()
{
    m_win32.timerResult = ::SetTimer( m_win32.childHwnd,
                                     smTimerIdCounter,
                                     USER_TIMER_MINIMUM,
                                     processTimerExpiry );

    if ( m_win32.timerResult == 0 )
    {
        LOG_ERROR( "failed to create windows timer. Error code: {}", ::GetLastError() );
        return false;
    }

    // TODO: unlikely to hit UINT32_MAX but it is theoretically possible
    ++smTimerIdCounter;
    return true;
}

/////////////////////////////////////////////////////////////////////////////
// STATIC PRIVATE
LRESULT EmbeddedWindowImplWin32::processWndEvent(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    return ::DefWindowProc( hwnd, msg, wParam, lParam );
}

/////////////////////////////////////////////////////////////////////////////
// STATIC PRIVATE
void EmbeddedWindowImplWin32::processTimerExpiry(HWND hwnd, UINT wmTimerMsg, UINT_PTR timerId, DWORD currentSysTime)
{
    std::ignore = wmTimerMsg;
    std::ignore = currentSysTime;

    auto it = smWndMap.find( hwnd );
    if ( it != smWndMap.end() )
    {
        // notify that a frame is ready to be processed
        it->second->m_observer( E_FrameReady );
    }
    else
    {
        // this means something has gone terribly wrong. events cannot be processed,
        // so kill the timer.
        LOG_ERROR( "timer is running but child HWND not found. stopping timer." );
        ::KillTimer( hwnd, timerId );
    }
}

/////////////////////////////////////////////////////////////////////////////
// PRIVATE NESTED, STATIC PUBLIC
std::string EmbeddedWindowImplWin32::Win32Helper::createUniqueName()
{
    UUID       uuid;
    const auto rpcStatus = ::UuidCreate(&uuid);

    if (rpcStatus == RPC_S_OK)
    {
        char* cszUuid = nullptr;
        const auto uuidToStringStatus = ::UuidToString(&uuid, (PTR_RPC_STR)&cszUuid);
        if (uuidToStringStatus == RPC_S_OK && cszUuid != nullptr)
        {
            // copy the UUID string before releasing the RPC buffer
            std::string str(::_strdup(cszUuid));
            ::RpcStringFree((PTR_RPC_STR)&cszUuid);
            return str;
        }
        else
            LOG_ERROR( "failed to convert UUID to string. Error code: {} ", uuidToStringStatus );
    }
    else
        LOG_ERROR( "failed to create UUID. Error code: {}", rpcStatus );

    return {};
}

/////////////////////////////////////////////////////////////////////////////
// PRIVATE NESTED, STATIC PUBLIC
sf::Vector2u EmbeddedWindowImplWin32::Win32Helper::getWin32WindowSize(sf::WindowHandle hwnd)
{
    RECT frame {};
    if ( ::GetWindowRect( hwnd, &frame ) )
    {
        return {( uint32_t )( frame.right  - frame.left ),
                ( uint32_t )( frame.bottom - frame.top  ) };
    }

    LOG_ERROR( "failed to obtain parent window size. Error code: {}", ::GetLastError() );
    // hand back an empty vector
    return sf::Vector2u {};
}
}
