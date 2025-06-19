/**
 * @file JunctionBuilder.cpp
 * @brief Definitons for building junction boxes.
 *
 * This module is part of the Junction Diagram Automation Suite. Unauthorized 
 * copying, distribution, or modification is prohibited.
 * 
 * @version 1.1.2
 * @author Ethan Barnes <ebarnes@gastecheng.com>
 * @date 2025-06-18
 * @copyright Proprietary - All Rights Reserved by GasTech Engineering LLC
 *
 */

#include "JunctionBuilder.h"

// -----------------------------------------------------------------------------
// Internal Types
// -----------------------------------------------------------------------------

/**
 * @enum BoxSize
 * @brief Predefined enclosure footprints supported by the tool.
 */
enum BoxSize {
    SMALL,  ///< 12" × 12" × 6" enclosure
    MEDIUM, ///< 16" × 16" × 6" enclosure
    LARGE,  ///< 24" × 24" × 8" enclosure
    CUSTOM  ///< Custom enclosure. User will place the cables
};

/**
 * @struct DialogResult
 * @brief Aggregates the data collected from the user via the dialog box.
 */
struct DialogResult {
    std::string filename;    ///< Absolute path to the workbook selected by the user.
    std::string selectedTag; ///< Junction tag chosen by the user.
    BoxSize selectedSize;    ///< Box size that the user confirmed they have open.
    bool accepted = false;   ///< Set to true if the user pressed **OK**.
};

// -----------------------------------------------------------------------------
// Forward Declarations
// -----------------------------------------------------------------------------

/**
 * @brief Draw a junction box from a file
 *
 * @param filename      Absolute path to the Excel (.xlsx) file.
 * @param selectedTag   Tag (e.g. "IJB-810") identifying the junction whose
 *                      cables should be extracted.
 * @param selectedSize  Size of the box to be drawn.
 * @param origin        Point where the box should be drawn. (Usually 0 0 0)
 */
void _drawJunctionBox(std::string filename, std::string selectedTag, BoxSize selectedSize, AcGePoint3d origin);

/**
 * @brief Given a cable list and a current position in that list,
 *        should the next cable be drawn on the next table.
 *
 * @param boxSize               Size of the box.
 * @param cables                Reference to a vector of `Cable` objects.
 * @param currentCableIndex     The index of the cable that is about to be added to the drawing.
 * @param currentTerminalIndex  The terminal that the next cable will reside on.
 * @param currentTableIndex     The current table being drawn to.
 * @return                      `true` if the cable being drawn should be placed on the next table, `false` otherwise.
 */
bool _shouldSplit(BoxSize boxSize, std::vector<Cable> &cables, int currentCableIndex, int currentTerminalIndex, int currentTableIndex);

/**
 * @brief Parse the provided Cable Schedule workbook and create a list of
 *        `Cable` objects for the specified junction tag.
 *
 * @param hDlg        Parent‑window handle used for any error message boxes.
 * @param filename    Absolute path to the Excel (.xlsx) file.
 * @param junctionTag Tag (e.g. "IJB-810") identifying the junction whose
 *                    cables should be extracted.
 * @return            Vector of fully‑populated `Cable` objects. If the file
 *                    fails to open or is incompatible, an empty vector is
 *                    returned.
 */
std::vector<Cable> _xlsxGetCables(HWND hDlg,
                                  const std::string& filename,
                                  const std::string& junctionTag);

/**
 * @brief Collect all unique junction tags found in the Cable Schedule sheet of
 *        the workbook.
 *
 * @param hDlg     Parent‑window handle for error dialogs.
 * @param filename Path to the Excel workbook.
 * @param tags     Reference that will be filled with unique tags (output).
 */
void _xlsxGetJunctionTags(HWND hDlg,
                           const std::string& filename,
                           std::vector<std::string>& tags);

/**
 * @brief Calculate the total number of terminals required by all cables that
 *        terminate in the given junction tag.
 *
 * @param hDlg        Parent‑window handle for error dialogs.
 * @param filename    Path to the Excel workbook.
 * @param junctionTag Junction tag whose footprint is required.
 * @param boxSize     Size of the box to calculate the footprint on. (Required to account for table splitting)
 * @return            Terminal count ("footprint").
 */
