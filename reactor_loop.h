#pragma once
#include "czmq.h"


void zloop_test()
{
    // Create two PAIR sockets and connect over inproc
    zsock_t *output = zsock_new (ZMQ_PAIR);
    assert (output);
    zsock_bind (output, "inproc://zloop.test");

    zsock_t *input = zsock_new (ZMQ_PAIR);
    assert (input);
    zsock_connect (input, "inproc://zloop.test");

    zloop_t *loop = zloop_new ();
    assert (loop);
    zloop_set_verbose (loop, verbose);

    // Create a timer that will be cancelled
    int timer_id = zloop_timer (loop, 1000, 1, s_timer_event, NULL);
    zloop_timer (loop, 5, 1, s_cancel_timer_event, &timer_id);

    // After 20 msecs, send a ping message to output3
    zloop_timer (loop, 20, 1, s_timer_event, output);

    // Set up some tickets that will never expire
    zloop_set_ticket_delay (loop, 10000);
    void *ticket1 = zloop_ticket (loop, s_timer_event, NULL);
    void *ticket2 = zloop_ticket (loop, s_timer_event, NULL);
    void *ticket3 = zloop_ticket (loop, s_timer_event, NULL);

    // When we get the ping message, end the reactor
    rc = zloop_reader (loop, input, s_socket_event, NULL);
    assert (rc == 0);
    zloop_reader_set_tolerant (loop, input);
    zloop_start (loop);

    zloop_ticket_delete (loop, ticket1);
    zloop_ticket_delete (loop, ticket2);
    zloop_ticket_delete (loop, ticket3);

    // Check whether loop properly ignores zsys_interrupted flag
    // when asked to
    zloop_destroy (&loop);
    loop = zloop_new ();

    bool timer_event_called = false;
    zloop_timer (loop, 1, 1, s_timer_event3, &timer_event_called);

    zsys_interrupted = 1;
    zloop_start (loop);
    // zloop returns immediately without giving any handler a chance to run
    assert (!timer_event_called);

    zloop_set_nonstop (loop, true);
    zloop_start (loop);
    // zloop runs the handler which will terminate the loop
    assert (timer_event_called);
    zsys_interrupted = 0;

    // cleanup
    zloop_destroy (&loop);
    assert (loop == NULL);

    zsock_destroy (&input); zsock_destroy (&output);
}



