# Welcome to panda

panda speaks CAN and CAN FD on the [STM32H725](https://www.st.com/resource/en/reference_manual/rm0468-stm32h723733-stm32h725735-and-stm32h730-value-line-advanced-armbased-32bit-mcus-stmicroelectronics.pdf).
The internal DOS panda in comma three (tici) is also supported on STM32F413, with classic CAN only.

### DOS firmware

Running `scons` in this directory builds both `board/obj/panda.bin.signed` and
`board/obj/bootstub.panda.bin` for DOS, alongside the existing H7 artifacts.
Python flashing/recovery selects images and flash geometry from the detected hardware.
Native pandad retains H7 SPI support and supports DOS over USB. DOS SPI can be
enabled for development with `ENABLE_SPI=1`.

DOS camera strobes use in-phase TIM1 channels at 20 Hz; H7 retains its TIM1/TIM8
staggered cabin strobe. Both use the current header-derived health/CAN version
hashes, 6-byte CAN headers and XOR checksums. F4 rejects FD and DLC > 8 packets
rather than truncating them. Its queues hold 1536 RX and 256 TX entries per bus
to fit the current 64-byte internal packet representation in SRAM.
DOS has no H7 sound/DTS peripheral: sound level is zero and the temperature
field uses the lower-bound sentinel (-40 °C); its health packet layout is unchanged.

After building, focused compatibility checks can be run with
`python -m pytest tests/usbprotocol/test_comms.py tests/usbprotocol/test_f4_compatibility.py`.
Physical flashing/recovery, CAN timing, harness orientation, fan/IR control and
camera strobe phase still require hardware-in-the-loop validation.

## Directory structure

```
.
├── board           # Code that runs on the STM32
├── python          # Python userspace library for interfacing with the panda
├── tests           # Tests for panda
├── scripts         # Miscellaneous used for panda development and debugging
├── examples        # Example scripts for using a panda in a car
```

## Safety Model

panda is compiled with vehicle-specific safety logic provided by [opendbc](https://github.com/commaai/opendbc). See details about the car safety models, safety testing, and code rigor in that repository.

## Code Rigor

The panda firmware is written for its use in conjunction with [openpilot](https://github.com/commaai/openpilot). The panda firmware, through its safety model, provides and enforces the
[openpilot safety](https://github.com/commaai/openpilot/blob/master/docs/SAFETY.md). Due to its critical function, it's important that the application code rigor within the `board` folder is held to high standards.

These are the [CI regression tests](https://github.com/commaai/panda/actions) we have in place:
* A generic static code analysis is performed by [cppcheck](https://github.com/danmar/cppcheck/).
* In addition, [cppcheck](https://github.com/danmar/cppcheck/) has a specific addon to check for [MISRA C:2012](https://misra.org.uk/) violations. See [current coverage](https://github.com/commaai/panda/blob/master/tests/misra/coverage_table).
* Compiler options are strict: the flags `-Wall -Wextra -Wstrict-prototypes -Werror` are enforced.
* The [safety logic](https://github.com/commaai/panda/tree/master/opendbc/safety) is tested and verified by [unit tests](https://github.com/commaai/panda/tree/master/opendbc/safety/tests) for each supported car variant to ensure that the behavior remains unchanged.
* A hardware-in-the-loop test verifies panda's functionalities on all active panda variants, including:
  * additional safety model checks
  * compiling and flashing the bootstub and app code
  * receiving, sending, and forwarding CAN messages on all buses
  * CAN loopback and latency tests through SPI

The above tests are themselves tested by:
* a [mutation test](tests/misra/test_mutation.py) on the MISRA coverage
* a [mutation test]([tests/misra/test_mutation.py](https://github.com/commaai/opendbc/blob/master/opendbc/safety/tests/mutation.sh)) on the vehicle-specific safety logic

## Usage

```bash
git clone https://github.com/commaai/panda.git
cd panda

# setup your environment
./setup.sh

# build fw + run the tests
./test.sh
```

See [the Panda class](https://github.com/commaai/panda/blob/master/python/__init__.py) for how to interact with the panda.

For example, to receive CAN messages:
``` python
>>> from panda import Panda
>>> panda = Panda()
>>> panda.can_recv()
```
And to send one on bus 0:
``` python
>>> from opendbc.car.structs import CarParams
>>> panda.set_safety_mode(CarParams.SafetyModel.allOutput)
>>> panda.can_send(0x1aa, b'message', 0)
```
Note that you may have to setup [udev rules](https://github.com/commaai/panda/tree/master/drivers/linux) for Linux, such as
``` bash
sudo tee /etc/udev/rules.d/11-panda.rules <<EOF
SUBSYSTEM=="usb", ATTRS{idVendor}=="0483", ATTRS{idProduct}=="df11", MODE="0666"
SUBSYSTEM=="usb", ATTRS{idVendor}=="3801", ATTRS{idProduct}=="ddcc", MODE="0666"
SUBSYSTEM=="usb", ATTRS{idVendor}=="3801", ATTRS{idProduct}=="ddee", MODE="0666"
SUBSYSTEM=="usb", ATTRS{idVendor}=="bbaa", ATTRS{idProduct}=="ddcc", MODE="0666"
SUBSYSTEM=="usb", ATTRS{idVendor}=="bbaa", ATTRS{idProduct}=="ddee", MODE="0666"
EOF
sudo udevadm control --reload-rules && sudo udevadm trigger
```

## Licensing

panda software is released under the MIT license unless otherwise specified.
