


#ifndef MOTOR_H
#define MOTOR_H

#include "define.h"

void motor_set_control_id(uint8 ctl_id);
uint8 motor_get_control_id(void);
void motor_set_train_id(uint8 no, uint8 tr_id);
void motor_clear_train_id(uint8 no);
void motor_set_speed(uint8 tr_id, uint8 speed);
void motor_set_direction(uint8 tr_id, bool direction);
void motor_set_sync(uint8 tr_id);
void motor_init(void);
uint8 motor_is_on_train(uint8 tr_id);

#endif /* MOTOR_H */