int _xlsxGetJunctionFootprint(HWND hDlg,
                              std::string filename,
                              std::string junctionTag,
                              BoxSize boxSize);

/**
 * brief Update the size‑selection radio buttons to show how many spare
 *        terminals each box size would have after the current selection.
 *
 * @param hDlg        Dialog‑box window handle.
 * @param sizeButtons Handles of size radio buttons in dialog.
 * @param spareCounts Spare‑terminal counts (same order as `sizeButtons`).
 */
void _updateSizeRadioButtons(HWND hDlg,
                             const std::vector<HWND>& sizeButtons,
                             const std::vector<int>& spareCounts);

/**
 * @brief Rebuild (or build) the dialog‑box controls for junction‑tag and box‑
 *        size selection.
 *
 * The function is called after selecting a new XLSX file and whenever the
 * spare‑terminal counts must be refreshed.
 *
 * @param junctionTags   List of available junction tags.
 * @param tagRadioButtons Vector that will receive handles to dynamically
 *                        created tag buttons (output).
 * @param sizeRadioButtons Vector that will receive handles to size buttons
 *                         (output).
 * @param spareCounts    Spare‑terminal counts for each predefined box size.
 * @param hDlg           Dialog window handle.
 */
void _rebuildDialogBox(const std::vector<std::string>& junctionTags,
                       std::vector<HWND>& tagRadioButtons,
                       std::vector<HWND>& sizeRadioButtons,
                       const std::vector<int>& spareCounts,
                       HWND hDlg);

/**
 * @brief Dialog‑box procedure that handles all messages for the Junction Box
 *        Builder dialog.
 *
 * @param hDlg    Dialog window handle.
 * @param message Windows message identifier.
 * @param wParam  Additional message information.
 * @param lParam  Additional message information (custom for WM_INITDIALOG).
 *
 * @return TRUE if the message was processed, FALSE otherwise (per Windows API
 *         requirements for dialog procedures).
 */
INT_PTR CALLBACK _DialogProc(HWND hDlg,
                             UINT message,
                             WPARAM wParam,
                             LPARAM lParam);


// -----------------------------------------------------------------------------
// Function Definitions
// -----------------------------------------------------------------------------

void buildJunctionBox() {
    /*
        Open a dialog box to get information from the user

        This dialog box should ask the user for an .xlsx file. It should then read that file and
        get a list of junction box tags that are defined in that .xlsx file. That list should be
        presented to the user, and the user will select the one junction box they want built.
        The dialog box will then inform the user of how many spare terminals will exist for each
        junction box size, and ask them which junction box size template they currently have open.

        From this dialog box we will recieve the following information:
            1. The filename and location of the .xlsx file to read from
            2. The size of junction box to be built
    */
   
    DialogResult result;

    HMODULE hModule = nullptr;
    GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
                    reinterpret_cast<LPCTSTR>(_DialogProc),
                    &hModule);
    
    DialogBoxParam(hModule, MAKEINTRESOURCE(IDD_DIALOG),
                    adsw_acadMainWnd(), _DialogProc, reinterpret_cast<LPARAM>(&result));

    if (!result.accepted) {
        acutPrintf(L"\nCanceled.");
        return;
    }

    if (result.selectedTag == "Select All") {
        // Draw every single box

        std::vector<std::string> junctionTags;

        _xlsxGetJunctionTags(adsw_acadMainWnd(), result.filename, junctionTags);

        for (int i = 0; i < junctionTags.size(); ++i) {
            std::string tag = junctionTags[i];

            _drawJunctionBox(result.filename, tag, result.selectedSize, AcGePoint3d(-11.0 * i, 0.0, 0.0));
        }
    } else {
        _drawJunctionBox(result.filename, result.selectedTag, result.selectedSize, AcGePoint3d(0.0, 0.0, 0.0));
    }
}

