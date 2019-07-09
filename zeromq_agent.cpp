#include "broker_api.h"
#include "server_api.h"
#include "client_api.h"

#include <iostream>
#include <random>

//TODO: test with raw void* ptr

const std::string UPDATE_TOKEN = "{'msg_type' : 'ipc', 'name': 'update_token', 'data': {'token': '<token>'}}";
const std::string CLIP_REQUEST = "{'msg_type' : 'ipc', 'name': 'clip_request', 'data': {'time': '<timestamp>'}}";
const std::string ADD_CAMERA = "{'msg_type' : 'ipc', 'name': 'add_camera|edit_camera|delete_camera', "
                               "'data': <camera_json_info>}";

//const std::string REPLY = "1234567{'status' : 'ok'}";
const std::string REPLY = "{'status' : 'ok'}";

const std::string FRONTEND_ENDPOINT = "tcp://*:5559";
const std::string BACKEND_ENDPOINT = "tcp://*:5560";

const char* end_point = "tcp://localhost:5559";
const char* end_point_front_end = "tcp://localhost:5560";
const char* end_point_server = "tcp://*:5559";
const int ITERATIONS_NUM = 100;
const int SLEEP_MS = 100;

const std::vector<std::string> messages{UPDATE_TOKEN, CLIP_REQUEST, ADD_CAMERA};

int req_rep_client(const std::string& end_point)
{
    zmq::Client client{zmq::SocketType::REQUEST};

    if (client.connect(end_point) != zmq::ErrorType::OK)
    {
        std::cout << "Coud not connect from client\n";
        return -1;
    }

    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist(0, 2);

    for(int i = 0; i < ITERATIONS_NUM; ++i)
    {
        const int index = dist(rng);
        const std::string& message_to_send = messages[index];
        std::cout << "Sending message: \n" << message_to_send << "\n";
        zmq::SendMessage zmq_message{message_to_send};
        client.send(zmq_message);
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_MS));

        const zmq::IMessage * message = client.receive();
        if (!message)
            continue;
        const std::string str_message{static_cast<char*>(message->get_data()), message->get_size()};
        std::cout << "Received a response from server: \n" << str_message << std::endl;
    }
    return 0;
}

int req_rep_server(const std::string& end_point_server)
{
    zmq::Server server{zmq::SocketType::REPLY};


    if (server.bind(end_point_server) != zmq::ErrorType::OK)
    {
        std::cout << "Could not bind a server\n";
        return -1;
    }

    for (int i = 0; i < ITERATIONS_NUM; ++i)
    {
        const zmq::IMessage* message = server.receive();
        if (!message)
            continue;
        const std::string str_message{static_cast<char*>(message->get_data()), message->get_size()};
        std::cout << "Received a message from client: \n" << str_message << std::endl;

        zmq::SendMessage send_message{REPLY};
        server.publish(send_message);
    }
    return 0;
}


int pub_sub_client(const std::string& end_point)
{
    zmq::Client client{zmq::SocketType::SUBSCRIBE};

    if (client.connect(end_point) != zmq::ErrorType::OK)
    {
        std::cout << "Coud not connect from client\n";
        return -1;
    }

    for(int i = 0; i < ITERATIONS_NUM; ++i)
    {
        const zmq::IMessage* message = client.receive();
        if (!message)
            continue;
        const std::string str_message{static_cast<char*>(message->get_data()), message->get_size()};
        std::cout << "Received a message from client: \n" << str_message << std::endl;
    }
    return 0;

}

int pub_sub_server(const std::string& end_point)
{
    zmq::Server server{zmq::SocketType::PUBLISH};

    if (server.bind(end_point) != zmq::ErrorType::OK)
    {
        std::cout << "Could not bind to server\n";
        return -1;
    }

    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist(0, 2);


    for(int i = 0; i < ITERATIONS_NUM; ++i)
    {
        const int index = dist(rng);
        const std::string& message_to_send = messages[index];
        std::cout << "Sending message: \n" << message_to_send << "\n";
        zmq::SendMessage zmq_message{message_to_send};
        server.publish(zmq_message);
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_MS));
    }
    return 0;
}


int req_rep_worker(const std::string& end_point)
{
    zmq::Client client{zmq::SocketType::REPLY};

    if (client.connect(end_point) != zmq::ErrorType::OK)
    {
        std::cout << "Coud not connect from client\n";
        return -1;
    }

    for(int i = 0; i < ITERATIONS_NUM; ++i)
    {
        const zmq::IMessage * message = client.receive();
        if (!message)
            continue;
        const std::string str_message{static_cast<char*>(message->get_data()), message->get_size()};
        std::cout << "Received a response from server: \n" << str_message << std::endl;

        std::cout << "Sending message: \n" << REPLY << "\n";
        zmq::SendMessage send_message{REPLY};
        client.send(send_message);
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_MS));
    }
    return 0;
}

