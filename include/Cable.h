/**
 * @file Cable.h
 * @brief Interface for the Cable class.
 *
 * This module is part of the Junction Diagram Automation Suite. Unauthorized 
 * copying, distribution, or modification is prohibited.
 * 
 * @version 1.1.0
 * @author Ethan Barnes <ebarnes@gastecheng.com>
 * @date 2025-06-16
 * @copyright Proprietary - All Rights Reserved by GasTech Engineering LLC
 *
 */

#pragma once

#include <iostream>
#include <vector>

#include "actrans.h"

#include "Device.h"
#include "helpers.h"

/**
 * @enum CableType
 * @brief Enumerates the different types of cables.
 */
enum CableType {
    PAIR1, ///< A cable with 1 pair configuration.
    PAIR2, ///< A cable with 2 pairs configuration.
    PAIR4, ///< A cable with 4 pairs configuration.
    TRIAD1,///< A cable with a triad configuration.
    WIRE7  ///< A seven-wire cable configuration.
};

/**
 * @enum SystemType
 * @brief Enumerates the types of systems that can use cables.
 */
enum SystemType {
    CONTROL,///< Cables used in control systems.
    SAFETY  ///< Cables used in safety-critical systems.
};

/**
 * @enum IOType
 * @brief Enumerates the input/output types supported by cables.
 */
enum IOType {
    ANALOG, ///< Analog signals (e.g., varying voltage).
    DIGITAL ///< Digital signals (e.g., on/off).
};

/**
 * @class Cable
 * @brief Represents a cable with specific attributes and devices connected to it.
 *
 * The `Cable` class encapsulates information about a cable including its type,
 * system usage, input/output characteristics, and the devices it connects to.
 */
class Cable
{
private:
    CableType _cableType; ///< Type of the cable (e.g., PAIR1, WIRE7).
    SystemType _sysType;  ///< System type the cable is used in (SAFETY or CONTROL).
    IOType _ioType;       ///< Input/output type the cable supports (DIGITAL or ANALOG).
    std::vector<Device> _devices; ///< Devices connected to this cable.

    /**
     * @brief Get the visual state of the cable.
     * 
     * @return The visual state as a wide string.
     */
    std::wstring _getVisState() const;
public:
    /**
     * @brief Construct a Cable object with specified types.
     * 
     * @param cableType Type of the cable.
     * @param sysType System type (e.g., SAFETY or CONTROL).
     * @param ioType Input/output type (e.g., DIGITAL or ANALOG).
     */
    Cable(CableType cableType, SystemType sysType, IOType ioType);

    /**
     * @brief Draw the cable starting from a given origin.
     * 
     * @param origin The starting point for drawing.
     * @param terminalNumber The number of the first terminal the cable connects to (from top to bottom).
     * @param flip Direction of the cable. true if the cable should be drawn to the right instead of to the left, false otherwise.
     * @param junctionTag Tag of the junction box this cable is attached to. Used for creating field tags.
     * @param tableNumber Number indicating which table this cable is attached to (e.g., 1 for TB1).
     */
    void draw(AcGePoint3d origin, int terminalNumber, bool flip, const wchar_t *junctionTag, int tableNumber) const;

    /* ----- Setters ----- */

    /**
     * @brief Add a device to the cable's list of connected devices.
     * 
     * @param device Device object to be added.
     */
    void addDevice(Device device);

    /* ----- Getters ----- */

    /**
     * @brief Get the type of this cable.
     * 
     * @return CableType representing the type of this cable.
     */
    CableType getCableType() const;

    /**
     * @brief Get the system type associated with this cable.
     * 
     * @return SystemType indicating whether it's a SAFETY or CONTROL system.
     */
    SystemType getSystemType() const;

    /**
     * @brief Get the input/output type of this cable.
     * 
     * @return IOType representing either DIGITAL or ANALOG.
     */
    IOType getIOType() const;

    /**
     * @brief Get all devices connected to this cable.
     * 
     * @return A vector containing all Device objects connected to this cable.
     */
    std::vector<Device> getDevices() const;

    /**
     * @brief Calculate and return the terminal footprint count.
     * 
     * @return The number of terminals on the cable must connect to.
     */
    int getTerminalFootprint() const;
    
    /* ----- Helpers ----- */

    /**
     * @brief Determine the CableType from a given cell string identifier.
     * 
     * @param cell A string representing the cell information.
     * @return The corresponding CableType.
     */
    static CableType getWireTypeFromCell(const std::string& cell);

    /**
     * @brief Determine the SystemType from a given cell string identifier.
     * 
     * @param cell A string representing the cell information.
     * @return The corresponding SystemType.
     */
    static SystemType getSystemTypeFromCell(const std::string& cell);

    /**
     * @brief Determine the IOType from a given cell string identifier.
     * 
     * @param cell A string representing the cell information.
     * @return The corresponding IOType.
     */
    static IOType getIOTypeFromCell(const std::string& cell);

    /* ----- Operators ----- */

    /**
     * @brief Access a device by index from the list of connected devices.
     * 
     * @param index Index of the device to access.
     * @return The Device object at the specified index.
     */
    Device operator[](int index) const;

    /**
     * @brief Compare this cable with another based on the following criteria:
     * 1. All CONTROL cables are less than all SAFETY cables.
     * 2. All ANALOG cables are less than all DIGITAL cables.
     * 3. Cables with lesser devices are lesser cables.
     * 
     * @param rhs Right-hand side Cable object for comparison.
     * @return True if this cable is considered less than rhs, otherwise false.
     */
    bool operator<(const Cable& rhs) const;
};




