{
    'includes': [ 'common.gypi' ],
    'target_defaults': {
        'dependencies' : [
            '../../mgbase/build-script/mgbase.gyp:mgbase',
        ],
    },
    'targets' : [
        {
            'target_name' : 'mgcom-header',
            'type': 'none',
            'all_dependent_settings': {
                'include_dirs': [
                    '../include',
                ],
            },
            'dependencies': [
                '../../mgbase/build-script/mgbase.gyp:mgbase',
            ],
        },
        {
            'target_name': 'mgcom-polling',
            'type': 'static_library',
            'sources': [
                '../src/common/polling.cpp'
            ],
            'dependencies': [
                'mgcom-header',
            ],
        },
        {
            'target_name' : 'mgcom-fjmpi',
            'type' : 'static_library',
            'standalone_static_library': 1,
            'sources' : [
                '../src/common/mpi_base.cpp',
                '../src/common/rma/contiguous.cpp',
                '../src/common/rma/buffer_pool.cpp',
                '../src/device/fjmpi/fjmpi.cpp',
                '../src/device/fjmpi/rma/rma.cpp',
                '../src/device/fjmpi/am/sender.cpp',
            ],
            'include_dirs': [
                '../src',
                '../src/device/fjmpi',
            ],
            'dependencies': [
                'mgcom-header',
            ],
        },
        {
            'target_name' : 'mgcom-ibv',
            'type' : 'static_library',
            'sources' : [
                '../src/common/mpi_base.cpp',
                '../src/common/rma/contiguous.cpp',
                '../src/common/rma/buffer_pool.cpp',
                '../src/device/ibv/rdma.cpp',
            ],
        },
        {
            'target_name': 'mgcom-mpi3',
            'type': 'static_library',
            'sources': [
                '../src/common/mpi_base.cpp',
                '../src/common/rma/contiguous.cpp',
                '../src/common/rma/buffer_pool.cpp',
                '../src/device/mpi3/mpi3.cpp',
                '../src/device/mpi3/collective.cpp',
                '../src/device/mpi3/rma/rma.cpp',
                '../src/device/mpi3/am/am.cpp',
                '../src/device/mpi3/am/receiver.cpp',
                '../src/device/mpi3/am/sender.cpp',
                '../src/device/mpi3/am/sender_queue.cpp',
            ],
            'include_dirs': [
                '../src',
                '../src/device/mpi3',
            ],
            'dependencies': [
                'mgcom-header',
            ],
        },
    ],
}
