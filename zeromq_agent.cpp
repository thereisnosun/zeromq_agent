#include "broker_api.h"
#include "server_api.h"
#include "client_api.h"

#include <iostream>
#include <random>

//const std::string UPDATE_TOKEN = "{'msg_type' : 'ipc', 'name': 'update_token', 'data': {'token': '<token>'}}";
const std::string UPDATE_TOKEN = "{'rock' : 'ipc', 'name': 'update_token', 'data': {'token': '<token>'}}";
const std::string CLIP_REQUEST = "{'msg_type' : 'ipc', 'name': 'clip_request', 'data': {'time': '<timestamp>'}}";
const std::string ADD_CAMERA = "{'msg_type' : 'ipc', 'name': 'add_camera|edit_camera|delete_camera', "
                               "'data': <camera_json_info>}";

const char* end_point = "tcp://localhost:5557";
const char* end_point_server = "tcp://*:5557";
const int ITERATIONS_NUM = 100;
const int SLEEP_MS = 10;

const std::vector<std::string> messages{UPDATE_TOKEN, UPDATE_TOKEN, UPDATE_TOKEN};

int req_rep_client()
{
    zmq::Client client{zmq::SocketType::REQ_REPLY};

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
    }
}

int req_rep_server()
{
    zmq::Server server{zmq::SocketType::REQ_REPLY};

    if (server.bind(end_point_server) != zmq::ErrorType::OK)
    {
        std::cout << "Could not bind a server\n";
        return -1;
    }

    for (int i = 0; i < ITERATIONS_NUM; ++i)
    {
        zmq::IMessage* message = server.receive();
        if (!message)
            continue;
        const std::string str_message{static_cast<char*>(message->get_data()), message->get_size()};
        std::cout << "Received a message from client: \n" << str_message << std::endl;
    }
}


int pub_sub_client()
{
    zmq::Client client{zmq::SocketType::PUB_SUB};

    if (client.connect(end_point) != zmq::ErrorType::OK)
    {
        std::cout << "Coud not connect from client\n";
        return -1;
    }

    for(int i = 0; i < ITERATIONS_NUM; ++i)
    {
        zmq::IMessage* message = client.receive();
        if (!message)
            continue;
        const std::string str_message{static_cast<char*>(message->get_data()), message->get_size()};
        std::cout << "Received a message from client: \n" << str_message << std::endl;
    }

}

int pub_sub_server()
{
    zmq::Server server{zmq::SocketType::PUB_SUB};

    if (server.bind(end_point_server) != zmq::ErrorType::OK)
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
        //req_rep_client();
        pub_sub_client();

    }
    else if (std::string{argv[1]} == "server")
    {
        std::cout << "Starting socket server... \n";
//        req_rep_server();
        pub_sub_server();

    }


    return 0;
}
