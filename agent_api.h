#pragma once
#include <future>
#include <functional>

class IMessage
{
public:
    virtual IMessage clone() const;
    virtual void serialize() noexcept;
    virtual void deserialize() noexcept;
    virtual void* get_data() = 0;
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
    virtual void* get_context() const;
    virtual std::string get_enpoint() const;
    virtual ~ISesion();
};

//TODO: return future or call the callback, the should share a context
class IClient
{
public:
    /* create client with absolutely new context */
    IClient();
    /* create client from the session, with already created context */
    explicit IClient(const ISession&);
    /* connect to the specific endpoint */
    ErrorType connect(const std::string&);
    /* return the bytes send, if failed return -1. Blocking call.*/
    Status send(const IMessage&);
    /* return the bytes send, if failed return error message. Will return the future which can be wait on.*/
    result_type async_send(const IMessage&);
    /* Will return immediatly. Call the callback on the completion*/
    void async_send(const IMessage&, finish_send_cbk_type);
    /* Retrieve session*/
    const ISession& get_session() const;
};

class IServer
{
public:
    /* create server with absolutely new context */
    IServer();
    /* create server from the already created context */
    explicit IServer(const ISession&);
    /* bind to the specific endpoint*/
    ErrorType bind(const std::string&);
    /* return the bytes received, if failed return -1. Blocking call */
    Status receive();
    /* Will return immediatly. Call the callback on the completion*/
    result_type async_receive(finish_send_cbk_type);

    /* Retrieve session*/
    const ISession& get_session() const;
};



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

