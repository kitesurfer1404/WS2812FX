#include <pgmspace.h>
char index_html[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang='en'>
<head>
  <meta http-equiv='Content-Type' content='text/html; charset=utf-8' />
  <meta name='viewport' content='width=device-width' />

  <title>WS2812FX Ctrl</title>

  <script type='text/javascript' src='main.js'></script>

  <style>
  * {
    font-family:sans-serif;
    margin:0;
    padding:0;
  }

  body {
    width:100%;
    max-width:675px;
    background-color:#202020;
  }
  
  h1 {
    width:65%;
    margin:25px 0 25px 25%;
    color:#454545;
    text-align:center;
  }
  
  #colorbar {
    float:left;
  }
  
  #controls {
    width:65%;
    display:inline-block;
    padding-left:5px;
  }

  ul {
    text-align:center;
  }

  ul#mode li {
    display:block;
  }

  ul#brightness li, ul#speed li, ul#auto li {
    display:inline-block;
    width:30%;
  }

  ul li a {
    display:block;
    margin:3px;
    padding:10px 5px;
    border:2px solid #454545;
    border-radius:5px;
    color:#454545;
    font-weight:bold;
    text-decoration:none;
  }

  ul li a.active {
    border:2px solid #909090;
    color:#909090;
  }
  </style>
</head>
<body>
  <h1>WS2812FX Control</h1>
  <canvas id='colorbar' width='75' height='1080'></canvas>
  <div id='controls'>
    <ul id='mode'></ul>

    <ul id='brightness'>
      <li><a href='#' class='b' id='-'>&#9788;</a></li>
      <li><a href='#' class='b' id='+'>&#9728;</a></li>
    </ul>

    <ul id='speed'>
      <li><a href='#' class='s' id='-'>&#8722;</a></li>
      <li><a href='#' class='s' id='+'>&#43;</a></li>
    </ul>

    <ul id='auto'>
      <li><a href='#' class='a' id='-'>&#9632;</a></li>
      <li><a href='#' class='a' id='+'>&#9658;</a></li>
    </ul>
  </div>
</body>
</html>
)=====";

