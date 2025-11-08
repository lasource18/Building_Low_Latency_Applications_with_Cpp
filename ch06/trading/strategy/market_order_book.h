#pragma once

#include "common/types.h"
#include "common/mem_pool.h"
#include "common/logging.h"

#include "market_order.h"
#include "exchange/market_data/market_update.h"

namespace Trading
{
    class TradeEngine;

    class MarketOrderBook final {
        public:
            MarketOrderBook(TickerId ticker_id, Logger* logger);
            ~MarketOrderBook();
        
        private:
            const TickerId ticker_id_;
            TradeEngine* trade_engine_;
            OrderHashMap oid_to_order_;
            MemPool<MarketOrdersAtPrice> order_at_price_pool_;
            MarketOrdersAtPrice* bids_by_price_ = nullptr;
            MarketOrdersAtPrice* asks_by_price_ = nullptr;
            OrdersAtPriceHashMap price_orders_at_price_;
            MemPool<MarketOrder> order_pool_;
            BBO bbo_;
            std::string time_str_;
            Logger* logger_ = nullptr;
    };

    typedef std::array<MarketOrderBook*, ME_MAX_TICKERS> MarketOrderBookHashMap;
} // namespace Trading
