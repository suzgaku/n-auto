
#ifndef TRAIN_INFO_H
#define TRAIN_INFO_H

uint8 train_info_save_train_speed(uint8 tr_id, uint8 tr_speed, bool dir);
uint8 train_info_save_rail_speed(uint8 tr_id, uint8 rail_speed);
uint8 train_info_load_speed(uint8 tr_id, bool *dir);

#endif /* TRAIN_INFO_H */