void flipCable() {
    ads_name ss;

    int result = acedSSGet(L"I", nullptr, nullptr, nullptr, ss);

    if (result != RTNORM) {
        // No implied selection — ask user to select objects manually
        acutPrintf(L"\nPlease select objects:");
        result = acedSSGet(nullptr, nullptr, nullptr, nullptr, ss);

        if (result != RTNORM) {
            acutPrintf(L"\nCanceled.");
            return;
        }
    }

    int length = 0;
    acedSSLength(ss, &length);
    
    for (int i = 0; i < length; ++i) {
        ads_name ent;
        acedSSName(ss, i, ent);

        AcDbObjectId objId;
        acdbGetObjectId(objId, ent);

        std::wstring blockName;
        Acad::ErrorStatus es = acadGetBlockName(objId, blockName);
        if (es != Acad::eOk) {
            acutPrintf(L"\nError: Unable to get object block name.");
            return;
        }

        AcGePoint3d position;
        acadGetObjectPosition(objId, position);
        
        if (blockName == L"Junction Termination" || blockName == L"Junction Termination (7 Wire)") {
            AcDbEvalVariant flipVariant;
            acadGetDynBlockProperty(objId, L"Flip state1", flipVariant);

            int flipValue;
            flipVariant.getValue(flipValue);

            acadSetDynBlockProperty(objId, L"Flip state1", AcDbEvalVariant((short)(flipValue == 0 ? 1 : 0)));
        } else if (blockName == L"Field Device Termination" || blockName == L"Field Device Termination (7 Wire)") {
            AcDbEvalVariant flipVariant;
            acadGetDynBlockProperty(objId, L"Flip state1", flipVariant);

            int flipValue;
            flipVariant.getValue(flipValue);

            if (flipValue == 1) {
                // Pointing right, move left
                position.x -= 18.0;
            } else {
                position.x += 18.0;
            }

            acadSetObjectPosition(objId, position);
            acadSetDynBlockProperty(objId, L"Flip state1", AcDbEvalVariant((short)(flipValue == 0 ? 1 : 0)));
        } else if (blockName == L"TBWIREMINI") {
            AcGeScale3d scaleValue;
            acadGetObjectScale(objId, scaleValue);

            if (scaleValue[0] == -1.0) {
                position.x -= 18.6876;
            } else {
                position.x += 18.6876;
            }

            acadSetObjectPosition(objId, position);
            acadSetObjectScale(objId, AcGeScale3d(-scaleValue[0], 1.0, 1.0));
        } else if (blockName == L"INST SYMBOL") {
            AcDbEvalVariant flipVariant;
            acadGetDynBlockProperty(objId, L"Flip state", flipVariant);

            int flipValue;
            flipVariant.getValue(flipValue);

            if (flipValue == 1) {
                // Pointing right, move left
                position.x -= 19.8751;
            } else {
                position.x += 19.8751;
            }

            acadSetObjectPosition(objId, position);
            acadSetDynBlockProperty(objId, L"Flip state", AcDbEvalVariant((short)(flipValue == 0 ? 1 : 0)));
        }
    }

    acedSSFree(ss);
}

