# TradeTracker: Stocks, Futures and Options positions tracker (includes Interactive Brokers IBKR TWS API integration)
[![Release](https://img.shields.io/github/v/release/PaulSquires/TradeTracker?style=flat-square)](https://github.com/PaulSquires/TradeTracker/releases)
[![License](https://img.shields.io/github/license/PaulSquires/TradeTracker?style=flat-square)](LICENSE)
<!-- ![Downloads](https://img.shields.io/github/downloads/PaulSquires/TradeTracker/total?style=flat-square) -->
   
### Why are you still using a spreadsheet to track your stock, futures and options trading? ###
**Let TradeTracker free you from the constraints of spreadsheets.**

## What is TradeTracker 
TradeTracker a free application that allows you to track your Stocks, Futures and Options positions. A fantastic alternative to using spreadsheets like Microsoft Excel or Google Sheets! 

**NOTE:** TradeTracker is the new name for this project. The old name IB-Tracker is being phased out but you may see references to that name on this site and in the prior Release downloads.

Stocks, Futures and Options positions tracker (includes Interactive Brokers IBKR TWS API integration)

You do not need an Interactive Brokers account but if you have one then TradeTracker can connect to your running IB Trader Workstation (TWS) instance in order to display real time price action for all of your positions. You can manually record and edit trades and transactions and be able to view all current active positions and all closed positions. Each trade shows all transaction history giving you an instant look at your overall financial picture. 

If you are not using TWS integration, TradeTracker allows you to "scrape" stock and futures prices from Yahoo Finance (albeit, the prices will be at least 15 minutes delayed). You just need to attempt to connect to TWS and when that fails, you will get a popup asking if you would like to retrieve the scraped market prices.

This program exists because tracking trades and their corresponding transactions in IBKR versus TastyTrade, Think or Swim, etc is not easy.  In many parts of the world outside of the USA, it seems that IBKR is the only affordable and viable alternative for options trading. IB-Trader aims to make the trade tracking process as enjoyable and painless as possible.

![screenshot](/TradeTracker/assets/TradeTracker-main.png?raw=true "TradeTracker Main")

![screenshot](/TradeTracker/assets/TradeTracker-closed.png?raw=true "TradeTracker Closed")

![screenshot](/TradeTracker/assets/TradeTracker-trade.png?raw=true "TradeTracker Trade")

![screenshot](/TradeTracker/assets/TradeTracker-trans.png?raw=true "TradeTracker Transactions")

## Goals
* Can easily be used with or without an IBKR account allowing TradeTracker to be a great alternative to tracking your positions using spreadsheets.
* Equity and Futures Options focused (underlying stocks and futures can also be tracked).
* Allow Interactive Broker users to have a better experience tracking their active trades and trade histories.
* Allow trades to be grouped and tracked by Category/Strategy.
* Easy to learn and very intuitive with all information available on the main screen. 
* Portable. You can easily run the program from a thumb drive. No intrusive install or uninstall procedures.
* High DPI aware. Works and looks great on monitors of all sizes, resolutions and font scalings.
* Small (about 620K), fast (written in C++), and self-contained (no external dependencies).
* Simple text file "database" that can easily be manually edited if needed. No additional database engine required.
* The only pain point is that you have to manually enter your transactions into TradeTracker in order to ensure that transactions are grouped with the correct trades. However, there is a "Reconcile" functionality that ensures that your local data always matches what exists in IBKR/TWS. 
* Currently, only Windows compatible (Windows 10 and Windows 11). May work on older Windows versions but not guaranteed. It will work on Linux using Wine but you may experience some visual glitches due to the font family Unicode characters that the program uses. On Linux Wine you should install the "Segoe UI" font. Details found here: https://github.com/mrbvrz/segoe-ui-linux  It is also necessary to use TradeTracker version 3.5.0 or higher.

## Download
The latest package can be downloaded from the [RELEASES](https://github.com/PaulSquires/TradeTracker/releases) page.
If you wish to compile the source code yourself, it was created using Microsoft Visual Studio Community 2022 and you will find the necessary solution and project files bundled in the source code. It compiles using the C++20 standard.

## Installation
All release packages come with a compiled EXE. That is all the application requires. Yes, just one file. The compiled EXE can be found inside the **TradeTracker.zip** download from the [RELEASES](https://github.com/PaulSquires/TradeTracker/releases) page.
1. Create a folder on your computer that you will use for TradeTracker. Ensure that the folder can be read and written to (eg. C:\TradeTracker).
2. Unpack/Unzip the downloaded archive (TradeTracker.zip) into your TradeTracker folder.
3. Double Click on the TradeTracker.exe application to start it. That's it. Simple.
4. To uninstall, just delete the files and folder.

TradeTracker will create four additional files as needed when it executes:
* config.txt
* journalnotes.txt
* tradeplan.txt
* database.db

**NOTE:** If you get a popup warning message box from the operating system saying that "a DLL is not found and reinstalling the program may fix this problem", then it means that you are missing the Visual C++ redistributable package that is normally available on most Windows 10 and Windows 11 machines. Download and install it from this Microsoft page:  https://learn.microsoft.com/en-US/cpp/windows/latest-supported-vc-redist?view=msvc-170   Here is the direct link to the file: https://aka.ms/vs/17/release/vc_redist.x86.exe  

## Usage

### Getting Started
Start IB Trader Workstation (TWS) and then click "Connect to TWS" from within TradeTracker.

TradeTracker can run without connecting to TWS but obviously you will not get streaming price data or be able to "reconcile" your trade data to what is in your IBKR account.

### Configuration
The configuration file (TradeTracker-config.txt) is an extremely simple text file. Each line is composed of two items (key and values) separated by one or more pipe ("|") characters.

By default, TradeTracker will connect to the regular TWS trading port (7496). You can manually edit the configuration file to change the **ENABLEPAPERTRADING** configuration option to **true** in order connect to the TWS paper trading port (7497).

**NOTE: Until the documentation is completed, you will find functionality available via right-click popup menus. Simply try right clicking on the trade's ticker line or any one (or more) selected legs of a trade. Doing so should popup a menu with related actions (such as rolling legs, expiring or closing legs, taking assignments, adding to existing position, etc).**

### Categories
When you enter or modify a Trade, you can specify that the Trade belongs to a Category. A Category is simply a logical grouping that you have designated for the Trade. For example, you may wish to group all 90 DTE Strangles together. When the list of Active Trades are shown, they will be displayed in order using your list of Categories.

### Creating a New Trade
When entering the ticker symbol, ensure that any Futures symbol is preceded by a forward slash. For example, Futures oil would be */CL*. This is necessary within TradeTracker even though IBKR itself does not use a forward slash preceding Futures ticker symbols. For Futures, you will also have to enter the ending date for the contract. This ensures that TradeTracker can correctly query TWS in order to get real time price data.

### Rolling, Closing or Expiring Legs of a Trade
Click (while holding down the CTRL key) on each Leg you wish to Roll, Close or Expire. With the Legs selected, right click on one of those legs to display a popup menu. Select the Roll, Close or Expire action from that menu.

### Multiple Instances of TradeTracker
If TradeTracker is executed multiple times then the previous instance of the program will be found and that instance will be displayed. This prevents multiple copies of the application from running at one time thereby overwriting the database data by each application instances. The exception to this rule is if TradeTracker.exe is executed from different folders/subdirectories. In this case, individual instances of TradeTracker will run because their databases will exist in different folders thereby avoiding the overwrite issue.

### Reconcile Local Database to TWS
If you are connected to TWS, performing a Reconcile will fetch your position data from the IBKR servers and then compare them to your locally stored data. Any discrepancies between the data will be displayed allowing you to find and correct any anomalies. This is a very useful feature to ensure that trades and transactions that you have created with TradeTracker match trades that you have executed through TWS.

## Contributing
Feel free to contribute something that would be convenient and useful to include in the application. You are welcome to open a pull request and contribute.

## License
This project is free software; you can redistribute it and/or modify it under the terms of the MIT license. See [LICENSE](https://github.com/PaulSquires/TradeTracker/blob/main/LICENSE.txt) for details.
