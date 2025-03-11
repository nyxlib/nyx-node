#pragma once

#if defined(PICO_BOARD)
#  define MG_ARCH MG_ARCH_PICOSDK
#endif

#if defined(ARDUINO)
#  if defined(ESP8266)
#    define MG_ARCH MG_ARCH_ESP8266
#  elif defined(ESP32)
#    define MG_ARCH MG_ARCH_ESP32
#  else
#    define MG_ARCH MG_ARCH_CUSTOM
#    define MG_ENABLE_DRIVER_W5500 1
#    define MG_ENABLE_SOCKET 0
#    define MG_ENABLE_TCPIP 1
#    define mkdir(a, b) (-1)
#    define MG_IO_SIZE 128
#  endif
#endif
