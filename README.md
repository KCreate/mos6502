# MOS Technology 6502 Emulator

This is an emulator for the original MOS 6502 microcontroller. It is written completly from scratch using C++.
I am writing this as a school project for the TBZ (Technische Berufsschule Zürich).

# Features

TODO

# What is not implemented?

TODO

# Custom modifications

Some custom opcodes were added to the CPU. Since the original 6502 didn't use all available opcodes,
those were used. You can see a list of unused 6502 opcodes [here](http://www.oxyron.de/html/opcodes02.html).
Instead of halting the CPU, they were assigned the following meanings:

- `0x02 - WAI` Same behaviour as on the WDC 65C02.

# Contributors

- Leonard Schütz, Lead Developer [@KCreate](https://github.com/KCreate)

# License

[MIT License](LICENSE.md)
