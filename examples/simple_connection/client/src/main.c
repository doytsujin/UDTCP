#include <errno.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#include "udtcp.h"

#define CLIENT_POLL_TIMEOUT (3 * 60 * 1000) /* 3 minutes */
#define CLIENT_WAIT_BETWEEN_TRY_CONNECTION (1) /* 1 second */

static int g_is_run = 0;

static void signal_stop(int sig_number)
{
    (void)sig_number;
    g_is_run = 0;
}

static void ini_signal(void)
{
    struct sigaction sig_action;

    sig_action.sa_handler = &signal_stop;
    sigemptyset(&sig_action.sa_mask);
    sig_action.sa_flags = 0;

    /* catch signal term actions */
    sigaction(SIGABRT, &sig_action, NULL);
    sigaction(SIGINT,  &sig_action, NULL);
    sigaction(SIGQUIT, &sig_action, NULL);
    sigaction(SIGTERM, &sig_action, NULL);
}

static void connect_callback(udtcp_client* client, udtcp_infos *infos)
{
    (void)client;
    fprintf(stdout,
        "CONNECT CALLBACK "
        "ip: %s, "
        "tcp port: %hu, "
        "udp server port: %hu, "
        "udp client port: %hu\n",
        infos->ip,
        infos->tcp_port,
        infos->udp_server_port,
        infos->udp_client_port);
}

static void disconnect_callback(udtcp_client* client, udtcp_infos *infos)
{
    (void)client;
    fprintf(stdout,
        "DISCONNECT CALLBACK "
        "ip: %s, "
        "tcp port: %hu, "
        "udp server port: %hu, "
        "udp client port: %hu\n",
        infos->ip,
        infos->tcp_port,
        infos->udp_server_port,
        infos->udp_client_port);
}

static void receive_tcp_callback(udtcp_client* client, udtcp_infos* infos, void* data, size_t data_size)
{
    (void)client;
    fprintf(stdout,
        "RECEIVE TCP CALLBACK "
        "ip: %s, "
        "tcp port: %hu > \"%.*s\"\n",
        infos->ip,
        infos->tcp_port,
        (int)data_size,
        (const char *)data);
}

static void receive_udp_callback(udtcp_client* client, udtcp_infos* infos, void* data, size_t data_size)
{
    (void)client;
    fprintf(stdout,
        "RECEIVE UDP CALLBACK "
        "ip: %s, "
        "udp port: %hu > [%i] \"%.*s\"\n",
        infos->ip,
        infos->udp_client_port,
        (int)infos->id,
        (int)data_size,
        (const char *)data);
}

static void log_callback(udtcp_client* client, enum udtcp_log_level_e level, const char* str)
{
    (void)client;
    static const char *level_to_str[3] = {"ERROR", "INFO ", "DEBUG"};
    fprintf(stdout, "[%s] %s\n", level_to_str[level], str);
}

int main(void)
{
    udtcp_client*       client;
    int                 is_connect = UDTCP_CONNECT_ERROR;

    g_is_run = 1;
    ini_signal();

    if (udtcp_create_client("0.0.0.0", 0, 0, 0, &client) == -1)
    {
        fprintf(stderr, "udtcp_create_client: %s\n", strerror(errno));
        return (1);
    }

    /* set callback */
    client->connect_callback = &connect_callback;
    client->disconnect_callback = &disconnect_callback;
    client->receive_tcp_callback = &receive_tcp_callback;
    client->receive_udp_callback = &receive_udp_callback;
    client->log_callback = &log_callback;

    while (g_is_run)
    {
        /* try to connect */
        while (g_is_run && is_connect != UDTCP_CONNECT_SUCCESS)
        {
            is_connect = udtcp_connect_client(client, "127.0.0.1", 4242, 5000);
            if (is_connect != UDTCP_CONNECT_SUCCESS)
                sleep(CLIENT_WAIT_BETWEEN_TRY_CONNECTION);
        }

        if (!g_is_run)
            break;

        /* create new poll thread */
        udtcp_start_client(client);

        /* wait */
        while (g_is_run && client->is_started)
        {
            sleep(1);
        }

        is_connect = UDTCP_CONNECT_ERROR;

        /* kill and join poll thread */
        udtcp_stop_client(client);
    }

    udtcp_delete_client(client);

    return (0);
}