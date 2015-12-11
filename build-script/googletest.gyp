{
    'includes' : [ 'external.gypi' ],
    'targets' : [
        {
            'target_name' : 'googletest',
            'type' : '<(library)',
            'sources' : [
                '../external/googletest/googletest/src/gtest-all.cc',
            ],
            'include_dirs' : [
                '../external/googletest/googletest',
                '../external/googletest/googletest/include',
            ],
            'all_dependent_settings' : {
                'include_dirs' : [
                    '../external/googletest/googletest/include',
                ],
                'xcode_settings': {
                    'OTHER_CPLUSPLUSFLAGS': [
                        '-Wno-c++11-extensions',
                        '-Wno-variadic-macros',
                    ]
                },
            },
        }
    ]
}
