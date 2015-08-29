{
    'includes': [ 'common.gypi' ],
    'targets' : [
        {
            'target_name' : 'latency',
            'type' : 'executable',
            'sources' : [
                '../example/latency.cpp',
            ],
            'dependencies' : [
                './mgcom.gyp:mgcom-mpi3',
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
            'target_name' : 'test',
            'type' : 'executable',
            'sources' : [
                '../example/test.cpp',
            ],
            'dependencies' : [
                './mgcom.gyp:mgcom-fjmpi',
            ],
        }
    ],
}
