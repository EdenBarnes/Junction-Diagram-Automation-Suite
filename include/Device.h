/**
 * @file Device.h
 * @brief Interface for the Device class.
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

#pragma once

#include <string>

#include "actrans.h"

#include "helpers.h"

/**
 * @class Device
 * @brief Represents a device within the Junction Diagram Automation Suite.
 *
 * The `Device` class encapsulates information about a device, including its 
 * tag, number, and footprint characteristics. It provides methods to retrieve 
 * these details as well as utility functions for processing device data.
 */
class Device
{
private:
    std::string _tag; ///< Tag identifying the device type (e.g., "SDV" for Shutdown Valve).
    std::string _number; ///< Specific number or identifier of the device.
    int _footprint; ///< Footprint count representing terminal connections.

public:
    /**
     * @brief Construct a Device object with combined tag and footprint.
     *
     * @param combinedTag A string that combines both tag and number (e.g., "R123").
     * @param footprint The footprint value representing the number of terminals.
     */
    Device(std::string combinedTag, int footprint);

    /**
     * @brief Draw the device starting from a given origin.
     * 
     * @param origin The starting point for drawing.
     * @param flip Direction of the device. true if the cable should be drawn to the right instead of to the left, false otherwise.
     */
    void draw(AcGePoint3d origin, bool flip) const;

    /* ----- Setters ----- */

    /* ----- Getters ----- */

    /**
     * @brief Retrieve the device's tag.
     *
     * @return A string representing the device's tag (e.g., "SDV" for Shutdown Valve).
     */
    std::string getTag() const;

    /**
     * @brief Retrieve the device's number or identifier.
     *
     * @return A string representing the specific number of the device.
     */
    std::string getNumber() const;

    /**
     * @brief Retrieve the combined tag and number for the device.
     *
     * @return A string that combines both tag and number (e.g., "SDV 60A").
     */
    std::string getCombinedTag() const;

    /**
     * @brief Get the terminal footprint of the device.
     *
     * @return The integer value representing the number of terminals.
     */
    int getTerminalFootprint() const;

    /* ----- Helpers ----- */

    /**
     * @brief Calculate and return the footprint from a combined tag and instrument specification.
     *
     * @param combinedTag A string that combines both tag and number (e.g., "LSLL 100A").
     * @param instrumentSpec Additional specification of the instrument (e.g., "ULTRASONICE SW").
     * @return The calculated footprint as an integer representing terminal connections.
     */
    static int footprintFromCells(const std::string& combinedTag, const std::string& instrumentSpec);

    /* ----- Operators ----- */

    /**
     * @brief Compare this device with another based on their combined tags.
     *
     * @param rhs Right-hand side Device object for comparison.
     * @return True if this device is considered less than rhs, otherwise false.
     */
    bool operator<(const Device& rhs) const;
};