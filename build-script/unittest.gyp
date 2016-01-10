{
    'includes' : [ 'common.gypi' ],
    'target_defaults': {
        'sources': [
            "../unittest/unittest.cpp",
        ]
    },
    'targets' : [
        {
            'target_name' : 'unittest',
            'type': 'executable',
            'sources' : [
                '../unittest/rma.cpp',
                '../unittest/am.cpp',
            ],
            'dependencies': [
                './mgcom.gyp:mgcom-mpi3',
                './mgcom.gyp:mgcom-polling',
                '../../mgbase/build-script/googletest.gyp:googletest',
            ],
        }
    ]
}
