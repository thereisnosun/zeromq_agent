#include "server_api.h"
#include "czmq.h"

#include <iostream>
#include <cstring>
#include <boost/asio.hpp>
namespace zmq
{

namespace internal
{

//TODO: clean messages after sending !!!
    class ServerImpl
    {
    public:
        ServerImpl(SocketType type):
            m_rcv_strand{m_io}
        {
            m_ctx = zmq_ctx_new ();
            m_type = std::move(type);
            m_message = new SendMessage;
        }

        ServerImpl(const ISession&):
            m_rcv_strand{m_io}
        {

        }
        ~ServerImpl()
        {
            zmq_ctx_destroy(m_ctx);
            zmq_close(m_socket);
            delete m_message;
        }

        ErrorType bind(const std::string& url)
        {
            m_socket = create_socket();
            if (!m_socket)
            {
                std::cout << "Could not create a socket" << errno << std::endl;
                return ErrorType::NOT_OK;
            }

            int error = zmq_bind (m_socket, url.c_str());
            if (error != 0)
            {
                std::cout << "ServerImpl: Error is - " << errno << " error:" << zmq_strerror(errno) << std::endl;
                //TODO: handle shit
                return ErrorType::NOT_OK;
            }

            return ErrorType::OK;
        }

        Status publish(const IMessage &message)
        {
            Status status;
            zmq_msg_t msg;
            int error = zmq_msg_init_data (&msg, message.get_data(), message.get_size(), NULL, NULL);
            if (error != 0)
            {
                std::cout << "SHIT!\n";
                status.error = ErrorType::NOT_OK;
                return status;
            }
            status.bytes_send = zmq_msg_send (&msg, m_socket, 0);

            if (status.bytes_send < 0)
            {
                status.error = ErrorType::NOT_OK;
            }
            return status;
        }

        const IMessage* receive()
        {
            if (m_type == SocketType::PUBLISH)
            {
                std::cout << "NOT IMPLEMENTED FOR PUB_SUB";
                return nullptr;
            }
            //TODO: handle multipart messages as well
            Status status;
            zmq_msg_t msg;
            if (zmq_msg_init (&msg) != 0)
            {
                //TODO: handle
                status.error = ErrorType::NOT_OK;
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
                return nullptr;
            }
            void* data = zmq_msg_data(&msg);
            int size = zmq_msg_size(&msg);
            std::cout << "After recv. Num of bytes - " << status.bytes_send <<
                         "Actual size - " << size << " \n";

            m_message->set_data(data);
            m_message->set_size(size);
            zmq_msg_close(&msg);

            const std::string str_message{static_cast<char*>(m_message->get_data()), m_message->get_size()};
            std::cout << "After SendMessage - " << str_message << std::endl;
            return m_message;
        }


        //TODO: register for all messages or only for one ?
        void async_receive(finish_receive_cbk_type&& cbk)
        {
            zmq_pollitem_t wait_items[] = {
                    { m_socket, 0, ZMQ_POLLIN, 0 }};

            zmq_msg_t msg;
            zmq_msg_init(&msg);

            if (zmq_msg_recv(&msg, m_socket, ZMQ_DONTWAIT) < 0)
            {
                if (errno == EAGAIN)
                {

                }
            }

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
            case SocketType::PUBLISH:
                    return zmq_socket (m_ctx, ZMQ_PUB);
            case SocketType::SUBSCRIBE:
                    return zmq_socket (m_ctx, ZMQ_SUB);
            case SocketType::REQUEST:
                    return zmq_socket (m_ctx, ZMQ_REQ);
            case SocketType::REPLY:
                    return zmq_socket (m_ctx, ZMQ_REP);
            }

            assert(false && "Client: Incorrect type of socket specified");

        }
        ISession m_session;
        void* m_ctx;
        void* m_socket;

        IMessage* m_message;

        SocketType m_type;

        finish_receive_cbk_type m_rcv_cbk;


        boost::asio::io_context m_io;
        boost::asio::io_context::strand m_rcv_strand;


     //   SendMessage m_cur_message; //shitty implementation

    };
} //namespace internal

Server::Server(SocketType type)
{
    m_impl = std::make_unique<internal::ServerImpl>(std::move(type));
}

Server::Server(const ISession& session)
{
 //TODO: restore from the session
}

Server::~Server()
{

}

ErrorType Server::bind(const std::string& url)
{
    return m_impl->bind(url);
}

const IMessage* Server::receive()
{
    return m_impl->receive();
}

void Server::async_receive(finish_receive_cbk_type&& cbk)
{
    m_impl->async_receive(std::move(cbk));
}

const ISession& Server::get_session() const
{
    m_impl->get_session();
}

Status Server::publish(const IMessage& message)
{
    return m_impl->publish(message);
}


}//namespace zmq
