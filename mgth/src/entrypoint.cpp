
#include <mgth.hpp>

namespace mgth {

int start(int argc, char* argv[], int (*f)(int, char**));

} // namespace mgth

int main(const int argc, char** const argv)
{
    return mgth::start(argc, argv, mgth_main);
}

