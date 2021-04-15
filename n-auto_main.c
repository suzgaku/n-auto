/* 
 * File:   n-auto_main.c
 * Author: suzuki.manabu
 *
 * Created on 2021/03/29, 23:50
 */
#include <18F26K83.h>
#include <define.h>
#include <communication.h>

#device WRITE_EEPROM = NOINT
#device ADC=12
//#fuses HS,NOWDT,NOPROTECT,NOLVP

#ifdef CONTROL

#else
//#include "motor.h"
#endif /* CONTROL */

#use delay(clock=64Mhz, crystal=16MHz)
//#use delay(crystal=16MHz)

#ifdef CONTROL
/* CAN */
#define CAN_TX_PIN    PIN_B2
#define CAN_RX_PIN    PIN_B3

/* SPI */
#pin_select SCK1=PIN_C7
#pin_select SDI1=PIN_B4
#pin_select SDO1=PIN_B5
#use spi (MASTER, SPI1, baud=5000000, MODE=0, BITS=8, STREAM=SPI_1)

#define IO_MULTIPLE_DEVICES
#define IO_CS_PIN PIN_A2

#include <control.h>
#include <n-auto_23s17.c>
#include <control_func.c>
#include <control_rail.c>
#include <control_train.c>

#else
#define CAN_TX_PIN    PIN_B0
#define CAN_RX_PIN    PIN_B1

#include <motor.c>
#endif /* CONTROL */

#include <can-pic18f_ecan.c>
#include <train_info.c>

int16 ms = 0;

#int_timer2
void isr_timer2(void) {
    ms++; //keep a running timer that increments every milli-second

#ifdef CONTROL    
    sensor_sampling();
#endif /* CONTROL */
}

/*
 * 
 */
#ifdef CONTROL
/**************************************************************************
 * CONTROL Main
 */

struct control_config ctrl = {0};

void main(int argc, char** argv) {

    CAN_RX_HEADER rx_header;
    uint8_t rx_data[8];

    uint8_t data; 

    int16 cmd;
    int8 ctl_id;
    
    setup_timer_2(T2_DIV_BY_4 | T2_CLK_INTERNAL, 250, 16); // 1kHz(pri, prx, post)
   
    can_init();
    
    enable_interrupts(INT_TIMER2);
    enable_interrupts(GLOBAL);

    spi_initialize();
    sensor_initialize();
    rail_initialize();
    train_initialize();
    
#ifdef USING_SENSOR_CALIBRATION    
    sensor_calibration();
#endif /* USING_SENSOR_CALIBRATION */
    
    while (1) {
       if (!own_comm_send_buffer_is_empty() || can_kbhit()) {
 
           (!own_comm_send_buffer_is_empty()) ?
                own_comm_send_buffer_read(&rx_header, rx_data) :
                can_getd(&rx_header, rx_data);

            cmd = rx_header.Id & COMM_CMD_MASK;
            ctl_id = rx_header.Id & COMM_CTL_ID_MASK;

            switch(cmd){
            case COMM_SYS_SETTING_MODE:
                break;
            case COMM_SYS_PAUSE_STOP:
                break;
            case COMM_SYS_MOTOR_CFG_WRITE:
                break;
            case COMM_SYS_EEPROM_INIT:
                break;
            case COMM_TRAIN_SPEED:
                rail_receive_train_speed(ctl_id, 
                        (struct comm_data_train_speed*)rx_data);
                break;
            case COMM_RAIL_ROUTE_SET:
            {    
                struct comm_data_rail_route_set *data = rx_data;
                
                if (data->set_control_id == ctrl.own.control_id) {
                    rail_receive_rail_route_set(ctl_id, data);
                }
            }
                break;
            case COMM_RAIL_SPEED:
                rail_receive_rail_speed(ctl_id, 
                        (struct comm_data_rail_speed*)rx_data);
                break;
            case COMM_RAIL_ON_TRAIN:
            {
                /* 強制コマンド */
                struct comm_data_rail_on_train *data = rx_data;
                
                if (data->on_control_id == ctrl.own.control_id) {
                    rail_receive_on_train(ctl_id, data);
                }
            }
                break;
            case COMM_RAIL_ENTRY_REQUEST:
            {
                struct comm_data_rail_entry_request *data = rx_data;
                
                if (data->to_control_id == ctrl.own.control_id) {
                    rail_receive_rail_entry_request(ctl_id, data);
                }
            }
                break;
            case COMM_RAIL_ENTRY_ACCEPT:
            {
                struct comm_data_rail_entry_accept *data = rx_data;
                
                /* data->from_control_id対する差別は不要 */
                if (data->owner_control_id == ctrl.own.control_id) {
                    rail_receive_rail_entry_accept(ctl_id, data);
                }

            }
                break;
            case COMM_RAIL_ENTRY:
            {
                struct comm_data_rail_entry *data = rx_data;
                
                if (data->from_control_id == ctrl.own.control_id) {
                    rail_receive_rail_entry(ctl_id, data);
                }
            }
                break;
            case COMM_RAIL_EXIT:
            {
                struct comm_data_rail_exit *data = rx_data;
                
                if (data->to_control_id == ctrl.own.control_id) {
                    rail_receive_rail_exit(ctl_id, data);
                }
            }
                break;
            case COMM_POINT_SET:
                break;
            case COMM_SIGNAL_SPEED:
                break;
            case COMM_SIGNAL_SPEED_SET:
                break;
            case COMM_SIGNAL_SET:
                break;
            case COMM_DEBUG_CONTROL:
                break;
            case COMM_DEBUG_MOTOR:

                break;
            }
            continue;
       }
       
       if (ms >= CYCLE_MS) {
           ms = 0;
           /* 周期処理 */
           rail_cycle_process();
           train_cycle_process();
           led_cycle_process();
           
       }
    } /* while loop */
}

