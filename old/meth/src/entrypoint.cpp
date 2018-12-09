
#include <menps/meth.hpp>

namespace menps {
namespace meth {

int start(int argc, char* argv[], int (*f)(int, char**));

} // namespace meth
} // namespace menps

int main(const int argc, char** const argv)
{
    return menps::meth::start(argc, argv, meth_main);
}

