#pragma once
#include "general_api.h"


namespace zmq
{
    namespace internal
    {
        class ServerImpl;
    }

    class Server
    {
    public:
        /* create server with absolutely new context */
        Server();

        /* create server from the already created context */
        explicit Server(const ISession&);

        ~Server();

        /* bind to the specific endpoint*/
        ErrorType bind(const std::string&);

        /* return the bytes received, if failed return -1. Blocking call */
        Status receive();

        /* Will return immediatly. Call the callback on the completion*/
        void async_receive(finish_send_cbk_type&&);

        /* Retrieve session*/
        const ISession& get_session() const;
    private:
        std::unique_ptr<internal::ServerImpl> m_impl;
    };

} //namespace zmq


