CONFIG(debug, debug|release) {
    targetDir = $$quote($$(OFFIS_Automation_Framework)/debug/plugins)
} else {
    targetDir = $$quote($$(OFFIS_Automation_Framework)/plugins)
}

TARGET = PylonCam
DESTDIR = $${targetDir}/olvisPlugins

include($${targetDir}/olvis/shared.pro)
include(PylonCamFiles.pri)
