#include <pgmspace.h>

#ifndef _APP_JS_
#define _APP_JS_

const char app_js[] PROGMEM = R"=====(
// global variables
var presets = [ // create a default preset
    {name: 'Default', pin: 4, numPixels: 30, brightness: 64,
    segments: [
        { start:  0, stop:  9, mode: 0, speed: 1000, options:   0, colors: ['#ff0000', '#00ff00', '#0000ff'] },
        { start: 10, stop: 19, mode: 1, speed: 2000, options: 128, colors: ['#808000', '#008080', '#800080'] }
    ]}
];
var presetIndex = 0;
var segmentIndex = 0;

// init MDC components
const presetSelect = new mdc.MDCSelect(document.getElementById('mdc-select-preset'));
const copyPresetBtn = new mdc.MDCRipple(document.getElementById('mdc-button-copyPreset'));
const deletePresetBtn = new mdc.MDCRipple(document.getElementById('mdc-button-deletePreset'));
// const loadPresetBtn = new mdc.MDCRipple(document.getElementById('mdc-button-loadPreset'));
const savePresetBtn = new mdc.MDCRipple(document.getElementById('mdc-button-savePreset'));
const numPixelsTextField = new mdc.MDCTextField(document.getElementById('mdc-text-field-numPixels'));
const pinTextField = new mdc.MDCTextField(document.getElementById('mdc-text-field-pin'));
const segmentList = new mdc.MDCList(document.getElementById('mdc-list-segment'));
segmentList.singleSelection = true;
const addSegmentBtn = new mdc.MDCRipple(document.getElementById('mdc-button-addSegment'));
const showCodeBtn = new mdc.MDCRipple(document.getElementById('mdc-button-showCode'));
const copyCodeBtn = new mdc.MDCRipple(document.getElementById('mdc-button-copyCode'));
const reverseSwitch = new mdc.MDCSwitch(document.getElementById('mdc-switch-reverse'));
const gammaSwitch = new mdc.MDCSwitch(document.getElementById('mdc-switch-gamma'));
const modeSelect = new mdc.MDCSelect(document.getElementById('mdc-select-mode'));
const fadeSelect = new mdc.MDCSelect(document.getElementById('mdc-select-fade'));
const sizeSelect = new mdc.MDCSelect(document.getElementById('mdc-select-size'));
const powerSwitch = new mdc.MDCSwitch(document.getElementById('mdc-switch-power'));
const playSwitch = new mdc.MDCSwitch(document.getElementById('mdc-switch-play'));
const codeDialog = new mdc.MDCDialog(document.getElementById('mdc-dialog-code'));
const presetNameDialog = new mdc.MDCDialog(document.getElementById('mdc-dialog-presetName'));
const presetNameTextField = new mdc.MDCTextField(document.getElementById('mdc-text-field-presetName'));
const presetNameBtn = new mdc.MDCRipple(document.getElementById('mdc-button-presetName'));
const snackbar = new mdc.MDCSnackbar(document.getElementById('mdc-snackbar'));

// onready function
document.addEventListener("DOMContentLoaded", function () {
    console.log("DOMContentLoaded");
    init().catch(error => {
        console.log(error);
        snackbar.labelText = error;
        snackbar.open();
    });
});

async function init() {
    // retrieve the mode info from the ESP in JSON format
    console.log('loading modes...');
    let response = await fetch('/getmodes');
    if (!response.ok) {
        console.log(response);
        throw new Error(`Could not retrieve WS2812FX modes from your ESP: ${response.status} ${response.statusText}`);
    }

    const modes = await response.json(); // throws an error if can't parse the JSON
    console.log(modes);
    updateModes(modes);

    // retrieve the preset info from the ESP in JSON format
    console.log('loading presets...');
    response = await fetch('/loadpresets');
    if (!response.ok) {
        console.log(response);
        throw new Error(`Could not retrieve list of presets from your ESP: ${response.status} ${response.statusText}`);
    }

    let newPresets = await response.json(); // throws an error if can't parse the JSON
    if(newPresets && newPresets.length > 0) {
        presets = newPresets;
    }
    console.log(presets);
    updatePresets();

    initSliders();
    updateWidgets();
    savePreset();
    initEventListeners();
}

