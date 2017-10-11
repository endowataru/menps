
#include <mgult/sm.hpp>
#include <mgbase/external/fmt.hpp>

namespace /*unnamed*/ {

int g_val_x = 0, g_val_y = 0;

void f()
{
    while (g_val_x < 100) {
        ++g_val_x;
        while (g_val_x != g_val_y) {
            mgult::sm::this_thread::yield();
        }
        fmt::print("x {}\n", g_val_x);
    }
}

} // unnamed namespace

int main()
{
    mgult::sm::initializer init;
    
    mgult::sm::thread t(f);
    
    while (g_val_y < 100) {
        while (g_val_x == g_val_y) {
            mgult::sm::this_thread::yield();
        }
        ++g_val_y;
        fmt::print("y {}\n", g_val_y);
    }
    
    t.join();
    
    return 0;
}

