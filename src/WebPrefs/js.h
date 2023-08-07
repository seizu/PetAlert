
static const char script_head[] PROGMEM = R"rawliteral(
<script type="text/javascript">
	function onloaded(fnc) {
		
		if((f=document.getElementById('myPrefs'))) f.onsubmit = postForm;
		
		getJSON('./getJson?fnc=' + fnc, function(err, response) {
			if (err) {
				alert("data load error. try to reload!");
			} else {
				setValuesByNames(response);
			}
		});
	}

function initCheckbox(obj) {
  if (obj.value == 'off') {
    obj.value = 'on';
    obj.previousElementSibling.name = '';
  } else {
    obj.value = 'off';
    obj.previousElementSibling.name = obj.name;
  }
}

function setValuesByNames(names) {
  for (var key in names) {
    elem = document.getElementsByName(key)[0];
		elem.setAttribute('value', decodeURIComponent(names[key]));
		if (elem.type == 'checkbox') {
      elem.checked = (names[key] == 'on');
			if(names[key] == 'off')
				elem.previousElementSibling.name = elem.name;
    }    
  }
}

function setValuesByIDs(ids) {
  for (var key in ids) {
		elem = document.getElementById(key);
    elem.setAttribute('value', decodeURIComponent(ids[key]));		
    if (elem.type == 'checkbox') {
			elem.checked = (names[key] == 'on');
			if(names[key] == 'off')
				elem.previousElementSibling.name = elem.name;
    }
  }
}

function getJSON(url, callback) {
  xhr = new XMLHttpRequest();
	loading(true);
  xhr.open('GET', url, true);
  xhr.responseType = 'json';
  xhr.onload= function() {
		loading(false);
    if (xhr.status === 200) {
      callback(null, xhr.response);
    } else {
      callback(xhr.status);
    }
  };
  xhr.send();
}

function loading(toggle) {	
	for(const bt of document.getElementsByTagName("button")) bt.disabled = toggle;
	toggle?spinnerOn():spinnerOff();		
}

function postForm(event) {
    event.preventDefault();   
    xhr = new XMLHttpRequest();
		loading(true);
    xhr.open('POST', event.target.action);
    xhr.onload = function() {
      loading(false);
			for(const bt of document.getElementsByTagName("button")) bt.disabled = false;
      if (xhr.status === 200) {
        console.log("Done!")
      } else {
        console.log("Form load error!")
      }     
    };    
    xhr.send(new FormData(event.target));
}

function postJSON(url, data, callback) {
  xhr = new XMLHttpRequest();
  xhr.open('POST', url, true);
  xhr.setRequestHeader('Content-Type', 'application/json');
  xhr.onload = function() {
    if (xhr.status === 200) {
      const response = JSON.parse(xhr.responseText);
      callback(null, response);
    } else {
      callback(xhr.status);
    }
  };
  xhr.send(JSON.stringify(data)); 
}

function spinnerOn() {
	spn = document.getElementById('spn');
	if(spn) {
  	spn.classList.add('spinner');
  	spn.style.display = "block";
	}
}

function spinnerOff() {
	sp = document.getElementById('spn');
	if(sp) {
		sp.style.display = "none";
		sp.classList.remove('spinner');
	}
}
</script>
)rawliteral";