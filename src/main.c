#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/net_mgmt.h>
#include <zephyr/net/wifi_mgmt.h>
#include <zephyr/net/net_event.h>
#include <zephyr/net/net_ip.h>

#define WIFI_SSID "ZEPHYR_TEST"
#define WIFI_PSK  "12345678"
// #define WIFI_SSID "Lyptus-Tech"
// #define WIFI_PSK  "L#@!4Tech"

static struct net_if *iface;
static struct net_mgmt_event_callback wifi_cb;


/* ---------- Wi-Fi connect ---------- */
static void wifi_connect(void)
{
    struct wifi_connect_req_params params = {0};

    params.ssid = WIFI_SSID;
    params.ssid_length = strlen(WIFI_SSID);
    params.psk = WIFI_PSK;
    params.psk_length = strlen(WIFI_PSK);
    params.security = WIFI_SECURITY_TYPE_PSK;
    params.channel = 6;

    printk("Requesting Wi-Fi connection...\n");

    net_mgmt(NET_REQUEST_WIFI_CONNECT, iface,
             &params, sizeof(params));
}

bool ipv4_ready(void)
{
    struct net_if *iface = net_if_get_default();
    struct net_if_addr *ifaddr;

    ifaddr = net_if_ipv4_addr_lookup(NULL, &iface);
    if (!ifaddr) {
        return false;
    }

    /* Ignore link-local 169.254.x.x */
    if (net_ipv4_is_ll_addr(&ifaddr->address.in_addr)) {
        return false;
    }

    return true;
}

void print_ip(void)
{
    struct net_if *iface = net_if_get_default();
    struct net_if_addr *ifaddr;
    char ip_buf[NET_IPV4_ADDR_LEN];

    ifaddr = net_if_ipv4_addr_lookup(NULL, &iface);
    if (!ifaddr) {
        printk("IPv4 not ready\n");
        return;
    }

    printk("IPv4: %s\n",
           net_addr_ntop(AF_INET,
                         &ifaddr->address.in_addr,
                         ip_buf,
                         sizeof(ip_buf)));
}


/* ---------- Disconnect ---------- */
static void wifi_disconnect(void)
{
    net_mgmt(NET_REQUEST_WIFI_DISCONNECT, iface, NULL, 0);
    printk("Requested Wi-Fi disconnection\n");
}
static bool scan_requested;

static void wifi_event_handler(struct net_mgmt_event_callback *cb,
                               uint64_t mgmt_event,
                               struct net_if *iface)
{
    if (mgmt_event & NET_EVENT_WIFI_CONNECT_RESULT) {
        printk("Wi-Fi connected\n");

        if (!scan_requested) {
            printk("Requesting scan after connect\n");
            net_mgmt(NET_REQUEST_WIFI_SCAN, iface, NULL, 0);
            scan_requested = true;
        }
    }

    if (mgmt_event & NET_EVENT_WIFI_SCAN_RESULT) {
        const struct wifi_scan_result *res = cb->info;
        printk("SSID: %s | RSSI: %d\n", res->ssid, res->rssi);
    }

    if (mgmt_event & NET_EVENT_WIFI_SCAN_DONE) {
        printk("Scan done\n");
    }

    if (mgmt_event & NET_EVENT_IPV4_ADDR_ADD) {
        printk("IPv4 address event\n");
    }
}


/* ---------- main ---------- */
int main(void)
{
    iface = net_if_get_default();

    net_mgmt_init_event_callback(
        &wifi_cb,
        wifi_event_handler,
        NET_EVENT_WIFI_CONNECT_RESULT |
        NET_EVENT_IPV4_ADDR_ADD |
        NET_EVENT_WIFI_SCAN_RESULT |
        NET_EVENT_WIFI_SCAN_DONE
    );

    net_mgmt_add_event_callback(&wifi_cb);

    printk("Connecting Wi-Fi\n");
    wifi_connect();

    while (1) {
        k_sleep(K_SECONDS(1));
    }
}
