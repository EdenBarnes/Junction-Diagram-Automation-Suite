 /**
 * @file main.cpp
 * @brief Entrypoint for the ObjectArx plugin. Handling AutoCAD messages,
 *        loading, and unloading the plugin are done here.
 *
 * This module is part of the Junction Diagram Automation Suite. Unauthorized 
 * copying, distribution, or modification is prohibited.
 * 
 * @version 1.1.2
 * @author Ethan Barnes <ebarnes@gastecheng.com>
 * @date 2025-06-16
 * @copyright Proprietary - All Rights Reserved by GasTech Engineering LLC
 *
 */

#include "main.h"

// -----------------------------------------------------------------------------
// Function Definitions
// -----------------------------------------------------------------------------

extern "C" AcRx::AppRetCode acrxEntryPoint(AcRx::AppMsgCode msg, void* appId)
{
    switch(msg) {
    case AcRx::kInitAppMsg:
    {
        acrxUnlockApplication(appId);
        acrxRegisterAppMDIAware(appId);

        initApp();
    }
    break;
    case AcRx::kUnloadAppMsg:
        unloadApp();
    break;
    }
    return AcRx::kRetOK;
}

void initApp() {
    acedRegCmds->addCommand(L"GSTCH_WIRING_COMMANDS", L"GSTCH_BUILDJUNCTION", L"BUILDJUNCTION", ACRX_CMD_MODAL, buildJunctionBox);
    acedRegCmds->addCommand(L"GSTCH_WIRING_COMMANDS", L"GSTCH_FLIPCABLE", L"FLIPCABLE", ACRX_CMD_MODAL | ACRX_CMD_USEPICKSET | ACRX_CMD_REDRAW, flipCable);
    acedRegCmds->addCommand(L"GSTCH_WIRING_COMMANDS", L"GSTCH_REINDEXCABLE", L"REINDEXCABLE", ACRX_CMD_MODAL | ACRX_CMD_USEPICKSET | ACRX_CMD_REDRAW, reIndexCable);
}

void unloadApp() {
    acedRegCmds->removeGroup(L"GSTCH_WIRING_COMMANDS");
}