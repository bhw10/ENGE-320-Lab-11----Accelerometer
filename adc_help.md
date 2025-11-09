# adc help from lecture
comparison = VDD/2 = 256
gain = 1/2
MUXPOS = Joystick Y input
adc in free run..
kick it off at the end of the setup

no free run 
interupt
Setup for X (MUXPOS)
kickoff a start

...

ISR .. 
what was I doing?
X: 
    Read RESULT and store in some volatile global static Xval
    MUXPOS = Y
    kick off start..
    set my flag to Y

Y:
    Read Result and store in some volatile global static Y val

    MUXPOS = X
    kick of start
    set my flag to X



need to creat a function to read them...
or 2 functions to read each o fthem


```C
    uint16_t joystick_get_x();
    uint16_t joystick_get_y();
    {
        uint16_t result;
        // ensure atomic-ness
        __disable_irq();
        result = Yval;
        __enable_irq();
        return result;
    }
```

