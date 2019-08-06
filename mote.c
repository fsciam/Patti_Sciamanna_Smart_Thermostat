#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"
#include "contiki-net.h"
#include "erbium.h"
#if WITH_COAP == 3
#include "er-coap-03.h"
#elif WITH_COAP == 7
#include "er-coap-07.h"
#elif WITH_COAP == 12
#include "er-coap-12.h"
#elif WITH_COAP == 13
#include "er-coap-13.h"
#else
#warning "Erbium example without CoAP-specifc functionality"
#endif /* CoAP-specific example */
static int16_t temperature=20;
/**************************************************************************************************************/
PERIODIC_RESOURCE(temperature,METHOD_GET, "sensors/temp", "title=\"Temperature\";obs", 5*CLOCK_SECOND);

void
temperature_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  REST.set_header_content_type(response, REST.type.TEXT_PLAIN);

  /* Usually, a CoAP server would response with the resource representation matching the periodic_handler. */
  const char *msg = "It's periodic!";
  REST.set_response_payload(response, msg, strlen(msg));

  /* A post_handler that handles subscriptions will be called for periodic resources by the REST framework. */
}
void
temperature_periodic_handler(resource_t *r)
{
  temperature--;
  char message[100];
  coap_packet_t notification[1]; /* This way the packet can be treated as pointer as usual. */
  coap_init_message(notification, COAP_TYPE_NON, REST.status.OK, 0 );
  coap_set_payload(notification, message, sprintf(message,(int)sizeof(message),"{\"room\":\"room1\"; \"temperature\":\"%d\"}",temperature));
  REST.notify_subscribers(r, 0, notification);

}
/***************************************************************************************************************/

PROCESS(thermostat, "Thermostate mote");
AUTOSTART_PROCESSES(&thermostat);

PROCESS_THREAD(thermostat, ev, data)
{
  PROCESS_BEGIN();

  	

  /* Initialize the REST engine. */
  rest_init_engine();

  /* Activate the application-specific resources. */

  rest_activate_resource(&resource_temperature);
   while(1) {
    PROCESS_WAIT_EVENT();
    }
    PROCESS_END();
   }

