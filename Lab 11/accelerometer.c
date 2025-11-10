//------------------------------------------------------------------------------
//             __             __   ___  __
//     | |\ | /  ` |    |  | |  \ |__  /__`
//     | | \| \__, |___ \__/ |__/ |___ .__/
//
//------------------------------------------------------------------------------

#include "sam.h"
#include <stdbool.h>
#include "accelerometer.h"
#include "bmi160.h"
#include "delay.h"
#include "i2c.h"

//------------------------------------------------------------------------------
//      __   ___  ___         ___  __
//     |  \ |__  |__  | |\ | |__  /__`
//     |__/ |___ |    | | \| |___ .__/
//
//------------------------------------------------------------------------------

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

struct bmi160_dev sensor;
static struct bmi160_sensor_data accel_1;
static struct bmi160_sensor_data accel_0;

static struct bmi160_sensor_data *accel_read_addr = &accel_0;
static struct bmi160_sensor_data *accel_write_addr = &accel_1;

static bool isSwapping = true;

//------------------------------------------------------------------------------
//      __   __   __  ___  __  ___      __   ___  __
//     |__) |__) /  \  |  /  \  |  \ / |__) |__  /__`
//     |    |  \ \__/  |  \__/  |   |  |    |___ .__/
//
//------------------------------------------------------------------------------
int test_eeprom(void);
void user_delay_ms(uint32_t ms);
int8_t user_i2c_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t length);
int8_t user_i2c_write(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t length);

//------------------------------------------------------------------------------
//      __        __          __
//     |__) |  | |__) |    | /  `
//     |    \__/ |__) |___ | \__,
//
//------------------------------------------------------------------------------

//==============================================================================
void accelerometer_init()
{
	uint8_t data[32];
	uint8_t address;
	struct bmi160_sensor_data accel;
	uint8_t retval;
	
	sensor.id = BMI160_I2C_ADDR;
	sensor.interface = BMI160_I2C_INTF;
	sensor.read = user_i2c_read;
	sensor.write = user_i2c_write;
	sensor.delay_ms = user_delay_ms;
	int8_t rslt = BMI160_OK;
	
	retval = bmi160_init(&sensor);
	
	/* After the above function call, accel and gyro parameters in the device structure
	are set with default values, found in the datasheet of the sensor */
	
	/* Select the Output data rate, range of accelerometer sensor */
	sensor.accel_cfg.odr = BMI160_ACCEL_ODR_1600HZ;
	sensor.accel_cfg.range = BMI160_ACCEL_RANGE_2G;
	sensor.accel_cfg.bw = BMI160_ACCEL_BW_NORMAL_AVG4;
	/* Select the power mode of accelerometer sensor */
	sensor.accel_cfg.power = BMI160_ACCEL_NORMAL_MODE;

	
	/* Set the sensor configuration */
	rslt = bmi160_set_sens_conf(&sensor);
	sensor.delay_ms(100);
	accel_read_addr = &accel_1;
	accel_write_addr = &accel_0;
}

//------------------------------------------------------------------------------
//      __   __              ___  ___
//     |__) |__) | \  /  /\   |  |__
//     |    |  \ |  \/  /~~\  |  |___
//
//------------------------------------------------------------------------------

//=============================================================================
bool accelerometer_get_from_source(uint8_t dev_addr, uint8_t reg_addr,uint8_t *data, uint16_t length)
{
	i2c_bmi_update_accel(dev_addr);
	return true;
}


//=============================================================================
// not used
void user_delay_ms(uint32_t ms)
{
	DelayMs(ms);
}

//=============================================================================
// not used
int8_t user_i2c_read(uint8_t dev_addr, uint8_t reg_addr,
uint8_t *data, uint16_t length)
{
	i2c_read_setup(dev_addr << 1, &reg_addr, 1);
	i2c_read(dev_addr << 1,data,length);
	return BMI160_OK;
}

//=============================================================================
// not used
int8_t user_i2c_write(uint8_t dev_addr, uint8_t reg_addr,
uint8_t *data, uint16_t length)
{
	uint8_t mydata[32];
	if (length > 31) length = 31;
	// Add 1 to include the reg address
	length++;
	//Send the address
	mydata[0] = reg_addr;
	bcopy(data, mydata+1, length);
	i2c_write(dev_addr << 1,mydata,length);
	return BMI160_OK;
}


uint16_t accelerometer_getX()
{
	return accel_read_addr->x;
}

uint16_t accelerometer_getY()
{
	return accel_read_addr->y;
}

/************************************************************************/
/* update with new data                                                 */
/************************************************************************/


uint8_t accelerometer_update()
{
	// if we are updateing values, return
	if(i2c_get_isBMI())
	{
		return 0;
	}
	__disable_irq(); // Atomic (disable interrupts)
	
	// if isBmi is set, we are getting i2c data
	// if we have new data lets update
	if(!i2c_get_isBMI() && i2c_bmiUpdated())
	{
		// define things
		uint8_t lsb;
		uint8_t msb;
		int16_t msblsb;
		
		// get that new data
		uint8_t *newData = get_newDataArray();
		
		// set to proper locations
		uint8_t i = 0;
		
		// from bmi code
		lsb = newData[i++];
		msb = newData[i++];
		msblsb = (int16_t)((msb << 8) | lsb);
		accel_write_addr->x = msblsb; /* Data in X axis */

		lsb = newData[i++];
		msb = newData[i++];
		msblsb = (int16_t)((msb << 8) | lsb);
		accel_write_addr->y = msblsb; /* Data in Y axis */

		lsb = newData[i++];
		msb = newData[i++];
		msblsb = (int16_t)((msb << 8) | lsb);
		accel_write_addr->z = msblsb; /* Data in Z axis */
		
	}
	
	if (memcmp(accel_read_addr, accel_write_addr, sizeof(accel_read_addr))) // if temp_accel is different than old accel
	{
		if (accel_read_addr == &accel_1)
		{
			accel_read_addr = &accel_0;
			accel_write_addr = &accel_1;
		}
		else
		{
			accel_read_addr = &accel_1;
			accel_write_addr = &accel_0;
		}
		__enable_irq(); // reenable interrupts
		return 1;
	}
	__enable_irq();
	return 0;
};

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
