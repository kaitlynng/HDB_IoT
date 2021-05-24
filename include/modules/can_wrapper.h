#ifndef CAN_WRAPPER_H_
#define CAN_WRAPPER_H_

#include <Arduino.h>
#include <ESP32CAN.h>
#include <CAN_config.h>

class CanWrapper 
{
public:
    CanWrapper(const gpio_num_t tx_pin, const gpio_num_t rx_pin, const int rx_queue_size = 151, const CAN_speed_t speed = CAN_SPEED_250KBPS)
    : m_tx_pin(tx_pin), m_rx_pin(rx_pin), m_rx_queue_size(rx_queue_size), m_speed(speed) {}

    CanWrapper() = delete;
    ~CanWrapper() {}

    void setup() {
        //can module setup
        CAN_cfg.speed = m_speed;
        CAN_cfg.tx_pin_id = m_tx_pin; //pin assignment
        CAN_cfg.rx_pin_id = m_rx_pin; //pin assignment
        
        CAN_cfg.rx_queue = xQueueCreate(m_rx_queue_size, sizeof(CAN_frame_t));
        // Init CAN Module
        ESP32Can.CANInit();
        Serial.println("Can BUS initialized success");
 
    }

    void poll(String data[3]) {
        String can_id;           //stores CANID

        String upper_bytes ;      //stores upper bytes of can msg
        String lower_bytes ;      //stores lower bytes of can msg

        String msb;                //stores upper bytes of can msg after translate
        String lsb;                //stores upper bytes of can msg after translate
        
        char modbus_data1[16];  //stores upper_bytes string hex into 16 chars and converts to floating point decimal 
        char modbus_data2[16];  

        union {
            uint32_t i;
            float f;
        } CAN_data;

        if(xQueueReceive(CAN_cfg.rx_queue, &rx_frame, 3 * portTICK_PERIOD_MS) == pdTRUE) {
            if (rx_frame.FIR.B.RTR != CAN_RTR) {
                can_id = String(rx_frame.MsgID,HEX); //get CANID
                upper_bytes = String(rx_frame.data.u8[7],HEX)+String(rx_frame.data.u8[6],HEX)+String(rx_frame.data.u8[5],HEX)+String(rx_frame.data.u8[4],HEX); //swaps endian to convert  ie  Data 0xA6 0x23 0xB7 0x3E 0xCC 0xCC 0xCC 0xBF [bfcccccc (-1.6) b7 b6 b5 b4]
                lower_bytes = String(rx_frame.data.u8[3],HEX)+String(rx_frame.data.u8[2],HEX)+String(rx_frame.data.u8[1],HEX)+String(rx_frame.data.u8[0],HEX); //swaps endians to convert  ie  Data 0xA6 0x23 0xB7 0x3E 0xCC 0xCC 0xCC 0xBF [3eb723a6 (0.36) b3 b2 b1 b0]
            }
        }

        upper_bytes.toCharArray(modbus_data1, 16);         //stores upper bytes of message tochar array for conversion to floatingpoint in union
        lower_bytes.toCharArray(modbus_data2, 16);       //stores lower bytes of message tochar array for conversion to floatingpoint in union

        CAN_data.i = strtoul(modbus_data1, NULL, 16);
        float upper = CAN_data.f;
        m_CAN_data.msb_i = CAN_data.i;
        m_CAN_data.msb_f = upper;
        static char message_data1[8];
        dtostrf(upper, 7, 2, message_data1);
        msb.concat(message_data1);
        
        CAN_data.i = strtoul(modbus_data2, NULL, 16);
        float lower= CAN_data.f;
        m_CAN_data.lsb_i = CAN_data.i;
        m_CAN_data.lsb_f = lower;
        static char message_data2[8];
        dtostrf(lower, 7, 2, message_data2);
        lsb.concat(message_data2);


        data[0] = can_id;
        data[1] = msb;
        data[2] = lsb;

        // respond to sender
        // ESP32Can.CANWriteFrame(&rx_frame);
    }

    float getMsbFloatCanValue(void){
        return m_CAN_data.msb_f;
    }

    float getLsbFloatCanValue(void){
        return m_CAN_data.lsb_f;
    } 


private:
    gpio_num_t m_tx_pin;
    gpio_num_t m_rx_pin;
    int m_rx_queue_size;
    CAN_speed_t m_speed;

    CAN_device_t CAN_cfg;                // CAN Config
    CAN_frame_t rx_frame;

    struct {
        uint32_t msb_i;
        uint32_t lsb_i;
        float msb_f;
        float lsb_f;
    } m_CAN_data;
};

#endif