load(qt_module)

TARGET = qtmedia_blackberry
QT += multimedia-private gui-private
CONFIG += no_private_qt_headers_warning

PLUGIN_TYPE=mediaservice

load(qt_plugin)
DESTDIR = $$QT.multimedia.plugins/$${PLUGIN_TYPE}
LIBS += -lmmrndclient -lstrm -lscreen

HEADERS += \
    bbserviceplugin.h \
    bbmediaplayerservice.h \
    bbmediaplayercontrol.h \
    bbmetadata.h \
    bbutil.h

SOURCES += \
    bbserviceplugin.cpp \
    bbmediaplayerservice.cpp \
    bbmediaplayercontrol.cpp \
    bbmetadata.cpp \
    bbutil.cpp

!isEmpty(QT.widgets.name) {
    HEADERS += bbvideowindowcontrol.h
    SOURCES += bbvideowindowcontrol.cpp
}

OTHER_FILES += blackberry.json

target.path += $$[QT_INSTALL_PLUGINS]/$${PLUGIN_TYPE}
INSTALLS += target