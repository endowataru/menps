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
            'standalone_static_library': 1,
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
        {
            'target_name': 'mgcom-mpi3',
            'type': 'static_library',
            'sources': [
                '../src-common/contiguous.cpp',
                '../src-device/mpi3/rma.cpp',
            ]
        }
    ],
}
