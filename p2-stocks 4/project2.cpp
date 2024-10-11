//0E04A31E0D60C01986ACB20081C9D8722A1899B6
#include "project2.hpp"
#include <getopt.h>
using namespace std;

int main(int argc, char *argv[]) {
    Market market;
    market.parse_command_line(argc, argv);
    market.read_input();
    if(market.median_specified){
        market.median_output();
    }

    market.summary_output();
    if(market.trader_info_specified){
        market.trader_output();
    }
}

//def works
void Market::parse_command_line(int argc, char*argv[]){
    opterr = false;
    int choice;
    int index = 0;
    option long_options[]={
        {"verbose", no_argument, nullptr, 'v'},
        {"median", no_argument, nullptr, 'm'},
        {"trader_info", no_argument, nullptr, 'i'},
        {"time_travelers", no_argument, nullptr, 't'},
        {nullptr,0,nullptr,'\0'},
    };
    while((choice = getopt_long(argc, argv, "vmit", long_options, &index)) !=-1){
        switch(choice){
            case 'v':
                verbose_specified = true;
                break;
            case 'm':
                median_specified = true;
                break;
            case 'i': 
                trader_info_specified = true;
                break;
            case 't':
                time_travelers_specified = true;
                break;
            default:
                cerr<<"invalid output mode"<<'\n';
                exit(1);
        }
    }
}

//def works for TL
void Market:: read_input(){
    cout<<"Processing orders..."<< '\n';
    //read in comment
    string junk, input_mode;
    getline(cin, junk);
    string mode_label, num_traders_label, num_stocks_label;
    //read in input mode, num traders, num stocks
    cin >> mode_label >> input_mode;        
    cin >> num_traders_label >> num_traders; 
    cin >> num_stocks_label >> num_stocks;   
    all_traders.resize(static_cast<size_t>(num_traders));
    all_stocks.resize(static_cast<size_t>(num_stocks));
    stringstream ss;

    if (input_mode == "PR") {
        int64_t random_seed, num_orders, arrival_rate;
        cin >> random_seed >> num_orders >> arrival_rate;

        P2random::PR_init(ss, static_cast<unsigned int>(random_seed), 
        static_cast<unsigned int>(num_traders),
        static_cast<unsigned int>(num_stocks),
        static_cast<unsigned int>(num_orders),
        static_cast<unsigned int>(arrival_rate));
        processOrders(ss);
    } 
    else {
        processOrders(cin);
    }
}

//def works for TL 
void Market::processOrders(istream &inputStream){
    int64_t timestamp, trader_id, stock_id, price, quantity;
    string buy_sell;
    bool is_buyer = false;
    char trader, stock, p, q;

   while(inputStream>>timestamp>>buy_sell>>trader>>trader_id>>stock>>stock_id
    >>p>>price>>q>>quantity){
        
        if (timestamp < 0) {
            cerr << "timestamp cannot be negative" << '\n';
            exit(1);
        }
        if(timestamp<curr_timestamp){
            cerr << "timestamps must be non-decreasing" << '\n';
            exit(1);
        }
        if(trader_id<0 || trader_id>=num_traders || stock_id<0 || 
        stock_id>=num_stocks || price<=0 || quantity<=0){
            cerr << "invalid input for trader_id, stock_id, price, or quantity"<<'\n';
            exit(1);
        }
        if(buy_sell=="BUY"){
            is_buyer = true;
        }
        else if (buy_sell == "SELL") {
            is_buyer = false;
        }
        Order order(trader_id, stock_id, is_buyer, price, quantity, timestamp);
        order.order_id = order_id_counter++;
        //median output
        if(timestamp!=curr_timestamp){
            if(median_specified){
                median_output();  
            }
            curr_timestamp = timestamp;
        }

        if(is_buyer){
            all_stocks[static_cast<size_t>(stock_id)].pq_buy.push(order);
        }
        else{
            all_stocks[static_cast<size_t>(stock_id)].pq_sell.push(order);
        }
        match(stock_id);
    }
}

