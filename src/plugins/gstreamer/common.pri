
QT += core-private multimedia-private network

qtHaveModule(widgets) {
    QT += widgets multimediawidgets-private
    DEFINES += HAVE_WIDGETS
}

LIBS += -lqgsttools_p

linux-armv7-kobo-g++ {
    QMAKE_CFLAGS += $$system(arm-linux-pkg-config --cflags gstreamer-1.0 gstreamer-base-1.0 gstreamer-tag-1.0 gstreamer-audio-1.0 gstreamer-video-1.0  gstreamer-pbutils-1.0 gstreamer-app-1.0)
    QMAKE_CXXFLAGS += $$system(arm-linux-pkg-config --cflags gstreamer-1.0 gstreamer-base-1.0 gstreamer-tag-1.0 gstreamer-audio-1.0 gstreamer-video-1.0  gstreamer-pbutils-1.0 gstreamer-app-1.0)
    QMAKE_LIBS += $$system(arm-linux-pkg-config --libs gstreamer-1.0 gstreamer-base-1.0 gstreamer-tag-1.0 gstreamer-audio-1.0 gstreamer-video-1.0  gstreamer-pbutils-1.0 gstreamer-app-1.0)
}
else: {
CONFIG += link_pkgconfig

PKGCONFIG += \
    gstreamer-$$GST_VERSION \
    gstreamer-base-$$GST_VERSION \
    gstreamer-audio-$$GST_VERSION \
    gstreamer-video-$$GST_VERSION \
    gstreamer-pbutils-$$GST_VERSION
}

maemo*:PKGCONFIG +=gstreamer-plugins-bad-$$GST_VERSION

mir: {
    DEFINES += HAVE_MIR
}


config_resourcepolicy {
    DEFINES += HAVE_RESOURCE_POLICY
    PKGCONFIG += libresourceqt5
}

config_gstreamer_appsrc {
    PKGCONFIG += gstreamer-app-$$GST_VERSION
    DEFINES += HAVE_GST_APPSRC
    LIBS += -lgstapp-$$GST_VERSION
}