void reIndexCable() {
    ads_name ss;

    int result = acedSSGet(L"I", nullptr, nullptr, nullptr, ss);

    if (result != RTNORM) {
        // No implied selection — ask user to select objects manually
        acutPrintf(L"\nPlease select objects:");
        result = acedSSGet(nullptr, nullptr, nullptr, nullptr, ss);

        if (result != RTNORM) {
            acutPrintf(L"\nCanceled.");
            return;
        }
    }

    int startingTerminal = 0;
    acedGetInt(L"What terminal number do you want to start from?", startingTerminal);

    int length = 0;
    acedSSLength(ss, &length);

    // Determine which junction box is the highest so we can index from there
    double highest = std::numeric_limits<double>::lowest();
    for (int i = 0; i < length; ++i) {
        ads_name ent;
        acedSSName(ss, i, ent);

        AcDbObjectId objId;
        acdbGetObjectId(objId, ent);

        std::wstring blockName;
        if (acadGetBlockName(objId, blockName) != Acad::eOk)
            continue; // skip if we can't resolve name

        if (blockName != L"Junction Termination" &&
            blockName != L"Junction Termination (7 Wire)")
            continue; // not a termination

        AcGePoint3d position;
        if (acadGetObjectPosition(objId, position) != Acad::eOk)
            continue; // skip unsupported entities

        if (position.y > highest)
            highest = position.y;
    }

    // Re-index
    for (int i = 0; i < length; ++i) { 
        ads_name ent;
        acedSSName(ss, i, ent);

        AcDbObjectId objId;
        acdbGetObjectId(objId, ent);

        std::wstring blockName;
        if (acadGetBlockName(objId, blockName) != Acad::eOk)
            continue; // skip if we can't resolve name

        AcGePoint3d position;
        if (acadGetObjectPosition(objId, position) != Acad::eOk)
            continue; // skip unsupported entities

        double heightDif = highest - position.y;
        int terminalDif = (int)(heightDif / 0.25);

        if (blockName == L"Junction Termination" || blockName == L"Field Device Termination") {
            for (int j = 1; j <= 9; ++j) {
                wchar_t tagName[32];
                swprintf(tagName, L"FLDTAG%d", j);

                std::wstring fldtag;
                acadGetBlockAttribute(objId, tagName, fldtag);

                int currentTerminal = terminalDif + j + startingTerminal - 1;
                
                if (j > 5) currentTerminal ++;
                if (j > 7) currentTerminal ++;

                wchar_t termText[32];
                swprintf(termText, L"(%d)", currentTerminal);

                fldtag.replace(fldtag.find('('), std::wstring::npos, termText);

                acadSetBlockAttribute(objId, tagName, fldtag.c_str());
            }
        } else if (blockName == L"Junction Termination (7 Wire)" || blockName == L"Field Device Termination (7 Wire)") {
            for (int j = 1; j <= 7; ++j) {
                wchar_t tagName[32];
                swprintf(tagName, L"FLDTAG%d", j);

                std::wstring fldtag;
                acadGetBlockAttribute(objId, tagName, fldtag);

                int currentTerminal = terminalDif + j + startingTerminal - 1;
                
                if (j > 2) currentTerminal ++;
                if (j > 4) currentTerminal ++;
                if (j > 6) currentTerminal ++;

                wchar_t termText[32];
                swprintf(termText, L"(%d)", currentTerminal);

                fldtag.replace(fldtag.find('('), std::wstring::npos, termText);

                acadSetBlockAttribute(objId, tagName, fldtag.c_str());
            }
        }
    }
}

// -----------------------------------------------------------------------------
// Helper Function Definitions
// -----------------------------------------------------------------------------

void _drawJunctionBox(std::string filename, std::string selectedTag, BoxSize selectedSize, AcGePoint3d origin) {
    // Go through the .xlsx and build a cable object for every cable listed in the file.
    std::vector<Cable> cables = _xlsxGetCables(adsw_acadMainWnd(), filename, selectedTag);

    /*
        Sort the cables as follows in decending priority:
            1. Control cables before safety cables
            2. Analog cables before digital cables
            3. Alphabetically by first device tag
    */

    std::sort(cables.begin(), cables.end());

    /*
        Place the cables
        
        Each cable must be placed on a terminal in the diagram. If two terminal blocks exist
        (as in the 24x24x8 box) then the control and safety cables should reside on seperate
        blocks. If this is not possible, present a warning to the user and place the extra
        cables off to the side
    */

    std::wstring junctionTag(selectedTag.begin(), selectedTag.end());

    switch (selectedSize)
    {
    case BoxSize::LARGE :
        origin.set(11.1875, 18.3250, 0.0);
        break;

    case BoxSize::MEDIUM :
        origin.set(19.3750, 14.7500, 0.0);
        break;

    case BoxSize::SMALL :
        origin.set(19.3750, 12.4977, 0.0);
        break;
    
    default:
        break;
    }

    int terminal = 1;
    int table = 1;
    for (int i = 0; i < cables.size(); i++) {

        if (_shouldSplit(selectedSize, cables, i, terminal, table)) {
            terminal = 1;
            table ++;
        }

        bool flip = false;
        AcGePoint3d drawPoint = origin + AcGeVector3d(0.0, -0.25, 0.0) * (terminal - 1);
        if (selectedSize == BoxSize::LARGE && table == 2) {
            flip = true;
            drawPoint += AcGeVector3d(10.625, 0.0, 0.0);
        }

        cables[i].draw(drawPoint, terminal, flip, junctionTag.c_str(), table);

        terminal += cables[i].getTerminalFootprint();
    }

    /*
        Customer side cables are out of the scope of this tool. If customer side cables are
        to be placed on the diagram, they should be done manually or with a seperate tool.
    */
}

