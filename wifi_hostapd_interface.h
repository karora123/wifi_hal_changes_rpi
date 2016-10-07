#define HOSTAPD_FNAME "/etc/hostapd"
#define SEC_FNAME "/etc/sec_file.txt"
enum hostap_names
{
	ssid=0,
	passphrase=1,
};
struct params
{
	 char name[32];
	 char value[32];
};
struct hostap_conf
{
	char ssid[32];
	char *passphrase;
	char *wpa_pairwise;
	char *wpa;
	char *wpa_keymgmt;
};
