 /**
 * @file Device.cpp
 * @brief Definitions for the Device class.
 *
 * This module is part of the Junction Diagram Automation Suite. Unauthorized 
 * copying, distribution, or modification is prohibited.
 * 
 * @version 1.1.0
 * @author Ethan Barnes <ebarnes@gastecheng.com>
 * @date 2025-06-16
 * @copyright Proprietary - All Rights Reserved by GasTech Engineering LLC
 *
 */

#include "Device.h"

// -----------------------------------------------------------------------------
// Function Definitions
// -----------------------------------------------------------------------------

Device::Device(std::string combinedTag, int footprint)
{
    _tag = combinedTag.substr(0, combinedTag.find(" "));
    _number = combinedTag.substr(combinedTag.find(" ") + 1);
    _footprint = footprint;
}

void Device::draw(AcGePoint3d origin, bool flip) const {
    AcGePoint3d termOrigin = origin + AcGeVector3d(-0.3438 * (flip ? -1 : 1), 0.125, 0.0);

    static const AcGeVector3d termOffset(0.0, -0.25, 0.0);

    AcDbObjectId term1Id = acadInsertBlock(L"TBWIREMINI", termOrigin);
    AcDbObjectId term2Id = acadInsertBlock(L"TBWIREMINI", termOrigin + termOffset);

    acadSetBlockAttribute(term1Id, L"#", L"+");
    acadSetBlockAttribute(term2Id, L"#", L"-");

    acadSetObjectProperty(term1Id, AcDb::kDxfLayerName, L"ELECTRICAL - LIGHT");
    acadSetObjectProperty(term2Id, AcDb::kDxfLayerName, L"ELECTRICAL - LIGHT");

    AcGeVector3d symbolOffset(-0.9375, -0.125, 0.0);
    if (flip) symbolOffset.x *= -1;

    if (_footprint == 4) {
        // TRIAD

        AcDbObjectId term3Id = acadInsertBlock(L"TBWIREMINI", termOrigin + termOffset * 2);

        acadSetBlockAttribute(term3Id, L"#", L"REF");

        acadSetObjectProperty(term3Id, AcDb::kDxfLayerName, L"ELECTRICAL - LIGHT");

        symbolOffset.y = -0.25;
    }

    if (_footprint == 6) {
        // 2 pair

        AcDbObjectId term3Id = acadInsertBlock(L"TBWIREMINI", termOrigin + termOffset * 3);
        AcDbObjectId term4Id = acadInsertBlock(L"TBWIREMINI", termOrigin + termOffset * 4);

        acadSetBlockAttribute(term1Id, L"#", L"L");
        acadSetBlockAttribute(term2Id, L"#", L"N");
        acadSetBlockAttribute(term3Id, L"#", L"5");
        acadSetBlockAttribute(term4Id, L"#", L"6");

        acadSetObjectProperty(term3Id, AcDb::kDxfLayerName, L"ELECTRICAL - LIGHT");
        acadSetObjectProperty(term4Id, AcDb::kDxfLayerName, L"ELECTRICAL - LIGHT");

        symbolOffset.y = -0.5;
    }

    // Draw the symbol
    AcDbObjectId symbolId = acadInsertBlock(L"INST SYMBOL", origin + symbolOffset);

    std::wstring tag_W(_tag.begin(), _tag.end());
    std::wstring number_W(_number.begin(), _number.end());

    acadSetBlockAttribute(symbolId, L"TAG", tag_W.c_str());
    acadSetBlockAttribute(symbolId, L"NUMBER", number_W.c_str());
}

std::string Device::getTag() const {
    return _tag;
}

std::string Device::getNumber() const {
    return _number;
}

std::string Device::getCombinedTag() const {
    return _tag + " " + _number;
}

int Device::getTerminalFootprint() const {
    return _footprint;
}

int Device::footprintFromCells(const std::string& combinedTag, const std::string& instrumentSpec) {
    std::string tag = combinedTag.substr(0, combinedTag.find(" "));

    if (tag == "LSLL" || tag == "LSHH" || tag == "LS") {
        if (instrumentSpec == "ULTRASONIC SW") {
            return 6;
        }
    }

    if (tag == "FT") {
        if (instrumentSpec == "ULTRASONIC FLOW" || instrumentSpec == "CORIOLIS FLOW") {
            return 6;
        }
    }

    if (tag == "TT" && instrumentSpec == "RTD") {
        return 4;
    }

    return 3;
}

bool Device::operator<(const Device& rhs) const{
    return getCombinedTag().compare(rhs.getCombinedTag()) < 0;
}