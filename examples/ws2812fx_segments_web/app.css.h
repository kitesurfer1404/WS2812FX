#include <pgmspace.h>

#ifndef _APP_CSS_
#define _APP_CSS_

const char app_css[] PROGMEM = R"=====(
/* tweaks for Material Design Components */
body {
    margin: 0; /* remove the default 8px body margin to fix MDC top-app-bar placement */
    /* --mdc-theme-primary: #3fb8af; */ /* change MDC color theme to match noUiSlide's theme */
}
.mdc-top-app-bar { /* force the top bar text color to white */
    color: #ffffff;
}
main, .mdc-switch {
    margin: 0px 8px; /* add a little margin the MDC switches */
}
h3 {
    margin: 4px 0px; /* add a little margin to the widget labels */
}
/* MDC select and text-field labels are hardcoded to purple??? reset to primary color */
/* .mdc-select:not(.mdc-select--disabled).mdc-select--focused .mdc-floating-label {
    color: var(--mdc-theme-primary, #3fb8af) 
}
.mdc-text-field--focused:not(.mdc-text-field--disabled) .mdc-floating-label {
    color: var(--mdc-theme-primary, #3fb8af);
} */
.mdc-list, div.border { /* add border around MDC lists */
    border: 1px solid rgba(0, 0, 0, 0.1);
    padding: 0px;
}
.mdc-list-item { /* make list items a little shorter (default is 48px) */
    height: 32px;
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
footer {
    margin: 4px 16px; /* add a little margin to the footer content */
}

div.fine-border { /* add a border */
    border: 1px solid rgba(0, 0, 0, 0.1);
    padding: 4px 16px;
}
)=====";
#endif
