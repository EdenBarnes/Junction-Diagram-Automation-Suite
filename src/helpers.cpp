/**
 * @file helpers.cpp
 * @brief Helper functions related to database operations.
 *
 * This module is part of the Junction Diagram Automation Suite. Unauthorized 
 * copying, distribution, or modification is prohibited.
 * 
 * @version 1.1.1
 * @author Ethan Barnes <ebarnes@gastecheng.com>
 * @date 2025-06-16
 * @copyright Proprietary - All Rights Reserved by GasTech Engineering LLC
 *
 */

#include "helpers.h"

// -----------------------------------------------------------------------------
// Function Definitions
// -----------------------------------------------------------------------------

AcDbObjectId acadInsertBlock(const wchar_t* blockName, const AcGePoint3d& origin) {
    AcDbObjectId blockRefId;

    // Get the current working database
    AcDbDatabase* pDb = acdbHostApplicationServices()->workingDatabase();
    if (!pDb) {
        acutPrintf(L"\nError: No active database.");
        return AcDbObjectId::kNull;
    }

    // Open the block table for reading
    AcDbBlockTable* pBlockTable = nullptr;
    if (pDb->getBlockTable(pBlockTable, AcDb::kForRead) != Acad::eOk || !pBlockTable) {
        acutPrintf(L"\nError: Could not access block table.");
        return AcDbObjectId::kNull;
    }

    // Get the ObjectId of the block definition
    AcDbObjectId blockDefId;
    if (pBlockTable->getAt(blockName, blockDefId) != Acad::eOk || !blockDefId) {
        acutPrintf(L"\nError: Block '%ls' not found in drawing.", blockName);
        pBlockTable->close();
        return AcDbObjectId::kNull;
    }

    // Create a new block reference at the given origin
    AcDbBlockReference* pBlockRef = new AcDbBlockReference(origin, blockDefId);

    // Open Model Space for writing
    AcDbBlockTableRecord* pModelSpace = nullptr;
    if (pBlockTable->getAt(ACDB_MODEL_SPACE, pModelSpace, AcDb::kForWrite) != Acad::eOk) {
        pBlockRef->close();
        pBlockTable->close();
        return AcDbObjectId::kNull;
    }

    // Add the block reference to Model Space
    if (pModelSpace->appendAcDbEntity(blockRefId, pBlockRef) != Acad::eOk) {
        acutPrintf(L"\nError: Failed to insert block reference.");
        pBlockRef->close();
        pModelSpace->close();
        pBlockTable->close();
        return AcDbObjectId::kNull;
    }

    // Open the block definition for reading
    AcDbBlockTableRecord* pBlockDef = nullptr;
    if (acdbOpenObject(pBlockDef, blockDefId, AcDb::kForRead) != Acad::eOk || !pBlockDef) {
        acutPrintf(L"\nError: Could not open block definition.");
        pBlockRef->close();
        pModelSpace->close();
        pBlockTable->close();
        return AcDbObjectId::kNull;
    }

    // Create an iterator for the block definition entities
    AcDbBlockTableRecordIterator* pIter = nullptr;
    if (pBlockDef->newIterator(pIter) != Acad::eOk) {
        acutPrintf(L"\nError: Could not create iterator for block definition.");
        pBlockDef->close();
        pBlockRef->close();
        pModelSpace->close();
        pBlockTable->close();
        return AcDbObjectId::kNull;
    }

    // Loop through the block definition to find attribute definitions
    for (; !pIter->done(); pIter->step()) {
        AcDbEntity* pEnt = nullptr;
        if (pIter->getEntity(pEnt, AcDb::kForRead) != Acad::eOk || !pEnt)
            continue;

        AcDbAttributeDefinition* pAttDef = AcDbAttributeDefinition::cast(pEnt);
        if (pAttDef && !pAttDef->isConstant()) {
           // Create a new attribute based on the definition
            AcDbAttribute* pAtt = new AcDbAttribute();
            pAtt->setPropertiesFrom(pAttDef);                               // Copy general properties

            // Transform the attribute definition position into world space
            AcGePoint3d localPos = pAttDef->position();
            AcGePoint3d worldPos = pBlockRef->blockTransform() * localPos;
            pAtt->setPosition(worldPos);                                    // Set transformed position

            pAtt->setJustification(pAttDef->justification());               // Match justification

            AcGePoint3d localAlign = pAttDef->alignmentPoint();
            AcGePoint3d worldAlign = pBlockRef->blockTransform() * localAlign;
            pAtt->setAlignmentPoint(worldAlign);                            // This must happen after justification

            pAtt->setHeight(pAttDef->height());                             // Match text height
            pAtt->setRotation(pAttDef->rotation());                         // Match rotation
            pAtt->setTag(pAttDef->tag());                                   // Match tag
            pAtt->setFieldLength(pAttDef->fieldLength());                   // Match field length
            pAtt->setWidthFactor(pAttDef->widthFactor());                   // Match the width factor
            pAtt->setLockPositionInBlock(pAttDef->lockPositionInBlock());   // Match Lock Position

            pAtt->setTextString(pAttDef->textString());                     // Use default value

            // Append the attribute to the block reference
            pBlockRef->appendAttribute(pAtt);
            pAtt->close();
        }

        pEnt->close();
    }

    // Clean up
    delete pIter;
    pBlockDef->close();
    pBlockRef->close();
    pModelSpace->close();
    pBlockTable->close();

    return blockRefId;
}

