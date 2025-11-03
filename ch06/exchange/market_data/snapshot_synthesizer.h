#pragma once
#include "common/types.h"
#include "common/thread_utils.h"
#include "common/lf_queue.h"
#include "common/macros.h"
#include "common/mcast_socket.h"
#include "common/mem_pool.h"
#include "common/logging.h"

#include "market_data/market_update.h"
#include "matcher/me_order.h"

namespace Exchange
{
    class SnapshotSynthesizer {
        private:
            MDPMarketUpdateLFQueue* snapshot_md_updates_ = nullptr;
            Common::Logger logger_;
            volatile bool run_;
            std::string time_str_;
            Common::McastSocket snapshot_socket_;
            std::array<std::array<MEMarketUpdate*, ME_MAX_ORDER_IDS>, ME_MAX_TICKERS> ticker_orders_;
            size_t last_inc_seq_num_ = 0;
            Nanos last_snapshot_time_ = 0;
            MemPool<MEMarketUpdate> order_pool_;
        
        public:
            SnapshotSynthesizer(MDPMarketUpdateLFQueue* market_updates, const std::string& iface, const std::string& snapshot_ip, int snapshot_port);
            ~SnapshotSynthesizer();

             auto start() {
                run_ = true;
                ASSERT(Common::createAndStartThread(-1, "Exchange/SnapshotSynthesizer", [this]() { run(); }) != nullptr, "Failed to start SnapshotSynthesizer thread.");
            }

            auto stop() -> void {
                run_ = false;
            }

            auto run() -> void;

            auto addToSnapshot(const MDPMarketUpdate* market_update) -> void;
            auto publishSnapshot() -> void;

            // deleted copy & move constructors and assignment-operators
            SnapshotSynthesizer() = default;
            SnapshotSynthesizer(const SnapshotSynthesizer&) = delete;
            SnapshotSynthesizer(const SnapshotSynthesizer&&) = delete;
            SnapshotSynthesizer &operator=(const SnapshotSynthesizer&) = delete;
            SnapshotSynthesizer &operator=(const SnapshotSynthesizer&&) = delete;
    };
} // namespace Exchange
