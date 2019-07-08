#include "client_api.h"

//#include <czmq.h>
//#include "czmq.h"
#include "czmq.h"
#include <iostream>
#include <boost/asio.hpp>
#include <thread>

namespace zmq
{

namespace internal
{
    class ClientImpl final
    {
    public:
        ClientImpl(SocketType type):
            m_rcv_strand{m_io},
            m_running{false}
        {
            m_context = zmq_ctx_new ();
            m_type = std::move(type);
            m_message = new SendMessage;

            m_worker = std::thread{[this]() -> void
            {
                auto work = std::make_shared<boost::asio::io_service::work>( m_io );
                m_running = true;
                std::cout << "Strating the main loop...\n";
                m_io.run();
            }};
        }

        ClientImpl(const ISession& session):
            m_rcv_strand{m_io},
            m_running{false}
        {

        }


        //TODO: clean messages after sending !!!
        ~ClientImpl()
        {
            std::cout << "Before destroy:\n";
            if (m_running)
            {
                m_io.stop();
            }
            if (m_worker.joinable())
                m_worker.join();
            zmq_close(m_socket);
            zmq_ctx_destroy(m_context);
            delete m_message;
            std::cout << "Before this shit:\n";

            std::cout << "Exiting from destructor...\n";
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

            if (m_type == zmq::SocketType::SUBSCRIBE)
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
            return send_worker(message, 0);
        }

        Status send_worker(const IMessage& message, int flags)
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
            status.bytes_send = zmq_msg_send (&msg, m_socket, flags);

            if (status.bytes_send < 0)
            {
                status.error = ErrorType::NOT_OK;
            }
            return status;
        }

        const IMessage* receive()
        {
            //m_message
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
                //TODO: handle
                status.error  = ErrorType::NOT_OK;
                return nullptr;
            }
            void* data = zmq_msg_data(&msg);
            int size = zmq_msg_size(&msg);
            const std::string str_message{static_cast<char*>(data), size};
            std::cout << "After recv. Num of bytes - " << status.bytes_send << ". Message:" <<
                    str_message << " \n";

            m_message->set_data(data);
            m_message->set_size(size);

            //TODO: zmq_getsockopt (socket, ZMQ_RCVMORE, &more, &more_size);

            zmq_msg_close(&msg);

            return m_message;
        }

        result_type async_send(const IMessage& message)
        {
            return result_type{};
        }

        void async_send(const IMessage& message, finish_send_cbk_type&& cbk)
        {
            m_rcv_strand.post([this, &message, &cbk]()
            {
               Status status = send_worker(message, ZMQ_NOBLOCK);
               cbk(status);
            });
        }

        void async_receive(finish_receive_cbk_type&& cbk)
        {
            std::cout << "async_receive\n";
            m_rcv_strand.post([this, &cbk]()
            {
                std::cout << "Entered strand\n";
                zmq_pollitem_t wait_items[] = {
                        { m_socket, 0, ZMQ_POLLIN, 0 }};

                zmq_msg_t msg;
                zmq_msg_init(&msg);


                zmq_poll (wait_items, 1, -1);
                if (wait_items [0].revents & ZMQ_POLLIN)
                {
                    std::cout << "Before recieve\n";
                    if (zmq_msg_recv(&msg, m_socket, ZMQ_DONTWAIT) < 0)
                    {
                        std::cout << "receive failed\n";
                        if (errno == EAGAIN)
                        {
                            std::cout << "Wait again\n";
                        }
                    }
                    else
                    {
                        SendMessage message{zmq_msg_data(&msg), zmq_msg_size(&msg)};
                        cbk(message);
                        zmq_msg_close(&msg);
                    }
                }
            });

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
                    return zmq_socket (m_context, ZMQ_PUB);
            case SocketType::SUBSCRIBE:
                    return zmq_socket (m_context, ZMQ_SUB);
            case SocketType::REQUEST:
                    return zmq_socket (m_context, ZMQ_REQ);
            case SocketType::REPLY:
                    return zmq_socket (m_context, ZMQ_REP);
            }

            assert(false && "Client: Incorrect type of socket specified");
        }

        void* m_context;
        void* m_socket;
        SocketType m_type;
        IMessage* m_message;

        boost::asio::io_context m_io;
        boost::asio::io_context::strand m_rcv_strand;
        bool m_running;
        std::thread m_worker;

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

const IMessage* Client::receive()
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

void Client::async_receive(finish_receive_cbk_type&& cbk)
{
    m_impl->async_receive(std::move(cbk));
}

const ISession& Client::get_session() const
{
    return m_impl->get_session();
}

}//namespace zmq
