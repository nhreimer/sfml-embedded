#pragma once

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////

#include <Windows.h>

#include <atomic>
#include <string>
#include <functional>
#include <unordered_map>

#include <SFML/Window/WindowHandle.hpp>
#include <SFML/System/Vector2.hpp>

#include "EmbeddedWindowImpl.hpp"

#ifdef UNICODE
using PTR_STR = LPCWSTR;
#else
using PTR_STR = LPCSTR;
#endif

namespace sf::priv
{

////////////////////////////////////////////////////////////
/// \brief Windows implementation of WindowImpl
///
////////////////////////////////////////////////////////////
class EmbeddedWindowImplWin32 : public sf::priv::EmbeddedWindowImpl
{

    ////////////////////////////////////////////////////////////////////////////////
    /// WIN32 WINDOW INFO
    ////////////////////////////////////////////////////////////////////////////////
    struct Win32WinInternals
    {
        std::string classname;                   // WNDCLASS classname. must be unique.
        std::string windowname;                  //
        sf::WindowHandle parentHwnd { nullptr }; // Parent HWND of the child HWND
        sf::WindowHandle childHwnd { nullptr };  // HWND to the child
        UINT_PTR timerResult { 0 };              // timer id
    };

    // converts Win32 specifics to SFML or std equivalents
    ////////////////////////////////////////////////////////////////////////////////
    /// WIN32 WINDOW HELPER
    ////////////////////////////////////////////////////////////////////////////////
    class Win32Helper
    {
    public:
        ////////////////////////////////////////////////////////////
        /// \brief Creates a unique name by requesting a Uuid from the OS
        ///
        /// \return Uuid as a string if successful, otherwise an empty string
        ///
        ////////////////////////////////////////////////////////////
        static std::string createUniqueName();

        ////////////////////////////////////////////////////////////
        /// \brief Gets the window size from the OS
        ///
        /// \param hwnd handle to a window
        /// \return size of window if successful, otherwise a vector of { 0, 0 }
        ///
        ////////////////////////////////////////////////////////////
        static sf::Vector2u getWin32WindowSize( sf::WindowHandle hwnd );
    };

public:

    // prevent default ctor and copying
    EmbeddedWindowImplWin32() = delete;
    EmbeddedWindowImplWin32(const EmbeddedWindowImplWin32& other) = delete;
    EmbeddedWindowImplWin32& operator=(const EmbeddedWindowImplWin32& other) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Construct the child window and attach it to a parent control
    ///
    /// \param handle Platform-specific handle of the parent control
    /// \param observer callback related to state of native window
    ////////////////////////////////////////////////////////////
    EmbeddedWindowImplWin32(sf::WindowHandle parentHandle,
                            const std::function< void( E_EmbeddedWindowEventState ) >& observer );

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    ~EmbeddedWindowImplWin32() override;

    [[nodiscard]]
    sf::WindowHandle getNativeHandle() const override;

    [[nodiscard]]
    sf::WindowHandle getParentNativeHandle() const override;

    [[nodiscard]]
    sf::Vector2u getParentWindowSize() const override;

    [[nodiscard]]
    uint32_t getPollRateInMS() const override;

    [[nodiscard]]
    sf::Vector2i getRelativeWindowPosition() const override;

    [[nodiscard]]
    sf::Vector2i getCursorPosition() const override;

private:

    /////////////////////////////////////////////////////////////////////////////
    /// WINDOW CREATION AND SETUP
    /////////////////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////////////////
    bool createChildWindow( HWND parentHwnd );

    /////////////////////////////////////////////////////////////////////////////
    bool createWindow();

    /////////////////////////////////////////////////////////////////////////////
    bool registerWindowClass();

    /////////////////////////////////////////////////////////////////////////////
    bool startMessagePump();

    /////////////////////////////////////////////////////////////////////////////
    /// WINAPI CALLBACKS
    /////////////////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////////////////
    static LRESULT WINAPI processWndEvent( HWND hwnd,
                                           UINT msg,
                                           WPARAM wParam,
                                           LPARAM lParam );

    /////////////////////////////////////////////////////////////////////////////
    static void WINAPI processTimerExpiry( HWND hwnd,
                                          UINT wmTimerMsg,
                                          UINT_PTR timerId,
                                          DWORD currentSysTime );

private:

    std::function<void(E_EmbeddedWindowEventState)> m_observer;

    // holds Win32 window specifics
    Win32WinInternals m_win32;

    // required for timer callbacks across multiple instances
    // each child HWND is mapped to Win32SfmlWindow instance
    inline static std::unordered_map< HWND, EmbeddedWindowImplWin32 * > smWndMap {};

    // usually starts at 5 and goes up
    inline static std::atomic< uint32_t > smTimerIdCounter { 5 };
};

}
