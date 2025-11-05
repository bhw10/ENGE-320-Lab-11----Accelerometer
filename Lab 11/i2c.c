//------------------------------------------------------------------------------
//             __             __   ___  __
//     | |\ | /  ` |    |  | |  \ |__  /__`
//     | | \| \__, |___ \__/ |__/ |___ .__/
//
//------------------------------------------------------------------------------

#include "i2c.h"
#include <stdbool.h>

//------------------------------------------------------------------------------
//      __   ___  ___         ___  __
//     |  \ |__  |__  | |\ | |__  /__`
//     |__/ |___ |    | | \| |___ .__/
//
//------------------------------------------------------------------------------

#define I2C_SCL (PORT_PA23)
#define I2C_SCL_GROUP (0)
#define I2C_SCL_PIN (PIN_PA23%32)
#define I2C_SCL_PMUX (I2C_SCL_PIN/2)

#define I2C_SDA (PORT_PA22)
#define I2C_SDA_GROUP (0)
#define I2C_SDA_PIN (PIN_PA22%32)
#define I2C_SDA_PMUX (I2C_SDA_PIN/2)


#define I2C_SERCOM            SERCOM3
#define I2C_SERCOM_PMUX       PORT_PMUX_PMUXE_C_Val
#define I2C_SERCOM_GCLK_ID    SERCOM3_GCLK_ID_CORE
#define I2C_SERCOM_CLK_GEN    0
#define I2C_SERCOM_APBCMASK   PM_APBCMASK_SERCOM3

enum
{
	I2C_TRANSFER_WRITE = 0,
	I2C_TRANSFER_READ  = 1,
};

//------------------------------------------------------------------------------
//     ___      __   ___  __   ___  ___  __
//      |  \ / |__) |__  |  \ |__  |__  /__`
//      |   |  |    |___ |__/ |___ |    .__/
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//                __          __        ___  __
//     \  /  /\  |__) |  /\  |__) |    |__  /__`
//      \/  /~~\ |  \ | /~~\ |__) |___ |___ .__/
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//      __   __   __  ___  __  ___      __   ___  __
//     |__) |__) /  \  |  /  \  |  \ / |__) |__  /__`
//     |    |  \ \__/  |  \__/  |   |  |    |___ .__/
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//      __        __          __
//     |__) |  | |__) |    | /  `
//     |    \__/ |__) |___ | \__,
//
//------------------------------------------------------------------------------

//==============================================================================
void i2c_init(void)
{

	// Set up the SCL Pin
	//Set the direction - it is an output, but we want the input enable on as well
	//so that we can read it at the same time ... because I2C. 
	PORT->Group[I2C_SCL_GROUP].DIRSET.reg = I2C_SCL;
	PORT->Group[I2C_SCL_GROUP].PINCFG[I2C_SCL_PIN].bit.INEN = 1;
	// Set the pullup
	PORT->Group[I2C_SCL_GROUP].OUTSET.reg = I2C_SCL;
	PORT->Group[I2C_SCL_GROUP].PINCFG[I2C_SCL_PIN].bit.PULLEN = 1;
	//Set the PMUX
	PORT->Group[I2C_SCL_GROUP].PINCFG[I2C_SCL_PIN].bit.PMUXEN = 1;
    if (I2C_SCL_PIN & 1)								
	  PORT->Group[I2C_SCL_GROUP].PMUX[I2C_SCL_PMUX].bit.PMUXO = I2C_SERCOM_PMUX;		
    else
	  PORT->Group[I2C_SCL_GROUP].PMUX[I2C_SCL_PMUX].bit.PMUXE = I2C_SERCOM_PMUX;					


    // Set up the SDA PIN
	//Set the direction - it is an output, but we want the input enable on as well
	//so that we can read it at the same time ... because I2C.
	PORT->Group[I2C_SDA_GROUP].DIRSET.reg = I2C_SDA;
	PORT->Group[I2C_SDA_GROUP].PINCFG[I2C_SDA_PIN].bit.INEN = 1;
	// Set the pullup
	PORT->Group[I2C_SDA_GROUP].OUTSET.reg = I2C_SDA;
	PORT->Group[I2C_SDA_GROUP].PINCFG[I2C_SDA_PIN].bit.PULLEN = 1;
	//Set the PMUX
	PORT->Group[I2C_SDA_GROUP].PINCFG[I2C_SDA_PIN].bit.PMUXEN = 1;
	if (I2C_SDA_PIN & 1)
	PORT->Group[I2C_SDA_GROUP].PMUX[I2C_SDA_PMUX].bit.PMUXO = I2C_SERCOM_PMUX;
	else
	PORT->Group[I2C_SDA_GROUP].PMUX[I2C_SDA_PMUX].bit.PMUXE = I2C_SERCOM_PMUX;
	
	// Turn on the clock
	PM->APBCMASK.reg |= I2C_SERCOM_APBCMASK;

    // Configure the clock
	GCLK->CLKCTRL.reg = I2C_SERCOM_GCLK_ID |
	                    GCLK_CLKCTRL_CLKEN | 
						I2C_SERCOM_CLK_GEN;

    //Turn off the I2C enable so that we can write the protected registers
	I2C_SERCOM->I2CM.CTRLA.bit.ENABLE = 0;
	while (I2C_SERCOM->I2CM.SYNCBUSY.reg);

	// Turn on smart mode (because it is smart)
	I2C_SERCOM->I2CM.CTRLB.bit.SMEN = 1;
	while (I2C_SERCOM->I2CM.SYNCBUSY.reg);

    // Set the baud rate - this is a confusing little formula as 
	// it involves the actual rise time of SCL on the board
	// We would need to measure this to predict the outcome,
	// Or, we can just change it until we like it. 
	// See 27.6.2.4 of the datasheet.
	I2C_SERCOM->I2CM.BAUD.reg = SERCOM_I2CM_BAUD_BAUD(232);
	while (I2C_SERCOM->I2CM.SYNCBUSY.reg);

    // Set us up as a Master
	I2C_SERCOM->I2CM.CTRLA.bit.MODE = SERCOM_I2CM_CTRLA_MODE_I2C_MASTER_Val;
	while (I2C_SERCOM->I2CM.SYNCBUSY.reg);
	
	// Set the hold time to 600ns
	I2C_SERCOM->I2CM.CTRLA.bit.SDAHOLD == 3;
	while (I2C_SERCOM->I2CM.SYNCBUSY.reg);

    //Turn on the I2C enable 
	I2C_SERCOM->I2CM.CTRLA.bit.ENABLE = 1;
	while (I2C_SERCOM->I2CM.SYNCBUSY.reg);

	// Set the bus state to be IDLE (this has to be after the enable)
	I2C_SERCOM->I2CM.STATUS.reg |= SERCOM_I2CM_STATUS_BUSSTATE(1);
	while (I2C_SERCOM->I2CM.SYNCBUSY.reg);
	
	I2C_SERCOM->I2CM.INTENSET.bit.MB = 1; // Enable master bus interrupt
	I2C_SERCOM->I2CM.INTENSET.bit.SB = 1; // Enable slave bus interrupt

}

