#ifndef DLT645DEFINES_H
#define DLT645DEFINES_H

#include <QString>

#define DLT645_WAKE_UP_BYTE             0xFE

#define DLT645_START_FRAME              0x68
#define DLT645_BROADCASTADDRPART        0x99  //A PART
#define DLT645_BROADCASTADDRLEN         6
#define DLT645_BROADCASTADDRLENNUMB     DLT645_BROADCASTADDRLEN * 2
#define DLT645_MIN_FRAME_LEN            12 //<START 1><ADDR 6><START 1><CONTROL 1><LEN 1><COMMAND 2><CHECK BYTE 1><END FRAME 1>

#define DLT645_END_FRAME                0x16

#define DLT645_CNTR_MASTER_READ_DATA    0x01
#define DLT645_CNTR_MASTER_WRITE_DATA   0x04

//T0,T1...T4, 4-bytes, XXXXXX,XX, Do not forget to add 0x33 and to change bytes order
#define DLT645_TOTAL_A_POSITIVE         0x901F
#define DLT645_TOTAL_A_REVERSE          0x902F
#define DLT645_TOTAL_R_POSITIVE         0x911F
#define DLT645_TOTAL_R_REVERSE          0x912F
//the maximum depth is 1 month
#define DLT645_EOF_MONTH_A_POSITIVE     0x941F
#define DLT645_EOF_MONTH_A_REVERSE      0x942F
#define DLT645_EOF_MONTH_R_POSITIVE     0x951F
#define DLT645_EOF_MONTH_R_REVERS       0x952F


#define DLT645_METER_SN                 0xC032
#define DLT645_DATE                     0xC010
#define DLT645_TIME                     0xC011

#define DLT645_HAS_NO_ERRORS            0
#define DLT645_HAS_ABNORMAL_ERROR       1


#endif // DLT645DEFINES_H
