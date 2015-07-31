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
                './mgcom.gyp:mgcom-fjmpi',
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
