{
    'targets' : [
        {
            'target_name' : 'mgbase',
            'type' : 'none',
            'all_dependent_settings' : {
                'include_dirs' : [
                    '../include',
                ],
            },
            'dependencies' : [
                './cppformat.gyp:cppformat'
            ],
        }
    ]
}
