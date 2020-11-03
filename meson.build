project('transparency', 'cpp')
qt5 = import('qt5')
qt5_dep = dependency('qt5', modules: ['Core', 'Gui', 'QuickWidgets', 'X11Extras'])

pkg = import('pkgconfig')

x11 = dependency('x11')
xcomposite = dependency('xcomposite')
xrender = dependency('xrender')
xdamage = dependency('xdamage')
xext = dependency('xext')
vlc = dependency('libvlc')

resources = qt5.preprocess(
    qresources: 'qml.qrc'
    )

moc_files = qt5.preprocess(
    moc_headers: [
        'OffscreenQmlView.hpp',
        'RenderWindow.hpp',
        'RenderClient.hpp',
    ])

executable('myprog',
    'main.cpp',
    'OffscreenQmlView.cpp',
    'RenderWindow.cpp',
    'RenderClient.cpp',
    moc_files, resources,
    dependencies : [qt5_dep, x11, xcomposite, xrender, xdamage, xext, vlc]
)

executable('onTop',
    'always_on_top.cpp',
    resources,
    dependencies : [qt5_dep,  vlc]
)
