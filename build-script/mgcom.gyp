{
    'includes': [ 'common.gypi' ],
    'target_defaults': {
        'include_dirs': [
            '../src',
        ],
        'sources': [
            '../src/common/c_interface.cpp',
        ],
        'dependencies' : [
            '../../mgbase/build-script/cppformat.gyp:cppformat',
        ],
    },
    'targets' : [
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
                '../src/device/fjmpi',
            ],
        },
        {
            'target_name' : 'mgcom-ibv',
            'type' : 'static_library',
            'sources' : [
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
                '../src/device/mpi3/barrier.cpp',
                '../src/device/mpi3/rma/rma.cpp',
                '../src/device/mpi3/am/am.cpp',
                '../src/device/mpi3/am/receiver.cpp',
                '../src/device/mpi3/am/sender.cpp',
                '../src/device/mpi3/am/sender_queue.cpp',
            ],
            'include_dirs': [
                '../src/device/mpi3',
            ],
        }
    ],
}