function updateModes(modes) {
    const modeSelectNode = document.querySelector('#mdc-select-mode select');
    while (modeSelectNode.firstChild) { // remove all child options
        modeSelectNode.removeChild(modeSelectNode.lastChild);
    }
    let newModes = modes.map(function (item, i) {
        return { 'name': item, 'index': i };
    });
    // sort the modes so the UI has them in alphabetical order
    newModes.sort((a, b) => a.name.localeCompare(b.name));
    newModes.forEach(mode => {
        modeSelectNode.append(new Option(mode.name, mode.index));
    });
}

function updatePresets() {
    const presetsNode = document.querySelector('#mdc-select-preset select');
    while (presetsNode.firstChild) { // remove all child options
        presetsNode.removeChild(presetsNode.lastChild);
    }
    presets.forEach((preset, i) => {
        presetsNode.append(new Option(preset.name, i));
    });
}

function initEventListeners() {
    presetSelect.listen('MDCSelect:change', () => {
        console.log(`Selected preset at index ${presetSelect.selectedIndex} with value "${presetSelect.value}"`);
        presetIndex = presetSelect.selectedIndex;
        segmentIndex = 0;
        updateWidgets();
        savePreset();
    });

    copyPresetBtn.listen('click', () => {
        presetNameDialog.open(); // open the preset name dialog
    });

    presetNameBtn.listen('click', () => {
        let presetName = presetNameTextField.value;
        if(presetName) {
            console.log(`presetName is ${presetName}, presetIndex is ${presetIndex}`);
            let presetsCopy = JSON.parse(JSON.stringify(presets[presetIndex])); // deep clone
            presetsCopy.name = presetName;
            presets.push(presetsCopy);
            document.querySelector('#mdc-select-preset select').append(new Option(presetName, presetIndex));
            presetSelect.selectedIndex = presets.length - 1; // eventlistener triggers updateWidgets() and savePreset()
        }
    });

    deletePresetBtn.listen('click', () => {
        if (presets.length > 1) {
            presets.splice(presetIndex, 1);
            updatePresets();
            presetSelect.selectedIndex = presetIndex - 1; // eventlistener triggers updateWidgets() and savePreset()
        }
    });

    savePresetBtn.listen('click', () => {
        sendPostReq('/savepresets', 'presets=' + JSON.stringify(presets));
    });

    const numPixelsInput = document.querySelector('#mdc-text-field-numPixels input');
    numPixelsInput.addEventListener('change', () => {
        console.log(`Text-field numPixels value is ${numPixelsTextField.value}`);
        presets[presetIndex].numPixels = Number(numPixelsTextField.value);
        rangeSlider.noUiSlider.updateOptions({
            range: {
                'min': 0,
                'max': Math.max(1, presets[presetIndex].numPixels - 1)
            }
        });
        updateSegmentList();
        savePreset();
    });

    const pinInput = document.querySelector('#mdc-text-field-pin input');
    pinInput.addEventListener('change', () => {
        console.log(`Text-field pin value is ${pinTextField.value}`);
        presets[presetIndex].pin = Number(pinTextField.value);
        savePreset();
    });

    document.getElementById('mdc-button-addSegment').onclick = function (event) {
        // event.preventDefault();
        // event.stopPropagation();

        if (presets[presetIndex].segments.length < 10) { // max 10 segments
            // calc the new segment's start LED by adding one to the maximum stop led
            var start = 0;
            for (var i = 0; i < presets[presetIndex].segments.length; i++) {
                if (presets[presetIndex].segments[i].stop >= start) start = presets[presetIndex].segments[i].stop + 1;
            }
            if (start > presets[presetIndex].numPixels - 1) start = presets[presetIndex].numPixels - 1;
        
            presets[presetIndex].segments.push({ start: start, stop: presets[presetIndex].numPixels - 1, mode: 0, speed: 1000, options: 0, colors: ['#ff0000', '#00ff00', '#0000ff'] });
            segmentIndex = presets[presetIndex].segments.length - 1;
            updateWidgets();
            savePreset();
        }
    }

    document.getElementById('color0').onchange = function (event) {
        presets[presetIndex].segments[segmentIndex].colors[0] = document.getElementById('color0').value;
        savePreset();
    }
    document.getElementById('color1').onchange = function (event) {
        presets[presetIndex].segments[segmentIndex].colors[1] = document.getElementById('color1').value;
        savePreset();
    }
    document.getElementById('color2').onchange = function (event) {
        presets[presetIndex].segments[segmentIndex].colors[2] = document.getElementById('color2').value;
        savePreset();
    }

    const reverseInput = document.querySelector('#mdc-switch-reverse input');
    reverseInput.addEventListener('click', () => {
        console.log(`Switch reverse checked is ${reverseSwitch.checked}`);
        if(reverseSwitch.checked) {
            presets[presetIndex].segments[segmentIndex].options |= 0x80; // set reverse bit
        } else {
            presets[presetIndex].segments[segmentIndex].options &= 0x7f; // clear reverse bit
        }
        savePreset();
    });

    const gammaInput = document.querySelector('#mdc-switch-gamma input');
    gammaInput.addEventListener('click', () => {
        console.log(`Switch gamma checked is ${gammaSwitch.checked}`);
        if(gammaSwitch.checked) {
            presets[presetIndex].segments[segmentIndex].options |= 0x08; // set gamma bit
        } else {
            presets[presetIndex].segments[segmentIndex].options &= 0xf7; // clear gamma bit
        }
        savePreset();
    });

     // add the event listener to the native select node, instead of the MDC component, to
     // avoid calling the event handler when the widget is updating in updateWidget().
     const modeSelectNode = document.querySelector('#mdc-select-mode select');
     modeSelectNode.onchange = function (event) {
         console.log(`Selected mode at index ${modeSelect.selectedIndex} with value "${modeSelect.value}"`);
         presets[presetIndex].segments[segmentIndex].mode = Number(modeSelect.value);
         updateSegmentList(); // update text in segment list
         savePreset();
     }

    const fadeSelectNode = document.querySelector('#mdc-select-fade select');
     fadeSelectNode.onchange = function (event) {
        console.log(`Selected fade at index ${fadeSelect.selectedIndex} with value "${fadeSelect.value}"`);
         let fadeValue = Number(fadeSelect.value) & 0x07;
         presets[presetIndex].segments[segmentIndex].options &= 0x8f; // clear 3 fade bits
         presets[presetIndex].segments[segmentIndex].options |= (fadeValue << 4);
         savePreset();
     }

    const sizeSelectNode = document.querySelector('#mdc-select-size select');
     sizeSelectNode.onchange = function (event) {
      console.log(`Selected size at index ${sizeSelect.selectedIndex} with value "${sizeSelect.value}"`);
         let sizeValue = Number(sizeSelect.value) & 0x03;
         presets[presetIndex].segments[segmentIndex].options &= 0xf9; // clear 2 size bits
         presets[presetIndex].segments[segmentIndex].options |= sizeValue << 1;
         savePreset();
     }

    const powerInput = document.querySelector('#mdc-switch-power input');
    powerInput.addEventListener('click', () => {
        console.log(`Switch power checked is ${powerSwitch.checked}`);
        if(powerSwitch.checked) {
            sendPostReq('/runcontrol', 'action=run');
        } else {
            sendPostReq('/runcontrol', 'action=stop');
        }
    });

    const playInput = document.querySelector('#mdc-switch-play input');
    playInput.addEventListener('click', () => {
        console.log(`Switch play checked is ${playSwitch.checked}`);
        if(playSwitch.checked) {
            sendPostReq('/runcontrol', 'action=resume');
        } else {
            sendPostReq('/runcontrol', 'action=pause');
        }
    });

    showCodeBtn.listen('click', () => {
        onBuildCode();
    });

    copyCodeBtn.listen('click', () => {
        onCopyCode();
    });

    // slider event listeners
    document.getElementById('rangeSlider').noUiSlider.on('end', function () {
        presets[presetIndex].segments[segmentIndex].start = parseInt(rangeSlider.noUiSlider.get()[0]);
        presets[presetIndex].segments[segmentIndex].stop = parseInt(rangeSlider.noUiSlider.get()[1]);
        updateWidgets(); // update text in segment list
        savePreset();
    });

    document.getElementById('brightnessSlider').noUiSlider.on('end', function () {
        presets[presetIndex].brightness = Number(document.getElementById('brightnessSlider').noUiSlider.get());
        savePreset();
    });

    document.getElementById('speedSlider').noUiSlider.on('end', function () {
        presets[presetIndex].segments[segmentIndex].speed = parseInt(speedSlider.noUiSlider.get());
        savePreset();
    });
}

