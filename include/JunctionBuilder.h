/**
 * @file JunctionBuilder.h
 * @brief Interface for building junction boxes.
 *
 * This module is part of the Junction Diagram Automation Suite. Unauthorized 
 * copying, distribution, or modification is prohibited.
 * 
 * @version 1.2.0
 * @author Ethan Barnes <ebarnes@gastecheng.com>
 * @date 2025-06-19
 * @copyright Proprietary - All Rights Reserved by GasTech Engineering LLC
 *
 */

#pragma once

#define NOMINMAX // makes std::numeric_limits<int>::max() work

#include <set>
#include <string>
#include <limits> // for std::numeric_limits
#include <regex>

#include <windows.h>
#include <commdlg.h> // for GetOpenFileName

#include "acedads.h"

#include "OpenXLSX.hpp"

#include "Cable.h"
#include "Device.h"
#include "resource.h"

/**
 * @brief Build a junction box based on the provided specifications.
 *
 * This function is responsible for constructing a junction box by utilizing 
 * available data such as device and cable information. It involves generating
 * the layout, placing components appropriately within the AutoCAD environment,
 * and ensuring all connections are accurately represented.
 *
 * The process may include reading from an Excel file to gather necessary input
 * data, interacting with AutoCAD for drawing purposes, and managing various 
 * attributes related to devices and cables. It serves as a core functionality 
 * of the Junction Diagram Automation Suite.
 */
void buildJunctionBox();

/**
 * @brief Flip a cable or a set of cables.
 * 
 * This function asks the user to select a set of cable-related blocks, and then
 * automatically flips the cables about the axis of the junction termination block
 */
void flipCable();

/**
 * @brief Change the terminal index of a cable or set of cables.
 * 
 * This function asks the user to select a set of cable-related blocks, and then asks
 * for the terminal number of the top wire. It then automatically changes the attributes
 * of each cable to match the respective terminal blocks they attach to, assuming proper
 * spacing.
 */
void reIndexCable();