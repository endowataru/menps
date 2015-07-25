{
    'includes': [ 'common.gypi' ],
    'targets' : [
        {
            'target_name' : 'test',
            'type' : 'executable',
            'sources' : [
                '../example/test.cpp',
            ],
            'dependencies' : [
                './fjmpi.gyp:fjmpi',
            ],
        }
    ],
}
