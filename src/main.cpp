 /**
 * @file main.cpp
 * @brief Entrypoint for the ObjectArx plugin. Handling AutoCAD messages,
 *        loading, and unloading the plugin are done here.
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

//TODO: REMOVE ALL OF THE STUFF BELOW THIS TODO

// AcDbObjectId insertBlock(wchar_t* blockName, AcGePoint3d& insertPoint)
// {
//     AcDbDatabase* db = acdbHostApplicationServices()->workingDatabase();
//     AcDbBlockTable* pBlockTable = nullptr;
//     if (db->getBlockTable(pBlockTable, AcDb::kForRead) != Acad::eOk)
//         return AcDbObjectId::kNull;

//     AcDbObjectId blockId;
//     if (pBlockTable->getAt(blockName, blockId) != Acad::eOk)
//     {
//         acutPrintf(L"\nBlock '%ls' not found in drawing.", blockName);
//         pBlockTable->close();
//         return AcDbObjectId::kNull;
//     }

//     AcDbBlockReference* pBlockRef = new AcDbBlockReference(insertPoint, blockId);

//     AcDbBlockTableRecord* pModelSpace = nullptr;
//     if (pBlockTable->getAt(ACDB_MODEL_SPACE, pModelSpace, AcDb::kForWrite) != Acad::eOk)
//     {
//         pBlockTable->close();
//         delete pBlockRef;
//         return AcDbObjectId::kNull;
//     }

//     AcDbObjectId refId;
//     if (pModelSpace->appendAcDbEntity(refId, pBlockRef) != Acad::eOk)
//     {
//         acutPrintf(L"\nFailed to insert block reference.");
//         delete pBlockRef;
//         pModelSpace->close();
//         pBlockTable->close();
//         return AcDbObjectId::kNull;
//     }

//      AcDbBlockTableRecord* pBlockDef = nullptr;
//     if (acdbOpenObject(pBlockDef, blockId, AcDb::kForRead) == Acad::eOk)
//     {
//         AcDbBlockTableRecordIterator* pIter = nullptr;
//         if (pBlockDef->newIterator(pIter) == Acad::eOk)
//         {
//             for (; !pIter->done(); pIter->step())
//             {
//                 AcDbEntity* pEnt = nullptr;
//                 if (pIter->getEntity(pEnt, AcDb::kForRead) == Acad::eOk)
//                 {
//                     AcDbAttributeDefinition* pAttDef = AcDbAttributeDefinition::cast(pEnt);
//                     if (pAttDef && !pAttDef->isConstant())
//                     {
//                         AcDbAttribute* pAtt = new AcDbAttribute();

//                         // Transform attribute position to match block reference transform
//                         AcGePoint3d pos = pAttDef->position();
//                         pos.transformBy(pBlockRef->blockTransform());

//                         pAtt->setPosition(pos);
//                         pAtt->setHeight(pAttDef->height());
//                         pAtt->setRotation(pAttDef->rotation());
//                         pAtt->setFieldLength(pAttDef->fieldLength());
//                         pAtt->setInvisible(pAttDef->isInvisible());
//                         pAtt->setTag(pAttDef->tag());
//                         pAtt->setTextString(pAttDef->textString());  // Set default text or override
//                         pAtt->setHorizontalMode(pAttDef->horizontalMode());
//                         pAtt->setVerticalMode(pAttDef->verticalMode());
//                         pAtt->adjustAlignment(db); // Ensure it's aligned properly

//                         AcDbObjectId attId;
//                         if (pBlockRef->appendAttribute(attId, pAtt) == Acad::eOk)
//                             pAtt->close();
//                         else
//                             delete pAtt;
//                     }
//                     pEnt->close();
//                 }
//             }
//             delete pIter;
//         }
//         pBlockDef->close();
//     }

//     pBlockRef->close();
//     pModelSpace->close();
//     pBlockTable->close();

//     return refId;
// }

// void updateBlockAttribute(AcDbObjectId blockRefId, const wchar_t* tag, const wchar_t* value) {
//     if (blockRefId.isNull()) return;

//     AcDbBlockReference* pBlockRef = nullptr;
//     if (acdbOpenObject(pBlockRef, blockRefId, AcDb::kForWrite) != Acad::eOk) return;

//     AcDbObjectIterator* it = pBlockRef->attributeIterator();
//     for (; !it->done(); it->step()) {
//         AcDbObjectId attId = it->objectId();
//         AcDbAttribute* pAtt = nullptr;

//         if (acdbOpenObject(pAtt, attId, AcDb::kForWrite) == Acad::eOk) {
//             if (wcscmp(pAtt->tag(), tag) == 0) pAtt->setTextString(value);
//             pAtt->close();
//         }
//     }

//     delete it;
//     pBlockRef->close();
// }

// void setDynamicBlockProperty(AcDbObjectId blockRefId, const wchar_t* propertyName, const AcDbEvalVariant& newValue) {
//     AcDbBlockReference* pBlockRef = nullptr;
//     if (acdbOpenObject(pBlockRef, blockRefId, AcDb::kForWrite) != Acad::eOk || pBlockRef == nullptr) {
//         acutPrintf(L"\nFailed to open block reference.");
//         return;
//     }

//     AcDbDynBlockReference dynBlockRef(pBlockRef);
//     AcDbDynBlockReferencePropertyArray propArray;
//     dynBlockRef.getBlockProperties(propArray);

//     for (int i = 0; i < propArray.length(); ++i) {
//         AcDbDynBlockReferenceProperty prop = propArray[i];

//         if (wcscmp(prop.propertyName(), propertyName) == 0) {
//             Acad::ErrorStatus es = prop.setValue(newValue);
//             if (es != Acad::eOk) {
//                 acutPrintf(L"\nFailed to set value for property: %ls", propertyName);
//             } else {
//                 acutPrintf(L"\nProperty %ls set successfully.", propertyName);
//             }
//             break;
//         }
//     }

//     pBlockRef->close();
// }

// std::set<std::wstring> getJunctionBoxTags(OpenXLSX::XLWorksheet& wks) {
//     std::set<std::wstring> junctionBoxTags;

//     for (int row = 3; wks.cell(row, 4).value().getString() != ""; ++row) {
//         std::string junctionBoxTag = wks.cell(row, 3).value();
//         std::wstring junctionBoxTag_W(junctionBoxTag.begin(), junctionBoxTag.end());

//         junctionBoxTags.insert(junctionBoxTag_W);
//     }

//     return junctionBoxTags;
// }

// void drawDevice(Device device, AcGePoint3d position, WireType_t type) {
//     // Create objects that make up a device
//     AcDbObjectId term1Id = insertBlock(L"TBWIREMINI", position + AcGeVector3d(-0.3438,  0.1250, 0.0));
//     AcDbObjectId term2Id = insertBlock(L"TBWIREMINI", position + AcGeVector3d(-0.3438, -0.1250, 0.0));

//     updateBlockAttribute(term1Id, L"#", L"+");
//     updateBlockAttribute(term2Id, L"#", L"-");

//     AcDbBlockReference* pBlockRef = nullptr;
//     if (acdbOpenObject(pBlockRef, term1Id, AcDb::kForWrite) == Acad::eOk) {
//         pBlockRef->setLayer(L"ELECTRICAL - LIGHT");
//         pBlockRef->close();
//     }

//     if (acdbOpenObject(pBlockRef, term2Id, AcDb::kForWrite) == Acad::eOk) {
//         pBlockRef->setLayer(L"ELECTRICAL - LIGHT");
//         pBlockRef->close();
//     }

//     if (type == WireType_t::TRIAD) {
//         AcDbObjectId term3Id = insertBlock(L"TBWIREMINI", position + AcGeVector3d(-0.3438, -0.3750, 0.0));

//         updateBlockAttribute(term3Id, L"#", L"REF");

//         if (acdbOpenObject(pBlockRef, term3Id, AcDb::kForWrite) == Acad::eOk) {
//             pBlockRef->setLayer(L"ELECTRICAL - LIGHT");
//             pBlockRef->close();
//         }
//     }

//     AcGePoint3d instPosition(0.0,0.0,0.0);
//     if (type == WireType_t::PAIR) {
//         instPosition = position + AcGeVector3d(-0.9376, -0.1250, 0.0);
//     } else if (type == WireType_t::TRIAD) {
//         instPosition = position + AcGeVector3d(-0.9376, -0.2500, 0.0);
//     }

//     AcDbObjectId instId = insertBlock(L"INST SYMBOL", instPosition);
//     updateBlockAttribute(instId, L"TAG", device.tag.c_str());
//     updateBlockAttribute(instId, L"NUMBER", device.number.c_str());
// }

// void drawCable(Cable cable, std::wstring junctionBoxTag) {
//     // Create objects that make up a cable
//     AcDbObjectId junctionTermId = insertBlock(L"Junction Termination", cable.getOrigin());
//     AcDbObjectId fieldDevTermId = insertBlock(L"Field Device Termination", cable.getOrigin() + AcGeVector3d(-9.0,0.0,0.0));

//     // Set visibility states
//     wchar_t visStateBuffer[32];
//     if (cable.getType() == WireType_t::PAIR) {
//         swprintf(visStateBuffer, 32, L"%d Pair", cable.getNumDevices());
//     } else if (cable.getType() == WireType_t::TRIAD) {
//         wcscpy(visStateBuffer, L"Triad");
//     }
//     AcDbEvalVariant visState(visStateBuffer);
//     setDynamicBlockProperty(junctionTermId, L"Visibility1", visState);
//     setDynamicBlockProperty(fieldDevTermId, L"Visibility1", visState);

//     // Set Field Device Termination distance1
//     AcDbEvalVariant distance1(3.0);
//     setDynamicBlockProperty(fieldDevTermId, L"Distance1", distance1);

//     // Set fldtags
//     for (int i = 0; i<9; ++i) {
//         int number = cable.getTerminalNumber()+i;
//         if (i > 4) number ++;
//         if (i > 6) number ++;

//         wchar_t fldtag[32];
//         swprintf(fldtag, 32, L"%ls-TB1(%d)", junctionBoxTag.c_str(), number);

//         wchar_t atttag[32];
//         swprintf(atttag, 32, L"FLDTAG%d", i+1);

//         updateBlockAttribute(junctionTermId, atttag, fldtag);
//         updateBlockAttribute(fieldDevTermId, atttag, fldtag);
//     }

//     // Set properties
//     AcDbBlockReference* pBlockRef = nullptr;
//     if (acdbOpenObject(pBlockRef, junctionTermId, AcDb::kForWrite) == Acad::eOk) {
//         pBlockRef->setLayer(L"SKID WIRE DC");
//         pBlockRef->close();
//     }

//     if (acdbOpenObject(pBlockRef, fieldDevTermId, AcDb::kForWrite) == Acad::eOk) {
//         pBlockRef->setLayer(L"SKID WIRE DC");
//         pBlockRef->close();
//     }

//     AcGePoint3d devicePosition = cable.getOrigin() + AcGeVector3d(-9.0, 0.0, 0.0);
//     for (int i = 0; i < cable.getNumDevices(); ++i) {
//         drawDevice(cable.getDevice(i), devicePosition, cable.getType());
//         if (cable.getType() == WireType_t::PAIR) {
//             devicePosition += AcGeVector3d(0.0, -0.7498, 0.0);
//         } else if (cable.getType() == WireType_t::TRIAD) {
//             devicePosition += AcGeVector3d(0.0, -1.0006, 0.0);
//         }
//     }


// }

// void buildJunctionCommand()
// {

//     // wchar_t filePath[MAX_PATH] = {0};
//     // askForFile(filePath);

//     // if (filePath[0] == L'\0') {
//     //     acutPrintf(L"\nNo file selected.");
//     //     return;
//     // }

//     // // Convert wchar_t* to std::ifstream-friendly narrow string
//     // char narrowPath[MAX_PATH];
//     // wcstombs(narrowPath, filePath, MAX_PATH);

//     // OpenXLSX::XLDocument doc;
//     // try {
//     // doc.open(narrowPath);
//     // } catch (const std::exception& e) {
//     //     acutPrintf(L"\nFailed to open Excel file: %S", e.what());
//     //     return;
//     // }

//     // OpenXLSX::XLWorksheet wks = doc.workbook().worksheet("Cable Schedule Data");

//     // // Ask user what junction box they want to use
//     // wchar_t desiredJunctionBoxTagBuffer[32] = {0};
//     // if (acedGetString(NULL, L"\nPlease input desired junction box tag: ", desiredJunctionBoxTagBuffer, 32) != RTNORM)
//     // {
//     //     acutPrintf(L"\nCanceled.");
//     //     return;
//     // }
//     // std::wstring desiredJunctionBoxTag_W(desiredJunctionBoxTagBuffer);

//     // // Verify this is a valid junction box
//     // std::set<std::wstring> junctionBoxTags = getJunctionBoxTags(wks);
//     // if (junctionBoxTags.find(desiredJunctionBoxTag_W) == std::end(junctionBoxTags)) {
//     //     acutPrintf(L"\nThat junction box does not exist in that excel file.");
//     // }

//     // // Build a big list of cables
//     // std::vector<Cable> cables;
//     // double origin_x = 19.375;
//     // double origin_y = 12.4977;
//     // int index = 0;
//     // for (int row = 3; wks.cell(row, 4).value().getString() != ""; ++row) {

//     //     // Make sure this row is a part of our junction box
//     //     std::string junctionBoxTag = wks.cell(row, 3).value();
//     //     std::wstring junctionBoxTag_W(junctionBoxTag.begin(), junctionBoxTag.end());
//     //     if (junctionBoxTag_W != desiredJunctionBoxTag_W) continue;

//     //     // If QTY has something in it, we've started a new cable
//     //     if (wks.cell(row, 1).value().typeAsString() == "string") {
//     //         std::string QTY = wks.cell(row, 1).value();
//     //         // Build the new cable

//     //         // Is it pair or triad?
//     //         WireType_t type = WireType_t::PAIR;
//     //         if (QTY[2] == 'T') type = WireType_t::TRIAD; 

//     //         if (cables.size()!=0) origin_y -= cables.back().getHeight();

//     //         Cable newCable(type, AcGePoint3d(origin_x, origin_y, 0.0), index*3+1);
//     //         cables.push_back(newCable);
//     //     }

//     //     std::string DESTINATION = wks.cell(row, 4).value();
//     //     std::string tag = DESTINATION.substr(0, DESTINATION.find(" "));
//     //     std::string number = DESTINATION.substr(DESTINATION.find(" ")+1);

//     //     std::wstring tag_W(tag.begin(), tag.end());
//     //     std::wstring number_W(number.begin(), number.end());

//     //     // Create the new device
//     //     Device newDevice{};
//     //     newDevice.tag = tag_W;
//     //     newDevice.number = number_W;

//     //     cables.back().addDevice(newDevice);

//     //     index ++;
//     // }

//     // for (Cable cable : cables) {
//     //     drawCable(cable, desiredJunctionBoxTag_W);
//     // }

//     // acedCommandS(RTSTR, L".ATTSYNC", RTSTR, L"NAME", RTSTR, L"Junction Termination", RTNONE);
//     // acedCommandS(RTSTR, L".ATTSYNC", RTSTR, L"NAME", RTSTR, L"Field Device Termination", RTNONE);
//     // acedCommandS(RTSTR, L".ATTSYNC", RTSTR, L"NAME", RTSTR, L"TBWIREMINI", RTNONE);
//     // acedCommandS(RTSTR, L".ATTSYNC", RTSTR, L"NAME", RTSTR, L"INST SYMBOL", RTNONE);

//     // doc.save();
//     // doc.close();
// }

// void askForFile(wchar_t* filePath)
// {
//     OPENFILENAMEW ofn = {0};
//     ofn.lStructSize = sizeof(ofn);
//     ofn.hwndOwner = adsw_acadMainWnd(); // Only works if windows.h included first
//     ofn.lpstrFilter = L"All Files\0*.*\0";
//     ofn.lpstrFile = filePath;
//     ofn.nMaxFile = MAX_PATH;
//     ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

//     GetOpenFileNameW(&ofn);
// }
