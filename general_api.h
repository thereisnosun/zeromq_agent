#pragma once
#include <future>
#include <functional>
#include <cstring>

#include "czmq.h"

namespace zmq
{
    class IMessage
    {
    public:
        virtual IMessage clone() const
        {
            return *this;
        }
        virtual void serialize() noexcept
        {
        }
        virtual void deserialize() noexcept
        {
        }
        virtual void* get_data() const
        {
            return nullptr;
        }
        virtual int get_size() const
        {
            return 0;
        }
        virtual void set_data(void* data)
        {

        }
        virtual void set_size(int size)
        {

        }

        virtual ~IMessage()
        {
        }
    };

    class SendMessage: public IMessage
    {
    public:
        SendMessage(const std::string& message):
            m_size{message.size()}
        {
             m_data = new char[m_size + 1];
             std::memset(m_data, 0x00, m_size);
             std::memcpy(m_data, message.data(), m_size);
             m_is_allocated = true;
        }
        ~SendMessage()
        {
            if (m_is_allocated)
                delete[] m_data;
        }

        /* It is caller's reponsibility to free the buffer, when using this constructor */
        SendMessage(void* start_ptr, int size):
            m_data{start_ptr},
            m_size{size},
            m_is_allocated{false}
        {
        }
        SendMessage():
            m_data{nullptr},
            m_size{0}
        {
        }
        SendMessage(const SendMessage& in_message):
            m_data{in_message.m_data},
            m_size{in_message.m_size}
        {

        }
        SendMessage(SendMessage&& in_message):
            m_data{std::move(in_message.m_data)},
            m_size{std::move(in_message.m_size)}
        {

        }
        SendMessage& operator=(SendMessage message)
        {
            swap(*this, message);
        }
        void* get_data() const override
        {
            return m_data;
        }
        int get_size() const override
        {
            return m_size;
        }
        void set_data(void* data) override
        {
            m_data = data;
        }
        void set_size(int size) override
        {
            m_size = size;
        }
        friend void swap(SendMessage& first, SendMessage& second)
        {
            std::swap(first.m_data, second.m_data);
            std::swap(first.m_size, second.m_size);
        }
    private:
        int m_size;
        void* m_data;
        bool m_is_allocated;
    };


    class ReceiveMessage: public IMessage
    {
    public:

    private:
    };

    enum class ErrorType
    {
        OK,
        NOT_OK
    };

    enum class SocketType
    {
        REQUEST,
        REPLY,
        PUBLISH,
        SUBSCRIBE
    };

    struct Status
    {
        Status():
            error{ErrorType::OK},
            bytes_send{0},
            error_str{}
        {
        }
        ErrorType error;
        int bytes_send;
        std::string error_str;
    };

    using result_type = std::future<Status>;
    using finish_send_cbk_type = std::function<void(Status)>;
    using finish_receive_cbk_type = std::function<void(IMessage&)>;

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

//    class ZeroMqSession: public ISession
//    {
//    public:
//        ZeroMqSession()
//        {
//            //m_context = zmq_ctx_new();
//        }
//        void* get_context() const override
//        {

//        }
//        std::string get_enpoint() const override
//        {

//        }
//    private:
//        void *m_context;

//    };

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

