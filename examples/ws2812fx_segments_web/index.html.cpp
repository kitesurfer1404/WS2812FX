#include <pgmspace.h>
char index_html[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
  <link rel="stylesheet" href="https://fonts.googleapis.com/icon?family=Material+Icons">
  <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/materialize/1.0.0/css/materialize.min.css">
  <!-- we're using the standard noUiSlider plugin, not the customized one in the materialize extras folder -->
  <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/noUiSlider/11.1.0/nouislider.min.css">

  <style>
    body { /* materialize sticky footer CSS */
      display: flex;
      min-height: 100vh;
      flex-direction: column;
    }
    main { /* materialize sticky footer CSS */
      flex: 1 0 auto;
    }
    .modal { /* make the modal a little wider */
      width: 80% !important;
    }
    .noUi-tooltip { /* only show slider tooltips when the slider is being moved */
      display: none;
    }
    .noUi-active .noUi-tooltip {
      display: block;
    }
  </style>

  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0" />
</head>

<body>
  <header>
    <nav class="grey" role="navigation">
      <div class="nav-wrapper container">
        <a id="logo-container" href="#" class="brand-logo">WS2812FX Web Interface</a>
      </div>
    </nav>
  </header>

  <main>
    <div class="container">
      <div class="row">
        <div class="col s12">
          <h5>Num Pixels</h5>
          <div id="numPixelsSlider"></div>

          <h5>Brightness</h5>
          <div id="brightnessSlider"></div>
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
          <div id="speedSlider"></div><br>

          <h6>Segment Mode</h6>
          <div>
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
              </div><br>

              <h6>Gamma Correction</h6>
              <div class="switch">
                <label>Off<input id="gamma" type="checkbox" onchange="updateSegment()"><span class="lever"></span>On</label>
              </div><br>

              <h6>Fade Rate</h6>
              <div>
                <select id="fade" onchange="updateSegment()">
                  <option value="0" selected>Default</option>
                  <option value="1">XFAST</option>
                  <option value="2">FAST</option>
                  <option value="3">MEDIUM</option>
                  <option value="4">SLOW</option>
                  <option value="5">XSLOW</option>
                  <option value="6">XXSLOW</option>
                  <option value="7">GLACIAL</option>
                </select>
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

          <a class="waves-effect waves-light btn" onclick="LED_pause()">
            <i class="material-icons left">pause</i>Pause</a>

          <a class="waves-effect waves-light btn" onclick="LED_resume()">
            <i class="material-icons left">replay</i>Play</a>

          <a class="waves-effect waves-light btn" onclick="LED_on()">
            <i class="material-icons left">blur_on</i>ON</a>

          <a class="waves-effect waves-light btn" onclick="LED_off()">
            <i class="material-icons left">blur_off</i>OFF</a>
        </div>
      </div>
    </div>
  </main>

  <footer class="grey page-footer">
    <div class="grey footer-copyright">
      <div class="container">&copy; 2018
        <span class="right">Made with
          <a class="black-text" href="http://materializecss.com">Materialize</a>
        </span>
      </div>
    </div>
  </footer>

  <!-- Code Modal  -->
  <div id="codeModal" class="modal">
    <div class="modal-content">
      <h4>ESP8266/Arduino Code</h4>
      <div class="input-field">
        <textarea id="codeModalContent" class="materialize-textarea" data-length="120"></textarea>
      </div>
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
  <script type="text/javascript" src="https://code.jquery.com/jquery-3.3.1.min.js"></script>
  <script type="text/javascript" src="https://cdnjs.cloudflare.com/ajax/libs/materialize/1.0.0/js/materialize.min.js"></script>
  <script type="text/javascript" src="https://cdnjs.cloudflare.com/ajax/libs/noUiSlider/11.1.0/nouislider.min.js"></script>

  <script type="text/javascript">
    // global variables
    var pin = "?";
    var numPixels = 30;
    var brightness = 255;
    var segmentIndex = 0;
    var segments = [
      { start: 0, stop: 9, mode: 0, speed: 1000, options: 0, colors: ['#ff0000', '#00ff00', '#0000ff'] }
    ];

    // onready function
    $(document).ready(function() {
      console.log("ready!");
      initSliders();
      $('.modal').modal(); // init materialize modals
      getModes(); // get WS2812FX modes (callback execs updateWidgets())
    });

    function initSliders() {
      noUiSlider.create(document.getElementById('rangeSlider'), {
        start: [0, 99],
        tooltips: [true, true],
        connect: [false, true, false],
        range: {
          'min': 0,
          'max': numPixels - 1
        },
        format: {
          to: function(value) {
            return value.toFixed(0);
          },
          from: function(value) {
            return parseInt(value, 10);
          }
        }
      });
      document.getElementById('rangeSlider').noUiSlider.on('end', function() {
        updateSegment();
      });

      noUiSlider.create(document.getElementById('numPixelsSlider'), {
        start: numPixels,
        tooltips: true,
        connect: [true, false],
        range: {
          'min': 1,
          'max': 600
        },
        format: {
          to: function(value) {
            return value.toFixed(0);
          },
          from: function(value) {
            return parseInt(value, 10);
          }
        }
      });
      document.getElementById('numPixelsSlider').noUiSlider.on('update', function() {
        numPixels = document.getElementById('numPixelsSlider').noUiSlider.get();
        rangeSlider.noUiSlider.updateOptions({
          range: {
            'min': 0,
            'max': Math.max(1, numPixels - 1)
          }
        });
      });

      noUiSlider.create(document.getElementById('brightnessSlider'), {
        start: 255,
        tooltips: true,
        connect: [true, false],
        range: {
          'min': 0,
          'max': 255
        },
        format: {
          to: function(value) {
            return value.toFixed(0);
          },
          from: function(value) {
            return parseInt(value, 10);
          }
        }
      });
      document.getElementById('brightnessSlider').noUiSlider.on('end', function() {
        brightness = document.getElementById('brightnessSlider').noUiSlider.get();
      });

      noUiSlider.create(document.getElementById('speedSlider'), {
        start: 1000,
        tooltips: true,
        connect: [true, false],
        step: 10,
        range: {
          'min': 20,
          'max': 2000
        },
        format: {
          to: function(value) {
            return value.toFixed(0);
          },
          from: function(value) {
            return parseInt(value, 10);
          }
        }
      });
      document.getElementById('speedSlider').noUiSlider.on('end', function() {
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

      // calc the new segment's start led by adding one to the maximum stop led
      var start = 0;
      for(var i=0; i < segments.length; i++) {
        if(segments[i].stop >= start) start = segments[i].stop + 1;
      }
      if(start > numPixels - 1) start = numPixels - 1;

      segments.push({ start: start, stop: numPixels - 1, mode: 0, speed: 1000, options: 0, colors: ['#ff0000', '#00ff00', '#0000ff'] });
      segmentIndex = segments.length - 1;
      updateWidgets();
    }

    function deleteSegment(event, index) {
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

      segments[segmentIndex].options = $('#reverse').prop('checked') ? 0x80 : 0;
      segments[segmentIndex].options |= $('#gamma').prop('checked')  ? 0x08 : 0;
      segments[segmentIndex].options |= $('#fade').val() ? $('#fade').val() << 4 : 0;

      segments[segmentIndex].colors[0] = $('#color0').val();
      segments[segmentIndex].colors[1] = $('#color1').val();
      segments[segmentIndex].colors[2] = $('#color2').val();
      updateWidgets();
    }

    // update GUI widgets
    function updateWidgets() {
      numPixelsSlider.noUiSlider.set(numPixels);
      brightnessSlider.noUiSlider.set(brightness);

      // recreate the segment list HTML
      $("#segmentList").empty();
      for (var i = 0; i < segments.length; i++) {
        $("#segmentList").append('<li class="collection-item" onclick="changeSegment($(this))">' +
          segments[i].start + ' - ' + segments[i].stop + ' : ' + getModeName(segments[i].mode) +
          '<i class="material-icons right" style="cursor: pointer" onclick="deleteSegment(event, ' + i + ')">delete</i>' +
          '</li>');
      }
      $("#segmentList li").eq(segmentIndex).addClass('active');

      // update the materialize widgets with the current segment's data
      if(segments.length > 0) {
        rangeSlider.noUiSlider.set([segments[segmentIndex].start, segments[segmentIndex].stop]);
        speedSlider.noUiSlider.set(segments[segmentIndex].speed);
        $('#modes').val(segments[segmentIndex].mode);
        $('#modes').formSelect();
        $('#fade').val((segments[segmentIndex].options & 0x70) >> 4);
        $('#fade').formSelect();
        $('#reverse').prop('checked', Boolean(segments[segmentIndex].options & 0x80));
        $('#gamma').prop('checked',   Boolean(segments[segmentIndex].options & 0x08));
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
        brightness = data.brightness;

        segments.length = 0;
        segmentIndex = 0;
        $.each(data.segments, function(i, item) {
          segments.push({
            start: item.start,
            stop: item.stop,
            mode: item.mode,
            speed: item.speed,
            options: item.options,
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

    // PAUSE
    function LED_pause() {
      jQuery.ajax({
        url: "runcontrol",
        type: "POST",
        data: "pause",
        dataType: "json",
        contentType: "application/json; charset=utf-8",
//      contentType: "application/x-www-form-urlencoded; charset=utf-8",
        success: function() {
          console.log(data);
        }
      });
    }

    // Resume after Pause
    function LED_resume() {
      jQuery.ajax({
        url: "runcontrol",
        type: "POST",
        data: "resume",
        dataType: "json",
        contentType: "application/json; charset=utf-8",
//      contentType: "application/x-www-form-urlencoded; charset=utf-8",
        success: function() {
          console.log(data);
        }
      });
    }

    // Switch strip on
    function LED_on() {
      jQuery.ajax({
        url: "runcontrol",
        type: "POST",
        data: "run",
        dataType: "json",
        contentType: "application/json; charset=utf-8",
//      contentType: "application/x-www-form-urlencoded; charset=utf-8",
        success: function() {
          console.log(data);
        }
      });
    }

     // Switch Strip off
    function LED_off() {
      jQuery.ajax({
        url: "runcontrol",
        type: "POST",
        data: "stop",
        dataType: "json",
        contentType: "application/json; charset=utf-8",
//      contentType: "application/x-www-form-urlencoded; charset=utf-8",
        success: function() {
          console.log(data);
        }
      });
    }

    // send the segment info to the web server in JSON format
    function save() {
      json = '{';
      json += '"numPixels":' + numPixels;
      json += ',"brightness":' + brightness;
      json += ',"segments":[';
      $.each(segments, function(i, item) {
        if(i != 0) json += ',';
        json += '{';
        json += '"start":' + item.start;
        json += ',"stop":' + item.stop;
        json += ',"mode":' + item.mode;
        json += ',"speed":' + item.speed;
        json += ',"options":' + item.options;
        json += ',"colors":[' +
          // transform the color info from '#000000' format into a number
          parseInt(item.colors[0].replace('#', ''), 16) + ',' +
          parseInt(item.colors[1].replace('#', ''), 16) + ',' +
          parseInt(item.colors[2].replace('#', ''), 16) + ']';
        json += "}";
      });
      json += "]}";

//    $.post("setsegments", json, function(data){
//      console.log(data);
//    });
      jQuery.ajax({
        url: "setsegments",
        type: "POST",
        data: JSON.stringify(json),
        dataType: "json",
        contentType: "application/json; charset=utf-8",
//      contentType: "application/x-www-form-urlencoded; charset=utf-8",
        success: function() {
          console.log(data);
        }
      });
    }

    // retrieve the mode info from the web server in JSON format
    function getModes() {
      $.getJSON("getmodes", function(data) {
        $.each(data, function (i, item) {
          $('#modes').append(new Option(item, i));
        });
        updateWidgets();
      });
    }

    function getModeName(index) {
      var name = "";
      $("#modes option").each(function(i, item) {
        if(index == item.value) {
          name = item.text;
          return false; // exit the each loop
        }
      });
      return name;
    }

    function buildCode() {
      var code = '#include <WS2812FX.h>\n\n'
      code += '#define LED_PIN ' + pin + '\n';
      code += '#define LED_COUNT ' + numPixels + '\n\n';
      code += 'WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);\n\n';
      code += 'void setup() {\n';
      code += '  ws2812fx.setBrightness(' + brightness + ');\n\n';

      $.each(segments, function(index, segment) {
//  uint32_t colors0[] = {0xff0000, 0x000000, 0x000000};
        code += '  uint32_t colors' + index + '[] = {' +
          segment.colors[0].replace('#', '0x') + ', ' +
          segment.colors[1].replace('#', '0x') + ', ' +
          segment.colors[2].replace('#', '0x') + '};\n';

// ws2812fx.setSegment(0, 0, 9, 53, colors0, 240, false);
        code += '  ws2812fx.setSegment(' +
          index + ', ' +
          segment.start + ', ' +
          segment.stop + ', ' +
          segment.mode + ', ' +
          'colors' + index + ', ' +
          segment.speed + ', ' +
          segment.options +
          '); // ' + getModeName(segment.mode) + '\n\n';
      });

      code += '  ws2812fx.start();\n';
      code += '}\n\n';
      code += 'void loop() {\n';
      code += '  ws2812fx.service();\n';
      code += '}';

      $("#codeModalContent").html(code);
      M.textareaAutoResize($('#codeModalContent'));

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
