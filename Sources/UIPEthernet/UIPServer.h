/*
 UIPServer.h - Arduino implementation of a uIP wrapper class.
 Copyright (c) 2013 Norbert Truchsess <norbert.truchsess@t-online.de>
 All rights reserved.

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
  */
#ifndef UIPSERVER_H
#define UIPSERVER_H
#include <Sources/_config.h>

#ifdef ETHERNET_EN

#include "ethernet_comp.h"
#if defined(ARDUINO)
  #if defined(__RFduino__)
    #include "Print.h"
  #else
    #include "Print.h"
  #endif
  #if defined(__STM32F3__) || (!defined(ARDUINO_ARCH_STM32) && defined(STM32F3)) || defined(__RFduino__)
    #include "mbed/Server.h"
  #else
    #include "Server.h"
  #endif
#endif
#if defined(__MBED__)
#endif
  #include "Sources/UIPEthernet/mbed/Print.h"
  #include "Sources/UIPEthernet/mbed/Server.h"
#include "UIPClient.h"

#if defined(ARDUINO) && (defined(ARDUINO_ARCH_STM32) || !defined(STM32F3)) && !defined(__RFduino__)
  class UIPServer : public Server {
#endif
#if defined(__MBED__) || (!defined(ARDUINO_ARCH_STM32) && defined(STM32F3)) || defined(__RFduino__)
#endif
  class UIPServer : public Print, public Server {
public:
  UIPServer(uint16_t);
  UIPClient available();
  UIPClient accept();
  virtual void begin();
  virtual void begin(uint16_t port);
  void end();
  operator bool();

  virtual size_t write(uint8_t);
  virtual size_t write(const uint8_t *buf, size_t size);

  using Print::write;

private:
  uint16_t _port;
  bool listening = false;
};

#endif
#endif
