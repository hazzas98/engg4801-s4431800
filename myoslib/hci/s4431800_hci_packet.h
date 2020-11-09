/********************************************************************************
 * @file    mylib/hci/s4431800_hal_packet.h
 * @author  Harry Stokes 44318008
 * @date    15/04/2020
 * @brief   hal hci packet structure
 ******************************************************************************
 *     EXTERNAL FUNCTIONS
 ******************************************************************************
 * 
 ******************************************************************************
 */

#ifndef PACKAGE_FORMAT_H
#define PACKAGE_FORMAT_H

#define SID1_WRITE_ADDRESS  0xD4
#define SID2_WRITE_ADDRESS  0x3C
#define SID3_WRITE_ADDRESS  0xBA
#define SID4_WRITE_ADDRESS  0x52
#define SID5_WRITE_ADDRESS  0xBE

#define SID1_READ_ADDRESS  0xD5
#define SID2_READ_ADDRESS  0x3D
#define SID3_READ_ADDRESS  0xBB
#define SID4_READ_ADDRESS  0x53
#define SID5_READ_ADDRESS  0xBF

struct packetRecv {
    char preamble;
    char type;
    char length;
    int SID;
    char i2cAddress;
    char registerAddress;
    char data_h;
    char data_l;
};

#endif