#include "client_api.h"

//#include <czmq.h>
//#include "czmq.h"
#include "czmq.h"
#include <iostream>

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
            m_message = new SendMessage;
        }

        ClientImpl(const ISession& session)
        {

        }

        ~ClientImpl()
        {
            zmq_ctx_destroy( &m_context);
            delete m_message;
        }

        ErrorType connect(const std::string& url)
        {
            m_socket = create_socket();
            if (!m_socket)
            {
                std::cout << "ClientImpl Could not create a socket is - " << errno << std::endl;
                //TODO: HANDLE
                return ErrorType::NOT_OK;
            }

            int error = zmq_connect(m_socket, url.c_str());
            if (error != 0)
            {
                std::cout << "ClientImpl: Error is - " << errno << std::endl;
                //TODO: handle
                return ErrorType::NOT_OK;
            }

            if (m_type == zmq::SocketType::PUB_SUB)
            {
                error =  zmq_setsockopt (m_socket, ZMQ_SUBSCRIBE,
                                     NULL, 0);
                if (error != 0)
                {
                    return ErrorType::NOT_OK;
                }

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
            return status;
        }


        IMessage* receive()
        {
            //m_message
            Status status;
            zmq_msg_t msg;
            if (zmq_msg_init (&msg) != 0)
            {
                //TODO: handle
                status.error = ErrorType::NOT_OK;
                //return status;
                return nullptr;
            }

            std::cout << "After msg init\n";
            status.bytes_send = zmq_msg_recv(&msg, m_socket, 0);
            if (status.bytes_send < 0)
            {
                std::cout << "Status bytes sent - " << status.bytes_send << ", error is "
                          << zmq_strerror(errno) << std::endl;
                //TODO: handle
                status.error  = ErrorType::NOT_OK;
                //return status;
                return nullptr;
            }
            std::cout << "After recv\n";
            void* data = zmq_msg_data(&msg);
            int size = zmq_msg_size(&msg);

            m_message->set_data(data);
            m_message->set_size(size);

            zmq_msg_close(&msg);

            return m_message;
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
        IMessage* m_message;


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

IMessage* Client::receive()
{
    return m_impl->receive();
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
