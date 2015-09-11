{
    'targets' : [
        {
            'target_name' : 'cppformat',
            'type' : '<(library)',
            'sources' : [
                '../external/cppformat/format.cc',
            ],
            'all_dependent_settings' : {
                'include_dirs' : [
                    '../external',
                ],
            },
        }
    ]
}
