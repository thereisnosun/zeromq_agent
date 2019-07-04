//  Weather update server
//  Binds PUB socket to tcp://*:5556
//  Publishes random weather updates

#include "zhelpers.h"
#include "consts.h"

const char szBigBuffer[] = "Most messaging projects, like AMQP, that try to solve this long list of problems in a reusable way do so by inventing a new concept, the broker, that does addressing, routing, and queuing. This results in a client/server protocol or a set of APIs on top of some undocumented protocol that allows applications to speak to this broker. Brokers are an excellent thing in reducing the complexity of large networks. But adding broker-based messaging to a product like Zookeeper would make it worse, not better. It would mean adding an additional big box, and a new single point of failure. A broker rapidly becomes a bottleneck and a new risk to manage. If the software supports it, we can add a second, third, and fourth broker and make some failover scheme. People do this. It creates more moving pieces, more complexity, and more things to break.";



/** need to free memory afterwards**/
char *generateString(int size)
{
    const char ARRAY[] = {'A', 'B', 'D', 'E', 'F', 'G', 'H', 'I'};
    const int ARRAY_SIZE = 8;
    char *mem = (char*)calloc(TEST_SIZE, 1);
    if (!mem)
    {
        printf ("Failed to allocate!\n");
        exit(-1);
    }

    int j = 0;
    for (int i = 0; i < size; ++i)
    {
        if (j == ARRAY_SIZE)
            j = 0;
        mem[i] = ARRAY[j];
        ++j;
    }
    mem[size-1] = '\0';

    return mem;
}

int main (void)
{
    //  Prepare our context and publisher
    void *context = zmq_ctx_new ();
    void *publisher = zmq_socket (context, ZMQ_PUB);

    char end_point [64];
    sprintf (end_point, "%s://*:5556", szPROTOCOL);
  //
    //int rc = zmq_bind (publisher, "tcp://*:5556");
    int rc = zmq_bind (publisher, end_point);
    int i = 0;
    assert (rc == 0);

    //  Initialize random number generator
    srandom ((unsigned) time (NULL));


    const char UPDATE_TOKEN[] = "{'msg_type' : 'ipc', 'name': 'update_token', 'data': {'token': '<token>'}}";

    char *bufferToSend = UPDATE_TOKEN;
    printf("LEN is %d", strlen(szBigBuffer));
    printf("Wait for key ....");
    getchar();

    while (1) {
        //  Get values that will fool the boss
//        int zipcode, temperature, relhumidity;
//        zipcode     = randof (100000);
//        //if (i % 2 == 0)
//            zipcode = 10001;
//        temperature = randof (215) - 80;
//        relhumidity = randof (50) + 10;

//        ++i;

        //  Send message to all subscribers
//        char update [20];
//        sprintf (update, "%05d %d %d", zipcode, temperature, relhumidity);
//        printf("SENT - %s \n", update);
        s_send (publisher, bufferToSend);
        s_sleep(40);
    }
    free(bufferToSend);
    zmq_close (publisher);
    zmq_ctx_destroy (context);
    return 0;
}
