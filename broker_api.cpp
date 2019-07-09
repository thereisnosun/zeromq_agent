#include "broker_api.h"
#include "general_api.h"

#include <zmq.h>
#include <vector>
#include <iostream>

namespace zmq
{
namespace internal
{
//TODO: how to hanbdle errors?,
//return - return codes ?

    class BrokerImpl final
    {
    public:
        BrokerImpl()
        {//TODO: add error checking
             m_context = zmq_ctx_new ();

             m_frontend = zmq_socket(m_context, ZMQ_ROUTER);
             m_backend = zmq_socket(m_context, ZMQ_DEALER);

             m_receivers.reserve(16);
        }

        ~BrokerImpl()
        {
            zmq_close(m_backend);
            zmq_close(m_frontend);
            zmq_ctx_destroy(m_context);
        }

        ErrorType bind(const std::string& backend, const std::string& frontend)
        {
            int error = zmq_bind(m_frontend, frontend.c_str());
            if (error != 0)
            {
                m_str_error = zmq_strerror(errno);
                return ErrorType::NOT_OK;
            }

            error = zmq_bind(m_backend,  backend.c_str());
            if (error != 0)
            {
                m_str_error = zmq_strerror(errno);
                return ErrorType::NOT_OK;
            }

            return ErrorType::OK;
        }

        void start_loop()
        {
            zmq_pollitem_t items [] =
            {
                    { m_frontend, 0, ZMQ_POLLIN, 0 },
                    { m_backend,  0, ZMQ_POLLIN, 0 }
            };

            while (true)
            {
                zmq_msg_t message;
                zmq_poll (items, 2, -1);
                if (items [0].revents & ZMQ_POLLIN)
                {
                    while (1)
                    {
                        zmq_msg_init (&message);
                        zmq_msg_recv (&message, m_frontend, 0);
                        int more = zmq_msg_more (&message);
                        const std::string str_message{static_cast<char*>(zmq_msg_data(&message)),
                                    zmq_msg_size(&message)};
                        std::cout << "Received message on frontend- " << str_message << std::endl;
                        zmq_msg_send (&message, m_backend, more? ZMQ_SNDMORE: 0);
                        zmq_msg_close (&message);

                        if (!more)
                            break;
                    }
                }
                if (items [1].revents & ZMQ_POLLIN)
                {
                    while (1)
                    {
                        zmq_msg_init (&message);
                        zmq_msg_recv (&message, m_backend, 0);
                        int more = zmq_msg_more (&message);
                        const std::string str_message{static_cast<char*>(zmq_msg_data(&message)),
                                    zmq_msg_size(&message)};
                        std::cout << "Received message on backend- " << str_message << std::endl;
                        zmq_msg_send (&message, m_frontend, more? ZMQ_SNDMORE: 0);
                        zmq_msg_close (&message);
                        if (!more)
                            break;
                    }
                }
            }

            //TODO: !IMPORTANT: add reply mechanism if a client stucked after the REQ

        }
        std::string get_str_error() const
        {
            return m_str_error;
        }
    private:
        void* m_context;
        void* m_backend;
        void* m_frontend;

        std::string m_str_error;

        std::vector<Receiver> m_receivers;

    };

    class BrokerPublisherImpl final
    {
    public:
        BrokerPublisherImpl()
        {
             m_context = zmq_ctx_new ();
        }
        ~BrokerPublisherImpl()
        {
            zmq_close(m_backend);
            zmq_close(m_frontend);
            zmq_ctx_destroy(m_context);
        }

        ErrorType bind(const std::string& backend, const std::string& frontend)
        {
            void* m_frontend = zmq_socket(m_context, ZMQ_XSUB);
            if (!m_frontend)
            {
                m_str_error = zmq_strerror(errno);
                return ErrorType::NOT_OK;
            }

            void* m_backend = zmq_socket(m_context, ZMQ_XPUB);
            if (!m_backend)
            {
                m_str_error = zmq_strerror(errno);
                return ErrorType::NOT_OK;
            }

            int error = zmq_connect(m_frontend, frontend.c_str());
            if (error != 0)
            {
                m_str_error = zmq_strerror(errno);
                return ErrorType::NOT_OK;
            }

            error = zmq_bind(m_backend, backend.c_str());
            if (error != 0)
            {
                m_str_error = zmq_strerror(errno);
                return ErrorType::NOT_OK;
            }

            return ErrorType::OK;
        }


        void start_loop()
        {
            int error = zmq_proxy(m_frontend, m_backend, NULL);
            if (error != 0)
            {
                m_str_error = zmq_strerror(errno);
                std::cout << "Error during proxy bind is - " << zmq_strerror(errno) << std::endl;
            }
        }
        std::string get_str_error() const
        {
            return m_str_error;
        }

    private:
        void* m_context;
        void* m_frontend;
        void* m_backend;

        std::string m_str_error;
    };

} //internal


Broker::Broker()
{
    m_impl = std::make_unique<internal::BrokerImpl>();
}

Broker::~Broker() = default;



//void Broker::register_receiver(const Receiver& receiver)
//{
//    m_impl->register_receiver(receiver);
//}

//void Broker::unregister_receiver(const Receiver& receiver)
//{
//    m_impl->unregister_receiver(receiver);
//}

ErrorType Broker::bind(const std::string& backend, const std::string& frontend)
{
    return m_impl->bind(backend, frontend);
}


void Broker::start_loop()
{
    m_impl->start_loop();
}

std::string Broker::get_str_error() const
{
    return m_impl->get_str_error();
}

BrokerPublisher::BrokerPublisher()
{
    m_impl = std::make_unique<internal::BrokerPublisherImpl>();
}

BrokerPublisher::~BrokerPublisher() = default;

ErrorType BrokerPublisher::bind(const std::string& backend, const std::string& frontend)
{
    return m_impl->bind(backend, frontend);
}


void BrokerPublisher::start_loop()
{
    m_impl->start_loop();
}

std::string BrokerPublisher::get_str_error() const
{
    return m_impl->get_str_error();
}



}//namespace zmq
