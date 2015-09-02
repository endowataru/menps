{
    'targets' : [
        {
            'target_name' : 'cppformat',
            'type' : '<(library)',
            'sources' : [
                '../external/cppformat/format.cc',
            ],
            'direct_dependent_settings' : {
                'include_dirs' : [
                    '../external',
                ],
            },
        }
    ]
}