function onDeleteSegment(index) {
    if (presets[presetIndex].segments.length > 1) {
      presets[presetIndex].segments.splice(index, 1);
      if(index == segmentIndex) { // deleted the current segment?
        segmentIndex --;
        updateWidgets();
      } else {
        updateSegmentList();
      }
      savePreset();
    }
}

function onChangeSegment(index) {
    segmentIndex = index;
    updateWidgets();
}

function updateSegmentList() {
    const segmentListNode = document.getElementById('mdc-list-segment');
    while (segmentListNode.firstChild) {
        segmentListNode.removeChild(segmentListNode.lastChild);
    }
    presets[presetIndex].segments.forEach((segment, i) => {
        let liStr;
        if(i == segmentIndex) {
            liStr = '<li class="mdc-list-item mdc-list-item--selected" role="option" aria-selected="true" tabindex="0">';
        } else {
            liStr = '<li class="mdc-list-item" role="option" tabindex="-1">';
        }
        let domFragment = document.createRange().createContextualFragment(liStr +
            '<span class="mdc-list-item__text">' + segment.start + ' - ' + segment.stop + ' : ' + getModeName(segment.mode) + '</span>' +
            '<span class="mdc-list-item__meta material-icons" aria-hidden="true">delete</span>' +
            '</li>');
        segmentListNode.append(domFragment);

        segmentListNode.lastChild.onclick = function (event) {
            onChangeSegment(i);
        }
        segmentListNode.lastChild.getElementsByClassName('material-icons')[0].onclick = function (event) {
            // event.preventDefault();
            event.stopPropagation(); // prevent onChangeSegment from firing too
            onDeleteSegment(i);
        }
    });
}

