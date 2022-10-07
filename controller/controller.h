#ifndef CONTROLLER_CONTROLLER_H
#define CONTROLLER_CONTROLLER_H

#include <cstdarg>
// #include <cstddef>
// #include <cstdint>
#include <utility>

#include "io/conversion.h"

class Controller {
  using FORMAT = const char *;

 public:
  enum class ParseState {
    WAIT_COMMAND = 0,
    WAIT_PARAM,
    WAIT_EOL
  };

  auto static get() -> Controller &;

  Controller();

  auto read_buffer() -> std::pair<uint8_t *, size_t>;
  auto read_done(size_t length) -> void;
  auto write_buffer() -> std::pair<const char *, size_t>;
  auto write_done(size_t length) -> void;

  auto putc(char c) -> bool;
  auto printf(FORMAT format...) -> int;
  auto vprintf(FORMAT format, va_list args) -> int;

 private:
  static constexpr size_t OUTPUT_BUFFER_SIZE = 2048;
  static constexpr size_t RX_BUFFER_SIZE = 64 + 1;

  static Conversion conversion_;
  static char output_buffer_[OUTPUT_BUFFER_SIZE];
  static uint8_t rx_buffer_[RX_BUFFER_SIZE];

  auto parse(char c) -> void;

  ParseState parse_state_;
  size_t tx_index_;
  size_t tx_sent_;
};

#endif  // CONTROLLER_CONTROLLER_H
