#pragma once
#include <future>
#include <functional>



namespace zmq
{
    class IMessage
    {
    public:
        virtual IMessage clone() const;
        virtual void serialize() noexcept;
        virtual void deserialize() noexcept;
        virtual void* get_data();
        virtual ~IMessage();
    };

    enum class ErrorType
    {
        OK
    };

    struct Status
    {
        ErrorType error;
        int bytes_send;
    };

    using result_type = std::future<Status>;
    using finish_send_cbk_type = std::function<void(Status)>;

    //as we should use one context
    class ISession
    {
    public:
        virtual void* get_context() const
        {
            return nullptr;
        }
        virtual std::string get_enpoint() const
        {
            return "";
        }
        virtual ~ISession(){}
    };

} //namespace zmq


//TODO:
// request-reply pattern (via broker)
// publish -subscribe pattern
// PULL - PUSH request


//both client and server should be used
//specify the protocol tcp or ipc

//how to handle service fail
//how to handle service kill signals


//contex is thread safe, sockets are not
//zero copy for large chunks of data


//TODO: use Agent wrapper class, which registers callbacks for receiving messaging
// and has and interface to send message

