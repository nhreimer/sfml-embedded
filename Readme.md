# sfml-embedded

## what is it

it's an extension to SFML that creates embedded render windows (sf::RenderWindow). It primarily targets audio plugins (VSt3, CLAP), where a parent window is handed off to a plugin, so that a child window can be attached to it.

## dependencies

There are no dependencies outside of SFML

## build

It's currently only a static Windows library. run cmake
```bash
cmake -DSFML_DIR=/path/to/sfml/cmake/files .
```

## logging

Because stderr isn't available, an optional built-in logger is provided via spdlog.

It can be enabled by using:

```cmake
cmake -DSPDLOG_DIR=/path/to/spdlog/include
add_definitions( -DUSE_LOGGING )
```

```c++
// before any logging can be used, it must be initialized
sf::EmbeddedLogger::initializeLogger( "/path/to/log/file.txt" );

// now you can use all the logging macros
LOG_ERROR( "check out this variable {}", myVar );
```

## VST3 Example

### create the IPluginView, which sets up our embedded window

```C++
class VST3EmbeddedWindow : public Steinberg::CPluginView
{
public:

  explicit VST3EmbeddedWindow( Steinberg::ViewRect viewRect )
  {
    this->rect = viewRect;
  }

  Steinberg::tresult isPlatformTypeSupported( Steinberg::FIDString type ) override
  {
    // Windows platform
    if ( strcmp( type, Steinberg::kPlatformTypeHWND ) == 0 )
      return Steinberg::kResultTrue;

    return Steinberg::kResultFalse;
  }

  Steinberg::tresult attached( void *parent, Steinberg::FIDString type ) override
  {
    m_ptrChildWindow =
      std::make_unique< sf::EmbeddedWindow >( (sf::WindowHandle )parent,
                                              m_eventReceiver,
                                              sf::ContextSettings { 0, 0, 2, 4, 6 } );

    return Steinberg::kResultTrue;
  }

private:

  std::unique_ptr< sf::EmbeddedWindow > m_ptrChildWindow;
  VST3EmbeddedWindowEventReceiver m_eventReceiver;

};
```

### create the event receiver / callback class
```C++
class VST3EmbeddedWindowEventReceiver : public sf::EmbeddedWindowEventReceiver
{
public:

  void onWindowCreated( const sf::EmbeddedWindow& emWin, sf::RenderWindow &window ) override
  {
    m_shape.setSize( { 50, 50 } );
    m_timer.restart();
  }

  void onWindowDestroyed( const sf::EmbeddedWindow&, sf::RenderWindow &window ) override
  {}

  void onError() override
  {}

  void onFrame( const sf::EmbeddedWindow& embeddedWindow, sf::RenderWindow &window ) override
  {
    if ( m_timer.getElapsedTime().asSeconds() > 5 )
    {
      if ( m_shape.getFillColor() == sf::Color::Green )
        m_shape.setFillColor( sf::Color::White );
      else
        m_shape.setFillColor( sf::Color::Green );

      m_timer.restart();
    }

    sf::Event event {};
    while ( window.isOpen() && window.pollEvent( event ) )
    {
      if ( event.type == sf::Event::Closed )
        break;

      if ( event.type == sf::Event::MouseButtonPressed )
      {
        auto relPosition = embeddedWindow.getCursorPosition();

        if ( m_shape.getGlobalBounds().contains( ( float )relPosition.x, ( float )relPosition.y ) )
        {
          if ( m_shape.getFillColor() == sf::Color::Magenta )
            m_shape.setFillColor( sf::Color::Cyan );
          else
            m_shape.setFillColor( sf::Color::Magenta );
          m_timer.restart();
        }
      }
    }

    window.clear( sf::Color( 32, 32, 32 ) );
    window.draw( m_shape );
    window.display();
  }

private:

  sf::RectangleShape m_shape;
  sf::Clock m_timer;

};
```
## SFML Window Example

### initialization

