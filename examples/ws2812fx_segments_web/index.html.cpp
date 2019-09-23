#include <pgmspace.h>
char index_html[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
  <link rel="stylesheet" href="https://fonts.googleapis.com/icon?family=Material+Icons">
  <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/noUiSlider/14.0.2/nouislider.min.css">
  <link rel="stylesheet" href="https://unpkg.com/material-components-web@3.2.0/dist/material-components-web.min.css">

  <style>
    body {
      margin: 0; /* remove the default 8px body margin to fix MDC top-app-bar placement */
      --mdc-theme-primary: #3fb8af; /* change MDC color theme to match noUiSlide's theme */
    }
    main, .mdc-switch {
      margin: 0px 8px; /* noUiSliders look a little cramped, so add horizontal margin to main content */
    }
    footer {
      margin: 4px 16px; /* add a little margin to the footer content */
    }
    .mdc-select:not(.mdc-select--disabled).mdc-select--focused .mdc-floating-label {
      color: var(--mdc-theme-primary, #3fb8af) /* MDC select label is hardcoded to purple??? reset to primary color */
    }
    .mdc-list { /* add border around MDC lists */
      border: 1px solid rgba(0, 0, 0, 0.1);
      padding: 0px;
    }
    .mdc-button { /* add margin-bottom to buttons so they wrap properly on small screens */
      margin-bottom: 8px;
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

<body class="mdc-typography">

  <header class="mdc-top-app-bar" data-mdc-auto-init="MDCTopAppBar">
    <div class="mdc-top-app-bar__row">
      <section class="mdc-top-app-bar__section mdc-top-app-bar__section--align-start">
<!--    <button class="material-icons mdc-top-app-bar__navigation-icon mdc-icon-button">menu</button> -->
        <span class="mdc-top-app-bar__title">WS2812FX Web Interface</span>
      </section>
    </div>
  </header>

  <main class="mdc-top-app-bar--fixed-adjust">

    <div class="mdc-layout-grid">
      <div class="mdc-layout-grid__inner">
        <div class="mdc-layout-grid__cell mdc-layout-grid__cell--span-12">
          <h3 class="mdc-typography--subtitle1">Number of Pixels</h3>
          <div id="numPixelsSlider"></div>
        </div>

        <div class="mdc-layout-grid__cell mdc-layout-grid__cell--span-12">
          <h3 class="mdc-typography--subtitle1">Brightness</h3>
          <div id="brightnessSlider"></div>
        </div>

        <div class="mdc-layout-grid__cell mdc-layout-grid__cell--span-5">
          <h3 class="mdc-typography--subtitle1" style="display: inline-block">Segments</h3>
          <button class="mdc-icon-button material-icons" onclick="onAddSegment(event)">add_box</button>
          <ul id="segmentList" class="mdc-list" data-mdc-auto-init="MDCList"></ul>
        </div>

        <div class="mdc-layout-grid__cell mdc-layout-grid__cell--span-7">
          <div class="mdc-layout-grid__inner">
            <div class="mdc-layout-grid__cell mdc-layout-grid__cell--span-12">
              <h3 class="mdc-typography--subtitle1">Segment Range</h3>
              <div id="rangeSlider"></div>
            </div>

            <div class="mdc-layout-grid__cell mdc-layout-grid__cell--span-12">
              <h3 class="mdc-typography--subtitle1">Segment Speed</h3>
              <div id="speedSlider"></div>
            </div>

            <div class="mdc-layout-grid__cell mdc-layout-grid__cell--span-6">
              <div class="mdc-layout-grid__inner">
                
                <div class="mdc-layout-grid__cell mdc-layout-grid__cell--span-12">
                  <h3 class="mdc-typography--subtitle1" style="display: inline-block">Colors</h3>
                  <input type="color" id="color0" onchange="updateSegment()">
                  <input type="color" id="color1" onchange="updateSegment()">
                  <input type="color" id="color2" onchange="updateSegment()">
                </div>

                <div class="mdc-layout-grid__cell mdc-layout-grid__cell--span-12">
                  <h3 class="mdc-typography--subtitle1" style="display: inline-block">Reverse</h3>
                  <div id="reverseSwitch" class="mdc-switch" data-mdc-auto-init="MDCSwitch">
                    <div class="mdc-switch__track"></div>
                    <div class="mdc-switch__thumb-underlay">
                      <div class="mdc-switch__thumb">
                        <input type="checkbox" id="reverse" class="mdc-switch__native-control" role="switch" onchange="updateSegment()">
                      </div>
                    </div>
                  </div>
                  <label for="reverse">off/on</label>
                </div>

                <div class="mdc-layout-grid__cell mdc-layout-grid__cell--span-12">
                  <h3 class="mdc-typography--subtitle1" style="display: inline-block">Gamma Correction</h3>
                  <div id="gammaSwitch" class="mdc-switch" data-mdc-auto-init="MDCSwitch">
                    <div class="mdc-switch__track"></div>
                    <div class="mdc-switch__thumb-underlay">
                      <div class="mdc-switch__thumb">
                        <input type="checkbox" id="gamma" class="mdc-switch__native-control" role="switch" onchange="updateSegment()">
                      </div>
                    </div>
                  </div>
                  <label for="gamma">off/on</label>
                </div>

              </div>
            </div>

            <div class="mdc-layout-grid__cell mdc-layout-grid__cell--span-6">
              <div class="mdc-layout-grid__inner">

                <div class="mdc-layout-grid__cell mdc-layout-grid__cell--span-12">
                  <div class="mdc-select mdc-select--outlined" data-mdc-auto-init="MDCSelect">
                    <i class="mdc-select__dropdown-icon"></i>
                    <select id="modes" class="mdc-select__native-control" onchange="updateSegment()">
                      <option value="0">Static</option>
                    </select>
                    <div class="mdc-notched-outline">
                      <div class="mdc-notched-outline__leading"></div>
                      <div class="mdc-notched-outline__notch">
                        <label class="mdc-floating-label mdc-floating-label--float-above">Segment Mode</label>
                      </div>
                      <div class="mdc-notched-outline__trailing"></div>
                    </div>
                  </div>
                </div>

                <div class="mdc-layout-grid__cell mdc-layout-grid__cell--span-12">
                  <div class="mdc-select mdc-select--outlined" data-mdc-auto-init="MDCSelect">
                    <i class="mdc-select__dropdown-icon"></i>
                    <select id="fade" class="mdc-select__native-control" onchange="updateSegment()">
                      <option value="0" selected>Default</option>
                      <option value="1">XFAST</option>
                      <option value="2">FAST</option>
                      <option value="3">MEDIUM</option>
                      <option value="4">SLOW</option>
                      <option value="5">XSLOW</option>
                      <option value="6">XXSLOW</option>
                      <option value="7">GLACIAL</option>
                    </select>
                    <div class="mdc-notched-outline">
                      <div class="mdc-notched-outline__leading"></div>
                      <div class="mdc-notched-outline__notch">
                        <label class="mdc-floating-label mdc-floating-label--float-above">Fade Rate</label>
                      </div>
                      <div class="mdc-notched-outline__trailing"></div>
                    </div>
                  </div>
                </div>

                <div class="mdc-layout-grid__cell mdc-layout-grid__cell--span-12">
                  <div class="mdc-select mdc-select--outlined" data-mdc-auto-init="MDCSelect">
                    <i class="mdc-select__dropdown-icon"></i>
                    <select id="size" class="mdc-select__native-control" onchange="updateSegment()">
                      <option value="0" selected>SMALL</option>
                      <option value="1">MEDIUM</option>
                      <option value="2">LARGE</option>
                      <option value="3">XLARGE</option>
                    </select>
                    <div class="mdc-notched-outline">
                      <div class="mdc-notched-outline__leading"></div>
                      <div class="mdc-notched-outline__notch">
                        <label class="mdc-floating-label mdc-floating-label--float-above">Size</label>
                      </div>
                      <div class="mdc-notched-outline__trailing"></div>
                    </div>
                  </div>
                </div>

              </div>
            </div>

          </div>
        </div>

        <div class="mdc-layout-grid__cell mdc-layout-grid__cell--span-12">
          <button class="mdc-button mdc-button--raised" onclick="onLoad()" data-mdc-auto-init="MDCRipple">
            <i class="material-icons mdc-button__icon" aria-hidden="true">cloud_download</i>
            <span class="mdc-button__label">Load</span>
          </button>

          <button class="mdc-button mdc-button--raised" onclick="onSave()" data-mdc-auto-init="MDCRipple">
            <i class="material-icons mdc-button__icon" aria-hidden="true">cloud_upload</i>
            <span class="mdc-button__label">Save</span>
          </button>

          <button class="mdc-button mdc-button--raised" onclick="onBuildCode()" data-mdc-auto-init="MDCRipple">
            <i class="material-icons mdc-button__icon" aria-hidden="true">code</i>
            <span class="mdc-button__label">Code</span>
          </button>

          <button class="mdc-button mdc-button--raised" onclick="onPause()" data-mdc-auto-init="MDCRipple">
            <i class="material-icons mdc-button__icon" aria-hidden="true">pause</i>
            <span class="mdc-button__label">Pause</span>
          </button>

          <button class="mdc-button mdc-button--raised" onclick="onResume()" data-mdc-auto-init="MDCRipple">
            <i class="material-icons mdc-button__icon" aria-hidden="true">play_arrow</i>
            <span class="mdc-button__label">Play</span>
          </button>

          <button class="mdc-button mdc-button--raised" onclick="onStart()" data-mdc-auto-init="MDCRipple">
            <i class="material-icons mdc-button__icon" aria-hidden="true">blur_on</i>
            <span class="mdc-button__label">On</span>
          </button>

          <button class="mdc-button mdc-button--raised" onclick="onStop()" data-mdc-auto-init="MDCRipple">
            <i class="material-icons mdc-button__icon" aria-hidden="true">blur_off</i>
            <span class="mdc-button__label">Off</span>
          </button>
        </div>
      </div>
    </div>

  </main>

  <footer>&copy; 2019
    <span>Made with
      <a href="https://material.io/develop/web/">Material Components for the Web</a>
    </span>
  </footer>

  <!-- Code dialog  -->
  <div id="codeDialog" class="mdc-dialog" role="alertdialog" aria-modal="true"
    aria-labelledby="code-dialog-title" aria-describedby="code-dialog-content" data-mdc-auto-init="MDCDialog">
    <div class="mdc-dialog__container">
      <div class="mdc-dialog__surface">
        <h2 class="mdc-dialog__title" id="code-dialog-title">ESP8266/Arduino Code</h2>
        <section class="mdc-dialog__content" id="code-dialog-content">
          <textarea id="codeDialogContent" rows="15" cols="60"></textarea>
        </section>
        <footer class="mdc-dialog__actions">
          <button type="button" class="mdc-button mdc-dialog__button mdc-button--raised" data-mdc-dialog-action="copy" onclick="onCopy()">
            <span class="mdc-button__label">Copy</span>
          </button>
          <button type="button" class="mdc-button mdc-dialog__button mdc-button--raised" data-mdc-dialog-action="close">
            <span class="mdc-button__label">Close</span>
          </button>
        </footer>
      </div>
    </div>
    <div class="mdc-dialog__scrim"></div>
  </div>



  <!--
    javascript goes here
  -->
  <script src="https://code.jquery.com/jquery-3.4.1.min.js"></script>
  <script src="https://cdnjs.cloudflare.com/ajax/libs/noUiSlider/14.0.2/nouislider.min.js"></script>
  <script src="https://unpkg.com/material-components-web@3.2.0/dist/material-components-web.min.js"></script>

  <script type="text/javascript">
    // global variables
    var pin = "?";
    var numPixels = 30;
    var brightness = 255;
    var segmentIndex = 0;
    var segments = [
      { start: 0, stop: 9, mode: 0, speed: 1000, options: 0, colors: ['#ff0000', '#00ff00', '#0000ff'] }
    ];

    mdc.autoInit(); // auto-init all of the Material Components for the Web components

    // onready function
    $(document).ready(function() {
      console.log("ready!");
      initSliders();
      getModes(); // get WS2812FX modes (callback execs updateWidgets())
    });

    function initSliders() { // init all the noUiSliders
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
          'min': [20, 1],
          '25%': [100, 10],
          '50%': [1000, 100],
          '75%': [10000, 500],
          'max': [30000]
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

    // retrieve the mode info from the web server in JSON format
    function getModes() {
      $("#modes").empty();
      $.getJSON("getmodes", function(data) {
        const modes = data.map(function (item, i) {
          return {'name':item, 'index':i};
        });
        // sort the modes so the UI has them in alphabetical order
        modes.sort((a, b) => a.name.localeCompare(b.name));

        $.each(modes, function (i, item) {
          $('#modes').append(new Option(item.name, item.index));
        });
        updateWidgets();
      });
    }

    function onAddSegment(event) {
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

    function onDeleteSegment(event, index) {
      event.preventDefault();
      event.stopPropagation();
      segments.splice(index, 1);
      if(segmentIndex >= segments.length) segmentIndex = segments.length - 1;
      updateWidgets();
    }
    
    function onChangeSegment(elem) {
      elem.addClass("active").siblings().removeClass('active');
      segmentIndex = elem.index();
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
      segments[segmentIndex].options |= $('#size').val() ? $('#size').val() << 1 : 0;

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
        $("#segmentList").append('<li class="mdc-list-item" onclick="onChangeSegment($(this))" data-mdc-auto-init="MDCRipple">' +
          '<span class="mdc-list-item__text">' + segments[i].start + ' - ' + segments[i].stop + ' : ' + getModeName(segments[i].mode) + '</span>' +
          '<span class="mdc-list-item__meta material-icons" aria-hidden="true" onclick="onDeleteSegment(event, ' + i + ')">delete</span>' +
          '</li>');
      }
      $("#segmentList li").eq(segmentIndex).addClass('mdc-list-item--selected');
      mdc.autoInit(document.getElementById('segmentList')); // reinit the list items to enable the ripple effect

      // update the UI widgets with the current segment's data
      if(segments.length > 0) {
        rangeSlider.noUiSlider.set([segments[segmentIndex].start, segments[segmentIndex].stop]);
        speedSlider.noUiSlider.set(segments[segmentIndex].speed);
        $('#modes').val(segments[segmentIndex].mode);
        $('#fade').val((segments[segmentIndex].options & 0x70) >> 4);
        $('#size').val((segments[segmentIndex].options & 0x06) >> 1);

//      $('#reverse').prop('checked', Boolean(segments[segmentIndex].options & 0x80));
//      $('#gamma').prop('checked',   Boolean(segments[segmentIndex].options & 0x08));
        document.getElementById('reverseSwitch').MDCSwitch.checked = Boolean(segments[segmentIndex].options & 0x80);
        document.getElementById('gammaSwitch').MDCSwitch.checked = Boolean(segments[segmentIndex].options & 0x08);
        
        $('#color0').val(segments[segmentIndex].colors[0]);
        $('#color1').val(segments[segmentIndex].colors[1]);
        $('#color2').val(segments[segmentIndex].colors[2]);
      }
    }

    // retrieve the segment info from the web server in JSON format
    function onLoad() {
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

    // send the segment info to the web server in JSON format
    function onSave() {
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

      jQuery.ajax({
        url: "setsegments",
        type: "POST",
        data: JSON.stringify(json),
        dataType: "json",
        contentType: "application/json; charset=utf-8",
        success: function() {
          console.log(data);
        }
      });
    }

    function onBuildCode() {
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

      $("#codeDialogContent").html(code);

      document.getElementById('codeDialog').MDCDialog.open();
    }

    function onCopy() { // copy code snippet to clipboard
      var content = document.getElementById("codeDialogContent");
      content.focus();
      content.select();
      document.execCommand('copy');
      window.getSelection().removeAllRanges();
      content.blur();
    }

    function onPause() { // pause the animation
      jQuery.ajax({
        url: "runcontrol",
        type: "POST",
        data: "pause",
        dataType: "json",
        contentType: "application/json; charset=utf-8",
        success: function() {
          console.log(data);
        }
      });
    }

    function onResume() { // resume from pause
      jQuery.ajax({
        url: "runcontrol",
        type: "POST",
        data: "resume",
        dataType: "json",
        contentType: "application/json; charset=utf-8",
        success: function() {
          console.log(data);
        }
      });
    }

    function onStop() { // stop animation
      jQuery.ajax({
        url: "runcontrol",
        type: "POST",
        data: "stop",
        dataType: "json",
        contentType: "application/json; charset=utf-8",
        success: function() {
          console.log(data);
        }
      });
    }

    function onStart() { // start animation
      jQuery.ajax({
        url: "runcontrol",
        type: "POST",
        data: "run",
        dataType: "json",
        contentType: "application/json; charset=utf-8",
        success: function() {
          console.log(data);
        }
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
  </script>
</body>
</html>
)=====";
