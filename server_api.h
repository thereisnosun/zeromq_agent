#pragma once
#include "general_api.h"


namespace zmq
{
    namespace internal
    {
        class ServerImpl;
    }



    //TODO: add filtering message
    class Server
    {
    public:
        /* create server with absolutely new context */
        Server(SocketType type);

        /* create server from the already created context */
        explicit Server(const ISession&);

        ~Server();

        /* bind to the specific endpoint*/
        ErrorType bind(const std::string& url);

        /* return the bytes received, if failed return -1. Blocking call */
        const IMessage* receive();

        /* Will return immediatly. Call the callback on the completion*/
        void async_receive(finish_receive_cbk_type&&);

        /* return the bytes send, if failed return -1. Blocking call.*/
        Status publish(const IMessage& );

        /* return the bytes send, if failed return -1. When sent cbk will be called*/
        void async_publish(const IMessage& message, finish_send_cbk_type&& cbk);

        /* Retrieve session*/
        const ISession& get_session() const;
    private:
        std::unique_ptr<internal::ServerImpl> m_impl;
    };

} //namespace zmq



