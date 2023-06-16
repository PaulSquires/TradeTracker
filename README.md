# IB-Tracker
[![Release](https://img.shields.io/github/v/release/PaulSquires/IB-Tracker?label=release)](https://github.com/PaulSquires/IB-Tracker/releases)
![Downloads](https://img.shields.io/github/downloads/PaulSquires/IB-Tracker/total?label=downloads)
[![License](https://img.shields.io/github/license/PaulSquires/IB-Tracker?label=license)](LICENSE.txt)

Interactive Brokers (IBKR) Stocks, Futures and Options positions tracker

## What is IB-Tracker
IB-Tracker a free application that connects to your running IB Trader Workstation (TWS) instance in order to display real time price action for all of your recorded stock, futures and options positions. You can record and edit trades and transactions and be able to view all current active positions and all closed positions. Each trade shows all transaction history giving you a instant look at the cost basis of your trade. 

![screenshot](https://pragtical.github.io/assets/img/editor.png)

## Goals
* Very Option focused (although stocks and futures can be tracked also).
* Allow Interactive Broker users to have a better experience tracking their active trades and trade histories.
* Easy to learn and very intuitive with all information available on the main screen. 
* Portable. You can easily run the program from a thumb drive. No intrusive install or uninstall procedures.
* High DPI aware. Works and looks great on monitors of all sizes, resolutions and font scalings.
* Small (less than 500K), fast (written in C++), and self-contained (no external dependencies).
* Simple text file "database" that can easily be manually edited if needed. No database engine needed.
* Currently, only Windows compatible (Windows 10 and Windows 11). May work on older Windows versions but not guaranteed.

## Download
The latest package can be downloaded from the [RELEASES](https://github.com/PaulSquires/IB-Tracker/releases) page.

## Installation
All release packages come with a compiled EXE. That is all the application requires. Yes, just one file.
1. Create a folder on your computer that you will use for IB-Tracker.
2. Unpack/Unzip the downloaded archive into your IB-Tracker folder.
3. Click on the IB-Tracker.exe application. That's it. Simple.
4. To uninstall, just delete the files and folder.

IB-Tracker will create two additional files when it first executes:
* IB-Tracker-config.txt
* IB-Tracker-database.db

## Usage

### Getting Started
Start IB Trader Workstation (TWS) and then click "Connect to TWS" from within IB-Tracker.

IB-Tracker can run without connecting to TWS but obviously you will not get streaming price data or be able to "reconcile" your trade data to what is in your IBKR account.

### Configuration
The configuration file (IB-Tracker-config.txt) is an extremely simple text file. Each line is composed of two items (key and value) separated by a pipe ("|") character.

```
IB-TRACKER-CONFIG|1.0.0
TRADERNAME|TheDude
STARTUPCONNECT|false
```

STARTUPCONNECT can be set directly from inside the IB-Tracker application. TRADERNAME can only be set by manually editing the configuration text file. TRADERNAME shows on the main menu.

## Contributing
Feel free to contribute something that would be convenient and useful to include in the application. You are welcome to open a pull request and contribute.

I have written pure low level WinAPI based applications for 30 years (in non "C" based languages) but I wanted to learn C++ and write an application without a framework such as MFC, ATL or WTL. If you are a seasoned C++ programmer then feel free to peruse the source code and offer suggestions on how I can improve the code and my C++ skills.

## License
This project is free software; you can redistribute it and/or modify it under the terms of the MIT license. See [LICENSE](https://github.com/PaulSquires/IB-Tracker/blob/main/LICENSE.txt) for details.
