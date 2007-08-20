#include "libnut_cli.h"

namespace libnut {
    CDeviceManager::CDeviceManager(QObject * parent) : QObject(parent) {}

//    void CDevice::setEnabled(bool value) {}
    void CDevice::enable() {}
    void CDevice::disable() {}
    CDevice::CDevice(QObject * parent) : QObject(parent) {}
    void CDeviceManager::refreshAll() {}
    
    void CEnvironment::enter() {}
    CEnvironment::CEnvironment(CDevice * parent) : QObject(parent) {}
    
    void CInterface::activate() {}
    void CInterface::deactivate() {}
    void CInterface::setIP(QHostAddress & address) {}
    void CInterface::setNetmask(QHostAddress & address) {}
    void CInterface::setGateway(QHostAddress & address) {}
    void CInterface::setStatic(bool state) {}
    
    CInterface::CInterface(QObject * parent) : QObject(parent) {}
};