bool _shouldSplit(BoxSize boxSize, std::vector<Cable> &cables, int currentCableIndex, int currentTerminalIndex, int currentTableIndex) {
    if (boxSize == BoxSize::LARGE) {
        // If the rest of the cables take more space than in table 2, dont split
        int sizeOfRest = 0;
        for (int j = currentCableIndex; j < cables.size(); j++) {
            sizeOfRest += cables[j].getTerminalFootprint();
        }

        if (sizeOfRest <= 72 && currentCableIndex != 0 && currentTableIndex == 1) {

            // If the current cable is a safety cable, but the previous cable is control, split
            if (cables[currentCableIndex].getSystemType() == SystemType::SAFETY && cables[currentCableIndex - 1].getSystemType() == SystemType::CONTROL) {
                return true;
            }

            // If we've reached the end of the table, split
            if (currentTerminalIndex + cables[currentCableIndex].getTerminalFootprint() - 1 > 72) {
                return true;
            }       
        }

    }


    return false;
}

std::vector<Cable> _xlsxGetCables(HWND hDlg, const std::string& filename, const std::string& junctionTag) {
    std::vector<Cable> cables;

    OpenXLSX::XLDocument doc;
    try {
        doc.open(filename);
    } catch (const std::exception& e) {
        doc.close();
        return cables;
    }

    OpenXLSX::XLWorksheet cableWks;
    OpenXLSX::XLWorksheet ioWks;
    try {
        cableWks = doc.workbook().worksheet("Cable Schedule Data");
        ioWks = doc.workbook().worksheet("IO List");


        for (int row = 3; cableWks.cell(row, 4).value() != ""; ++row) {
            if (cableWks.cell(row, 3).value() != junctionTag) continue;

            std::string combinedTag = cableWks.cell(row, 4).value();
            std::string instrumentSpec;
            
            CableType wireType;
            SystemType systemType;
            IOType ioType;

            // Go find the respective info in IO List
            bool found = false;
            for (int row = 7; ioWks.cell(row, 2).value() != ""; ++row) {
                if (ioWks.cell(row, 2).value() == combinedTag) {
                    systemType = Cable::getSystemTypeFromCell(ioWks.cell(row, 8).value());
                    ioType = Cable::getIOTypeFromCell(ioWks.cell(row, 7).value());
                    instrumentSpec = ioWks.cell(row, 5).value().getString();
                    found = true;
                    break;
                }
            }
            if (!found) {
                throw std::runtime_error("Device in Cable Schedule Data does not exist in IO List");
            }
            
            if (cableWks.cell(row, 1).value().typeAsString() == "string") {
                // We are on a new cable
                std::string qty = cableWks.cell(row, 1).value();

                wireType = Cable::getWireTypeFromCell(qty);

                cables.push_back(Cable(
                    wireType,
                    systemType,
                    ioType
                ));
            }

            // Add the current device to the cable
            int deviceFootprint = Device::footprintFromCells(combinedTag, instrumentSpec);
            Device device(combinedTag, deviceFootprint);

            if (!cables.empty()) {
                cables.back().addDevice(device);
            }
        }

    } catch (const std::exception& e) {
        std::string message = "Excel file is not compatible: ";
        message += e.what();
        MessageBox(hDlg, message.c_str(), "Error", MB_OK | MB_ICONERROR);
        doc.close();
        cables.clear();
        return cables;
    }

    doc.close();

    return cables;
}

void _xlsxGetJunctionTags(HWND hDlg, const std::string& filename, std::vector<std::string>& tags) {
    // Empty tags
    tags.clear();

    OpenXLSX::XLDocument doc;
    try {
        doc.open(filename);
    } catch (const std::exception& e) {
        std::string message = "Failed to open Excel file: ";
        message += e.what();
        MessageBox(hDlg, message.c_str(), "Error", MB_OK | MB_ICONERROR);
        doc.close();
        return;
    }

    // Open the Cable Schedule Data worksheet
    OpenXLSX::XLWorksheet wks;
    try {
        wks = doc.workbook().worksheet("Cable Schedule Data");

        for (int row = 3; wks.cell(row, 4).value() != ""; ++row) {
            std::string junctionTag = wks.cell(row, 3).value();

            if (junctionTag == "N/A") continue;

            if (std::find(tags.begin(), tags.end(), junctionTag) == tags.end()) {
                tags.push_back(junctionTag);
            }
        }
    } catch (const std::exception& e) {
        std::string message = "Excel file is not compatible: ";
        message += e.what();
        MessageBox(hDlg, message.c_str(), "Error", MB_OK | MB_ICONERROR);
        doc.close();
        return;
    }

    doc.close();
}

