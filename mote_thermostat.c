#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"
#include "contiki-net.h"
#include "erbium.h"
#include "er-coap-13.h"

#define SENDING_PERIOD 5
#define TEMPERATURE_PERIOD 3

static int16_t temperature;

static int8_t air_conditioning=0;
static int8_t heating_unit=0;
static int8_t ventilation_unit=0;
/**************************************************************************************************************/
RESOURCE(heating_opt,METHOD_GET | METHOD_POST, "actuators/heating", "title=\"Heating options\";rt=\"heating_opt\"");

void
heating_opt_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  
}

/***************************************************************************************************************/

PERIODIC_RESOURCE(temperature,METHOD_GET, "sensors/temp", "title=\"Temperature\";obs",  SENDING_PERIOD*CLOCK_SECOND);

void
temperature_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
	const uint16_t *accept = NULL;
  int num = REST.get_header_accept(request, &accept);
  
	if((num == 0) || (num && (accept[0]==REST.type.APPLICATION_JSON)))
  {
		REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
		snprintf((char *)buffer, REST_MAX_CHUNK_SIZE,"{\"room\":\"room1\"; \"temperature\":\"%d\"}",temperature);
		REST.set_response_payload(response, buffer, strlen((char *)buffer));
	}
	else
	{
		REST.set_response_status(response, REST.status.NOT_ACCEPTABLE);
    const char *msg = "Supporting content-types application/json";
    REST.set_response_payload(response, msg, strlen(msg));
	}

}
void
temperature_periodic_handler(resource_t *r)
{
  
  char message[100];
  coap_packet_t notification[1]; 
  coap_init_message(notification, COAP_TYPE_NON, REST.status.OK, 0 );
  coap_set_payload(notification, message, snprintf(message,(int)sizeof(message),"{\"room\":\"room1\"; \"temperature\":\"%d\"}",temperature));
  REST.notify_subscribers(r, 0, notification);

}
/***************************************************************************************************************/

PROCESS(thermostat, "Thermostate mote");
AUTOSTART_PROCESSES(&thermostat);

PROCESS_THREAD(thermostat, ev, data)
{	
	static struct etimer et;
	
  PROCESS_BEGIN();
  printf("Starting mote...\n");
  
  /*Initialize temperature, not a uniform distribution*/
  
  temperature=10+rand()%21;
  printf("Initialize temperature to %d Celsius degrees\n",temperature);

	/* set timer to expire after TEMPERATURE_PERIOD seconds to allow temperature simulation */
  etimer_set(&et, CLOCK_SECOND * TEMPERATURE_PERIOD);
	
  /* Initialize the REST engine. */
  rest_init_engine();

  /* Activate the application-specific resources. */

  rest_activate_periodic_resource(&periodic_resource_temperature);
   while(1) {
			PROCESS_WAIT_EVENT();
			if(etimer_expired(&et))
			{
				
				if(air_conditioning)
				{
					temperature-=ventilation_unit? 2 : 1;
				}
				else if(heating_unit)
				{
					temperature+=ventilation_unit? 2 : 1;
				}
				
				etimer_restart(&et);
			}
    }
    PROCESS_END();
   }

