#include <pgmspace.h>

/*
The tiny Javascript/canvas based color picker is based on the clever work of the folks
at Sparkbox. https://seesparkbox.com/foundry/how_i_built_a_canvas_color_picker
*/

char main_js[] PROGMEM = R"=====(

var activeButton = null;
var colorCanvas = null;

window.addEventListener('DOMContentLoaded', (event) => {
  // init the canvas color picker
  colorCanvas = document.getElementById('color-canvas');
  var colorctx = colorCanvas.getContext('2d');

  // Create color gradient
  var gradient = colorctx.createLinearGradient(0, 0, colorCanvas.width - 1, 0);
  gradient.addColorStop(0,    "rgb(255,   0,   0)");
  gradient.addColorStop(0.16, "rgb(255,   0, 255)");
  gradient.addColorStop(0.33, "rgb(0,     0, 255)");
  gradient.addColorStop(0.49, "rgb(0,   255, 255)");
  gradient.addColorStop(0.66, "rgb(0,   255,   0)");
  gradient.addColorStop(0.82, "rgb(255, 255,   0)");
  gradient.addColorStop(1,    "rgb(255,   0,   0)");

  // Apply gradient to canvas
  colorctx.fillStyle = gradient;
  colorctx.fillRect(0, 0, colorCanvas.width - 1, colorCanvas.height - 1);

  // Create semi transparent gradient (white -> transparent -> black)
  gradient = colorctx.createLinearGradient(0, 0, 0, colorCanvas.height - 1);
  gradient.addColorStop(0,    "rgba(255, 255, 255, 1)");
  gradient.addColorStop(0.48, "rgba(255, 255, 255, 0)");
  gradient.addColorStop(0.52, "rgba(0,     0,   0, 0)");
  gradient.addColorStop(1,    "rgba(0,     0,   0, 1)");

  // Apply gradient to canvas
  colorctx.fillStyle = gradient;
  colorctx.fillRect(0, 0, colorCanvas.width - 1, colorCanvas.height - 1);

  // setup the canvas click listener
  colorCanvas.addEventListener('click', (event) => {
    var imageData = colorCanvas.getContext('2d').getImageData(event.offsetX, event.offsetY, 1, 1);

    var selectedColor = 'rgb(' + imageData.data[0] + ',' + imageData.data[1] + ',' + imageData.data[2] + ')'; 
    //console.log('click: ' + event.offsetX + ', ' + event.offsetY + ', ' + selectedColor);
    document.getElementById('color-value').value = selectedColor;

    selectedColor = imageData.data[0] * 65536 + imageData.data[1] * 256 + imageData.data[2];
    submitVal('c', selectedColor);
  });

  // get list of modes from ESP
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
   if (xhttp.readyState == 4 && xhttp.status == 200) {
     document.getElementById('modes').innerHTML = xhttp.responseText;
     modes = document.querySelectorAll('ul#modes li a');
     modes.forEach(initMode);
   }
  };
  xhttp.open('GET', 'modes', true);
  xhttp.send();
});

function initMode(mode, index) {
  mode.addEventListener('click', (event) => onMode(event, index));
}

function onColor(event, color) {
  event.preventDefault();
  var match = color.match(/rgb\(([0-9]*),([0-9]*),([0-9]*)\)/);
  if(match) {
    var colorValue = Number(match[1]) * 65536 + Number(match[2]) * 256 + Number(match[3]);
    //console.log('onColor:' + match[1] + "," + match[2] + "," + match[3] + "," + colorValue);
    submitVal('c', colorValue);
  }
}

function onMode(event, mode) {
  event.preventDefault();
  if(activeButton) activeButton.classList.remove('active')
  activeButton = event.target;
  activeButton.classList.add('active');
  submitVal('m', mode);
}

function onBrightness(event, dir) {
  event.preventDefault();
  submitVal('b', dir);
}

function onSpeed(event, dir) {
  event.preventDefault();
  submitVal('s', dir);
}

function onAuto(event, dir) {
  event.preventDefault();
  submitVal('a', dir);
}

function submitVal(name, val) {
  var xhttp = new XMLHttpRequest();
  xhttp.open('GET', 'set?' + name + '=' + val, true);
  xhttp.send();
}
)=====";
