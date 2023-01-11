## Pet Alert
### ESP32 based motion detector - Informs you when your pet is waiting at the door to be let in.  

The goal was to create a small portable motion detector that runs on battery power only and does not require an external power source. When motion is detected, a push notification is to be sent to the smartphone via WLAN. For the push notification Telegram is used, which means no additional APP is needed, except Telegram of course. The challenge was to consume little power in order to extend the operating time of the motion detector in battery mode.  

There are of course several applications for which a portable motion detector can be used. But as the project name suggests, we use it for our pet, more precisely for our cat. When our cat comes back from stray, she often stands unnoticed in front of the front door for hours, wanting to be let in. In the winter, she's usually pretty peeved when we finally let her in. A cat door would be a possible solution, but we opted for the PetAlert solution, which of course also works for dogs.  

Technically, it works as follows:
---------------------------------
The motion detector should be positioned near the front door, where your pet usually stays when it wants to be let in, within range of the WiFi signal. If no motion is detected, the device is in deep sleep mode, no active WLAN connection exists. When motion is detected, the device is awakened from deep sleep mode and monitors further motion for 30 seconds in active mode. If nothing is detected, the device automatically switches back to deep sleep mode. However, if 3 more movements are registered within the 30 seconds, a WLAN connection is established and a predefined URL is invoked. In our case, the Telegram API for push notifications. After that, the device switches to deep sleep mode for 5 minutes, during which time no further movements are detected. All mentioned parameters like time, number of movements as well as the URL are configurable via a web interface.  
<img src="./docs/IFRBat.svg"/>
