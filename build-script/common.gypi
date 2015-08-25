{
  'target_defaults': {
    'default_configuration': 'Release',
    #'xcode_settings': {
    #    'CLANG_CXX_LANGUAGE_STANDARD': 'c++11',
    #    'MACOSX_DEPLOYMENT_TARGET': '10.8',
    #    'CLANG_CXX_LIBRARY': 'libc++',
    #},
    'configurations': {
      'Debug': {
        'defines':['MGBASE_DEBUG'],
        'cflags': ['-g3', '-O0', '-Wall'],
        'ldflags': ['-g3'],
        'xcode_settings': {
            'OTHER_CPLUSPLUSFLAGS': ['-g3', '-O0', '-Wall',
            ],
            'OTHER_LDFLAGS': ['-g3'],
        },
      }, # Debug
      'Release': {
        'defines':[],
        'cflags': ['-O3', '-Wall',
            '-pedantic',
                '-Wno-variadic-macros'
        ],
        'xcode_settings': {
            'OTHER_CPLUSPLUSFLAGS': [
                '-O3', '-Wall', '-Werror',
                '-Wno-variadic-macros',
                '-pedantic', '-Wextra', '-Wcast-qual', '-Wctor-dtor-privacy',
                '-Wdisabled-optimization', '-Wformat=2', '-Winit-self', '-Wmissing-declarations',
                '-Wmissing-include-dirs', '-Woverloaded-virtual', '-Wredundant-decls',
                '-Wshadow', '-Wsign-conversion', '-Wsign-promo', '-Wstrict-overflow=5', '-Wswitch-default', '-Wundef', '-Wno-unused',
                '-Wno-c++11-long-long',
            ],
            # '-Wold-style-cast', '-Wcast-align', # mpi.h
        },
      }, # Release
    }, # configurations
    'variables': {
      'component%': 'static_library',
    },
    'include_dirs': [
        '../include',
        '../mgbase/include',
    ],
  }, # target_defaults
}
