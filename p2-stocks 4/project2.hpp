//0E04A31E0D60C01986ACB20081C9D8722A1899B6
#include <string>
#include <queue>
#include <vector>
#include <sstream>
#include "P2random.h"
#include <iostream>
using namespace std;

//timestatus enum for time traveller output
enum TimeStatus{ 
    INITIAL, 
    POTENTIAL, 
    COMPLETED 
};

//order class
class Order{
    public:
    int64_t trader_id;
    int64_t stock_id;
    bool is_buyer;
    int64_t price_limit;
    int64_t quantity;
    int64_t timestamp;
    int64_t order_id;


    //constructor
    Order(int64_t ti, int64_t si, bool ib, int64_t pl, int64_t q, int64_t ts)
    : trader_id(ti), stock_id(si), is_buyer(ib), price_limit(pl), quantity(q), timestamp(ts){
    }
};


// functor for buyer
struct buyer_functor{
    bool operator()(const Order &o1, const Order &o2) const{
        if (o1.price_limit == o2.price_limit) {
            return o1.order_id > o2.order_id;  
        }
        return o1.price_limit < o2.price_limit;
    }
};

//functor for seller
struct seller_functor{
    bool operator()(const Order &o1, const Order &o2) const{
        if (o1.price_limit == o2.price_limit) {
            return o1.order_id > o2.order_id;  
        }
        return o1.price_limit > o2.price_limit;
    }
};

//stock struct
struct Stock{
    priority_queue<Order, vector<Order>, buyer_functor> pq_buy;
    priority_queue<Order, vector<Order>, seller_functor> pq_sell;
    priority_queue<int64_t, vector<int64_t>, greater<int64_t> > upper;
    priority_queue<int64_t, vector<int64_t>, less<int64_t> > lower;
    vector<int64_t> timestamps; 
    vector<int64_t> prices;      

    int64_t get_median();
    void update_time_traveler(int64_t &buy_time, int64_t &sell_time);
    void update_median(int64_t match_price, int64_t timestamp); 
};

//trader struct
struct Trader{  
    int64_t stocks_bought=0;
    int64_t stocks_sold=0;
    int64_t net_transfer=0;
};

//Market struct
struct Market{
    bool verbose_specified = false;
    bool median_specified = false;
    bool trader_info_specified = false;
    bool time_travelers_specified = false;
    int64_t num_traders;
    int64_t num_stocks;
    vector<Stock> all_stocks;
    vector<Trader> all_traders;
    int64_t comp_trades = 0;
    int64_t curr_timestamp = 0; 
    int64_t order_id_counter = 0;
    
    void read_input();
    void match(int64_t stock_id);
    void processOrders(istream &inputStream);
    void verbose_output(Order &buy, Order &sell, int64_t quantity, int64_t match_price);
    void trader_output();
    void median_output();
    void time_traveler_output();
    void summary_output();
    void parse_command_line(int argc, char*argv[]);
};