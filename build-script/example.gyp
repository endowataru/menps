{
    'includes': [ 'common.gypi' ],
    'targets' : [
        {
            'target_name' : 'bench-latency-read',
            'type' : 'executable',
            'sources' : [
                '../example/bench/bench-latency-read.cpp',
            ],
            'dependencies' : [
                './mgcom.gyp:mgcom-mpi3',
            ],
        },
        {
            'target_name' : 'bench-latency-cas',
            'type' : 'executable',
            'sources' : [
                '../example/bench/bench-latency-cas.cpp',
            ],
            'dependencies' : [
                './mgcom.gyp:mgcom-mpi3',
            ],
        },
        {
            'target_name' : 'bench-latency-am',
            'type' : 'executable',
            'sources' : [
                '../example/bench/bench-latency-am.cpp',
            ],
            'dependencies' : [
                './mgcom.gyp:mgcom-mpi3',
            ],
        },
        {
            'target_name' : 'latency',
            'type' : 'executable',
            'sources' : [
                '../example/latency.cpp',
            ],
            'dependencies' : [
                './mgcom.gyp:mgcom-fjmpi',
            ],
        },
        {
            'target_name' : 'latency-am',
            'type' : 'executable',
            'sources' : [
                '../example/latency-am.cpp',
            ],
            'dependencies' : [
                './mgcom.gyp:mgcom-mpi3',
            ],
        },
        {
            'target_name' : 'latency-cas',
            'type' : 'executable',
            'sources' : [
                '../example/latency-cas.cpp',
            ],
            'dependencies' : [
                './mgcom.gyp:mgcom-mpi3',
            ],
        },
        {
            'target_name' : 'test',
            'type' : 'executable',
            'sources' : [
                '../example/test.cpp',
            ],
            'dependencies' : [
                './mgcom.gyp:mgcom-fjmpi',
            ],
        },
        {
            'target_name' : 'typed',
            'type' : 'executable',
            'sources' : [
                '../example/typed.cpp',
            ],
            'dependencies' : [
                './mgcom.gyp:mgcom-fjmpi',
            ],
        }
    ],
}
