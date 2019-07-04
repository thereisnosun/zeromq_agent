#pragma once
#include <memory>



namespace zmq
{
    namespace internal
    {
        class BrokerImpl;
    }

    struct Receiver
    {

    };



    //TODO: if it is publish subscribe message
    class Broker
    {
    public:
        Broker(const std::string&, const std::string&);
        ~Broker();
        void register_receiver(const Receiver&);
        void unregister_receiver(const Receiver&);

        void bind();
        void start_loop();
        void set_logging_cbk();

        Broker(const Broker&) = delete;
        Broker& operator=(const Broker&) = delete;
        //TODO; think about copying and moving
    private:
        std::unique_ptr<internal::BrokerImpl> m_impl;
    };

}//namespace zmq
