<h2 align="center">
    <a href="https://github.com/EdenBarnes/Junction-Diagram-Automation-Suite" target="blank_">
        <img height="200" alt="GasTech Logo" src="https://raw.githubusercontent.com/EdenBarnes/Junction-Diagram-Automation-Suite/refs/heads/main/docs/images/GasTech_logo.png" />
    </a>
</h2>

<div align="center">

# Junction Diagram Automation Suite

An AutoCAD 2024 plugin for the ~~manipulation~~ and generation of junction box wiring diagrams

Proprietary - All Rights Reserved by GasTech Engineering LLC

[![Latest release](https://img.shields.io/github/v/release/edenbarnes/Junction-Diagram-Automation-Suite)](https://github.com/EdenBarnes/Junction-Diagram-Automation-Suite/releases/tag/v1.1.1)

</div>

## Introduction

The Junction Diagram Automation Suite is an all in one tool for generating and ~~manipulating~~ GasTech junction box wiring diagrams in AutoCAD 2024. Users can easily create junction box diagrams directly from GasTech IO lists. ~~Additionally, users can easily modify these junction box wiring diagrams using a set of tools to flip and re-index batches of cables~~. All in a single `.arx` plugin.

## Getting Started

*This document is for version 1.1.1*

### Installation

* Install the `JunctionBuilder.arx` file from the <a href="https://github.com/EdenBarnes/Junction-Diagram-Automation-Suite/releases">Releases tab</a>. (Consider storing this file in a dedicated AutoCAD plugin folder)

* Open AutoCAD and create a new drawing.

* Run the `APPLOAD` command.

* In the **Load/Unload Applications** dialog box, click **Contents** under **Startup Suite**.

* Click **Add** and navigate to the location of the `JunctionBuilder.arx` file you downloaded.

* Select `JunctionBuilder.arx` and click **Open**.

* Click **Close** in each dialog box.

The Junction Diagram Automation Suite will now automatically load each time you open AutoCAD.

### Usage

*The main functionality of the Junction Diagram Automation Suite is the automatic generation of junction box wiring diagrams from an input IO list. For a full list of commands and their usages please click [here](#commands).*

* You must first [install](#installation) the plugin before it can be used.

* Open AutoCAD and create a new drawing from one of GasTech's blank junction box wiring diagram templates. (Make an educated guess on the size of junction box you'll need)

* Run the `BUILDJUNCTION` command.

* In the **Junction Box Setup** dialog box, click **Browse**.

* Navigate to the location of your IO list `.xlsx` file.

* Select the `.xlsx` and click **Open**.

* The **Select a Junction Tag** box now lists every junction box defined in the IO list.

* Select the tag of the junction box you want to build, or click **Select All** to build every junction box in the IO list.

* The **Select a Junction Box Size** box now tells you how many spare terminals will exist when you build a box of the respective size.

    * If a certain size of junction box is not able to fit the cables defined in the IO list, the option will be greyed out.

    * If the size associated with the template you opened is greyed out, please exit the dialog, close the drawing, and open one that is able to accomodate your junction box.

* Select one of the sizes from the **Select a Junction Box Size** box, or select **Custom Box**. (**Custom Box** will be the only option if you previously clicked **Select All** in the **Select a Junction Tag** box)

* Click **Ok**.

* The plugin will now automatically draw the junction box you selected.

## Building From Source

*This is an advanced topic intended only for people who wish to modify the program in the future. If you simply wish to use the plugin, you may ignore this section.*

[*Click here for documentation*](https://edenbarnes.github.io/Junction-Diagram-Automation-Suite/html/index.html)

### General Requirements

Ensure you system includes the following:

 * **Git** for repository cloning.

 * **MSVC** supporting **C++17** or newer.

 * **CMake** (version 3.16+)

 * **Doxygen** if you wish to generate documentation.

### Cloning Sources

To obtain the Junction Diagram Automation Suite source code, use the following commands:

``` bash
git clone https://github.com/EdenBarnes/Junction-Diagram-Automation-Suite.git
```

### Get Dependencies

The Junction Diagram Automation Suite has the following dependencies:

* [OpenXLSX](https://github.com/troldal/OpenXLSX)
* [ObjectArx 2024](https://www.autodesk.com/developer-network/platform-technologies/autocad/objectarx-download)

#### Get OpenXLSX

To obtain OpenXLSX, use the following commands:

``` bash
git clone https://github.com/troldal/OpenXLSX.git
```

Place the `OpenXLSX\OpenXLSX` folder inside of `Junction-Diagram-Automation-Suite\external`.

The file `Junction-Diagram-Automation-Suite\external\OpenXLSX\CMakeLists.txt` should now exist.  `Junction-Diagram-Automation-Suite\external\OpenXLSX` should not contain another directory named `OpenXLSX`.

#### Get ObjectARX

Click [here](https://help.autodesk.com/view/OARX/2024/ENU/?guid=GUID-2A0C6C5A-9C98-465F-BFB6-012A4899F53A) for system requirements and click [here](https://www.autodesk.com/developer-network/platform-technologies/autocad/objectarx-download) to download the SDK.

> [!WARNING]
> Make sure you install ObjectArx 2024. Other versions may be incompatible. AutoCAD 2024 is only compatible with ObjectArx 2024.

After you've installed the ObjectArx 2024 SDK, you need tell CMake where to find it.
Edit this line in `CMakeLists.txt` to point to the location of the ObjectArx 2024 SDK.

``` CMake
# Path to ObjectARX SDK
set(ARX_SDK "C:/Autodesk/ObjectArxSDK2024")
```

### Build with CMake

To compile the Junction Diagram Automation Suite using **CMake**, use the follwing commands inside of `Junction-Diagram-Automation-Suite\`:

``` bash
cmake -B ./build
cmake --build ./build --config RELEASE
```

> [!WARNING]
> `rc.exe` must exist in the `PATH` when building with **CMake**. If `rc.exe` (which is part of **MSVC**) is missing, the dialog boxes will not function correctly.

## Commands

| Command                   | Description                                            |
| :---                      |                                                   ---: |
| [`BUILDJUNCTION`](#usage) | Builds a junction box diagram using data in an IO list |