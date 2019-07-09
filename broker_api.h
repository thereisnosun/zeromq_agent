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

    class IBroker
    {
    public:
        virtual void start_loop() = 0;
        virtual ErrorType bind(const std::string& backend, const std::string& frontend) = 0;
        virtual std::string get_str_error() const = 0;
    };

    class Broker: public IBroker
    {
    public:
        Broker();
        ~Broker();
//        void register_receiver(const Receiver&);
//        void unregister_receiver(const Receiver&);

        ErrorType bind(const std::string& backend, const std::string& frontend) override;
        void start_loop() override;
        std::string get_str_error() const override;

        //void set_logging_cbk();

        Broker(const Broker&) = delete;
        Broker& operator=(const Broker&) = delete;
        Broker(Broker&&) = delete;
        Broker& operator=(Broker&&) = delete;
        //TODO; think about copying and moving
    private:
        std::unique_ptr<internal::BrokerImpl> m_impl;
    };


    class BrokerPublisher: public IBroker
    {
    public:
        BrokerPublisher();
        ~BrokerPublisher();
        ErrorType bind(const std::string& backend, const std::string& frontend) override;
        void start_loop() override;
        std::string get_str_error() const override;

        BrokerPublisher(const BrokerPublisher&) = delete;
        BrokerPublisher& operator=(const BrokerPublisher&) = delete;
        BrokerPublisher(BrokerPublisher&&) = delete;
        BrokerPublisher& operator=(BrokerPublisher&&) = delete;
    private:
        std::unique_ptr<internal::BrokerPublisherImpl> m_impl;
    };

}//namespace zmq
