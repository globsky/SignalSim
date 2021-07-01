# SignalSim
 A multi-stage GNSS signal and data simulator

This is a GNSS signal and data simulator and generator to help the development of GNSS receiver.
This program support multiple stage of simulation output:
1. Reference trajectory of receiver (as standard reference result)
2. Observation (help to debug and test PVT program)
3. Baseband correlation result (combine with local channel configuration parameters help to debug baseband tracking and control program)
4. Digital IF result (help to debug baseband process algorithm)
5. A realtime GNSS signal simulator (with hardware signal interpolation and up convertor)

Change list as below:
7/1/2021
	An optimized initial version re-published on github
