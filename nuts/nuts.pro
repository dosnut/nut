
CONFIG += qdbus debug
QT += network

SOURCES += main.cpp device.cpp config.cpp
HEADERS += common.h device.h   config.h   hardware.h
YACCSOURCES += configparser.y
LEXSOURCES += configparser.l

OBJECTS_DIR = build/
