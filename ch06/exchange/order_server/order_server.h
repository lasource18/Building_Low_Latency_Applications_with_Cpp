#pragma once

#include <functional>

#include "common/thread_utils.h"
#include "common/macros.h"
#include "common/tcp_server.h"

#include "order_server/client_request.h"
#include "order_server/client_response.h"
#include "order_server/fifo_sequencer.h"

namespace Exchange
{
    class OrderServer {
        private:
            const std::string iface_;
            const int port_ = 0;
            ClientResponseLFQueue* outgoing_responses_ = nullptr;
            volatile bool run_ = false;
            std::string time_str_;
            Logger logger_;

            // Hasmap from ClientId to the next seq number to be sent on outgoing client
            std::array<size_t, ME_MAX_NUM_CLIENTS> cid_next_outgoing_seq_num_;
            // Hash map from Cilent id -> next seq number expected on incomming client requests
            std::array<size_t, ME_MAX_NUM_CLIENTS> cid_next_exp_seq_num_;
            // Hash map from ClientId -> TCP socket / client connection
            std::array<Common::TCPSocket*, ME_MAX_NUM_CLIENTS> cid_tcp_socket_;
            // TCP server instance listening for new client connections
            TCPServer tcp_server_;
            // FIFO Sequencer responsible for ensuring incoming client requests are processed in the order in which they are received
            FIFOSequencer fifo_sequencer_;
            
        public:
            OrderServer(ClientRequestLFQueue* client_requests, ClientResponseLFQueue* client_responses, const std::string& iface, int port);
            ~OrderServer();

            auto start() -> void;
            auto stop() -> void;

            auto run() noexcept {
                logger_.log("%:% %() %\n", __FILE__, __LINE__, __FUNCTION__, Common::getCurrentTimeStr(&time_str_));
                while (run_)
                {
                    tcp_server_.poll();
                    tcp_server_.sendAndRecv();

                    for(auto client_response = outgoing_responses_->getNextToRead(); outgoing_responses_->size() && client_response; client_response = outgoing_responses_->getNextToRead()) {
                        auto& next_outgoing_seq_num = cid_next_outgoing_seq_num_[client_response->client_id_];
                        logger_.log("%:% %() % Processing cid:% seq:% %\n", __FILE__, __LINE__, __FUNCTION__, Common::getCurrentTimeStr(&time_str_),
                        client_response->client_id_, next_outgoing_seq_num, client_response->toString());

                        ASSERT(cid_tcp_socket_[client_response->client_id_] != nullptr,
                            "Don\'t have a TCPSocket for ClientId:" + std::to_string(client_response->client_id_));
                        cid_tcp_socket_[client_response->client_id_]->send(&next_outgoing_seq_num, sizeof(next_outgoing_seq_num));
                        cid_tcp_socket_[client_response->client_id_]->send(client_response, sizeof(MEClientResponse));

                        outgoing_responses_->updateReadIndex();

                        ++next_outgoing_seq_num;
                    }
                }
                
            };
            
            // Callback methods for TCP server
            auto recvCallback(Common::TCPSocket* socket, Common::Nanos rx_time) noexcept {
                logger_.log("%:% %() % Received socket:% len:% rx:%\n", __FILE__, __LINE__, __FUNCTION__, Common::getCurrentTimeStr(&time_str_),
                socket->socket_fd_, socket->next_rcv_valid_index_, rx_time);

                if(socket->next_rcv_valid_index_ >= sizeof(OMClientRequest)) {
                    size_t i = 0;
                    for(; i + sizeof(OMClientRequest) <= socket->next_rcv_valid_index_; i += sizeof(OMClientRequest)) {
                        auto request = reinterpret_cast<const OMClientRequest*>(socket->inbound_data_.data() + i);
                        logger_.log("%:% %() % Received % \n", __FILE__, __LINE__, __FUNCTION__, Common::getCurrentTimeStr(&time_str_), request->toString());

                        if(UNLIKELY(cid_tcp_socket_[request->me_client_request_.client_id_] == nullptr)) { // first message back to the client
                            cid_tcp_socket_[request->me_client_request_.client_id_] = socket;
                        }

                        if(cid_tcp_socket_[request->me_client_request_.client_id_] != socket) { // TODO - change this to send a reject back to the client
                            logger_.log("%:% %() % Received ClientRequest from ClientId:% on different socket:% expected:%", 
                                __FILE__, __LINE__, __FUNCTION__, Common::getCurrentTimeStr(&time_str_), request->me_client_request_.client_id_, socket->socket_fd_,
                            cid_tcp_socket_[request->me_client_request_.client_id_]->socket_fd_);
                            continue;
                        }

                        auto& next_exp_seq_num = cid_next_exp_seq_num_[request->me_client_request_.client_id_];
                        if(request->seq_num_ != next_exp_seq_num) { // TODO - change this to send a reject back to the client
                            logger_.log("%:% %() Incorrect sequence number. ClientId:% SeqNum expected:% received:%", __FILE__, __LINE__, __FUNCTION__,
                            Common::getCurrentTimeStr(&time_str_), request->me_client_request_.client_id_, next_exp_seq_num, request->seq_num_);
                            continue;
                        }

                        ++next_exp_seq_num;

                        fifo_sequencer_.addClientRequest(rx_time, request->me_client_request_);
                    }
                    memcpy(socket->inbound_data_.data(), socket->inbound_data_.data() + i, socket->next_rcv_valid_index_ - i);
                    socket->next_rcv_valid_index_ -= i;
                }
            }

            auto recvFinishedCallback() noexcept {
                fifo_sequencer_.sequenceAndPublish();
            }

            // deleted copy & move constructors and assignment-operators
            OrderServer() = default;
            OrderServer(const OrderServer&) = delete;
            OrderServer(const OrderServer&&) = delete;
            OrderServer &operator=(const OrderServer&) = delete;
            OrderServer &operator=(const OrderServer&&) = delete;
    };
} // namespace Exchange

