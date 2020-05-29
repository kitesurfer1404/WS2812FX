#include <pgmspace.h>
char index_html[] PROGMEM = R"=====(
<!doctype html>
<html lang='en' dir='ltr'>
<head>
  <meta http-equiv='Content-Type' content='text/html; charset=utf-8' />
  <meta name='viewport' content='width=device-width, initial-scale=1.0' />
  <title>WS2812FX Control</title>
  <script type='text/javascript' src='main.js'></script>

  <style>
    body {
      font-family:Arial,sans-serif;
      margin:10px;
      padding:0;
      background-color:#202020;
      color:#909090;
      text-align:center;
    }

    .flex-row {
      display:flex;
      flex-direction:row;
    }

    .flex-row-wrap {
      display:flex;
      flex-direction:row;
      flex-wrap:wrap;
    }

    .flex-col {
      display:flex;
      flex-direction:column;
      align-items:center;
    }

    input[type='text'] {
      background-color: #d0d0d0;
      color:#404040;
    }

    ul {
      list-style-type: none;
    }

    ul li a {
      display:block;
      margin:3px;
      padding:10px;
      border:2px solid #404040;
      border-radius:5px;
      color:#909090;
      text-decoration:none;
    }

    ul#modes li a {
      min-width:220px;
    }

    ul.control li a {
      min-width:60px;
      min-height:24px;
    }

    ul.control {
      display:flex;
      flex-direction:row;
      justify-content: flex-end;
      align-items: center;
      padding: 0px;
    }

    ul li a.active {
      border:2px solid #909090;
    }
  </style>
</head>
<body>
  <h1>WS2812FX Control</h1>
  <div class='flex-row'>

    <div class='flex-col'>
      <div><canvas id='color-canvas' width='360' height='360'></canvas><br/></div>
      <div><input type='text' id='color-value' oninput='onColor(event, this.value)'/></div>

      <div>
        <ul class='control'>
          <li>Brightness:</li>
          <li><a href='#' onclick="onBrightness(event, '-')">&#9788;</a></li>
          <li><a href='#' onclick="onBrightness(event, '+')">&#9728;</a></li>
        </ul>

        <ul class='control'>
          <li>Speed:</li>
          <li><a href='#' onclick="onSpeed(event, '-')">&#8722;</a></li>
          <li><a href='#' onclick="onSpeed(event, '+')">&#43;</a></li>
        </ul>

        <ul class='control'>
          <li>Auto cycle:</li>
          <li><a href='#' onclick="onAuto(event, '-')">&#9632;</a></li>
          <li><a href='#' onclick="onAuto(event, '+')">&#9658;</a></li>
        </ul>
      </div>
    </div>

    <div>
      <ul id='modes' class='flex-row-wrap'>
    </div>
  </div>
</body>
</html>
)=====";
