# trap-basic-software V1.1

## Uploading
Because the code uses `__DATE__` and `__TIME__` to set the RTC each time uploading the program to the Arduino pro you will need to make a change to the code, just adding/removing a new line has worked for me. This will make sure that `__DATE__` and `__TIME__` get updated and don't use the values of last time the code was changed and compiled.

When uploading have the serial monitiro running and you should see if the time was set propery from that.