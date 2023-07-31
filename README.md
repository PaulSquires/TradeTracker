# IB-Tracker: Interactive Brokers (IBKR) Options Tracker
[![Release](https://img.shields.io/github/downloads/PaulSquires/IB-Tracker/total)
![Downloads](https://img.shields.io/github/downloads/PaulSquires/IB-Tracker/total?label=downloads)
![GitHub](https://img.shields.io/github/license/PaulSquires/IB-Tracker)

[![Release](https://img.shields.io/github/v/release/PaulSquires/IB-Tracker?style=flat-square)](https://github.com/PaulSquires/IB-Tracker/releases)
![Downloads](https://img.shields.io/github/downloads/PaulSquires/IB-Tracker/total?style=flat-square)
[![License](https://img.shields.io/github/license/PaulSquires/IB-Tracker?style=flat-square)](LICENSE)

Interactive Brokers (IBKR) Stocks, Futures and Options positions tracker

## What is IB-Tracker
IB-Tracker a free application that connects to your running IB Trader Workstation (TWS) instance in order to display real time price action for all of your recorded stock, futures and options positions. You can record and edit trades and transactions and be able to view all current active positions and all closed positions. Each trade shows all transaction history giving you a instant look at the cost basis of your trade. 

I wrote this program because tracking trades and their corresponding transactions in IBKR versus TastyTrade, Think or Swim, etc is not easy. Being a member of several trading groups (Scott Stewart, MrTopPick. I use the Discord handle "TheDude"), I was jealous of users of those other non-IBKR platforms. Being a Canadian, IBKR is the only available and viable alternative for me for options trading so I am trying to make the trade tracking process as enjoyable and painless as possible.

![screenshot](/IB-Tracker/assets/ib-tracker-main.png?raw=true "IB-Tracker")

## Goals
* Equity and Futures Options focused (although underlying stocks and futures can also be tracked).
* Allow Interactive Broker users to have a better experience tracking their active trades and trade histories.
* Allow trades to be grouped and tracked by Category/Strategy.
* Easy to learn and very intuitive with all information available on the main screen. 
* Portable. You can easily run the program from a thumb drive. No intrusive install or uninstall procedures.
* High DPI aware. Works and looks great on monitors of all sizes, resolutions and font scalings.
* Small (about 475K), fast (written in C++), and self-contained (no external dependencies).
* Simple text file "database" that can easily be manually edited if needed. No database engine needed.
* The only pain point is that you have to manually enter your transactions into IB-Tracker in order to ensure that transactions are grouped with the correct trades. However, there is a "Reconcile" functionality that ensures that your local data always matches what exists in IBKR/TWS. 
* Currently, only Windows compatible (Windows 10 and Windows 11). May work on older Windows versions but not guaranteed. It will work on Linux using Wine but you may experience some visual glitches due to the font family unicode characters that the program uses.

## Download
The latest package can be downloaded from the [RELEASES](https://github.com/PaulSquires/IB-Tracker/releases) page.
If you wish to compile the source code yourself, it was created using Microsoft Visual Studio Community 2022 and you will find the necessary solution and project files bundled in the source code. It compiles using the C++20 standard.

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
The configuration file (IB-Tracker-config.txt) is an extremely simple text file. Each line is composed of two items (key and values) separated by one or more pipe ("|") characters.

**NOTE: Until the documentation is completed, you will find functionality available via right-click popup menus. Simply try right clicking on the trade's ticker line or any one (or more) selected legs of a trade. Doing so should popup a menu with related actions (such as rolling legs, expiring or closing legs, taking assignments, etc).**

### Trades, Transaction, and Legs
Documentation work in progress...

### Categories
Documentation work in progress...

### Creating a New Trade
Documentation work in progress...

### Adding to an Existing Trade
Documentation work in progress...

### Closing or Expiring Legs of a Trade
Documentation work in progress...

### Rolling Legs of a Trade
Documentation work in progress...

### Option Assignment
Documentation work in progress...

### Editing Transactions
Documentation work in progress...

### Multiple Instances of IB-Tracker
Documentation work in progress...

### Reconcile Local Database to TWS
Documentation work in progress...

## TO DO
- Continue to improve the keyboard navigation of the custom built controls.
- Implement a "purge closed trades" function to remove closed trades (useful for closing out at year end).
- Continue to refine and refactor existing functionality and code base.

## Contributing
Feel free to contribute something that would be convenient and useful to include in the application. You are welcome to open a pull request and contribute.

Having written low level WinAPI based applications for many years in non "C" based languages, I wanted to learn C++ and write an application without a framework such as MFC, ATL or WTL. If you are a seasoned C++ programmer then feel free to peruse the source code and offer suggestions on how I can improve the code and my C++ skills because I realize that my approach to C++ may not be optimal.

## License
This project is free software; you can redistribute it and/or modify it under the terms of the MIT license. See [LICENSE](https://github.com/PaulSquires/IB-Tracker/blob/main/LICENSE.txt) for details.
