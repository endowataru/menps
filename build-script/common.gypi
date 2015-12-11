{
    'include' : [ '../mgbase/build-script/common.gypi', ],
    'target_defaults': {
        'default_configuration': 'Release',
        'configurations': {
            'Debug': {
                'defines':['MGBASE_DEBUG'],
                'cflags': ['-g3', '-O0', '-Wall'],
                'ldflags': ['-g3'],
                'xcode_settings': {
                    'OTHER_CPLUSPLUSFLAGS': [
                        '-g3', '-O0', '-Wall',
                        
                        # warnings
                        '-pedantic', '-Wextra', '-Wcast-qual', '-Wctor-dtor-privacy',
                        '-Wdisabled-optimization', '-Wformat=2', '-Winit-self', '-Wmissing-declarations',
                        '-Wmissing-include-dirs', '-Woverloaded-virtual', '-Wredundant-decls',
                        '-Wshadow', '-Wsign-promo', '-Wstrict-overflow=5', '-Wswitch-default',
                        '-Wno-c++11-long-long',
                        '-Wno-format-nonliteral', # for cppformat
                        # '-Wold-style-cast', '-Wcast-align', # for mpi.h
                        # '-Wundef', '-Wsign-conversion',  # for cppformat
                    ],
                    'OTHER_LDFLAGS': ['-g3'],
                },
            },
            'Release': {
                'defines':['MGBASE_DISABLE_ASSERT'],
                'cflags': ['-O3', '-Wall','-pedantic',],
                'xcode_settings': {
                    'OTHER_CPLUSPLUSFLAGS': [
                        '-O3', '-Wall', '-Werror', '-ferror-limit=3',
                        
                        # warnings
                        '-pedantic', '-Wextra', '-Wcast-qual', '-Wctor-dtor-privacy',
                        '-Wdisabled-optimization', '-Wformat=2', '-Winit-self', '-Wmissing-declarations',
                        '-Wmissing-include-dirs', '-Woverloaded-virtual', '-Wredundant-decls',
                        '-Wshadow', '-Wsign-promo', '-Wstrict-overflow=5', '-Wswitch-default',
                        '-Wno-c++11-long-long',
                        '-Wno-format-nonliteral', # for cppformat
                        # '-Wold-style-cast', '-Wcast-align', # for mpi.h
                        # '-Wundef', '-Wsign-conversion',  # for cppformat
                    ],
                },
            },
        },
    },
}
