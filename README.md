# IV27-VFD-Clock

This project uses an obscure Soviet era vacuum flourescent tube (IV27m). The intitial design idea was for a clock that looked like a scifi movie explosive device (think 80's cheesy movie bombs)![119057616_10224747282215496_3767550900032111040_n](https://user-images.githubusercontent.com/8798897/189245810-1d5e1dd0-8b66-444c-a2c3-2a027090b2fa.jpg)

The first step was decoding the tube pinouts
![119114848_10224747282415501_1258083057988973816_n](https://user-images.githubusercontent.com/8798897/189246215-2be51854-b959-4ab6-8d01-9126a9f74a10.jpg)

These pins were decoded manually as the little extant documentation is in cyrillic for the most part
![118980220_10224747281735484_5323936303906219770_n](https://user-images.githubusercontent.com/8798897/189246290-36e448cc-9123-4c90-bea7-6cfc3773ceb0.jpg)
![119069551_10224747281895488_4151508991232845230_n](https://user-images.githubusercontent.com/8798897/189246295-a2036543-67bd-4110-8be9-d17f33df7fd8.jpg)

From there a main PCB and end cap PCBs were designed

![119095870_10224746568717659_4204971245167933345_n](https://user-images.githubusercontent.com/8798897/189246334-aca46918-31dc-4e4d-ae01-a7c40d0309f0.jpg)
![119109238_10224746568877663_8699561781063773989_n](https://user-images.githubusercontent.com/8798897/189246340-b68bc235-2872-409b-adde-cc0f5475f882.jpg)

The boards were spun using EaglePCB and built by JLPCB
![118980679_10224746586598106_3718895178650649830_n](https://user-images.githubusercontent.com/8798897/189246377-5cc8f143-e2f6-4607-8990-e510302fff7c.jpg)
![119049793_10224746586518104_891389979846681961_n](https://user-images.githubusercontent.com/8798897/189246381-4b9d9563-7e0b-43b0-898f-291490906f4d.jpg)

From there assembly, feature expansion and done! It was modified to use an NTP time server, and uses the excellent WIFI manager arduino library. When the device boots and doesn't find a known SSID it rebootsd with a webserver running, and then scans for local SSIDs and then lets you select the correct one, load the password, and then if it connects successfully will reboot and connect itself.
![119061632_10224746569037667_7521624622622321505_n](https://user-images.githubusercontent.com/8798897/189246423-b0fd23ab-5875-4067-8e89-788d858ce10f.jpg)
![119061191_10224746569237672_3589316026076861871_n](https://user-images.githubusercontent.com/8798897/189246431-f59deab7-b6b2-4eb0-8f33-a12238592362.jpg)

Future plans for it involve using Geolocation to automatically set the timezone (versus the current hardcoded Central time) so it can be a bit more flexible.