//matches orders
void Market::match(int64_t stock_id){
    auto &pq_buy = all_stocks[static_cast<size_t>(stock_id)].pq_buy;
    auto &pq_sell = all_stocks[static_cast<size_t>(stock_id)].pq_sell;
    while (!pq_buy.empty() && !pq_sell.empty()) {
        Order buy = pq_buy.top();
        Order sell = pq_sell.top();

        int64_t match_quantity;
        int64_t match_price;

        if (buy.price_limit >= sell.price_limit){
            if(buy.quantity<sell.quantity){
                match_quantity = buy.quantity;
            }
            else{
                match_quantity = sell.quantity;
            }

            if(buy.order_id<sell.order_id){
                match_price = buy.price_limit;
            }
            else{
                match_price = sell.price_limit;
            }

            if(verbose_specified){
                verbose_output(buy, sell, match_quantity, match_price);
            }
            comp_trades++;
            all_traders[static_cast<size_t>(buy.trader_id)].stocks_bought += match_quantity;
            all_traders[static_cast<size_t>(sell.trader_id)].stocks_sold += match_quantity;
            //multiply because number of stocks matter not just price
            all_traders[static_cast<size_t>(buy.trader_id)].net_transfer -= match_quantity * match_price;  
            all_traders[static_cast<size_t>(sell.trader_id)].net_transfer += match_quantity * match_price;  

            if(buy.quantity>match_quantity){
                buy.quantity-=match_quantity;
                pq_buy.pop();
                pq_buy.push(buy);
            }
            else{
                pq_buy.pop();
            }
            if(sell.quantity>match_quantity){
                sell.quantity-=match_quantity;
                pq_sell.pop();
                pq_sell.push(sell);
            }
            else{
                pq_sell.pop();
            }

            //updating the median here
             all_stocks[static_cast<size_t>(stock_id)].update_median(match_price, buy.timestamp);
        }
        else{
            break;
        }
    }
}

//summary output
void Market::summary_output() {
    // Print end of day summary with total number of trades completed
    cout << "---End of Day---" << '\n';
    cout << "Trades Completed: " << comp_trades << '\n';
}

//verbose output
void Market::verbose_output(Order &buy, Order &sell, int64_t quantity, int64_t match_price) {
    cout << "Trader " << buy.trader_id << " purchased " << quantity 
    << " shares of Stock " << buy.stock_id 
    << " from Trader " << sell.trader_id << " for $" << match_price 
    << "/share" << '\n';
}

//trader info output
void Market::trader_output(){
     cout << "---Trader Info---" << '\n';
     for (int64_t i = 0; i < num_traders; ++i) {
        Trader &t = all_traders[static_cast<size_t>(i)];
        cout << "Trader " << i << " bought " << t.stocks_bought 
        << " and sold " << t.stocks_sold 
        << " for a net transfer of $" << t.net_transfer << '\n';
    }
}


//update median
void Stock::update_median(int64_t match_price, int64_t timestamp){
    prices.push_back(match_price);
    timestamps.push_back(timestamp);
    if (lower.empty() || match_price <= lower.top()) {
        lower.push(match_price);
    } 
    else {
        upper.push(match_price);
    }
    if (lower.size() > upper.size() + 1) {
        upper.push(lower.top());
        lower.pop();
    } 
    else if (upper.size() > lower.size()) {
        lower.push(upper.top());
        upper.pop();
    }
}

//get median
int64_t Stock::get_median(){
    if(lower.size()>upper.size()){
        return lower.top();
    }
    else{
       return (lower.top()+upper.top())/2;
    }
}

//median output
void Market::median_output() {
    for (int64_t i = 0; i < num_stocks; ++i) {
        if (!all_stocks[static_cast<size_t>(i)].prices.empty()) {
            int64_t median_price = all_stocks[static_cast<size_t>(i)].get_median();
            cout << "Median match price of Stock " << i
            << " at time " << curr_timestamp 
            << " is $" << median_price << '\n';
        }
    }
}
