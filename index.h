const char MAIN_page[] PROGMEM = R"=====(
<html>
  <head>
    <script>
      var connection = new WebSocket("ws://" + location.hostname + ":81/", ["arduino"]);
      connection.onopen = function () {
        connection.send("Connect " + new Date());
      };
      connection.onerror = function (error) {
        console.log("WebSocket Error ", error);
      };
      connection.onmessage = function (e) {
        console.log("Server: ", e.data);
      };
      function sendServo(id, value) {
        var s = id + parseInt(value).toString(16) + "0000";
        console.log("SERVO: " + s);
        connection.send(s);
      }
    </script>
  </head>
  <body>
    <center>
      ESP8266<br />
      WebSocket Arm Control SG90<br />
      <input id="X" type="range" min="0" max="180" step="1" oninput="sendServo(this.id,this.value);"><br />
      <input id="Y" type="range" min="0" max="180" step="1" oninput="sendServo(this.id,this.value);"><br />
      <input id="Z" type="range" min="0" max="180" step="1" oninput="sendServo(this.id,this.value);"><br />
      <input id="G" type="range" min="0" max="180" step="1" oninput="sendServo(this.id,this.value);"><br />
    </center>
  </body>
</html>
)=====";
