project('transparency', 'cpp')
qt5 = import('qt5')
qt5_dep = dependency('qt5', modules: ['Core', 'Gui', 'QuickWidgets', 'X11Extras'])

pkg = import('pkgconfig')

x11 = dependency('x11')
xcomposite = dependency('xcomposite')
xrender = dependency('xrender')
xdamage = dependency('xdamage')
xext = dependency('xext')
xcb = dependency('xcb')
xcb_render = dependency('xcb-render')
xcb_damage = dependency('xcb-damage')
xcb_composite = dependency('xcb-composite')
vlc = dependency('libvlc')

moc_files = qt5.preprocess(
    qresources: 'qml.qrc',
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
    moc_files,
    dependencies : [qt5_dep, x11,
            xcomposite, xrender, xdamage, xext,
            xcb, xcb_composite, xcb_damage, xcb_render,
            vlc]
)
