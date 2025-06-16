/**
 * @file JunctionBuilder.cpp
 * @brief Definitons for building junction boxes.
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
    LARGE   ///< 24" × 24" × 8" enclosure
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
 * @return            Terminal count ("footprint").
 */
int _xlsxGetJunctionFootprint(HWND hDlg,
                              std::string filename,
                              std::string junctionTag);

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

    // Go through the .xlsx and build a cable object for every cable listed in the file.
    std::vector<Cable> cables = _xlsxGetCables(adsw_acadMainWnd(), result.filename, result.selectedTag);

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

    std::wstring junctionTag(result.selectedTag.begin(), result.selectedTag.end());

    // AcGePoint3d origin(19.3750, 12.4977, 0.0);

    AcGePoint3d origin(50.0, 0.0, 0.0);
    int terminal = 1;
    for (int i = 0; i < cables.size(); i++) {
        Cable cable = cables[i];

        cable.draw(origin, terminal, true, junctionTag.c_str(), 1);

        int terminalFootprint = cable.getTerminalFootprint();

        terminal += terminalFootprint;
        origin += AcGeVector3d(0.0, -0.25, 0.0) * terminalFootprint;
    }

    /*
        Customer side cables are out of the scope of this tool. If customer side cables are
        to be placed on the diagram, they should be done manually or with a seperate tool.
    */
}

// -----------------------------------------------------------------------------
// Helper Function Definitions
// -----------------------------------------------------------------------------

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

int _xlsxGetJunctionFootprint(HWND hDlg, std::string filename, std::string junctionTag) {
    std::vector<Cable> cables = _xlsxGetCables(hDlg, filename, junctionTag);

    int footprint = 0;
    for (Cable cable : cables) {
        footprint += cable.getTerminalFootprint();
    }

    return footprint;
}

void _updateSizeRadioButtons(HWND hDlg, const std::vector<HWND>& sizeButtons, const std::vector<int>& spareCounts) {
    const std::vector<std::string> boxSizes = { "24x24x8", "16x16x6", "12x12x6" };

    for (size_t i = 0; i < sizeButtons.size(); ++i) {
        std::string displayText;
        if (spareCounts[i] < 0)
            displayText = boxSizes[i] + " - Doesn't Fit";
        else
            displayText = boxSizes[i] + " - " + std::to_string(spareCounts[i]) + " Spare";

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
    int tagsGroupBoxHeight = static_cast<int>(junctionTags.size()) * radioSpacing + 2 * groupBoxPadding;

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

    // --- Static Box Size Group ---
    const std::vector<std::string> boxSizes = { "24x24x8", "16x16x6", "12x12x6" };

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
    static std::vector<int> spareCounts = {0, 0, 0};

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
        if (ctrlId >= IDC_RADIO_TAG_GROUP + 1 && ctrlId < IDC_RADIO_TAG_GROUP + 1 + junctionTags.size()) {
            int selectedIndex = ctrlId - IDC_RADIO_TAG_GROUP - 1;
            std::string selectedTag = junctionTags[selectedIndex];

            int numTermUsed = _xlsxGetJunctionFootprint(hDlg, filenameString, selectedTag);

            spareCounts[0] = 144 - numTermUsed;
            spareCounts[1] = 42 - numTermUsed;
            spareCounts[2] = 24 - numTermUsed;

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
                    result->selectedTag = junctionTags[i];
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