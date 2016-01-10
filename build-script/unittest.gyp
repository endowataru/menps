{
    'includes' : [ './external.gypi' ],
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
                '../unittest/bound_function.cpp',
                '../unittest/deferred.cpp',
            ],
            'dependencies' : [
                './mgbase.gyp:mgbase',
                './googletest.gyp:googletest',
            ],
        }
    ],
}
