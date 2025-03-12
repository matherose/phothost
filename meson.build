project('phothost', 'c',
    version: '0.1',
    default_options: ['c_std=gnu99', 'warning_level=3', 'optimization=2']
)

ffmpeg_dep = dependency('libavutil', fallback : ['ffmpeg', 'libavutil_dep'])

# Intégration des sous-projets via leurs fichiers wrap placés dans subprojects/
if host_machine.system() == 'darwin'
  platform_link_args = ['-dead_strip_dylibs', '-headerpad_max_install_names', '-rpath', '/opt/homebrew/Cellar/ffmpeg/7.1.1_1/lib']
elif host_machine.system() == 'linux'
  platform_link_args = ['-Wl,-z,relro', '-Wl,-z,now', '-rpath', '/opt/homebrew/Cellar/ffmpeg/7.1.1_1/lib']
else
  platform_link_args = []
endif

# Utilisation de tous les sous-projets dans l'exécutable
executable('phothost', 'src/main.c',
    c_args: ['-fstack-protector-strong', '-D_FORTIFY_SOURCE=2'],
    link_args: platform_link_args,
    dependencies : ffmpeg_dep
)