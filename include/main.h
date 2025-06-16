/**
 * @file main.h
 * @brief Public declarations and interface for the plugin entrypoint
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

#pragma once

// #include <cmath>
// #include <windows.h>
// #include <commdlg.h>
// #include <fstream>
// #include <sstream>
// #include <set>
// #include <vector>

#include "Version.h"

#include "rxregsvc.h"
#include "acutads.h"
#include "aced.h"

#include "JunctionBuilder.h"
// #include "acdb.h"
// #include "dbents.h"
// #include "dbapserv.h"
// #include "dbdynblk.h"
// #include "geassign.h"
// #include "adscodes.h"
// #include "acedads.h"
// #include "acedCmdNF.h"
// #include "actrans.h"
// #include "dbeval.h"

// #include "OpenXLSX.hpp"

// #include "cable.h"


void initApp();
void unloadApp();
// AcDbObjectId insertBlock(wchar_t* blockName, AcGePoint3d& insertPoint);
// void updateBlockAttribute(AcDbObjectId blockRefId, const wchar_t* tag, const wchar_t* value);
// void setDynamicBlockProperty(AcDbObjectId blockRefId, const wchar_t* propertyName, const AcDbEvalVariant& newValue);
// std::set<std::wstring> getJunctionBoxTags(OpenXLSX::XLWorksheet& wks);
void buildJunctionCommand();
// void askForFile(wchar_t* filePath);
// void drawCable(Cable cable, std::wstring junctionBoxTag);