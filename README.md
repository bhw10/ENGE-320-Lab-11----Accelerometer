# accelerometer_get() function done;
# We need to make an accelerometer_update() function that is called every 17 ms (60 Hz);
# SysTick_Handler already set up for timer;
# We need to set up ISR for I2C;
# We will check if the accelerometer has been updated; if it has, we begin I2C transfer;
# I2C ISR should have state machine to check which part of data send we are in (similar to SPI ISR);

# Make sure to fork or clone your code and make pull requests when you want to add to the repo;
# I will be checking every day to approve new commits;

# 11/7/25
# i2c sends address to be written to, sends byte, when ack received, start read
# read address followed by 6 data bytes
# Take logic from i2c_read and i2c_read_setup and put it into the interrupt

