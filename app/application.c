#include <application.h>

// LED instance
bc_led_t led;

uint64_t my_device_address;
uint64_t peer_device_address;

// Button instance
bc_button_t button;
bc_button_t button_5s;

static bc_module_relay_t relay_0_0;


void button_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

        if (event == BC_BUTTON_EVENT_PRESS)
    {
        bc_led_pulse(&led, 100);

        static uint16_t event_count = 0;

        bc_radio_pub_push_button(&event_count);

        event_count++;
    }
    else if (event == BC_BUTTON_EVENT_HOLD)
    {
        static bool base = true;
        if (base)
            bc_radio_enrollment_start();
        else
            bc_radio_enroll_to_gateway();
        base = !base;

        bc_led_set_mode(&led, BC_LED_MODE_BLINK_FAST);
    }
}

static void button_5s_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param)
{
	(void) self;
	(void) event_param;

	if (event == BC_BUTTON_EVENT_HOLD)
	{
		bc_radio_enrollment_stop();

		uint64_t devices_address[BC_RADIO_MAX_DEVICES];

		// Read all remote address
		bc_radio_get_peer_devices_address(devices_address, BC_RADIO_MAX_DEVICES);

		for (int i = 0; i < BC_RADIO_MAX_DEVICES; i++)
		{
			if (devices_address[i] != 0)
			{
				// Remove device
				bc_radio_peer_device_remove(devices_address[i]);
			}
		}

		bc_led_pulse(&led, 2000);
	}
}

void radio_event_handler(bc_radio_event_t event, void *event_param)
{
    (void) event_param;

    bc_led_set_mode(&led, BC_LED_MODE_OFF);

    peer_device_address = bc_radio_get_event_device_address();

    if (event == BC_RADIO_EVENT_ATTACH)
    {
        bc_led_pulse(&led, 1000);
    }
    else if (event == BC_RADIO_EVENT_DETACH)
	{
		bc_led_pulse(&led, 3000);
	}
    else if (event == BC_RADIO_EVENT_ATTACH_FAILURE)
    {
        bc_led_pulse(&led, 10000);
    }

    else if (event == BC_RADIO_EVENT_INIT_DONE)
    {
        my_device_address = bc_radio_get_device_address();
    }
}

void bc_radio_on_push_button(uint64_t *peer_device_address, uint16_t *event_count)
{
	(void) peer_device_address;
	(void) event_count;

    bc_led_pulse(&led, 100);
}





void bc_radio_on_relay(uint64_t *peer_device_address, uint16_t *onoff)
{
    (void) peer_device_address;

    bc_led_pulse(&led, 10);

    bool state;
    state = (bool)*onoff;
    bc_module_relay_set_state(&relay_0_0, state);
}

void application_init(void)
{
    // Initialize LED
    bc_led_init(&led, BC_GPIO_LED, false, false);

    // Initialize button
    bc_button_init(&button, BC_GPIO_BUTTON, BC_GPIO_PULL_DOWN, false);
    bc_button_set_event_handler(&button, button_event_handler, NULL);

    bc_button_init(&button_5s, BC_GPIO_BUTTON, BC_GPIO_PULL_DOWN, false);
    bc_button_set_event_handler(&button_5s, button_5s_event_handler, NULL);
    bc_button_set_hold_time(&button_5s, 5000);

    // Initialize relay
    bc_module_relay_init(&relay_0_0, BC_MODULE_RELAY_I2C_ADDRESS_DEFAULT);
    bc_module_relay_set_state(&relay_0_0, BC_MODULE_RELAY_STATE_FALSE);

    // Initialize radio
    bc_radio_init();
    bc_radio_set_event_handler(radio_event_handler, NULL);
    bc_radio_listen();

}