int _xlsxGetJunctionFootprint(HWND hDlg, std::string filename, std::string junctionTag, BoxSize boxSize) {
    std::vector<Cable> cables = _xlsxGetCables(hDlg, filename, junctionTag);

    int footprint = 0;
    int terminal = 1;
    int table = 1;

    for (int i = 0; i < cables.size(); ++i) {
        if (_shouldSplit(boxSize, cables, i, terminal, table)) {
            terminal = 1;
            table ++;
        }

        terminal += cables[i].getTerminalFootprint();
        footprint += cables[i].getTerminalFootprint();

        // Check if we over ran any tables, and return the largest int possible to indicate the box is full
        if (boxSize == BoxSize::LARGE && terminal > 72) return std::numeric_limits<int>::max();
    }

    return footprint;
}

void _updateSizeRadioButtons(HWND hDlg, const std::vector<HWND>& sizeButtons, const std::vector<int>& spareCounts) {
    const std::vector<std::string> boxSizes = { "24x24x8", "16x16x6", "12x12x6", "Custom Box" };

    for (size_t i = 0; i < sizeButtons.size(); ++i) {
        std::string displayText;
        if (boxSizes[i] == "Custom Box") {
            displayText = "Custom Box";
        } else {
            if (spareCounts[i] < 0)
                displayText = boxSizes[i] + " - Doesn't Fit";
            else
                displayText = boxSizes[i] + " - " + std::to_string(spareCounts[i]) + " Spare";
        }

        SetWindowText(sizeButtons[i], displayText.c_str());
        EnableWindow(sizeButtons[i], spareCounts[i] >= 0);
    }

    // Ensure at least one valid button is selected
    bool hasChecked = false;
    for (HWND btn : sizeButtons) {
        if (IsWindowEnabled(btn)) {
            if (!hasChecked) {
                SendMessage(btn, BM_SETCHECK, BST_CHECKED, 0);
                hasChecked = true;
            } else {
                SendMessage(btn, BM_SETCHECK, BST_UNCHECKED, 0);
            }
        } else {
            SendMessage(btn, BM_SETCHECK, BST_UNCHECKED, 0);
        }
    }
}

