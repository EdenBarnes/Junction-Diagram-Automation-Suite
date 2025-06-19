/**
 * @file helpers.h
 * @brief Interface for helper functions related to database operations.
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

#pragma once

#include "dbents.h"
#include "dbapserv.h"
#include "dbdynblk.h"
#include "dbeval.h"
#include "acdb.h"
#include "rxregsvc.h"

/**
 * @brief Insert a block into the database at a specified origin point.
 *
 * This function inserts a block with the given name into the database at the
 * provided origin point and returns the object ID of the inserted block.
 *
 * @param blockName The name of the block to be inserted. Must be a valid 
 *                  string representing an existing block definition.
 * @param origin    The 3D point where the block should be placed in the database.
 *
 * @return The AcDbObjectId of the newly inserted block, or an invalid ID if
 *         insertion fails.
 */
AcDbObjectId acadInsertBlock(const wchar_t* blockName, const AcGePoint3d& origin);

/**
 * @brief Set a dynamic block property to a new value.
 *
 * This function updates the specified property of a dynamic block reference 
 * with a new value.
 *
 * @param blockRefId The object ID of the block reference whose property is being set.
 * @param propName   The name of the property to be updated. Must exist in the
 *                   dynamic block definition.
 * @param newValue   The new value for the specified property, encapsulated as an 
 *                   AcDbEvalVariant.
 *
 * @return Acad::ErrorStatus indicating success or failure of the operation.
 */
Acad::ErrorStatus acadSetDynBlockProperty(
    const AcDbObjectId& blockRefId,
    const wchar_t* propName,
    const AcDbEvalVariant& newValue
);

/**
 * @brief Retrieve the value of a dynamic block property.
 *
 * This function reads the current value of a specified property from a 
 * dynamic block reference.
 *
 * @param blockRefId The object ID of the block reference whose property is being read.
 * @param propName   The name of the property to retrieve. Must exist in the
 *                   dynamic block definition.
 * @param outValue   Output parameter that receives the current value of the property,
 *                   encapsulated as an AcDbEvalVariant.
 *
 * @return Acad::ErrorStatus indicating success or failure of the operation.
 *         Returns Acad::eKeyNotFound if the property does not exist.
 */
Acad::ErrorStatus acadGetDynBlockProperty(
    const AcDbObjectId& blockRefId,
    const wchar_t* propName,
    AcDbEvalVariant& outValue
);

/**
 * @brief Set a block attribute to a new value.
 *
 * This function updates the specified attribute of a block reference with a 
 * new value.
 *
 * @param blockRefId The object ID of the block reference whose attribute is being set.
 * @param tagName    The name of the tag associated with the attribute. Must exist in
 *                   the block definition.
 * @param newValue   A wide string pointer to the new value for the specified attribute.
 *
 * @return Acad::ErrorStatus indicating success or failure of the operation.
 */
Acad::ErrorStatus acadSetBlockAttribute(
    const AcDbObjectId& blockRefId,
    const wchar_t* tagName,
    const wchar_t* newValue
);

/**
 * @brief Set a general object property to a new value.
 *
 * This function updates the specified property of an object with a new value, 
 * identified by a DXF group code.
 *
 * @param objId      The object ID of the entity whose property is being set.
 * @param groupCode  The DXF group code representing the property type to be updated.
 *                   For example, AcDb::kDxfLayerName for the layer name.
 * @param value      A wide string pointer to the new value for the specified property.
 *
 * @return Acad::ErrorStatus indicating success or failure of the operation.
 */
Acad::ErrorStatus acadSetObjectProperty(
    const AcDbObjectId& objId,
    AcDb::DxfCode groupCode,
    const wchar_t* value
);

/**
 * @brief Set the position of a supported entity.
 *
 * This function updates the position of a supported object type, such as a 
 * block reference, point, text, or circle. Entities not supporting a direct
 * position cannot be modified using this function.
 *
 * @param objId    The object ID of the entity to be updated.
 * @param position The new position to assign to the entity.
 *
 * @return Acad::ErrorStatus indicating success or failure of the operation.
 *         Returns Acad::eInvalidInput if the entity does not support positioning.
 */
Acad::ErrorStatus acadSetObjectPosition(
    const AcDbObjectId& objId,
    const AcGePoint3d& position
);

/**
 * @brief Get the position of a supported entity.
 *
 * This function retrieves the position of a supported object type, such as a 
 * block reference, point, circle, or text. The result is stored in a 3D point.
 *
 * @param objId       The object ID of the entity.
 * @param outPosition The position of the entity, if supported.
 *
 * @return Acad::ErrorStatus indicating success or failure of the operation.
 *         Returns Acad::eInvalidInput if the entity does not have a position.
 */
Acad::ErrorStatus acadGetObjectPosition(
    const AcDbObjectId& objId,
    AcGePoint3d& outPosition
);

/**
 * @brief Get block name that an object references
 * 
 * This function takes an objectId, checks to see if its a reference to a block, and
 * returns the name of that block reference.
 * 
 * @param objId     The object ID of the entity whose block name we want.ABC
 * @param name      A wide string reference where the name will be stored.
 *                  Empty if the object is not a block reference.
 * 
 * @return Acad::ErrorState indicating success or failure of the operation.
 */
Acad::ErrorStatus acadGetBlockName(
    const AcDbObjectId& objId,
    std::wstring &name
);