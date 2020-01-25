#include "websocket_client.h"
#include "precompiled.h"

using namespace msgipc;
using websocketpp::lib::bind;

void OnClose(asio_client* c, websocketpp::connection_hdl hdl) {
    Sleep(msgipc::kClient.timeout());
    spdlog::info("retry connecting...");
    msgipc::kClient.connect();
}

void OnMessage(asio_client* c, websocketpp::connection_hdl hdl, message_ptr msg) {
    if (msgipc::kClient.callback())
        msgipc::kClient.callback()(msg->get_payload());
}

namespace msgipc
{

Client kClient("ws://127.0.0.1:5678");

Client::Client(const String& connect_str) :
    connected_(nullptr),
    callback_(nullptr),
    timeout_(3000),
    connect_str_(connect_str)
{
    client_.clear_access_channels(websocketpp::log::alevel::all);
    client_.init_asio();
    client_.set_message_handler(bind(&OnMessage,&client_, std::placeholders::_1, std::placeholders::_2));
    client_.set_fail_handler(bind(&OnClose, &client_, std::placeholders::_1));
    client_.set_close_handler(bind(&OnClose, &client_, std::placeholders::_1));
}

void Client::connect()
{
    spdlog::info("connecting...");
    websocketpp::lib::error_code ec;
    connected_ = client_.get_connection(connect_str_.c_str(), ec);
    if (ec) {
        spdlog::error("could not create connection because: {}", ec.message());
        return;
    }

    client_.connect(connected_);
    client_.run();
}

void Client::close()
{
    if(is_connected())
    {
        websocketpp::close::status::value val{};
        connected_->close(val, "");
        spdlog::info("connected close.");
    }
}

}
