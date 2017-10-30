#include <pgmspace.h>
char index_html[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
  <link type="text/css" rel="stylesheet" href="https://fonts.googleapis.com/icon?family=Material+Icons">
  <link type="text/css" rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/materialize/0.100.2/css/materialize.min.css">
  <link type="text/css" rel="stylesheet" href="http://champlainsystems.com/css/nouislider.css">

  <meta name="viewport" content="width=device-width, initial-scale=1.0" />
</head>

<body>
  <nav class="grey" role="navigation">
    <div class="nav-wrapper container">
      <a id="logo-container" href="#" class="brand-logo">WS2812FX Web Interface</a>
    </div>
  </nav>

  <div class="container">
    <div class="row">
      <div class="col s12">
        <h5>Num Pixels</h5>
        <div id="numPixelsSlider"></div>
      </div>
    </div><br>

    <div class="row">
      <div class="col s6">
        <h5>Segments <a href="#" onclick="addSegment(event)"><i class="material-icons">add_box</i></a></h5>
        <ul id="segmentList" class="collection"></ul>
      </div>

      <div class="col s6">
        <h6>Segment Range</h6>
        <div id="rangeSlider"></div><br>
        <h6>Segment Speed</h6>
        <div id="speedSlider"></div>
        <div class="input-field">
          <select id="modes" onchange="updateSegment()"></select>
        </div>
        <div class="row">
          <div class="col s6">
            <h6>Colors</h6>
            <input type="color" id="color0" onchange="updateSegment()">
            <input type="color" id="color1" onchange="updateSegment()">
            <input type="color" id="color2" onchange="updateSegment()">
          </div>
          <div class="col s6">
            <h6>Reverse</h6>
            <div class="switch">
              <label>Off<input id="reverse" type="checkbox" onchange="updateSegment()"><span class="lever"></span>On</label>
            </div>
          </div>
        </div>
      </div>
    </div>

    <div class="row">
      <div class="col s12">
        <a class="waves-effect waves-light btn" onclick="load()">
          <i class="material-icons left">cloud_download</i>Load</a>
        <a class="waves-effect waves-light btn" onclick="save()">
          <i class="material-icons left">cloud_upload</i>Save</a>
        <a class="waves-effect waves-light btn" onclick="buildCode()">
          <i class="material-icons left">code</i>Code</a>
      </div>
    </div>
  </div>

  <br><br>
  <footer class="grey page-footer">
    <div class="grey footer-copyright">
      <div class="container">
        Made by <a class="black-text" href="http://materializecss.com">Materialize</a>
      </div>
    </div>
  </footer>

  <!-- Code Modal  -->
  <div id="codeModal" class="modal">
    <div class="modal-content">
      <h4>ESP8266/Arduino Code</h4>
      <textarea id="codeModalContent" class="materialize-textarea" data-length="120"></textarea>
    </div>
    <div class="modal-footer">
      <a class="waves-effect waves-light btn left" onclick="code2clipboard()">
        <i class="material-icons left">content_copy</i>Copy</a>
      <a href="#!" class="modal-action modal-close waves-effect waves-green btn-flat">Close</a>
    </div>
  </div>




  <!-- 
    javascript goes here
  -->
  <script type="text/javascript" src="https://code.jquery.com/jquery-3.2.1.min.js"></script>
  <script type="text/javascript" src="https://cdnjs.cloudflare.com/ajax/libs/materialize/0.100.2/js/materialize.min.js"></script>
  <script type="text/javascript" src="http://champlainsystems.com/js/nouislider.min.js"></script>
  
  <script type="text/javascript">
    // global variables
    var pin = "?";
    var numPixels = 30;
    var segmentIndex = 0;
    var segments = [
      {start: 0, stop: 9, mode:0, speed:200, reverse:false, colors:['#ff0000','#00ff00','#0000ff']}
    ];

    // onready function
    $(document).ready(function () {
      console.log("ready!");
      initSliders();
      $('.modal').modal(); // init materialize modals
      getModes(); // get WS2812FX modes (callback execs updateWidgets())
    });

    function initSliders() {
      noUiSlider.create(document.getElementById('rangeSlider'), {
        start: [0, 99],
        connect: [false, true, false],
        range: {
          'min': 0,
          'max': numPixels - 1
        },
        format: wNumb({
          decimals: 0,
          encoder: function (a) {
            return Math.round(a);
          }
        })
      });
      document.getElementById('rangeSlider').noUiSlider.on('end', function(){
        updateSegment();
      });

      noUiSlider.create(document.getElementById('numPixelsSlider'), {
        start: numPixels,
        connect: [true, false],
        range: {
          'min': 1,
          'max': 600
        },
        format: wNumb({
          decimals: 0,
          encoder: function (a) {
            return Math.round(a);
          }
        })
      });
      document.getElementById('numPixelsSlider').noUiSlider.on('update', function(){
        numPixels = document.getElementById('numPixelsSlider').noUiSlider.get();
        rangeSlider.noUiSlider.updateOptions({
          range: {
            'min': 0,
            'max': numPixels - 1
          }
        });
      });

      noUiSlider.create(document.getElementById('speedSlider'), {
        start: 200,
        connect: [true, false],
        range: {
          'min': 0,
          'max': 255
        },
        format: wNumb({
          decimals: 0,
          encoder: function (a) {
            return Math.round(a);
          }
        })
      });
      document.getElementById('speedSlider').noUiSlider.on('end', function(){
        updateSegment();
      });
    }

    function changeSegment(elem) {
      elem.addClass("active").siblings().removeClass('active');
      segmentIndex = elem.index();
      updateWidgets();
    }
    
    function addSegment(event) {
      event.preventDefault();
      event.stopPropagation();
      if(segments.length > 9) return; // max 10 segments

      var start = 0;
      for (var i = 0; i < segments.length; i++) {
        if(segments[i].stop >= start) start = segments[i].stop + 1;
      }
      if(start > numPixels - 1) start = numPixels - 1;

      segments.push({start:start, stop:numPixels - 1, mode:0, speed:200, reverse:false, colors:['#ff0000','#00ff00','#0000ff']});
      segmentIndex = segments.length - 1;
      updateWidgets();
    }

    function deleteSegment(index) {
      event.preventDefault();
      event.stopPropagation();
      segments.splice(index, 1);
      if(segmentIndex >= segments.length) segmentIndex = segments.length - 1;
      updateWidgets();
    }

    function updateSegment() {
      segments[segmentIndex].start = parseInt(rangeSlider.noUiSlider.get()[0]);
      segments[segmentIndex].stop = parseInt(rangeSlider.noUiSlider.get()[1]);
      segments[segmentIndex].speed = parseInt(speedSlider.noUiSlider.get());
      segments[segmentIndex].mode = $('#modes').val();
      segments[segmentIndex].reverse = $('#reverse').prop('checked');
      segments[segmentIndex].colors[0] = $('#color0').val();
      segments[segmentIndex].colors[1] = $('#color1').val();
      segments[segmentIndex].colors[2] = $('#color2').val();
      updateWidgets();
    }

    // update GUI widgets
    function updateWidgets() {
      numPixelsSlider.noUiSlider.set(numPixels);

      $("#segmentList").empty();
      for (var i = 0; i < segments.length; i++) {
        $("#segmentList").append('<li class="collection-item" onclick="changeSegment($(this))">'+
          segments[i].start + ' - '+ segments[i].stop + ' : ' + getModeName(segments[i].mode)+
          '<i class="material-icons right" style="cursor: pointer" onclick="deleteSegment('+i+')">delete</i>'+
          '</li>');
      }
      $("#segmentList li").eq(segmentIndex).addClass('active');

      if(segments.length > 0) {
        rangeSlider.noUiSlider.set([segments[segmentIndex].start, segments[segmentIndex].stop]);
        speedSlider.noUiSlider.set(segments[segmentIndex].speed);
        $('#modes').val(segments[segmentIndex].mode);
        $('#modes').material_select(); // re-initialize material-select
        $('#reverse').prop('checked', segments[segmentIndex].reverse);
        $('#color0').val(segments[segmentIndex].colors[0]);
        $('#color1').val(segments[segmentIndex].colors[1]);
        $('#color2').val(segments[segmentIndex].colors[2]);
      }
    }

    // retrieve the segment info from the web server in JSON format
    function load() {
      $.getJSON("getsegments", function(data) {
        pin = data.pin;
        numPixels = data.numPixels;

        segments.length = 0;
        segmentIndex = 0;
        $.each(data.segments, function (i, item) {
          segments.push({
            start: item.start,
            stop: item.stop,
            mode: item.mode,
            speed: item.speed,
            reverse: item.reverse,
            // transform the color info from a number into '#000000' format
            colors: [ // convert int colors to HTML representation
              '#' + ('000000' + item.colors[0].toString(16)).substr(-6),
              '#' + ('000000' + item.colors[1].toString(16)).substr(-6),
              '#' + ('000000' + item.colors[2].toString(16)).substr(-6),
             ]
          });
        });

        updateWidgets();
      });
    }

    // send the segment info to the web server in JSON format
    function save() {
      json = '{';
      json += '"numPixels":' + numPixels;
      json += ',"segments":[';
      $.each(segments, function (i, item) {
        if(i != 0) json += ',';
        json += '{';
        json += '"start":'+item.start;
        json += ',"stop":'+item.stop;
        json += ',"mode":'+item.mode;
        json += ',"speed":'+item.speed;
        json += ',"reverse":'+item.reverse;
        json += ',"colors":['+
          // transform the color info from '#000000' format into a number
          parseInt(item.colors[0].replace('#',''), 16)+','+
          parseInt(item.colors[1].replace('#',''), 16)+','+
          parseInt(item.colors[2].replace('#',''), 16)+']';
        json += "}";
      });
      json += "]}";

      $.post("setsegments", json, function(data){
        console.log(data);
      });
    }

    // retrieve the mode info from the web server in JSON format
    function getModes() {
      $.getJSON("getmodes", function(data){
        $.each(data, function (i, item) {
          $('#modes').append(new Option(item, i));
        });
        updateWidgets();
      });
    }

    function getModeName(index) {
      name = "";
      $("#modes option").each(function(i, item) {
        if(index == item.value) {
          name = item.text;
          return false;
        }
      });
      return name;
    }

    function buildCode() {
      var code = '#define LED_PIN ' + pin + '\n';
      code += '#define LED_COUNT ' + numPixels + '\n\n';
      code += 'WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);\n\n';
      code += 'void setup() {\n';
      $.each(segments, function( index, segment ) {
// ws2812fx.setSegment(0, 0, 9, 53, (const uint32_t[]) {0xff0000, 0, 0}, 240, false);
        code += '  ws2812fx.setSegment('+
          index + ', ' +
          segment.start + ', ' +
          segment.stop + ', ' +
          segment.mode + ', (const uint32_t[]) {' +
          segment.colors[0].replace('#','0x') + ', ' +
          segment.colors[1].replace('#','0x') + ', ' +
          segment.colors[2].replace('#','0x') + '}, ' +
          segment.speed + ', ' +
          segment.reverse +
          ');\n';
      });
      code += '  ws2812fx.start();\n'
      code += '}\n\n'
      code += 'void loop() {\n'
      code += '  ws2812fx.service();\n'
      code += '}\n\n'

      $("#codeModalContent").html(code);
      $('#codeModalContent').trigger('autoresize');

      $('#codeModal').modal('open');
    }

    function code2clipboard() {
      var content = document.getElementById("codeModalContent");
      content.focus();
      content.select();
      document.execCommand('copy');
      window.getSelection().removeAllRanges();
      content.blur();
    }
  </script>
</body>
</html>
)=====";
