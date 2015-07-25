{
    'includes': [ 'common.gypi' ],
    'targets' : [
        {
            'target_name' : 'fjmpi',
            'type' : 'static_library',
            'standalone_static_library': 1,
            'sources' : [
                '../src/fjmpi.cpp',
            ],
        }
    ],
}
