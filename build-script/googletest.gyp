{
    'targets' : [
        {
            'target_name' : 'googletest',
            'type' : '<(library)',
            'sources' : [
                '../external/googletest/src/gtest-all.cc',
            ],
            'include_dirs' : [
                '../external/googletest/include',
                '../external/googletest',
            ],
            'direct_dependent_settings' : {
                'include_dirs' : [
                    '../external/googletest/include',
                ],
            },
        }
    ]
}
