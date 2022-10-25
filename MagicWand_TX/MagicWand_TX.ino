/**
 * @brief This example uses username and password authentication to connect to a
 * MQTT server.
 */

#include <Arduino.h>

#include <ecc608.h>
#include <led_ctrl.h>
#include <log.h>
#include <lte.h>
#include <mqtt_client.h>
#include <sequans_controller.h>

#define MQTT_PUB_TOPIC "magicwand"

#define MQTT_THING_NAME    "wand1"
#define MQTT_BROKER        "6de6ef5108aa44d390ca2e675d9d6aad.s1.eu.hivemq.cloud"
#define MQTT_PORT          8883
#define MQTT_USE_TLS       true
#define MQTT_KEEPALIVE     60
#define MQTT_USE_ECC       false
#define HIVEMQ_USERNAME "magicwand_tx"
#define HIVEMQ_PASSWORD "Rossisboss4"

#define LOG Serial3

static uint32_t counter = 0;

void TLS_config(void);
void connect_lte(void);
void connect_mqtt(void);

void setup() {
    Log.begin(115200);
    Log.setLogLevelStr("debug");
    LedCtrl.begin();
    LedCtrl.startupCycle();

    Log.info("Starting initialization of MQTT with username and password");
    TLS_config();
    connect_lte();
    connect_mqtt();

    Log.rawf(" OK!\r\n");
}

void loop() {

    String message_to_publish = "Plus";

    bool publishedSuccessfully = MqttClient.publish(MQTT_PUB_TOPIC,
                                                    message_to_publish.c_str());

    if (publishedSuccessfully) {
        Log.infof("Published message: %s\r\n", message_to_publish.c_str());
        counter++;
    } else {
        Log.error("Failed to publish");
    }

    delay(10000);
}

void TLS_config(void)
{
    SequansController.begin();
    Log.info(">> Setting up security profile for MQTT TLS without ECC");

    const char* command = "AT+SQNSPCFG=2,2,\"\",0,1";
    
    SequansController.writeBytes((uint8_t*)command, strlen(command), true);

    if (!SequansController.waitForURC("SQNSPCFG", NULL, 0, 4000)) {
        Log.infof(">> Error: Failed to set security profile\r\n");
        return;
    }
    Log.info(">> TLS config complete");
    SequansController.end();

}

void connect_lte(void)
{
    Log.info(">> LTE connect");

    // Establish LTE connection
    if (!Lte.begin()) 
    {
        Log.error(">> LTE connect fail\r\n");
        // Halt here
        while (1) {}
    }
   } // end connect lte

void connect_mqtt(void)
{
    int connect_attempt = 0;
  // Connect to the MGtt broker
    MqttClient.begin(MQTT_THING_NAME,MQTT_BROKER,MQTT_PORT,MQTT_USE_TLS,MQTT_KEEPALIVE,MQTT_USE_ECC,HIVEMQ_USERNAME,
                         HIVEMQ_PASSWORD);
    

    Log.infof(">> broker connect");
    while (!MqttClient.isConnected() && connect_attempt < 10) 
      {
            Log.rawf(".");
            // LedCtrl.toggle(Led::CON);
            delay(1000);
            connect_attempt++;
          
        }
        
    if(connect_attempt < 10)
      {
          Log.rawf(">> broker connected");
          delay(5000);
          /*
          MqttClient.end();
          Log.rawf("hanging up\r\n");
          Lte.end();
          */              
       }
    else 
      {
        Log.rawf("\r\n");
        Log.error(">> broker connect fail, \r\n>> terminating LTE connection\r\n");
        Lte.end();
      }
}
