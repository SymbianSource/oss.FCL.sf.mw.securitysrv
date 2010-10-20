HEADERS += contentwidget.h dirviewitem.h modelfactory.h viewfutedataform.h treedataform.h mailtreeviewitem.h brownevenviewitem.h greenoddviewitem.h
SOURCES += contentwidget.cpp main.cpp dirviewitem.cpp modelfactory.cpp viewfutedataform.cpp treedataform.cpp mailtreeviewitem.cpp brownevenviewitem.cpp greenoddviewitem.cpp
RESOURCES += SecUiTestQt.qrc shared.qrc

LIBS += -lsecui -letelmm -letel -lcustomapi -lcentralrepository
LIBS += -lcone -lws32 -lkeylockpolicyapi
LIBS += -lpower_save_display_mode
LIBS += -ltstaskmonitorclient		# for TsTaskSettings
LIBS += -lavkon									# for KeySounds
LIBS += -lapgrfx								# for CApaWindowGroupName
LIBS += -lscpclient							# SCP server         
LIBS += -llockclient
LIBS += -letelmm
LIBS += -letel
LIBS += -lcustomapi
LIBS += -letel3rdparty

symbian*: {
				TARGET.CAPABILITY = CAP_APPLICATION NetworkControl
				TARGET.UID3 = 0xEE89E3CE
				TARGET.EPOCHEAPSIZE = 0x20000 0x1000000
				}
HB_NOMAKE_PARTS += tests performance localization
CONFIG += Hb

INCLUDEPATH += /sf/mw/hb/include/hbcore/private

include(SecUiTestQt.pri)

BLD_INF_RULES.prj_exports += "./rom/SecUiTestQt.iby            CORE_APP_LAYER_IBY_EXPORT_PATH(SecUiTestQt.iby)"
