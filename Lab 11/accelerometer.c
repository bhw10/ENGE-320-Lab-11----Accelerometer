//------------------------------------------------------------------------------
//             __             __   ___  __
//     | |\ | /  ` |    |  | |  \ |__  /__`
//     | | \| \__, |___ \__/ |__/ |___ .__/
//
//------------------------------------------------------------------------------

#include "accelerometer.h"
#include "i2c.h"
#include "delay.h"
#include "bmi160.h"

//------------------------------------------------------------------------------
//      __   ___  ___         ___  __
//     |  \ |__  |__  | |\ | |__  /__`
//     |__/ |___ |    | | \| |___ .__/
//
//------------------------------------------------------------------------------

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
int8_t rslt = BMI160_OK;
struct bmi160_dev sensor;
static struct bmi160_sensor_data accel_0, accel_1;
static struct bmi160_sensor_data *accel_read_addr = &accel_0;
static struct bmi160_sensor_data *accel_write_addr = &accel_1;

//------------------------------------------------------------------------------
//      __   __   __  ___  __  ___      __   ___  __
//     |__) |__) /  \  |  /  \  |  \ / |__) |__  /__`
//     |    |  \ \__/  |  \__/  |   |  |    |___ .__/
//
//------------------------------------------------------------------------------
int test_eeprom(void);
void user_delay_ms(uint32_t ms);
int8_t user_i2c_read(uint8_t dev_addr, uint8_t reg_addr,
uint8_t *data, uint16_t length);
int8_t user_i2c_write(uint8_t dev_addr, uint8_t reg_addr,
uint8_t *data, uint16_t length);
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
	uint8_t retval;

	sensor.id = BMI160_I2C_ADDR;
	sensor.interface = BMI160_I2C_INTF;
	sensor.read = user_i2c_read;
	sensor.write = user_i2c_write;
	sensor.delay_ms = user_delay_ms;
	
	retval = bmi160_init(&sensor);
	sensor.delay_ms(100);
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

//==============================================================================

uint16_t accelerometer_getX()
{
	return accel_read_addr->x;
}

uint16_t accelerometer_getY()
{
	return accel_read_addr->y;
}

uint8_t accelerometer_update()
{
	bmi160_get_sensor_data(BMI160_ACCEL_SEL, accel_read_addr, NULL, &sensor); // pack sensor data into temp_accel
	__disable_irq(); // Atomic (disable interrupts)
	if (memcmp(accel_read_addr, accel_write_addr, sizeof(accel_read_addr))) // if temp_accel is different than old accel
	{
		if (accel_read_addr == &accel_0)
		{
			accel_read_addr = &accel_1;
			accel_write_addr = &accel_0;
		}
		else
		{
			accel_read_addr = &accel_0;
			accel_write_addr = &accel_1;
		}
		__enable_irq(); // reenable interrupts
		return 1; 
	}
	__enable_irq();
	return 0;
};
//------------------------------------------------------------------------------
//      __   __              ___  ___
//     |__) |__) | \  /  /\   |  |__
//     |    |  \ |  \/  /~~\  |  |___
//
//------------------------------------------------------------------------------
void user_delay_ms(uint32_t ms)
{
	DelayMs(ms);
}

//=============================================================================
int8_t user_i2c_read(uint8_t dev_addr, uint8_t reg_addr,
uint8_t *data, uint16_t length)
{
	i2c_read_setup(dev_addr << 1, &reg_addr, 1);
	i2c_read(dev_addr << 1,data,length);
	return BMI160_OK;
}

//=============================================================================
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
