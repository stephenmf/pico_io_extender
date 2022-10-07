#include <stdio.h>

#include "config.h"
#include "controller/controller.h"
#include "io/console.h"

namespace {

auto controller_io(uint8_t io, Controller &controller) -> void {
  if (tud_cdc_n_write_available(io)) {
    auto [out, size] = controller.write_buffer();
    auto sent = tud_cdc_n_write(io, out, size);
    controller.write_done(sent);
    tud_cdc_n_write_flush(io);
  }
  if (tud_cdc_n_available(io)) {
    auto [in, size] = controller.read_buffer();
    auto read = tud_cdc_n_read(io, in, size);
    controller.read_done(read);
  }
}

auto debug_io(uint8_t io, Console &console) -> void {
  if (tud_cdc_n_write_available(io)) {
    auto [out, size] = console.write_buffer();
    auto sent = tud_cdc_n_write(io, out, size);
    console.write_done(sent);
    tud_cdc_n_write_flush(io);
  }
  if (tud_cdc_n_available(io)) {
    auto [in, size] = console.read_buffer();
    auto read = tud_cdc_n_read(io, in, size);
    console.read_done(read);
  }
}

auto periodic(Console &console, Controller &controller) -> void {
  tud_task();
  controller_io(0, controller);
  debug_io(1, console);
}

// int64_t alarm_callback(alarm_id_t id, void *user_data) {
//   // Put your timeout handler code in here
//   return 0;
// }

void setup_io() {
  static uart_inst_t *uart_inst;

  bi_decl(bi_1pin_with_name(LED_PIN, "LED"));
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);

  bi_decl(bi_2pins_with_func(UART_TX_PIN, UART_TX_PIN, GPIO_FUNC_UART));
  uart_inst = uart_get_instance(UART_DEV);
  stdio_uart_init_full(uart_inst, CFG_BOARD_UART_BAUDRATE, UART_TX_PIN,
                       UART_RX_PIN);

  // SPI initialisation. This example will use SPI at 1MHz.
  spi_init(SPI_PORT, 1000 * 1000);
  gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
  gpio_set_function(PIN_CS, GPIO_FUNC_SIO);
  gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
  gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

  // Chip select is active-low, so we'll initialise it to a driven-high state
  gpio_set_dir(PIN_CS, GPIO_OUT);
  gpio_put(PIN_CS, 1);

  // I2C Initialisation. Using it at 400Khz.
  i2c_init(I2C_PORT, 400 * 1000);

  gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
  gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
  gpio_pull_up(I2C_SDA);
  gpio_pull_up(I2C_SCL);

  tusb_init();
}

static auto help(Console &console, Console::CommandLine &tokens) -> void {
  console.printf("Greenhouse controller help:\r\n");
  console.printf("    help    - This help.\r\n");
}

Console::Command commands[] = {{"help", help}, {nullptr, nullptr}};
const char *banner = "\r\n\r\nUSB Greenhouse controller\r\n";

}  // namespace

int main() {
  setup_io();
  auto &console = Console::get(banner, commands);
  auto &controller = Controller::get();

  // Timer example code - This example fires off the callback after 2000ms
  // add_alarm_in_ms(2000, alarm_callback, NULL, false);

  puts(banner);

  for (;;) {
    periodic(console, controller);
  }
  return 0;
}
