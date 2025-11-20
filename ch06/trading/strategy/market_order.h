#pragma once

#include <array>
#include <sstream>
#include "common/types.h"

using namespace Common;

namespace Trading
{
    // Used by the matching engine to rpz a single limit order book
    struct MarketOrder
    {
        OrderId order_id_ = OrderId_INVALID;
        Side side_ = Side::INVALID;
        Price price_ = Price_INVALID;
        Qty qty_ = Qty_INVALID;
        Priority priority_ = Price_INVALID;

        // MarketOrder also serves as a node in doubly linked list of all orders at price levels arranged in FIFO order
        MarketOrder *prev_order_ = nullptr;
        MarketOrder *next_order_ = nullptr;

        // only needed for use with MemPool
        MarketOrder() = default;

        MarketOrder(OrderId order_id, Side side, Price price, Qty qty, Priority priority,
            MarketOrder *prev_order, MarketOrder *next_order) noexcept
            : order_id_(order_id),
              side_(side),
              price_(price),
              qty_(qty),
              priority_(priority),
              prev_order_(prev_order),
              next_order_(next_order)
            {}
            auto toString() const -> std::string;
    };
    
    // HashMap from OrderId -> MarketOrder
    typedef std::array<MarketOrder*, ME_MAX_ORDER_IDS> OrderHashMap;

    // HashMap from ClientId -> OrderId -> MarketOrder
    typedef std::array<OrderHashMap, ME_MAX_NUM_CLIENTS> ClientOrderHashMap;

    // Used by the matching engine to represent a price level in the limit order book
    // Internally maintains a list of MarketOrder objects arranged in FIFO order
    struct MarketOrdersAtPrice {
        Side side_ = Side::INVALID;
        Price price_ = Price_INVALID;

        MarketOrder *first_mkt_order_ = nullptr;

        // MarketOrdersAtPrice also serve as a node in a doubly linked list of price levels arranged from the most aggressive to least aggressive price
        MarketOrdersAtPrice *prev_entry_ = nullptr;
        MarketOrdersAtPrice *next_entry_ = nullptr;

        // Only needed for use with the MemPool
        MarketOrdersAtPrice() = default;

        MarketOrdersAtPrice(Side side, Price price, MarketOrder *first_me_order, MarketOrdersAtPrice *prev_entry, MarketOrdersAtPrice *next_entry)
            : side_(side), price_(price), first_mkt_order_(first_me_order), prev_entry_(prev_entry), next_entry_(next_entry) {}
        
        auto toString() const {
            std::stringstream ss;
            ss << "MarketOrdersAtPrice["
               << "side:" << sideToString(side_) << " " 
               << "price:" << priceToString(price_) << " "
               << "first_me_order:" << (first_mkt_order_ ? first_mkt_order_->toString() : "null") << " "
               << "prev:" << (prev_entry_? prev_entry_->price_ : Price_INVALID) << " "
               << "next:" << (next_entry_? next_entry_->price_ : Price_INVALID) << " ]";

               return ss.str();
        }
    };

    // HashMap from Price -> MarketOrders
    typedef std::array<MarketOrdersAtPrice*, ME_MAX_PRICE_LEVELS> OrdersAtPriceHashMap;

    struct BBO
    {
        Price bid_price_ = Price_INVALID, ask_price_ = Price_INVALID;
        Qty bid_qty_ = Qty_INVALID, ask_qty_ = Qty_INVALID;

        auto toString() const {
            std::stringstream ss;
            ss << "BBO{"
               << qtyToString(bid_qty_) << "@" << priceToString(bid_price_)
               << "X"
               << qtyToString(ask_qty_) << "@" << priceToString(ask_price_)
               << "}";

            return ss.str();
        }
    };
    
} // namespace Exchange
