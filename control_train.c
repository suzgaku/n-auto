
#include <communication.h>
#include <control.h>

extern struct control_config ctrl;



void train_initialize(void)
{
    
#ifdef  USING_STATIC_CONFIG
   //send_comm_rail_route_set(0, 0, 1);
    //( rid,  on_ctl_id,  on_rid,  train_id,  max_sec,  head,  tail)
   send_comm_rail_on_train(0, 0, 0, 1, 1/*max_sec*/, 1, 1);
   //send_comm_rail_on_train(0, 0, 5, 2, 1/*max_sec*/, 1, 1);
   //send_comm_rail_on_train(0, 0, 4, 3, 1/*max_sec*/, 1, 1);
   
   delay_ms(500);

   send_comm_train_speed(1, 20, 0, 1, 14);
   send_comm_rail_speed(1, 100);
   send_comm_train_speed(2, 20, 0, 1, 14);
   send_comm_rail_speed(2, 100);
   send_comm_train_speed(3, 20, 0, 1, 14);
   send_comm_rail_speed(3, 100);
   
#endif /* USING_STATIC_CONFIG */
}

#define TRAIN_CYCLE_PROCESS_MS      200
void train_cycle_process(void)
{
    static uint16 cycle_count = 0;
    uint8 rid; 
    bool enable_flag = FALSE;
    uint8 speed;
    bool dir;
    
    cycle_count++;
    if (cycle_count > (TRAIN_CYCLE_PROCESS_MS / CYCLE_MS)) {
        enable_flag = TRUE;
        cycle_count = 0;
    }
    
    for (rid = 0; rid < RAIL_MAX; rid++) {
        struct rail_config *rail = &ctrl.rail[rid];
        
        if (rail->req.rail_status_update || enable_flag) {
            
            rail->req.rail_status_update = FALSE;
            
            speed = train_info_load_speed(rail->train_id, &dir);
            
            if (rail->head) {
                if (rail->reserved_num == 0) {
                    if (rail->tail) {
                        //if (speed != 0)
                                send_comm_rail_speed(rail->train_id, 0);
                    } else {
                        //if (speed != 10)
                            send_comm_rail_speed(rail->train_id, 10);
                    }
                } else if (rail->reserved_num == 1) {
                    //if (speed != 15)
                        send_comm_rail_speed(rail->train_id, 15);
                } else {
                    //if (speed != 20)
                        send_comm_rail_speed(rail->train_id, 20);
                }
            }
        }
    }
}