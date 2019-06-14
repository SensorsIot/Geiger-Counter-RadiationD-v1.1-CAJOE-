# Geiger Counter

RadiationD gieger counter arduino serial port code .ino

the project i forked from was linked to by the manufacturer (why?) but the only thing that was relevant was the interrupt of the pin that the geiger counter was connected to.

I added cursory click per minute averages. they're garbage. if you get a ton of clicks for a few seconds your clicks per minute may only go up by 1, depending on how long this runs.

I will fix it later, i just wanted some way to see the clicks on a serial console, and this does that, as it prints the total clicks since the arduino started, as well as the overall average (as in, total clicks divided by total minutes)

I know that the cheap-o amazon and ebay geiger counters have much smaller averaging windows, but i don't have a display that will work with an arduino micro yet, so i don't have any inclination to fix this to have better math or averaging yet. 

take this (specifically the .ino) as a way to start your own exploration into hardware interrupts and using pulses to count something. because that's all i did.
