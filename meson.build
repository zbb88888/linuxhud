project('linuxhud', 'c',
  version : '0.1.0',
  default_options : ['warning_level=2', 'c_std=c11'],
  license : 'MIT'
)

# 依赖库
libdrm_dep = dependency('libdrm', required : true)
cairo_dep = dependency('cairo', required : true)
pango_dep = dependency('pangocairo', required : true)
glib_dep = dependency('glib-2.0', required : true)

# 源文件
src = files(
  'src/main.c',
  'src/drm_device.c',
  'src/buffer.c',
  'src/renderer.c'
)

# 头文件
include_dir = include_directories('include')

# 可执行文件
executable('linuxhud',
  src,
  dependencies : [libdrm_dep, cairo_dep, pango_dep, glib_dep],
  include_directories : include_dir,
  install : true
)

# 测试程序
test_drm = executable('test_drm',
  'test_drm.c',
  dependencies : [libdrm_dep],
  include_directories : include_dir
)

# 安装配置
install_data('linuxhud.conf', install_dir : '/etc')
install_data('linuxhud.service', install_dir : '/etc/systemd/system')
