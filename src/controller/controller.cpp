#include "controller/controller.h"

namespace {

auto perform_command(Controller::Command command, unsigned param1,
                     unsigned param2) -> void {
  auto &controller = Controller::get();
  switch (command) {
    case Controller::Command::STATUS:
      controller.printf("{\"led\":\"undefined\"}\r\n");
      break;
    case Controller::Command::UPDATE_LED:
      controller.printf("{\"led\":%s}\r\n", (param2) ? "true" : "false");
      break;
  }
}

}  // namespace

auto Controller::get() -> Controller & {
  static Controller controller{};
  return controller;
}

Controller::Controller()
    : state_{State::WAIT_COMMAND},
      command_{Command::STATUS},
      param1_{0},
      param2_{0},
      tx_index_{0},
      tx_sent_{0} {}

auto Controller::read_buffer() -> std::pair<uint8_t *, size_t> {
  return std::pair<uint8_t *, size_t>{rx_buffer_, sizeof(rx_buffer_)};
}

auto Controller::read_done(size_t length) -> void {
  for (auto i = 0U; i < length; ++i) {
    auto c = rx_buffer_[i];
    parse(c);
  }
}

auto Controller::write_buffer() -> std::pair<const char *, size_t> {
  size_t size = 0;
  size_t send_index = tx_sent_;
  // If buffer has been wrapped this is true.
  if (tx_index_ < tx_sent_)
    size = sizeof(output_buffer_) - tx_sent_;
  else
    size = tx_index_ - tx_sent_;
  return std::pair<const char *, size_t>{&output_buffer_[send_index], size};
}

auto Controller::write_done(size_t length) -> void {
  size_t new_sent = tx_sent_ + length;
  if (new_sent >= sizeof(output_buffer_)) new_sent = 0;
  tx_sent_ = new_sent;
}

auto Controller::putc(char c) -> bool {
  auto new_index = tx_index_ + 1;
  // wrap the buffer.
  if (new_index >= sizeof(output_buffer_)) new_index = 0;
  // Check for buffer full.
  if (new_index == tx_sent_) return false;
  output_buffer_[tx_index_] = c;
  tx_index_ = new_index;
  return true;
}

auto Controller::printf(FORMAT format...) -> int {
  va_list args;
  va_start(args, format);
  int result = vprintf(format, args);
  va_end(args);
  return result;
}

auto Controller::vprintf(FORMAT format, va_list args) -> int {
  int result;
  bool room = true;
  while (room && *format != '\0') {
    if (*format == '%') {
      ++format;
      auto type = conversion_.parse(format);
      if (type == Conversion::Type::UNKNOWN) break;
      const char *buffer = nullptr;
      switch (type) {
        case Conversion::Type::PERCENT: {
          buffer = conversion_.from_character('%');
        } break;
        case Conversion::Type::CHARACTER: {
          auto value = va_arg(args, int);
          buffer = conversion_.from_character(value);
        } break;
        case Conversion::Type::SIGNED_INT: {      auto& controller = Controller::get();

          auto value = va_arg(args, int);
          buffer = conversion_.from_signed_int(value);
        } break;
        case Conversion::Type::UNSIGNED_INT: {
          auto value = va_arg(args, unsigned);
          buffer = conversion_.from_unsigned_int(value);
        } break;
        case Conversion::Type::LONG_SIGNED_INT: {
          auto value = va_arg(args, long);
          buffer = conversion_.from_signed_int(value);
        } break;
        case Conversion::Type::LONG_UNSIGNED_INT: {
          auto value = va_arg(args, unsigned long);
          buffer = conversion_.from_unsigned_int(value);
        } break;
        case Conversion::Type::LONG_LONG_SIGNED_INT: {
          auto value = va_arg(args, long long);
          buffer = conversion_.from_signed_int(value);
        } break;
        case Conversion::Type::LONG_LONG_UNSIGNED_INT: {
          auto value = va_arg(args, unsigned long long);
          buffer = conversion_.from_unsigned_int(value);
        } break;
        case Conversion::Type::POINTER: {
          auto value = (uint32_t)va_arg(args, void *);
          buffer = conversion_.from_unsigned_int(value);
        } break;
        case Conversion::Type::DOUBLE: {
          auto value = va_arg(args, double);
          buffer = conversion_.from_double(value);
        } break;
        case Conversion::Type::STRING: {
          auto value = va_arg(args, char *);
          buffer = conversion_.from_string(value);
        } break;
        default:
          break;
      }
      if (buffer) {
        while (room && *buffer != '\0') room = putc(*buffer++);
      }
    } else {
      room = putc(*format++);
      ++result;
    }
  }
  return result;
}

auto Controller::parse(char c) -> void {
  switch (state_) {
    case State::WAIT_COMMAND:
      command(c);
      break;
    case State::WAIT_PARAM1:
      wait_param1(c);
      break;
    case State::COLLECT_PARAM1:
      collect_param1(c);
      break;
    case State::WAIT_PARAM2:
      wait_param2(c);
      break;
    case State::COLLECT_PARAM2:
      collect_param2(c);
      break;
  }
  printf("state: %d response: 0x%02X\r\n", state_, (int)c);
}

auto Controller::command(char c) -> void {
  switch (c) {
    case 0x1B:
    case '\r':
    case '\n':
      state_ = State::WAIT_COMMAND;
      break;
    case 'S':
    case 's':
      command_ = Command::STATUS;
      state_ = State::COLLECT_PARAM2;
      break;
    case 'L':
    case 'l':
      command_ = Command::UPDATE_LED;
      state_ = State::WAIT_PARAM1;
      param2_ = 0;
      break;
    default:
      printf("\r\nUnrecognised command code '%c'\r\n", c);
      break;
  }
}

auto Controller::wait_param1(char c) -> void {
  switch (c) {
    case 0x1B:
      state_ = State::WAIT_COMMAND;
      break;
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      param1_ = c - '0';
      state_ = State::COLLECT_PARAM1;
      break;
    default:
      break;
  }
}

auto Controller::collect_param1(char c) -> void {
  switch (c) {
    case 0x1B:
      state_ = State::WAIT_COMMAND;
      break;
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      param1_ = (param1_ * 10) + (c - '0');
      break;
    default:
      state_ = State::WAIT_PARAM2;
      break;
  }
}

auto Controller::wait_param2(char c) -> void {
  switch (c) {
    case 0x1B:
      state_ = State::WAIT_COMMAND;
      break;
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      param2_ = c - '0';
      state_ = State::COLLECT_PARAM2;
      break;
    default:
      break;
  }
}

auto Controller::collect_param2(char c) -> void {
  switch (c) {
    case 0x1B:
      state_ = State::WAIT_COMMAND;
      break;
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      param2_ = (param2_ * 10) + (c - '0');
      break;
    default:
      printf("ack\r\n");
      perform_command(command_, param1_, param2_);
      state_ = State::WAIT_COMMAND;
      break;
  }
}

char Controller::output_buffer_[OUTPUT_BUFFER_SIZE];
uint8_t Controller::rx_buffer_[RX_BUFFER_SIZE];
Conversion Controller::conversion_;
