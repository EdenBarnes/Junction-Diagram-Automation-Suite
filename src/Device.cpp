 /******************************************************************************
 * \file Device.cpp
 * \brief Definitions for the Device class.
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
    // TODO: Add support for triads
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

    return 3;
}

bool Device::operator<(const Device& rhs) const{
    return getCombinedTag().compare(rhs.getCombinedTag()) < 0;
}