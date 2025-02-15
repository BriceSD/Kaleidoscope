/* -*- mode: c++ -*-
 * Kaleidoscope-FocusSerial -- Bidirectional communication plugin
 * Copyright (C) 2017, 2018, 2021  Keyboard.io, Inc
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <Arduino.h>         // for delayMicroseconds
#include <HardwareSerial.h>  // for HardwareSerial
#include <stdint.h>          // for uint8_t, uint16_t

#include "kaleidoscope/Runtime.h"               // for Runtime, Runtime_
#include "kaleidoscope/device/device.h"         // for cRGB
#include "kaleidoscope/event_handler_result.h"  // for EventHandlerResult, EventHandlerResult::OK
#include "kaleidoscope/key_defs.h"              // for Key
#include "kaleidoscope/plugin.h"                // for Plugin
// -----------------------------------------------------------------------------
// Deprecation warning messages
#include "kaleidoscope_internal/deprecations.h"  // for DEPRECATED

#define _DEPRECATED_MESSAGE_FOCUS_HANDLEHELP                      \
  "The `Focus.handleHelp()` method is deprecated. Please use\n"   \
  "`Focus.inputMatchesHelp()` and `Focus.printHelp()` instead.\n" \
  "This method will be removed after 2022-12-26."
// -----------------------------------------------------------------------------

// IWYU pragma: no_include "WString.h"

namespace kaleidoscope {
namespace plugin {
class FocusSerial : public kaleidoscope::Plugin {
 public:
  static constexpr char COMMENT   = '#';
  static constexpr char SEPARATOR = ' ';
  static constexpr char NEWLINE   = '\n';

  void manageFlowControl();

#ifndef NDEPRECATED
  DEPRECATED(FOCUS_HANDLEHELP)
  bool handleHelp(const char *input, const char *help_message);
#endif

  bool inputMatchesHelp(const char *input);
  bool inputMatchesCommand(const char *input, const char *expected);

  EventHandlerResult printHelp() {
    return EventHandlerResult::OK;
  }
  template<typename... Vars>
  EventHandlerResult printHelp(const char *h1, Vars... vars) {
    Runtime.serialPort().println((const __FlashStringHelper *)h1);
    delayAfterPrint();
    return printHelp(vars...);
  }

  EventHandlerResult sendName(const __FlashStringHelper *name) {
    Runtime.serialPort().print(name);
    delayAfterPrint();
    Runtime.serialPort().println();
    delayAfterPrint();
    return EventHandlerResult::OK;
  }

  void send() {}
  void send(const cRGB color) {
    send(color.r, color.g, color.b);
  }
  void send(const Key key) {
    send(key.getRaw());
  }
  void send(const bool b) {
    printBool(b);
    delayAfterPrint();
    Runtime.serialPort().print(SEPARATOR);
    delayAfterPrint();
  }
  template<typename V>
  void send(V v) {
    Runtime.serialPort().print(v);
    delayAfterPrint();
    Runtime.serialPort().print(SEPARATOR);
    delayAfterPrint();
  }
  template<typename Var, typename... Vars>
  void send(Var v, Vars... vars) {
    send(v);
    send(vars...);
  }

  void sendRaw() {}
  template<typename Var, typename... Vars>
  void sendRaw(Var v, Vars... vars) {
    Runtime.serialPort().print(v);
    delayAfterPrint();
    sendRaw(vars...);
  }

  const char peek() {
    manageFlowControl();
    return Runtime.serialPort().peek();
  }

  void read(Key &key) {
    manageFlowControl();
    key.setRaw(Runtime.serialPort().parseInt());
  }
  void read(cRGB &color) {
    manageFlowControl();
    color.r = Runtime.serialPort().parseInt();
    color.g = Runtime.serialPort().parseInt();
    color.b = Runtime.serialPort().parseInt();
  }
  void read(char &c) {
    manageFlowControl();
    Runtime.serialPort().readBytes(&c, 1);
  }
  void read(uint8_t &u8) {
    manageFlowControl();
    u8 = Runtime.serialPort().parseInt();
  }
  void read(uint16_t &u16) {
    manageFlowControl();
    u16 = Runtime.serialPort().parseInt();
  }


  bool isEOL();

  /* Hooks */
  EventHandlerResult afterEachCycle();
  EventHandlerResult onFocusEvent(const char *input);

 private:
  static constexpr char XOFF                     = 0x13;
  static constexpr char XON                      = 0x11;
  static constexpr uint8_t RECV_BUFFER_RESUME    = 4;
  static constexpr uint8_t RECV_BUFFER_THRESHOLD = 32;

  char input_[32];
  uint8_t buf_cursor_ = 0;
  void printBool(bool b);

  // This is a hacky workaround for the host seemingly dropping characters
  // when a client spams its serial port too quickly
  // Verified on GD32 and macOS 12.3 2022-03-29
  static constexpr uint8_t focus_delay_us_after_character_ = 100;
  void delayAfterPrint() { delayMicroseconds(focus_delay_us_after_character_); }
};

}  // namespace plugin
}  // namespace kaleidoscope

extern kaleidoscope::plugin::FocusSerial Focus;
