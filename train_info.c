
#include <define.h>

#define TRAIN_TABLE_NEWEST_MAX 255

struct train_control {
    uint8 id;
    uint8 train_speed;
    uint8 rail_speed;
    bool direction;
    uint8 newest;
};

struct train_control tr_cnt[TRAIN_INFO_TABLE_MAX] = {0};


static uint8 get_oldest_train(void)
{
    int i;
    uint8 oldest = TRAIN_TABLE_NEWEST_MAX;
    /*
     * ���̊֐��͍ŌẪ��X�g���������A�e�[�u���S�̂��g�p���ċL�^�������A
     * �g���Ă��Ȃ�index��D��I�ɓ������̂ŁA���߂Ŏ擾����index���L�^���Ă����B
     */
    static uint8 first_check_index = TRAIN_INFO_TABLE_MAX;
    
    for (i = 0; i < TRAIN_INFO_TABLE_MAX; i++) {
        if (tr_cnt[i].newest < oldest) {
            oldest = tr_cnt[i].newest;
        }
    }
    
    first_check_index++;
    for (; first_check_index < TRAIN_INFO_TABLE_MAX; first_check_index++) {
        if (tr_cnt[first_check_index].newest == oldest) {
            return first_check_index;
        }
    }
    for (first_check_index = 0; 
            first_check_index < TRAIN_INFO_TABLE_MAX; 
            first_check_index++) {
        if (tr_cnt[first_check_index].newest == oldest) {
            return first_check_index;
        }
    }
    /* �����ɂ͓��B���Ȃ��͂������A���Ă��܂����ꍇ��index��0��Ԃ� */
    first_check_index = 0;
    return 0;
}

static uint8 get_index_from_train(uint8 tr_id)
{
    int i;

    for (i = 0; i < TRAIN_INFO_TABLE_MAX; i++) {
        if (tr_cnt[i].id == tr_id) {
            tr_cnt[i].newest = TRAIN_TABLE_NEWEST_MAX;
            return i;
        }
        if (tr_cnt[i].newest > 0) {
            tr_cnt[i].newest--;
        }
    }
    
    return DATA_UNKNOWN;
}

uint8 train_info_save_train_speed(uint8 tr_id, uint8 tr_speed, bool dir)
{
    int i;

    i = get_index_from_train(tr_id);
    
    if (i == DATA_UNKNOWN) {
        i = get_oldest_train();
        tr_cnt[i].id = tr_id;
        tr_cnt[i].newest = TRAIN_TABLE_NEWEST_MAX;
    }
    
    tr_cnt[i].train_speed = tr_speed;
    tr_cnt[i].direction =dir;

    return MIN(tr_cnt[i].rail_speed, tr_speed);
}

uint8 train_info_save_rail_speed(uint8 tr_id, uint8 rail_speed)
{
    int i;

    i = get_index_from_train(tr_id);
    
    if (i == DATA_UNKNOWN) {
        i = get_oldest_train();
        tr_cnt[i].id = tr_id;
        tr_cnt[i].newest = TRAIN_TABLE_NEWEST_MAX;
    }
    
    tr_cnt[i].rail_speed = rail_speed;
    
    return MIN(tr_cnt[i].train_speed, rail_speed);
}

uint8 train_info_load_speed(uint8 tr_id, bool *dir)
{
    int i;

    i = get_index_from_train(tr_id);

    *dir = tr_cnt[i].direction;

    return MIN(tr_cnt[i].train_speed,  tr_cnt[i].rail_speed);
}

