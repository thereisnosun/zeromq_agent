#include "client_api.h"


namespace zmq
{

namespace internal
{
    class ClientImpl final
    {
    public:
        ClientImpl()
        {

        }

        ClientImpl(const ISession& session)
        {

        }

        ~ClientImpl()
        {

        }

        ErrorType connect(const std::string& )
        {
            return ErrorType::OK;
        }

        Status send(const IMessage& message)
        {
            return Status{};
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
        ISession m_session;


    };
}//namespace internal

Client::Client()
{
    m_impl = std::make_unique<internal::ClientImpl>();
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
