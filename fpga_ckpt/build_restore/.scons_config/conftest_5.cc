
#include <pybind11/embed.h>

int
main(int argc, char **argv) {
    pybind11::scoped_interpreter guard{};
    pybind11::exec(
        "import sys\n"
        "vi = sys.version_info\n"
        "sys.stdout.write('%i.%i.%i' % (vi.major, vi.minor, vi.micro));\n");
    return 0;
}
    