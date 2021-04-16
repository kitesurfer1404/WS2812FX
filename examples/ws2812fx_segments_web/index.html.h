#include <pgmspace.h>

#ifndef _INDEX_HTML_
#define _INDEX_HTML_

const char index_html[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>

<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0" />

  <!-- <link rel="stylesheet" href="https://fonts.googleapis.com/icon?family=Material+Icons"> -->
  <!-- <link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/nouislider@14.6.4/distribute/nouislider.min.css"> -->
  <!-- <link rel="stylesheet" href="https://unpkg.com/material-components-web@3.2.0/dist/material-components-web.min.css"> -->
  <link rel="stylesheet" href="/bundle.css">
  <link rel="stylesheet" href="/app.css">

  <style>
    /* CSS for local Google Material Icons */
    @font-face {
      font-family: 'Material Icons';
      font-style: normal;
      font-weight: 400;
      src: url(/Material-Icons-subset.woff2) format('woff2');
    }

    .material-icons {
      font-family: 'Material Icons';
      font-weight: normal;
      font-style: normal;
      font-size: 24px;
      line-height: 1;
      letter-spacing: normal;
      text-transform: none;
      display: inline-block;
      white-space: nowrap;
      word-wrap: normal;
      direction: ltr;
      -webkit-font-feature-settings: 'liga';
      font-feature-settings: 'liga';
      -webkit-font-smoothing: antialiased;
    }
  </style>
</head>

<body class="mdc-typography">

  <!-- <header class="mdc-top-app-bar"> -->
  <header class="mdc-top-app-bar mdc-top-app-bar--dense">
    <div class="mdc-top-app-bar__row">
      <section class="mdc-top-app-bar__section mdc-top-app-bar__section--align-start">
        <span class="mdc-top-app-bar__title">WS2812FX Web Interface</span>
      </section>
    </div>
  </header>

  <main class="mdc-top-app-bar--dense-fixed-adjust">

    <div id="mdc-snackbar" class="mdc-snackbar">
      <div class="mdc-snackbar__surface" role="status" aria-relevant="additions">
        <div class="mdc-snackbar__label" aria-atomic="false">
          Error connecting to your ESP. Make sure it's turned on, then refresh the browser.
        </div>
      </div>
    </div>

    <div class="mdc-layout-grid">

      <div class="mdc-layout-grid__inner">
        <div class="mdc-layout-grid__cell mdc-layout-grid__cell--span-12">
          <div id="mdc-select-preset" class="mdc-select mdc-select--outlined">
            <i class="mdc-select__dropdown-icon"></i>
            <select id="presetSelect" class="mdc-select__native-control">
              <option value="0" selected>Default</option>
            </select>
            <div class="mdc-notched-outline">
              <div class="mdc-notched-outline__leading"></div>
              <div class="mdc-notched-outline__notch">
                <label class="mdc-floating-label mdc-floating-label--float-above">Presets</label>
              </div>
              <div class="mdc-notched-outline__trailing"></div>
            </div>
          </div>

          <button id="mdc-button-copyPreset" class="mdc-button mdc-button--raised">
            <i class="material-icons mdc-button__icon" aria-hidden="true">content_copy</i>
            <span class="mdc-button__label">Copy Preset</span>
          </button>
          <button id="mdc-button-deletePreset" class="mdc-button mdc-button--raised">
            <i class="material-icons mdc-button__icon" aria-hidden="true">delete</i>
            <span class="mdc-button__label">Delete Preset</span>
          </button>

          <button id="mdc-button-savePreset" class="mdc-button mdc-button--raised">
            <i class="material-icons mdc-button__icon" aria-hidden="true">save</i>
            <span class="mdc-button__label">Save Presets</span>
          </button>
        </div>
      </div>
      <hr />

      <div class="mdc-layout-grid__inner">
        <div class="mdc-layout-grid__cell mdc-layout-grid__cell--span-6">
          <p>
            <label id="mdc-text-field-numPixels" class="mdc-text-field mdc-text-field--outlined">
              <span class="mdc-notched-outline">
                <span class="mdc-notched-outline__leading"></span>
                <span class="mdc-notched-outline__notch">
                  <span class="mdc-floating-label" id="numPixelsTextLabel">Number of Pixels</span>
                </span>
                <span class="mdc-notched-outline__trailing"></span>
              </span>
              <input type="text" class="mdc-text-field__input" aria-labelledby="numPixelsTextLabel" value="30">
            </label>

            <label id="mdc-text-field-pin" class="mdc-text-field mdc-text-field--outlined">
              <span class="mdc-notched-outline">
                <span class="mdc-notched-outline__leading"></span>
                <span class="mdc-notched-outline__notch">
                  <span class="mdc-floating-label" id="pinTextLabel">GPIO Pin</span>
                </span>
                <span class="mdc-notched-outline__trailing"></span>
              </span>
              <input type="text" class="mdc-text-field__input" aria-labelledby="pinTextLabel" value="4">
            </label>
          </p>

          <p>
          <h3 class="mdc-typography--subtitle1">Brightness</h3>
          <div id="brightnessSlider"></div>
          </p>

          <p>
            <h2 class="mdc-typography--subtitle1">Segments</h2>
            <ul id="mdc-list-segment" class="mdc-list" role="listbox"></ul>
          </p>

          <p>
            <button id="mdc-button-addSegment" class="mdc-button mdc-button--raised">
              <i class="material-icons mdc-button__icon" aria-hidden="true">add</i>
              <span class="mdc-button__label">Add Segment</span>
            </button>
            <button id="mdc-button-showCode" class="mdc-button mdc-button--raised">
              <i class="material-icons mdc-button__icon" aria-hidden="true">code</i>
              <span class="mdc-button__label">Show Code</span>
            </button>
          </p>
        </div>

        <div class="mdc-layout-grid__cell mdc-layout-grid__cell--span-6 fine-border">
          <p>
          <h3 class="mdc-typography--subtitle1">Segment Range</h3>
          <div id="rangeSlider"></div>
          </p>

          <p>
          <h3 class="mdc-typography--subtitle1">Segment Speed</h3>
          <div id="speedSlider"></div>
          </p>

          <div class="mdc-layout-grid__inner">
            <div class="mdc-layout-grid__cell mdc-layout-grid__cell--span-6">
              <p>
              <h3 class="mdc-typography--subtitle1" style="display: inline-block">Colors</h3>
              <input type="color" id="color0">
              <input type="color" id="color1">
              <input type="color" id="color2">
              </p>

              <p>
              <h3 class="mdc-typography--subtitle1" style="display: inline-block">Reverse</h3>
              <div id="mdc-switch-reverse" class="mdc-switch">
                <div class="mdc-switch__track"></div>
                <div class="mdc-switch__thumb-underlay">
                  <div class="mdc-switch__thumb">
                    <input type="checkbox" id="reverse" class="mdc-switch__native-control" role="switch">
                  </div>
                </div>
              </div>
              <label for="reverse">off/on</label>
              </p>

              <p>
              <h3 class="mdc-typography--subtitle1" style="display: inline-block">Gamma Correction</h3>
              <div id="mdc-switch-gamma" class="mdc-switch">
                <div class="mdc-switch__track"></div>
                <div class="mdc-switch__thumb-underlay">
                  <div class="mdc-switch__thumb">
                    <input type="checkbox" id="gamma" class="mdc-switch__native-control" role="switch">
                  </div>
                </div>
              </div>
              <label for="gamma">off/on</label>
              </p>
            </div>

            <div class="mdc-layout-grid__cell mdc-layout-grid__cell--span-6">
              <p>
              <div id="mdc-select-mode" class="mdc-select mdc-select--outlined">
                <i class="mdc-select__dropdown-icon"></i>
                <select class="mdc-select__native-control">
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
              </p>

              <p>
              <div id="mdc-select-fade" class="mdc-select mdc-select--outlined">
                <i class="mdc-select__dropdown-icon"></i>
                <select class="mdc-select__native-control">
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
              </p>

              <p>
              <div id="mdc-select-size" class="mdc-select mdc-select--outlined">
                <i class="mdc-select__dropdown-icon"></i>
                <select class="mdc-select__native-control">
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
              </p>

            </div>
          </div>
        </div>
      </div>

      <div class="mdc-layout-grid__inner">
        <div class="mdc-layout-grid__cell mdc-layout-grid__cell--span-12">
          <div id="mdc-switch-power" class="mdc-switch mdc-switch--checked">
            <div class="mdc-switch__track"></div>
            <div class="mdc-switch__thumb-underlay">
              <div class="mdc-switch__thumb">
                <input type="checkbox" id="powerSwitch" class="mdc-switch__native-control" role="switch" checked>
              </div>
            </div>
          </div>
          <label for="powerSwitch" style="margin-right:20px;">LEDs off/on</label>

          <div id="mdc-switch-play" class="mdc-switch mdc-switch--checked">
            <div class="mdc-switch__track"></div>
            <div class="mdc-switch__thumb-underlay">
              <div class="mdc-switch__thumb">
                <input type="checkbox" id="playSwitch" class="mdc-switch__native-control" role="switch" checked>
              </div>
            </div>
          </div>
          <label for="playSwitch">LEDs pause/play</label>
        </div>
      </div>
    </div>

  </main>

  <footer>
    <div>&copy; 2021
      <span>Made with
        <a href="https://material.io/develop/web/">Material Components for the Web</a>
      </span>
    </div>
  </footer>

  <!-- Code dialog  -->
  <div id="mdc-dialog-code" class="mdc-dialog" role="alertdialog" aria-modal="true" aria-labelledby="code-dialog-title" aria-describedby="code-dialog-content">
    <div class="mdc-dialog__container">
      <div class="mdc-dialog__surface">
        <h2 class="mdc-dialog__title" id="code-dialog-title">ESP8266/Arduino Code</h2>
        <section class="mdc-dialog__content" id="code-dialog-content">
          <textarea id="codeDialogContent" rows="15" cols="60"></textarea>
        </section>

        <footer class="mdc-dialog__actions">
          <button id="mdc-button-copyCode" type="button" class="mdc-button mdc-dialog__button mdc-button--raised" data-mdc-dialog-action="copy">
            <span class="mdc-button__label">Copy To Clipboard</span>
          </button>
          <button type="button" class="mdc-button mdc-dialog__button mdc-button--raised" data-mdc-dialog-action="close">
            <span class="mdc-button__label">Close</span>
          </button>
        </footer>
      </div>
    </div>
    <div class="mdc-dialog__scrim"></div>
  </div>

  <!-- Preset name dialog  -->
  <div id="mdc-dialog-presetName" class="mdc-dialog" role="alertdialog" aria-modal="true" aria-labelledby="presetName-dialog-title" aria-describedby="presetName-dialog-content">
    <div class="mdc-dialog__container">
      <div class="mdc-dialog__surface">
        <h2 class="mdc-dialog__title" id="presetName-dialog-title">Enter a preset name</h2>
        <section class="mdc-dialog__content" id="presetName-dialog-content">
          <label id="mdc-text-field-presetName" class="mdc-text-field mdc-text-field--outlined mdc-text-field--no-label">
            <span class="mdc-notched-outline">
              <span class="mdc-notched-outline__leading"></span>
              <span class="mdc-notched-outline__trailing"></span>
            </span>
            <input type="text" class="mdc-text-field__input" aria-label="Label" value="">
          </label>
        </section>

        <footer class="mdc-dialog__actions">
          <button id="mdc-button-presetName" type="button" class="mdc-button mdc-dialog__button mdc-button--raised" data-mdc-dialog-action="ok">
            <span class="mdc-button__label">OK</span>
          </button>
          <button type="button" class="mdc-button mdc-dialog__button mdc-button--raised" data-mdc-dialog-action="close">
            <span class="mdc-button__label">Cancel</span>
          </button>
        </footer>

      </div>
    </div>
    <div class="mdc-dialog__scrim"></div>
  </div>

  <!-- <script src="https://cdn.jsdelivr.net/npm/nouislider@14.6.4/distribute/nouislider.min.js"></script> -->
  <!-- <script src="https://unpkg.com/material-components-web@3.2.0/dist/material-components-web.min.js"></script> -->
  <script src="/bundle.js"></script>
  <script src="/app.js"></script>

  <!-- <script>window.mdc.autoInit();</script> -->
</body>

</html>
)=====";
#endif