// update GUI widgets
function updateWidgets() {

    numPixelsTextField.value = presets[presetIndex].numPixels;
    rangeSlider.noUiSlider.updateOptions({
        range: {
            'min': 0,
            'max': Math.max(1, presets[presetIndex].numPixels - 1)
        }
    });

    pinTextField.value = presets[presetIndex].pin;
    brightnessSlider.noUiSlider.set(presets[presetIndex].brightness);

    updateSegmentList();

    // update the UI widgets with the current segment's data
    if (presets[presetIndex].segments.length > 0) {
        const seg = presets[presetIndex].segments[segmentIndex];
        rangeSlider.noUiSlider.set([seg.start, seg.stop]);
        speedSlider.noUiSlider.set(seg.speed);
        modeSelect.value = seg.mode;
        fadeSelect.value = (seg.options & 0x70) >> 4;
        sizeSelect.value = (seg.options & 0x06) >> 1;
        reverseSwitch.checked = Boolean(seg.options & 0x80);
        gammaSwitch.checked = Boolean(seg.options & 0x08);
        document.getElementById('color0').value = seg.colors[0];
        document.getElementById('color1').value = seg.colors[1];
        document.getElementById('color2').value = seg.colors[2];
    }
}

// send the current preset's info to the ESP in JSON format
function savePreset() {
    let presetCopy = JSON.parse(JSON.stringify(presets[presetIndex])); // deep clone
    // transform the color info from '#000000' hex string format into integers 
    presetCopy.segments.forEach(segment => {
        segment.colors = [
            parseInt(segment.colors[0].replace('#', ''), 16),
            parseInt(segment.colors[1].replace('#', ''), 16),
            parseInt(segment.colors[2].replace('#', ''), 16)
        ];
    });

    fetch('/savePreset', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(presetCopy)
    }).then(response => response.text())
    .then(data => console.log(data));
}