//==============================================================================
uint8_t i2c_write(uint8_t addr, uint8_t *data, int size)
{
    // Send the address
	I2C_SERCOM->I2CM.ADDR.reg = addr | I2C_TRANSFER_WRITE;

	while (0 == (I2C_SERCOM->I2CM.INTFLAG.reg & SERCOM_I2CM_INTFLAG_MB));

	if (I2C_SERCOM->I2CM.STATUS.reg & SERCOM_I2CM_STATUS_RXNACK)
	{
		I2C_SERCOM->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(3);
		return false;
	}

	for (int i = 0; i < size; i++)
	{
		I2C_SERCOM->I2CM.DATA.reg = data[i];

		while (0 == (I2C_SERCOM->I2CM.INTFLAG.reg & SERCOM_I2CM_INTFLAG_MB));

		if (I2C_SERCOM->I2CM.STATUS.reg & SERCOM_I2CM_STATUS_RXNACK)
		{
			I2C_SERCOM->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(3);
			return false;
		}
	}

	I2C_SERCOM->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(3);

	return size;
}

//==============================================================================
uint8_t i2c_read_setup(uint8_t addr, uint8_t *data, int size)
{
	// Send the address
	I2C_SERCOM->I2CM.ADDR.reg = addr | I2C_TRANSFER_WRITE;

	while (0 == (I2C_SERCOM->I2CM.INTFLAG.reg & SERCOM_I2CM_INTFLAG_MB));

	if (I2C_SERCOM->I2CM.STATUS.reg & SERCOM_I2CM_STATUS_RXNACK)
	{
		I2C_SERCOM->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(3);
		return false;
	}

	for (int i = 0; i < size; i++)
	{
		I2C_SERCOM->I2CM.DATA.reg = data[i];

		while (0 == (I2C_SERCOM->I2CM.INTFLAG.reg & SERCOM_I2CM_INTFLAG_MB));

		if (I2C_SERCOM->I2CM.STATUS.reg & SERCOM_I2CM_STATUS_RXNACK)
		{
			I2C_SERCOM->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(3);
			return false;
		}
	}

    // Issue a restart instead
	I2C_SERCOM->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(1);

	return size;
}


//==============================================================================
uint8_t i2c_read(uint8_t addr, uint8_t *data, int size)
{
	I2C_SERCOM->I2CM.ADDR.reg = addr | I2C_TRANSFER_READ;

	while (0 == (I2C_SERCOM->I2CM.INTFLAG.reg & SERCOM_I2CM_INTFLAG_SB));

	if (I2C_SERCOM->I2CM.STATUS.reg & SERCOM_I2CM_STATUS_RXNACK)
	{
		I2C_SERCOM->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(3);
		return false;
	}

	I2C_SERCOM->I2CM.CTRLB.reg &= ~SERCOM_I2CM_CTRLB_ACKACT;

	for (int i = 0; i < size-1; i++)
	{
		data[i] = I2C_SERCOM->I2CM.DATA.reg;
		while (0 == (I2C_SERCOM->I2CM.INTFLAG.reg & SERCOM_I2CM_INTFLAG_SB));
	}

	if (size)
	{
		I2C_SERCOM->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_ACKACT;
		I2C_SERCOM->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(3);
		data[size-1] = I2C_SERCOM->I2CM.DATA.reg;
	}

	return size;
}


//------------------------------------------------------------------------------
//      __   __              ___  ___
//     |__) |__) | \  /  /\   |  |__
//     |    |  \ |  \/  /~~\  |  |___
//
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
//      __                  __        __        __
//     /  `  /\  |    |    |__)  /\  /  ` |__/ /__`
//     \__, /~~\ |___ |___ |__) /~~\ \__, |  \ .__/
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//        __   __  , __
//     | /__` |__)  /__`   
//     | .__/ |  \  .__/
//
//------------------------------------------------------------------------------
void SERCOM3_Handler()
{
	
}