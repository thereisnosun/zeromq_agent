#include "server_api.h"


namespace zmq
{

namespace internal
{
    class ServerImpl
    {
    public:
        ServerImpl()
        {

        }

        ServerImpl(const ISession&)
        {

        }

        ErrorType bind(const std::string& url)
        {

        }

        Status receive()
        {
            return Status{};
        }

        void async_receive(finish_send_cbk_type&& cbk)
        {
        }

        const ISession& get_session() const
        {
            return m_session;
        }
    private:
        ISession m_session;

    };
} //namespace internal

Server::Server()
{
    m_impl = std::make_unique<internal::ServerImpl>();
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

Status Server::receive()
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