function onBuildCode() {
    let code = '#include <WS2812FX.h>\n\n'
    code += '#define LED_PIN ' + presets[presetIndex].pin + '\n';
    code += '#define LED_COUNT ' + presets[presetIndex].numPixels + '\n\n';
    code += 'WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);\n\n';
    code += 'void setup() {\n';
    code += '  ws2812fx.setBrightness(' + presets[presetIndex].brightness + ');\n\n';

    presets[presetIndex].segments.forEach((segment, i) => {
        //  uint32_t colors0[] = {0xff0000, 0x000000, 0x000000};
        code += '  uint32_t colors' + i + '[] = {' +
            segment.colors[0].replace('#', '0x') + ', ' +
            segment.colors[1].replace('#', '0x') + ', ' +
            segment.colors[2].replace('#', '0x') + '};\n';

        // ws2812fx.setSegment(0, 0, 9, 53, colors0, 240, false);
        code += '  ws2812fx.setSegment(' +
            i + ', ' +
            segment.start + ', ' +
            segment.stop + ', ' +
            segment.mode + ', ' +
            'colors' + i + ', ' +
            segment.speed + ', ' +
            segment.options +
            '); // ' + getModeName(segment.mode) + '\n\n';
    });

    code += '  ws2812fx.start();\n';
    code += '}\n\n';
    code += 'void loop() {\n';
    code += '  ws2812fx.service();\n';
    code += '}';

    document.getElementById('codeDialogContent').value = code;

    // document.getElementById('codeDialog').MDCDialog.open();
    codeDialog.open();
}

function onCopyCode() { // copy code snippet to clipboard
    var content = document.getElementById("codeDialogContent");
    content.focus();
    content.select();
    document.execCommand('copy');
    window.getSelection().removeAllRanges();
    content.blur();
}

function sendPostReq(uri, params) {
    fetch(uri, {
        method: 'POST',
        headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
        body: params
    }).then(response => response.text())
        .then(data => console.log(data));
}

function getModeName(index) {
    let name = 'undefined';
    const modeOptions = document.querySelector('#mdc-select-mode select').querySelectorAll('option');
    // let modeOptions = document.getElementById('modes').querySelectorAll('option');
    modeOptions.forEach((option) => {
        if (index == option.value) {
            name = option.text;
        }
    })
    return name;
}

function initSliders() { // init all the noUiSliders
    noUiSlider.create(document.getElementById('rangeSlider'), {
        start: [0, 99],
        tooltips: [true, true],
        connect: [false, true, false],
        range: {
            'min': 0,
            'max': presets[presetIndex].numPixels - 1
        },
        format: {
            to: function (value) {
                return value.toFixed(0);
            },
            from: function (value) {
                return parseInt(value, 10);
            }
        }
    });

    noUiSlider.create(document.getElementById('brightnessSlider'), {
        start: 64,
        tooltips: true,
        connect: [true, false],
        range: {
            'min': 0,
            'max': 255
        },
        format: {
            to: function (value) {
                return value.toFixed(0);
            },
            from: function (value) {
                return parseInt(value, 10);
            }
        }
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
            to: function (value) {
                return value.toFixed(0);
            },
            from: function (value) {
                return parseInt(value, 10);
            }
        }
    });
}
)=====";
#endif
