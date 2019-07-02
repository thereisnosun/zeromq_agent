#pragma once
#include "general_api.h"

namespace zmq
{

    namespace internal
    {
        class ClientImpl;
    }

    //TODO: return future or call the callback, the should share a context
    class Client
    {
    public:
        /* create client with absolutely new context */
        Client(SocketType type);

        /* create client from the session, with already created context */
        explicit Client(const ISession&);

        ~Client();

        /* connect to the specific endpoint */
        ErrorType connect(const std::string& url);

        /* return the bytes send, if failed return -1. Blocking call.*/
        Status send(const IMessage&);

        /* return the bytes send, if failed return error message. Will return the future which can be wait on.*/
        result_type async_send(const IMessage&);

        /* Will return immediatly. Call the callback on the completion*/
        void async_send(const IMessage&, finish_send_cbk_type&& callback);

        /* Retrieve session*/
        const ISession& get_session() const;
    private:
        std::unique_ptr<internal::ClientImpl> m_impl;
    };

}//namespace zmq
