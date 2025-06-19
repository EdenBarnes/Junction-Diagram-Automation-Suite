/**
 * @file main.h
 * @brief Public declarations and interface for the plugin entrypoint
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

#include "Version.h"

#include "rxregsvc.h"
#include "acutads.h"
#include "aced.h"

#include "JunctionBuilder.h"

void initApp();
void unloadApp();
void buildJunctionCommand();