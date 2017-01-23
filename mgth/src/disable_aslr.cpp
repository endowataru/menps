
#include "disable_aslr.hpp"

#include <mgbase/logger.hpp>

#if defined(MGBASE_OS_LINUX)

#include <unistd.h>
#include <sys/personality.h>

extern char** environ;

namespace mgth {

namespace /*unnamed*/ {

void exec_disable_aslr(char** const argv, char** const envp)
{
    const int old = personality(0xffffffff);
    
    const int ret = personality(old | ADDR_NO_RANDOMIZE);
    
    if (ret == -1) {
        throw disable_aslr_error{};
    }
    
    execvpe(argv[0], argv, envp);
}

} // unnamed namespace

} // namespace mgth

#elif defined(MGBASE_OS_MAC_OS_X)

#include <spawn.h>

extern char** environ;

namespace mgth {

namespace /*unnamed*/ {

void exec_disable_aslr(char** const argv, char** const envp)
{
    // Reference: Source code of GDB.
    
    posix_spawnattr_t attr{};
    
    const int init_ret = posix_spawnattr_init(&attr);
    
    if (init_ret != 0) {
        MGBASE_LOG_FATAL(
            "msg:posix_spawnattr_init() failed.\n"
            "ret:{}"
        ,   init_ret
        );
        
        throw disable_aslr_error{};
    }
    
    // The constant doesn't look to be available
    // outside the kernel include files.
#ifndef _POSIX_SPAWN_DISABLE_ASLR
#define _POSIX_SPAWN_DISABLE_ASLR 0x0100
#endif
    
    const short ps_flags =
        // Do like execve: replace the image.
        POSIX_SPAWN_SETEXEC
        // Disable ASLR.
        | _POSIX_SPAWN_DISABLE_ASLR;
    
    const int setflags_ret
        = posix_spawnattr_setflags(&attr, ps_flags);
    
    if (setflags_ret != 0) {
        MGBASE_LOG_FATAL(
            "msg:posix_spawnattr_setflags() failed.\n"
            "ret:{}"
        ,   setflags_ret
        );
        
        throw disable_aslr_error{};
    }
    
    posix_spawnp(
        MGBASE_NULLPTR  // pid
    ,   argv[0]         // path
    ,   MGBASE_NULLPTR  // file_actions
    ,   &attr           // attrp
    ,   argv            // argv
    ,   envp            // envp
    );
}

} // unnamed namespace

} // namespace mgth

#endif

namespace mgth {

void disable_aslr(const int /*argc*/, char** const argv)
{
    const auto val = getenv("MGTH_ASLR_DISABLED");
    
    if (! ((val != MGBASE_NULLPTR) && strcmp(val, "1") == 0))
    {
        std::vector<char*> envv;
        for (char** p = ::environ; *p != MGBASE_NULLPTR; ++p) {
            envv.push_back(*p);
        }
        
        const auto var_cstr = "MGTH_ASLR_DISABLED=1";
        envv.push_back(const_cast<char*>(var_cstr));
        
        envv.push_back(MGBASE_NULLPTR);
        
        exec_disable_aslr(argv, &envv[0]);
        
        // The current image is replaced.
    }
}

} // namespace mgth

