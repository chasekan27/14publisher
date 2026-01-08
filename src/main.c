#include <zephyr/kernel.h> 
#include <zephyr/sys/printk.h> 
#include <zephyr/net/net_if.h> 
#include <zephyr/net/net_mgmt.h> 
#include <zephyr/net/wifi_mgmt.h> 
#include <zephyr/net/wifi_mgmt.h> 
#define WIFI_SSID "ZEPHYR_TEST" 
#define WIFI_PSK "12345678" 
static struct net_if *iface; 
static bool wifi_connected; 
static struct net_mgmt_event_callback wifi_cb; 
static struct k_work_delayable wifi_retry_work; 

/* ---------- Wi-Fi connect ---------- */ 

static void wifi_connect(void) { 
    struct wifi_connect_req_params params = {0}; 
    params.ssid = WIFI_SSID; 
    params.ssid_length = strlen(WIFI_SSID); 
    params.psk = WIFI_PSK; 
    params.psk_length = strlen(WIFI_PSK); 
    params.security = WIFI_SECURITY_TYPE_PSK; 
    params.channel = 6; 
    printk("Requesting Wi-Fi connection...\n"); 
    net_mgmt(NET_REQUEST_WIFI_CONNECT, iface, &params, sizeof(params)); } 

/* ---------- Retry work ---------- */ 

static void wifi_retry_handler(struct k_work *work) { if (!wifi_connected) { printk("Retrying Wi-Fi...\n"); wifi_connect(); } } 

/* ---------- Event handler (CORRECT signature) ---------- */ 

static void wifi_event_handler(struct net_mgmt_event_callback *cb, uint64_t mgmt_event, struct net_if *iface) { if (mgmt_event & NET_EVENT_WIFI_CONNECT_RESULT) { if (cb->info) { printk("Wi-Fi connected ✅\n"); wifi_connected = true; } else { printk("Wi-Fi connect failed ❌\n"); k_work_schedule(&wifi_retry_work, K_SECONDS(5)); } } if (mgmt_event & NET_EVENT_WIFI_DISCONNECT_RESULT) { printk("Wi-Fi disconnected ❌\n"); wifi_connected = false; k_work_schedule(&wifi_retry_work, K_SECONDS(5)); } } 

/* ---------- Disconnect ---------- */ 

static void wifi_disconnect(void) { net_mgmt(NET_REQUEST_WIFI_DISCONNECT, iface, NULL, 0); } 

/* ---------- main ---------- */ 

int main(void) { iface = net_if_get_default(); k_work_init_delayable(&wifi_retry_work, wifi_retry_handler); net_mgmt_init_event_callback( &wifi_cb, wifi_event_handler, NET_EVENT_WIFI_CONNECT_RESULT | NET_EVENT_WIFI_DISCONNECT_RESULT ); printk("stage 1\n"); net_mgmt_add_event_callback(&wifi_cb); printk("stage 2\n"); /* allow ESP32 Wi-Fi firmware to start */ k_sleep(K_SECONDS(2)); printk("stage 3\n"); /* clear stale state after reset */ wifi_disconnect(); k_sleep(K_SECONDS(1)); printk("stage 4\n"); wifi_connect(); printk("stage 5\n"); while (1) { k_sleep(K_SECONDS(1)); } }