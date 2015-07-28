{
    'includes': [ 'common.gypi' ],
    'targets' : [
        {
            'target_name' : 'ibv',
            'type' : 'static_library',
            'standalone_static_library': 1,
            'sources' : [
                '../src/ibv.cpp',
            ],
        }
    ],
}
