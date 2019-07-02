#include "client_api.h"

//#include <czmq.h>
//#include "czmq.h"
#include "czmq.h"

namespace zmq
{

namespace internal
{
    class ClientImpl final
    {
    public:
        ClientImpl(SocketType type)
        {
            m_context = zmq_ctx_new ();
            m_type = std::move(type);
        }

        ClientImpl(const ISession& session)
        {

        }

        ~ClientImpl()
        {
            zmq_ctx_destroy( &m_context);
        }

        ErrorType connect(const std::string& url)
        {
            m_socket = create_socket();
            if (!m_socket)
            {
                //TODO: HANDLE
                return ErrorType::NOT_OK;
            }

            int error = zmq_connect(m_socket, url.c_str());
            if (error != 0)
            {
                //TODO: handle
                return ErrorType::NOT_OK;
            }

            return ErrorType::OK;
        }

        Status send(const IMessage& message)
        {
            Status status;
            status.bytes_send = zmq_send (m_socket, message.get_data(), message.get_size(), 0);
            if (status.bytes_send < 0)
            {
                status.error = ErrorType::NOT_OK;
            }
            return status;;
        }

        result_type async_send(const IMessage& message)
        {
            return result_type{};
        }

        void async_send(const IMessage&, finish_send_cbk_type&& callback)
        {

        }

        const ISession& get_session() const
        {
            return m_session;
        }
       private:
        void* create_socket()
        {
            switch(m_type)
            {
            case SocketType::PUB_SUB:
                    return zmq_socket (m_context, ZMQ_SUB);
            case SocketType::PUSH_PULL:
                    return zmq_socket (m_context, ZMQ_PULL);
            case SocketType::REQ_REPLY:
                    return zmq_socket (m_context, ZMQ_REQ);
            }

            assert(false && "Client: Incorrect type of socket specified");

        }

        void* m_context;
        void* m_socket;
        SocketType m_type;
        //TODO: integrate within the session
        ISession m_session;


    };
}//namespace internal

Client::Client(SocketType type)
{
    m_impl = std::make_unique<internal::ClientImpl>(type);
}

Client::Client(const ISession& session)
{
    //TODO: recreate somehow from the session
}

Client::~Client() = default;


ErrorType Client::connect(const std::string& url)
{
    return m_impl->connect(url);
}


Status Client::send(const IMessage& message)
{
    return m_impl->send(message);
}

result_type Client::async_send(const IMessage& message)
{
    return m_impl->async_send(message);
}


void Client::async_send(const IMessage& message, finish_send_cbk_type&& callback)
{
    m_impl->async_send(message, std::move(callback));
}

const ISession& Client::get_session() const
{
    return m_impl->get_session();
}

}//namespace zmq
