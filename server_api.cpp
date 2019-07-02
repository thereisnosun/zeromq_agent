#include "server_api.h"
#include "czmq.h"


namespace zmq
{

namespace internal
{
    class ServerImpl
    {
    public:
        ServerImpl(SocketType type)
        {
            m_type = std::move(type);
        }

        ServerImpl(const ISession&)
        {

        }

        ErrorType bind(const std::string& url)
        {
            m_socket = create_socket();
            if (!m_socket)
            {
               return ErrorType::NOT_OK;
            }

            int error = zmq_bind (m_socket, url.c_str());
            if (error != 0)
            {
                //TODO: handle shit
                return ErrorType::NOT_OK;
            }

            return ErrorType::OK;
        }

        IMessage* receive()
        {
            //TODO: handle multipart messages as well
            Status status;
            zmq_msg_t msg;
            if (zmq_msg_init (&msg) != 0)
            {
                //TODO: handle
                status.error = ErrorType::NOT_OK;
                //return status;
                return nullptr;
            }

            status.bytes_send = zmq_msg_recv(&msg, m_socket, 0);
            if (status.bytes_send < 0)
            {
                //TODO: handle
                status.error  = ErrorType::NOT_OK;
                //return status;
                return nullptr;
            }



            void* data = zmq_msg_data(&msg);
            int size = zmq_msg_size(&msg);

            //TODO: make it ok, not so shitty, hust quick for testing
            SendMessage* message = new SendMessage{data, size};


            return message;
        }

        void async_receive(finish_send_cbk_type&& cbk)
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
                    return zmq_socket (m_ctx, ZMQ_PUB);
            case SocketType::PUSH_PULL:
                    return zmq_socket (m_ctx, ZMQ_PUSH);
            case SocketType::REQ_REPLY:
                    return zmq_socket (m_ctx, ZMQ_REP);
            }

            assert(false && "Client: Incorrect type of socket specified");

        }
        ISession m_session;
        void* m_ctx;
        void* m_socket;

        SocketType m_type;


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

IMessage* Server::receive()
{
    return m_impl->receive();
}

void Server::async_receive(finish_send_cbk_type&& cbk)
{
    m_impl->async_receive(std::move(cbk));
}

const ISession& Server::get_session() const
{
    m_impl->get_session();
}


}//namespace zmq
