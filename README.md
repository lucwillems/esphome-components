# esphome-components
Bunch of esphome components

to include in your project see https://esphome.io/components/external_components.html

```

  external_components:
  # ext_threshold
  - source:
      type: git
      url: https://github.com/lucwillems/esphome-components
      ref: main
    components: [ ext_threshold ]

```

# ext_threshold

based on analog_threshold but keeps track of both ampltitude and time based data. it's also has auto calibration .
is compines Schmitt trigger function with ADC to convert incoming signals into binary_sensor data simular to a button

## example
see gas-counter-atom.yaml which use a optocoupler to read usage pulses from a gas meter.
it's build ontop of a M5Stack ATOM Lite platform  (esp32)


