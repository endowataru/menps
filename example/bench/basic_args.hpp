
#include <mgbase/external/cmdline.hpp>

struct basic_args
{
    mgbase::uint32_t    num_threads;
    double              duration;
    mgbase::uint64_t    num_startup_samples;
    std::string         output_file;
    mgbase::int32_t     master_proc;
    mgbase::uint64_t    msg_size;
};

inline void add_basic_args(cmdline::parser& p)
{
    p.add<mgbase::uint32_t>("num_threads", 't', "number of threads", false, 1);
    p.add<double>("duration", 'd', "duration [sec]", false, 3.0);
    p.add<mgbase::uint64_t>("num_startup_samples" , '\0', "number of initially omitted samples" , false, 1000);
    p.add<std::string>("output_file", 'o', "path to the output file", false, "output.yaml");
    p.add<mgbase::int32_t>("master_proc", 'm', "master process that doesn't send requests", false, -1);
    p.add<mgbase::uint64_t>("msg_size", 's', "message size [bytes]", false, 8);
}

inline basic_args get_basic_args(cmdline::parser& p)
{
    return {
        p.get<mgbase::uint32_t>("num_threads")
    ,   p.get<double>("duration")
    ,   p.get<mgbase::uint64_t>("num_startup_samples")
    ,   p.get<std::string>("output_file")
    ,   p.get<mgbase::int32_t>("master_proc")
    ,   p.get<mgbase::uint64_t>("msg_size")
    };
}

inline basic_args get_basic_args(int argc, char* argv[])
{
    cmdline::parser p;
    add_basic_args(p);
    p.parse_check(argc, argv);
    return get_basic_args(p);
}

