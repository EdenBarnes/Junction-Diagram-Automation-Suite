 /**
 * @file Cable.cpp
 * @brief Definitions for the Cable class.
 *
 * This module is part of the Junction Diagram Automation Suite. Unauthorized 
 * copying, distribution, or modification is prohibited.
 * 
 * @version 0.5.0
 * @author Ethan Barnes <ebarnes@gastecheng.com>
 * @date 2025-06-16
 * @copyright Proprietary - All Rights Reserved by GasTech Engineering LLC
 *
 */

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
        if (getTerminalFootprint() <= 9) return L"Show 6";
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

void Cable::draw(AcGePoint3d origin, int terminalNumber, bool flip, const wchar_t *junctionTag, int tableNumber) const{
    AcGeVector3d fldDevOffset(-9.0, 0.0, 0.0);
    if (flip) fldDevOffset *= -1;
    
    AcDbObjectId junctionTermId;
    AcDbObjectId fldDevTermId;

    // 7-Wire cables have their own block
    if (_cableType == CableType::WIRE7) {
        junctionTermId = acadInsertBlock(L"Junction Termination (7 Wire)", origin);
        fldDevTermId = acadInsertBlock(L"Field Device Termination (7 Wire)", origin + fldDevOffset);
    } else {
        junctionTermId = acadInsertBlock(L"Junction Termination", origin);
        fldDevTermId = acadInsertBlock(L"Field Device Termination", origin + fldDevOffset);
    }

    // The blocks must be flipped if the whole cable is flipped
    acadSetDynBlockProperty(junctionTermId, L"Flip state1", AcDbEvalVariant((short)(flip ? 1 : 0)));
    acadSetDynBlockProperty(fldDevTermId, L"Flip state1", AcDbEvalVariant((short)(flip ? 1 : 0)));

    acadSetDynBlockProperty(junctionTermId, L"Visibility1", AcDbEvalVariant(_getVisState().c_str()));
    acadSetDynBlockProperty(fldDevTermId, L"Visibility1", AcDbEvalVariant(_getVisState().c_str()));

    acadSetObjectProperty(junctionTermId, AcDb::kDxfLayerName, L"SKID WIRE DC");
    acadSetObjectProperty(fldDevTermId, AcDb::kDxfLayerName, L"SKID WIRE DC");

    acadSetDynBlockProperty(fldDevTermId, L"Distance1", AcDbEvalVariant(3.0));

    // Cable lables
    std::string firstDevTag = _devices.at(0).getCombinedTag();
    std::wstring firstDevTag_W(firstDevTag.begin(), firstDevTag.end());
    firstDevTag_W.replace(firstDevTag_W.find(L' '), 1, L"-");

    wchar_t cabelLabel[32];
    swprintf(cabelLabel, L"%ls-%ls", (_ioType == IOType::DIGITAL ? L"C" : L"I"), firstDevTag_W.c_str());

    acadSetBlockAttribute(fldDevTermId, L"CL", cabelLabel);

    // Set FLDTAG attributes (different for 7 wire)
    int numFldTags = 9;
    if (_cableType == CableType::WIRE7) {
        numFldTags = 7;
    }

    for (int i = 1; i <= numFldTags; ++i) {
        int wireTerminal = terminalNumber + (i - 1);

        // Deal with gaps
        if (_cableType == CableType::WIRE7) {
            // There os a gap between tag 2 and 3, 4 and 5, and 6 and 7
            if (i > 2) wireTerminal++;
            if (i > 4) wireTerminal++;
            if (i > 6) wireTerminal++;
        } else {
            // There is a gap between tag 5 and 6 as well as 7 and 8 
            if (i > 5) wireTerminal++;
            if (i > 7) wireTerminal++;
        }


        wchar_t fldtag[32];
        swprintf(fldtag, L"%ls-TB%d(%d)", junctionTag, tableNumber, wireTerminal);

        wchar_t tagName[32];
        swprintf(tagName, L"FLDTAG%d", i);

        acadSetBlockAttribute(junctionTermId, tagName, fldtag);
        acadSetBlockAttribute(fldDevTermId, tagName, fldtag);
    }

    // Draw every device
    AcGeVector3d deviceOffset(0.0, -0.25, 0.0);
    int numTerms = 0;
    for (Device device : _devices) {
        device.draw(origin + fldDevOffset + deviceOffset * numTerms, flip);

        numTerms += device.getTerminalFootprint();
    }
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

// TODO: REMOVE
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