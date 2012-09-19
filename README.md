auto-watering-system
====================

Arduino system to automatically water my chilli plant when the soil gets dry with some [cubism](http://square.github.com/cubism) visulisation as a bonus.

Read more [in this blog post](http://theon.github.com/plant-watering-with-arduino.html)

# ./arduino

The arduino code has two dependencies. These pages contain details of how to install them:

 * [aJson](https://github.com/interactive-matter/aJson)
 * [SimpleTimer](http://arduino.cc/playground/Code/SimpleTimer)

The arduino code logs plant moisture and watering events by posting them to a [cube server](http://square.github.com/cube). Check out the comments in the code to configure it to point to your own cube server. Comments also details how to configure the watering logic - e.g. how often to check the moisture, how dry to let th soil get before watering, how long to water for etc.

# ./cubism

A demo html page which uses cubism to read from the cube server and visualise the moisture readings and watering events in real-time.