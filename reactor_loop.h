#pragma once



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
