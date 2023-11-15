project(
    'movlist',
    ['c', 'cpp'],
    default_options : [
        'c_std=c99',
        'cpp_std=c++11',
    ],
    license : 'GPL-2.0',
    version : '1.0',
)

c_args = [
    '-Wall',
    '-Wextra',
    '-O2',
    '-D_GNU_SOURCE',
]

cpp_args = [
    '-Wall',
    '-Wextra',
    '-O2',
    '-D_GNU_SOURCE',
    '-fno-rtti',
    '-fno-exceptions',
]

app_deps = [
    dependency('tinyc'),
    dependency('libmediainfo'),
]

app_sources = [
    'global.c',
    'main.c',
    'mediainfo.cpp',
    'moventry.c',
    'movlist.c',
]

executable(
    meson.project_name(),
    c_args: c_args,
    cpp_args : cpp_args,
    dependencies : app_deps,
    sources : app_sources,
    install : true,
)


