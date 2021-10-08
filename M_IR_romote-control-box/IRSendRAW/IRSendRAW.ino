#include "IRDevice.h"

// User defined txPin, rxPin and carrier frequency
//#define IR_RX_PIN           8
//#define IR_TX_PIN           9

#define IR_RX_PIN           3
#define IR_TX_PIN           6
#define CARRIER_FREQ_PP     35710
#define CARRIER_FREQ_SS     35710

#define PIN_vol_p           0
#define PIN_vol_m           1
#define PIN_ch_p            2
#define PIN_ch_m            3

unsigned int irRawSignal_PP_power[] = {
    2674, 915,
    426, 895,
    426, 465,
    426, 467,
    1315, 1355,
    426, 468,
    426, 468,
    426, 468,
    426, 468,
    426, 468,
    426, 468,
    426, 468,
    426, 468,
    426, 468,
    426, 468,
    426, 467,    // 16
    870, 469,
    426, 896,
    426, 459,
    426, 459
};

unsigned int irRawSignal_SS_power[] = {
    2686, 897,
    460, 878,
    460, 457,
    460, 457,
    1333, 1317,
    469, 459,
    469, 459,
    469, 459,
    469, 459,
    469, 459,
    469, 459,
    469, 459,
    469, 459,
    469, 459,
    469, 459,
    469, 459,    // 16
    943, 459,
    460, 879,
    460, 459,
    460, 459
};

unsigned int irRawSignal_SS_vol_p[] = {
    2674, 893,
    457, 875,
    457, 456,
    457, 456,
    1327, 1312,
    457, 457,
    457, 457,
    457, 457,
    457, 457,
    457, 457,
    457, 457,
    457, 457,
    457, 457,
    457, 457,
    457, 457,
    939, 876,    // 16
    457, 458,
    457, 457,
    457, 458,
    457, 457
};

unsigned int irRawSignal_SS_vol_m[] = {
    2675, 893,
    457, 874,
    457, 456,
    457, 456,
    457, 884,
    874, 454,
    457, 457,
    457, 457,
    457, 457,
    457, 457,
    457, 457,
    457, 457,
    457, 457,
    457, 457,
    457, 457,
    457, 457,    // 16
    939, 876,
    457, 457,
    457, 457,
    940, 457
};

unsigned int irRawSignal_SS_ch_p[] = {
    2674, 894,
    457, 875,
    457, 456,
    457, 456,
    1327, 1312,
    457, 458,
    457, 457,
    457, 457,
    457, 457,
    457, 457,
    457, 457,
    457, 457,
    457, 457,
    457, 457,
    939, 876,
    457, 458,    // 16
    457, 457,
    457, 457,
    457, 457,
    457, 457
};

unsigned int irRawSignal_SS_ch_m[] = {
    2675, 893,
    457, 875,
    457, 456,
    457, 456,
    457, 884,
    874, 454,
    457, 457,
    457, 457,
    457, 458,
    457, 457,
    457, 457,
    457, 457,
    457, 456,
    457, 457,
    457, 457,
    939, 876,    // 16
    457, 457,
    457, 457,
    457, 457,
    940, 457
};


int DataLen_PP_power = sizeof(irRawSignal_PP_power) / sizeof(irRawSignal_PP_power[0]);

int DataLen_SS_power = sizeof(irRawSignal_SS_power) / sizeof(irRawSignal_SS_power[0]);
int DataLen_SS_vol_p = sizeof(irRawSignal_SS_vol_p) / sizeof(irRawSignal_SS_vol_p[0]);
int DataLen_SS_vol_m = sizeof(irRawSignal_SS_vol_m) / sizeof(irRawSignal_SS_vol_m[0]);
int DataLen_SS_ch_p = sizeof(irRawSignal_SS_ch_p) / sizeof(irRawSignal_SS_ch_p[0]);
int DataLen_SS_ch_m = sizeof(irRawSignal_SS_ch_m) / sizeof(irRawSignal_SS_ch_m[0]);


void setup() {
    Serial.begin(115200);

    pinMode(PIN_vol_p, INPUT);
    pinMode(PIN_vol_m, INPUT);
    pinMode(PIN_ch_p, INPUT);
    pinMode(PIN_ch_m, INPUT);
        
    //IR.begin(IR_RX_PIN, IR_TX_PIN, IR_MODE_TX, CARRIER_FREQ_PP);
    IR.begin(IR_RX_PIN, IR_TX_PIN, IR_MODE_TX, CARRIER_FREQ_SS);

    //IR.send(irRawSignal_PP_power, DataLen_PP_power);
    IR.send(irRawSignal_SS_power, DataLen_SS_power);
    
    //IR.send(irRawSignal_SS_vol_p, DataLen_SS_vol_p);
    //IR.send(irRawSignal_SS_vol_m, DataLen_SS_vol_m);
    //IR.send(irRawSignal_SS_ch_p, DataLen_SS_ch_p);
    //IR.send(irRawSignal_SS_ch_m, DataLen_SS_ch_m);

    Serial.println("Finished power on/off");
}

void loop() {
    //TV_send ();
    delay(2000);
}

void TV_send (void) {
  if (digitalRead(PIN_vol_p) == HIGH) {
    IR.send(irRawSignal_SS_vol_p, DataLen_SS_vol_p);
    printf("   Send data irRawSignal_SS_vol_p. \r\n");
  } else if (digitalRead(PIN_vol_m) == HIGH) {
    IR.send(irRawSignal_SS_vol_m, DataLen_SS_vol_m);
    printf("   Send data irRawSignal_SS_vol_m. \r\n");
  } else if (digitalRead(PIN_ch_p) == HIGH) {
    IR.send(irRawSignal_SS_ch_p, DataLen_SS_ch_p);
    printf("   Send data irRawSignal_SS_chl_p. \r\n");
  } else if (digitalRead(PIN_ch_m) == HIGH) {
    IR.send(irRawSignal_SS_ch_m, DataLen_SS_ch_m);
    printf("   Send data irRawSignal_SS_chl_m. \r\n");
  } else {
    printf("   Send data wrong. \r\n");
  }
}
