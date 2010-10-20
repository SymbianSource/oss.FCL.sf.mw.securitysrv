######################################################################
# hbtest.pri
######################################################################

isEmpty(TEMPLATE):TEMPLATE = app

contains(TEMPLATE, .*subdirs$) {

    !contains(_PRO_FILE_PWD_, .*/fute$) {
        # recurse into subdirs
        test.CONFIG += recursive
        autotest.CONFIG += recursive
        citest.CONFIG += recursive
    }

    root {
        # compile all tests first at the root level
        !win32|CONFIG(debug, debug|release):test.depends = first
        !win32|CONFIG(debug, debug|release):autotest.depends = first
        !win32|CONFIG(debug, debug|release):citest.depends = first
    }

} else {

    # Flag all the test binaries SMP-safe in Symbian
    symbian:MMP_RULES += SMPSAFE

    INCLUDEPATH += $$_PRO_FILE_PWD_
    DEPENDPATH += $$_PRO_FILE_PWD_

    QMAKE_RPATHDIR += $${HB_BUILD_DIR}/lib

    HB_TEST_INSTALL_DIR = $${HB_INSTALL_DIR}/tsrc/bin
    DEFINES += HB_BUILD_DIR=\"\\\"$${HB_BUILD_DIR}\\\"\"

    hbtestresources {
        LIBS += -L$${HB_BUILD_DIR}/lib -lHbTestResources
    }

    HB = $$lower($$unique(HB))
    isEmpty(HB):HB = hbcore hbwidgets
    !contains(HB, "hbcore"):!contains(HB_NOMAKE_PARTS, hbcore): HB += hbcore
    contains(HB, "hbutils")|contains(HB, "hbinput")|contains(HB, "hbfeedback"): HB *= hbwidgets

    for(COLLECTION, HB) {
        isEqual(COLLECTION, hbcore) {
            !contains(HB_NOMAKE_PARTS, hbcore):hbAddLibrary(hbcore/HbCore)
        } else:isEqual(COLLECTION, hbwidgets) {
            !contains(HB_NOMAKE_PARTS, hbwidgets):hbAddLibrary(hbwidgets/HbWidgets)
        } else:isEqual(COLLECTION, hbutils) {
            !contains(HB_NOMAKE_PARTS, hbutils):hbAddLibrary(hbutils/HbUtils)
        } else:isEqual(COLLECTION, hbinput) {
            !contains(HB_NOMAKE_PARTS, hbinput):hbAddLibrary(hbinput/HbInput)
        } else:isEqual(COLLECTION, hbfeedback) {
            !contains(HB_NOMAKE_PARTS, hbfeedback):hbAddLibrary(hbfeedback/HbFeedback)
        } else {
            message(Unknown COLLECTION: $$COLLECTION)
            next()
        }
    }

    plugin {

        # nothing to do for plugins

    } else {

        unit_test = false
        perf_test = false
        loc_test = false
        contains(_PRO_FILE_PWD_, .*/tsrc/unit/.*):unit_test = true
        contains(_PRO_FILE_PWD_, .*/tsrc/performance/.*):perf_test = true
        contains(_PRO_FILE_PWD_, .*/tsrc/loc/.*):loc_test = true

        $$unit_test|$$perf_test|$$loc_test {

            # a runnable test
            HEADERS += hbtest.h
            CONFIG -= app_bundle
            CONFIG += qtestlib console

            $$perf_test:include($${HB_SOURCE_DIR}/tsrc/performance/shared/shared.pri)

            # TODO: cleanup test execution
            win32:DESTDIR = debug
            # only Makefile.Debug on win32:
            !win32|CONFIG(debug, debug|release):build_pass:!isEmpty(DESTDIR) {
                test.commands += cd $$DESTDIR &&
                autotest.commands += cd $$DESTDIR &&
                citest.commands += cd $$DESTDIR &&
            }
            unix {
                exists( /usr/local/bin/launcher ) {
                  test.commands += /usr/local/bin/launcher ./$(TARGET)
                  autotest.commands += /usr/local/bin/launcher ./$(TARGET) -style plastique -xml -o $${HB_BUILD_DIR}/autotest/$(QMAKE_TARGET).xml
                  citest.commands += /usr/local/bin/launcher ./$(TARGET) -style plastique -xunitxml -o $${HB_BUILD_DIR}/autotest/$(QMAKE_TARGET).xml
                } else {
                  test.commands += ./$(TARGET)
                  autotest.commands += ./$(TARGET) -style plastique -xml -o $${HB_BUILD_DIR}/autotest/$(QMAKE_TARGET).xml
                  citest.commands += ./$(TARGET) -style plastique -xunitxml -o $${HB_BUILD_DIR}/autotest/$(QMAKE_TARGET).xml
                }
            } else:win32 {
        test.CONFIG += recursive
                autotest.CONFIG += recursive
                citest.CONFIG += recursive
                # only Makefile.Debug:
                win32-g++:!isEmpty(QMAKE_SH) {
                    CONFIG(debug, debug|release):build_pass {
                        test.commands += ./$(TARGET)
                        autotest.commands += ./$(TARGET) -xml -o $${HB_BUILD_DIR}/autotest/$(QMAKE_TARGET).xml
                        citest.commands += ./$(TARGET) -xunitxml -o $${HB_BUILD_DIR}/autotest/$(QMAKE_TARGET).xml
                    }
                } else {
                    CONFIG(debug, debug|release):build_pass {
                        test.commands += $(TARGET)
                        autotest.commands += $(TARGET) -xml -o $${HB_BUILD_DIR}/autotest/$(QMAKE_TARGET).xml
                        citest.commands += $(TARGET) -xunitxml -o $${HB_BUILD_DIR}/autotest/$(QMAKE_TARGET).xml
                    }
                }
            }

            $$unit_test {
                symbian:RSS_RULES = "group_name=\"HbUnitTests\";"
                else:target.path = $${HB_TEST_INSTALL_DIR}/unit
            } else:$$loc_test {
                symbian:RSS_RULES = "group_name=\"HbLocTests\";"
                else:target.path = $${HB_TEST_INSTALL_DIR}/loc
            } else:$$perf_test {
                symbian:RSS_RULES = "group_name=\"HbPerfApps\";"
                else:target.path = $${HB_TEST_INSTALL_DIR}/perf
            } else {
                symbian:RSS_RULES = "group_name=\"HbUnknown\";"
                else:target.path = $${HB_TEST_INSTALL_DIR}/unknown
            }
            !symbian:INSTALLS += target

        } else:contains(_PRO_FILE_PWD_, .*/fute/.*) {

            # a fute app
            target.path = $${HB_TEST_INSTALL_DIR}/fute
            INSTALLS += target
            symbian {
                RSS_RULES = "group_name=\"HbTestApps\";"
            }

        }
    }
}

!contains(QMAKE_EXTRA_TARGETS, test):QMAKE_EXTRA_TARGETS += test
!contains(QMAKE_EXTRA_TARGETS, autotest)::QMAKE_EXTRA_TARGETS += autotest
!contains(QMAKE_EXTRA_TARGETS, citest)::QMAKE_EXTRA_TARGETS += citest

