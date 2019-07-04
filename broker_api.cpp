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
        BrokerImpl(const std::string& backend, const std::string& frontend):
            m_backend_end_point{backend},
            m_frontend_end_point{frontend}
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

        ErrorType bind()
        {
            int error = zmq_bind(m_frontend, m_frontend_end_point.c_str());
            if (error != 0)
            {
                return ErrorType::NOT_OK;
            }

            error = zmq_bind(m_backend,  m_backend_end_point.c_str());
            if (error != 0)
            {
                return ErrorType::NOT_OK;
            }

            return ErrorType::OK;
        }

        void register_receiver(const Receiver& receiver)
        {

        }

        void unregister_receiver(const Receiver& receiver)
        {

        }

        void start_loop()
        {
            zmq_pollitem_t items [] = {
                    { m_frontend, 0, ZMQ_POLLIN, 0 },
                    { m_backend,  0, ZMQ_POLLIN, 0 }
                };

            while (true)
            {
                zmq_msg_t message;
                zmq_poll (items, 2, -1);
                if (items [0].revents & ZMQ_POLLIN) {
                    while (1) {
                        //  Process all parts of the message
                        zmq_msg_init (&message);
                        zmq_msg_recv (&message, m_frontend, 0);
                        int more = zmq_msg_more (&message);
                        const std::string str_message{static_cast<char*>(zmq_msg_data(&message)),
                                    zmq_msg_size(&message)};
                        std::cout << "Received message on frontend- " << str_message << std::endl;
                        zmq_msg_send (&message, m_backend, more? ZMQ_SNDMORE: 0);
                        zmq_msg_close (&message);


                        if (!more)
                            break;      //  Last message part
                    }
                }
                if (items [1].revents & ZMQ_POLLIN) {
                    while (1) {
                        //  Process all parts of the message
                        zmq_msg_init (&message);
                        zmq_msg_recv (&message, m_backend, 0);
                        int more = zmq_msg_more (&message);
                        const std::string str_message{static_cast<char*>(zmq_msg_data(&message)),
                                    zmq_msg_size(&message)};
                        std::cout << "Received message on backend- " << str_message << std::endl;
                        zmq_msg_send (&message, m_frontend, more? ZMQ_SNDMORE: 0);
                        zmq_msg_close (&message);
                        if (!more)
                            break;      //  Last message part
                    }
                }
            }

        }
    private:
        void* m_context;
        void* m_backend;
        void* m_frontend;
        std::vector<Receiver> m_receivers;

        std::string m_backend_end_point;
        std::string m_frontend_end_point;
    };

} //internal


Broker::Broker(const std::string& backend, const std::string& frontend)
{
    m_impl = std::make_unique<internal::BrokerImpl>(backend, frontend);
}

Broker::~Broker() = default;



void Broker::register_receiver(const Receiver& receiver)
{
    m_impl->register_receiver(receiver);
}

void Broker::unregister_receiver(const Receiver& receiver)
{
    m_impl->unregister_receiver(receiver);
}

void Broker::bind()
{
    m_impl->bind();
}


void Broker::start_loop()
{
    m_impl->start_loop();
}

}//namespace zmq
