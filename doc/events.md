# Events

Zircon uses events to allow user interaction into the simulator.
We define a handful of event locations, see the table below.
Users can either add callbacks to these events via the interface or directly write new callbacks in the code and rebuild the simulator.

Note: the exact line numbers may not be up-to-date, but the file names should be.

|  Subsystem |                 Name |                                        Description |                                          Callback Parameters |                  Location |
|        --- |                  --- |                                                --- |                                                          --- |                       --- |
|        mem |           event_read |                          Fires when memory is read |                               (address, value read, n bytes) | src/mem/memory-image.h:82 |
|        mem |          event_write |                       Fires when memory is written |                 (address, value written, old value, n bytes) | src/mem/memory-image.h:86 |
|        mem |      event_exception |                            Currently unimplemented |                                                              | src/mem/memory-image.h:90 |
|        mem |     event_allocation |                     Fires when memory is allocated |                              (base address, allocation size) | src/mem/memory-image.h:94 |
| reg_$CLASS |           event_read |                      Fires when a register is read |                 (register class, register index, value read) | src/cpu/isa/register.h:95 |
| reg_$CLASS |          event_write |                   Fires when a register is written |   (register class, register index, value written, old value) | src/cpu/isa/register.h:99 |
|       hart | event_before_execute |  Fires just before current instruction is executed |                                          (Hart State object) |          src/cpu/cpu.h:74 |
|       hart |  event_after_execute |   Fires just after current instruction is executed |                                          (Hart State object) |          src/cpu/cpu.h:78 |

