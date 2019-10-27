#include <gtest/gtest.h>
#include "../mock_weak.h"

#include "udtcp.h"

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;

#define FUNCTIONNAL_TEST() std::cout << "\033[32m[==========]\n[ RUN TEST ] " << std::string(67, '-') << "\033[0m" << std::endl;
#define DGTEST_TEST(a, b) GTEST_TEST(a, DISABLED_##b)

GTEST_TEST(udtcp, start_stop)
{
    udtcp_server *server = nullptr;
    udtcp_client *client = nullptr;

    EXPECT_EQ(0, udtcp_create_server("0.0.0.0", 4242, 0, 0, &server));
    EXPECT_EQ(0, udtcp_create_client("0.0.0.0", 0, 0, 0, &client));

    EXPECT_EQ(0, udtcp_start_server(server));
    usleep(1000);
    EXPECT_EQ(0, udtcp_connect_client(client, "127.0.0.1", server->server_infos->tcp_port, 1000));
    usleep(1000);

    EXPECT_EQ(0, udtcp_start_client(client));
    usleep(1000);
    udtcp_stop_client(client);
    udtcp_delete_client(&client);
    udtcp_stop_server(server);
    udtcp_delete_server(&server);
}

GTEST_TEST(udtcp, tcp_request)
{
    udtcp_server *server = nullptr;
    udtcp_client *client = nullptr;

    EXPECT_EQ(0, udtcp_create_server("0.0.0.0", 4242, 0, 0, &server));
    EXPECT_EQ(0, udtcp_create_client("0.0.0.0", 0, 0, 0, &client));

    EXPECT_EQ(0, udtcp_start_server(server));
    usleep(1000);
    EXPECT_EQ(0, udtcp_connect_client(client, "127.0.0.1", server->server_infos->tcp_port, 1000));
    usleep(1000);

    EXPECT_EQ(0, udtcp_start_client(client));
    usleep(1000);

    udtcp_send_tcp(client->server_infos, "test", 5);
    udtcp_send_tcp(&(server->clients_infos[0]), "test", 5);

    udtcp_stop_client(client);
    udtcp_delete_client(&client);
    udtcp_stop_server(server);
    udtcp_delete_server(&server);
}

static void signalStop(int sig_number)
{
    (void)sig_number;
}

static void ini_signal(void)
{
    struct sigaction sig_action;

    sig_action.sa_handler = &signalStop;
    sigemptyset(&sig_action.sa_mask);
    sig_action.sa_flags = 0;

    /* catch signal term actions */
    sigaction(SIGABRT, &sig_action, NULL);
    sigaction(SIGINT,  &sig_action, NULL);
    sigaction(SIGQUIT, &sig_action, NULL);
    sigaction(SIGTERM, &sig_action, NULL);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    ini_signal();
    FUNCTIONNAL_TEST();
    return RUN_ALL_TESTS();
}