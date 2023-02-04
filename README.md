## Pet Alert
### ESP32 based motion detector - Informs you when your pet is waiting at the door to be let in.  

The goal was to create a small portable motion detector that runs on battery power only and does not require an external power source. When motion is detected, a push notification is to be sent to the smartphone via WLAN. For the push notification Telegram is used, which means no additional APP is needed, except Telegram of course. The challenge was to consume little power in order to extend the operating time of the motion detector in battery mode.  

<img src="./docs/PetAlert.png"/>  

Sure, there are several use cases for which a portable motion detector can be used. But as the project name suggests, we use it for our pet, more precisely for our cat. When our cat comes back from stray, she often stands unnoticed in front of the front door for hours, wanting to be let in. In the winter, she's usually pretty peeved when we finally let her in. A cat door would be a possible solution, but we opted for the PetAlert solution, which of course also works for dogs.  

How it works
------------
The motion detector should be positioned near the front door, where your pet usually stays when it wants to be let in, within range of the WiFi signal. If no motion is detected, the device is in deep sleep mode, no active WLAN connection exists. When motion is detected, the device is awakened from deep sleep mode and monitors further motion for 30 seconds in active mode. If nothing is detected, the device automatically switches back to deep sleep mode. However, if 3 more movements are registered within the 30 seconds, a WLAN connection is established and a predefined URL is invoked. In our case, the Telegram API for push notifications. After that, the device switches to deep sleep mode for 5 minutes, during which time no further movements are detected. All mentioned parameters like time, number of movements as well as the URL are configurable via web interface.  

<img src="./docs/WebPrefs.png"/>  

Web interface
-------------
There are 2 configuration modes for accessing the web interface. Mode A) the device acts as a WLAN access point. Mode B) the device acts as a WLAN client. For the very first time, the device must be placed in mode A) in order to access the web interface. To do this, set jumper J1 and press the RST/EN button on the ESP32. The flashing LED1 indicates that mode A) is active. The mode remains active for 5 minutes. A SSID with the initial letters PETxxxx should be visible. Now you can connect to this access point with your WLAN device. The password is "petalert". To open the web interface, simply enter http://10.100.10.1 in the address bar of your browser. You should remember the SSID, which is also the host name of the device. Helpful in **mode B**, where the device is assigned a dynamic IP address. In mode A you can enter your WLAN data and assign user and password for the web interface, do not forget to save. The remaining parameters should be added in **mode B**. If all data are correct, you can switch to **mode B**, just remove jumper J1 and press the RST/EN button on the ESP32. The LED1 will light up constantly. If you don't know your IP address, you can also access the web interface by hostname. Now you need to enter the previously assigned credentials to access the web interface (user and password).  

**Alert URL** in this input field a URL can be entered, which will be called in case of an alert. The placeholders *$token$* and *$text$* are replaced by the corresponding input fields **Alert Text** and **Token**. If you want to use Telegram as a notification service, you can keep the URL `https://api.telegram.org/bot$token$/sendMessage?chat_id=1234567&text=$text$` only the chat_id has to be changed. Instructions on how to create your personal Telegram token are explained in detail here: https://core.telegram.org/bots#how-do-i-create-a-bot. In a nutshell, you need to create a bot with your Telegram app, which will give you your token. Then you need to create a group and add the bot to the group. The only other thing you need is the chat_id, which is the ID of your group. I've explained the details here in section 4: https://github.com/seizu/siteping#readme  

**Alert Text**