Acad::ErrorStatus acadSetDynBlockProperty(
    const AcDbObjectId& blockRefId,
    const wchar_t* propName,
    const AcDbEvalVariant& newValue
) {
    // Open block reference for writing
    AcDbBlockReference* pBlkRef = nullptr;
    Acad::ErrorStatus es = acdbOpenObject(pBlkRef, blockRefId, AcDb::kForWrite);
    if (es != Acad::eOk || !pBlkRef) {
        acutPrintf(L"\nError: Could not open block reference.");
        return es;
    }

    // Get dynamic block properties
    AcDbDynBlockReference dynBlkRef(pBlkRef);
    AcDbDynBlockReferencePropertyArray propArray;
    dynBlkRef.getBlockProperties(propArray);

    // Search for matching property
    for (int i = 0; i < propArray.length(); ++i) {
        AcDbDynBlockReferenceProperty& prop = propArray[i];

        if (wcscmp(prop.propertyName(), propName) == 0) {
            es = prop.setValue(newValue);
            if (es == Acad::eOk) {
                pBlkRef->close();
                return Acad::eOk;
            } else {
                acutPrintf(L"\nError: Failed to set value for property '%ls'.", propName);
                pBlkRef->close();
                return es;
            }
        }
    }

    acutPrintf(L"\nWarning: Property '%ls' not found.", propName);
    pBlkRef->close();
    return Acad::eKeyNotFound;
}

Acad::ErrorStatus acadSetBlockAttribute(
    const AcDbObjectId& blockRefId,
    const wchar_t* tagName,
    const wchar_t* newValue
) {
    // Open block reference for writing
    AcDbBlockReference* pBlkRef = nullptr;
    Acad::ErrorStatus es = acdbOpenObject(pBlkRef, blockRefId, AcDb::kForWrite);
    if (es != Acad::eOk || !pBlkRef) {
        acutPrintf(L"\nError: Could not open block reference.");
        return es;
    }

    // Create iterator for attached attributes
    AcDbObjectIterator* pIter = pBlkRef->attributeIterator();
    if (!pIter) {
        acutPrintf(L"\nError: Failed to get attribute iterator.");
        pBlkRef->close();
        return Acad::eNullIterator;
    }

    // Search for matching attribute tag
    for (; !pIter->done(); pIter->step()) {
        AcDbObjectId attId = pIter->objectId();
        AcDbAttribute* pAtt = nullptr;

        if (acdbOpenObject(pAtt, attId, AcDb::kForWrite) == Acad::eOk && pAtt) {
            if (_wcsicmp(pAtt->tag(), tagName) == 0) {
                pAtt->setTextString(newValue);
                pAtt->adjustAlignment();
                pAtt->close();
                delete pIter;
                pBlkRef->close();
                return Acad::eOk;
            }
            pAtt->close();
        }
    }

    // Not found
    delete pIter;
    pBlkRef->close();
    acutPrintf(L"\nWarning: Attribute '%ls' not found.", tagName);
    return Acad::eKeyNotFound;
}

Acad::ErrorStatus acadSetObjectProperty(
    const AcDbObjectId& objId,
    AcDb::DxfCode groupCode,
    const wchar_t* value
) {
    AcDbEntity* pEnt = nullptr;
    Acad::ErrorStatus es = acdbOpenObject(pEnt, objId, AcDb::kForWrite);
    if (es != Acad::eOk || !pEnt) {
        acutPrintf(L"\nError: Unable to open entity for writing.");
        return es;
    }

    switch (groupCode) {
        case AcDb::kDxfLayerName:
            es = pEnt->setLayer(value);
            break;
        case AcDb::kDxfLinetypeName:
            es = pEnt->setLinetype(value);
            break;
        case AcDb::kDxfLinetypeScale:
            es = pEnt->setLinetypeScale(wcstod(value, nullptr));
            break;
        case AcDb::kDxfColor:
            es = pEnt->setColorIndex(_wtoi(value));
            break;
        default:
            acutPrintf(L"\nError: Unsupported DXF code %d", groupCode);
            es = Acad::eNotImplementedYet;
            break;
    }

    pEnt->close();
    return es;
}

