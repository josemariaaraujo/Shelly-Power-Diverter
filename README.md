# Shelly Power Diverter
Power diverter of excess solar production using a Shelly 2.5

In small scale grid-tied solar production, surplus power (production in excess of local consumption) is injected **for free** back into the grid. 

A power diverter can be used to connect a charge, usually a water heater, so that no solar energy is wasted. Water heaters are the most suited as they have a resistive element that doesn't care about the power quality, and it ends up stored as hot water, making it similar to a battery. A power diverter will only connect the charge some cycles at a time as to avoid using more than the production, to avoid grid charges.

There are commercial solutions, like the eddi:https://myenergi.com/product/eddi/, but it's expensive for a small production unit (around €450 at the time of this writing), delayin the ROI a couple of years.

There are also already DIY solutions, listed here:https://learn.openenergymonitor.org/pv-diversion/introduction/choosing-an-energy-diverter.md, but they're somewhat complex, and parts kits cost around 100€, mainly beacause of the metering circuit and power supply.

Studying those DIY solutions, they're comprised of some way to measure power, a solid state switch and a microcontroller to tie it all together. Something clicked and remebered that there are Sonoff and Shelly modules that incorporate a power meter, and controlling an external SSR should be trivial, bringing the cost down.

Looking at how those modules metered power, found out that the Shelly 2.5 uses an ADE7953 https://www.analog.com/en/products/ade7953.html, which is a commercial meter grade IC. Other modules either are more expensive or used simpler ICs (pulse only).

As to not insert the diverter into the main power feed, I decided to use a split core current transformer. It isn't supported by the Shelly 2.5 but shouldn't be too hard do modify it.

The solution as is now costs a total of around 30€:
* Shelly 2.5 - €20.35 (locally)
* Current transformer (for Shelly EM) - €8,99 (locally, similar price to Ali and faster)
* Fotek SSR-25DA or Jotta SSR-40DA - ~3€

~~May have to change the SSR or add dissipation, to be tested.~~ No dissipation needed in my use case, if SSR gets hot may be needed for others

**This project is heavily inspired by the existing DIY solution in https://learn.openenergymonitor.org/pv-diversion/introduction/choosing-an-energy-diverter.md**

## Shelly 2.5 Hardware
* Circuit ground is external live
* AC Neutral is first regulated to 12V, then to 3.3V for the ESP8266
* Relays are driven by the 12V controlled by mosfets -> **easy to swap for SSR!**
* Relays and current shunts are in daughter board. Only connections are:
  * Live
  * Relay control (live works as ground)
  * Current value (in a differential pair, ADE7953 limit +/-500mv) **to swap for CT**
  * Contact terminals

## Software
~~Has to be custom, AFAIK existing solutions like ESPEasy or Tasmota won't work because of 50Hz interrupt for energy reading.~~
Custom module developped for ESPHome. Chosen between ESPEasy, Tasmota and Espurna because seemed easier (easier to read and understand code, very fast to pick up)

Used PlatformIO as development environment.
