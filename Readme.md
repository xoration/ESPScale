# F3X ESPScale

[![Donate](https://img.shields.io/badge/Donate-PayPal-green.svg)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=M9J2VFXKKF4MC&source=url)

This is a open source CG Scale for F3X gliders. With different supports it is suitable for all kinds of model airplanes.

This project is similiar to the famous CG_Scale project (https://github.com/olkal/CG_scale) but it uses a ESP to calculate the CG 
and uses the Wifi Module of the ESP to either connect to your home WiFi (or it can create a Hotspot). 
To print the frame for the CG Scale use the STL Files found in the CG_Scale project. For additional supports:

www.thingiverse.com/thing:2805321

www.thingiverse.com/thing:2805317 

www.thingiverse.com/thing:2752095

It then offers a Webpage to configure the weight modules and also to display the CG, so no external display is necessary.
It is also possible to save models and their CG to rebalance them later e.g. after a crash.

A demo can be seen in the following video, it does not have all features of the current version but it gives you the idea behind the project!

[![Demo ESP Scale](https://j.gifs.com/59qmZq.gif)](https://www.youtube.com/watch?v=NDpFU1eD0Ts)

The project uses PlatformIO, so it is recommended to also use PlatformIO to compile and upload it. 
Also don't forget to upload the filesystem image! The Webpages are not part of the firmware itself.

In Visual Studio Code: Terminal -> Run Task (ctrl + alt + t) -> "PlatformIO: Upload File System image".
