#include "xil_printf.h"
#include "sleep.h"
#include "ps_i2c.h"

/* Sensor transactions fit entirely in the FIFO. Preserve clock divisors and
 * bound polling instead of the 2019.1 driver's unbounded completion loop. */
static int transfer(XIicPs *iic, u16 address, u8 *bytes, unsigned count, int receive)
{
    u32 base = iic->Config.BaseAddress;
    u32 control, status;
    unsigned wait, i;
    for (wait = 0; wait < 1000 && XIicPs_BusIsBusy(iic); ++wait) usleep(100);
    if (wait == 1000) return XST_FAILURE;
    XIicPs_WriteReg(base, XIICPS_IDR_OFFSET, XIICPS_IXR_ALL_INTR_MASK);
    control = XIicPs_ReadReg(base, XIICPS_CR_OFFSET);
    control &= ~(XIICPS_CR_HOLD_MASK | XIICPS_CR_RD_WR_MASK);
    control |= XIICPS_CR_CLR_FIFO_MASK | XIICPS_CR_MS_MASK |
               XIICPS_CR_ACKEN_MASK | XIICPS_CR_NEA_MASK;
    if (receive) control |= XIICPS_CR_RD_WR_MASK;
    XIicPs_WriteReg(base, XIICPS_CR_OFFSET, control);
    XIicPs_WriteReg(base, XIICPS_ISR_OFFSET, XIICPS_IXR_ALL_INTR_MASK);
    if (receive) XIicPs_WriteReg(base, XIICPS_TRANS_SIZE_OFFSET, count);
    else for (i = 0; i < count; ++i) XIicPs_WriteReg(base, XIICPS_DATA_OFFSET, bytes[i]);
    XIicPs_WriteReg(base, XIICPS_ADDR_OFFSET, address);
    for (wait = 0; wait < 1000; ++wait) {
        status = XIicPs_ReadReg(base, XIICPS_ISR_OFFSET);
        if (status & (XIICPS_IXR_NACK_MASK | XIICPS_IXR_ARB_LOST_MASK |
                      XIICPS_IXR_TO_MASK | XIICPS_IXR_RX_OVR_MASK | XIICPS_IXR_TX_OVR_MASK))
            break;
        if (status & XIICPS_IXR_COMP_MASK) {
            if (receive) {
                if (!(XIicPs_ReadReg(base, XIICPS_SR_OFFSET) & XIICPS_SR_RXDV_MASK)) break;
                bytes[0] = (u8)XIicPs_ReadReg(base, XIICPS_DATA_OFFSET);
            }
            for (i = 0; i < 1000 && XIicPs_BusIsBusy(iic); ++i) usleep(100);
            return i == 1000 ? XST_FAILURE : XST_SUCCESS;
        }
        usleep(100);
    }
    XIicPs_WriteReg(base, XIICPS_CR_OFFSET, control & ~XIICPS_CR_HOLD_MASK);
    return XST_FAILURE;
}

int i2c_reg8_write(XIicPs *iic, char address, char reg, char data)
{
    u8 bytes[2] = {(u8)reg, (u8)data};
    return transfer(iic, (u8)address, bytes, 2, 0);
}

char i2c_reg8_read(XIicPs *iic, char address, char reg)
{
    u8 byte = (u8)reg;
    if (transfer(iic, (u8)address, &byte, 1, 0) != XST_SUCCESS ||
        transfer(iic, (u8)address, &byte, 1, 1) != XST_SUCCESS) return 0;
    return (char)byte;
}

int i2c_reg16_write(XIicPs *iic, char address, unsigned short reg, char data)
{
    u8 bytes[3] = {(u8)(reg >> 8), (u8)reg, (u8)data};
    return transfer(iic, (u8)address, bytes, 3, 0);
}

int i2c_reg16_read_checked(XIicPs *iic, u16 address, u16 reg, u8 *data)
{
    u8 bytes[2] = {(u8)(reg >> 8), (u8)reg};
    if (transfer(iic, address, bytes, 2, 0) != XST_SUCCESS) return XST_FAILURE;
    return transfer(iic, address, data, 1, 1);
}

char i2c_reg16_read(XIicPs *iic, char address, unsigned short reg)
{
    u8 byte = 0;
    (void)i2c_reg16_read_checked(iic, (u8)address, reg, &byte);
    return (char)byte;
}

int i2c_init(XIicPs *iic, short device, u32 rate)
{
    XIicPs_Config *config = XIicPs_LookupConfig(device);
    if (!config || XIicPs_CfgInitialize(iic, config, config->BaseAddress) != XST_SUCCESS)
        return XST_FAILURE;
    XIicPs_Reset(iic);
    return XIicPs_SetSClk(iic, rate);
}
