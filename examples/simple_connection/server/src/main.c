#include <errno.h>
#include <signal.h>
#include <string.h>

#include "udtcp.h"

#define SERVER_POLL_TIMEOUT (3 * 60 * 1000) /* 3 minutes */

static int g_is_run = 0;

static void signalStop(int sig_number)
{
    (void)sig_number;
    g_is_run = 0;
}

static void ini_signal(void)
{
    struct sigaction sig_action;

    sig_action.sa_handler = &signalStop;
    sigemptyset (&sig_action.sa_mask);
    sig_action.sa_flags = 0;

    /* catch signal term actions */
    sigaction (SIGABRT, &sig_action, NULL);
    sigaction (SIGINT,  &sig_action, NULL);
    sigaction (SIGQUIT, &sig_action, NULL);
    sigaction (SIGSTOP, &sig_action, NULL);
    sigaction (SIGTERM, &sig_action, NULL);
}

static void connect_callback(struct udtcp_server_s* server, udtcp_infos* infos)
{
    (void)server;
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

static void disconnect_callback(struct udtcp_server_s* server, udtcp_infos* infos)
{
    (void)server;
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

static void receive_tcp_callback(struct udtcp_server_s* server, udtcp_infos* infos, void* data, size_t data_size)
{
    (void)server;
    fprintf(stdout,
        "RECEIVE TCP CALLBACK "
        "ip: %s, "
        "tcp port: %hu > \"%.*s\"\n",
        infos->ip,
        infos->tcp_port,
        (int)data_size,
        (const char *)data);
}

static void receive_udp_callback(struct udtcp_server_s* server, udtcp_infos* infos, void* data, size_t data_size)
{
    (void)server;
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

static void log_callback(struct udtcp_server_s* server, enum udtcp_log_level_e level, const char* str)
{
    (void)server;
    static const char *level_to_str[3] = {"ERROR", "INFO ", "DEBUG"};
    fprintf(stdout, "[%s] %s\n", level_to_str[level], str);
}

int main(void)
{
    udtcp_server* server;

    g_is_run = 1;
    ini_signal();

    if (udtcp_create_server("0.0.0.0", 4242, 4243, 4244, &server) == -1)
    {
        fprintf(stderr, "udtcp_create_server: %s\n", strerror(errno));
        return (1);
    }

    server->connect_callback = &connect_callback;
    server->disconnect_callback = &disconnect_callback;
    server->receive_tcp_callback = &receive_tcp_callback;
    server->receive_udp_callback = &receive_udp_callback;
    server->log_callback = &log_callback;

    while (g_is_run)
    {
        enum udtcp_poll_e ret_poll = udtcp_server_poll(server, SERVER_POLL_TIMEOUT);
        if (ret_poll == UDTCP_POLL_ERROR || ret_poll == UDTCP_POLL_SIGNAL)
            break;
    }

    udtcp_delete_server(&server);

    return (0);
}