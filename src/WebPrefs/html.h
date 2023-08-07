static const char html_head[] PROGMEM = R"rawliteral(
	<!DOCTYPE html>
	<html><head>
	<title>WebPrefs</title>
	<meta http-equiv="content-type" content="text/html; charset=UTF-8"><meta charset="utf-8">
	<meta name="apple-mobile-web-app-capable" content="yes">
	<meta name="apple-touch-fullscreen" content="yes">
	<meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, minimum-scale=1.0, user-scalable=no">
	<link rel="shortcut icon" href="#">
)rawliteral";

static const char done_body[] PROGMEM = R"rawliteral(
</head><body onload="onloaded('done');"><div class="center">Good Bye!</div>
</body></html>
)rawliteral";
	
static const char settings_body[] PROGMEM = R"rawliteral(
</head><body onload="onloaded('form');"><div id="spn" calss="spinner"></div>
<div class="bog">
	<h5>Preferences</h5>
	<form id="myPrefs" action="./postForm">
	<label for="0n">WLAN SSID</label>
	<input type="text" id="0n" name="wlanssid">

	<label for="1n">WLAN Password</label>
	<input type="password" id="1n" name="wlanpasswd">

	<label for="2n">Web User</label>
	<input type="text" id="2n" name="webuser">

	<label>Web Password</label>
	<input type="password" id="3n" name="webpasswd">
	
	<label>Alert URL</label>
	<input type="text" id="4n" name="alertlink">
	
	<label>Alert Text</label>
	<input type="text" id="5n" name="alerttext">
	
	<label>Token</label>
	<input type="password" id="6n" name="token">

	<label>Delay between detections (seconds)</label>
	<input type="text" id="7n" name="ddelay">
	
	<label>Detections to trigger an alert (counts)</label>
	<input type="text" id="8n" name="dthreshold">

	<label>Sleep time after alert (seconds)</label>
	<input type="text" id="9n" name="stimer">
	
	<label>Attention timer (seconds)</label>
	<input type="text" id="10n" name="atimer">
	
	<label>LED Off/On</label>
	<div class="cbWrapper">
		<input type="hidden" value="off">
		<input type="checkbox" onclick="initCheckbox(this);" name="led" class="cbToggle" id="11n">
		<label class="switch" for="11n"></label>
	</div>

	<label>Power Off/On</label>
	<div class="cbWrapper">
		<input type="hidden" value="off">
		<input type="checkbox" onclick="initCheckbox(this);" name="power" class="cbToggle" id="12n">
		<label class="switch" for="12n"></label>
	</div>

	<div class="container">
		<div class="column-center"><button type="submit">Save</button></div>
		<div><button onclick="location.href='./done'" type="button">Done</button></div>
	</div>
</form>
</div></body><html>
)rawliteral";