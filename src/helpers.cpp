/**
 * @file helpers.cpp
 * @brief Helper functions related to database operations.
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
        acutPrintf(L"\nError: Could not open block reference for writing.");
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

Acad::ErrorStatus acadGetDynBlockProperty(
    const AcDbObjectId& blockRefId,
    const wchar_t* propName,
    AcDbEvalVariant& outValue
) {
    // Open block reference for reading
    AcDbBlockReference* pBlkRef = nullptr;
    Acad::ErrorStatus es = acdbOpenObject(pBlkRef, blockRefId, AcDb::kForRead);
    if (es != Acad::eOk || !pBlkRef) {
        acutPrintf(L"\nError: Could not open block reference for reading.");
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
            outValue = prop.value();
            pBlkRef->close();

            if (es == Acad::eOk) {
                return Acad::eOk;
            } else {
                acutPrintf(L"\nError: Failed to read value for property '%ls'.", propName);
                return es;
            }
        }
    }

    // Property not found
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

Acad::ErrorStatus acadGetBlockAttribute(
    const AcDbObjectId& blockRefId,
    const wchar_t*      tagName,
    std::wstring&       outValue
) {
    // Open block reference for reading
    AcDbBlockReference* pBlkRef = nullptr;
    Acad::ErrorStatus es = acdbOpenObject(pBlkRef, blockRefId, AcDb::kForRead);
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

        if (acdbOpenObject(pAtt, attId, AcDb::kForRead) == Acad::eOk && pAtt) {
            if (_wcsicmp(pAtt->tag(), tagName) == 0) {
                // Found – copy value to outValue
                outValue = pAtt->textString();
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

Acad::ErrorStatus acadSetObjectPosition(
    const AcDbObjectId& objId,
    const AcGePoint3d& position
) {
    AcDbEntity* pEnt = nullptr;
    Acad::ErrorStatus es = acdbOpenObject(pEnt, objId, AcDb::kForWrite);
    if (es != Acad::eOk || !pEnt) {
        acutPrintf(L"\nError: Could not open object for writing.");
        return es;
    }

    if (pEnt->isKindOf(AcDbBlockReference::desc())) {
        AcDbBlockReference* pBlkRef = AcDbBlockReference::cast(pEnt);

        // Compute translation vector
        const AcGeVector3d offset = position - pBlkRef->position();

        // Build translation matrix and apply
        const AcGeMatrix3d xform = AcGeMatrix3d::translation(offset);
        pBlkRef->transformBy(xform);

        // Optional but recommended: keep attributes in sync with their
        // definitions after any block modification.
        pBlkRef->recordGraphicsModified(true); // marks graphics dirty
    }
    else if (pEnt->isKindOf(AcDbPoint::desc())) {
        AcDbPoint* pPoint = AcDbPoint::cast(pEnt);
        pPoint->setPosition(position);
    }
    else if (pEnt->isKindOf(AcDbText::desc())) {
        AcDbText* pText = AcDbText::cast(pEnt);
        pText->setPosition(position);
    }
    else if (pEnt->isKindOf(AcDbMText::desc())) {
        AcDbMText* pMText = AcDbMText::cast(pEnt);
        pMText->setLocation(position);
    }
    else if (pEnt->isKindOf(AcDbCircle::desc())) {
        AcDbCircle* pCircle = AcDbCircle::cast(pEnt);
        pCircle->setCenter(position);
    }
    else {
        acutPrintf(L"\nError: Unsupported entity type for setting position.");
        pEnt->close();
        return Acad::eInvalidInput;
    }

    pEnt->close();
    return Acad::eOk;
}

Acad::ErrorStatus acadGetObjectPosition(
    const AcDbObjectId& objId,
    AcGePoint3d& outPosition
) {
    AcDbEntity* pEnt = nullptr;
    Acad::ErrorStatus es = acdbOpenObject(pEnt, objId, AcDb::kForRead);
    if (es != Acad::eOk || !pEnt) {
        acutPrintf(L"\nError: Could not open object for reading.");
        return es;
    }

    if (pEnt->isKindOf(AcDbBlockReference::desc())) {
        AcDbBlockReference* pBlockRef = AcDbBlockReference::cast(pEnt);
        outPosition = pBlockRef->position();
    }
    else if (pEnt->isKindOf(AcDbPoint::desc())) {
        AcDbPoint* pPoint = AcDbPoint::cast(pEnt);
        outPosition = pPoint->position();
    }
    else if (pEnt->isKindOf(AcDbCircle::desc())) {
        AcDbCircle* pCircle = AcDbCircle::cast(pEnt);
        outPosition = pCircle->center();
    }
    else if (pEnt->isKindOf(AcDbText::desc())) {
        AcDbText* pText = AcDbText::cast(pEnt);
        outPosition = pText->position();
    }
    else if (pEnt->isKindOf(AcDbMText::desc())) {
        AcDbMText* pMText = AcDbMText::cast(pEnt);
        outPosition = pMText->location();
    }
    else {
        acutPrintf(L"\nError: Unsupported entity type for position extraction.");
        pEnt->close();
        return Acad::eInvalidInput;
    }

    pEnt->close();
    return Acad::eOk;
}

Acad::ErrorStatus acadSetObjectScale(
    const AcDbObjectId& objId,
    const AcGeScale3d& scale
) {
    AcDbEntity* pEnt = nullptr;
    Acad::ErrorStatus es = acdbOpenObject(pEnt, objId, AcDb::kForWrite);
    if (es != Acad::eOk || !pEnt) {
        acutPrintf(L"\nError: Could not open object for writing.");
        return es;
    }

    if (pEnt->isKindOf(AcDbBlockReference::desc())) {
        AcDbBlockReference* pBlkRef = AcDbBlockReference::cast(pEnt);
        pBlkRef->setScaleFactors(scale);
    }
    else {
        acutPrintf(L"\nError: Unsupported entity type for setting scale.");
        pEnt->close();
        return Acad::eInvalidInput;
    }

    pEnt->close();
    return Acad::eOk;
}

Acad::ErrorStatus acadGetObjectScale(
    const AcDbObjectId& objId,
    AcGeScale3d& outScale
) {
    AcDbEntity* pEnt = nullptr;
    Acad::ErrorStatus es = acdbOpenObject(pEnt, objId, AcDb::kForRead);
    if (es != Acad::eOk || !pEnt) {
        acutPrintf(L"\nError: Could not open object for reading.");
        return es;
    }

    if (pEnt->isKindOf(AcDbBlockReference::desc())) {
        AcDbBlockReference* pBlkRef = AcDbBlockReference::cast(pEnt);
        outScale = pBlkRef->scaleFactors();
    }
    else {
        acutPrintf(L"\nError: Unsupported entity type for reading scale.");
        pEnt->close();
        return Acad::eInvalidInput;
    }

    pEnt->close();
    return Acad::eOk;
}

Acad::ErrorStatus acadGetBlockName(
    const AcDbObjectId& objId,
    std::wstring &name
) {
    AcDbEntity *pEnt = nullptr;
    Acad::ErrorStatus es = acdbOpenObject(pEnt, objId, AcDb::kForRead);
    if (es != Acad::eOk || !pEnt) {
        acutPrintf(L"\nError: Unable to open entity for reading.");
        return es;
    }

    if (!pEnt->isKindOf(AcDbBlockReference::desc())) {
        name = L"";
        pEnt->close();
        return Acad::eOk;
    }

    AcDbBlockReference *pBlockRef = AcDbBlockReference::cast(pEnt);

    AcDbObjectId blockDefId = pBlockRef->blockTableRecord();

    AcDbBlockTableRecord *pBlockDef = nullptr;
    es = acdbOpenObject(pBlockDef, blockDefId, AcDb::kForRead);
    if (es != Acad::eOk || !pBlockDef) {
        acutPrintf(L"\nError: Unable to open block definition for reading.");
        pEnt->close();
        return es;
    }

    const ACHAR* blockName = nullptr;
    pBlockDef->getName(blockName);

    // Check if this is an anonymous block (name starts with *)
    if (blockName[0] != '*') {
        pBlockDef->close();
        pEnt->close();

        name = blockName;

        return Acad::eOk;
    }

    AcDbDynBlockReference dynBlkDefRef(objId);
    AcDbObjectId dynBlkDefId = dynBlkDefRef.dynamicBlockTableRecord();

    AcDbBlockTableRecord *pDynBlockDef = nullptr;
    es = acdbOpenObject(pDynBlockDef, dynBlkDefId, AcDb::kForRead);

    if (es != Acad::eOk || !pDynBlockDef) {
        acutPrintf(L"\nError: Unable to open dynamic block reference for reading.");
        return es;
    }

    const ACHAR* dynName = nullptr;
    pDynBlockDef->getName(dynName);

    pDynBlockDef->close();
    pBlockDef->close();
    pEnt->close();

    name = dynName;

    return Acad::eOk;
}