void _rebuildDialogBox(const std::vector<std::string>& junctionTags, std::vector<HWND>& tagRadioButtons, std::vector<HWND>& sizeRadioButtons, const std::vector<int>& spareCounts, HWND hDlg) {
    // --- Layout Constants ---
    const int dialogBaseHeight = 50;
    const int radioButtonHeight = 20;
    const int radioSpacing = 20;
    const int groupBoxPadding = 20;
    const int groupBoxX = 15;
    const int groupBoxWidth = 340;
    const int radioButtonWidth = groupBoxWidth - 2 * groupBoxPadding;
    const int okCancelSpacing = 10;
    const int buttonWidth = 80;
    const int buttonHeight = 24;

    // --- Group box handles ---
    static HWND hGroupBoxTags = nullptr;
    static HWND hGroupBoxSizes = nullptr;

    // --- Cleanup previous group boxes and buttons ---
    if (hGroupBoxTags) { DestroyWindow(hGroupBoxTags); hGroupBoxTags = nullptr; }
    if (hGroupBoxSizes) { DestroyWindow(hGroupBoxSizes); hGroupBoxSizes = nullptr; }

    for (HWND hBtn : tagRadioButtons) DestroyWindow(hBtn);
    tagRadioButtons.clear();

    for (HWND hBtn : sizeRadioButtons) DestroyWindow(hBtn);
    sizeRadioButtons.clear();

    // --- Dynamic Junction Tag Group ---
    int tagsGroupBoxY = 45;
    int tagsGroupBoxHeight = static_cast<int>(junctionTags.size() + 1) * radioSpacing + 2 * groupBoxPadding;

    hGroupBoxTags = CreateWindowEx(
        0, "BUTTON", "Select a Junction Tag",
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        groupBoxX, tagsGroupBoxY, groupBoxWidth, tagsGroupBoxHeight,
        hDlg, (HMENU)IDC_RADIO_TAG_GROUP, GetModuleHandle(NULL), NULL
    );

    int tagsStartY = tagsGroupBoxY + groupBoxPadding;
    int tagsStartX = groupBoxX + groupBoxPadding;

    for (size_t i = 0; i < junctionTags.size(); ++i) {
        HWND hRadio = CreateWindowEx(
            0, "BUTTON", junctionTags[i].c_str(),
            WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | (i == 0 ? WS_GROUP : 0),
            tagsStartX, tagsStartY + static_cast<int>(i) * radioSpacing,
            radioButtonWidth, radioButtonHeight,
            hDlg, (HMENU)(IDC_RADIO_TAG_GROUP + 1 + i), GetModuleHandle(NULL), NULL
        );
        tagRadioButtons.push_back(hRadio);
    }

    // --- Add a Select All Button ---

    if (junctionTags.size() > 0) {
        HWND hRadio = CreateWindowEx(
            0, "BUTTON", "Select All",
            WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
            tagsStartX, tagsStartY + static_cast<int>(junctionTags.size()) * radioSpacing,
            radioButtonWidth, radioButtonHeight,
            hDlg, (HMENU)(IDC_RADIO_TAG_GROUP + 1 + junctionTags.size()), GetModuleHandle(NULL), NULL
        );
        tagRadioButtons.push_back(hRadio);
    }

    // --- Static Box Size Group ---
    const std::vector<std::string> boxSizes = { "24x24x8", "16x16x6", "12x12x6", "Custom Box" };

    int sizesGroupBoxY = tagsGroupBoxY + tagsGroupBoxHeight + 10;
    int sizesGroupBoxHeight = static_cast<int>(boxSizes.size()) * radioSpacing + 2 * groupBoxPadding;

    hGroupBoxSizes = CreateWindowEx(
        0, "BUTTON", "Select Junction Box Size",
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        groupBoxX, sizesGroupBoxY, groupBoxWidth, sizesGroupBoxHeight,
        hDlg, (HMENU)IDC_RADIO_SIZE_GROUP, GetModuleHandle(NULL), NULL
    );

    int sizesStartY = sizesGroupBoxY + groupBoxPadding;
    int sizesStartX = groupBoxX + groupBoxPadding;

    for (size_t i = 0; i < boxSizes.size(); ++i) {
        HWND hRadio = CreateWindowEx(
            0, "BUTTON", "", // Placeholder text
            WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | (i == 0 ? WS_GROUP : 0),
            sizesStartX, sizesStartY + static_cast<int>(i) * radioSpacing,
            radioButtonWidth, radioButtonHeight,
            hDlg, (HMENU)(IDC_RADIO_SIZE_GROUP + 1 + i), GetModuleHandle(NULL), NULL
        );
        sizeRadioButtons.push_back(hRadio);
    }

    // --- Apply spare count info using helper function ---
    _updateSizeRadioButtons(hDlg, sizeRadioButtons, spareCounts);

    // --- Resize Dialog to Fit Everything ---
    int newDialogHeight = dialogBaseHeight + sizesGroupBoxY + sizesGroupBoxHeight + buttonHeight + 2 * okCancelSpacing;

    RECT windowRect;
    GetWindowRect(hDlg, &windowRect);
    int newDialogWidth = windowRect.right - windowRect.left;

    SetWindowPos(hDlg, nullptr, 0, 0, newDialogWidth, newDialogHeight, SWP_NOMOVE | SWP_NOZORDER);

    // --- Move OK/Cancel buttons to bottom-right ---
    RECT clientRect;
    GetClientRect(hDlg, &clientRect);
    int bottomY = clientRect.bottom - buttonHeight - okCancelSpacing;

    HWND hOkButton = GetDlgItem(hDlg, IDC_OK_BTN);
    HWND hCancelButton = GetDlgItem(hDlg, IDC_CANCEL_BTN);

    SetWindowPos(hOkButton, nullptr,
        clientRect.right - 2 * (buttonWidth + okCancelSpacing), bottomY,
        buttonWidth, buttonHeight, SWP_NOZORDER);

    SetWindowPos(hCancelButton, nullptr,
        clientRect.right - (buttonWidth + okCancelSpacing), bottomY,
        buttonWidth, buttonHeight, SWP_NOZORDER);
}

