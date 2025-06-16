 /**
 * @file main.cpp
 * @brief Entrypoint for the ObjectArx plugin. Handling AutoCAD messages,
 *        loading, and unloading the plugin are done here.
 *
 * This module is part of the Junction Diagram Automation Suite. Unauthorized 
 * copying, distribution, or modification is prohibited.
 * 
 * @version 1.0.0
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
}

void unloadApp() {
    acedRegCmds->removeGroup(L"GSTCH_WIRING_COMMANDS");
}