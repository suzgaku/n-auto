/* 
 * File:   pwm_test.c
 * Author: suzuki.manabu
 *
 * Created on 2020/06/03, 22:29
 */

#include "define.h"
#include "motor.h"
#include "train_info.h"

/*
 * 
 */
#define SECTION_MAX         7
#define PWM_ELEMENT_MAX     2

struct section_control{
    uint8 train_id;
    bool direction;
    uint8 speed;
    uint8 count;
};

struct rail_control {
    uint8 control_id;
    uint8 direction[PWM_ELEMENT_MAX];
    uint8 pwm[PWM_ELEMENT_MAX];
    struct section_control section[SECTION_MAX];
};

struct rail_control cnt = {0};

#INT_TIMER4
void timer4(void)
{
    int inv_count;
    int i;
    uint8 pwm = 0;
     
     for (i = 0; i < SECTION_MAX; i++) {
         inv_count = SPEED_MAX - cnt.section[i].count;
         if (inv_count < cnt.section[i].speed) {
            pwm |= 1 << i; 
        }
        cnt.section[i].count++;
        if (cnt.section[i].count > SPEED_MAX) {
            cnt.section[i].count = 0;
        }
    }
    
    cnt.pwm[0] = cnt.direction[0] & pwm;
    cnt.pwm[1] = cnt.direction[1] & pwm;
    
    output_a(cnt.pwm[0] | ((cnt.pwm[1] & 0x40) << 1));
    output_c(cnt.pwm[1]);
}

void motor_set_control_id(uint8 ctl_id)
{
    cnt.control_id = ctl_id;
}

uint8 motor_get_control_id(void)
{
    return cnt.control_id;
}

void motor_set_train_id(uint8 no, uint8 tr_id)
{
    int8 speed;
    bool dir;
    
    cnt.section[no].train_id = tr_id;
    
    speed = train_info_load_speed(tr_id, &dir);

    motor_set_direction(tr_id, dir);
    motor_set_speed(tr_id, speed);
}

void motor_clear_train_id(uint8 no)
{
    cnt.section[no].speed = 0;
    cnt.section[no].train_id = DATA_UNKNOWN;
}

void motor_set_speed(uint8 tr_id, uint8 speed)
{
    int i;
    
    for (i = 0; i < SECTION_MAX; i++) {
        if (cnt.section[i].train_id == tr_id) {
            cnt.section[i].speed = speed;
            cnt.section[i].count = 0;
        }
    }
}

void motor_set_direction(uint8 tr_id, bool direction)
{
    int i;
    
    for (i = 0; i < SECTION_MAX; i++) {
        if (cnt.section[i].train_id == tr_id) {
            cnt.section[i].direction = direction;
    
            cnt.direction[0] &= ~(1 << i);
            cnt.direction[1] &= ~(1 << i);
            if (direction) {
                cnt.direction[1] |= 1 << i;
            } else {
                cnt.direction[0] |= 1 << i;
            }
        }
    }
}

void motor_set_sync(uint8 tr_id)
{
    int i;
    
    for (i = 0; i < SECTION_MAX; i++) {
        if (cnt.section[i].train_id == tr_id) {
            cnt.section[i].count = 0;
        }
    }
}

void motor_init(void) 
{
    set_tris_a(0x01); // 0:out
    set_tris_c(0x01); // 0:out

 //   setup_timer_4(T4_DIV_BY_4 | T2_CLK_INTERNAL, 250, 16); // 1kHz(pri, prx, post)
 //   setup_timer_4(T4_DIV_BY_4 | T2_CLK_INTERNAL, 200, 4); // 5kHz(pri, prx, post)
   setup_timer_4(T4_DIV_BY_4 | T2_CLK_INTERNAL, 250, 2); // 8kHz(pri, prx, post)

    enable_interrupts(INT_TIMER4);
    //enable_interrupts(GLOBAL);
    
 }

uint8 motor_is_on_train(uint8 tr_id)
{
    int i;
    
    for (i = 0; i < SECTION_MAX; i++) {
        if (cnt.section[i].train_id == tr_id) {
            return i;
        }
    }
    
    return DATA_UNKNOWN;
}
