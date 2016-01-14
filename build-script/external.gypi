{
    'target_defaults': {
        'default_configuration': 'Release',
        'configurations': {
            'Debug': {
                'cflags': ['-g3', '-O0', '-Wall'],
                'ldflags': ['-g3', '-lpthread'],
                'xcode_settings': {
                    'OTHER_CPLUSPLUSFLAGS': [
                        '-g3', '-O0', '-Wall',
                    ],
                    'OTHER_LDFLAGS': ['-g3'],
                },
            },
            'Release': {
                'cflags': ['-O3',],
                'ldflags': ['-lpthread'],
                'xcode_settings': {
                    'OTHER_CPLUSPLUSFLAGS': [
                        '-O3',
                    ],
                    'OTHER_LDFLAGS': [
                        '-g3'
                    ],
                },
            },
        },
    },
}
