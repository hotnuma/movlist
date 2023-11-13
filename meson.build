project(
    'newlist',
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
    dependency('tinyc', static : true),
    dependency('libmediainfo'),
]

app_sources = [
    'global.c',
    'main.c',
    'mlist.c',
    'mlist_entry.c',
    'MovMediaInfo.cpp',
]

executable(
    meson.project_name(),
    c_args: c_args,
    cpp_args : cpp_args,
    dependencies : app_deps,
    sources : app_sources,
    install : true,
)


