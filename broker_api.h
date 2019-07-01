#pragma once


struct Receiver
{

};


class Broker
{
public:
    void register_receiver(const Receiver&);
    void unregister_receiver(const Receiver&);
};
