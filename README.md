# Bus stop screen powered by ESP32-C3-DEV


This text is WIP.


Building the complete setup and documenting afterwards.


Todo:

<ul>
    <li>
        Add sleep mode to save battery life
    </li>
    <li>
        Create a physical box for the ESP32 + LCD display + battery
    </li>
    <li>
        Add buttons which display different bus stops.
    </li>
    <li>
        Pictures! And a how-to-do guide
    </li>
    <li>
        Component listing and link to components
    </li>
    <li>
        .dxf file for the casing (laser cutting). Alternative could be a 3D model.
    </li>
</ul>

Nice to have things:
<ul>
    <li>
        Make the code a bit smarter by supporting multiple lines on all stations. Iidesranta struct should be expanded for all stops
    </li>
    <li>
        Use RTC instead of the time API as the API is not stable. <b>Note</b>: If using RTC need to check how much power consuption is during sleep.
    </li>
</ul>

Might happen (probably not)
<ul>
    <li>
        Rewrite whole code and helper functions to own files.
    </li>
</ul>