#else 
/**************************************************************************
 * MOTOR Main
 */
void main(int argc, char** argv) {

   CAN_RX_HEADER rHeader;
   uint8_t rData[8];

   int16 cmd;
   int8 ctl_id;

   setup_timer_2(T2_DIV_BY_1 | T2_CLK_INTERNAL, 250, 16); // 1kHz(pri, prx, post)
   
   can_init();
   motor_init();
   
   output_float(PIN_B4);
   output_float(PIN_B3);
   
   enable_interrupts(INT_TIMER2);
   enable_interrupts(GLOBAL);
   
   while (TRUE) {
       if (can_kbhit()) {
            can_getd(&rHeader, rData);

            cmd = rHeader.Id & COMM_CMD_MASK;
            ctl_id = rHeader.Id & COMM_CTL_ID_MASK;
            switch(cmd){
            case COMM_SYS_PAUSE_STOP:
                break;
               
            case COMM_SYS_MOTOR_CFG_WRITE:
                break;
               
            case COMM_SYS_EEPROM_INIT:
                break;
               
            case COMM_TRAIN_SPEED:
            {
                uint8 speed;
                struct comm_data_train_speed *data = rData; 
                
                speed = train_info_save_train_speed(
                        data->train_id,
                        data->speed,
                        (data->direction == 1));

                motor_set_speed(data->train_id, speed);
            };
                break;
               
            case COMM_RAIL_SPEED:
             {
                uint8 speed;
                struct comm_data_rail_speed *data = rData; 
                
                speed = train_info_save_rail_speed(
                        data->train_id,
                        data->speed);

                motor_set_speed(data->train_id, speed);
            };
                break;
            case COMM_RAIL_ON_TRAIN:
             {
                struct comm_data_rail_on_train *data = rData; 

                if (ctl_id != motor_get_control_id()) {
                    break;
                }
                if (data->train_id == DATA_UNKNOWN) {
                    motor_clear_train_id(data->rail_id);
                } else {
                    motor_set_train_id(data->rail_id, data->train_id);
                }
             }   
            case COMM_RAIL_ENTRY_ACCEPT:
             {
                struct comm_data_rail_entry_accept *data = rData; 
                
                if (ctl_id != motor_get_control_id()) {
                    motor_set_sync(data->train_id);
                    break;
                }
                
                motor_set_train_id(data->rail_id, data->train_id);
            }
                break;
               
            case COMM_RAIL_EXIT:
             {
                struct comm_data_rail_exit *data = rData; 
                
                if (ctl_id != motor_get_control_id()) {
                    break;
                }
                
                motor_clear_train_id(data->rail_id);
            }
            
            case COMM_DEBUG_MAINTENANCE:
            {
                struct comm_data_debug_maintenance *data = rData;
                switch (data->command) {
                case 0x01:
                {
                    int i;
                    for (i = 0; i < SECTION_MAX; i++) {
                        motor_set_direction(i, 0);
                        motor_set_speed(i, data->data[0]);
                    }
                }
                    break;
                default:
                    break;
                }
            }
                break;
               
           default:
               break;
           }
       }
   }
}
#endif /* CONTROL */
