#include "broker_api.h"

#include <zmq.h>
#include <vector>


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
        {
             m_context = zmq_ctx_new ();
             m_receivers.reserve(16);
        }
        ~BrokerImpl()
        {
            zmq_ctx_destroy(m_context);
        }
        void register_receiver(const Receiver& receiver)
        {

        }
        void unregister_receiver(const Receiver& receiver)
        {

        }

        void start_loop()
        {

        }
    private:
        void* m_context;
        std::vector<Receiver> m_receivers;
    };

} //internal


Broker::Broker()
{
    m_impl = std::make_unique<internal::BrokerImpl>();
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

void Broker::start_loop()
{
    m_impl->start_loop();
}

}//namespace zmq