void zsock_test()
{
    zsock_t *writer = zsock_new_push ("@tcp://127.0.0.1:5560");
    assert (writer);
    assert (zsock_resolve (writer) != writer);
    assert (streq (zsock_type_str (writer), "PUSH"));

    int rc;
    #if (ZMQ_VERSION >= ZMQ_MAKE_VERSION (3, 2, 0))
    // Check unbind
    rc = zsock_unbind (writer, "tcp://127.0.0.1:%d", 5560);
    assert (rc == 0);

    // In some cases and especially when running under Valgrind, doing
    // a bind immediately after an unbind causes an EADDRINUSE error.
    // Even a short sleep allows the OS to release the port for reuse.
    zclock_sleep (100);

    // Bind again
    rc = zsock_bind (writer, "tcp://127.0.0.1:%d", 5560);
    assert (rc == 5560);
    assert (streq (zsock_endpoint (writer), "tcp://127.0.0.1:5560"));
    #endif

    zsock_t *reader = zsock_new_pull (">tcp://127.0.0.1:5560");
    assert (reader);
    assert (zsock_resolve (reader) != reader);
    assert (streq (zsock_type_str (reader), "PULL"));

    // Basic Hello, World
    zstr_send (writer, "Hello, World");
    zmsg_t *msg = zmsg_recv (reader);
    assert (msg);
    char *string = zmsg_popstr (msg);
    assert (streq (string, "Hello, World"));
    free (string);
    zmsg_destroy (&msg);

    // Test resolve libzmq socket
    #if (ZMQ_VERSION >= ZMQ_MAKE_VERSION (3, 2, 0))
    void *zmq_ctx = zmq_ctx_new ();
    #else
    void *zmq_ctx = zmq_ctx_new (1);
    #endif
    assert (zmq_ctx);
    void *zmq_sock = zmq_socket (zmq_ctx, ZMQ_PUB);
    assert (zmq_sock);
    assert (zsock_resolve (zmq_sock) == zmq_sock);
    zmq_close (zmq_sock);
    zmq_ctx_term (zmq_ctx);

    // Test resolve zsock
    zsock_t *resolve = zsock_new_pub("@tcp://127.0.0.1:5561");
    assert (resolve);
    assert (zsock_resolve (resolve) == resolve->handle);
    zsock_destroy (&resolve);

    // Test resolve FD
    SOCKET fd = zsock_fd (reader);
    assert (zsock_resolve ((void *) &fd) == NULL);

    // Test binding to ephemeral ports, sequential and random
    int port = zsock_bind (writer, "tcp://127.0.0.1:*");
    assert (port >= DYNAMIC_FIRST && port <= DYNAMIC_LAST);
    port = zsock_bind (writer, "tcp://127.0.0.1:*[50000-]");
    assert (port >= 50000 && port <= DYNAMIC_LAST);
    port = zsock_bind (writer, "tcp://127.0.0.1:*[-50001]");
    assert (port >= DYNAMIC_FIRST && port <= 50001);
    port = zsock_bind (writer, "tcp://127.0.0.1:*[60000-60050]");
    assert (port >= 60000 && port <= 60050);

    port = zsock_bind (writer, "tcp://127.0.0.1:!");
    assert (port >= DYNAMIC_FIRST && port <= DYNAMIC_LAST);
    port = zsock_bind (writer, "tcp://127.0.0.1:![50000-]");
    assert (port >= 50000 && port <= DYNAMIC_LAST);
    port = zsock_bind (writer, "tcp://127.0.0.1:![-50001]");
    assert (port >= DYNAMIC_FIRST && port <= 50001);
    port = zsock_bind (writer, "tcp://127.0.0.1:![60000-60050]");
    assert (port >= 60000 && port <= 60050);

    // Test zsock_attach method
    zsock_t *server = zsock_new (ZMQ_DEALER);
    assert (server);
    rc = zsock_attach (server, "@inproc://myendpoint,tcp://127.0.0.1:5556,inproc://others", true);
    assert (rc == 0);
    rc = zsock_attach (server, "", false);
    assert (rc == 0);
    rc = zsock_attach (server, NULL, true);
    assert (rc == 0);
    rc = zsock_attach (server, ">a,@b, c,, ", false);
    assert (rc == -1);
    zsock_destroy (&server);

    // Test zsock_endpoint method
    rc = zsock_bind (writer, "inproc://test.%s", "writer");
    assert (rc == 0);
    assert (streq (zsock_endpoint (writer), "inproc://test.writer"));

    // Test error state when connecting to an invalid socket type
    // ('txp://' instead of 'tcp://', typo intentional)
    rc = zsock_connect (reader, "txp://127.0.0.1:5560");
    assert (rc == -1);

    // Test signal/wait methods
    rc = zsock_signal (writer, 123);
    assert (rc == 0);
    rc = zsock_wait (reader);
    assert (rc == 123);

    // Test zsock_send/recv pictures
    uint8_t number1 = 123;
    uint16_t number2 = 123 * 123;
    uint32_t number4 = 123 * 123;
    number4 *= 123;
    uint32_t number4_MAX = UINT32_MAX;
    uint64_t number8 = 123 * 123;
    number8 *= 123;
    number8 *= 123;
    uint64_t number8_MAX = UINT64_MAX;

    zchunk_t *chunk = zchunk_new ("HELLO", 5);
    assert (chunk);
    zframe_t *frame = zframe_new ("WORLD", 5);
    assert (frame);
    zhashx_t *hash = zhashx_new ();
    assert (hash);
    zuuid_t *uuid = zuuid_new ();
    assert (uuid);
    zhashx_set_destructor (hash, (zhashx_destructor_fn *) zstr_free);
    zhashx_set_duplicator (hash, (zhashx_duplicator_fn *) strdup);
    zhashx_insert (hash, "1", "value A");
    zhashx_insert (hash, "2", "value B");
    char *original = "pointer";

    // Test zsock_recv into each supported type
    zsock_send (writer, "i124488zsbcfUhp",
     -12345, number1, number2, number4, number4_MAX,
     number8, number8_MAX,
     "This is a string", "ABCDE", 5,
     chunk, frame, uuid, hash, original);
    char *uuid_str = strdup (zuuid_str (uuid));
    zchunk_destroy (&chunk);
    zframe_destroy (&frame);
    zuuid_destroy (&uuid);
    zhashx_destroy (&hash);

    int integer;
    byte *data;
    size_t size;
    char *pointer;
    number8_MAX = number8 = number4_MAX = number4 = number2 = number1 = 0ULL;
    rc = zsock_recv (reader, "i124488zsbcfUhp",
     &integer, &number1, &number2, &number4, &number4_MAX,
     &number8, &number8_MAX, &string, &data, &size, &chunk,
     &frame, &uuid, &hash, &pointer);
    assert (rc == 0);
    assert (integer == -12345);
    assert (number1 == 123);
    assert (number2 == 123 * 123);
    assert (number4 == 123 * 123 * 123);
    assert (number4_MAX == UINT32_MAX);
    assert (number8 == 123 * 123 * 123 * 123);
    assert (number8_MAX == UINT64_MAX);
    assert (streq (string, "This is a string"));
    assert (memcmp (data, "ABCDE", 5) == 0);
    assert (size == 5);
    assert (memcmp (zchunk_data (chunk), "HELLO", 5) == 0);
    assert (zchunk_size (chunk) == 5);
    assert (streq (uuid_str, zuuid_str (uuid)));
    assert (memcmp (zframe_data (frame), "WORLD", 5) == 0);
    assert (zframe_size (frame) == 5);
    char *value = (char *) zhashx_lookup (hash, "1");
    assert (streq (value, "value A"));
    value = (char *) zhashx_lookup (hash, "2");
    assert (streq (value, "value B"));
    assert (original == pointer);
    free (string);
    free (data);
    free (uuid_str);
    zframe_destroy (&frame);
    zchunk_destroy (&chunk);
    zhashx_destroy (&hash);
    zuuid_destroy (&uuid);

    // Test zsock_recv of short message; this lets us return a failure
    // with a status code and then nothing else; the receiver will get
    // the status code and NULL/zero for all other values
    zsock_send (writer, "i", -1);
    zsock_recv (reader, "izsbcfp",
     &integer, &string, &data, &size, &chunk, &frame, &pointer);
    assert (integer == -1);
    assert (string == NULL);
    assert (data == NULL);
    assert (size == 0);
    assert (chunk == NULL);
    assert (frame == NULL);
    assert (pointer == NULL);

    msg = zmsg_new ();
    zmsg_addstr (msg, "frame 1");
    zmsg_addstr (msg, "frame 2");
    zsock_send (writer, "szm", "header", msg);
    zmsg_destroy (&msg);

    zsock_recv (reader, "szm", &string, &msg);

    assert (streq ("header", string));
    assert (zmsg_size (msg) == 2);
    assert (zframe_streq (zmsg_first (msg), "frame 1"));
    assert (zframe_streq (zmsg_next (msg), "frame 2"));
    zstr_free (&string);
    zmsg_destroy (&msg);

    // Test zsock_recv with null arguments
    chunk = zchunk_new ("HELLO", 5);
    assert (chunk);
    frame = zframe_new ("WORLD", 5);
    assert (frame);
    zsock_send (writer, "izsbcfp",
     -12345, "This is a string", "ABCDE", 5, chunk, frame, original);
    zframe_destroy (&frame);
    zchunk_destroy (&chunk);
    zsock_recv (reader, "izsbcfp", &integer, NULL, NULL, NULL, &chunk, NULL, NULL);
    assert (integer == -12345);
    assert (memcmp (zchunk_data (chunk), "HELLO", 5) == 0);
    assert (zchunk_size (chunk) == 5);
    zchunk_destroy (&chunk);

    // Test zsock_bsend/brecv pictures with binary encoding
    frame = zframe_new ("Hello", 5);
    chunk = zchunk_new ("World", 5);

    msg = zmsg_new ();
    zmsg_addstr (msg, "Hello");
    zmsg_addstr (msg, "World");

    zsock_bsend (writer, "1248sSpcfm",
     number1, number2, number4, number8,
     "Hello, World",
     "Goodbye cruel World!",
     original,
     chunk, frame, msg);
    zchunk_destroy (&chunk);
    zframe_destroy (&frame);
    zmsg_destroy (&msg);

    number8 = number4 = number2 = number1 = 0;
    char *longstr;
    zsock_brecv (reader, "1248sSpcfm",
     &number1, &number2, &number4, &number8,
     &string, &longstr,
     &pointer,
     &chunk, &frame, &msg);
    assert (number1 == 123);
    assert (number2 == 123 * 123);
    assert (number4 == 123 * 123 * 123);
    assert (number8 == 123 * 123 * 123 * 123);
    assert (streq (string, "Hello, World"));
    assert (streq (longstr, "Goodbye cruel World!"));
    assert (pointer == original);
    zstr_free (&longstr);
    zchunk_destroy (&chunk);
    zframe_destroy (&frame);
    zmsg_destroy (&msg);

    #ifdef ZMQ_SERVER

    // Test zsock_bsend/brecv pictures with binary encoding on SERVER and CLIENT sockets
    server = zsock_new_server ("tcp://127.0.0.1:5561");
    assert (server);
    zsock_t* client = zsock_new_client ("tcp://127.0.0.1:5561");
    assert (client);

    // From client to server
    chunk = zchunk_new ("World", 5);
    zsock_bsend (client, "1248sSpc",
     number1, number2, number4, number8,
     "Hello, World",
     "Goodbye cruel World!",
     original,
     chunk);
    zchunk_destroy (&chunk);

    number8 = number4 = number2 = number1 = 0;
    zsock_brecv (server, "1248sSpc",
     &number1, &number2, &number4, &number8,
     &string, &longstr,
     &pointer,
     &chunk);
    assert (number1 == 123);
    assert (number2 == 123 * 123);
    assert (number4 == 123 * 123 * 123);
    assert (number8 == 123 * 123 * 123 * 123);
    assert (streq (string, "Hello, World"));
    assert (streq (longstr, "Goodbye cruel World!"));
    assert (pointer == original);
    assert (zsock_routing_id (server));
    zstr_free (&longstr);
    zchunk_destroy (&chunk);

    // From server to client
    chunk = zchunk_new ("World", 5);
    zsock_bsend (server, "1248sSpc",
     number1, number2, number4, number8,
     "Hello, World",
     "Goodbye cruel World!",
     original,
     chunk);
    zchunk_destroy (&chunk);

    number8 = number4 = number2 = number1 = 0;
    zsock_brecv (client, "1248sSpc",
     &number1, &number2, &number4, &number8,
     &string, &longstr,
     &pointer,
     &chunk);
    assert (number1 == 123);
    assert (number2 == 123 * 123);
    assert (number4 == 123 * 123 * 123);
    assert (number8 == 123 * 123 * 123 * 123);
    assert (streq (string, "Hello, World"));
    assert (streq (longstr, "Goodbye cruel World!"));
    assert (pointer == original);
    assert (zsock_routing_id (client) == 0);
    zstr_free (&longstr);
    zchunk_destroy (&chunk);

    zsock_destroy (&client);
    zsock_destroy (&server);

    #endif

    #ifdef ZMQ_SCATTER

    zsock_t* gather = zsock_new_gather ("inproc://test-gather-scatter");
    assert (gather);
    zsock_t* scatter = zsock_new_scatter ("inproc://test-gather-scatter");
    assert (scatter);

    rc = zstr_send (scatter, "HELLO");
    assert (rc == 0);

    char* message;
    message = zstr_recv (gather);
    assert (streq(message, "HELLO"));
    zstr_free (&message);

    zsock_destroy (&gather);
    zsock_destroy (&scatter);

    #endif

    // Check that we can send a zproto format message
    zsock_bsend (writer, "1111sS4", 0xAA, 0xA0, 0x02, 0x01, "key", "value", 1234);
    zgossip_msg_t *gossip = zgossip_msg_new ();
    zgossip_msg_recv (gossip, reader);
    assert (zgossip_msg_id (gossip) == ZGOSSIP_MSG_PUBLISH);
    zgossip_msg_destroy (&gossip);

    zsock_destroy (&reader); zsock_destroy (&writer);
}
