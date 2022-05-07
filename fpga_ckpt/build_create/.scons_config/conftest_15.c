

#include "sys/mman.h"

int
main() {
  shm_open("/test", 0, 0);
return 0;
}
