#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/net/wifi_mgmt.h>
#include <zephyr/net/net_if.h>

#define WIFI_SSID "ZEPHYR_TEST"
#define WIFI_PSK  "12345678"

static void wifi_connect(void)
{
    struct net_if *iface = net_if_get_default();
    struct wifi_connect_req_params params = {0};

    params.ssid = WIFI_SSID;
    params.ssid_length = strlen(WIFI_SSID);
    params.psk = WIFI_PSK;
    params.psk_length = strlen(WIFI_PSK);
    params.security = WIFI_SECURITY_TYPE_PSK;
    params.channel = WIFI_CHANNEL_ANY;

    printk("Requesting Wi-Fi connection...\n");

    net_mgmt(NET_REQUEST_WIFI_CONNECT,
             iface, &params, sizeof(params));
}

int main(void)
{
    wifi_connect();

    while (1) {
        k_sleep(K_SECONDS(1));
    }
}
