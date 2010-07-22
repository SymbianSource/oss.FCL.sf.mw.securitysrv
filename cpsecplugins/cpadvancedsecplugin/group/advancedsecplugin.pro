
#
# Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
# All rights reserved.
# This component and the accompanying materials are made available
# under the terms of "Eclipse Public License v1.0"
# which accompanies this distribution, and is available
# at the URL "http://www.eclipse.org/legal/epl-v10.html".
#
# Initial Contributors:
# Nokia Corporation - initial contribution.
#
# Contributors:
#
# Description: 
#

TEMPLATE = lib
TARGET = cpadvancedsecplugin

CONFIG += hb plugin

LIBS += -lcpframework

include ( advancedsecplugin.pri )
include ( ../rom/cpsecplugins_rom.pri )

symbian: { 
	TARGET.CAPABILITY = CAP_ECOM_PLUGIN
	TARGET.UID3 = 0X2002E684
	INCLUDEPATH += $$MW_LAYER_SYSTEMINCLUDE
	INCLUDEPATH += ../inc
	TARGET.EPOCALLOWDLLDATA = 1
	LIBS += -lcertstore
	LIBS += -lCTFramework
	LIBS += -lX509
	LIBS += -lpkixcert
	LIBS += -lx509certnameparser
	LIBS += -lhash
	LIBS += -lcryptography
	LIBS += -lDevTokenClient
	LIBS += -lcrypto
	LIBS += -lefsrv

	PLUGIN_STUB_PATH = /resource/qt/plugins/controlpanel
		
	deploy.path = C:
	pluginstub.sources = $${TARGET}.dll
	pluginstub.path = $$PLUGIN_STUB_PATH
	DEPLOYMENT += pluginstub

    qtplugins.path = $$PLUGIN_STUB_PATH
    qtplugins.sources += qmakepluginstubs/$${TARGET}.qtplugin
     
    for(qtplugin, qtplugins.sources):BLD_INF_RULES.prj_exports += "./$$qtplugin  $$deploy.path$$qtplugins.path/$$basename(qtplugin)" 
	:BLD_INF_RULES.prj_exports += "../translations/certificate_management_ar.ts /epoc32/include/platform/qt/translations/certificate_management_ar.ts"  
	:BLD_INF_RULES.prj_exports += "../translations/certificate_management_bg.ts /epoc32/include/platform/qt/translations/certificate_management_bg.ts"  
	:BLD_INF_RULES.prj_exports += "../translations/certificate_management_ca.ts /epoc32/include/platform/qt/translations/certificate_management_ca.ts"  
	:BLD_INF_RULES.prj_exports += "../translations/certificate_management_cs.ts /epoc32/include/platform/qt/translations/certificate_management_cs.ts"  
	:BLD_INF_RULES.prj_exports += "../translations/certificate_management_da.ts /epoc32/include/platform/qt/translations/certificate_management_da.ts"  
	:BLD_INF_RULES.prj_exports += "../translations/certificate_management_de.ts /epoc32/include/platform/qt/translations/certificate_management_de.ts"  
	:BLD_INF_RULES.prj_exports += "../translations/certificate_management_el.ts /epoc32/include/platform/qt/translations/certificate_management_el.ts"  
	:BLD_INF_RULES.prj_exports += "../translations/certificate_management_en.ts /epoc32/include/platform/qt/translations/certificate_management_en.ts"  
	:BLD_INF_RULES.prj_exports += "../translations/certificate_management_en_US.ts /epoc32/include/platform/qt/translations/certificate_management_en_US.ts"  
	:BLD_INF_RULES.prj_exports += "../translations/certificate_management_es.ts /epoc32/include/platform/qt/translations/certificate_management_es.ts"  
	:BLD_INF_RULES.prj_exports += "../translations/certificate_management_es_419.ts /epoc32/include/platform/qt/translations/certificate_management_es_419.ts"  
	:BLD_INF_RULES.prj_exports += "../translations/certificate_management_et.ts /epoc32/include/platform/qt/translations/certificate_management_et.ts"  
	:BLD_INF_RULES.prj_exports += "../translations/certificate_management_eu.ts /epoc32/include/platform/qt/translations/certificate_management_eu.ts"  
	:BLD_INF_RULES.prj_exports += "../translations/certificate_management_fi.ts /epoc32/include/platform/qt/translations/certificate_management_fi.ts"  
	:BLD_INF_RULES.prj_exports += "../translations/certificate_management_fr.ts /epoc32/include/platform/qt/translations/certificate_management_fr.ts"  
	:BLD_INF_RULES.prj_exports += "../translations/certificate_management_fr_CA.ts /epoc32/include/platform/qt/translations/certificate_management_fr_CA.ts"  
	:BLD_INF_RULES.prj_exports += "../translations/certificate_management_gl.ts /epoc32/include/platform/qt/translations/certificate_management_gl.ts"  
	:BLD_INF_RULES.prj_exports += "../translations/certificate_management_he.ts /epoc32/include/platform/qt/translations/certificate_management_he.ts"  
	:BLD_INF_RULES.prj_exports += "../translations/certificate_management_hi.ts /epoc32/include/platform/qt/translations/certificate_management_hi.ts"  
	:BLD_INF_RULES.prj_exports += "../translations/certificate_management_hr.ts /epoc32/include/platform/qt/translations/certificate_management_hr.ts"  
	:BLD_INF_RULES.prj_exports += "../translations/certificate_management_hu.ts /epoc32/include/platform/qt/translations/certificate_management_hu.ts"  
	:BLD_INF_RULES.prj_exports += "../translations/certificate_management_id.ts /epoc32/include/platform/qt/translations/certificate_management_id.ts"  
	:BLD_INF_RULES.prj_exports += "../translations/certificate_management_is.ts /epoc32/include/platform/qt/translations/certificate_management_is.ts"  
	:BLD_INF_RULES.prj_exports += "../translations/certificate_management_it.ts /epoc32/include/platform/qt/translations/certificate_management_it.ts"  
	:BLD_INF_RULES.prj_exports += "../translations/certificate_management_ja.ts /epoc32/include/platform/qt/translations/certificate_management_ja.ts"  
	:BLD_INF_RULES.prj_exports += "../translations/certificate_management_ko.ts /epoc32/include/platform/qt/translations/certificate_management_ko.ts"  
	:BLD_INF_RULES.prj_exports += "../translations/certificate_management_lt.ts /epoc32/include/platform/qt/translations/certificate_management_lt.ts"  
	:BLD_INF_RULES.prj_exports += "../translations/certificate_management_lv.ts /epoc32/include/platform/qt/translations/certificate_management_lv.ts"  
	:BLD_INF_RULES.prj_exports += "../translations/certificate_management_ms.ts /epoc32/include/platform/qt/translations/certificate_management_ms.ts"  
	:BLD_INF_RULES.prj_exports += "../translations/certificate_management_nl.ts /epoc32/include/platform/qt/translations/certificate_management_nl.ts"  
	:BLD_INF_RULES.prj_exports += "../translations/certificate_management_no.ts /epoc32/include/platform/qt/translations/certificate_management_no.ts"  
	:BLD_INF_RULES.prj_exports += "../translations/certificate_management_pl.ts /epoc32/include/platform/qt/translations/certificate_management_pl.ts"  
	:BLD_INF_RULES.prj_exports += "../translations/certificate_management_pt.ts /epoc32/include/platform/qt/translations/certificate_management_pt.ts"  
	:BLD_INF_RULES.prj_exports += "../translations/certificate_management_pt_BR.ts /epoc32/include/platform/qt/translations/certificate_management_pt_BR.ts"  
	:BLD_INF_RULES.prj_exports += "../translations/certificate_management_ro.ts /epoc32/include/platform/qt/translations/certificate_management_ro.ts"  
	:BLD_INF_RULES.prj_exports += "../translations/certificate_management_ru.ts /epoc32/include/platform/qt/translations/certificate_management_ru.ts"  
	:BLD_INF_RULES.prj_exports += "../translations/certificate_management_sk.ts /epoc32/include/platform/qt/translations/certificate_management_sk.ts"  
	:BLD_INF_RULES.prj_exports += "../translations/certificate_management_sl.ts /epoc32/include/platform/qt/translations/certificate_management_sl.ts"  
	:BLD_INF_RULES.prj_exports += "../translations/certificate_management_sr.ts /epoc32/include/platform/qt/translations/certificate_management_sr.ts"  
	:BLD_INF_RULES.prj_exports += "../translations/certificate_management_sv.ts /epoc32/include/platform/qt/translations/certificate_management_sv.ts"  
	:BLD_INF_RULES.prj_exports += "../translations/certificate_management_tl.ts /epoc32/include/platform/qt/translations/certificate_management_tl.ts"  
	:BLD_INF_RULES.prj_exports += "../translations/certificate_management_th.ts /epoc32/include/platform/qt/translations/certificate_management_th.ts" 
	:BLD_INF_RULES.prj_exports += "../translations/certificate_management_tr.ts /epoc32/include/platform/qt/translations/certificate_management_tr.ts"  
	:BLD_INF_RULES.prj_exports += "../translations/certificate_management_uk.ts /epoc32/include/platform/qt/translations/certificate_management_uk.ts"  
	:BLD_INF_RULES.prj_exports += "../translations/certificate_management_ur.ts /epoc32/include/platform/qt/translations/certificate_management_ur.ts"  
	:BLD_INF_RULES.prj_exports += "../translations/certificate_management_vi.ts /epoc32/include/platform/qt/translations/certificate_management_vi.ts"  
	:BLD_INF_RULES.prj_exports += "../translations/certificate_management_zh.ts /epoc32/include/platform/qt/translations/certificate_management_zh.ts"  
	:BLD_INF_RULES.prj_exports += "../translations/certificate_management_zh_HK.ts /epoc32/include/platform/qt/translations/certificate_management_zh_HK.ts"  
	:BLD_INF_RULES.prj_exports += "../translations/certificate_management_zh_TW.ts /epoc32/include/platform/qt/translations/certificate_management_zh_TW.ts"  
	
	
	}

TRANSLATIONS= certificate_management.ts