#include "etc_controller.h"
#include "mbed.h"
#include <cstdio>

ETCController etc{PC_1, PC_2};

CAN can{PB_8, PB_9, 500000};

// Predefined for use in main()
void send_etc_CAN_messages();
void send_sme_CAN_messages();

EventQueue can_queue;
Thread can_thread;


int main() {
    printf("Hello World!!\n");

    /*
        Make send_CAN messages loop consistently
        at 20hz (20 messages / second)
    */

    //Ticker interrupts main from finsihing, causing a deadlock
    can_queue.call_every(50ms, send_etc_CAN_messages);
    can_queue.call_every(50ms, send_sme_CAN_messages);
    can_thread.start(callback(&can_queue, &events::EventQueue::dispatch_forever));

    // No need to edit this while loop
    while (true) {
        // Must implement in etc/ subfolder
        etc.update_state();
    }

    return 0;
}

/* 
    Must send the VCU_TPDO_STATUS CAN message with
    VCU_ETC_READY_TO_DRIVE & VCU_ETC_MOTOR_ENABLED
    set to 1, and VCU_ETC_IMPLAUS_APPS_OUT_OF_RANGE
    & VCU_ETC_IMPLAUS_APPS_DEVIATION properly added.
*/
void send_etc_CAN_messages() {
    uint8_t TPDO_STATUS_DATA[8] = {0};
    TPDO_STATUS_DATA[0] = 3;
    TPDO_STATUS_DATA[0] |= etc.apps_range_implaus << 4;
    TPDO_STATUS_DATA[0] |= etc.apps_disagree_implaus << 6;

    CANMessage VCU_TPDO_STATUS{403, TPDO_STATUS_DATA, 8};

    can.write(VCU_TPDO_STATUS);
}

/*
    Must send the SME_RPDO_Throttle_Demand CAN
    message with SME_THROTL_TorqueDemand set to the
    torque demand determined by the ETC.
*/

void send_sme_CAN_messages() {
    uint8_t SME_THROTTLE_DATA[8] = {0};

    SME_THROTTLE_DATA[0] = etc.torque_output & 0xFF;
    SME_THROTTLE_DATA[1] = (etc.torque_output >> 8) & 0xFF;

    CANMessage SME_RPDO_Throttle_Demand(390, SME_THROTTLE_DATA, 8);

    can.write(SME_RPDO_Throttle_Demand);
}
