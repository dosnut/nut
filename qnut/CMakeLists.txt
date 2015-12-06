project(qnut)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

set(INSTALL_SHARE_PREFIX ${CMAKE_INSTALL_PREFIX}/share)
set(INSTALL_SHARE_TARGET_PREFIX ${INSTALL_SHARE_PREFIX}/qnut)

set(FORMS
	ui/accesspointconfig.ui
	ui/adhocconfig.ui
	ui/devicedetails.ui
	ui/devicesettings.ui
	ui/ipconfiguration.ui
	ui/wirelesssettings.ui
)

file(GLOB TRANSLATIONS_TS translations/qnut_*.ts)
file(GLOB_RECURSE TRANSLATIONS_USAGES *.cpp *.ui)

qt5_wrap_ui(FORMS_H ${FORMS})

add_executable(qnut
	cabstractwifinetconfigdialog.cpp
	caccesspointconfig.cpp
	cadhocconfig.cpp
	cconnectionmanager.cpp
	cdevicesettings.cpp
	cipconfiguration.cpp
	cnotificationmanager.cpp
	common.cpp
	cuidevice.cpp
	cwirelesssettings.cpp
	main.cpp

	modelview/cavailableapmodel.cpp
	modelview/ccommandlistmodel.cpp
	modelview/cdnslistmodel.cpp
	modelview/cenvironmentdetailsmodel.cpp
	modelview/cenvironmenttreemodel.cpp
	modelview/cinterfacedetailsmodel.cpp
	modelview/cipeditdelegate.cpp
	modelview/cmanagedapmodel.cpp
	modelview/cuidevicemodel.cpp

	utils/cerrorcodeevaluator.cpp

	${FORMS_H}
)

qt5_translate(
	SOURCES ${TRANSLATIONS_USAGES}
	TRANSLATIONS ${TRANSLATIONS_TS}
	TARGET qnut)

target_link_libraries(qnut
	nutclient
	Qt5::Gui
	Qt5::Widgets
)

file(GLOB RES_ICONS res/*.png res/qnut.svg res/qnut_small.svg)

install(TARGETS qnut DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(FILES ${TRANSLATIONS_QM} DESTINATION ${INSTALL_SHARE_TARGET_PREFIX}/lang)
install(FILES ${RES_ICONS} DESTINATION ${INSTALL_SHARE_TARGET_PREFIX}/icons)
install(FILES qnut.desktop DESTINATION ${INSTALL_SHARE_PREFIX}/applications)
