{
  'targets': [
    {
      'target_name': 'koki',
      'sources': [
        'koki.cpp',
      ],
      'include_dirs': [
        '../../include',
        '<!@(pkg-config glib-2.0 --cflags-only-I | sed s/-I//g)',
        '<!@(pkg-config opencv --cflags-only-I | sed s/-I//g)'
      ],
      'libraries': [
          '<!@(pkg-config --libs opencv)',
          '<!@(pkg-config --libs glib-2.0)',
      ],
      'ldflags': [
        '-Wl,-whole-archive ../../../lib/libkoki.a -Wl,-no-whole-archive'
      ],
      'cflags': [
        '-O3',
        '<!@(pkg-config --cflags --libs "opencv >= 2.3.1" )'
      ],
      'cflags!' : [ '-fno-exceptions'],
      'cflags_cc!': [ '-fno-rtti', '-fno-exceptions']
    },
  ],
}
