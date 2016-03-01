{
  'targets': [
    {
      'target_name': 'bmsampler',
      'type': 'executable',
      'include_dirs': [
        '/usr/local/include',
        './vendor/json/include',
        './vendor/libsndfile/include'
      ],
      'sources': [
        'bmsampler.cpp'
      ],
      'ldflags': [
        '-L/usr/local/lib',
        '-lsndfile'
      ],
      'xcode_settings': {
        'GCC_VERSION': 'com.apple.compilers.llvm.clang.1_0',
        'CLANG_CXX_LANGUAGE_STANDARD': 'c++11',
        'CLANG_CXX_LIBRARY': 'libc++',
        'OTHER_LDFLAGS': [
          '-L/usr/local/lib',
          '-lsndfile'
        ]
      }
    }
  ]
}
