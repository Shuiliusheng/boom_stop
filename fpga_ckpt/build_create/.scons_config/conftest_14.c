

#include "execinfo.h"

int
main() {
  char temp; backtrace_symbols_fd((void *)&temp, 0, 0);
return 0;
}
