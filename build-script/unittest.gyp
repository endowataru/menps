{
    'includes' : [ 'common.gypi' ],
    'target_defaults': {
        'sources': [
            "../unittest/unittest.cpp",
            '../unittest/rma.cpp',
            '../unittest/am.cpp',
        ]
    },
    'targets' : [
        #{
        #    'target_name' : 'unittest',
        #    'type': 'static_library',
        #    'sources' : [
        #    ],
        #    'dependencies': [
        #        './mgcom.gyp:mgcom-header',
        #        '../../mgbase/build-script/googletest.gyp:googletest',
        #    ],
        #},
        {
            'target_name' : 'unittest-mpi3',
            'type': 'executable',
            'dependencies': [
                #'unittest',
                './mgcom.gyp:mgcom-mpi3',
                './mgcom.gyp:mgcom-polling',
            ],
        },
    ]
}
