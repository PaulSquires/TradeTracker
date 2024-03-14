# TradeTracker: Stocks, Futures and Options positions tracker (includes Interactive Brokers IBKR TWS API integration)
[![Release](https://img.shields.io/github/v/release/PaulSquires/TradeTracker?style=flat-square)](https://github.com/PaulSquires/TradeTracker/releases)
![Downloads](https://img.shields.io/github/downloads/PaulSquires/TradeTracker/total?style=flat-square)
[![License](https://img.shields.io/github/license/PaulSquires/TradeTracker?style=flat-square)](LICENSE)
<!-- ![Downloads](https://img.shields.io/github/downloads/PaulSquires/TradeTracker/total?style=flat-square) -->
   
### Why are you still using a spreadsheet to track your stock, futures and options trading? ###
**Let TradeTracker free you from the constraints of spreadsheets.**

## What is TradeTracker 
TradeTracker a free application that allows you to track your Stocks, Futures and Options positions. A fantastic alternative to using spreadsheets like Microsoft Excel or Google Sheets! 

Stocks, Futures and Options positions tracker (includes Interactive Brokers IBKR TWS API integration)

You do not need an Interactive Brokers account but if you have one then TradeTracker can connect to your running IB Trader Workstation (TWS) instance in order to display real time price action for all of your positions. You can manually record and edit trades and transactions and be able to view all current active positions and all closed positions. Each trade shows all transaction history giving you an instant look at your overall financial picture. 

If you are not using TWS integration, TradeTracker allows you to "scrape" stock and futures prices from Yahoo Finance (albeit, the prices will be at least 15 minutes delayed). You just need to attempt to connect to TWS and when that fails, you will get a popup asking if you would like to retrieve the scraped market prices.

This program exists because tracking trades and their corresponding transactions in IBKR versus TastyTrade, Think or Swim, etc is not easy.  In many parts of the world outside of the USA, it seems that IBKR is the only affordable and viable alternative for options trading. TradeTracker aims to make the trade tracking process as enjoyable and painless as possible.

![screenshot](/TradeTracker/assets/active_trades.png?raw=true "TradeTracker Version4 User Interface")


## Goals
* Can easily be used with or without an IBKR account allowing TradeTracker to be a great alternative to tracking your positions using spreadsheets.
* Equity and Futures Options focused (underlying stocks and futures can also be tracked).
* Allow Interactive Broker users to have a better experience tracking their active trades and trade histories.
* Allow trades to be grouped and tracked by Category/Strategy.
* Easy to learn and very intuitive with all information available on the main screen. 
* Portable. You can easily run the program from a thumb drive. No intrusive install or uninstall procedures.
* High DPI aware. Works and looks great on monitors of all sizes, resolutions and font scalings.
* Small and lightning fast (written in C++), and self-contained (no external dependencies).
* Simple text file "database" that can easily be manually edited if needed. No additional database engine required.
* Currently, only Windows compatible (Windows 10 and Windows 11).

## Download and Installation
The latest package can be downloaded from the [RELEASES](https://github.com/PaulSquires/TradeTracker/releases/latest) page.
If you wish to compile the source code yourself, it was created using Microsoft Visual Studio Community 2022 and you will find the necessary solution and project files bundled in the source code. It compiles using the C++20 standard.

Documentation and instructions can be found at the [TradeTracker website](https://www.tradetracker.planetsquires.com/)

## Contributing
Feel free to contribute something that would be convenient and useful to include in the application. You are welcome to open a pull request and contribute.

## License
This project is free software; you can redistribute it and/or modify it under the terms of the MIT license. See [LICENSE](https://github.com/PaulSquires/TradeTracker/blob/main/LICENSE.txt) for details.
