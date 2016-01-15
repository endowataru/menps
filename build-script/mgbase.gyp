{
    'targets' : [
        {
            'target_name' : 'mgbase',
            'type' : 'none',
            'all_dependent_settings' : {
                'include_dirs' : [
                    '../include',
                ],
                'libraries': ['-lpthread', ],
            },
            'dependencies' : [
                './cppformat.gyp:cppformat'
            ],
        }
    ]
}
