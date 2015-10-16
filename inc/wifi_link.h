/*
 * wifi_link.h
 *
 *  Created on: Oct 5, 2015
 *      Author: micha
 */

#ifndef WIF_INC_WIFI_LINK_H_
#define WIF_INC_WIFI_LINK_H_
#define IPV4LEN 4


class wifi_link
{
    public:

	  wifi_link();
	  ~wifi_link();
    protected:
	  int16 init();
	  int16 wifi_connect(signed char* ssid_name, uint8 ssid_size);
	  int16 wifi_on_off(bool on);
    private:
	  bool wan_conn;
	  bool lan_conn;
	  uint8* ip_addr;
};

#endif /* WIF_INC_WIFI_LINK_H_ */