INT_PTR CALLBACK _DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    static std::string filenameString = "";
    static std::vector<std::string> junctionTags;
    static std::vector<HWND> tagRadioButtons;
    static std::vector<HWND> sizeRadioButtons;
    static DialogResult* result = nullptr;
    static std::vector<int> spareCounts = {-1, -1, -1, -1};

    switch (message)
    {
    case WM_INITDIALOG:
        // Initialize dialog box
        result = reinterpret_cast<DialogResult*>(lParam);
        _rebuildDialogBox(junctionTags, tagRadioButtons, sizeRadioButtons, spareCounts, hDlg);
        break;
    
    case WM_COMMAND: {
        int ctrlId = LOWORD(wParam);

        // If one of the junction tag radio buttons is selected
        if (ctrlId >= IDC_RADIO_TAG_GROUP + 1 && ctrlId < IDC_RADIO_TAG_GROUP + 2 + junctionTags.size()) {
            int selectedIndex = ctrlId - IDC_RADIO_TAG_GROUP - 1;
            std::string selectedTag = junctionTags[selectedIndex];

            if (selectedIndex == junctionTags.size()) {
                spareCounts[0] = -1;
                spareCounts[1] = -1;
                spareCounts[2] = -1;
            } else {
                spareCounts[0] = 144 - _xlsxGetJunctionFootprint(hDlg, filenameString, selectedTag, BoxSize::LARGE);
                spareCounts[1] = 42  - _xlsxGetJunctionFootprint(hDlg, filenameString, selectedTag, BoxSize::MEDIUM);;
                spareCounts[2] = 24  - _xlsxGetJunctionFootprint(hDlg, filenameString, selectedTag, BoxSize::SMALL);;
            }

            spareCounts[3] = 0;
            
            _updateSizeRadioButtons(hDlg, sizeRadioButtons, spareCounts);
        }

        switch (ctrlId)
        {
        case IDC_BROWSE_BTN: {
            char fileName[MAX_PATH] = {};
            OPENFILENAME ofn = { sizeof(ofn) };
            ofn.lpstrFilter = "Excel Files\0*.xlsx\0";
            ofn.lpstrFile = fileName;
            ofn.nMaxFile = MAX_PATH;
            ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
            ofn.hwndOwner = hDlg;

            if (GetOpenFileName(&ofn)) {
                // Parse the file
                filenameString = fileName;
                _xlsxGetJunctionTags(hDlg, filenameString, junctionTags);

                _rebuildDialogBox(junctionTags, tagRadioButtons, sizeRadioButtons, spareCounts, hDlg);
            }
            break;
        }

        case IDC_OK_BTN: {
            result->accepted = true;

            bool tagSelected = false;
            for (size_t i = 0; i < tagRadioButtons.size(); ++i) {
                if (SendMessage(tagRadioButtons[i], BM_GETCHECK, 0, 0) == BST_CHECKED) {
                    if (i == junctionTags.size()) {
                        result->selectedTag = "Select All";
                    } else {
                        result->selectedTag = junctionTags[i];
                    }
                    tagSelected = true;
                    break;
                }
            }

            if (!tagSelected) {
                result->accepted = false;
            }

            // Find selected size
            for (size_t i = 0; i < sizeRadioButtons.size(); ++i) {
                if (SendMessage(sizeRadioButtons[i], BM_GETCHECK, 0, 0) == BST_CHECKED) {
                    switch (i)
                    {
                    case 0:
                        result->selectedSize = BoxSize::LARGE;
                        break;
                    case 1:
                        result->selectedSize = BoxSize::MEDIUM;
                        break;
                    case 2:
                        result->selectedSize = BoxSize::SMALL;
                        break;
                    case 3:
                        result->selectedSize = BoxSize::CUSTOM;
                        break;
                    default:
                        result->accepted = false;
                        break;
                    }
                    break;
                }
            }

            result->filename = filenameString;

            EndDialog(hDlg, IDOK);
            break;
        }

        case IDC_CANCEL_BTN:
            result->accepted = false;
            EndDialog(hDlg, IDCANCEL);
            break;
        }
        break;
    }
    
    case WM_CLOSE:
        result->accepted = false;
        EndDialog(hDlg, IDCLOSE);
        break;

    default:
        return FALSE;
    }

    return TRUE;
}