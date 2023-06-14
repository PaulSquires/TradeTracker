# IB-Tracker
Interactive Brokers (IBKR) Stocks, Futures and Options positions tracker

[![Release](https://img.shields.io/github/v/release/akaunting/akaunting?label=release)](https://github.com/akaunting/akaunting/releases)
![Downloads](https://img.shields.io/github/downloads/akaunting/akaunting/total?label=downloads)
[![Translations](https://badges.crowdin.net/akaunting/localized.svg)](https://crowdin.com/project/akaunting)
[![Tests](https://img.shields.io/github/actions/workflow/status/akaunting/akaunting/tests.yml?label=tests)](https://github.com/akaunting/akaunting/actions)
[![License](https://img.shields.io/github/license/akaunting/akaunting?label=license)](LICENSE.txt)

## What is IB-Tracker
IB-Tracker a free application that connects to your running IB Trader Workstation (TWS) instance in order to display real time price action for all of your recorded stock, futures and options positions. You can record and edit trades and transactions and be able to view all current active positions and all closed positions. Each trade shows all transaction history giving you a instant look at the cost basis of your trade. 

![screenshot](https://pragtical.github.io/assets/img/editor.png)

## Goals
* I have written pure low level WinAPI based applications for 20 years (in non "C" based languages) but I wanted to learn C++ and write an application without a framework such as MFC, ATL or WTL.
* Small (less than 500K), fast (written in C++), and self-contained (no external dependencies).
* Portable. You can easily run the program from a thumb drive.
* Modern looking. Dark and light themes.
* High DPI aware. Works and looks great on monitors of all sizes, resolutions and font scalings.
* Simple text file "database" that can easily be manually edited if needed. No database engine like SQLite needed.
* Easy to learn and very intuitive with all information available on the main screen. 
* Allow Interactive Broker users to have a better experience tracking their active trades and trade histories.
* Currently only Windows compatible (Windows 10 and Windows 11). May work on older Windows versions but not supported.

## Download
The latest package can be downloaded from the [RELEASES page](https://github.com/PaulSquires/IB-Tracker/releases).

## Installation
All release packages come with a compiled EXE. That is all the application requires. Yes, just one file.
1. Create a folder on your computer that you will use for IB-Tracker.
2. Unpack/Unzip the downloaded archive into your IB-Tracker folder.
3. Click on the IB-Tracker.exe application. That's it. Simple.

IB-Tracker will create two additional files when it first executes:
* IB-Tracker-config.txt
* IB-Tracker-database.db

## Usage

### Getting Started

### Configuration
The configuration file (IB-Tracker-config.txt) is an extremely simple text file. Each line is composed of two items (key and value) separated by a pipe ("|") character.

```
IB-TRACKER-CONFIG|1.0.0
THEME|Dark
TRADERNAME|TheDude
STARTUPCONNECT|false
```

THEME and STARTUPCONNECT can be set directly from inside the IB-Tracker application. TRADERNAME can only be set by manually editing the configuration text file. TRADERNAME shows on the main menu.

## Contributing
Feel free to contribute something that would be convenient and "Pragtical" to include on the core, you are welcome to open a pull request and contribute.

## License
This project is free software; you can redistribute it and/or modify it under the terms of the MIT license. See [LICENSE](https://github.com/PaulSquires/IB-Tracker/blob/main/LICENSE.txt) for details.
