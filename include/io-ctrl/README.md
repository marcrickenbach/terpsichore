# I/O Control Library

The I/O Control library is a Zephyr-specific library of modules meant
to interface primarily with the Zephyr RTOS drivers, as well as some
custom drivers. Each module contains its own thread and is only 
responsible for passing data between the middleware layer and the 
hardware peripherals. For example, the ADC module is responsible for
the timing and reading of the ADC peripheral and broadcasting its 
readings to a listening module, say, a Potentiometer module. That
data will be enqueued in the middleware module's thread where it will
wait for processing, such as filtering and scaling, before it is sent 
to the Application layer. 

## Todo

1. Make each module as custumizable and reusable as possible.
2. Encapsulate library in its own project.