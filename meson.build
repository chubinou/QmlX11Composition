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

moc_files = qt5.preprocess(
    moc_headers: [
        'RenderWindow.hpp',
        'RenderClient.hpp',
    ],
    qresources: 'qml.qrc')

executable('myprog',
    'main.cpp',
    'RenderWindow.cpp',
    'RenderClient.cpp',
    moc_files,
    dependencies : [qt5_dep, x11, xcomposite, xrender, xdamage, xext, vlc]
)
