# TradeTracker: Stocks, Futures and Options positions tracker (includes Interactive Brokers IBKR TWS API integration)
[![Release](https://img.shields.io/github/v/release/PaulSquires/TradeTracker?style=flat-square)](https://github.com/PaulSquires/TradeTracker/releases)
![Downloads](https://img.shields.io/github/downloads/PaulSquires/TradeTracker/total?style=flat-square)
[![License](https://img.shields.io/github/license/PaulSquires/TradeTracker?style=flat-square)](LICENSE)
<!-- ![Downloads](https://img.shields.io/github/downloads/PaulSquires/TradeTracker/total?style=flat-square) -->

### ANNOUNCEMENT (November 20, 2024)
TradeTracker is now fully cross-platform (Windows, Linux, MacOS).
Built using Dear ImGui and backward compatible with the current released version.
- Source code for the Windows only version (4.2.0) still available from previous older Github commits.


### Why are you still using a spreadsheet to track your stock, futures and options trading? ###
**Let TradeTracker free you from the constraints of spreadsheets.**

## What is TradeTracker 
TradeTracker a free application that allows you to track your Stocks, Futures and Options positions. A fantastic alternative to using spreadsheets like Microsoft Excel or Google Sheets! 

Stocks, Futures and Options positions tracker (includes Interactive Brokers IBKR TWS API integration)

You do not need an Interactive Brokers account but if you have one then TradeTracker can connect to your running IB Trader Workstation (TWS) instance in order to display real time price action for all of your positions. You can manually record and edit trades and transactions and be able to view all current active positions and all closed positions. Each trade shows all transaction history giving you an instant look at your overall financial picture. 

This program exists because tracking trades and their corresponding transactions in IBKR versus TastyTrade, Think or Swim, etc is not easy.  In many parts of the world outside of the USA, it seems that IBKR is the only affordable and viable alternative for options trading. TradeTracker aims to make the trade tracking process as enjoyable and painless as possible.

![screenshot](/src/resources/active_trades.png?raw=true "TradeTracker Version5 User Interface")


## Goals
* Can easily be used with or without an IBKR account allowing TradeTracker to be a great alternative to tracking your positions using spreadsheets.
* Equity and Futures Options focused (underlying stocks and futures can also be tracked).
* Allow Interactive Broker users to have a better experience tracking their active trades and trade histories.
* Allow trades to be grouped and tracked by Category/Strategy.
* Easy to learn and very intuitive with all information available on the main screen. 
* High DPI aware. Works and looks great on monitors of all sizes, resolutions and font scalings.
* Small and fast (written in C++).
* Simple text file "database" that can easily be manually edited if needed. No additional database engine required.

## Download and Installation
The latest package can be downloaded from the [RELEASES](https://github.com/PaulSquires/TradeTracker/releases/latest) page.

Documentation and instructions can be found at the [TradeTracker website](https://www.tradetracker.planetsquires.com/)

## Contributing
Feel free to contribute something that would be convenient and useful to include in the application. You are welcome to open a pull request and contribute.

## License
This project is free software; you can redistribute it and/or modify it under the terms of the MIT license. See [LICENSE](https://github.com/PaulSquires/TradeTracker/blob/main/LICENSE.txt) for details.