```c++
int main()
{
  // set up console logging
  sf::EmbeddedLogger::initializeConsole();
  sf::Window window;

  // create our parent window
  window.create( sf::VideoMode { 800, 600 }, "Parent Window" );
  window.setFramerateLimit( 60 );

  // embedded window #1
  nx::EventReceiver eventReceiver( sf::Color::Cyan );
  sf::EmbeddedWindow emWin( window.getSystemHandle(), eventReceiver );

  // embedded window #2
  nx::EventReceiver eventReceiver2( sf::Color::Magenta );
  sf::EmbeddedWindow emWin2( window.getSystemHandle(), eventReceiver2 );

  while ( window.isOpen() )
  {
    sf::Event event {};
    while ( window.pollEvent( event ) )
    {
      if ( event.type == sf::Event::Closed )
        return 0;
    }
    
    // prevent flickering by only update as needed
    if ( nx::g_update )
    {
      window.display();
      nx::g_update = false;
    }
  }

  return 0;
}
```

### set up event receiver

```c++
namespace nx
{

bool g_update = false;

class EventReceiver : public sf::EmbeddedWindowEventReceiver
{
public:

  explicit EventReceiver( const sf::Color& bgColor )
    : m_winId( m_winCounter ),
      m_bgColor( bgColor )
  {
    ++m_winCounter;
  }

  ~EventReceiver()
  {
    --m_winCounter;
  }

  void onWindowCreated( const sf::EmbeddedWindow &embeddedWindow, sf::RenderWindow &window ) override
  {
    std::cout << "created window\n";
    window.setActive();
    window.requestFocus();
    updateClientSize( embeddedWindow );
  }

  void onWindowDestroyed( const sf::EmbeddedWindow &embeddedWindow, sf::RenderWindow &window ) override
  {
    std::cout << "destroying window\n";
  }

  void onError() override
  {
    std::cerr << "failed to create window\n";
  }

  void onFrame( const sf::EmbeddedWindow &embeddedWindow, sf::RenderWindow &window ) override
  {
    sf::Event event {};
    while ( window.isOpen() && window.pollEvent( event ) )
    {
      if ( event.type == sf::Event::Closed )
        return;

      if ( event.type == sf::Event::MouseButtonReleased )
        updateClientSize( embeddedWindow );
    }

    window.clear( m_bgColor );
    window.display();
  }

private:

  void updateClientSize( const sf::EmbeddedWindow& embeddedWindow ) const
  {
    // parent window size
    ::RECT parentPosRect;
    // ::GetClientRect( embeddedWindow.getParentNativeHandle(), &rect );
    ::GetWindowRect( embeddedWindow.getParentSystemHandle(), &parentPosRect );

    auto parentSize = sf::Vector2u { ( uint32_t )parentPosRect.right - parentPosRect.left,
                                     ( uint32_t )parentPosRect.bottom - parentPosRect.top };

    ::RECT childPosRect;
    ::GetWindowRect( embeddedWindow.getSystemHandle(), &childPosRect );

    auto childSize = sf::Vector2u { ( uint32_t )childPosRect.right - childPosRect.left,
                                    ( uint32_t )childPosRect.bottom - childPosRect.top };

//        std::cout << +m_winId << ": " << size.x << ", " << size.y
//                  << " -> " << parentPosRect.left << ", " << parentPosRect.top << ", " << parentPosRect.right << ", " << parentPosRect.bottom
//                  << " -> " << childPosRect.left << ", " << childPosRect.top << ", " << childPosRect.right << ", " << childPosRect.bottom
//                  << std::endl;

    auto proposedX = ( float )( parentPosRect.right - parentPosRect.left ) / ( float )m_winCounter;
    sf::Vector2u newPosition = { ( uint32_t )proposedX * ( m_winId - 1 ), parentSize.y };

    //           409                  0                        647
//    std::cout << proposedX << ": " << newPosition.x << ", " << newPosition.y << std::endl;
//    std::cout << parentSize.x << " vs " << childSize.x << std::endl;

    if ( childSize.x > ( uint32_t )proposedX )
    {
      // TODO: find how to use positions to set
      // TODO: request native refreshes?
      //::SetWindowPos( embeddedWindow.getNativeHandle(), NULL, 0, 0, 409, 647, 0 );
      ::SetWindowPos( embeddedWindow.getSystemHandle(),
                      nullptr,
                      ( int )newPosition.x, 0,
                      ( int )proposedX, ( int )newPosition.y,
                      0 );

      g_update = true;
    }
  }

private:

  uint8_t m_winId;
  sf::Color m_bgColor;
  static inline std::atomic< uint8_t > m_winCounter { 1 };

};
}
```

## License

Same license as SFML [zlib/libpng license](https://opensource.org/licenses/Zlib).



