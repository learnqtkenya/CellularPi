# CellularPi

A modern Qt6-based cellular modem testing interface with SMS capabilities and REST API testing functionality. This application provides a user-friendly GUI for controlling cellular modems and testing internet connectivity on Linux systems.

![SMS tab](docs/SMS_tab.png)
![Internet tab](docs/Internet_tab.png)
![Cellular Pi](docs/Cellular_pi.jpeg)

## Blogs

1. [Cellular Pi: part 1]()
2. [Cellular Pi: par 2]()
3. [Cellular Pi: part 3]()

## Features

### SMS Management
- Send SMS messages through cellular modem
- Real-time delivery status updates
- Message length tracking and validation
- Queue management for multiple messages
- Automatic retry mechanism
- Error handling and user feedback

### Internet Connectivity
- REST API testing interface
- Support for common HTTP methods (GET, POST, PUT, DELETE)
- JSON request/response visualization
- SSL/TLS support
- Built-in demo API integration

### User Interface
- Modern, responsive design
- Universal theme support
- Dynamic scaling for different screen sizes
- Tabbed interface for organized functionality
- Real-time status updates

## Requirements

### System Requirements
- Linux-based operating system
- ModemManager service installed and running
- Qt 6.7.0 or later
- CMake 3.16 or later
- C++17 compliant compiler

### Qt Modules Required
- Qt Core
- Qt Quick
- Qt Network
- Qt DBus
- Qt Concurrent

## Cross compilation

Refer to this project([QTonRaspberryPi](https://github.com/learnqtkenya/QTonRaspberryPi))for cross-compiling this project for pi.

## Project Structure

```
CellularPi/
├── CMakeLists.txt              # Main CMake configuration
├── Modem/                      # Modem management module
│   ├── CMakeLists.txt
│   ├── modem.h/cpp            # Core modem functionality
│   └── modemdbusmanager.h/cpp # D-Bus communication
├── REST/                       # REST client module
│   ├── CMakeLists.txt
│   └── restclient.h/cpp       # REST API functionality
├── Qml/                       # QML interface files
│   ├── CMakeLists.txt
│   └── Main.qml              # Main application window
└── README.md
```

## Usage

### SMS Functionality
1. Launch the application
2. Enter the recipient's phone number in international format (e.g., +1234567890)
3. Type your message (160 characters max)
4. Click "Send SMS" to transmit

### REST API Testing
1. Switch to the "Internet" tab
2. Select an endpoint from the dropdown or enter a custom one
3. Use the GET/POST buttons to make requests
4. View formatted responses in the response area
5. Test custom APIs by modifying the base URL

## Technical Details

### Modem Integration
- Uses ModemManager's D-Bus interface
- Asynchronous message handling
- Queued message processing
- Automatic retry mechanism for failed messages

### REST Client Features
- Based on Qt 6.7's new REST client features
- Support for modern REST APIs
- JSON parsing and formatting
- Error handling and retry logic
- SSL/TLS certificate handling

### Architecture
- Modular design with separate components for modem and REST functionality
- QML for the user interface
- C++ for core functionality
- Event-driven communication between components

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Submit a pull request

Please follow our coding style and include appropriate tests.

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- Qt Company for Qt 6.7 and the REST client features
- ModemManager team for the D-Bus interface
- JSONPlaceholder for the demo API
