include($$PWD/tcp/SAKTcp.pri)
include($$PWD/udp/SAKUdp.pri)
#include($$PWD/ble/SAKBle.pri)
include($$PWD/test/SAKTest.pri)
include($$PWD/websocket/SAKWebSocket.pri)
include($$PWD/serialport/SAKSerialPort.pri)
include($$PWD/canbus/SAKCanBus.pri)
include($$PWD/modbus/SAKModbus.pri)
#----------------------------------------------------------------------------------------
include($$PWD/debugger/SAKDebugger.pri)

HEADERS += \
    $$PWD/SAKDebuggerFactory.hh

SOURCES += \
    $$PWD/SAKDebuggerFactory.cc

INCLUDEPATH += \
    $$PWD
