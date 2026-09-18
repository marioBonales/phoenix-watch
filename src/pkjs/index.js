var Clay = require('@rebble/clay');
var clayConfig = require('./config');
var clay = new Clay(clayConfig);

var xhrRequest = function(url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function (){
    callback(this.responseText);
  }
  xhr.open(type, url);
  xhr.send();
}

function weatherCodeToCondition(code) {
  if (code === 0) return 'Clear';
  if (code <= 3) return 'Cloudy';
  if (code <= 48) return 'Fog';
  if (code <= 55) return 'Drizzle';
  if (code <= 57) return 'Fz. Drizzle';
  if (code <= 65) return 'Rain';
  if (code <= 67) return 'Fz. Rain';
  if (code <= 75) return 'Snow';
  if (code <= 77) return 'Snow Grains';
  if (code <= 82) return 'Showers';
  if (code <= 86) return 'Snow Shwrs';
  if (code === 95) return 'T-Storm';
  if (code <= 99) return 'T-Storm';
  return 'Unknown';
}

function locationSuccess(pos) {
  var url = 'https://api.open-meteo.com/v1/forecast?' +
      'latitude=' + pos.coords.latitude +
      '&longitude=' + pos.coords.longitude +
      '&current=temperature_2m,weather_code'+
      '&hourly=precipitation_probability' +
      '&forecast_hours=10';

  console.log(url);

  xhrRequest(url, 'GET', 
    function(responseText) {
      var json = JSON.parse(responseText);
      
      var temp = Math.round(json.current.temperature_2m);
      var conditions = weatherCodeToCondition(json.current.weather_code);

      //Get max precipitation probablity for the day
      var precipitation_probability = Math.max(...json.hourly.precipitation_probability);
      console.log(json.hourly.precipitation_probability);

      var dictionary = {
        "Temp": temp,
        "Conditions": conditions,
        "Precipitation": precipitation_probability
      }

      Pebble.sendAppMessage(dictionary, 
        function(e) { console.log('Weather Info sent'); },
        function(e) { console.log('Error sending weather data'); }
      );
    }
  )

}

function locationError(err) {
  console.log('Error getting location');
}

function getWeather() {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    { timeout: 15000, maximumAge: 60000 }
  )
}

Pebble.addEventListener('ready',
  function(e) {
    getWeather();
  }
)

Pebble.addEventListener('appMessage',
  function(e) {
    if(e.payload['RequestWeather']){
      getWeather();
    }
  }
)
