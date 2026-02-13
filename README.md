# BasicRTC

This example is to demonstrate the retention and continuity of the RTC clock and RTC Back-up Registers over power cycles.
MX_RTC_INIT() is modified so that it is resilient to round-tripping between the CUBEIDE Editor and CUBEMX Wizard.
Earlier code copied over from F105/F405 does not work and we seem to be unable to read from the RTC Back-up Registers before calling HAL_RTC_INIT().
It looks like the sub-second ( upto ~1s max ) loss by the HAL_RTC_INIT() has now been fixed by ST.
