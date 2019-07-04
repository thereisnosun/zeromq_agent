//  Weather update client
//  Connects SUB socket to tcp://localhost:5556
//  Collects weather updates and finds avg temp in zipcode

#include "zhelpers.h"
#include <czmq.h>

#include "consts.h"

int main (int argc, char *argv [])
{
    //  Socket to talk to server
    printf ("Collecting updates from weather server...\n");
    void *context = zmq_ctx_new ();
    void *subscriber = zmq_socket (context, ZMQ_SUB);
    char end_point [64];
    sprintf (end_point, "%s://localhost:5556", szPROTOCOL);
    int rc = zmq_connect (subscriber, end_point);
    assert (rc == 0);

    //  Subscribe to zipcode, default is NYC, 10001
//    char *filter = (argc > 1)? argv [1]: "10001 ";
    rc = zmq_setsockopt (subscriber, ZMQ_SUBSCRIBE,
                         NULL, 0);
    assert (rc == 0);

    //  Process 100 updates
    int update_nbr;

    char buffer [TEST_SIZE];
    uint32_t totalMicroSec = 0;
    uint32_t max = 0;
    uint32_t min = 999999;
    for (update_nbr = 0; update_nbr < NUM_OF_PACKETS; update_nbr++) {
        uint64_t start = s_clock( );
        //char *string = s_recv (subscriber);
        int size = zmq_recv (subscriber, buffer, TEST_SIZE, 0);
        if (size == -1)
        {
            printf("Failed to read shit!");
            exit(-1);
        }
        uint64_t end = s_clock( );
        uint32_t timeTook = (uint32_t)(end - start);
        if (max != 0)
            totalMicroSec += timeTook;
        if (timeTook > max)
            max = timeTook;
        if (timeTook < min)
            min = timeTook;
        s_console("publisher took:%lu; SIZE=%d", timeTook, size);
        printf("RESULT = [%s]\n", buffer);
//        break;
    }
    const float average = (float)totalMicroSec / (float)NUM_OF_PACKETS;
    printf("//////////////\n\nRESULTS: average=%f max=%lu min=%lu samples=%d sample size=%d protocol=%s",
           average, max, min, NUM_OF_PACKETS, TEST_SIZE, szPROTOCOL);



    printf("Exiting...\n");
    zmq_close (subscriber);
    zmq_ctx_destroy (context);
    return 0;
}
