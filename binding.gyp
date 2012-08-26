{
  'targets': [
    {
      'target_name': 'winpty',
      'sources': [
        'src/bindings.cc',
        'src/winpty.cc'
      ],
      'include_dirs': [ 'src/' ],
      'copies': [
        {
          'destination': '<(module_root_dir)/build/Release/',
          'files': [
            '<(module_root_dir)/winpty-agent.exe',
          ]
        },
      ]
    }
  ]
}
