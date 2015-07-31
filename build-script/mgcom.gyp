{
    'includes': [ 'common.gypi' ],
    'targets' : [
        {
            'target_name' : 'mgcom-fjmpi',
            'type' : 'static_library',
            'sources' : [
                '../src/fjmpi.cpp',
            ],
        },
        {
            'target_name' : 'mgcom-ibv',
            'type' : 'static_library',
            'sources' : [
                '../src/ibv.cpp',
            ],
        }
    ],
}
