#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"
#include "contiki-net.h"
#include "random.h"

#include "erbium.h"
#include "er-coap-13.h"

#include "dev/leds.h"
#include "sys/node-id.h"
#include "sys/clock.h"

#define SENDING_PERIOD 5
#define TEMPERATURE_PERIOD 3

static int16_t temperature;

static int8_t air_conditioning=0;
static int8_t heating_unit=0;
static int8_t ventilation_unit=0;
/**************************************************************************************************************/
RESOURCE(heating_opt,METHOD_GET | METHOD_POST, "actuators/heating", "title=\"Heating options: POST opt=0|1|2|3|4|5|6|7|8,\";rt=\"heating_opt\"");

void
heating_opt_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  if(REST.get_method_type(request) == METHOD_GET)
  {
		const uint16_t *accept = NULL;
		int num = REST.get_header_accept(request, &accept);

		if((num == 0) || (num && (accept[0]==REST.type.APPLICATION_JSON)))
		{
			REST.set_header_content_type(response, REST.type.APPLICATION_JSON);
			snprintf((char *)buffer, REST_MAX_CHUNK_SIZE,"{\"air_conditioning\":%d, \"heating_unit\":%d,\"ventilation\":%d}",air_conditioning,heating_unit,ventilation_unit);
			REST.set_response_payload(response, buffer, strlen((char *)buffer));
		}
		else
		{
			REST.set_response_status(response, REST.status.NOT_ACCEPTABLE);
			const char *msg = "Supporting content-types application/json";
			REST.set_response_payload(response, msg, strlen(msg));
		}
	}
	else
	{
	
		const char *opt = NULL;
		int option_selected;
		int success=0;
		int status=0;
	 	
	 	REST.get_post_variable(request, "opt", &opt);
	 	option_selected=atoi(opt);
		REST.set_header_content_type(response, REST.type.APPLICATION_JSON);
	
		printf("Received request for option: %d",option_selected);
	
	
	
	
			
			
			
			
		if(option_selected == 0 || option_selected == 3 || option_selected == 4)
		{
				if(!heating_unit || option_selected == 3 || option_selected == 4 )
				{
					if(option_selected == 0)
						air_conditioning=air_conditioning? 0 : 1;
					else if(option_selected == 3)
					{
						air_conditioning=1;
						heating_unit=0;
					}
					else
					{
						air_conditioning=0;
						heating_unit=0;
					}
					printf("Air conditioning: %d",air_conditioning);
				
					success=1;
					status=air_conditioning;
				}
			

		}
		else if(option_selected == 1 || option_selected == 5 || option_selected == 6)
		{
			
				if(!air_conditioning || option_selected == 5 || option_selected == 6)
				{
					if(option_selected == 1)
						heating_unit=heating_unit? 0 : 1;
					else if(option_selected == 5)
					{
						heating_unit=1;
						air_conditioning=0;
					}
					else
					{
						heating_unit=0;
						air_conditioning=0;
					}
					printf("Heating unit: %d",heating_unit);
				
					success=1;
					status=heating_unit;
				}
			
		}
		else if(option_selected == 2 || option_selected == 7 || option_selected == 8)
		{
				if(option_selected == 7)
					ventilation_unit=1;
				else if(option_selected == 8)
					ventilation_unit=0;
				else
					ventilation_unit=ventilation_unit? 0: 1;
				printf("Ventilation unit: %d",ventilation_unit);
				
				success=1;
				status=ventilation_unit;
		}
			

	
		if(success) 
		{
			REST.set_response_status(response, REST.status.OK);
		}
		else
		{
			REST.set_response_status(response, REST.status.BAD_REQUEST);
		}		
	
		snprintf((char *)buffer, REST_MAX_CHUNK_SIZE,"{\"opt\":%d, \"result\":%d,\"status\":%d}",option_selected,success,status);
		REST.set_response_payload(response, buffer, strlen((char *)buffer));
		if(air_conditioning)
		{
		
			leds_on(LEDS_BLUE);
		}
		else
		{
	
			leds_off(LEDS_BLUE);
		}
		if(heating_unit)
		{
		
			leds_on(LEDS_RED);
		}
		else
		{
		
			leds_off(LEDS_RED);
		}
	
		if(ventilation_unit)
		{
		
			leds_on(LEDS_GREEN);
		}
		else
		{
	
			leds_off(LEDS_GREEN);
		}

	}
}

/***************************************************************************************************************/

PERIODIC_RESOURCE(temperature,METHOD_GET, "sensors/temp", "title=\"Temperature\";obs",  SENDING_PERIOD*CLOCK_SECOND);

void
temperature_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
	
	snprintf((char *)buffer, REST_MAX_CHUNK_SIZE,"%d",temperature);
	REST.set_response_payload(response, buffer, strlen((char *)buffer));
	

}
void
temperature_periodic_handler(resource_t *r)
{
  static int order=0;
  char message[10];
  order++;
  coap_packet_t notification[1]; 
  coap_init_message(notification, COAP_TYPE_NON, REST.status.OK, 0 );
  coap_set_payload(notification, message, snprintf(message,(int)sizeof(message),"%d",temperature));
  REST.notify_subscribers(r, order, notification);

}
/***************************************************************************************************************/
long random_at_most(long max) {
  unsigned long
    
    num_bins = (unsigned long) max + 1,
    num_rand = (unsigned long) RANDOM_RAND_MAX + 1,
    bin_size = num_rand / num_bins,
    defect   = num_rand % num_bins;

  long x;
  do {
   x = random_rand();
  }
  
  while (num_rand - defect <= (unsigned long)x);

  
  return x/bin_size;
}
/***************************************************************************************************************/
PROCESS(thermostat, "Thermostate mote");
AUTOSTART_PROCESSES(&thermostat);

PROCESS_THREAD(thermostat, ev, data)
{	
	static struct etimer et;
	
  PROCESS_BEGIN();
  
  /*Initialize temperature, we tried to generate random numbers but, even if we've tried lot of algos,
  cooja simulation generate always the same set of numbers*/ 
  temperature=10+random_at_most(20);
  printf("Initialize temperature to %d Celsius degrees\n",temperature);

	/* set timer to expire after TEMPERATURE_PERIOD seconds to allow temperature simulation */
  etimer_set(&et, CLOCK_SECOND * TEMPERATURE_PERIOD);
	
  /* Initialize the REST engine. */
  rest_init_engine();

  /* Activate resources*/
  rest_activate_periodic_resource(&periodic_resource_temperature);
  rest_activate_resource(&resource_heating_opt);
   while(1) {
			PROCESS_WAIT_EVENT();
			if(etimer_expired(&et))
			{
				
				if(air_conditioning && temperature>0)
				{
					temperature-=ventilation_unit? 2 : 1;
				}
				else if(heating_unit && temperature<50)
				{
					temperature+=ventilation_unit? 2 : 1;
				}
				printf("Temperature: %d",temperature);
				etimer_restart(&et);
			}
    }
    PROCESS_END();
   }