int pub_sub_client_async(const std::string& end_point)
{
    zmq::Client client{zmq::SocketType::SUBSCRIBE};

    if (client.connect(end_point) != zmq::ErrorType::OK)
    {
        std::cout << "Coud not connect from client\n";
        return -1;
    }

    for(int i = 0; i < ITERATIONS_NUM; ++i)
    {
        client.async_receive([](zmq::IMessage& message) -> void
        {
            const std::string str_message{static_cast<char*>(message.get_data()), message.get_size()};
            std::cout << "Received a message from client: \n" << str_message << std::endl;
        });
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_MS));
        std::cout << "Waiting for the previous message\n";
    }
    std::cout << "Press a key\n";
    //getchar();
    std::cout << "End of function\n";
    return 0;
}



int req_rep_client_async(const std::string& end_point)
{
    zmq::Client client{zmq::SocketType::REQUEST};

    if (client.connect(end_point) != zmq::ErrorType::OK)
    {
        std::cout << "Coud not connect from client\n";
        return -1;
    }

    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist(0, 2);

    for(int i = 0; i < ITERATIONS_NUM; ++i)
    {
        const int index = dist(rng);
        const std::string& message_to_send = messages[index];
        std::cout << "Sending message: \n" << message_to_send << "\n";
        zmq::SendMessage zmq_message{message_to_send};
        client.async_send(zmq_message, [&client] (zmq::Status status) -> void {
            const zmq::IMessage * message = client.receive();
            if (!message)
            {
                std::cout << "Failed to receive stuff\n";
                return;
            }

            const std::string str_message{static_cast<char*>(message->get_data()), message->get_size()};
            std::cout << "Received a response from server: \n" << str_message << std::endl;
        });
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_MS));
        std::cout << "Iteration " << i << " is finished\n";
    }
    return 0;
}


int req_rep_server_async(const std::string& end_point_server)
{
    zmq::Server server{zmq::SocketType::REPLY};

    if (server.bind(end_point_server) != zmq::ErrorType::OK)
    {
        std::cout << "Could not bind a server\n";
        return -1;
    }

    for (int i = 0; i < ITERATIONS_NUM; ++i)
    {
        server.async_receive([&server, &REPLY](zmq::IMessage& message)
        {
            const std::string str_message{static_cast<char*>(message.get_data()), message.get_size()};
            std::cout << "Received a message from client: \n" << str_message << std::endl;

            zmq::SendMessage send_message{REPLY};
            server.publish(send_message);
        });
        std::cout << "Iteration " << i << " is finished\n";
    }
    std::cout << "Enter a character...\n";
    getchar();
    return 0;
}



int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cout << "Specify mode, bitch! \n";
        return -1;
    }

    if (std::string{argv[1]} == "client")
    {
        std::cout << "Starting socket client... \n";
        req_rep_client(end_point);
    }
    else if (std::string{argv[1]} == "server")
    {
        std::cout << "Starting socket server... \n";
        req_rep_server(end_point_server);
    }
    else if (std::string{argv[1]} == "client_pub")
    {
        std::cout << "Starting socket subscribe client... \n";
        pub_sub_client(end_point);
    }
    else if (std::string{argv[1]} == "server_pub")
    {
        std::cout << "Starting socket publish server... \n";
        pub_sub_server(end_point_server);
    }
    else if (std::string{argv[1]} == "broker")
    {
        std::cout << "Starting broker REQ - REP...\n";
        zmq::Broker broker;
        broker.bind(BACKEND_ENDPOINT, FRONTEND_ENDPOINT);
        broker.start_loop();
    }
    else if (std::string{argv[1]} == "worker")
    {
        std::cout << "Starting worker...\n";
        req_rep_worker(end_point_front_end);
    }
    else if (std::string{argv[1]} == "broker_publish")
    {
        std::cout << "Starting broker PUB - SUB...\n";
        zmq::BrokerPublisher broker;
        broker.bind(BACKEND_ENDPOINT, FRONTEND_ENDPOINT);
    }
    else if (std::string{argv[1]} == "client_pub_async")
    {
        std::cout << "Starting async subscribe client...\n";
        pub_sub_client_async(end_point);
    }
    else if (std::string{argv[1]} == "client_req_async")
    {
        std::cout << "Starting async REQ_REP client...\n";
        req_rep_client_async(end_point);
    }
    else if (std::string{argv[1]} == "server_req_async")
    {
        std::cout << "Starting async REQ_REP server\n";
        req_rep_server_async(end_point_server);
    }
    else
    {
        std::cout << "You've specified wrong mode, bitch.\n";
        return -1;
    }

    std::cout << "Normal program exit\n";
    return 0;
}
