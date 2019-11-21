#include <stdio.h>
#include <stdlib.h>
#include <jack/jack.h>

static int jack_process(jack_nframes_t nframes, void *arg);

jack_port_t *out[2];
jack_client_t *client;


static int jack_process(jack_nframes_t nframes, void *arg)
{
    jack_default_audio_sample_t *o[2];
    int n;
    float smp;

    o[0] = (jack_default_audio_sample_t*)jack_port_get_buffer(out[0], nframes);
    o[1] = (jack_default_audio_sample_t*)jack_port_get_buffer(out[1], nframes);

    for (n = 0; n < nframes; n++) {
        smp = (float)rand() / RAND_MAX;
        smp = (2 * smp) - 1;
        o[0][n] = smp;
        o[1][n] = smp;
    }

    return 0;
}

void audio_start(void)
{
    const char **ports;
    const char *client_name;
    const char *server_name;
    jack_options_t options;
    jack_status_t status;

    if (client != NULL) {
        fprintf(stderr, "JACK audio server seems to be started already\n");
        return;
    }

    options = JackNullOption;

    server_name=NULL;
    client_name="Monolith"; /* set the client name */

    client = jack_client_open(client_name, options, &status, server_name);

    if (client == NULL) {
        fprintf(stderr, "JACK has failed you.\n");
        if (status & JackServerFailed) {
            fprintf (stderr, "It was unable to connect to the JACK server\n");
        }
        return;
    }

    if (status & JackServerStarted) {
        fprintf(stderr, "JACK server started\n");
    }

    if (status & JackNameNotUnique) {
        client_name = jack_get_client_name(client);
        fprintf(stderr, "unique name `%s' assigned\n", client_name);
    }

    jack_set_process_callback (client, jack_process, NULL);

    out[0] = jack_port_register(client, "output1",
                                JACK_DEFAULT_AUDIO_TYPE,
                                JackPortIsOutput, 0);
    out[1] = jack_port_register (client, "output2",
                                    JACK_DEFAULT_AUDIO_TYPE,
                                    JackPortIsOutput, 0);

    if ((out[0] == NULL) || (out[1] == NULL)) {
        fprintf(stderr, "no more JACK ports available\n");
        return;
    }

    if (jack_activate(client)) {
        fprintf(stderr, "cannot activate client\n");
        return;
    }

    ports = jack_get_ports (client, NULL, NULL,
                            JackPortIsPhysical|JackPortIsInput);

    if (ports == NULL) {
        fprintf(stderr, "no physical playback ports\n");
        return;
    }

    if (jack_connect(client, jack_port_name(out[0]), ports[0])) {
        fprintf (stderr, "cannot connect output ports\n");
    }

    if (jack_connect(client, jack_port_name(out[1]), ports[1])) {
        fprintf (stderr, "cannot connect output ports\n");
    }

    jack_free (ports);
}

void audio_stop(void)
{
    if (client != NULL) {
        jack_client_close (client);
        client = NULL;
    }
}

int main(int argc, char *argv[])
{
    out[0] = NULL;
    out[1] = NULL;
    client = NULL;
    audio_start();
    printf("Hit any key to stop.\n");
    fgetc(stdin);
    audio_stop();
    return 0;
}
