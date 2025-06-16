 /******************************************************************************
 * \file Cable.cpp
 * \brief Definitions for the Cable class.
 *
 * This module is part of the Junction Diagram Automation Suite. Unauthorized 
 * copying, distribution, or modification is prohibited.
 * 
 * \version 0.3.0
 * \author Ethan Barnes <ebarnes@gastecheng.com>
 * \date 2025-06-16
 * \copyright Proprietary - All Rights Reserved by GasTech Engineering LLC
 *
 ******************************************************************************/

#include "Cable.h"

// -----------------------------------------------------------------------------
// Function Definitions
// -----------------------------------------------------------------------------

Cable::Cable(CableType wireType, SystemType sysType, IOType ioType) : 
_cableType(wireType),
_sysType(sysType),
_ioType(ioType)
{}

std::wstring Cable::_getVisState() const{
    // Get the visual state of the cable blocks from the properties of the cable
    switch (_cableType)
    {
    case CableType::WIRE7:
        if (getTerminalFootprint() <= 6) return L"Show 6";
        return L"Show All";
    
    case CableType::TRIAD1:
        return L"Triad";

    case CableType::PAIR1:
        return L"1 Pair";

    case CableType::PAIR2:
        return L"2 Pair";

    case CableType::PAIR4:
        if (getTerminalFootprint() <= 9) return L"3 Pair";
        return L"4 Pair";
    }

    return L"1 Pair";
}

void Cable::draw(AcGePoint3d origin) const{
    // TODO : If the cable is 7-wire. there are seperate junction termination and field device terminations

    AcDbObjectId junctionTermId = acadInsertBlock(L"Junction Termination", origin);

    acadSetDynBlockProperty(junctionTermId, L"Visibility1", AcDbEvalVariant(_getVisState().c_str()));
    acadSetObjectProperty(junctionTermId, AcDb::kDxfLayerName, L"SKID WIRE DC");
}

void Cable::addDevice(Device device) {
    _devices.push_back(device);
}

CableType Cable::getCableType() const{
    return _cableType;
}

SystemType Cable::getSystemType() const{
    return _sysType;
}

IOType Cable::getIOType() const{
    return _ioType;
}

std::vector<Device> Cable::getDevices() const{
    return _devices;
}

// After adding devices, determine the number of terminals needed for this cable
int Cable::getTerminalFootprint() const{
    int footprint = 0;
    for (Device device : _devices) {
        footprint += device.getTerminalFootprint();
    }
    return footprint;
}

CableType Cable::getWireTypeFromCell(const std::string& cell) {
    if (cell == "1 Pair") return CableType::PAIR1;
    else if (cell == "2 Pair") return CableType::PAIR2;
    else if (cell == "4 Pair") return CableType::PAIR4;
    else if (cell == "1 Triad") return CableType::TRIAD1;
    else if (cell == "1-7/C") return CableType::WIRE7;
    else return CableType::PAIR1;
}

SystemType Cable::getSystemTypeFromCell(const std::string& cell) {
    if (cell == "Safety") return SystemType::SAFETY;

    return SystemType::CONTROL;
}

IOType Cable::getIOTypeFromCell(const std::string& cell) {
    if (cell.at(0) == 'D') return IOType::DIGITAL;

    return IOType::ANALOG;
}

Device Cable::operator[](int index) const{
    return _devices.at(index);
}

// For sorting
bool Cable::operator<(const Cable& rhs) const{
    if (_sysType != rhs._sysType)
        return _sysType == SystemType::CONTROL; // CONTROL before SAFETY

    if (_ioType != rhs._ioType)
        return _ioType == IOType::ANALOG; // ANALOG before DIGITAL

    return _devices.at(0) < rhs._devices.at(0); // fallback: sort by first device
}

std::string Cable::textDesc() const{
    std::string desc = "";

    switch (_cableType)
    {
    case CableType::PAIR1 :
        desc += "1 PAIR";
        break;
    case CableType::PAIR2 :
        desc += "2 PAIR";
        break;
    case CableType::PAIR4 :
        desc += "4 PAIR";
        break;
    case CableType::TRIAD1 :
        desc += "1 TRIAD";
        break;
    case CableType::WIRE7 :
        desc += "7 WIRE";
        break;
    }

    desc += ", ";

    switch (_sysType)
    {
    case SystemType::SAFETY :
        desc += "Safety";
        break;
    case SystemType::CONTROL :
        desc += "Control";
        break;
    }

    desc += ", ";

    switch (_ioType)
    {
    case IOType::DIGITAL :
        desc += "Digital";
        break;
    case IOType::ANALOG :
        desc += "Analog";
        break;
    }

    return desc;
}