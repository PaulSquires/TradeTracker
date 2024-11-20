/*

MIT License

Copyright(c) 2023-2024 Paul Squires

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#ifndef APPSTATE_H
#define APPSTATE_H

#include "imgui.h"
#include <memory>
#include <string> 
#include <thread>
#include <unordered_map>
#include <vector>


#ifdef _WIN32
    #include "tws-api/windows/Decimal.h"
    #include "tws-api/windows/Contract.h"
#else   // Linux/Apple
    #include "tws-api/linux/Decimal.h"
    #include "tws-api/linux/Contract.h"
#endif

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))


typedef long TickerId;

class AppState;      // forward declare
class Transaction;   // forward declare
class Trade;         // forward declare
class Leg;           // forward declare

struct TickerData {
    double last_price = 0;
    double open_price = 0;
    double close_price = 0;
    double delta = 0;         // For Option legs
};

// Structure & vector to hold all positions returned from connection to IBKR (TWS).
// These are used for the reconciliation between TradeTracker and IBKR.
struct positionStruct {
    int contract_id = 0;
    Contract contract;
    std::vector<std::shared_ptr<Leg>> legs;    // pointer list for all legs that make up the position
    int open_quantity = 0;
    std::string ticker_symbol;
    std::string underlying;
    std::string expiry_date;
    double strike_price = 0;
    std::string put_call;
    // References to Trade and Leg that this position belongs to. The IBKR contract reference gets
    // set when the positions are reconciled.
    std::shared_ptr<Trade> trade;
    std::shared_ptr<Leg> leg;
};

struct PortfolioData {
    Decimal position = 0;
    double market_price = 0;
    double market_value = 0;
    double average_cost = 0;
    double unrealized_pnl = 0;
    double realized_pnl = 0;
};

struct JournalFolder {
    int id = 0;                 // auto generated during load
    std::string description;    // eg.  Folder1
    int notes_count = 0;
};

struct JournalNote {
    int id = 0;                 // auto generated during load
    int folder_id = 0;          // during load match to JournalFolder id
    bool is_trash = false;
    bool is_perm_delete = false;
    std::string folder;
    std::string text;
};

enum class TableType {
    none = 1000,
    active_trades,
    closed_trades,
    trade_history,
    trans_panel,
    trans_edit,
    import_left,
    import_right
};

enum class ActiveTradesFilterType {
    Category = 0,
    Expiration,
    TickerSymbol,
    TradeCompletedPercentage,
    SharesFuturesQuantity
};

enum class TransDateFilterType {
    Today,
    Yesterday,
    Days7,
    Days14,
    Days30,
    Days60,
    Days120,
    MonthToDate,
    YearToDate,
    Custom
};

enum class NumberFormatType {
    American,
    European
};

enum class CostingMethodType{
    AverageCost,
    fifo
};

enum class StartWeekdayType {
    Sunday,
    Monday
};

enum class ColorThemeType {
    Dark,
    Light
};

enum class TabPanelItem {
    ActiveTrades,
    ClosedTrades,
    Transactions,
    JournalNotes
};

enum class PutCall {
    Put,
    Call,
    Count,      // used in StrategyButton when toggling state
    Nothing     // always put last for historical reasons saving to database
};

enum class LongShort {
    Long = 0,
    Short,
    Count,
    Nothing
};

enum class Underlying {
    Options,
    Shares,
    Futures,
    Dividend,
    Other,
    Nothing     // always put last for historical reasons saving to database
};

enum class Action {
    STO,
    BTO,
    STC,
    BTC,
    Nothing     // always put last for historical reasons saving to database
};

enum class TradeAction {
    new_options_trade,
    new_shares_trade,
    new_futures_trade,
    manage_shares,
    manage_futures,
    roll_leg,
    close_leg,
    expire_leg,
    close_all_legs,
    close_all_shares,
    close_all_futures,
    assignment,
    add_options_to_trade,
    add_shares_to_trade,
    add_futures_to_trade,
    add_dividend_to_trade,
    add_income_expense_to_trade,
    other_income_expense,
    edit_transaction,
    no_action
};

enum class TradeActionOptionType {
    short_put_vertical,
    long_put_vertical,
    short_call_vertical,
    long_call_vertical,
    short_strangle,
    long_strangle,
    short_straddle,
    long_straddle,
    short_put,
    long_put,
    short_call,
    long_call,
    short_iron_condor,
    long_iron_condor,
    short_put_butterfly,
    long_put_butterfly,
    short_call_butterfly,
    long_call_butterfly,
    short_put_LT112,
    long_put_LT112,
    short_put_ratio,
    long_put_ratio,
    short_call_ratio,
    long_call_ratio,
    short_call_LT112,
    long_call_LT112,
    none
};    

enum class RightClickMenuLineType {
    none,
    trade_header_line,
    options_leg_line,
    shares_line,
    futures_line
};

enum class QuestionCallback {
    None,
    ExpireSelectedLegs,
    DeleteTransaction,
    ProcessYearEnd,
    AddJournalFolder,
    RenameJournalFolder,
    DeleteJournalFolder,
    PermanentDeleteNote
};


constexpr int CATEGORY_START = 0;
constexpr int CATEGORY_END = 15;
constexpr int CATEGORY_ALL = 99;
constexpr int CATEGORY_OTHER = 100;

constexpr int JOURNAL_NOTES_ALLNOTES_ID = -100;   // All Notes folder id
constexpr int JOURNAL_NOTES_TRASH_ID = -99;       // Trash folder id
constexpr int JOURNAL_NOTES_NOTES_ID = -98;       // Notes folder id

ImU32 clrBackDarkBlack(AppState& state);
ImU32 clrBackDarkGray(AppState& state);
ImU32 clrBackMediumGray(AppState& state);
ImU32 clrBackLightGray(AppState& state);
ImU32 clrTextDarkWhite(AppState& state);
ImU32 clrTextMediumWhite(AppState& state);
ImU32 clrTextLightWhite(AppState& state);
ImU32 clrTextBrightWhite(AppState& state);
ImU32 clrGreen(AppState& state);
ImU32 clrBlue(AppState& state);
ImU32 clrOrange(AppState& state);
ImU32 clrRed(AppState& state);
ImU32 clrMagenta(AppState& state);
ImU32 clrYellow(AppState& state);
ImU32 clrSelection(AppState& state);
ImU32 clrTextSelection(AppState& state);
ImU32 clrCheckBox(AppState& state);
ImU32 clrCheckBoxHover(AppState& state);
ImU32 clrPopupBg(AppState& state);
ImU32 ColorConvertImVec4ToU32(const ImVec4& color);
ImVec4 ColorConvertU32ToFloat4(ImU32 color);
bool ColoredButton(AppState& state, const char* label, const float x_position, 
    const ImVec2& size, const ImU32& text_color, const ImU32& back_color);
bool ColoredSmallButton(AppState& state, const char* label, const float x_position, 
    const ImU32& text_color, const ImU32& back_color);
void TextLabel(AppState& state, const char* label, const float x_position, 
    const ImU32& text_color, const ImU32& back_color);
bool TextLabelCentered(AppState& state, const char* label, const float x_position, 
    const ImVec2& size, const ImU32& text_color, const ImU32& back_color);
bool CheckBox(AppState& state, const char* label, bool* v, 
    const float x_position, const ImU32& text_color, const ImU32& back_color);
bool TextInput(AppState& state, const char* id, std::string* str, ImGuiInputTextFlags flags, 
    const float x_position, const float width, const ImU32& text_color, const ImU32& back_color, ImGuiInputTextCallback callback = NULL);
bool DoubleInput(AppState& state, const char* id, double* v,  
    const float x_position, const float width, const ImU32& text_color, const ImU32& back_color);
bool IntegerInput(AppState& state, const char* id, int* v, const float x_position, const float width, 
            const ImU32& text_color, const ImU32& back_color);
void DialogTitleBar(AppState& state, const char* label, bool& show_dialog_popup, bool& is_popup_open);
void Tooltip(AppState& state, const char* label, const ImU32& text_color, const ImU32& back_color);


class Leg {
public:
    bool         isOpen();                   // method to calc if leg quantity is not zero
    bool         option_data_requested = false;  // Option data already requested
    TickerId     ticker_id           = -1;   // Set when retrieving Option leg data
    int          contract_id         = 0;    // Contract ID received from IBKR 
    int          leg_id              = 0;    // Unique id for this leg within the Trade (see Trade nextleg_id) 
    int          leg_back_pointer_id = 0;    // If transaction is CLOSE, EXPIRE, ROLL this points back to leg where quantity modified
    int          original_quantity   = 0;
    int          open_quantity       = 0;
    std::string  expiry_date         = "";
    std::string  strike_price        = "";
    PutCall      put_call            = PutCall::Nothing;
    Action       action              = Action::Nothing;      // STO,BTO,STC,BTC
    Underlying   underlying          = Underlying::Nothing;  // OPTIONS, STOCKS, FUTURES, DIVIDEND, OTHER
    std::shared_ptr<Transaction> trans = nullptr;            // back pointer to transaction that this leg belongs to

    double calculated_leg_cost       = 0;    // refer to CalculateLegCosting(). Alternative for position_cost.

    double position_cost             = 0;    // ACB as calculated CalculateLegCosting()
    double market_value              = 0;    // real time data receive via updatePortfolio
    double percentage                = 0;    // real time data receive via updatePortfolio
    double unrealized_pnl            = 0;    // real time data receive via updatePortfolio
};


class Transaction {
public:
    Underlying    underlying  = Underlying::Nothing;   // OPTIONS,STOCKS,FUTURES,DIVIDEND
    std::string   description = "";                    // Iron Condor, Strangle, Roll, Expired, Closed, Exercised, etc
    std::string   trans_date  = "";                    // YYYY-MM-DD
    int           quantity    = 0;
    double        price       = 0;
    double        multiplier  = 0;
    double        fees        = 0;
    double        total       = 0;
    double        share_average_cost = 0;
    Action        share_action = Action::BTO;

    std::vector<std::shared_ptr<Leg>> legs;            //  pointer list for all legs in the transaction
};


class SharesHistory {
public:
    std::shared_ptr<Transaction> trans;
    Action       leg_action    = Action::Nothing;
    int          open_quantity = 0;
    double       average_cost  = 0;
};


class Trade {
public:
    bool          is_open            = true; // false if all legs are closed
    TickerId      ticker_id          = -1;
    std::string   ticker_symbol      = "";
    std::string   ticker_name        = "";
    std::string   future_expiry      = "";   // YYYYMM of Futures contract expiry
    std::string   notes              = "";     
    int           category           = 0;    // Category number
    int           nextleg_id         = 0;    // Incrementing counter that gets unique ID for legs being generated in TransDetail.    
    bool          warning_3_dte      = true;      
    bool          warning_21_dte     = false;      

    int           aggregate_shares   = 0;    // Calculated from all transactions roll-up
    int           aggregate_futures  = 0;    // Calculated from all transactions roll-up
    double        acb_total          = 0;    // adjusted cost base of entire trade (shares + non-shares items)
    double        acb_shares         = 0;    // adjusted cost base for shares/futures (may include/exclude costs like dividends)
    double        acb_non_shares     = 0;    // all non-shares items (dividends, options, etc)
    double        total_share_profit = 0;    // total shares/futures profit/loss (total income less total average costs for all shares transactions in the trade)
    double        trade_bp           = 0;    // Buying Power for the entire trade 
    double        multiplier         = 0;    // Retrieved from Transaction and needed for updatePortfolio real time calculations

    double        ticker_last_price  = 0;
    double        ticker_close_price = 0;
    int           ticker_decimals    = 2;    // upated via data from Config. 

    // Vector holding all the data related to how shares/futures are allocated during a buy/sell. This
    // vector is created during the CalculateAdjustedCostBase method. We use this vector when displaying
    // the Trade history.
    std::vector<SharesHistory> shares_history;
    void AddSharesHistory(std::shared_ptr<Transaction> trans, Action leg_action, int open_quantity, double average_cost);
    void CalculateTotalSharesProfit(AppState& state);

    // The earliest DTE from the Legs of the Trade are calculated in the SetTradeOpenStatus() function
    // and is used when displaying ActiveTrades with the ExpiryDate filter set.
    int earliest_legs_DTE = 9999999;

    // The following are string representations of the marketdata and updatePortfolio values. We save them here
    // so that the Active Trades lists gets visually updated immediately after a new Trade or close trade.
    // The ticker data would not be updated until a new price occurs so we simply display the most recent price
    // rather than blank text.
    std::string itm_text = "";
    ImU32 itm_color = 0;

    // Save the Trade's percentage complete so that it can be used for sorting
    // when the application is connected to TWS.
    double trade_completed_percentage = 0;

    std::string ticker_column_1;
    std::string ticker_column_2;
    std::string ticker_column_3;
    
    ImU32 ticker_column_1_clr{};
    ImU32 ticker_column_2_clr{};
    ImU32 ticker_column_3_clr{};

    // Dates used to calculate ROI on TradeBP.
    std::string  bp_start_date = "99999999";            // YYYYMMDD  First transaction date
    std::string  bp_end_date = "00000000";              // YYYYMMDD  Last trans expiry date or trade close date if earlier) 
    std::string  oldest_trade_trans_date = "00000000";  // If Trade is closed then this trans will be the BPendDate

    std::vector<std::shared_ptr<Transaction>> transactions;     // pointer list for all transactions in the trade
    std::vector<std::shared_ptr<Leg>> open_legs;                // sorted list of open legs for this trade

    void SetTradeOpenStatus();
    void CalculateAdjustedCostBase(AppState& state);
    void CreateOpenLegsVector();
};


class CDatabase {
public:
    std::string dbFilename;;
    std::string dbJournalNotes;

    bool is_previously_loaded = false;

    // Pointer list for all trades (initially loaded from database)
    std::vector<std::shared_ptr<Trade>> trades;

    std::string PutCallToString(const PutCall e);
    PutCall StringToPutCall(const std::string& text);

    Underlying StringToUnderlying(const std::string& underlying);
    std::string UnderlyingToString(const Underlying e);

    Action StringToAction(const std::string& action);
    Action StringDescriptionToAction(const std::string& action);
    std::string ActionToString(const Action e);
    std::string ActionToStringDescription(const Action e);

    bool LoadDatabase(AppState& state);
    bool SaveDatabase(AppState& state);

    CDatabase();

};


class CConfig {
public:
    std::string dbConfig;

    int startup_width = 0;  
    int startup_height = 0;
    int startup_right_panel_width = 0;

    bool display_open_source_license = true;
    bool show_portfolio_value = true;
    bool allow_update_check = true;
    bool exclude_nonstock_costs = false;
    bool show_45day_trade_date = true;

    std::string label_45day_trade_date;

    ColorThemeType color_theme = ColorThemeType::Dark;
    NumberFormatType number_format_type = NumberFormatType::American;
    CostingMethodType costing_method = CostingMethodType::AverageCost;
    StartWeekdayType start_weekday = StartWeekdayType::Monday;

    std::unordered_map<int, std::string> mapCategoryDescriptions{
        { 0, "Category 0"},
        { 1, "Category 1" },
        { 2, "Category 2" },
        { 3, "Category 3" },
        { 4, "Category 4" },
        { 5, "Category 5" },
        { 6, "Category 6" },
        { 7, "Category 7" },
        { 8, "Category 8" },
        { 9, "Category 9" },
        { 10, "Category 10" },
        { 11, "Category 11" },
        { 12, "Category 12" },
        { 13, "Category 13" },
        { 14, "Category 14" },
        { 15, "Category 15" },
        { 99, "All Categories" },
        {100, "Other Income/Expense" }
    };

    std::unordered_map<std::string, std::string> mapFuturesExchanges{
        { "/AUD", "CME" },
        { "/EUR", "CME" },
        { "/GBP", "CME" },
        { "/ES",  "CME" },
        { "/MES", "CME" },
        { "/HE",  "CME" },
        { "/LE",  "CME" },
        { "/GC",  "COMEX" },
        { "/HG",  "COMEX" },
        { "/NG",  "NYMEX" },
        { "/CL",  "NYMEX" },
        { "/MCL", "NYMEX" },
        { "/ZN",  "CBOT" },
        { "/ZB",  "CBOT" },
        { "/ZC",  "CBOT" },
        { "/ZS",  "CBOT" }
    };

    std::unordered_map<std::string, std::string> mapMultipliers{
        { "/AUD", "100000" },
        { "/EUR", "125000" },
        { "/GBP", "62500" },
        { "/ES",  "50" },
        { "/MES", "5" },
        { "/CL",  "1000" },
        { "/HE",  "400" },
        { "/LE",  "400" },
        { "/HG",  "25000" },
        { "/ZN",  "1000" },
        { "/ZB",  "1000" }
    };

    std::unordered_map<std::string, int> mapTickerDecimals{
        { "/AUD", 5 },
        { "/EUR", 5 },
        { "/GBP", 4 },
        { "/ZC", 3 },
        { "/LE", 3 },
        { "/HE", 4 },
        { "/ZN", 3 },
        { "/ZB", 3 },
        { "/ZS", 4 }
    };
    
    bool SaveConfig(AppState& state);
    bool LoadConfig(AppState& state);

    int GetStartupWidth(AppState& appstate);
    int GetStartupHeight(AppState& appstate);
    void SetStartupHeight(AppState& appstate, int height);
    void SetStartupWidth(AppState& appstate, int width);
    int GetStartupRightPanelWidth(AppState& appstate);
    void SetStartupRightPanelWidth(AppState& appstate, int width);

    void DisplayLicense();
    int GetTickerDecimals(const std::string& underlying);
    void SetTickerDecimals(const std::string& underlying, int decimals);
    std::string GetMultiplier(const std::string& wunderlying);
    void SetMultiplier(const std::string& wunderlying, const std::string& multiplier);
    std::string GetFuturesExchange(const std::string& underlying);
    void SetFuturesExchange(const std::string& underlying, const std::string& exchange);
    std::string GetCategoryDescription(int index);
    void SetCategoryDescription(int index, const std::string& description);
    bool IsFuturesTicker(const std::string& ticker);

    CConfig();
};


struct AppState {
    CDatabase db{};
    CConfig config{};

    std::string version_current_display = "5.00";
    std::string version_available_display = "";

    int ticker_id = 1;    // incrementing counter for requesting market data

    float dpi_scale = 1;
    int display_width = 0;
    int display_height = 0;

    ImFont* gui_font = nullptr;
    ImFont* gui_font_mono = nullptr;

    bool show_activetrades = true;
    bool show_closedtrades = false;
    bool show_transpanel = false;
    bool show_transedit = false;
    bool show_tradehistory = true;
    bool show_journalnotes = false;
    bool show_reconciliation_popup = false;
    bool show_messagebox_popup = false;
    bool show_questionbox_popup = false;
    bool show_gettext_popup = false;
    bool show_assignment_popup = false;
    bool show_settingsdialog_popup = false;
    bool show_yearenddialog_popup = false;
    bool show_categoriesdialog_popup = false;
    bool show_importdialog_popup = false;
    bool show_tradedialog_popup = false;
    bool show_activetrades_rightclickmenu = false;
    bool show_journalfolders_rightclickmenu = false;
    bool show_journalnotes_rightclickmenu = false;
    bool show_connect_rightclickmenu = false;
    
    bool is_activetrades_data_loaded = false;
    bool is_closedtrades_data_loaded = false;
    bool is_transactions_data_loaded = false;
    bool is_transedit_data_loaded = false;
    bool is_tradehistory_data_loaded = false;
    bool is_journalnotes_data_loaded = false;

    bool is_paper_trading = false;

    // Pause thread updating active market prices in order to avoid
    // pointer issues should a transaction be updated/deleted causing
    // a stale pointer.
    bool is_pause_market_data = false;

    int tradefilter_current_item = 0;    // Index of the current selected item
    int datefilter_current_item = 0;     // Index of the current selected item
    int newtrade_current_item = 0;       // Index of the current selected item
    int strategy_current_item = 0;       // Index of the current selected item
    int category_current_item = 0;       // Index of the current selected item
    int manage_current_item = 0;         // Index of the current selected item

    ActiveTradesFilterType activetrades_filter_type = ActiveTradesFilterType::Category;
    // Can not include CListPanelData because we will get a circular reference that can
    // not be resolved using forward declares. Use a void pointer instead cast it as needed.
    // std::vector<CListPanelData>* vec = static_cast<std::vector<CListPanelData>*>(state.vecActiveTrades);
    // CListPanelData* ld = &vec->at(index_trade);
    void* vecActiveTrades;
    RightClickMenuLineType activetrades_rightclickmenu_linetype = RightClickMenuLineType::none;
    bool activetrades_rightclickmenu_multiselected = false;
    std::vector<std::shared_ptr<Leg>> activetrades_selected_legs;
    int activetrades_rightclickmenu_shares_count = 0;
    int activetrades_rightclickmenu_futures_count = 0;
    bool activetrades_rightclickmenu_shares_exist = false;
    bool activetrades_rightclickmenu_futures_exist = false;
    bool activetrades_rightclickmenu_options_exist = false;
    std::shared_ptr<Leg> activetrades_rightclickmenu_assignment_leg = nullptr;
    int activetrades_current_row_index = -1;   // used when positioning selected row after a reload

    std::shared_ptr<Trade> activetrades_selected_trade = nullptr;
    std::shared_ptr<Trade> closedtrades_selected_trade = nullptr;
    std::shared_ptr<Trade> transactions_selected_trade = nullptr;
    std::shared_ptr<Trade> trade_dialog_trade = nullptr;
    std::shared_ptr<Transaction> trade_dialog_transaction = nullptr;
    std::shared_ptr<Trade> trans_edit_trade = nullptr;
    std::shared_ptr<Transaction> trans_edit_transaction = nullptr;
    std::shared_ptr<Trade> tradehistory_trade = nullptr;

    // Same circular reference issue with Client (TwsClient)
    void* client;

    std::atomic<bool> is_monitor_thread_active = false;
    std::atomic<bool> is_ping_thread_active = false;
    std::atomic<bool> is_ticker_update_thread_active = false;
    std::atomic<bool> is_checkforupdate_thread_active = false;

    std::atomic<bool> stop_monitor_thread_requested = false;
    std::atomic<bool> stop_ping_thread_requested = false;
    std::atomic<bool> stop_ticker_update_thread_requested = false;

    std::thread monitoring_thread;
    std::thread ticker_update_thread;
    std::thread ping_thread;
    std::thread check_for_update_thread;

    std::string year_to_close;

    std::string tradehistory_ticker = "";
    std::string tradehistory_notes = "";
    bool tradehistory_notes_modified = false;

    std::string journalnotes_notes = "";
    bool journalnotes_notes_modified = false;
    std::vector<JournalFolder> vector_folders_top_panel;
    std::vector<JournalFolder> vector_folders;
    std::vector<JournalNote> vector_notes;     // notes loaded from file
    int journal_folders_selected_id = JOURNAL_NOTES_ALLNOTES_ID;
    int journal_notes_selected_id = -1;
    int journal_notes_next_id = 0;      // incrementing to provide unique id's for folders & notes
    int current_vector_index_top_panel = 0;
    int current_vector_index_bottom_panel = -1;

    std::string id_reconciliation_popup = "Reconciliation_Popup";
    std::string reconciliation_results_text = "";

    std::string gettextpopup_caption = "";
    std::string gettextpopup_message = "";
    std::string gettextpopup_result_string = "";
    QuestionCallback gettextpopup_callback = QuestionCallback::None;

    bool gettextpopup_cancelled = false;

    std::string messagebox_caption = "";
    std::string messagebox_message = "";

    std::string questionbox_caption = "";
    std::string questionbox_message = "";
    bool questionbox_display_ok = false;
    QuestionCallback questionbox_callback = QuestionCallback::None;

    std::string filterpanel_start_date = "1999-01-01";
    std::string filterpanel_end_date = "9999-12-31";
    std::string filterpanel_ticker_symbol = "";
    int filterpanel_selected_category = CATEGORY_ALL;

    std::string id_settingsdialog_popup = "Settings";
    std::string id_yearenddialog_popup = "YearEnd";
    std::string id_categoriesdialog_popup = "Categories";
    std::string id_assignment_popup = "Assignment";
    std::string id_importdialog_popup = "Import";

    std::string id_tradedialog_popup = "Trade Management";
    TradeAction trade_action = TradeAction::no_action;
    TradeActionOptionType trade_action_option_type = TradeActionOptionType::none;

    float bottom_panel_height = 0;
    float right_panel_width = 0;
    float top_panel_height = 0;
    float left_panel_width = 0;
    float client_width = 0;
    float client_height = 0;

    TabPanelItem selected_tabpanelitem = TabPanelItem::ActiveTrades;

    float dpi(const float value) {return (float)(value * dpi_scale);}
    int   dpi(const int value) {return (int)(value * dpi_scale);}
    float undpi(const float value) {return (float)(value / dpi_scale);}
    int   undpi(const int value) {return (int)(value / dpi_scale);}

    bool is_modal_active() {
        if (show_assignment_popup) return true;
        if (show_settingsdialog_popup) return true;
        if (show_yearenddialog_popup) return true;
        if (show_categoriesdialog_popup) return true;
        if (show_tradedialog_popup) return true;
        if (show_gettext_popup) return true;
        return false;
    }
};


#endif  // APPSTATE_H
