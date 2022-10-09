#include <stdio.h>

#include "app/app.h"
#include "io/framework.h"

namespace {

static auto help(Console& console, Console::CommandLine& tokens) -> void {
  console.printf("Greenhouse controller help:\r\n");
  console.printf("    help    - This help.\r\n");
}

Console::Command commands[] = {{"help", help}, {nullptr, nullptr}};
const char* banner = "\r\n\r\nUSB Greenhouse controller\r\n";

}  // namespace

int main() {
  auto* framework = Framework::get(banner, commands);
  static auto app = App{};
  framework->app(&app);
  framework->init();

  puts(banner);

  for (;;) {
    framework->periodic();
  }
  return 0;
}
