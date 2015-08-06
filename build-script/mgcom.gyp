{
    'includes': [ 'common.gypi' ],
    'target_defaults': {
        'include_dirs': [
            '../src-common'
        ],
        'sources': [
            '../src-common/c_interface.cpp'
        ]
    },
    'targets' : [
        {
            'target_name' : 'mgcom-fjmpi',
            'type' : 'static_library',
            'sources' : [
                '../src-device/fjmpi/rdma.cpp',
            ],
        },
        {
            'target_name' : 'mgcom-ibv',
            'type' : 'static_library',
            'sources' : [
                '../src-device/ibv/rdma.cpp',
            ],
        },
    ],
}
