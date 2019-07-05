#pragma once
#include <memory>
#include "general_api.h"


namespace zmq
{
    namespace internal
    {
        class BrokerImpl;
        class BrokerPublisherImpl;
    }

    struct Receiver
    {

    };


    //TODO: if it is publish subscribe message
//TODO: make one base class for these two common functionality
    class Broker
    {
    public:
        Broker();
        ~Broker();
        void register_receiver(const Receiver&);
        void unregister_receiver(const Receiver&);

        void bind(const std::string&, const std::string&);
        void start_loop();
        void set_logging_cbk();

        Broker(const Broker&) = delete;
        Broker& operator=(const Broker&) = delete;
        //TODO; think about copying and moving
    private:
        std::unique_ptr<internal::BrokerImpl> m_impl;
    };


    class BrokerPublisher
    {
    public:
        BrokerPublisher();
        ~BrokerPublisher();
        ErrorType bind(const std::string& backend, const std::string& frontend);
        void start_loop();
    private:
        std::unique_ptr<internal::BrokerPublisherImpl> m_impl;
    };

}//namespace zmq
